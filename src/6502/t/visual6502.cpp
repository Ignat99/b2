#define _CRT_NONSTDC_NO_DEPRECATE
#include <shared/system.h>
#include <shared/testing.h>
#include <shared/log.h>
#include <shared/debug.h>
#include <shared/path.h>
#include <6502/6502.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
extern "C" {
#include <types.h>
#include <perfect6502.h>
#include <netlist_sim.h>
//#include <netlist_6502.h>
}
#include <stdlib.h>
#include <limits.h>
#include <string>
#include <vector>

LOG_DEFINE(DEBUG, "", &log_printer_stdout_and_debugger);

#ifndef VISUAL6502_PATH
#error
#endif

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
// Compare 6502 simulator against perfect6502.
//
// The original plan was just to use visual6502 URLs: design tests in
// the Visual6502 simulator page, then copy and paste URL into code.
// So there's a bunch of code to run stuff based on the data in a
// Visual6502 URL. But this proved rather unwieldy; I found it a bit
// hard to keep on top of all those long URLs, especially when there
// are several related tests that differ only in a couple of settings.
//
// So the tests are generated by the code in AddTestCases, then turned
// into a URL, and then the URL is used to run the tests :-/ - though
// it's not totally useless, because that way it can also output an
// HTML page with some clickable links.
//
// Limitations:
//
// - only a subset of query parameters are supported: a, d, r, irq0,
//   irq1, nmi0, nmi1, steps (others are ignored)
//
// - IRQ/NMI signals can go low or high on cycle boundaries only
//
// Note also: the CPU state isn't checked on every cycle, only at instruction
// boundaries (as indicated by the SYNC signal). The simulator promises to (try
// to) replicate the address bus, data bus and rw line behaviour, so those are
// checked every cycle, but doesn't promise more than that.
//
// Would be nice to get the program counter matched up though...
//
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Initial contents of the visual6502 sim memory, starting at 0x0000.
static const uint8_t DEFAULT_MEMORY[] = {
    0xa9,
    0x00,
    0x20,
    0x10,
    0x00,
    0x4c,
    0x02,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x40,
    0xe8,
    0x88,
    0xe6,
    0x0f,
    0x38,
    0x69,
    0x02,
    0x60,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct PinState {
    int cycle;
    int level;
    bool seen;

    PinState(int level, int cycle);
};

PinState::PinState(int level_, int cycle_)
    : cycle(cycle_)
    , level(level_)
    , seen(false) {
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static uint8_t g_mem[65536];
static std::vector<PinState> g_irqs, g_nmis;
static int g_num_test_cycles;

/* static void ClearMem(void) { */
/*     memset(g_mem,0,65536); */
/* } */

static uint8_t unhex(char c) {
    if (c >= '0' && c <= '9') {
        return (uint8_t)(c - '0');
    } else if (c >= 'a' && c <= 'f') {
        return (uint8_t)(c - 'a' + 10);
    } else if (c >= 'A' && c <= 'F') {
        return (uint8_t)(c - 'A' + 10);
    } else {
        TEST_TRUE(0);
        return 0;
    }
}

/* static void LoadMem(uint16_t addr,const char *str) { */
/*     const char *c=str; */
/*     while(*c!=0) { */
/*         TEST_TRUE(isspace(*c)||isxdigit(*c)); */
/*         if(isspace(*c)) { */
/*             // ignore. */
/*             ++c; */
/*         } else if(isxdigit(*c)) { */
/*             TEST_TRUE(isxdigit(c[1])); */
/*             g_mem[addr++]=(unhex(*c)<<4)|unhex(c[1]); */
/*             c+=2; */
/*         } */
/*     } */
/* } */

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

std::string strprintfv(const char *fmt, va_list v) {
    char *str;
    if (vasprintf(&str, fmt, v) == -1) {
        perror("vasprintf failed");
        exit(1);
    }

    std::string result(str);

    free(str);
    str = NULL;

    return result;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

std::string strprintf(const char *fmt, ...) {
    va_list v;

    va_start(v, fmt);
    std::string result = strprintfv(fmt, v);
    va_end(v);

    return result;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//static const char URL_PREFIX[]="http://visual6502.org/JSSim/expert.html?";

static int GetInt(const char *str, int num_chars, int base) {
    std::string tmp;
    if (num_chars < 0) {
        tmp = str;
    } else {
        tmp = std::string(str, str + num_chars);
    }

    char *ep;
    long l = strtol(tmp.c_str(), &ep, base);

    TEST_EQ_II(*ep, 0);
    TEST_TRUE(l >= INT_MIN && l <= INT_MAX);

    return (int)l;
}

static void AddIRQ(int level, const char *v) {
    int irq_cycle = GetInt(v, -1, 10);
    TEST_TRUE(irq_cycle % 2 == 0);
    g_irqs.emplace_back(level, irq_cycle);
}

static void AddNMI(int level, const char *v) {
    int nmi_cycle = GetInt(v, -1, 10);
    TEST_TRUE(nmi_cycle % 2 == 0);
    g_nmis.emplace_back(level, nmi_cycle);
}

static void InitVisual6502(const std::string &url) {
    g_irqs.clear();
    g_nmis.clear();

    memset(g_mem, 0, 65536);
    memcpy(g_mem, DEFAULT_MEMORY, sizeof DEFAULT_MEMORY);

    if (!url.empty()) {
        std::string::size_type query_pos = url.find_first_of("?");
        TEST_TRUE(query_pos != std::string::npos);

        char *args = strdup(url.c_str() + query_pos + 1);

        int address = -1;

        // this isn't the cleverest
        for (char *tok = strtok(args, "&"); tok; tok = strtok(NULL, "&")) {
            char *k = strdup(tok);

            char *v = strchr(k, '=');
            TEST_NON_NULL(v);
            *v++ = 0;

            if (strcmp(k, "a") == 0) {
                address = GetInt(v, -1, 16);
            } else if (strcmp(k, "d") == 0) {
                TEST_EQ_UU(strlen(v) % 2, 0);
                for (size_t i = 0; v[i] != 0; i += 2) {
                    g_mem[(uint16_t)address++] = (uint8_t)GetInt(v + i, 2, 16);
                }
            } else if (strcmp(k, "r") == 0) {
                int r = GetInt(v, -1, 16);
                g_mem[0xfffc] = (uint8_t)(r >> 0);
                g_mem[0xfffd] = (uint8_t)(r >> 8);
            } else if (strcmp(k, "irq0") == 0) {
                AddIRQ(0, v);
            } else if (strcmp(k, "irq1") == 0) {
                AddIRQ(1, v);
            } else if (strcmp(k, "nmi0") == 0) {
                AddNMI(0, v);
            } else if (strcmp(k, "nmi1") == 0) {
                AddNMI(1, v);
            } else if (strcmp(k, "steps") == 0) {
                int steps = GetInt(v, -1, 10);
                TEST_TRUE(steps % 2 == 0);
                g_num_test_cycles = steps / 2;
            } else if (strcmp(k, "r") == 0) {
                address = GetInt(v, -1, 16);
                TEST_EQ_II(address & ~0xffff, 0);
                g_mem[0xfffc] = (uint8_t)(address >> 0);
                g_mem[0xfffd] = (uint8_t)(address >> 8);
            }
        }

        free(args);
        args = NULL;
    }

    memcpy(memory, g_mem, 65536);
}

static void TestInitVisual6502(void) {
    InitVisual6502("http://visual6502.org/JSSim/expert.html?graphics=f&loglevel=2&steps=50&a=0011&d=58&a=fffe&d=2000&a=0020&d=e840&r=0010&irq0=18&irq1=100&logmore=irq,D1x1&a=0014&d=78");
    TEST_EQ_UU(g_mem[0x11], 0x58);
    TEST_EQ_UU(g_mem[0xfffe], 0x20);
    TEST_EQ_UU(g_mem[0xffff], 0x00);
    TEST_EQ_UU(g_mem[0x0020], 0xe8);
    TEST_EQ_UU(g_mem[0x0021], 0x40);
    TEST_EQ_UU(g_mem[0x0014], 0x78);
    TEST_EQ_UU(g_irqs.size(), 2);
    TEST_EQ_II(g_irqs[0].level, 0);
    TEST_EQ_II(g_irqs[0].cycle, 18);
    TEST_EQ_II(g_irqs[1].level, 1);
    TEST_EQ_II(g_irqs[1].cycle, 100);
}

static PinState *FindPinStateByCycle(std::vector<PinState> *states, int cycle_) {
    for (size_t i = 0; i < states->size(); ++i) {
        if ((*states)[i].cycle == cycle_) {
            return &(*states)[i];
        }
    }

    return nullptr;
}

static void PrintCheckResult(uint32_t value, char type) {
    switch (type) {
    case 'b':
        printf("$%02x", value);
        break;

    case 'B':
        printf("%s", BOOL_STR(value));
        break;

    case 'w':
        printf("$%04x", value);
        break;

    default:
        ASSERT(false);
        break;
    }
}

static void Check(bool *discrepancy,
                  uint32_t simulator,
                  uint32_t perfect6502,
                  const char *what,
                  char type) {
    if (simulator != perfect6502) {
        *discrepancy = true;

        printf("***** discrepancy: %s: simulator=", what);
        PrintCheckResult(simulator, type);
        printf(" perfect6502=");
        PrintCheckResult(perfect6502, type);
        printf("\n");
    }
}

//static bool readD1x1(state_t *state) {
//    return !!isNodeHigh(state,D1x1);
//}

enum Node {
    Node_sync = 539,
    Node_irq = 103,
    Node_nmi = 1297,
    Node_clk0 = 1171,
    Node_clk1out = 1163,
    Node_clk2out = 421,
    Node_D1x1 = 827,
    Node_rw = 1156,
    Node_t2 = 971,
    Node_t3 = 1567,
    Node_t4 = 690,
    Node_t5 = 909,

    // The C and js code have conflicting values for clock1 and clock2. This
    // table copies the js code, in the interests of matching the visual6502
    // website output.
    Node_clock1 = 1536,
    Node_clock2 = 156,
};

static bool readSync(state_t *state) {
    return !!isNodeHigh(state, Node_sync);
}

static void setIRQ(state_t *state, bool level) {
    setNode(state, Node_irq, level);
}

static void setNMI(state_t *state, bool level) {
    setNode(state, Node_nmi, level);
}

//void
//chipStatus(void *state)
//{
//    BOOL clk = isNodeHigh(state, clk0);
//    uint16_t a = readAddressBus(state);
//    uint8_t d = readDataBus(state);
//    BOOL r_w = isNodeHigh(state, rw);
//
//    printf("halfcyc:%d phi0:%d AB:%04X D:%02X RnW:%d PC:%04X A:%02X X:%02X Y:%02X SP:%02X P:%02X IR:%02X",
//           cycle,
//           clk,
//           a,
//           d,
//           r_w,
//           readPC(state),
//           readA(state),
//           readX(state),
//           readY(state),
//           readSP(state),
//           readP(state),
//           readIR(state));
//
//    if (clk) {
//        if (r_w)
//            printf(" R$%04X=$%02X", a, memory[a]);
//        else
//            printf(" W$%04X=$%02X", a, d);
//    }
//    printf("\n");
//}

static void printStatus(state_t *state) {
    bool clk0 = !!isNodeHigh(state, Node_clk0);
    // bool clk1out=!!isNodeHigh(state,Node_clk1out);
    bool clk2out = !!isNodeHigh(state, Node_clk2out);
    uint16_t address = readAddressBus(state);
    uint8_t data = readDataBus(state);
    bool rw = !!isNodeHigh(state, Node_rw);
    uint16_t pc = readPC(state);
    uint8_t a = readA(state);
    uint8_t x = readX(state);
    uint8_t y = readY(state);
    uint8_t sp = readSP(state);
    uint8_t p = readP(state);
    uint8_t ir = readIR(state);
    bool nmi = !!isNodeHigh(state, Node_nmi);
    bool irq = !!isNodeHigh(state, Node_irq);
    bool d1x1 = !!isNodeHigh(state, Node_D1x1);

    printf("%d %d AB:%04x D:%02x RW:%d PC:%04x A:%02x X:%02x Y:%02x SP:%02x P:%02x IR:%02x %d%d%d",
           clk0, clk2out, address, data, rw, pc, a, x, y, sp, p, ir, irq, nmi, d1x1);

    if (clk0) {
        if (rw) {
            printf(" R$%04x=$%02x", address, memory[address]);
        } else {
            printf(" W$%04x=$%02x", address, data);
        }
    } else {
        //       W$xxxx=$xx
        printf("           ");
    }

    {
        static const Node tstates[] = {
            Node_clock1,
            Node_clock2,
            Node_t2,
            Node_t3,
            Node_t4,
            Node_t5,
            (Node)-1,
        };

        bool any = false;
        for (size_t i = 0; tstates[i] >= 0; ++i) {
            if (!isNodeHigh(state, (nodenum_t)tstates[i])) {
                if (any) {
                    printf("+");
                } else {
                    printf("  ");
                }

                printf("T%zu", i);
                any = true;
            }
        }
    }

    printf("\n");
}

static void TestVisual6502URL(const std::string &description, const std::string &url) {

    printf("************************************************************************\n");
    printf("\n");
    printf("%s\n", description.c_str());
    printf("\n");
    printf("%s\n", url.c_str());
    printf("\n");

    InitVisual6502(url);

    // Create perfect6502 state.
    state_t *perfect6502 = initAndResetChip();

    // Wait for the 6502 to reach the equivalent of the simulator's
    // initial state.
    while (readAddressBus(perfect6502) != 0xfffd) {
        step(perfect6502);
    }

    step(perfect6502);

    // Create M6502 state.
    auto s = new M6502;
    M6502_Init(s, &M6502_nmos6502_config);
    s->tfn = &M6502_NextInstruction;

    // Copy p6502 state.
    s->a = readA(perfect6502);
    s->x = readX(perfect6502);
    s->y = readY(perfect6502);
    M6502_SetP(s, readP(perfect6502));
    s->s.b.l = readSP(perfect6502);
    s->pc.b.l = g_mem[0xfffc];
    s->pc.b.h = g_mem[0xfffd];

    int first_discrepancy_cycle = -1;

    // Run.

    int c = 0;
    bool wasSync = false;

    // sync up...
    s->read = !!isNodeHigh(perfect6502, Node_rw);
    s->abus.w = readAddressBus(perfect6502);
    s->dbus = readDataBus(perfect6502);

    for (const PinState &state : g_irqs) {
        TEST_FALSE(state.seen);
    }

    for (const PinState &state : g_nmis) {
        TEST_FALSE(state.seen);
    }

    for (int i = 0; i < g_num_test_cycles; ++i) {
        bool discrepancy = false;
        //int printCycle=cycle/2;

        PinState *irq_state = FindPinStateByCycle(&g_irqs, c);
        PinState *nmi_state = FindPinStateByCycle(&g_nmis, c);

        if (irq_state) {
            setIRQ(perfect6502, irq_state->level);
            irq_state->seen = true;
        }

        if (nmi_state) {
            setNMI(perfect6502, nmi_state->level);
            nmi_state->seen = true;
        }

        // phi2 leading edge
        step(perfect6502);
        ASSERT(!isNodeHigh(perfect6502, Node_clk0));
        uint16_t phi1_addr = readAddressBus(perfect6502);
        printf("%-3d %-3d ", c / 2, c);
        printStatus(perfect6502);
        //chipStatus(perfect6502);
        ++c;

        // phi2 trailing edge
        step(perfect6502);
        ASSERT(isNodeHigh(perfect6502, Node_clk0));
        TEST_EQ_UU(phi1_addr, readAddressBus(perfect6502));
        printf("%-3d %-3d ", c / 2, c);
        printStatus(perfect6502);
        //chipStatus(perfect6502);
        ++c;

        if (wasSync) {
            Check(&discrepancy, s->a, readA(perfect6502), "A", 'b');
            Check(&discrepancy, s->x, readX(perfect6502), "X", 'b');
            Check(&discrepancy, s->y, readY(perfect6502), "Y", 'b');
            Check(&discrepancy, s->pc.w, readPC(perfect6502), "PC", 'b');
            Check(&discrepancy, s->s.b.l, readSP(perfect6502), "S", 'b');

            // Don't bother checking the unused bit - it's driven by D1x1,
            // which isn't modelled perfectly.
            //
            // If there's a discrepancy when P is pushed, the data bus contents
            // check will pick it up.
            uint8_t sim_p = (M6502_GetP(s).value) & ~0x10;
            uint8_t real_p = readP(perfect6502) & ~0x10;
            Check(&discrepancy, sim_p, real_p, "P", 'b');
        }

        if (c == 67) {
            int x = 0;
            (void)x;
        }

        const char *old_state = M6502_GetStateName(s, 1);

        if (irq_state) {
            M6502_SetDeviceIRQ(s, 1, irq_state->level == 0);
        }

        if (nmi_state) {
            M6502_SetDeviceNMI(s, 1, nmi_state->level == 0);
        }

        (*s->tfn)(s);

        if (s->read) {
            s->dbus = g_mem[s->abus.w];
        } else {
            g_mem[s->abus.w] = s->dbus;
        }

        {
            M6502P p = M6502_GetP(s);
            printf("            AB:%04X      RW:%d PC:%04X A:%02X X:%02X Y:%02X SP:%02X P:%02X       %d%d%d",
                   s->abus.w, s->read, s->pc.w, s->a, s->x, s->y, s->s.b.l, p.value,
                   s->device_irq_flags ? 0 : 1,
                   s->device_nmi_flags ? 0 : 1,
                   s->d1x1);
            printf(" %c$%04X=$%02X", s->read ? 'R' : 'W', s->abus.w, g_mem[s->abus.w]);
            printf("  called: %s\n", old_state);
            old_state = NULL;
            printf("                                                                                     ");
            printf(" next: %s\n", M6502_GetStateName(s, 1));
        }

        wasSync = readSync(perfect6502);

        Check(&discrepancy, s->abus.w, readAddressBus(perfect6502), "address bus contents", 'w');
        Check(&discrepancy, !!s->read, !!readRW(perfect6502), "rw status", 'B');
        Check(&discrepancy, s->dbus, readDataBus(perfect6502), "data bus contents", 'b');

        if (discrepancy) {
            if (first_discrepancy_cycle < 0) {
                first_discrepancy_cycle = c - 1;
            }
        } else {
            printf("\n");
        }
    }

    printf("\n");
    printf("%s\n", description.c_str());
    printf("\n");
    printf("************************************************************************\n");

    TEST_EQ_II(first_discrepancy_cycle, -1);

    for (const PinState &state : g_irqs) {
        TEST_TRUE(state.seen);
    }

    for (const PinState &state : g_nmis) {
        TEST_TRUE(state.seen);
    }

    destroyChip(perfect6502);

    InitVisual6502("");
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct TestCase {
    std::string description;
    std::string url;

    // if any test has its prefer flag set, run only tests with the
    // prefer flag set.
    bool prefer = false;

    //
    int num_cycles = 120;
};
typedef struct TestCase TestCase;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static int g_tc_mem[65536]; //-1 if not set
static int g_tc_num_cycles = 120;
static uint16_t g_tc_next_addr = 0;
static std::vector<int> g_tc_irqs, g_tc_nmis;
static std::vector<TestCase> g_test_cases;

static void SetTCVectors(uint16_t nmiv, uint16_t rstv, uint16_t irqv) {
    g_tc_mem[0xfffa] = (nmiv >> 0) & 0xff;
    g_tc_mem[0xfffb] = (nmiv >> 8) & 0xff;
    g_tc_mem[0xfffc] = (rstv >> 0) & 0xff;
    g_tc_mem[0xfffd] = (rstv >> 8) & 0xff;
    g_tc_mem[0xfffe] = (irqv >> 0) & 0xff;
    g_tc_mem[0xffff] = (irqv >> 8) & 0xff;
}

static void SetTCMem(int addr, const char *bytes) {
    uint16_t a;
    if (addr < 0) {
        a = g_tc_next_addr;
    } else {
        a = (uint16_t)addr;
    }

    const char *c = bytes;
    while (*c != 0) {
        if (isspace(*c)) {
            ++c;
            continue;
        }

        char hc = *c++;
        TEST_TRUE(isxdigit(hc));

        char lc = *c++;
        TEST_TRUE(isxdigit(lc));

        uint8_t value = unhex(hc) << 4 | unhex(lc);
        g_tc_mem[a] = value;
        ++a;
    }

    g_tc_next_addr = a;
}

static void ResetTC(void) {
    g_tc_irqs.clear();
    g_tc_nmis.clear();

    for (int i = 0; i < 65536; ++i) {
        g_tc_mem[i] = -1;
    }

    // Overwrite the Visual6502 standard bits
    for (size_t i = 0; i < sizeof DEFAULT_MEMORY; ++i) {
        if (DEFAULT_MEMORY[i] != 0) {
            g_tc_mem[i] = 0;
        }
    }

    // IRQ handler
    g_tc_mem[0x40] = 0x40;

    // NMI handler
    g_tc_mem[0x50] = 0x40;

    g_tc_num_cycles = 120;

    // Add the preamble
    //
    // After the preamble: A=0, X=0, Y=0, S=255, P=nv__dIZc
    SetTCMem(0x0200, "a2ff 9a e8 8a a8 18 d8 b8 eaeaeaeaeaeaea");
    //TEST_EQ_II(g_tc_mem[15],0xea);
    //TEST_EQ_II(g_tc_mem[16],-1);

    SetTCVectors(0x0050, 0x0200, 0x0040);
}

static std::string GetTCURL(const std::string &base) {
    std::string url = base + "?graphics=f&loglevel=2&logmore=irq,nmi,D1x1,clk0&steps=" + std::to_string(g_tc_num_cycles);

    int a = -1;
    for (int b = 0; b < 65537; ++b) {
        if (b < 65536 && g_tc_mem[b] >= 0) {
            if (a < 0) {
                a = b;
            }
        } else {
            if (a >= 0) {
                url += strprintf("&a=%04x&d=", a);
                for (int j = a; j < b; ++j) {
                    ASSERT(g_tc_mem[j] >= 0 && g_tc_mem[j] <= 255);
                    url += strprintf("%02x", g_tc_mem[j]);
                }

                a = -1;
            }
        }
    }

    for (size_t i = 0; i < g_tc_irqs.size(); ++i) {
        url += strprintf("&irq%zu=%d", i % 2, g_tc_irqs[i]);
    }

    for (size_t i = 0; i < g_tc_nmis.size(); ++i) {
        url += strprintf("&nmi%zu=%d", i % 2, g_tc_nmis[i]);
    }

    return url;
}

static void AddTC(const char *fmt, ...) {
    TestCase tc;

    va_list v;
    va_start(v, fmt);
    tc.description = strprintfv(fmt, v);
    va_end(v);

    // Not very clever.
    tc.url = GetTCURL("file:///" VISUAL6502_PATH "/expert.html");
    g_test_cases.push_back(tc);
}

// Mark the last TC as preferred.
static void PreferTC(void) {
    TEST_FALSE(g_test_cases.empty());
    g_test_cases.back().prefer = true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static void AddTestCases(void) {
    ResetTC();
    {
        SetTCMem(0x10, "e6ff 90fc"); //.L:inc $ff:bcc L

        AddTC("Preamble + simple loop");
    }

    ResetTC();
    {
        SetTCMem(0x10, "58 e6ff 78 eaea eaea eaea eaea eaea eaea"); //cli:inc $ff:sei:nop...

        for (int i = 0; i < 3; ++i) {
            g_tc_irqs = {74 - i * 2};

            AddTC("IRQ at end-%d of INC, then SEI", i);
        }
    }

    ResetTC();
    {
        // 58=cli, 78=sei
        for (int i = 0; i < 50; ++i)
            g_tc_mem[0x10 + i] = i % 2 == 0 ? 0x58 : 0x78;

        g_tc_irqs = {0};

        AddTC("Stream of CLI/SEI/CLI/SEI with IRQ held low the whole time");
    }

    ResetTC();
    {
        SetTCMem(0x10, "4c8000");

        // d0=bne (not taken); f0=beq (taken); 10=bpl (taken)
        SetTCMem(0x80, "a900 d000 a900 f000 a900 107f");
        SetTCMem(0x100, "eaeaeaeaeaeaeaeaeaeaeaeaeaeaeaea 4c0001");

        AddTC("Branch instruction timings");
    }

    ResetTC();
    {
        SetTCMem(0x10, "5890fe");

        for (int i = 0; i < 10; ++i) {
            int c = 64 + i * 2;
            g_tc_irqs = {c};

            //if(cycle!=67&&cycle!=73&&cycle!=79)
            AddTC("Branch taken to same page - IRQ on cycle %d", g_tc_irqs[0]);
        }
    }

    ResetTC();
    {
        SetTCMem(0x10, "ca 88 0a"); //dex:dey:asl A

        AddTC("Single-byte instructions timing");
    }

    ResetTC();
    {
        // a2=ldx #; a0=ldy #
        SetTCMem(0x10, "a280a080");    //ldx #$80; ldy #$80
        SetTCMem(-1, "a900");          //lda #0
        SetTCMem(-1, "a500 b500");     // lda $00; lda $00,X
        SetTCMem(-1, "ad0010");        //lda $1000
        SetTCMem(-1, "bd0010 bd9010"); //lda $1000,X; lda $1090,X
        SetTCMem(-1, "b90010 b99010"); //lda $1000,Y; lda $1090,Y
        SetTCMem(-1, "a100");          //lda ($80,X)
        SetTCMem(-1, "b180 b182");     //lda ($80),Y; lda ($82),Y

        SetTCMem(0x80, "0030 9030");

        g_tc_num_cycles = 200;

        AddTC("Read instructions timing");
    }

    ResetTC();
    {
        // a2=ldx #; a0=ldy #
        SetTCMem(0x10, "a280a080");    //ldx #$80; ldy #$80
        SetTCMem(-1, "8500 9500");     // sta $00; sta $00,X
        SetTCMem(-1, "8d0010");        //sta $1000
        SetTCMem(-1, "9d0010 9d9010"); //sta $1000,X; sta $1090,X
        SetTCMem(-1, "990010 999010"); //sta $1000,Y; sta $1090,Y
        SetTCMem(-1, "8100");          //sta ($80,X)
        SetTCMem(-1, "9180 9182");     //sta ($80),Y; sta ($82),Y

        SetTCMem(0x80, "0030 9030");

        g_tc_num_cycles = 200;

        AddTC("Store instructions timing");
    }

    ResetTC();
    {
        SetTCMem(-1, "a200a000"); //ldx #0 ldy #0
        SetTCMem(-1, "4680 4e0080 5680 5e0080");
        SetTCMem(-1, "0380 1380"); //slo* ($80,x); slo* ($80),y

        g_tc_num_cycles = 200;

        AddTC("Read-modify-write instructions");
    }

    ResetTC();
    {
        SetTCMem(-1, "48 68 209000"); //pha pla jsr $0090
        SetTCMem(-1, "a900 48 a9b0 48 4c a000");

        SetTCMem(0x90, "60");
        SetTCMem(0xa0, "08 40"); //php rti
        SetTCMem(0xb0, "6c 0010");
        SetTCMem(0x1000, "c000");
        SetTCMem(0xc0, "ea00");

        g_tc_num_cycles = 200;

        AddTC("Miscellaneous instructions timing");
    }

    ResetTC();
    {
        SetTCMem(-1, "90fe");

        for (int i = 0; i < 10; ++i) {
            int c = 60 + i * 2;

            g_tc_irqs = {c};
            g_tc_nmis = {c};

            AddTC("Simultaneous IRQ and NMI on cycle %d", c);

            g_tc_irqs.push_back(c + 2);
            g_tc_nmis.push_back(c + 2);

            AddTC("Simultaneous 2-cycle IRQ+NMI blip on cycle %d", c);
        }
    }

    {
        int num_cycles = 390;

        // the preamble is 65 cycles long.
        for (int i = 64; i < num_cycles; i += 2) {
            ResetTC();

            // Read

            SetTCMem(-1, "58");             //cli
            SetTCMem(-1, "a9 00");          //lda #0
            SetTCMem(-1, "a5 00");          //lda $00
            SetTCMem(-1, "b5 00");          //lda $00,x
            SetTCMem(-1, "b1 00");          //ldx $00,y
            SetTCMem(-1, "ad 00 00");       //lda $0000,x
            SetTCMem(-1, "a2 00 bd 80 03"); //ldx #$00:lda $3080,x
            SetTCMem(-1, "a2 ff bd 80 03"); //ldx #$ff:lda $3080,x
            SetTCMem(-1, "a0 00 b9 80 03"); //ldy #$00:lda $3080,y
            SetTCMem(-1, "a0 ff b9 80 03"); //ldy #$ff:lda $3080,y
            SetTCMem(-1, "a0 00 b1 00");    //ldy #$00:lda ($00),y
            SetTCMem(-1, "a0 ff b1 00");    //ldy #$ff:lda ($00),y
            SetTCMem(-1, "a1 00");          //lda ($00,x)

            // RMW
            SetTCMem(-1, "a2 00");    //ldx #$00
            SetTCMem(-1, "06 80");    //asl $80
            SetTCMem(-1, "16 80");    //asl $80,x
            SetTCMem(-1, "0e 80 30"); //asl $3080
            SetTCMem(-1, "1e 80 30"); //asl $3080,x

            // Write
            SetTCMem(-1, "a2 00");    //ldx #$00
            SetTCMem(-1, "a0 00");    //ldy #$00
            SetTCMem(-1, "85 80");    //sta $80
            SetTCMem(-1, "95 80");    //sta $80,x
            SetTCMem(-1, "96 80");    //stx $80,y
            SetTCMem(-1, "8d 80 30"); //sta $3080
            SetTCMem(-1, "9d 80 30"); //sta $3080,x
            SetTCMem(-1, "99 80 30"); //sta $3080,y
            SetTCMem(-1, "91 00");    //sta ($00),y
            SetTCMem(-1, "81 00");    //sta ($00,x)

            // Other
            SetTCMem(-1, "48");       //pha
            SetTCMem(-1, "68");       //pla
            SetTCMem(-1, "20 02 00"); //jsr $0002 - goes to a JMP then an RTS

            SetTCMem(-1, "18 90 fe"); //clc:.L:bcc L

            SetTCMem(0x00, "80 03"); //$3080

            SetTCMem(0x02, "4c 05 00 60"); //RTS

            // blip.
            g_tc_irqs = {i, i + 2};

            // add a few more cycles, to accommodate the IRQ routine.
            g_tc_num_cycles = num_cycles + 20;

            AddTC("IRQ at +%d during instruction mix", i);

            if (i == 65) {
                //PreferTC();
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//{
//    "IRQ in 3rd cycle of branch taken to same page is delayed by 1 instruction",
//    "http://visual6502.org/JSSim/expert.html?graphics=f&loglevel=2&steps=100&a=0010&d=581890fe&a=fffe&d=3000&a=0030&d=40&r=0010&irq0=15&irq1=31&logmore=irq,D1x1",
//},
//{
//    "NMI",
//    "http://visual6502.org/JSSim/expert.html?graphics=f&a=fffa&d=2000&a=20&d=40&a=0&d=eaeaeaeaeaeaeaeaeaea4c0000&steps=100&nmi0=13&loglevel=3&logmore=irq,nmi,D1x1",
//},
//{
//    "Simultaneous IRQ and NMI",
//    "http://visual6502.org/JSSim/expert.html?graphics=f&a=fffe&d=3000&a=30&d=40&a=fffa&d=2000&a=20&d=40&a=0&d=a2ff9a58eaeaeaeaeaeaeaeaeaea4c0002&steps=150&nmi0=21&irq0=21&loglevel=3&logmore=irq,nmi,D1x1",
//},
//{
//    "IRQ then late NMI when pushing PCL (NMI gets priority)",
//    "http://visual6502.org/JSSim/expert.html?graphics=f&a=fffe&d=3000&a=30&d=40&a=fffa&d=2000&a=20&d=40&a=0&d=58eaeaeaeaeaeaeaeaeaea4c0000&steps=100&nmi0=31&irq0=21&loglevel=3&logmore=irq,nmi,D1x1",
//},
//{
//    "IRQ then late NMI when pushing P (IRQ gets priority)",
//    "http://visual6502.org/JSSim/expert.html?graphics=f&a=fffe&d=3000&a=30&d=40&a=fffa&d=2000&a=20&d=40&a=0&d=58eaeaeaeaeaeaeaeaeaea4c0000&steps=100&nmi0=33&irq0=21&loglevel=3&logmore=irq,nmi,D1x1",
//},

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static void WriteTestHTML(void) {
    FILE *f = fopen("visual6502tests.html", "wt");
    fprintf(f, "<html><head><title>Visual 6502 test cases</title></head><body><h1>Visual 6502 Test Cases</h1><ul>");
    for (size_t i = 0; i < g_test_cases.size(); ++i) {
        fprintf(f, "<li><a href=\"%s\">", g_test_cases[i].url.c_str());
        for (char c : g_test_cases[i].description) {
            switch (c) {
            case '\'':
                fprintf(f, "&apos;");
                break;

            case '&':
                fprintf(f, "&amp;");
                break;

            case '<':
                fprintf(f, "&lt;");
                break;

            case '>':
                fprintf(f, "&gt;");
                break;

            default:
                fprintf(f, "%c", c);
                break;
            }
        }
        fprintf(f, "</a>");
    }
    fprintf(f, "</body></html>");
    fclose(f);
    f = NULL;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int main(void) {
    (void)&PreferTC;

    TestInitVisual6502();

    AddTestCases();

    WriteTestHTML();

    for (size_t i = 0; i < g_test_cases.size(); ++i)
        LOGF(DEBUG, "%s\n", g_test_cases[i].url.c_str());

    int check_prefer = 0;
    for (size_t i = 0; i < g_test_cases.size(); ++i) {
        const TestCase *tc = &g_test_cases[i];

        if (tc->prefer) {
            check_prefer = 1;
            break;
        }
    }

    for (size_t i = 0; i < g_test_cases.size(); ++i) {
        const TestCase *tc = &g_test_cases[i];

        if (check_prefer) {
            if (!tc->prefer) {
                continue;
            }
        }

        TestVisual6502URL(tc->description, tc->url);
    }

    return 0;
}
