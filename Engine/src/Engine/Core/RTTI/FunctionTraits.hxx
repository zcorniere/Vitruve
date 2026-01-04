#pragma once

#include <cstddef>
#include <tuple>

// Thanks to https://stackoverflow.com/questions/7943525/
// is-it-possible-to-figure-out-the-parameter-type-and-return-type-of-a-lambda?rq=1

namespace RTTI
{

template <typename>
struct FunctionTraits;

template <typename ReturnType, typename... Args>
struct FunctionTraits<ReturnType(Args...)> : FunctionTraits<ReturnType (*)(Args...)>
{
};

// Specialization for pointers to const member function or lambdas
template <typename ClassType, typename ReturnType, typename... Args>
struct FunctionTraits<ReturnType (ClassType::*)(Args...) const> : FunctionTraits<ReturnType (*)(Args...)>
{
    using Class = ClassType;
};

// Specialization for pointers to member function or lambdas
template <typename ClassType, typename ReturnType, typename... Args>
struct FunctionTraits<ReturnType (ClassType::*)(Args...)> : FunctionTraits<ReturnType (*)(Args...)>
{
    using Class = ClassType;
};

template <typename T>
struct FunctionTraits : public FunctionTraits<decltype(&T::operator())>
{
};

// Specialization for function pointers
template <typename ReturnType, typename... Args>
struct FunctionTraits<ReturnType (*)(Args...)>
{
    static constexpr size_t arity = sizeof...(Args);

    using result_type = ReturnType;

    using args_type = std::tuple<Args...>;

    template <size_t i>
    struct arg
    {
        using type = typename std::tuple_element<i, std::tuple<Args...>>::type;
    };
};

template <typename, typename, typename>
struct IsApplicable : std::false_type
{
};

template <typename RetValue, typename Func, template <typename...> typename Tuple, typename... Args>
struct IsApplicable<RetValue, Func, Tuple<Args...>> : std::is_invocable_r<RetValue, Func, Args...>
{
};

template <typename RetValue, typename F, typename Tuple>
concept Applicable = IsApplicable<RetValue, F, Tuple>::value;

}    // namespace RTTI
