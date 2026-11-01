#pragma once

#include "track/track_generator.h"

#include <engine/ecs/world.h>

namespace game::ecs {

void update_car_physics(engine::ecs::World& world, const track::GeneratedTrack& track, float dt);

} // namespace game::ecs
