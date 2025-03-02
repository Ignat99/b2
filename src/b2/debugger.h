#ifndef HEADER_73C614F48AE84A78936CD1D7AE2D1876 // -*- mode:c++ -*-
#define HEADER_73C614F48AE84A78936CD1D7AE2D1876

#include "conf.h"

#if BBCMICRO_DEBUGGER

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// Now a misnomer...
#include "SettingsUI.h"
#include <memory>
#include <beeb/type.h>

class BeebWindow;

std::unique_ptr<SettingsUI> Create6502DebugWindow(BeebWindow *beeb_window);
std::unique_ptr<SettingsUI> CreateHostMemoryDebugWindow(BeebWindow *beeb_window);
std::unique_ptr<SettingsUI> CreateParasiteMemoryDebugWindow(BeebWindow *beeb_window);
std::unique_ptr<SettingsUI> CreateExtMemoryDebugWindow(BeebWindow *beeb_window);
std::unique_ptr<SettingsUI> CreateHostDisassemblyDebugWindow(BeebWindow *beeb_window, bool initial_track_pc);
std::unique_ptr<SettingsUI> CreateParasiteDisassemblyDebugWindow(BeebWindow *beeb_window, bool initial_track_pc);
std::unique_ptr<SettingsUI> CreateCRTCDebugWindow(BeebWindow *beeb_window);
std::unique_ptr<SettingsUI> CreateVideoULADebugWindow(BeebWindow *beeb_window);
std::unique_ptr<SettingsUI> CreateSystemVIADebugWindow(BeebWindow *beeb_window);
std::unique_ptr<SettingsUI> CreateUserVIADebugWindow(BeebWindow *beeb_window);
std::unique_ptr<SettingsUI> CreateNVRAMDebugWindow(BeebWindow *beeb_window);
std::unique_ptr<SettingsUI> CreateSN76489DebugWindow(BeebWindow *beeb_window);
std::unique_ptr<SettingsUI> CreatePagingDebugWindow(BeebWindow *beeb_window);
std::unique_ptr<SettingsUI> CreateBreakpointsDebugWindow(BeebWindow *beeb_window);
#if VIDEO_TRACK_METADATA
std::unique_ptr<SettingsUI> CreatePixelMetadataDebugWindow(BeebWindow *beeb_window);
#endif
std::unique_ptr<SettingsUI> CreateHostStackDebugWindow(BeebWindow *beeb_window);
std::unique_ptr<SettingsUI> CreateParasiteStackDebugWindow(BeebWindow *beeb_window);
std::unique_ptr<SettingsUI> CreateTubeDebugWindow(BeebWindow *beeb_window);
std::unique_ptr<SettingsUI> CreateADCDebugWindow(BeebWindow *beeb_window);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#endif

#endif
