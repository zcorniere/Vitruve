#pragma once

#include "Engine/Math/Matrix.hxx"
#include "Engine/Math/Quaternion.hxx"
#include "Engine/Math/Vector.hxx"

namespace Math
{

template <typename T>
class TTransform
{
    RTTI_DECLARE_TYPEINFO_MINIMAL(TTransform)
public:
    TTransform() = default;
    constexpr TTransform(const TVector3<T>& InLocation, const TQuaternion<T>& InRotation, const TVector3<T>& InScale);

    constexpr void SetLocation(const TVector3<T>& InLocation);
    constexpr void SetRotation(const TQuaternion<T>& InRotation);
    constexpr void SetScale(const TVector3<T>& InScale);

    constexpr const TVector3<T>& GetLocation() const;
    constexpr const TQuaternion<T>& GetRotation() const;
    constexpr const TVector3<T>& GetScale() const;

    constexpr TVector3<T>& GetLocation();
    constexpr TQuaternion<T>& GetRotation();
    constexpr TVector3<T>& GetScale();

    constexpr TMatrix4<T> GetModelMatrix();

    constexpr TMatrix4<T> GetTranslationMatrix() const;
    constexpr TMatrix4<T> GetRotationMatrix() const;
    constexpr TMatrix4<T> GetScaleMatrix() const;

public:
    TVector3<T> Location = {0, 0, 0};
    TQuaternion<T> Rotation = {0, 0, 0, 1};
    TVector3<T> Scale = {1, 1, 1};

    bool bModelMatrixDirty = true;
    TMatrix4<T> ModelMatrix;
};

template <typename T>
void ENGINE_API ComputeModelMatrixBatch(const size_t Count, const T* LocationX, const T* LocationY, const T* LocationZ,
                                        const T* QuaternionX, const T* QuaternionY, const T* QuaternionZ,
                                        const T* QuaternionW, const T* ScaleX, const T* ScaleY, const T* ScaleZ,
                                        TMatrix4<T>* OutModelMatrix);

// Do not call these directly, they are only exposed for unit testing
template <typename T>
size_t ENGINE_API ComputeModelMatrixBatch_AVX512(size_t Count, const T* RESTRICT PositionX, const T* RESTRICT PositionY,
                                                 const T* RESTRICT PositionZ, const T* RESTRICT QuaternionX,
                                                 const T* RESTRICT QuaternionY, const T* RESTRICT QuaternionZ,
                                                 const T* RESTRICT QuaternionW, const T* RESTRICT ScaleX,
                                                 const T* RESTRICT ScaleY, const T* RESTRICT ScaleZ,
                                                 T* RESTRICT OutModelMatrix);

template <typename T>
size_t ENGINE_API ComputeModelMatrixBatch_AVX2(size_t Count, const T* RESTRICT PositionX, const T* RESTRICT PositionY,
                                               const T* RESTRICT PositionZ, const T* RESTRICT QuaternionX,
                                               const T* RESTRICT QuaternionY, const T* RESTRICT QuaternionZ,
                                               const T* RESTRICT QuaternionW, const T* RESTRICT ScaleX,
                                               const T* RESTRICT ScaleY, const T* RESTRICT ScaleZ,
                                               T* RESTRICT OutModelMatrix);

}    // namespace Math

template <typename T>
using TTransform = Math::TTransform<T>;

using FTransform = Math::TTransform<float>;
using DTransform = Math::TTransform<double>;

#include "Transform.inl"
