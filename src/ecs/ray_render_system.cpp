#include "ecs/ray_render_system.h"

#include "ecs/components.h"

#include <engine/ecs/transform.h>
#include <engine/render/sprite.h>

#include <cmath>

namespace game::ecs {

void update_ray_visuals(engine::ecs::World& world) {
    const auto& config = world.ctx<SimulationConfig>();

    // 1. Find the current leader car
    const Car* leader_car = nullptr;
    const RaycastSensors* leader_sensors = nullptr;
    glm::vec2 leader_pos{0.0f, 0.0f};

    auto car_view = world.view<engine::Transform, Car, RaycastSensors>();
    for (engine::ecs::Entity e : car_view) {
        const auto& car = car_view.get<Car>(e);
        if (car.is_leader && car.is_alive) {
            leader_car = &car;
            leader_sensors = &car_view.get<RaycastSensors>(e);
            const auto& tf = car_view.get<engine::Transform>(e);
            leader_pos = {tf.position.x, tf.position.y};
            break;
        }
    }

    // 2. Update visual ray entities
    auto ray_view = world.view<engine::Transform, engine::render::Sprite, RayVisualTag>();
    for (engine::ecs::Entity e : ray_view) {
        auto& tf = ray_view.get<engine::Transform>(e);
        auto& sprite = ray_view.get<engine::render::Sprite>(e);
        const auto& tag = ray_view.get<RayVisualTag>(e);

        if (!config.show_rays || !leader_car || !leader_sensors || tag.ray_index >= RaycastSensors::kRayCount) {
            tf.scale = {0.0f, 0.0f, 1.0f}; // Hide
            continue;
        }

        const glm::vec2 hit = leader_sensors->hit_points[tag.ray_index];
        const glm::vec2 delta = hit - leader_pos;
        const float len = std::hypot(delta.x, delta.y);
        const float angle = std::atan2(delta.y, delta.x);
        const glm::vec2 mid = 0.5f * (leader_pos + hit);

        tf.position = {mid.x, mid.y, 0.0f};
        tf.rotation = engine::rotation_z(angle);
        tf.scale = {len, 1.0f, 1.0f};

        const float norm_dist = leader_sensors->distances[tag.ray_index];
        if (norm_dist < 0.20f) {
            sprite.color = glm::vec4{1.0f, 0.25f, 0.25f, 0.90f}; // Red warning when close to wall
        } else if (norm_dist < 0.45f) {
            sprite.color = glm::vec4{1.0f, 0.75f, 0.20f, 0.85f}; // Amber when approaching
        } else {
            sprite.color = glm::vec4{0.15f, 0.92f, 1.0f, 0.65f}; // Thin, delicate neon cyan laser
        }
    }
}

} // namespace game::ecs
