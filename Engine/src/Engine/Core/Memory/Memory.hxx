#pragma once

namespace Memory
{
ENGINE_API void* Malloc(uint32 Size, uint32 Alignment = 0);
ENGINE_API void* Realloc(void* Original, uint32 Size, uint32 Alignment = 0);

ENGINE_API void Free(void* Ptr);

ENGINE_API bool GetAllocationSize(void* Ptr, uint32& OutSize);
ENGINE_API const char* GetAllocatorName();
};    // namespace Memory

/// Allocator Interface
class IMallocInterface
{
public:
    virtual void* Alloc(uint32 Size, uint32 Alignment = 0) = 0;
    virtual void* Realloc(void* Original, uint32 Size, uint32 Alignment = 0) = 0;

    virtual void Free(void* Ptr) = 0;

    virtual bool GetAllocationSize(void* Ptr, uint32& OutSize) = 0;
    virtual const char* GetAllocatorName() const = 0;

    virtual bool SupportPoison() const = 0;
};

namespace Vitruve
{

/// @brief STL-compliant allocator
/// @tparam T The type of the allocated memory
template <typename T>
class FAllocator
{
public:
    using value_type = T;

    constexpr FAllocator() = default;
    constexpr ~FAllocator() = default;
    constexpr FAllocator(const FAllocator&) noexcept = default;
    constexpr FAllocator(FAllocator&&) noexcept = default;

    template <class U>
    constexpr FAllocator(const FAllocator<U>&) noexcept
    {
    }

    /// @brief Return new allocated memory location of (sizeof(T) * Size)
    /// @param Size The amount of the T we want to allocate
    /// @param hint (unused)
    /// @return return a valid memory location
    T* allocate(std::size_t Size, const void* hint = nullptr)
    {
        (void)hint;
        if (Size == 0)
            return nullptr;
        assert(Size <= std::numeric_limits<std::size_t>::max() / sizeof(T));

        if (T* p = static_cast<T*>(Memory::Malloc(Size * sizeof(T))))
        {
            return p;
        }

        assert(false && "FAllocator: Failed to allocate memory");
    }

    /// @brief Deallocate the given memory pointer
    /// @param pPointer the memory location to release
    /// @param n (unused)
    void deallocate(T* pPointer, std::size_t n = 0) noexcept
    {
        (void)n;
        if (pPointer)
            Memory::Free(pPointer);
    }

    bool operator==(const FAllocator&) const
    {
        return true;
    }
};

}    // namespace Vitruve

void* operator new(std::size_t n);
void* operator new(std::size_t n, const std::nothrow_t& tag) noexcept;
void* operator new(std::size_t n, std::align_val_t alignment);
void* operator new(std::size_t n, std::align_val_t alignment, const std::nothrow_t& tag) noexcept;

void* operator new[](std::size_t n);
void* operator new[](std::size_t n, const std::nothrow_t& tag) noexcept;
void* operator new[](std::size_t n, std::align_val_t alignment);
void* operator new[](std::size_t n, std::align_val_t alignment, const std::nothrow_t& tag) noexcept;

void operator delete(void* p) noexcept;
void operator delete(void* p, std::size_t n) noexcept;
void operator delete(void* p, std::size_t n, std::align_val_t alignment) noexcept;

void operator delete[](void* p) noexcept;
void operator delete[](void* p, std::size_t n) noexcept;
void operator delete[](void* p, std::size_t n, std::align_val_t alignment) noexcept;
