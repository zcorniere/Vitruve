#include "Engine/Vitruve.hxx"

#include "Engine/Containers/Tree.hxx"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

struct RandomType
{
    int Value;
};

TEST_CASE("QuadTree Test", "[Containers]")
{
    FQuadtree<RandomType> QuadTree(Math::TRect<float>({0, 0}, {100, 100}), 0);

    RandomType Item1{42};
    RandomType Item2{43};

    QuadTree.Insert({{10, 10}, {20, 20}}, Item1);
    QuadTree.Insert({{15, 15}, {25, 25}}, Item2);

    SECTION("Querying QuadTree")
    {
        auto Results = QuadTree.Query({{10, 10}, {30, 30}});
        CHECK(Results.Size() == 2);
        CHECK(Results[0].Value == 42);
        CHECK(Results[1].Value == 43);
    }

    SECTION("Querying Non-Intersecting Area")
    {
        auto Results = QuadTree.Query({{50, 50}, {60, 60}});
        CHECK(Results.Size() == 0);
    }
}

TEST_CASE("Octree Test", "[Containers]")
{
    FOctree<RandomType> Octree(Math::TBox<float>({0, 0, 0}, {100, 100, 100}), 0);

    RandomType Item1{42};
    RandomType Item2{43};
    Octree.Insert({{10, 10, 10}, {20, 20, 20}}, Item1);
    Octree.Insert({{15, 15, 15}, {25, 25, 25}}, Item2);

    SECTION("Querying Octree")
    {
        auto Results = Octree.Query({{10, 10, 10}, {30, 30, 30}});
        CHECK(Results.Size() == 2);
        CHECK(Results[0].Value == 42);
        CHECK(Results[1].Value == 43);
    }

    SECTION("Querying Non-Intersecting Area")
    {
        auto Results = Octree.Query({{50, 50, 50}, {60, 60, 60}});
        CHECK(Results.Size() == 0);
    }
}
