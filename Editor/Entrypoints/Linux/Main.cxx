#include "Engine/Main.hxx"
#include "EditorApplication.hxx"
#include "Engine/Modules/ImplementationMacros.hxx"

MODULE_BOILERPLATE

int main(int argc, char** argv)
{
    return EngineMain(argc, argv, &CreateApplication);
}
