#pragma once

#include "ecs/evolution_system.h"

#include <engine/ecs/world.h>

namespace game::ecs {

void register_simulation_systems(engine::ecs::World& world, EvolutionManager& manager);

} // namespace game::ecs
