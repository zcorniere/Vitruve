#include "Engine/Main.hxx"
#include "EditorApplication.hxx"

int main(int argc, char** argv)
{
    return EngineMain(argc, argv, &CreateApplication);
}
