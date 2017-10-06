#include <shared/system.h>
#include "MessagesUI.h"
#include "dear_imgui.h"
#include "Messages.h"
#include "commands.h"
#include "SettingsUI.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static const ImVec4 INFO_COLOUR(1.f,1.f,1.f,1.f);
static const ImVec4 WARNING_COLOUR(1.f,1.f,0.f,1.f);
static const ImVec4 ERROR_COLOUR(1.f,0.f,0.f,1.f);

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ImGuiMessageListMessage(const MessageList::Message *m) {
    ImGuiStyleColourPusher pusher;

    switch(m->type) {
    case MessageType_Info:
        pusher.Push(ImGuiCol_Text,INFO_COLOUR);
        break;

    case MessageType_Warning:
        pusher.Push(ImGuiCol_Text,WARNING_COLOUR);
        break;

    case MessageType_Error:
        pusher.Push(ImGuiCol_Text,ERROR_COLOUR);
        break;
    }

    ImGui::TextUnformatted(m->text.c_str(),m->text.c_str()+m->text.size());
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class MessagesUI:
    public SettingsUI
{
public:
    MessagesUI(std::shared_ptr<MessageList> message_list);

    void DoImGui(CommandContextStack *cc_stack) override;

    bool OnClose() override;
protected:
private:
    std::shared_ptr<MessageList> m_message_list;

    ObjectCommandContext<MessagesUI> m_occ;

    void Copy();
    void Clear();

    static ObjectCommandTable<MessagesUI> ms_command_table;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

ObjectCommandTable<MessagesUI> MessagesUI::ms_command_table("Messages Window",{
    {"copy","Copy",&MessagesUI::Copy},
    {"clear","Clear",&MessagesUI::Clear}
});

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

MessagesUI::MessagesUI(std::shared_ptr<MessageList> message_list):
    m_message_list(std::move(message_list)),
    m_occ(this,&ms_command_table)
{
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MessagesUI::DoImGui(CommandContextStack *cc_stack) {
    cc_stack->Push(m_occ);

    m_occ.DoButton("clear");

    ImGui::SameLine();

    m_occ.DoButton("copy");

    ImGui::BeginChild("",ImVec2(),true);

    m_message_list->ForEachMessage(&ImGuiMessageListMessage);

    ImGui::EndChild();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool MessagesUI::OnClose() {
    return false;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MessagesUI::Copy() {
    ImGuiIO &io=ImGui::GetIO();

    std::string text;

    m_message_list->ForEachMessage(
        [&text](const MessageList::Message *m) {
        text+=m->text;
    });

    (*io.SetClipboardTextFn)(io.ClipboardUserData,text.c_str());
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void MessagesUI::Clear() {
    m_message_list->ClearMessages();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

std::unique_ptr<SettingsUI> CreateMessagesUI(std::shared_ptr<MessageList> message_list) {
    return std::make_unique<MessagesUI>(std::move(message_list));
}

