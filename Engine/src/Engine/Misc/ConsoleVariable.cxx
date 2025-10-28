#include "Engine/Misc/ConsoleVariable.hxx"

FConsoleVariableRegistry& FConsoleVariableRegistry::Get()
{
    static FConsoleVariableRegistry Instance;
    return Instance;
}

bool FConsoleVariableRegistry::SetVariableValue(const std::string_view Name, const std::string_view Value)
{
    IConsoleVariable** FoundVar = Variables.Find(Name.data());
    if (FoundVar)
    {
        (*FoundVar)->SetFromString(Value);
        return true;
    }
    return false;
}

template <>
void TConsoleVariable<int32>::SetFromString(const std::string_view View)
{
    SetValue(static_cast<int32>(std::stoi(std::string(View))));
}

template <>
void TConsoleVariable<float>::SetFromString(const std::string_view View)
{
    SetValue(std::stof(std::string(View)));
}

template <>
void TConsoleVariable<bool>::SetFromString(const std::string_view View)
{
    if (View == "1" || View == "true" || View == "True" || View == "TRUE")
    {
        SetValue(true);
    }
    else
    {
        SetValue(false);
    }
}
