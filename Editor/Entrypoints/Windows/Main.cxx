#include "Engine/Main.hxx"
#include "EditorApplication.hxx"
#include "Engine/Modules/ImplementationMacros.hxx"

#include <stdlib.h>
#include <windows.h>

MODULE_BOILERPLATE

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    (void)hInstance;
    (void)hPrevInstance;
    (void)nShowCmd;
    (void)lpCmdLine;
    
    // TODO: Convert lpCmdLine to argc/argv
    return EngineMain(__argc, __argv, &CreateApplication);
}

