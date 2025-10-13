#include "Engine/Main.hxx"
#include "EditorApplication.hxx"

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    (void)hInstance;
    (void)hPrevInstance;
    (void)nShowCmd;
    // TODO: Convert lpCmdLine to argc/argv
    return EngineMain(0, nullptr, &CreateApplication);
}
