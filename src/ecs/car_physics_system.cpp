#include "ecs/car_physics_system.h"

#include "ecs/components.h"
#include "ecs/spawner.h"

#include <engine/ecs/transform.h>
#include <engine/render/sprite.h>

#include <algorithm>
#include <cmath>

namespace game::ecs {

void update_car_physics(engine::ecs::World& world, const track::GeneratedTrack& track, float dt) {
    if (track.checkpoints.empty()) {
        return;
    }

    auto view = world.view<engine::Transform, engine::render::Sprite, Car>();
    const int cp_count = static_cast<int>(track.checkpoints.size());

    for (engine::ecs::Entity e : view) {
        auto& car = view.get<Car>(e);
        if (!car.is_alive) {
            continue;
        }

        auto& tf = view.get<engine::Transform>(e);

        // 1. Longitudinal acceleration & friction
        if (car.throttle > 0.0f) {
            car.velocity += car.throttle * car.acceleration * dt;
        } else if (car.throttle < 0.0f) {
            car.velocity += car.throttle * (car.acceleration * 0.75f) * dt; // Braking / reverse
        }
        // Rolling resistance friction
        car.velocity -= car.velocity * car.friction * dt;
        car.velocity = std::clamp(car.velocity, -60.0f, car.max_speed);

        // 2. Turning dynamics
        // Turn rate scales with forward speed ratio (cannot rotate purely in place while motionless)
        const float speed_ratio = std::clamp(std::abs(car.velocity) / (car.max_speed * 0.4f), 0.0f, 1.0f);
        const float turn_delta = car.steering * car.turn_speed * speed_ratio * dt;
        car.heading += (car.velocity >= 0.0f) ? turn_delta : -turn_delta;

        // 3. Position integration
        const glm::vec2 forward{std::cos(car.heading), std::sin(car.heading)};
        const glm::vec2 displacement = forward * (car.velocity * dt);
        tf.position.x += displacement.x;
        tf.position.y += displacement.y;
        tf.rotation = engine::rotation_z(car.heading);

        const float step_dist = std::abs(car.velocity * dt);
        car.distance_travelled += step_dist;
        car.time_since_checkpoint += dt;

        // 4. Checkpoint advancement
        const glm::vec2 car_pos{tf.position.x, tf.position.y};
        const int next_cp_idx = (car.current_checkpoint_idx + 1) % cp_count;
        const auto& next_cp = track.checkpoints[next_cp_idx];

        // Vector from checkpoint center to car
        const glm::vec2 to_car = car_pos - next_cp.center;
        // Project onto checkpoint forward normal
        const float dot_forward = to_car.x * next_cp.forward.x + to_car.y * next_cp.forward.y;
        const float dist_to_next = std::hypot(to_car.x, to_car.y);

        // Crossing threshold: close to checkpoint line and passed in forward direction
        if (dot_forward >= 0.0f && dist_to_next < (track.road_width * 0.75f)) {
            car.checkpoints_passed++;
            car.current_checkpoint_idx = next_cp_idx;
            car.time_since_checkpoint = 0.0f;
        }

        // 5. Stalling / timeout death
        if (car.time_since_checkpoint > 4.5f) {
            auto& sprite = view.get<engine::render::Sprite>(e);
            spawner::kill_car(car, sprite);
            continue;
        }

        // 6. Live fitness calculation
        const float dist_fraction = std::clamp(1.0f - (dist_to_next / track.road_width), 0.0f, 1.0f);
        car.fitness = static_cast<float>(car.checkpoints_passed) * 100.0f +
                      dist_fraction * 50.0f +
                      (car.distance_travelled * 0.04f);
    }
}

} // namespace game::ecs
