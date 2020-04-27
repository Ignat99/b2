#include <shared/system.h>
#include "TraceUI.h"
#include <beeb/Trace.h>
#include "dear_imgui.h"
#include "BeebThread.h"
#include "JobQueue.h"
#include "BeebWindow.h"
#include "BeebWindows.h"
#include <beeb/BBCMicro.h>
#include <beeb/6502.h>
#include <shared/debug.h>
#include <string.h>
#include "native_ui.h"
#include <inttypes.h>
#include "keys.h"
#include <math.h>
#include <atomic>
#include "SettingsUI.h"

#include <shared/enum_def.h>
#include "TraceUI.inl"
#include <shared/enum_end.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static const std::string RECENT_PATHS_TRACES="traces";

// It's a bit ugly having a single set of default settings, but compared to the
// old behaviour (per-instance settings, defaults overwritten when dialog
// closed) this arrangement makes more sense when using the docking UI. Since
// with the UI docked, it's much more rarely going to be closed.
static TraceUISettings g_default_settings;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

TraceUISettings GetDefaultTraceUISettings() {
    return g_default_settings;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void SetDefaultTraceUISettings(const TraceUISettings &settings) {
    g_default_settings=settings;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#if BBCMICRO_TRACE

class TraceUI:
    public SettingsUI
{
public:
#if BBCMICRO_TRACE
    TraceUI(BeebWindow *beeb_window);

    void DoImGui() override;
    bool OnClose() override;
protected:
private:
    class SaveTraceJob;
    struct Key;

    BeebWindow *m_beeb_window=nullptr;

    std::shared_ptr<SaveTraceJob> m_save_trace_job;
    std::vector<uint8_t> m_keys;

    char m_stop_num_cycles_str[100]={};
    char m_start_instruction_address_str[100]={};
    char m_start_write_address_str[100]={};
    char m_stop_write_address_str[100]={};

    bool m_config_changed=false;

    int GetKeyIndex(uint8_t beeb_key) const;
    void ResetTextBoxes();
    void DoAddressGui(uint16_t *addr,char *str,size_t str_size);

    static bool GetBeebKeyName(void *data,int idx,const char **out_text);
#endif
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class TraceUI::SaveTraceJob:
public JobQueue::Job
{
public:
    explicit SaveTraceJob(std::shared_ptr<Trace> trace,
                          std::string file_name,
                          std::shared_ptr<MessageList> message_list,
                          TraceCyclesOutput cycles_output):
    m_trace(std::move(trace)),
    m_file_name(std::move(file_name)),
    m_cycles_output(cycles_output),
    m_msgs(message_list)
    {
    }

    void ThreadExecute() {
        FILE *f=fopen(m_file_name.c_str(),"wt");
        if(!f) {
            int err=errno;
            m_msgs.e.f(
                       "failed to open trace output file: %s\n",
                       m_file_name.c_str());
            m_msgs.i.f(
                       "(fopen failed: %s)\n",
                       strerror(err));
            return;
        }

        setvbuf(f,NULL,_IOFBF,262144);

        uint64_t start_ticks=GetCurrentTickCount();

        if(SaveTrace(m_trace,
                     m_cycles_output,
                     &SaveData,f,
                     &WasCanceledThunk,this,
                     &m_progress))
        {
            m_msgs.i.f(
                       "trace output file saved: %s\n",
                       m_file_name.c_str());
        } else {
            m_msgs.w.f(
                       "trace output file canceled: %s\n",
                       m_file_name.c_str());
        }

        double secs=GetSecondsFromTicks(GetCurrentTickCount()-start_ticks);
        if(secs!=0.) {
            double mbytes=m_progress.num_bytes_written/1024./1024.;
            m_msgs.i.f("(%.2f MBytes/sec)\n",mbytes/secs);
        }

        fclose(f);
        f=NULL;
    }

    void GetProgress(uint64_t *num,uint64_t *total) {
        *num=m_progress.num_events_handled;
        *total=m_progress.num_events;
    }
protected:
private:
    std::shared_ptr<Trace> m_trace;
    std::string m_file_name;
    TraceCyclesOutput m_cycles_output=TraceCyclesOutput_Relative;
    Messages m_msgs;            // this is quite a big object
    SaveTraceProgress m_progress;

    static bool SaveData(const void *data,size_t num_bytes,void *context) {
        size_t num_bytes_written=fwrite(data,1,num_bytes,(FILE *)context);
        return num_bytes_written==num_bytes;
    }

    static bool WasCanceledThunk(void *context) {
        auto this_=(TraceUI::SaveTraceJob *)context;

        return this_->WasCanceled();
    }
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

TraceUI::TraceUI(BeebWindow *beeb_window):
    m_beeb_window(beeb_window)
{
    this->ResetTextBoxes();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static void DoTraceStatsImGui(const volatile TraceStats *stats) {
    ImGui::Separator();

    ImGui::Columns(2);

    ImGui::TextUnformatted("Events");
    ImGui::NextColumn();
    ImGui::Text("%" PRIthou "zu",stats->num_events);
    ImGui::NextColumn();

    ImGui::TextUnformatted("Bytes Used");
    ImGui::NextColumn();
    ImGui::Text("%" PRIthou ".3f MB",stats->num_used_bytes/1024./1024.);
    ImGui::NextColumn();

    ImGui::TextUnformatted("Bytes Allocated");
    ImGui::NextColumn();
    ImGui::Text("%" PRIthou ".3f MB",stats->num_allocated_bytes/1024./1024.);
    ImGui::NextColumn();

    ImGui::Columns(1);

    ImGui::Separator();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static void DoTraceFlag(uint32_t *flags_seen,
                        uint32_t flag,
                        const char *text,
                        bool last=false)
{
    *flags_seen|=flag;

    if(!text) {
        text=GetBBCMicroTraceFlagEnumName((int)flag);
    }

    ImGuiIDPusher id_pusher(flag);

    if(ImGui::CheckboxFlags(text,&g_default_settings.flags,flag)) {
        //printf("toggle %s\n",text);
    }

    if(!last) {
        ImGui::SameLine();
    }
}

void TraceUI::DoImGui() {
    std::shared_ptr<BeebThread> beeb_thread=m_beeb_window->GetBeebThread();

    if(m_save_trace_job) {
        uint64_t n,t;
        m_save_trace_job->GetProgress(&n,&t);

        ImGui::Columns(2);

        ImGui::TextUnformatted("Saving");

        ImGui::NextColumn();

        ImGui::Text("%" PRIthou PRIu64 "/%" PRIthou PRIu64,n,t);

        ImGui::NextColumn();

        ImGui::Columns(1);

        float fraction=0.f;
        if(t>0) {
            fraction=(float)(n/(double)t);
        }

        ImGui::ProgressBar(fraction);

        if(m_save_trace_job->IsFinished()) {
            m_save_trace_job=nullptr;
        }

        if(ImGui::Button("Cancel")) {
            m_save_trace_job->Cancel();
            m_save_trace_job=nullptr;
        }

        return;
    }

    const volatile TraceStats *running_stats=beeb_thread->GetTraceStats();

    if(!running_stats) {
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("Start condition");
        {
            ImGuiIDPusher pusher("start conditions");

            ImGuiRadioButton(&g_default_settings.start,TraceUIStartCondition_Now,"Immediate");
            ImGuiRadioButton(&g_default_settings.start,TraceUIStartCondition_Return,"Return");
            ImGuiRadioButton(&g_default_settings.start,
                             TraceUIStartCondition_Instruction,
                             "Execute $%04x",
                             g_default_settings.start_instruction_address);
            if(g_default_settings.start==TraceUIStartCondition_Instruction) {
                this->DoAddressGui(&g_default_settings.start_instruction_address,
                                   m_start_instruction_address_str,
                                   sizeof m_start_instruction_address_str);
            }
            ImGuiRadioButton(&g_default_settings.start,
                             TraceUIStartCondition_WriteAddress,
                             "Write $%04x",
                             g_default_settings.start_write_address);
            if(g_default_settings.start==TraceUIStartCondition_WriteAddress) {
                this->DoAddressGui(&g_default_settings.start_write_address,
                                   m_start_write_address_str,
                                   sizeof m_start_write_address_str);
            }
        }
        ImGui::Spacing();

        ImGui::TextUnformatted("Stop condition");
        {
            ImGuiIDPusher pusher("stop conditions");

            ImGuiRadioButton(&g_default_settings.stop,TraceUIStopCondition_ByRequest,"By request");
            ImGuiRadioButton(&g_default_settings.stop,TraceUIStopCondition_OSWORD0,"OSWORD 0");
            ImGuiRadioButton(&g_default_settings.stop,TraceUIStopCondition_NumCycles,"Cycle count");
            if(g_default_settings.stop==TraceUIStopCondition_NumCycles) {
                if(ImGui::InputText("Cycles",
                                    m_stop_num_cycles_str,
                                    sizeof m_stop_num_cycles_str,
                                    ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    if(!GetUInt64FromString(&g_default_settings.stop_num_cycles,
                                            m_stop_num_cycles_str))
                    {
                        this->ResetTextBoxes();
                    }
                }
            }
            ImGuiRadioButton(&g_default_settings.stop,
                             TraceUIStopCondition_WriteAddress,
                             "Write $%04x",
                             g_default_settings.stop_write_address);
            if(g_default_settings.stop==TraceUIStopCondition_WriteAddress) {
                this->DoAddressGui(&g_default_settings.stop_write_address,
                                   m_stop_write_address_str,
                                   sizeof m_stop_write_address_str);
            }
        }

        ImGui::Spacing();

        ImGui::TextUnformatted("Other traces");

        uint32_t flags_seen=0;

        DoTraceFlag(&flags_seen,BBCMicroTraceFlag_6845VSync,"6845 VSync");
        DoTraceFlag(&flags_seen,BBCMicroTraceFlag_6845Scanlines,"Scanlines");
        DoTraceFlag(&flags_seen,BBCMicroTraceFlag_6845ScanlinesSeparators,"Separators",true);

        DoTraceFlag(&flags_seen,BBCMicroTraceFlag_SystemVIA,"System VIA");
        DoTraceFlag(&flags_seen,BBCMicroTraceFlag_SystemVIAExtra,"Extra",true);

        DoTraceFlag(&flags_seen,BBCMicroTraceFlag_UserVIA,"User VIA");
        DoTraceFlag(&flags_seen,BBCMicroTraceFlag_UserVIAExtra,"Extra",true);

        for(uint32_t i=1;i!=0;i<<=1) {
            if(!(flags_seen&i)) {
                const char *name=GetBBCMicroTraceFlagEnumName((int)i);
                if(name[0]=='?') {
                    continue;
                }

                ImGui::CheckboxFlags(name,&g_default_settings.flags,i);
            }
        }

        ImGui::Spacing();

        ImGui::TextUnformatted("Other settings");
        ImGui::Checkbox("Unlimited recording", &g_default_settings.unlimited);

        if(ImGui::Button("Start")) {
            TraceConditions c;

            switch(g_default_settings.start) {
                case TraceUIStartCondition_Now:
                    c.start=BeebThreadStartTraceCondition_Immediate;
                    break;

                case TraceUIStartCondition_Return:
                    c.start=BeebThreadStartTraceCondition_NextKeypress;
                    c.start_key=BeebKey_Return;
                    break;

                case TraceUIStartCondition_Instruction:
                    c.start=BeebThreadStartTraceCondition_Instruction;
                    c.start_address=g_default_settings.start_instruction_address;
                    break;

                case TraceUIStartCondition_WriteAddress:
                    c.start=BeebThreadStartTraceCondition_WriteAddress;
                    c.start_address=g_default_settings.start_write_address;
                    break;
            }

            switch(g_default_settings.stop) {
                case TraceUIStopCondition_ByRequest:
                    c.stop=BeebThreadStopTraceCondition_ByRequest;
                    break;

                case TraceUIStopCondition_OSWORD0:
                    c.stop=BeebThreadStopTraceCondition_OSWORD0;
                    break;

                case TraceUIStopCondition_NumCycles:
                    c.stop=BeebThreadStopTraceCondition_NumCycles;
                    c.stop_num_cycles=g_default_settings.stop_num_cycles;
                    break;

                case TraceUIStopCondition_WriteAddress:
                    c.stop=BeebThreadStopTraceCondition_WriteAddress;
                    c.stop_address=g_default_settings.stop_write_address;
                    break;
            }

            c.trace_flags=g_default_settings.flags;

            size_t max_num_bytes;
            if(g_default_settings.unlimited) {
                max_num_bytes=SIZE_MAX;
            } else {
                // 64MBytes = ~12m cycles, or ~6 sec, with all the flags on,
                // recorded sitting at the BASIC prompt, producing a ~270MByte
                // text file.
                //
                // 256MBytes, then, ought to be ~25 seconds, and a ~1GByte
                // text file. This ought to be enough to be getting on with,
                // and the buffer size is not excessive even for 32-bit systems.
                max_num_bytes=256*1024*1024;
            }

            beeb_thread->Send(std::make_shared<BeebThread::StartTraceMessage>(c,max_num_bytes));
        }

        std::shared_ptr<Trace> last_trace=beeb_thread->GetLastTrace();

        if(!!last_trace) {
            TraceStats stats;
            last_trace->GetStats(&stats);

            DoTraceStatsImGui(&stats);

            ImGui::TextUnformatted("Cycles output:");
            ImGuiRadioButton(&g_default_settings.cycles_output,TraceCyclesOutput_Absolute,"Absolute");
            ImGuiRadioButton(&g_default_settings.cycles_output,TraceCyclesOutput_Relative,"Relative");
            ImGuiRadioButton(&g_default_settings.cycles_output,TraceCyclesOutput_None,"None");

            if(ImGui::Button("Save...")) {
                SaveFileDialog fd(RECENT_PATHS_TRACES);

                fd.AddFilter("Text files",{".txt"});
                fd.AddAllFilesFilter();

                std::string path;
                if(fd.Open(&path)) {
                    fd.AddLastPathToRecentPaths();
                    m_save_trace_job=std::make_shared<SaveTraceJob>(last_trace,
                                                                    path,
                                                                    m_beeb_window->GetMessageList(),
                                                                    g_default_settings.cycles_output);
                    BeebWindows::AddJob(m_save_trace_job);
                }
            }

            ImGui::SameLine();

            if(ImGui::Button("Clear")) {
                beeb_thread->ClearLastTrace();
            }
        }
    } else {
        if(ImGui::Button("Stop")) {
            beeb_thread->Send(std::make_shared<BeebThread::StopTraceMessage>());
        }

        DoTraceStatsImGui(running_stats);
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool TraceUI::OnClose() {
    return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int TraceUI::GetKeyIndex(uint8_t beeb_key) const {
    for(size_t i=0;i<m_keys.size();++i) {
        if(m_keys[i]==beeb_key) {
            return (int)i;
        }
    }

    return -1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TraceUI::ResetTextBoxes() {
    snprintf(m_start_instruction_address_str,
             sizeof m_start_instruction_address_str,
             "%x",
             g_default_settings.start_instruction_address);

    snprintf(m_start_write_address_str,
             sizeof m_start_write_address_str,
             "%x",
             g_default_settings.start_write_address);

    snprintf(m_stop_num_cycles_str,
             sizeof m_stop_num_cycles_str,
             "%" PRIu64,
             g_default_settings.stop_num_cycles);

    snprintf(m_stop_write_address_str,
             sizeof m_stop_write_address_str,
             "%x",
             g_default_settings.stop_write_address);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void TraceUI::DoAddressGui(uint16_t *addr,char *str,size_t str_size) {
    static constexpr ImGuiInputTextFlags FLAGS=ImGuiInputTextFlags_CharsHexadecimal|ImGuiInputTextFlags_EnterReturnsTrue;
    if(ImGui::InputText("Address (hex)",str,str_size,FLAGS)) {
        if(!GetUInt16FromString(addr,str,16)) {
            this->ResetTextBoxes();
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool TraceUI::GetBeebKeyName(void *data,int idx,const char **out_text) {
    auto this_=(TraceUI *)data;

    if(idx>=0&&(size_t)idx<this_->m_keys.size()) {
        *out_text=::GetBeebKeyName((BeebKey)this_->m_keys[(size_t)idx]);
        ASSERT(*out_text);
        return true;
    } else {
        return false;
    }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

std::unique_ptr<SettingsUI> CreateTraceUI(BeebWindow *beeb_window) {
    return std::make_unique<TraceUI>(beeb_window);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#endif
