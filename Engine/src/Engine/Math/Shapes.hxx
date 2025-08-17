#pragma once

namespace Math
{
template <typename T>
class TTransform;

template <typename T>
requires(std::is_floating_point_v<T>)
class TBoundingBox
{
public:
    TBoundingBox() = default;
    constexpr TBoundingBox(const TVector<3, T>& InMin, const TVector<3, T>& InMax): Min(InMin), Max(InMax)
    {
        checkMsg(Min.x <= Max.x && Min.y <= Max.y && Min.z <= Max.z,
                 "Invalid AABB: Min ({}, {}, {}) must be less than or equal to Max ({}, {}, {})", Min.x, Min.y, Min.z,
                 Max.x, Max.y, Max.z);
    }

    /// Add a point to the bounding box, expanding it if necessary
    constexpr void AddPoint(const TVector<3, T>& Point)
    {
        Min.x = std::min(Min.x, Point.x);
        Min.y = std::min(Min.y, Point.y);
        Min.z = std::min(Min.z, Point.z);

        Max.x = std::max(Max.x, Point.x);
        Max.y = std::max(Max.y, Point.y);
        Max.z = std::max(Max.z, Point.z);
    }

    /// Return the vertices of the bouding box
    [[nodiscard]] constexpr TArray<TVector<3, T>> GetCorners() const
    {
        return {{Min.x, Min.y, Min.z}, {Max.x, Min.y, Min.z}, {Max.x, Max.y, Min.z}, {Min.x, Max.y, Min.z},
                {Min.x, Min.y, Max.z}, {Max.x, Min.y, Max.z}, {Max.x, Max.y, Max.z}, {Min.x, Max.y, Max.z}};
    }

    /// Return true if the point is inside the bounding box
    constexpr bool IsInside(const TVector<3, T>& Point) const
    {
        return Point.x >= Min.x && Point.x <= Max.x && Point.y >= Min.y && Point.y <= Max.y && Point.z >= Min.z &&
               Point.z <= Max.z;
    }

    /// Return true if the other bounding box is inside this bounding box
    constexpr bool IsInside(const TBoundingBox<T>& Other) const
    {
        return IsInside(Other.Min) && IsInside(Other.Max);
    }

    /// Return true if this bounding box intersects with another bounding box
    constexpr bool Intersects(const TBoundingBox<T>& Other) const
    {
        return !(Min.x > Other.Max.x || Max.x < Other.Min.x || Min.y > Other.Max.y || Max.y < Other.Min.y ||
                 Min.z > Other.Max.z || Max.z < Other.Min.z);
    }

    /// Transform the bounding box by the given Transform
    [[nodiscard]] constexpr TBoundingBox<T> Transform(const TTransform<T>& Transform) const
    {
        TBoundingBox<T> NewBox;
        NewBox.Min = Transform.GetLocation() + Transform.GetRotationMatrix() * Min;
        NewBox.Max = Transform.GetLocation() + Transform.GetRotationMatrix() * Max;

        // Scale the bounding box by the scale factor
        const TVector<3, T> Scale = Transform.GetScale();
        NewBox.Min.x *= Scale.x;
        NewBox.Min.y *= Scale.y;
        NewBox.Min.z *= Scale.z;
        NewBox.Max.x *= Scale.x;
        NewBox.Max.y *= Scale.y;
        NewBox.Max.z *= Scale.z;

        return NewBox;
    }

    bool operator==(const TBoundingBox<T>& Other) const = default;

public:
    TVector<3, T> Min = {0, 0, 0};
    TVector<3, T> Max = {0, 0, 0};
};

template <typename T>
requires(std::is_floating_point_v<T>)
class TSphere
{
public:
    TSphere() = default;
    constexpr TSphere(const TVector<3, T>& InCenter, T InRadius): Center(InCenter), Radius(InRadius)
    {
        checkMsg(Radius >= 0.0f, "Invalid sphere radius: {}. Radius must be non-negative.", Radius);
    }

    constexpr void SetCenter(const TVector<3, T>& InCenter)
    {
        Center = InCenter;
    }
    constexpr void SetRadius(T InRadius)
    {
        checkMsg(InRadius >= 0.0f, "Invalid sphere radius: {}. Radius must be non-negative.", InRadius);
        Radius = InRadius;
    }

    /// Test if a point is inside the sphere
    constexpr bool IsInside(const TVector<3, T>& Point) const
    {
        const T DistanceSquared = Math::SizeSquared(Point - Center);
        return DistanceSquared <= Radius * Radius;
    }
    /// Test if another sphere is inside this sphere
    constexpr bool IsInside(const TSphere<T>& Other) const
    {
        const T DistanceSquared = Math::SizeSquared(Other.Center - Center);
        const T RadiusSum = Radius + Other.Radius;
        return DistanceSquared <= RadiusSum * RadiusSum;
    }

    /// Test if this sphere intersects with another sphere
    constexpr bool Intersects(const TSphere<T>& Other) const
    {
        const T DistanceSquared = Math::SizeSquared(Other.Center - Center);
        const T RadiusSum = std::max(T(0), Radius + Other.Radius);
        return DistanceSquared <= RadiusSum * RadiusSum;
    }

    /// Transform the sphere by the given Transform
    [[nodiscard]] constexpr TSphere<T> Transform(const TTransform<T>& Transform)
    {
        TSphere<T> NewSphere;
        NewSphere.Center = Transform.GetLocation() + Transform.GetRotationMatrix() * Center;
        NewSphere.Radius =
            Radius * std::max(Transform.GetScale().x, std::max(Transform.GetScale().y, Transform.GetScale().z));
        return NewSphere;
    }

    bool operator==(const TSphere<T>& Other) const = default;

public:
    T Radius = 0.0f;
    TVector<3, T> Center = {0, 0, 0};
};

}    // namespace Math

using FBoundingBox = Math::TBoundingBox<float>;
using DBoundingBox = Math::TBoundingBox<double>;
