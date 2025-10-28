#pragma once
#include "Engine/Misc/ConsoleVariable.hxx"
#include "Transform.hxx"

#include <immintrin.h>

namespace Math
{

template <typename T>
constexpr TTransform<T>::TTransform(const TVector3<T>& InLocation, const TQuaternion<T>& InRotation,
                                    const TVector3<T>& InScale)
    : Location(InLocation)
    , Rotation(InRotation)
    , Scale(InScale)
{
}

template <typename T>
constexpr void TTransform<T>::SetLocation(const TVector3<T>& InLocation)
{
    Math::CheckNaN(InLocation);
    Location = InLocation;
    bModelMatrixDirty = true;
}

template <typename T>
constexpr void TTransform<T>::SetRotation(const TQuaternion<T>& InRotation)
{
    Math::CheckNaN(InRotation);
    Rotation = InRotation;
    bModelMatrixDirty = true;
}

template <typename T>
constexpr void TTransform<T>::SetScale(const TVector3<T>& InScale)
{
    Math::CheckNaN(InScale);
    Scale = InScale;
    bModelMatrixDirty = true;
}

template <typename T>
constexpr const TVector3<T>& TTransform<T>::GetLocation() const
{
    return Location;
}

template <typename T>
constexpr const TQuaternion<T>& TTransform<T>::GetRotation() const
{
    return Rotation;
}

template <typename T>
constexpr const TVector3<T>& TTransform<T>::GetScale() const
{
    return Scale;
}

template <typename T>
constexpr TVector3<T>& TTransform<T>::GetLocation()
{
    return Location;
}

template <typename T>
constexpr TQuaternion<T>& TTransform<T>::GetRotation()
{
    return Rotation;
}

template <typename T>
constexpr TVector3<T>& TTransform<T>::GetScale()
{
    return Scale;
}

template <typename T>
constexpr TMatrix4<T> TTransform<T>::GetTranslationMatrix() const
{
    TMatrix4<T> TranslationMatrix = TMatrix4<T>::Identity();

    TranslationMatrix[3, 0] = Location.x;
    TranslationMatrix[3, 1] = Location.y;
    TranslationMatrix[3, 2] = Location.z;
    return TranslationMatrix;
}

template <typename T>
constexpr TMatrix4<T> TTransform<T>::GetRotationMatrix() const
{
    return Rotation.GetRotationMatrix();
}

template <typename T>
constexpr TMatrix4<T> TTransform<T>::GetScaleMatrix() const
{
    TMatrix4<T> ScaleMatrix = TMatrix4<T>::Identity();
    ScaleMatrix[0, 0] = Scale.x;
    ScaleMatrix[1, 1] = Scale.y;
    ScaleMatrix[2, 2] = Scale.z;
    return ScaleMatrix;
}

template <typename T>
constexpr TMatrix4<T> TTransform<T>::GetModelMatrix()
{
    if (!bModelMatrixDirty)
    {
        return ModelMatrix;
    }
    else
    {
        VIT_PROFILE_FUNC()
        ModelMatrix = GetTranslationMatrix() * Rotation.GetRotationMatrix() * GetScaleMatrix();

        Math::CheckNaN(ModelMatrix);
        bModelMatrixDirty = false;
    }
    return ModelMatrix;
}

extern TConsoleVariable<bool> CVar_EnableSIMD;

template <typename T>
void ENGINE_API ComputeModelMatrixBatch(const size_t Count, const T* RESTRICT PositionX, const T* RESTRICT PositionY,
                                        const T* RESTRICT PositionZ, const T* RESTRICT QuaternionX,
                                        const T* RESTRICT QuaternionY, const T* RESTRICT QuaternionZ,
                                        const T* RESTRICT QuaternionW, const T* RESTRICT ScaleX,
                                        const T* RESTRICT ScaleY, const T* RESTRICT ScaleZ,
                                        TMatrix4<T>* RESTRICT OutModelMatrix)
{
    VIT_PROFILE_FUNC()
    check(Count > 0);
    ensure(PositionX && PositionY && PositionZ && QuaternionX && QuaternionY && QuaternionZ && QuaternionW && ScaleX &&
           ScaleY && ScaleZ && OutModelMatrix);

    size_t WorkedCount = 0;
    const size_t AVX512BlockSize = sizeof(__m512) / sizeof(T);
    const size_t AVX2BlockSize = sizeof(__m256) / sizeof(T);
    const FCPUInformation& CPUInfo = FPlatformMisc::GetCPUInformation();

    if (CPUInfo.AVX512 && CVar_EnableSIMD.GetValue() && Count >= AVX512BlockSize)
    {
        const size_t BatchCount = (Count / AVX512BlockSize) * AVX512BlockSize;    // largest multiple of 16 <= Count
        if (BatchCount > 0)
        {
            WorkedCount += ComputeModelMatrixBatch_AVX512(
                BatchCount, PositionX + WorkedCount, PositionY + WorkedCount, PositionZ + WorkedCount,
                QuaternionX + WorkedCount, QuaternionY + WorkedCount, QuaternionZ + WorkedCount,
                QuaternionW + WorkedCount, ScaleX + WorkedCount, ScaleY + WorkedCount, ScaleZ + WorkedCount,
                reinterpret_cast<T*>(OutModelMatrix + WorkedCount));
        }
    }

    if (CPUInfo.AVX2 && CVar_EnableSIMD.GetValue() && Count >= AVX2BlockSize)
    {
        const size_t Remaining = Count - WorkedCount;
        const size_t BatchCount = (Remaining / AVX2BlockSize) * AVX2BlockSize;
        if (BatchCount > 0)
        {
            WorkedCount += ComputeModelMatrixBatch_AVX2(
                BatchCount, PositionX + WorkedCount, PositionY + WorkedCount, PositionZ + WorkedCount,
                QuaternionX + WorkedCount, QuaternionY + WorkedCount, QuaternionZ + WorkedCount,
                QuaternionW + WorkedCount, ScaleX + WorkedCount, ScaleY + WorkedCount, ScaleZ + WorkedCount,
                reinterpret_cast<T*>(OutModelMatrix + WorkedCount));
        }
    }

    // Fallback to scalar implementation
    for (; WorkedCount < Count; WorkedCount++)
    {
        const TVector3<T> Location(PositionX[WorkedCount], PositionY[WorkedCount], PositionZ[WorkedCount]);
        const TQuaternion<T> Rotation(QuaternionW[WorkedCount], QuaternionX[WorkedCount], QuaternionY[WorkedCount],
                                      QuaternionZ[WorkedCount]);
        const TVector3<T> Scale(ScaleX[WorkedCount], ScaleY[WorkedCount], ScaleZ[WorkedCount]);
        TTransform<T> Transform(Location, Rotation, Scale);

        OutModelMatrix[WorkedCount] = Transform.GetModelMatrix();
    }
}

}    // namespace Math
