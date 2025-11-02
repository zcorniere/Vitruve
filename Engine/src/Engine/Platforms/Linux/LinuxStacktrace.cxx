#include "Engine/Platforms/Linux/LinuxStacktrace.hxx"

#include <dlfcn.h>
#include <elfutils/libdwfl.h>
#include <execinfo.h>
#include <fcntl.h>

static Dwfl* dwfl = nullptr;

void FLinuxStacktrace::InitDWARF()
{
    VIT_PROFILE_FUNC()
    static const Dwfl_Callbacks callbacks = {
        .find_elf = dwfl_linux_proc_find_elf,
        .find_debuginfo = dwfl_standard_find_debuginfo,
        .section_address = nullptr,
        .debuginfo_path = nullptr,
    };
    dwfl = dwfl_begin(&callbacks);

    RefreshDWARF();
}

void FLinuxStacktrace::RefreshDWARF()
{
    if (dwfl_linux_proc_report(dwfl, getpid()) == -1)
    {
        dwfl_end(dwfl);
        dwfl = nullptr;
        LOG(LogUnixPlateform, Warning, "Failed to init DWARF for stacktrace");
        return;
    }
    else
    {
        dwfl_report_end(dwfl, nullptr, nullptr);
    }
}

void FLinuxStacktrace::ShutdownDWARF()
{
    if (dwfl != nullptr)
    {
        dwfl_end(dwfl);
    }
}

StacktraceContent FLinuxStacktrace::GetStackTraceFromReturnAddress(void* returnAddress)
{
    StacktraceContent trace;
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

bool FLinuxStacktrace::TryFillDetailedSymbolInfo(int64 ProgramCounter, DetailedSymbolInfo& detailed_info)
{
    detailed_info.ProgramCounter = ProgramCounter;
    if (dwfl == nullptr)
    {
        return false;
    }

    Dwfl_Module* const mod = dwfl_addrmodule(dwfl, ProgramCounter);
    if (mod == nullptr)
    {
        return false;
    }

    std::string_view moduleName = dwfl_module_info(mod, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    if (!moduleName.empty())
    {
        auto Position = moduleName.find_last_of('/');
        if (Position != std::string_view::npos)
        {
            moduleName = moduleName.substr(Position + 1);
        }
        std::strncpy(detailed_info.ModuleName, moduleName.data(), DetailedSymbolInfo::MaxNameLength);
    }

    Dwfl_Line* line = dwfl_module_getsrc(mod, ProgramCounter);
    if (line != nullptr)
    {
        std::string_view filename = dwfl_lineinfo(line, nullptr, &detailed_info.LineNumber, nullptr, nullptr, nullptr);
        if (!filename.empty())
        {
            auto Position = filename.find_first_of("Vitruve/");
            if (Position != std::string_view::npos)
            {
                filename = filename.substr(Position);
            }
            std::strncpy(detailed_info.Filename, filename.data(), DetailedSymbolInfo::MaxNameLength);
        }
    }

    const char* const symbolName = dwfl_module_addrname(mod, ProgramCounter);
    if (symbolName != nullptr)
    {
        std::strncpy(detailed_info.FunctionName, symbolName, DetailedSymbolInfo::MaxNameLength);
    }
    return true;
}
