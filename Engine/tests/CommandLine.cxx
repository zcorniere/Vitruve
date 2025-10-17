#include "Engine/Vitruve.hxx"

#include "Engine/Misc/CommandLine.hxx"
#include "Engine/Misc/Delegate.hxx"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

TEST_CASE("Sanity test : Simple command line")
{
    FCommandLine::Set("Test");
    REQUIRE(strcmp(FCommandLine::Get(), "Test") == 0);
    FCommandLine::Reset();
}

TEST_CASE("Command line with multiple argument")
{
    FCommandLine::Set("-intarg=42 -qargument=\"Test Value Argument\" -argument=TestValue Argument");

    SECTION("Command line with quote argument")
    {
        std::string value;
        REQUIRE(FCommandLine::Parse("-qargument=", value));
        REQUIRE(value == "Test Value Argument");
    }
    SECTION("Command line without quote argument")
    {
        std::string value;
        REQUIRE(FCommandLine::Parse("-argument=", value));
        REQUIRE(value == "TestValue");
    }
    SECTION("Command line with int argument")
    {
        int value;
        REQUIRE(FCommandLine::Parse("-intarg=", value));
        REQUIRE(value == 42);
    }
    FCommandLine::Reset();
}

TEST_CASE("Delegates")
{
    TDelegate<void(int)> Delegate;

    SECTION("Simple lambda")
    {
        int Called = false;
        Delegate.Add(
            [&Called](int Value)
            {
                Called += 1;
                REQUIRE(Value == 42);
            });
        Delegate.Add(
            [&Called](int Value)
            {
                Called += 1;
                REQUIRE(Value == 42);
            });

        Delegate.Broadcast(42);
        REQUIRE(Called == 2);
    }

    SECTION("Member function")
    {
        class Test
        {
        public:
            Test(int InNumber): Number(InNumber)
            {
            }

            void DelegateCallBack(int Value)
            {
                bCalled = true;
                REQUIRE(Number == 63);
                Number = Value;
            }

        public:
            int Number = 0;
            bool bCalled = false;
        };
        Test Instance(63);

        int Value2 = 3;
        Delegate.Add(&Instance, &Test::DelegateCallBack);
        Delegate.Add(
            [&Value2](int Value)
            {
                REQUIRE(Value == 90);
                Value2 = Value;
            });
        REQUIRE(!Instance.bCalled);
        Delegate.Broadcast(90);

        REQUIRE(Instance.bCalled);
        REQUIRE(Instance.Number == 90);
        REQUIRE(Value2 == 90);
    }
}
