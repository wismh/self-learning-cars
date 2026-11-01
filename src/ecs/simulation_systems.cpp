#include "ecs/simulation_systems.h"

#include "ecs/car_brain_system.h"
#include "ecs/car_physics_system.h"
#include "ecs/car_sensors_system.h"
#include "ecs/components.h"
#include "ecs/ray_render_system.h"

#include <engine/core/time.h>
#include <engine/ecs/schedule.h>

#include <algorithm>

namespace game::ecs {

void register_simulation_systems(engine::ecs::World& world, EvolutionManager& manager) {
    // 1. Fixed Simulation Tick (Physics, Brains, Sensors, Evolution)
    world.add_system(engine::ecs::Schedule::Fixed, engine::ecs::Phase::Game, [&manager](engine::ecs::World& w) {
        auto& config = w.ctx<SimulationConfig>();
        if (config.paused) {
            return;
        }

        const int sub_steps = std::clamp(static_cast<int>(config.time_scale), 1, 10);
        constexpr float kDt = engine::kFixed;

        for (int step = 0; step < sub_steps; ++step) {
            update_car_sensors(w, manager.current_track());
            update_car_brains(w);
            update_car_physics(w, manager.current_track(), kDt);
            manager.update(w, kDt);
        }
    });

    // 2. Frame Update System (Visuals & Raycasts)
    world.add_system(engine::ecs::Schedule::Frame, engine::ecs::Phase::Game, [](engine::ecs::World& w) {
        auto& stats = w.ctx<SimulationStats>();
        if (const auto* time = w.try_ctx<engine::Time>(); time && time->delta_time > 0.0001f) {
            stats.frame_time_ms = time->delta_time * 1000.0f;
            stats.fps = 1.0f / time->delta_time;
        }

        update_ray_visuals(w);
    });
}

} // namespace game::ecs
