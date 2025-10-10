#pragma once

#include "Engine/Core/RTTI/FunctionTraits.hxx"

template <typename Fn>
concept IsDelegateFunction =
    requires { requires std::is_same_v<typename ::RTTI::FunctionTraits<Fn>::result_type, void>; };

namespace details
{

template <typename T>
struct wrapper : public wrapper<typename RTTI::FunctionTraits<T>::args_type>
{
};

template <typename... ArgsType>
struct wrapper<std::tuple<ArgsType...>>
{
    static constexpr auto wrap_function(auto system)
    {
        return [system = std::move(system)](ArgsType&&... Args) { std::apply(system, Args...); };
    }

    template <typename TClass>
    static constexpr auto wrap_function(TClass* InContext, auto system)
    {
        return [system = std::move(system), InContext](ArgsType&&... Args)
        { std::invoke(system, InContext, std::forward<ArgsType>(Args)...); };
    }

    template <typename TClass>
    static constexpr auto wrap_function(WeakRef<TClass> InContext, auto system)
    {
        return [system = std::move(system), InContext = InContext.Pin()](ArgsType&&... Args)
        {
            if (InContext) [[likely]]
            {
                std::invoke(system, InContext, std::forward<ArgsType>(Args)...);
            }
        };
    }
};

}    // namespace details

template <IsDelegateFunction TFunctionSignature>
class TDelegate
{
private:
    using Traits = RTTI::FunctionTraits<TFunctionSignature>;

public:
    TDelegate() = default;
    ~TDelegate() = default;

    void Add(std::function<TFunctionSignature>&& Function)
    {
        if (bIsBroadcasting)
        {
            LOG(LogCore, Error, "Adding a function to a delegate while broadcasting is not allowed");
            return;
        }
        BindedFunctions.Emplace(std::move(Function));
    }

    template <typename T, typename Fn>
    void Add(WeakRef<T> OwnerPtr, Fn Function)
    requires(std::is_same_v<typename Traits::args_type, typename RTTI::FunctionTraits<Fn>::args_type>) &&
            (std::is_base_of_v<RObject, T>)    // Use the WeakRef version for RObject, please
    {
        if (bIsBroadcasting)
        {
            LOG(LogCore, Error, "Adding a function to a delegate while broadcasting is not allowed");
            return;
        }
        BindedFunctions.Emplace(
            details::wrapper<decltype(Function)>::template wrap_function<WeakRef<T>>(OwnerPtr, std::move(Function)));
    }
    template <typename T, typename Fn>
    void Add(T* OwnerPtr, Fn Function)
    requires std::is_member_function_pointer_v<std::decay_t<Fn>> &&
             (std::is_same_v<typename Traits::args_type, typename RTTI::FunctionTraits<Fn>::args_type>)
    // not the best requires expression, but it gets the job done. A bit.
    {
        if (bIsBroadcasting)
        {
            LOG(LogCore, Error, "Adding a function to a delegate while broadcasting is not allowed");
            return;
        }
        BindedFunctions.Emplace(
            details::wrapper<decltype(Function)>::template wrap_function<T>(OwnerPtr, std::move(Function)));
    }

    template <typename... TFunctionArgs>
    requires std::is_invocable_v<TFunctionSignature, TFunctionArgs...>
    void Broadcast(TFunctionArgs&&... Args)
    {
        bIsBroadcasting = true;
        for (std::function<TFunctionSignature>& Function: BindedFunctions)
        {
            Function(std::forward<TFunctionArgs>(Args)...);
        }
        bIsBroadcasting = false;
    }

private:
    bool bIsBroadcasting = false;
    TArray<std::function<TFunctionSignature>> BindedFunctions;
};
