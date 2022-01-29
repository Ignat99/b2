#if EINTERNAL
#define EPREFIX static
#else
#define EPREFIX
#endif

#define EEND_EXTRA()

#define EBEGIN()                                                          \
    EPREFIX UNUSED const char *CONCAT3(Get, ENAME, EnumName)(int value) { \
        switch (value) {                                                  \
        default:                                                          \
            return "?" STRINGIZE(ENAME) "?";

#define EBEGIN_DERIVED(BASE_NAME)                                      \
    EPREFIX const char *CONCAT3(Get, ENAME, EnumName)(int value) {     \
        switch (value) {                                               \
        default: {                                                     \
            const char *result = CONCAT3(Get, BASE_NAME, Name)(value); \
            if (result[0] == '?') {                                    \
                result = "?" STRINGIZE(ENAME) "?";                     \
            }                                                          \
                                                                       \
            return result;                                             \
        }

#define EN_INTERNAL(NAME, STR) \
    case (NAME):               \
        return (STR);

#define EN(NAME) EN_INTERNAL(NAME, #NAME)

#define EPN(NAME) EN_INTERNAL(CONCAT3(ENAME, _, NAME), STRINGIZE(NAME))

#define ENV(NAME, VALUE) EN(NAME)

#define EPNV(NAME, VALUE) EPN(NAME)

#define EQN(NAME)
#define EQNV(NAME, VALUE)
#define EQPN(NAME)
#define EQPNV(NAME, VALUE)

#define EEND() \
    }          \
    }          \
    EEND_EXTRA()

/* This gets a bit freakish. */
#define NBEGIN(NAME)                                                \
    EPREFIX const char *CONCAT3(Get, NAME, EnumName)(ENAME value) { \
        switch (value) {                                            \
        default:                                                    \
            return "?" STRINGIZE(NAME) "?";

#define NN(NAME) \
    case (NAME): \
        return #NAME;
#define NNS(NAME, STR) \
    case (NAME):       \
        return (STR);

#define NEND() \
    }          \
    }
