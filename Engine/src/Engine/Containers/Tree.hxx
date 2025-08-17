#pragma once

#include "Engine/Containers/Array.hxx"

#include "Engine/Math/Shapes.hxx"
#include "Engine/Math/Vector.hxx"

template <typename T, unsigned TDimension>
struct TAreaType;

template <typename T>
struct TAreaType<T, 3>
{
    using Type = Math::TBox<T>;
    using CoordinateType = Math::TVector<3, T>;
    static constexpr unsigned ChildrenCount = 8;
};

template <typename T>
struct TAreaType<T, 2>
{
    using Type = Math::TRect<T>;
    using CoordinateType = Math::TVector<2, T>;
    static constexpr unsigned ChildrenCount = 4;
};

static constexpr unsigned MaxTreeDepth = 32;

template <unsigned TDimension, typename TPrecision, typename T>
requires(std::is_floating_point_v<TPrecision>)
class TTree
{
private:
    using AreaType = typename TAreaType<TPrecision, TDimension>::Type;
    using CoordinateType = typename TAreaType<TPrecision, TDimension>::CoordinateType;
    static constexpr unsigned ChildrenCount = TAreaType<TPrecision, TDimension>::ChildrenCount;

public:
    TTree() = default;
    TTree(const AreaType& InArea, const uint32 InDepth): Depth(InDepth), Area(InArea)
    {
        Resize(InArea);
    }
    TTree(const TTree&) = delete;

    void Resize(const AreaType& InArea)
    {
        Area = InArea;

        Resize_Internal();
    }

    void Empty()
    {
        Items.Empty();
        for (std::shared_ptr<TTree>& Child: Children)
        {
            if (Child)
            {
                Child->Empty();
            }
            Child.reset();
        }
    }

    uint32 Size() const
    {
        uint32 Count = Items.Size();
        for (const std::shared_ptr<TTree>& Child: Children)
        {
            if (Child)
            {
                Count += Child->Size();
            }
        }
        return Count;
    }

    void Insert(const AreaType& ItemArea, const T& Item)
    {
        for (unsigned i = 0; i < ChildrenCount; i++)
        {
            if (ChildrenAreas[i].IsInside(ItemArea))
            {
                if (Depth + 1 < MaxTreeDepth)
                {
                    if (Children[i] == nullptr)
                    {
                        Children[i] = std::make_shared<TTree>(ChildrenAreas[i], Depth + 1);
                    }
                    Children[i]->Insert(ItemArea, Item);
                    return;
                }
            }
        }

        Items.Emplace(ItemArea, Item);
    }

    TArray<T> Query(const AreaType& QueryArea) const
    {
        TArray<T> Result;
        Query(QueryArea, Result);
        return Result;
    }

    void Query(const AreaType& QueryArea, TArray<T>& OutResults) const
    {
        if (!Area.Intersects(QueryArea))
        {
            return;    // No intersection, return empty result
        }

        // Check items in this node
        for (const auto& ItemPair: Items)
        {
            if (QueryArea.Intersects(ItemPair.template Get<0>()))
            {
                OutResults.Add(ItemPair.template Get<1>());
            }
        }

        // Check children nodes
        for (unsigned i = 0; i < ChildrenCount; i++)
        {
            if (Children[i] != nullptr)
            {
                if (QueryArea.IsInside(ChildrenAreas[i]))
                {
                    Children[i]->CollectItems(OutResults);
                }
                else if (QueryArea.Intersects(ChildrenAreas[i]))
                {
                    Children[i]->Query(QueryArea, OutResults);
                }
            }
        }
    }

    void CollectItems(TArray<T>& OutItems) const
    {
        for (const auto& ItemPair: Items)
        {
            OutItems.Add(ItemPair.template Get<1>());
        }
        for (const std::shared_ptr<TTree>& Child: Children)
        {
            if (Child)
            {
                Child->CollectItems(OutItems);
            }
        }
    }

    AreaType GetArea() const
    {
        return Area;
    }

private:
    void Resize_Internal()
    requires(TDimension == 3)
    {
        static_assert(ChildrenCount == 8, "ChildrenCount must be 8 for 3D trees");

        const CoordinateType ChildSize = (Area.Max - Area.Min) / 2.f;
        ChildrenAreas[0] = {Area.Min, Area.Min + ChildSize};
        ChildrenAreas[1] = {Area.Min + CoordinateType{ChildSize.x, 0, 0},
                            Area.Min + CoordinateType{ChildSize.x, ChildSize.y, ChildSize.z}};
        ChildrenAreas[2] = {Area.Min + CoordinateType{0, ChildSize.y, 0},
                            Area.Min + CoordinateType{ChildSize.x, ChildSize.y, ChildSize.z}};
        ChildrenAreas[3] = {Area.Min + CoordinateType{ChildSize.x, ChildSize.y, 0},
                            Area.Min + CoordinateType{ChildSize.x, ChildSize.y, ChildSize.z}};
        ChildrenAreas[4] = {Area.Min + CoordinateType{0, 0, ChildSize.z},
                            Area.Min + CoordinateType{ChildSize.x, ChildSize.y, ChildSize.z}};
        ChildrenAreas[5] = {Area.Min + CoordinateType{ChildSize.x, 0, ChildSize.z},
                            Area.Min + CoordinateType{ChildSize.x, ChildSize.y, ChildSize.z}};
        ChildrenAreas[6] = {Area.Min + CoordinateType{0, ChildSize.y, ChildSize.z},
                            Area.Min + CoordinateType{ChildSize.x, ChildSize.y, ChildSize.z}};
        ChildrenAreas[7] = {Area.Min + CoordinateType{ChildSize.x, ChildSize.y, ChildSize.z}, Area.Max};
    }
    void Resize_Internal()
    requires(TDimension == 2)
    {
        static_assert(ChildrenCount == 4, "ChildrenCount must be 4 for 2D trees");

        const CoordinateType ChildSize = (Area.Max - Area.Min) / 2.f;
        ChildrenAreas[0] = {Area.Min, Area.Min + ChildSize};
        ChildrenAreas[1] = {Area.Min + CoordinateType{ChildSize.x, 0},
                            Area.Min + CoordinateType{ChildSize.x, ChildSize.y}};
        ChildrenAreas[2] = {Area.Min + CoordinateType{0, ChildSize.y},
                            Area.Min + CoordinateType{ChildSize.x, ChildSize.y}};
        ChildrenAreas[3] = {Area.Min + CoordinateType{ChildSize.x, ChildSize.y}, Area.Max};
    }

public:
    uint32 Depth = 0;

    AreaType Area = {};
    AreaType ChildrenAreas[ChildrenCount]{};
    std::shared_ptr<TTree> Children[ChildrenCount]{};

    TArray<TPair<AreaType, T>> Items{};
};

template <typename T>
using FOctree = TTree<3, float, T>;
template <typename T>
using DOctree = TTree<3, double, T>;

template <typename T>
using FQuadtree = TTree<2, float, T>;
template <typename T>
using DQuadtree = TTree<2, double, T>;
