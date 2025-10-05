#pragma once

#define UE_DEFINE_FMEMORY_WRAPPERS                                       \
    void* FMemory_Malloc(size_t Size, size_t Alignment)                  \
    {                                                                    \
        return Memory::Malloc(Size, Alignment);                          \
    }                                                                    \
    void* FMemory_Realloc(void* Original, size_t Size, size_t Alignment) \
    {                                                                    \
        return Memory::Realloc(Original, Size, Alignment);               \
    }                                                                    \
    void FMemory_Free(void* Ptr)                                         \
    {                                                                    \
        Memory::Free(Ptr);                                               \
    }

static_assert(__STDCPP_DEFAULT_NEW_ALIGNMENT__ <= 16, "Expecting 16-byte default operator new alignment");
#define REPLACEMENT_OPERATOR_NEW_AND_DELETE                                                                     \
    FORCENOINLINE void* operator new(size_t Size)                                                               \
    {                                                                                                           \
        return FMemory_Malloc(Size ? Size : 1, Size <= 8 ? 8 : __STDCPP_DEFAULT_NEW_ALIGNMENT__);               \
    }                                                                                                           \
    FORCENOINLINE void* operator new[](size_t Size)                                                             \
    {                                                                                                           \
        return FMemory_Malloc(Size ? Size : 1, Size <= 8 ? 8 : __STDCPP_DEFAULT_NEW_ALIGNMENT__);               \
    }                                                                                                           \
    FORCENOINLINE void* operator new(size_t Size, const std::nothrow_t&) noexcept                               \
    {                                                                                                           \
        return FMemory_Malloc(Size ? Size : 1, Size <= 8 ? 8 : __STDCPP_DEFAULT_NEW_ALIGNMENT__);               \
    }                                                                                                           \
    FORCENOINLINE void* operator new[](size_t Size, const std::nothrow_t&) noexcept                             \
    {                                                                                                           \
        return FMemory_Malloc(Size ? Size : 1, Size <= 8 ? 8 : __STDCPP_DEFAULT_NEW_ALIGNMENT__);               \
    }                                                                                                           \
    FORCENOINLINE void* operator new(size_t Size, std::align_val_t Alignment)                                   \
    {                                                                                                           \
        return FMemory_Malloc(Size ? Size : 1, (std::size_t)Alignment);                                         \
    }                                                                                                           \
    FORCENOINLINE void* operator new[](size_t Size, std::align_val_t Alignment)                                 \
    {                                                                                                           \
        return FMemory_Malloc(Size ? Size : 1, (std::size_t)Alignment);                                         \
    }                                                                                                           \
    FORCENOINLINE void* operator new(size_t Size, std::align_val_t Alignment, const std::nothrow_t&) noexcept   \
    {                                                                                                           \
        return FMemory_Malloc(Size ? Size : 1, (std::size_t)Alignment);                                         \
    }                                                                                                           \
    FORCENOINLINE void* operator new[](size_t Size, std::align_val_t Alignment, const std::nothrow_t&) noexcept \
    {                                                                                                           \
        return FMemory_Malloc(Size ? Size : 1, (std::size_t)Alignment);                                         \
    }                                                                                                           \
    FORCENOINLINE void operator delete(void* Ptr) noexcept                                                      \
    {                                                                                                           \
        FMemory_Free(Ptr);                                                                                      \
    }                                                                                                           \
    FORCENOINLINE void operator delete[](void* Ptr) noexcept                                                    \
    {                                                                                                           \
        FMemory_Free(Ptr);                                                                                      \
    }                                                                                                           \
    FORCENOINLINE void operator delete(void* Ptr, const std::nothrow_t&) noexcept                               \
    {                                                                                                           \
        FMemory_Free(Ptr);                                                                                      \
    }                                                                                                           \
    FORCENOINLINE void operator delete[](void* Ptr, const std::nothrow_t&) noexcept                             \
    {                                                                                                           \
        FMemory_Free(Ptr);                                                                                      \
    }                                                                                                           \
    FORCENOINLINE void operator delete(void* Ptr, size_t Size) noexcept                                         \
    {                                                                                                           \
        (void)Size;                                                                                             \
        FMemory_Free(Ptr);                                                                                      \
    }                                                                                                           \
    FORCENOINLINE void operator delete[](void* Ptr, size_t Size) noexcept                                       \
    {                                                                                                           \
        (void)Size;                                                                                             \
        FMemory_Free(Ptr);                                                                                      \
    }                                                                                                           \
    FORCENOINLINE void operator delete(void* Ptr, size_t Size, const std::nothrow_t&) noexcept                  \
    {                                                                                                           \
        (void)Size;                                                                                             \
        FMemory_Free(Ptr);                                                                                      \
    }                                                                                                           \
    FORCENOINLINE void operator delete[](void* Ptr, size_t Size, const std::nothrow_t&) noexcept                \
    {                                                                                                           \
        (void)Size;                                                                                             \
        FMemory_Free(Ptr);                                                                                      \
    }                                                                                                           \
    FORCENOINLINE void operator delete(void* Ptr, std::align_val_t Alignment) noexcept                          \
    {                                                                                                           \
        (void)Alignment;                                                                                        \
        FMemory_Free(Ptr);                                                                                      \
    }                                                                                                           \
    FORCENOINLINE void operator delete[](void* Ptr, std::align_val_t Alignment) noexcept                        \
    {                                                                                                           \
        (void)Alignment;                                                                                        \
        FMemory_Free(Ptr);                                                                                      \
    }                                                                                                           \
    FORCENOINLINE void operator delete(void* Ptr, std::align_val_t Alignment, const std::nothrow_t&) noexcept   \
    {                                                                                                           \
        (void)Alignment;                                                                                        \
        FMemory_Free(Ptr);                                                                                      \
    }                                                                                                           \
    FORCENOINLINE void operator delete[](void* Ptr, std::align_val_t Alignment, const std::nothrow_t&) noexcept \
    {                                                                                                           \
        (void)Alignment;                                                                                        \
        FMemory_Free(Ptr);                                                                                      \
    }                                                                                                           \
    FORCENOINLINE void operator delete(void* Ptr, size_t Size, std::align_val_t Alignment) noexcept             \
    {                                                                                                           \
        (void)Size;                                                                                             \
        (void)Alignment;                                                                                        \
        FMemory_Free(Ptr);                                                                                      \
    }                                                                                                           \
    FORCENOINLINE void operator delete[](void* Ptr, size_t Size, std::align_val_t Alignment) noexcept           \
    {                                                                                                           \
        (void)Size;                                                                                             \
        (void)Alignment;                                                                                        \
        FMemory_Free(Ptr);                                                                                      \
    }                                                                                                           \
    FORCENOINLINE void operator delete(void* Ptr, size_t Size, std::align_val_t Alignment,                      \
                                       const std::nothrow_t&) noexcept                                          \
    {                                                                                                           \
        (void)Size;                                                                                             \
        (void)Alignment;                                                                                        \
        FMemory_Free(Ptr);                                                                                      \
    }                                                                                                           \
    FORCENOINLINE void operator delete[](void* Ptr, size_t Size, std::align_val_t Alignment,                    \
                                         const std::nothrow_t&) noexcept                                        \
    {                                                                                                           \
        (void)Size;                                                                                             \
        (void)Alignment;                                                                                        \
        FMemory_Free(Ptr);                                                                                      \
    }

#define MODULE_BOILERPLATE UE_DEFINE_FMEMORY_WRAPPERS REPLACEMENT_OPERATOR_NEW_AND_DELETE

#define IMPLEMENT_MODULE(ModuleClass)                      \
    extern "C" ENGINE_API IModuleInterface* CreateModule() \
    {                                                      \
        return new ModuleClass();                          \
    }                                                      \
    MODULE_BOILERPLATE
