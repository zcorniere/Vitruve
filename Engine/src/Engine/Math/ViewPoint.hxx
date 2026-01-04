#pragma once

#include "Engine/Math/Math.hxx"
#include "Engine/Math/Transform.hxx"

namespace Math
{

template <typename T>
class TViewPoint
{
public:
    TViewPoint() = default;

    /// @brief Construct a new ViewPoint object
    /// @param FOV The field of view in degrees
    /// @param Near The near plane
    /// @param Far The far plane
    /// @param AspectRatio The aspect ratio of the screen
    constexpr TViewPoint(T FOV, T Near, T Far, T AspectRatio = T(0))
        : m_FOV(DegreeToRadian(FOV))
        , m_AspectRatio(AspectRatio)
        , m_Near(Near)
        , m_Far(Far)
        , bProjectionMatrixDirty(true)
    {
    }

    constexpr void SetFOV(T FOV)
    {
        const T NewFOV = DegreeToRadian(FOV);
        if (m_FOV == NewFOV)
        {
            return;
        }
        m_FOV = NewFOV;
        bProjectionMatrixDirty = true;
    }
    constexpr T GetFOV() const
    {
        return m_FOV;
    }
    constexpr void SetAspectRatio(T AspectRatio)
    {
        if (m_AspectRatio == AspectRatio)
        {
            return;
        }
        m_AspectRatio = AspectRatio;
        bProjectionMatrixDirty = true;
    }
    constexpr T GetAspectRatio() const
    {
        return m_AspectRatio;
    }
    constexpr void SetNear(T Near)
    {
        if (m_Near == Near)
        {
            return;
        }
        m_Near = Near;
        bProjectionMatrixDirty = true;
    }
    constexpr T GetNear() const
    {
        return m_Near;
    }
    constexpr void SetFar(T Far)
    {
        if (m_Far == Far)
        {
            return;
        }
        m_Far = Far;
        bProjectionMatrixDirty = true;
    }
    constexpr T GetFar() const
    {
        return m_Far;
    }

    constexpr TMatrix4<T> GetProjectionMatrix()
    {
        if (bProjectionMatrixDirty)
        {
            ComputeProjectionMatrix();
            bProjectionMatrixDirty = false;
        }
        return m_ProjectionMatrix;
    }

    constexpr TMatrix4<T> GetViewMatrix(const TTransform<T>& InTransform)
    {
        if (bViewMatrixDirty)
        {
            ComputeViewMatrix(InTransform);
            bViewMatrixDirty = false;
        }
        return m_ViewMatrix;
    }

private:
    constexpr void ComputeProjectionMatrix();
    constexpr void ComputeViewMatrix(const TTransform<T>& InTransform);

private:
    /// @brief The field of view in radians
    T m_FOV = 80;

    T m_AspectRatio = 0;
    T m_Near = 0.001;
    T m_Far = std::numeric_limits<T>::max();

    bool bProjectionMatrixDirty = true;
    TMatrix4<T> m_ProjectionMatrix;

    bool bViewMatrixDirty = true;
    TMatrix4<T> m_ViewMatrix;

    TTransform<T> Transform;
};

template <typename T>
requires std::is_floating_point_v<T>
void CheckNan(const TViewPoint<T>& Value);

}    // namespace Math

using FViewPoint = Math::TViewPoint<float>;
using DViewPoint = Math::TViewPoint<double>;

#include "ViewPoint.inl"
