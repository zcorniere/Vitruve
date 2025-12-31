#pragma once

#include "Engine/Platforms/PlatformStacktrace.hxx"
// IWYU pragma: private, include "PlatformStacktrace.hxx"

class ENGINE_API FWindowsStacktrace : public FGenericStacktrace
{
public:
    /// @brief Return a stacktrace of the current running process
    static FStacktraceContent GetStackTraceFromReturnAddress(void* returnAddress);

    static bool TryFillDetailedSymbolInfo(int64 ProgramCounter, FDetailedSymbolInfo& detailed_info);
};
using FPlatformStacktrace = FWindowsStacktrace;
