#include "Engine/Math/Transform.hxx"
#include "Engine/Misc/ConsoleVariable.hxx"

namespace Math
{

TConsoleVariable<bool> CVar_EnableSIMD("math.EnableSIMD", true, "If true, enable SIMD code path for math coputations.");

}    // namespace Math
