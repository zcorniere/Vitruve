#include "UUID.hxx"

#include <random>

ENGINE_API std::random_device s_RandomDevice;
ENGINE_API std::mt19937_64 eng(s_RandomDevice());
ENGINE_API std::uniform_int_distribution<uint64> s_UniformDistribution;

namespace Vitruve
{

FUUID::FUUID(): m_UUID(s_UniformDistribution(eng))
{
}

FUUID::FUUID(uint64 uuid): m_UUID(uuid)
{
}

FUUID::FUUID(const FUUID& other): m_UUID(other.m_UUID)
{
}

}    // namespace Vitruve
