#include "ecs/evolution_system.h"

#include <asset_ids.h>

#include "ecs/components.h"
#include "ecs/spawner.h"

#include <engine/render/sprite.h>

#include <algorithm>

namespace game::ecs {

EvolutionManager::EvolutionManager(engine::AssetsDb& assets, std::size_t population_size)
    : assets_(&assets)
    , ga_(population_size) {}

void EvolutionManager::reset(engine::ecs::World& world, uint32_t track_seed) {
    auto& config = world.ctx<SimulationConfig>();
    config.track_seed = track_seed;
    config.current_generation = 1;
    config.best_fitness_current_gen = 0.0f;
    config.all_time_best_fitness = 0.0f;
    generation_timer_ = 0.0f;

    ga_.reset(track_seed);

    current_track_ = track::TrackGenerator::generate(config.track_seed);
    spawner::spawn_track(world, *assets_, current_track_);
    spawner::spawn_cars(world, *assets_, current_track_, ga_);

    if (new_gen_callback_) {
        new_gen_callback_(config.current_generation, config.track_seed);
    }
}

void EvolutionManager::next_track(engine::ecs::World& world) {
    auto& config = world.ctx<SimulationConfig>();
    config.track_seed += 1;
    current_track_ = track::TrackGenerator::generate(config.track_seed);
    spawner::spawn_track(world, *assets_, current_track_);
    spawner::respawn_cars(world, *assets_, current_track_, ga_);
    generation_timer_ = 0.0f;

    if (new_gen_callback_) {
        new_gen_callback_(config.current_generation, config.track_seed);
    }
}

void EvolutionManager::mutate_all(float strength) {
    ga_.mutate_all(strength);
}

void EvolutionManager::update(engine::ecs::World& world, float dt) {
    auto& config = world.ctx<SimulationConfig>();
    generation_timer_ += dt;

    int alive_count = 0;
    float max_fitness = -1.0f;
    engine::ecs::Entity leader_entity{};

    auto view = world.view<engine::render::Sprite, Car, Brain>();

    // 1. Find leader and count alive cars
    for (engine::ecs::Entity e : view) {
        auto& car = view.get<Car>(e);
        if (car.is_alive) {
            alive_count++;
            if (car.fitness > max_fitness) {
                max_fitness = car.fitness;
                leader_entity = e;
            }
        }
    }

    config.alive_cars_count = alive_count;
    config.best_fitness_current_gen = std::max(config.best_fitness_current_gen, max_fitness);

    // 2. Update leader status and sprites
    for (engine::ecs::Entity e : view) {
        auto& car = view.get<Car>(e);
        auto& sprite = view.get<engine::render::Sprite>(e);

        if (e == leader_entity && car.is_alive) {
            car.is_leader = true;
            config.leader_speed = car.velocity;
            sprite = assets_->get_sprite(assets::textures::leader_car);
            sprite.order_in_layer = 10;
        } else {
            car.is_leader = false;
            if (car.is_alive) {
                sprite = assets_->get_sprite(assets::textures::car);
                sprite.order_in_layer = 5;
            }
        }
    }

    // 3. Check if generation completed
    // Generation ends when all cars crash (with safety grace period) OR timer reaches 35 seconds
    if ((alive_count == 0 && generation_timer_ > 0.25f) || generation_timer_ >= 35.0f) {
        advance_generation(world);
    }
}

void EvolutionManager::advance_generation(engine::ecs::World& world) {
    auto& config = world.ctx<SimulationConfig>();

    // 1. Record fitness for each genome from corresponding car
    auto view = world.view<Car, Brain>();
    std::size_t idx = 0;
    for (engine::ecs::Entity e : view) {
        if (idx < ga_.population().size()) {
            const auto& car = view.get<Car>(e);
            ga_.set_fitness(idx, car.fitness);
            idx++;
        }
    }

    // 2. Run genetic evolution (selection, elitism, crossover, mutation)
    ga_.evolve();
    config.all_time_best_fitness = ga_.all_time_best_fitness();
    config.current_generation++;

    // 3. PROCEDURAL TRACK ADVANCEMENT:
    // Generate a fresh track every generation so cars learn generalized driving rules!
    config.track_seed += 1;
    current_track_ = track::TrackGenerator::generate(config.track_seed);

    // 4. Rebuild ECS track entities and respawn cars
    spawner::spawn_track(world, *assets_, current_track_);
    spawner::respawn_cars(world, *assets_, current_track_, ga_);
    generation_timer_ = 0.0f;

    if (new_gen_callback_) {
        new_gen_callback_(config.current_generation, config.track_seed);
    }
}

} // namespace game::ecs
