#pragma once

#include <numbers>

namespace Math
{

DECLARE_LOGGER_CATEGORY(Core, LogMath, Info)

// The math should be done, considering a right-handed coordinate system, with Z pointing up
// The unit of measurement is centimeters
constexpr static FVector3 UpVector = {0, 0, 1};
constexpr static FVector3 FrontVector = {1, 0, 0};
constexpr static FVector3 RightVector = {0, 1, 0};

template <typename T>
constexpr T DegreeToRadian(T Degree)
{
    return Degree * (std::numbers::pi_v<T> / T(180));
}

template <typename T>
constexpr T RadianToDegree(T Radian)
{
    return Radian * (T(180) / std::numbers::pi_v<T>);
}

template <typename T, unsigned Size>
constexpr T Dot(const TVector<Size, T>& lhs, const TVector<Size, T>& rhs)
{
    const TVector<Size, T> ResultTemp(lhs * rhs);

    T Result = 0;
    for (unsigned i = 0; i < Size; i++)
    {
        Result += ResultTemp.data[i];
    }
    return Result;
}

template <typename T, unsigned Size>
constexpr TVector<Size, T> Normalize(const TVector<Size, T>& Vector)
{
    const T Sqrt = sqrt(Dot(Vector, Vector));
    if (Sqrt == T(0)) [[unlikely]]
    {
        LOG(LogMath, Warning, "Trying to normalize a zero-length vector");
        return Vector;
    }
    const T InverseSqrt = T(1) / Sqrt;
    return Vector * InverseSqrt;
}

template <typename T>
constexpr TVector<3, T> Cross(const TVector<3, T>& x, const TVector<3, T>& y)
{
    return TVector<3, T>(x.y * y.z - y.y * x.z, x.z * y.x - y.z * x.x, x.x * y.y - y.x * x.y);
}

template <typename T, unsigned TColumns, unsigned TRows>
constexpr TMatrix<TColumns, TRows, T> Transpose(const TMatrix<TRows, TColumns, T>& Matrix)
{
    TMatrix<TColumns, TRows, T> Result;
    for (unsigned i = 0; i < TRows; ++i)
    {
        for (unsigned j = 0; j < TColumns; ++j)
        {
            Result[j][i] = Matrix[i][j];
        }
    }
    return Result;
}

}    // namespace Math
