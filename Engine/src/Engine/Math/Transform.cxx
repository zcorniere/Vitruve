#include "Engine/Math/Transform.hxx"
#include "Engine/Misc/ConsoleVariable.hxx"

namespace Math
{

TConsoleVariable<bool> CVar_EnableSIMD("math.EnableSIMD", true, "If true, use SIMD code path for math computations");

}    // namespace Math
