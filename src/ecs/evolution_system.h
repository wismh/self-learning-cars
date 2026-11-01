#pragma once

#include "ai/genetic_algorithm.h"
#include "track/track_generator.h"

#include <engine/ecs/world.h>
#include <engine/resources/assets_db.h>

#include <functional>

namespace game::ecs {

class EvolutionManager {
public:
    EvolutionManager(engine::AssetsDb& assets, std::size_t population_size = 60);

    void reset(engine::ecs::World& world, uint32_t track_seed = 101);
    void next_track(engine::ecs::World& world);
    void mutate_all(float strength = 0.4f);

    void update(engine::ecs::World& world, float dt);

    [[nodiscard]] const track::GeneratedTrack& current_track() const { return current_track_; }
    [[nodiscard]] const ai::GeneticAlgorithm& ga() const { return ga_; }
    [[nodiscard]] ai::GeneticAlgorithm& ga() { return ga_; }

    void on_new_generation(std::function<void(int gen, uint32_t seed)> callback) {
        new_gen_callback_ = std::move(callback);
    }

private:
    void advance_generation(engine::ecs::World& world);

    engine::AssetsDb* assets_;
    ai::GeneticAlgorithm ga_;
    track::GeneratedTrack current_track_{};
    float generation_timer_ = 0.0f;
    std::function<void(int, uint32_t)> new_gen_callback_{};
};

} // namespace game::ecs
