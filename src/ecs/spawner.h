#pragma once

#include "ai/genetic_algorithm.h"
#include "track/track_generator.h"

#include <engine/ecs/world.h>
#include <engine/resources/assets_db.h>

namespace engine::render {
struct Sprite;
}

namespace game::ecs {
struct Car;
}

namespace game::ecs::spawner {

void setup_camera(engine::ecs::World& world);

void spawn_track(engine::ecs::World& world, engine::AssetsDb& assets, const track::GeneratedTrack& track);

void clear_track(engine::ecs::World& world);

void clear_cars(engine::ecs::World& world);

void spawn_cars(
        engine::ecs::World& world,
        engine::AssetsDb& assets,
        const track::GeneratedTrack& track,
        const ai::GeneticAlgorithm& ga);

void respawn_cars(
        engine::ecs::World& world,
        engine::AssetsDb& assets,
        const track::GeneratedTrack& track,
        const ai::GeneticAlgorithm& ga);

void setup_ray_visualizers(engine::ecs::World& world, engine::AssetsDb& assets);

void kill_car(Car& car, engine::render::Sprite& sprite);

} // namespace game::ecs::spawner
