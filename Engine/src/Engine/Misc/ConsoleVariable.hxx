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

    TConsoleVariable(const char* InName, T InDefaultValue, const char* InHelpText = nullptr)
        : Name(InName)
        , HelpText(InHelpText)
        , Value(InDefaultValue)
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

    T operator*()
    {
        return GetValue();
    }

private:
    const char* Name;
    const char* HelpText;
    std::atomic<T> Value;
};

template <>
class ENGINE_API TConsoleVariable<const char*> final : public IConsoleVariable
{
public:
    using Type = std::string;

    TConsoleVariable(const char* InName, const char* InDefaultValue, const char* InHelpText)
        : Name(InName)
        , HelpText(InHelpText)
        , Value(InDefaultValue)
    {
        FConsoleVariableRegistry::Get().RegisterVariable(this);
    }

    const char* GetName() const override
    {
        return Name;
    }

    std::string GetValue() const
    {
        return Value;
    }

    void SetValue(const std::string& InValue)
    {
        Value = InValue;
    }

    virtual std::string GetValueAsString() const override
    {
        return GetValue();
    }

    virtual void SetFromString(const std::string_view View) override
    {
        SetValue(std::string(View));
    }

    const char* GetHelpText() const override
    {
        return HelpText;
    }

private:
    const char* Name;
    const char* HelpText;
    std::string Value;
};
