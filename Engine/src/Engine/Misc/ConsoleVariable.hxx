#pragma once

class IConsoleVariable
{
public:
    virtual const char* GetName() const = 0;
    virtual const char* GetHelpText() const = 0;
    virtual void SetFromString(const std::string_view View) = 0;
    virtual std::string GetValueAsString() const = 0;
};

class FConsoleVariableRegistry
{
public:
    ENGINE_API static FConsoleVariableRegistry& Get();

    IConsoleVariable* RegisterVariable(IConsoleVariable* Variable)
    {
        Variables.Insert(Variable->GetName(), Variable);
        return Variable;
    }

    bool SetVariableValue(const std::string_view Name, const std::string_view Value);

    TArray<const IConsoleVariable*> GetAllConsoleVariables() const
    {
        TArray<const IConsoleVariable*> Vars;
        for (const auto& Pair: Variables)
        {
            Vars.Add(Pair.Get<1>());
        }
        return Vars;
    }

private:
    TMap<const char*, IConsoleVariable*> Variables;
};

template <typename T>
class ENGINE_API TConsoleVariable final : public IConsoleVariable
{
public:
    using Type = T;

    TConsoleVariable(const char* InName, T InDefaultValue, const char* InHelpText)
        : Name(InName)
        , Value(InDefaultValue)
        , HelpText(InHelpText)
    {
        FConsoleVariableRegistry::Get().RegisterVariable(this);
    }

    const char* GetName() const override
    {
        return Name;
    }

    T GetValue() const
    {
        return Value.load();
    }

    void SetValue(T InValue)
    {
        Value.store(InValue);
    }

    virtual std::string GetValueAsString() const override
    {
        return std::to_string(GetValue());
    }

    virtual void SetFromString(const std::string_view View) override;

    const char* GetHelpText() const override
    {
        return HelpText;
    }

private:
    const char* Name;
    std::atomic<T> Value;
    const char* HelpText;
};
