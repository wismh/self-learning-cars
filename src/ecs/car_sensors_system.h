#pragma once

#include "track/track_generator.h"

#include <engine/ecs/world.h>

namespace game::ecs {

void update_car_sensors(engine::ecs::World& world, const track::GeneratedTrack& track);

} // namespace game::ecs
