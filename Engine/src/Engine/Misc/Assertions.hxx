#pragma once

// The following header are used in the macro definitions
#include "Engine/Compilers/Compiler.hxx"    // IWYU pragma: keep
#include "Engine/Platforms/Platform.hxx"

#include <atomic>    // IWYU pragma: keep

#define VIT_CHECK_STACKTRACE

namespace Vitruve::Debug
{

ENGINE_API bool HandleCheckFailure(const std::string& Message, bool bShouldAbort);

/// Whether or not the check should print the stacktrace
consteval bool ShouldCheckPrintStackTrace()
{
#if defined(VIT_CHECK_STACKTRACE)
    return true;
#else
    return false;
#endif
}

#ifndef NDEBUG
class FRecursionScopeMarker
{
public:
    FRecursionScopeMarker(uint16& InCounter): Counter(InCounter)
    {
        ++Counter;
    }
    ~FRecursionScopeMarker()
    {
        --Counter;
    }

private:
    uint16& Counter;
};
#endif    // !NDEBUG

}    // namespace Vitruve::Debug

#ifndef NDEBUG

    #define VITRUVE_ENSURE_IMPL(Always, Expression, Format, ...) \
        (((Expression)) || (([__VA_OPT__(&)] {                                                             \
                                static std::atomic_bool bExecuted = false;                                 \
                                if (!bExecuted || Always) {                                                \
                                    bExecuted.exchange(true, std::memory_order_release);                   \
                                                                                                           \
                                    const std::string Message =                                            \
                                        std::format("Assertion failed: " STR(#Expression) " in {}" Format, \
                                                    ::RTTI::FilePosition() __VA_OPT__(, ) __VA_ARGS__);    \
                                    return ::Vitruve::Debug::HandleCheckFailure(Message, false);           \
                                }                                                                          \
                                return false;                                                              \
                            }()) &&                                                                        \
                            ([] {                                                                          \
                                PLATFORM_BREAK();                                                          \
                                return false;                                                              \
                            }())))

    #define ensure(Expression) VITRUVE_ENSURE_IMPL(false, Expression, )
    #define ensureMsg(Expression, Format, ...) VITRUVE_ENSURE_IMPL(false, Expression, " :: " Format, ##__VA_ARGS__)
    #define ensureAlways(Expression) VITRUVE_ENSURE_IMPL(true, Expression, )
    #define ensureAlwaysMsg(Expression, Format, ...) VITRUVE_ENSURE_IMPL(true, Expression, " :: " Format, ##__VA_ARGS__)

    #define Vitruve_CHECK_IMPL(Expression, Format, ...)                                                        \
        {                                                                                                      \
            if (!(Expression)) [[unlikely]]                                                                    \
            {                                                                                                  \
                const std::string Message = std::format("Assertion failed: " STR(#Expression) " in {}" Format, \
                                                        ::RTTI::FilePosition() __VA_OPT__(, ) __VA_ARGS__);    \
                ::Vitruve::Debug::HandleCheckFailure(Message, true);                                           \
            }                                                                                                  \
        }

    #define check(Expression) Vitruve_CHECK_IMPL(Expression, )
    #define checkSlow(Expression) check(Expression)
    #define checkMsg(Expression, Format, ...) Vitruve_CHECK_IMPL(Expression, " :: " Format, ##__VA_ARGS__)
    #define checkNoEntry()                                             \
        {                                                              \
            checkMsg(false, "Enclosing block should never be called"); \
            ::Compiler::Unreachable();                                 \
        }
    #define checkNoReentry()                                                                            \
        {                                                                                               \
            static std::atomic_bool MACRO_EXPENDER(beenHere, __LINE__) = false;                         \
            checkMsg(!MACRO_EXPENDER(beenHere, __LINE__), "Enclosing block was called more than once"); \
            MACRO_EXPENDER(beenHere, __LINE__) = true;                                                  \
        }

    #define checkNoRecursion()                                                                                \
        static uint16 MACRO_EXPENDER(RecursionCounter, __LINE__) = 0;                                         \
        checkMsg(MACRO_EXPENDER(RecursionCounter, __LINE__) == 0, "Enclosing block was entered recursively"); \
        const ::Vitruve::RecursionScopeMarker MACRO_EXPENDER(ScopeMarker,                                     \
                                                             __LINE__)(MACRO_EXPENDER(RecursionCounter, __LINE__))

#else
    #define VITRUVE_ENSURE_IMPL(Expression) (bool)(Expression)
    #define ensure(Expression) VITRUVE_ENSURE_IMPL(Expression)
    #define ensureMsg(Expression, ...) VITRUVE_ENSURE_IMPL(Expression)
    #define ensureAlways(Expression) VITRUVE_ENSURE_IMPL(Expression)
    #define ensureAlwaysMsg(Expression, ...) VITRUVE_ENSURE_IMPL(Expression)

    #define Vitruve_CHECK_IMPL(Expression) (void)(Expression);
    #define check(Expression) Vitruve_CHECK_IMPL(Expression)
    #define checkSlow(Expression)
    #define checkMsg(Expression, ...) Vitruve_CHECK_IMPL(Expression)
    #define checkNoEntry()             \
        {                              \
            ::Compiler::Unreachable(); \
        }
    #define checkNoReentry()                                                    \
        {                                                                       \
            static std::atomic_bool MACRO_EXPENDER(beenHere, __LINE__) = false; \
            if (MACRO_EXPENDER(beenHere, __LINE__) == true) [[unlikely]]        \
            {                                                                   \
                ::Compiler::Unreachable();                                      \
            }                                                                   \
            MACRO_EXPENDER(beenHere, __LINE__) = true;                          \
        }
    #define checkNoRecursion()

#endif
