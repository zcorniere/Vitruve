#pragma once

DECLARE_LOGGER_CATEGORY(Core, LogECS, Info)

namespace ecs
{

class RWorld;
using FEntity = uint64;

/// Create a new world
[[nodiscard]]
ENGINE_API Ref<RWorld> CreateWorld();

/// Destroy the given world
/// Note: The world may not be deleted right away but it will start the deletion process. The Ref given as argument is
/// expected to be the last one.
ENGINE_API void DestroyWorld(Ref<RWorld>* World);

}    // namespace ecs
