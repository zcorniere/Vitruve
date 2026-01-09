#pragma once

namespace Vitruve
{

class ENGINE_API FUUID
{
    RTTI_DECLARE_TYPEINFO_MINIMAL(FUUID);

public:
    FUUID();
    explicit FUUID(uint64 uuid);
    FUUID(const FUUID& other);

    FORCEINLINE uint64 ID() const
    {
        return m_UUID;
    }

    FORCEINLINE operator uint64() const
    {
        return m_UUID;
    }

private:
    const uint64 m_UUID;
};

}    // namespace Vitruve

FORCEINLINE bool operator==(const Vitruve::FUUID& lhs, const Vitruve::FUUID& rhs)
{
    return lhs.ID() == rhs.ID();
}

namespace std
{

template <>
struct hash<Vitruve::FUUID>
{
    std::size_t operator()(const Vitruve::FUUID& uuid) const
    {
        // uuid is already a randomly generated number, and is suitable as a hash key as-is.
        return uuid;
    }
};

}    // namespace std
