#pragma once

#include "Engine/Core/RTTI/FunctionTraits.hxx"

#include "ECS/World.hxx"

namespace ecs
{

namespace details
{

    template <typename T>
    using cleaned_component = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

    template <typename T>
    struct wrapper : public wrapper<typename ::RTTI::FunctionTraits<T>::args_type>
    {
    };

    template <typename... Components>
    struct wrapper<std::tuple<Components...>>
    {
        static constexpr auto wrap_system(auto system)
        {
            return [system](ecs::RWorld* World)
            {
                TComponentArray components_map =
                    World->GetComponentStorage().JoinComponents<cleaned_component<Components>...>();

                for (auto& [Entity, components]: components_map)
                    std::apply(system, components);
            };
        }

        template <typename TClass>
        static constexpr auto wrap_system(TClass* InContext, auto system)
        {
            return [InContext, system](ecs::RWorld* World)
            {
                TComponentArray components_map =
                    World->GetComponentStorage().JoinComponents<cleaned_component<Components>...>();

                for (auto& [Entity, components]: components_map)
                {
                    std::apply(std::bind_front(system, InContext), components);
                }
            };
        }
    };

}    // namespace details

template <typename Fn>
concept IsSystemFunction =
    std::is_same_v<typename ::RTTI::FunctionTraits<Fn>::result_type, void> && ::RTTI::FunctionTraits<Fn>::arity > 0;

class FSystem
{
public:
    FSystem(std::function<void(RWorld*)> system);

    template <typename Fn>
    requires(IsSystemFunction<Fn>)
    FSystem(Fn system): CallWrapper(details::wrapper<decltype(system)>::wrap_system(system))
    {
    }

    template <typename TClass, typename Fn>
    requires(IsSystemFunction<Fn> && std::is_same_v<typename ::RTTI::FunctionTraits<Fn>::Class, TClass>)
    /// Call context for the system (basically the this pointer for member functions systems)
    FSystem(TClass* InContext, Fn system)
        : CallWrapper(details::wrapper<decltype(system)>::template wrap_system<TClass>(InContext, system))
    {
    }

    FSystem(const FSystem&) = default;
    FSystem(FSystem&&) = default;

    void Call(RWorld*) const;

private:
    std::function<void(RWorld*)> CallWrapper;
};

}    // namespace ecs
