#include "ecs/spawner.h"

#include <asset_ids.h>

#include "ecs/components.h"

#include <engine/builtin_ids.h>
#include <engine/ecs/camera.h>
#include <engine/ecs/transform.h>
#include <engine/render/renderable.h>
#include <engine/render/sprite.h>

#include <cmath>
#include <numbers>
#include <vector>

namespace game::ecs::spawner {

namespace {

glm::vec2 calculate_grid_position(
        const track::GeneratedTrack& track,
        std::size_t index,
        float normal_x,
        float normal_y) {
    // 4-column staggered racing grid on the wide straightaway
    const float lateral = (static_cast<float>(index % 4) - 1.5f) * 6.5f;     // -9.75, -3.25, +3.25, +9.75 px
    const float longitudinal = -6.0f - static_cast<float>(index / 4) * 4.0f; // 6px to 62px behind start line
    return {
            track.start_position.x + normal_x * lateral + std::cos(track.start_heading) * longitudinal,
            track.start_position.y + normal_y * lateral + std::sin(track.start_heading) * longitudinal,
    };
}

void reset_car_state(Car& car, float heading, int checkpoint_idx, bool is_leader) {
    car.velocity = 0.0f;
    car.heading = heading;
    car.throttle = 0.0f;
    car.steering = 0.0f;
    car.is_alive = true;
    car.is_leader = is_leader;
    car.fitness = 0.0f;
    car.checkpoints_passed = 0;
    car.current_checkpoint_idx = checkpoint_idx;
    car.time_since_checkpoint = 0.0f;
    car.distance_travelled = 0.0f;
}

} // namespace

void setup_camera(engine::ecs::World& world) {
    const engine::ecs::Entity camera = world.create();
    world.emplace<engine::Transform>(camera, engine::Transform{.position = {0.0f, 0.0f, 10.0f}});
    world.emplace<engine::Camera>(camera, engine::Camera{
            .projection = engine::Projection::Orthographic,
            .ortho_size = 450.0f,
            .auto_aspect = true,
            .near_clip = -100.0f,
            .far_clip = 100.0f,
    });
    world.ctx<engine::ActiveCamera>().entity = camera;
}

void clear_track(engine::ecs::World& world) {
    std::vector<engine::ecs::Entity> to_destroy;

    auto surface_view = world.view<TrackSurfaceTag>();
    for (engine::ecs::Entity e : surface_view) {
        to_destroy.push_back(e);
    }
    auto wall_view = world.view<TrackWallTag>();
    for (engine::ecs::Entity e : wall_view) {
        to_destroy.push_back(e);
    }
    auto cp_view = world.view<CheckpointTag>();
    for (engine::ecs::Entity e : cp_view) {
        to_destroy.push_back(e);
    }

    for (engine::ecs::Entity e : to_destroy) {
        world.destroy(e);
    }
}

void spawn_track(engine::ecs::World& world, engine::AssetsDb& assets, const track::GeneratedTrack& track) {
    clear_track(world);

    auto* factory = assets.graphic_factory();
    auto unlit_material = assets.get<engine::render::IMaterial>(engine::builtin::material_unlit);

    if (factory && unlit_material) {
        // 1. Smooth Road Asphalt Ribbon Mesh
        auto road_mesh = factory->create_mesh(track.road_mesh);
        const engine::ecs::Entity road_entity = world.create();
        world.emplace<engine::Transform>(road_entity, engine::Transform{
                .position = {0.0f, 0.0f, 0.0f},
                .scale = {1.0f, 1.0f, 1.0f},
        });
        world.emplace<engine::render::Renderable>(road_entity, engine::render::Renderable{
                .mesh = road_mesh,
                .material = unlit_material,
                .color = glm::vec4{0.13f, 0.16f, 0.22f, 1.0f}, // Deep asphalt
                .layer = 0,
                .order_in_layer = 0,
        });
        world.emplace<TrackSurfaceTag>(road_entity);

        // 2. Dashed Centerline Mesh
        auto center_line_mesh = factory->create_mesh(track.center_line_mesh);
        const engine::ecs::Entity line_entity = world.create();
        world.emplace<engine::Transform>(line_entity, engine::Transform{
                .position = {0.0f, 0.0f, 0.0f},
                .scale = {1.0f, 1.0f, 1.0f},
        });
        world.emplace<engine::render::Renderable>(line_entity, engine::render::Renderable{
                .mesh = center_line_mesh,
                .material = unlit_material,
                .color = glm::vec4{0.92f, 0.92f, 0.95f, 0.85f}, // White dashed road markings
                .layer = 0,
                .order_in_layer = 1,
        });
        world.emplace<TrackSurfaceTag>(line_entity);

        // 3. Left Wall Barrier / Curb Mesh
        auto left_wall_mesh = factory->create_mesh(track.left_wall_mesh);
        const engine::ecs::Entity left_entity = world.create();
        world.emplace<engine::Transform>(left_entity, engine::Transform{
                .position = {0.0f, 0.0f, 0.0f},
                .scale = {1.0f, 1.0f, 1.0f},
        });
        world.emplace<engine::render::Renderable>(left_entity, engine::render::Renderable{
                .mesh = left_wall_mesh,
                .material = unlit_material,
                .color = glm::vec4{0.95f, 0.32f, 0.32f, 1.0f}, // Red outer racing curb
                .layer = 0,
                .order_in_layer = 2,
        });
        world.emplace<TrackWallTag>(left_entity);

        // 4. Right Wall Barrier / Curb Mesh
        auto right_wall_mesh = factory->create_mesh(track.right_wall_mesh);
        const engine::ecs::Entity right_entity = world.create();
        world.emplace<engine::Transform>(right_entity, engine::Transform{
                .position = {0.0f, 0.0f, 0.0f},
                .scale = {1.0f, 1.0f, 1.0f},
        });
        world.emplace<engine::render::Renderable>(right_entity, engine::render::Renderable{
                .mesh = right_wall_mesh,
                .material = unlit_material,
                .color = glm::vec4{0.25f, 0.75f, 1.0f, 1.0f}, // Neon cyan inner racing curb
                .layer = 0,
                .order_in_layer = 2,
        });
        world.emplace<TrackWallTag>(right_entity);
    }

    // 5. Start / Finish Line at Checkpoint 0
    if (!track.checkpoints.empty()) {
        const auto& cp0 = track.checkpoints[0];
        const engine::ecs::Entity start_line = world.create();
        const glm::vec2 delta = cp0.right - cp0.left;
        const float len = std::hypot(delta.x, delta.y);
        const float angle = std::atan2(delta.y, delta.x);

        world.emplace<engine::Transform>(start_line, engine::Transform{
                .position = {cp0.center.x, cp0.center.y, 0.0f},
                .rotation = engine::rotation_z(angle),
                .scale = {len, 6.0f, 1.0f},
        });

        engine::render::Sprite sprite = assets.get_sprite(assets::textures::pixel);
        sprite.color = glm::vec4{0.15f, 0.95f, 0.45f, 0.95f}; // Bright green finish gate
        sprite.order_in_layer = 3;
        world.emplace<engine::render::Sprite>(start_line, std::move(sprite));
        world.emplace<CheckpointTag>(start_line, CheckpointTag{.index = 0});
    }
}

void clear_cars(engine::ecs::World& world) {
    std::vector<engine::ecs::Entity> to_destroy;
    auto view = world.view<Car>();
    for (engine::ecs::Entity e : view) {
        to_destroy.push_back(e);
    }
    for (engine::ecs::Entity e : to_destroy) {
        world.destroy(e);
    }
}

void spawn_cars(
        engine::ecs::World& world,
        engine::AssetsDb& assets,
        const track::GeneratedTrack& track,
        const ai::GeneticAlgorithm& ga) {
    clear_cars(world);
    const auto pop = ga.population();
    const float normal_x = -std::sin(track.start_heading);
    const float normal_y = std::cos(track.start_heading);
    const int initial_cp = track.checkpoints.empty() ? 0 : static_cast<int>(track.checkpoints.size() - 1);

    for (std::size_t i = 0; i < pop.size(); ++i) {
        const engine::ecs::Entity car_entity = world.create();
        const glm::vec2 pos = calculate_grid_position(track, i, normal_x, normal_y);

        world.emplace<engine::Transform>(car_entity, engine::Transform{
                .position = {pos.x, pos.y, 0.0f},
                .rotation = engine::rotation_z(track.start_heading),
                .scale = {1.0f, 1.0f, 1.0f},
        });

        engine::render::Sprite sprite = assets.get_sprite(assets::textures::car);
        sprite.color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        sprite.order_in_layer = (i == 0) ? 10 : 5;
        world.emplace<engine::render::Sprite>(car_entity, std::move(sprite));

        Car car;
        reset_car_state(car, track.start_heading, initial_cp, i == 0);
        world.emplace<Car>(car_entity, car);

        world.emplace<RaycastSensors>(car_entity);

        Brain brain;
        brain.network.set_weights(pop[i].weights);
        brain.genome_id = pop[i].id;
        world.emplace<Brain>(car_entity, std::move(brain));
    }
}

void respawn_cars(
        engine::ecs::World& world,
        engine::AssetsDb& assets,
        const track::GeneratedTrack& track,
        const ai::GeneticAlgorithm& ga) {
    auto view = world.view<engine::Transform, engine::render::Sprite, Car, RaycastSensors, Brain>();
    if (view.begin() == view.end()) {
        spawn_cars(world, assets, track, ga);
        return;
    }

    const auto pop = ga.population();
    const float normal_x = -std::sin(track.start_heading);
    const float normal_y = std::cos(track.start_heading);
    const int initial_cp = track.checkpoints.empty() ? 0 : static_cast<int>(track.checkpoints.size() - 1);

    std::size_t idx = 0;
    for (engine::ecs::Entity e : view) {
        if (idx >= pop.size()) {
            break;
        }

        const glm::vec2 pos = calculate_grid_position(track, idx, normal_x, normal_y);

        auto& tf = view.get<engine::Transform>(e);
        tf.position = {pos.x, pos.y, 0.0f};
        tf.rotation = engine::rotation_z(track.start_heading);

        auto& sprite = view.get<engine::render::Sprite>(e);
        sprite = assets.get_sprite(assets::textures::car);
        sprite.color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
        sprite.order_in_layer = (idx == 0) ? 10 : 5;

        auto& car = view.get<Car>(e);
        reset_car_state(car, track.start_heading, initial_cp, idx == 0);

        auto& sensors = view.get<RaycastSensors>(e);
        sensors.distances.fill(1.0f);

        auto& brain = view.get<Brain>(e);
        brain.network.set_weights(pop[idx].weights);
        brain.genome_id = pop[idx].id;

        idx++;
    }
}

void setup_ray_visualizers(engine::ecs::World& world, engine::AssetsDb& assets) {
    // 7 ray entities for visualizing leader's vision
    for (std::size_t r = 0; r < RaycastSensors::kRayCount; ++r) {
        const engine::ecs::Entity ray = world.create();
        world.emplace<engine::Transform>(ray, engine::Transform{
                .position = {0.0f, 0.0f, 0.0f},
                .rotation = engine::rotation_z(0.0f),
                .scale = {0.0f, 1.5f, 1.0f},
        });

        engine::render::Sprite sprite = assets.get_sprite(assets::textures::pixel);
        sprite.color = glm::vec4{0.15f, 0.95f, 0.75f, 0.75f}; // Cyan/Neon sensor laser
        sprite.order_in_layer = 4;
        world.emplace<engine::render::Sprite>(ray, std::move(sprite));
        world.emplace<RayVisualTag>(ray, RayVisualTag{.ray_index = r});
    }
}

void kill_car(Car& car, engine::render::Sprite& sprite) {
    car.is_alive = false;
    car.velocity = 0.0f;
    sprite.color = glm::vec4{0.30f, 0.32f, 0.38f, 0.16f}; // Subtle translucent ghost
    sprite.order_in_layer = 4;                             // Beneath living cars (5) and leader (10)
}

} // namespace game::ecs::spawner
