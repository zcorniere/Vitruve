#include "Engine/Vitruve.hxx"

#include "Engine/Math/Shapes.hxx"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "Common.hxx"
#include "Engine/Math/Transform.hxx"

TEMPLATE_TEST_CASE("Bouding Box Tests", "[Math]", float, double)
{

    const TestType Epsilon = TEpsilon<TestType>::Value;

    Math::TBoundingBox<TestType> Box(
        Math::TVector<3, TestType>{GENERATE(take(1, random(-50.0f, 0.0f))), GENERATE(take(1, random(-50.0f, 0.0f))),
                                   GENERATE(take(1, random(-50.0f, 0.0f)))},
        Math::TVector<3, TestType>{GENERATE(take(1, random(0.0f, 50.0f))), GENERATE(take(1, random(0.0f, 50.0f))),
                                   GENERATE(take(1, random(0.0f, 50.0f)))});

    SECTION("Bounding Box Copy")
    {
        Math::TBoundingBox<TestType> CopyBox = Box;
        REQUIRE(Box == CopyBox);
    }

    SECTION("Bouding Box - AddPoint")
    {
        Math::TBoundingBox<TestType> BodyBox;

        BodyBox.AddPoint({Box.Max.x, Box.Max.y, Box.Max.z});
        BodyBox.AddPoint({Box.Min.x, Box.Min.y, Box.Min.z});
        BodyBox.AddPoint({Box.Min.x, Box.Max.y, Box.Max.z});
        BodyBox.AddPoint({Box.Max.x, Box.Min.y, Box.Min.z});
        BodyBox.AddPoint({Box.Max.x, Box.Min.y, Box.Max.z});
        BodyBox.AddPoint({Box.Min.x, Box.Max.y, Box.Min.z});

        REQUIRE(Box == BodyBox);
    }

    SECTION("Bounding Box - GetCorners")
    {
        const TArray<Math::TVector<3, TestType>> Corners = Box.GetCorners();
        REQUIRE(Corners.Size() == 8);

        REQUIRE(Corners[0] == Math::TVector<3, TestType>{Box.Min.x, Box.Min.y, Box.Min.z});
        REQUIRE(Corners[1] == Math::TVector<3, TestType>{Box.Max.x, Box.Min.y, Box.Min.z});
        REQUIRE(Corners[2] == Math::TVector<3, TestType>{Box.Max.x, Box.Max.y, Box.Min.z});
        REQUIRE(Corners[3] == Math::TVector<3, TestType>{Box.Min.x, Box.Max.y, Box.Min.z});
        REQUIRE(Corners[4] == Math::TVector<3, TestType>{Box.Min.x, Box.Min.y, Box.Max.z});
        REQUIRE(Corners[5] == Math::TVector<3, TestType>{Box.Max.x, Box.Min.y, Box.Max.z});
        REQUIRE(Corners[6] == Math::TVector<3, TestType>{Box.Max.x, Box.Max.y, Box.Max.z});
        REQUIRE(Corners[7] == Math::TVector<3, TestType>{Box.Min.x, Box.Max.y, Box.Max.z});
    }

    SECTION("Bouding Box - Intersections")
    {
        const Math::TBoundingBox<TestType> OtherBox(Math::TVector<3, TestType>{Box.Min.x, Box.Min.y, Box.Min.z},
                                                    Math::TVector<3, TestType>{Box.Max.x, Box.Max.x, Box.Max.x});

        REQUIRE(Box.Intersects(OtherBox));
    }
}

TEMPLATE_TEST_CASE("Sphere Test", "[Math]", float, double)
{
    using namespace Math;

    const TestType Epsilon = TEpsilon<TestType>::Value;

    TSphere<TestType> Sphere(TVector<3, TestType>{GENERATE(take(1, random(-50.0f, 50.0f))),
                                                  GENERATE(take(1, random(-50.0f, 50.0f))),
                                                  GENERATE(take(1, random(-50.0f, 50.0f)))},
                             GENERATE(take(1, random(12.f, 50.0f))));

    SECTION("Sphere Copy")
    {
        TSphere<TestType> CopySphere = Sphere;
        REQUIRE(Sphere == CopySphere);
    }

    SECTION("Sphere - SetCenter")
    {
        const TVector<3, TestType> NewCenter{GENERATE(take(1, random(-50.0f, 50.0f))),
                                             GENERATE(take(1, random(-50.0f, 50.0f))),
                                             GENERATE(take(1, random(-50.0f, 50.0f)))};
        Sphere.SetCenter(NewCenter);
        REQUIRE(Sphere.Center == NewCenter);
    }

    SECTION("Sphere - IsInside Point")
    {
        // Since Radius is random between 12 and 50, center + 4 should be inside the sphere
        const TVector<3, TestType> InsidePoint{Sphere.Center.x + 4, Sphere.Center.y + 4, Sphere.Center.z + 4};
        REQUIRE(Sphere.IsInside(InsidePoint));
    }

    SECTION("Sphere - IsInside Sphere")
    {
        const TSphere<TestType> OtherSphere(
            Math::TVector<3, TestType>{Sphere.Center.x, Sphere.Center.y, Sphere.Center.z - 10}, Sphere.Radius);

        REQUIRE(Sphere.IsInside(OtherSphere));
    }
}
