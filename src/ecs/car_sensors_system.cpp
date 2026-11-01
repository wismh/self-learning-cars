#include "ecs/car_sensors_system.h"

#include "ecs/components.h"
#include "ecs/spawner.h"
#include "track/geometry.h"

#include <engine/ecs/transform.h>
#include <engine/render/sprite.h>

#include <algorithm>
#include <cmath>

namespace game::ecs {

void update_car_sensors(engine::ecs::World& world, const track::GeneratedTrack& track) {
    auto view = world.view<engine::Transform, engine::render::Sprite, Car, RaycastSensors>();

    constexpr float kCarCrashRadius = 9.0f;

    for (engine::ecs::Entity e : view) {
        auto& car = view.get<Car>(e);
        if (!car.is_alive) {
            continue;
        }

        const auto& tf = view.get<engine::Transform>(e);
        const glm::vec2 car_pos{tf.position.x, tf.position.y};
        auto& sensors = view.get<RaycastSensors>(e);
        bool collided = false;

        // 1. Check direct car collision against track walls
        for (const auto& wall : track.left_walls) {
            if (track::geometry::distance_to_segment(car_pos, wall.p1, wall.p2) < kCarCrashRadius) {
                collided = true;
                break;
            }
        }
        if (!collided) {
            for (const auto& wall : track.right_walls) {
                if (track::geometry::distance_to_segment(car_pos, wall.p1, wall.p2) < kCarCrashRadius) {
                    collided = true;
                    break;
                }
            }
        }

        if (collided) {
            auto& sprite = view.get<engine::render::Sprite>(e);
            spawner::kill_car(car, sprite);
            continue;
        }

        // 2. Cast sensor rays
        for (std::size_t r = 0; r < RaycastSensors::kRayCount; ++r) {
            const float ray_angle = car.heading + sensors.angles[r];
            const glm::vec2 ray_dir{std::cos(ray_angle), std::sin(ray_angle)};
            const glm::vec2 ray_end = car_pos + ray_dir * RaycastSensors::kMaxRange;

            float closest_t = 1.0f;

            for (const auto& wall : track.left_walls) {
                if (const auto hit = track::geometry::intersect_ray_segment(car_pos, ray_end, wall.p1, wall.p2)) {
                    if (*hit < closest_t) {
                        closest_t = *hit;
                    }
                }
            }

            for (const auto& wall : track.right_walls) {
                if (const auto hit = track::geometry::intersect_ray_segment(car_pos, ray_end, wall.p1, wall.p2)) {
                    if (*hit < closest_t) {
                        closest_t = *hit;
                    }
                }
            }

            sensors.distances[r] = closest_t;
            sensors.hit_points[r] = car_pos + ray_dir * (RaycastSensors::kMaxRange * closest_t);
        }
    }
}

} // namespace game::ecs
