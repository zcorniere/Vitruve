#pragma once

#if VIT_ENABLE_PROFILING
    #define TracyFunction ::RTTI::FunctionName()
    #define TracyFile __FILE__
    #define TracyLine __LINE__

    #include <tracy/Tracy.hpp>

    #define VIT_PROFILE_MARK_FRAME FrameMark;
    #define VIT_PROFILE_FUNC(...) ZoneScoped##__VA_OPT__(N(__VA_ARGS__));
    #define VIT_PROFILE_SCOPE_DYNAMIC(Name) \
        ZoneScoped;                         \
        ZoneName(Name, strlen(Name));
    #define VIT_PROFILE_THREAD(...) tracy::SetThreadName(__VA_ARGS__);

    #if VIT_ENABLE_MEMORY_PROFILING
        #define VIT_PROFILE_ALLOC(Pointer, Size) TracyAlloc(Pointer, Size);
        #define VIT_PROFILE_FREE(Pointer) TracyFree(Pointer);
    #else
        #define VIT_PROFILE_ALLOC(Pointer, Size)
        #define VIT_PROFILE_FREE(Pointer)
    #endif    //! VIT_ENABLE_MEMORY_PROFILING

#else
    #define VIT_PROFILE_MARK_FRAME
    #define VIT_PROFILE_FUNC(...)
    #define VIT_PROFILE_SCOPE_DYNAMIC(Name)
    #define VIT_PROFILE_THREAD(...)
    #define VIT_PROFILE_ALLOC(Pointer, Size)
    #define VIT_PROFILE_FREE(Pointer)
#endif    //! VIT_ENABLE_PROFILING
