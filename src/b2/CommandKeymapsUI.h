#ifndef HEADER_297BD1FD68834B09A1FBE55C2A4AAE8B// -*- mode:c++ -*-
#define HEADER_297BD1FD68834B09A1FBE55C2A4AAE8B

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include "SettingsUI.h"

class CommandContextStack;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class CommandKeymapsUI:
    public SettingsUI
{
public:
    CommandKeymapsUI();

    void DoImGui(CommandContextStack *cc_stack) override;

    bool WantsKeyboardFocus() const override;
    bool OnClose() override;
protected:
private:
    bool m_edited=false;
    bool m_wants_keyboard_focus=false;
    float m_max_command_text_width=0.f;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#endif
