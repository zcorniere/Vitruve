#include "Engine/Core/Application.hxx"

#include "Application.hxx"
#include "Engine/Core/Window.hxx"
#include "Engine/Misc/ConsoleVariable.hxx"
#include "Engine/RHI/RHI.hxx"
#include "Engine/RHI/Resources/RHIViewport.hxx"

#include <imgui.h>

#include "Engine/Misc/Utils.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogBaseApplication, Info)

IApplication* GApplication = nullptr;

FBaseApplication::FBaseApplication()
{
    GApplication = this;
}

FBaseApplication::~FBaseApplication()
{
    GApplication = nullptr;
}

bool FBaseApplication::OnEngineInitialization()
{
    VIT_PROFILE_FUNC()

    FWindowDefinition WindowDef{
        .AppearsInTaskbar = true,
        .Title = "Vitruve Engine",
        .EventCallback = [this](FEvent& event) { WindowEventHandler(event); },
    };
    MainWindow = Ref<RWindow>::Create();
    MainWindow->SetName("MainWindow");
    MainWindow->Initialize(WindowDef);
    MainWindow->Show();
    MainWindow->Maximize();

    MainViewport = RHI::CreateViewport(MainWindow, UVector2{500u, 500u}, true);
    MainViewport->SetName("MainViewport");
    return true;
}

void FBaseApplication::OnEngineDestruction()
{
    VIT_PROFILE_FUNC()

    MainViewport = nullptr;

    MainWindow->Destroy();
    MainWindow = nullptr;
}

void FBaseApplication::WindowEventHandler(FEvent& Event)
{
    FEventDispatcher dispatcher(Event);
    dispatcher.Dispatch<FWindowResizeEvent>([this](FWindowResizeEvent& Event) { return OnWindowResize(Event); });
    dispatcher.Dispatch<FWindowMinimizeEvent>([this](FWindowMinimizeEvent& Event) { return OnWindowMinimize(Event); });
    dispatcher.Dispatch<FWindowCloseEvent>([this](FWindowCloseEvent& Event) { return OnWindowClose(Event); });

    if (!Event.Handled)
    {
        LOG(LogBaseApplication, Trace, "Unhandled Event : {}", Event);
    }
}

void FBaseApplication::Tick(const double DeltaTime)
{
    VIT_PROFILE_FUNC()

    (void)DeltaTime;

    ImGui::Begin("Base Application");
    ImGui::Text("Hello from the Base Application!");
    ImGui::BeginTable("ConsoleVariables", 2);
    for (const IConsoleVariable* CVar: FConsoleVariableRegistry::Get().GetAllConsoleVariables())
    {
        ImGui::PushID(CVar->GetName());
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(CVar->GetName());
        ImGui::TableNextColumn();
        std::string valueStr = CVar->GetValueAsString();
        char valueBuffer[256] = {0};
        if (ImGui::InputTextWithHint(valueStr.c_str(), valueStr.c_str(), valueBuffer, sizeof(valueBuffer),
                                     ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
        {
            FConsoleVariableRegistry::Get().SetVariableValue(CVar->GetName(), valueBuffer);
        }

        const char* const HelpText = CVar->GetHelpText();
        if (HelpText)
        {
            ImGui::TextWrapped("%s", HelpText);
        }
        ImGui::PopID();
    }
    ImGui::EndTable();
    ImGui::End();

    MainWindow->ProcessEvents();
}

bool FBaseApplication::OnWindowResize(FWindowResizeEvent& E)
{
    VIT_PROFILE_FUNC()

    const uint32 width = E.GetWidth();
    const uint32 height = E.GetHeight();
    if (width == 0 || height == 0)
    {
        return false;
    }
    // The viewport was not created yet
    if (MainViewport)
    {
        MainViewport->ResizeViewport(width, height);
    }
    return false;
}

bool FBaseApplication::OnWindowMinimize(FWindowMinimizeEvent& E)
{
    (void)E;
    return false;
}
bool FBaseApplication::OnWindowClose(FWindowCloseEvent& E)
{
    (void)E;
    Utils::RequestExit(0);
    return false;
}
