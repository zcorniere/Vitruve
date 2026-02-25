#include "Engine/Vitruve.hxx"

#include "Engine/Core/Log.hxx"
#include "Engine/Core/RTTI/RObject.hxx"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

class RTestObject : public RObject
{
    RTTI_DECLARE_TYPEINFO(RTestObject, RObject)
public:
    RTestObject(int NewI = 0): i(NewI)
    {
    }
    int i = 0;
};

TEST_CASE("RObject - Ref Count")
{
    FLog log;

    Ref<RTestObject> Object;

    REQUIRE(!Object.IsValid());
    REQUIRE(!Object);

    REQUIRE(Object.Raw() == nullptr);
    REQUIRE(Object.As<RObject>() == nullptr);

    Object = Ref<RTestObject>::Create();

    REQUIRE(Object.Raw() != nullptr);
    REQUIRE(Object.As<RObject>() != nullptr);
    REQUIRE(Object->GetRefCount() == 1);

    SECTION("Copy")
    {
        Ref<RTestObject> Copy = Object;

        CHECK(Copy.Raw() == Object.Raw());
        CHECK(Copy.As<RObject>() == Object.As<RObject>());
        CHECK(Copy->GetRefCount() == 2);
        CHECK(Object->GetRefCount() == 2);

        Copy = nullptr;
        CHECK(Object->GetRefCount() == 1);
    }
    SECTION("Weak Ref")
    {
        WeakRef<RTestObject> Copy = Object;

        CHECK(Copy.Raw() == Object.Raw());
        CHECK(Copy->GetRefCount() == 1);
        CHECK(Object->GetRefCount() == 1);

        Copy = nullptr;
        CHECK(Object->GetRefCount() == 1);
    }

    Object = nullptr;
    CHECK(!RObjectUtils::AreThereAnyLiveObject());
}

TEST_CASE("RObject - Names")
{
    FLog log;

    Ref<RTestObject> Object = Ref<RTestObject>::Create();
    REQUIRE(Object->GetRefCount() == 1);
    CHECK(Object->GetName() == "Unnamed");
    CHECK(Object->GetTypeName() == "RTestObject");
    CHECK(Object->GetBaseTypeName() == "RTestObject");

    Object->SetName("TestName");
    CHECK(Object->GetName() == "TestName");
    Object->SetNamef("TestName {}", Object->i);
    CHECK(Object->GetName() == std::format("TestName {}", Object->i));

    SECTION("Type Names")
    {
        Ref<RObject> Copy = Object;

        CHECK(Copy.Raw() == Object.Raw());
        CHECK(Copy.As<RObject>() == Object.As<RObject>());
        CHECK(Copy->GetRefCount() == 2);
        CHECK(Object->GetRefCount() == 2);

        CHECK(Copy->GetTypeName() == "RObject");
        CHECK(Copy->GetBaseTypeName() == "RTestObject");
    }

    SECTION("CreateNamed")
    {
        Ref<RTestObject> Copy = Ref<RTestObject>::CreateNamed("NamedObject", 6);
        CHECK(Copy->GetName() == "NamedObject");
        CHECK(Copy->i == 6);
    }
}

TEST_CASE("RObjectUtils")
{
    FLog log;

    Ref<RTestObject> Object = Ref<RTestObject>::Create();
    RTestObject* StaleRef = Object.Raw();

    CHECK(RObjectUtils::IsLive(Object.Raw()));
    Object = nullptr;
    CHECK(!RObjectUtils::IsLive(StaleRef));
}
