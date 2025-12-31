#include "Engine/Platforms/Linux/LinuxStacktrace.hxx"

#include <dlfcn.h>
#include <elfutils/libdwfl.h>
#include <execinfo.h>
#include <fcntl.h>

static Dwfl* dwfl = nullptr;

void FLinuxStacktrace::InitDWARF()
{
#if VIT_ENABLE_STACKTRACE
    VIT_PROFILE_FUNC()

    static const Dwfl_Callbacks callbacks = {
        .find_elf = dwfl_linux_proc_find_elf,
        .find_debuginfo = dwfl_standard_find_debuginfo,
        .section_address = nullptr,
        .debuginfo_path = nullptr,
    };
    dwfl = dwfl_begin(&callbacks);

    RefreshDWARF();
#endif    // VIT_ENABLE_STACKTRACE
}

void FLinuxStacktrace::RefreshDWARF()
{
#if VIT_ENABLE_STACKTRACE
    VIT_PROFILE_FUNC()
    if (dwfl_linux_proc_report(dwfl, getpid()) == -1)
    {
        dwfl_end(dwfl);
        dwfl = nullptr;
        LOG(LogUnixPlateform, Warning, "Failed to init DWARF for stacktrace");
    }
    else
    {
        dwfl_report_end(dwfl, nullptr, nullptr);
    }
#endif    // VIT_ENABLE_STACKTRACE
}

void FLinuxStacktrace::ShutdownDWARF()
{
#if VIT_ENABLE_STACKTRACE
    if (dwfl != nullptr)
    {
        dwfl_end(dwfl);
    }
#endif    // VIT_ENABLE_STACKTRACE
}

FStacktraceContent FLinuxStacktrace::GetStackTraceFromReturnAddress(void* returnAddress)
{
    FStacktraceContent trace;
    trace.Depth = backtrace(reinterpret_cast<void**>(trace.StackTrace), trace.MaxDepth);
    trace.CurrentDepth = trace.Depth;

    if (returnAddress != nullptr)
    {
        for (std::uint32_t i = 0; i < trace.Depth; ++i)
        {
            if (trace.StackTrace[i] != int64(returnAddress))
            {
                continue;
            }
            trace.CurrentDepth = i;
            break;
        }
    }

    return trace;
}

bool FLinuxStacktrace::TryFillDetailedSymbolInfo(int64 ProgramCounter, FDetailedSymbolInfo& detailed_info)
{
    detailed_info.ProgramCounter = ProgramCounter;
    if (dwfl == nullptr)
    {
        return false;
    }

#if VIT_ENABLE_STACKTRACE
    Dwfl_Module* const mod = dwfl_addrmodule(dwfl, ProgramCounter);
    if (mod == nullptr)
    {
        return false;
    }

    std::string_view moduleName = dwfl_module_info(mod, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    if (!moduleName.empty())
    {
        size_t Position = moduleName.find_last_of('/');
        if (Position != std::string_view::npos)
        {
            moduleName = moduleName.substr(Position + 1);
        }
        std::strncpy(detailed_info.ModuleName, moduleName.data(), FDetailedSymbolInfo::MaxNameLength);
    }

    Dwfl_Line* const line = dwfl_module_getsrc(mod, ProgramCounter);
    if (line != nullptr)
    {
        std::string_view filename = dwfl_lineinfo(line, nullptr, &detailed_info.LineNumber, nullptr, nullptr, nullptr);
        if (!filename.empty())
        {
            size_t Position = filename.find("/Vitruve/");
            if (Position != std::string_view::npos)
            {
                filename = filename.substr(Position + 1);
            }
            std::strncpy(detailed_info.Filename, filename.data(), FDetailedSymbolInfo::MaxNameLength);
        }
    }

    const char* const symbolName = dwfl_module_addrname(mod, ProgramCounter);
    if (symbolName != nullptr)
    {
        std::strncpy(detailed_info.FunctionName, symbolName, FDetailedSymbolInfo::MaxNameLength);
    }
#endif    // VIT_ENABLE_STACKTRACE
    return true;
}
