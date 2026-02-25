#include "Engine/Vitruve.hxx"

#include "Engine/Misc/Delegate.hxx"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

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
