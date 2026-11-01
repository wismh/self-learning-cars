#pragma once

#include "ai/neural_network.h"

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include <array>
#include <cstdint>

namespace game::ecs {

struct Car {
    float velocity = 0.0f;
    float heading = 0.0f; // Radians
    float throttle = 0.0f;
    float steering = 0.0f;

    bool is_alive = true;
    bool is_leader = false;

    float fitness = 0.0f;
    int checkpoints_passed = 0;
    int current_checkpoint_idx = 0;
    float time_since_checkpoint = 0.0f;
    float distance_travelled = 0.0f;

    float max_speed = 360.0f;
    float acceleration = 280.0f;
    float friction = 0.85f;
    float turn_speed = 3.8f;
};

struct RaycastSensors {
    static constexpr std::size_t kRayCount = 7;
    static constexpr float kMaxRange = 260.0f;

    // Relative angles in radians: -60, -40, -20, 0, 20, 40, 60 degrees
    std::array<float, kRayCount> angles{
            -1.04719755f, // -60 deg
            -0.69813170f, // -40 deg
            -0.34906585f, // -20 deg
            0.0f,         //   0 deg
            0.34906585f,  //  20 deg
            0.69813170f,  //  40 deg
            1.04719755f,  //  60 deg
    };

    std::array<float, kRayCount> distances{1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    std::array<glm::vec2, kRayCount> hit_points{};
};

struct Brain {
    ai::NeuralNetwork network{};
    uint32_t genome_id = 0;
};

// Tag components for ECS entity categories
struct TrackSurfaceTag {};
struct TrackWallTag {};
struct CheckpointTag {
    int index = 0;
};
struct RayVisualTag {
    std::size_t ray_index = 0;
};

struct SimulationConfig {
    int population_size = 60;
    bool paused = false;
    float time_scale = 1.0f;
    bool show_rays = true;

    uint32_t track_seed = 101;
    int current_generation = 1;
    float best_fitness_current_gen = 0.0f;
    float all_time_best_fitness = 0.0f;
    int alive_cars_count = 60;
    float leader_speed = 0.0f;
};

struct SimulationStats {
    float fps = 60.0f;
    float frame_time_ms = 16.6f;
};

} // namespace game::ecs
