#pragma once

#include <engine/render/graphic_factory.h>

#include <glm/vec2.hpp>

#include <cstdint>
#include <vector>

namespace game::track {

struct LineSegment {
    glm::vec2 p1{0.0f, 0.0f};
    glm::vec2 p2{0.0f, 0.0f};
};

struct Checkpoint {
    glm::vec2 left{0.0f, 0.0f};
    glm::vec2 right{0.0f, 0.0f};
    glm::vec2 center{0.0f, 0.0f};
    glm::vec2 forward{1.0f, 0.0f};
    float accumulated_distance = 0.0f;
    int index = 0;
};

struct TrackSegmentQuad {
    glm::vec2 center{0.0f, 0.0f};
    glm::vec2 size{1.0f, 1.0f};
    float rotation = 0.0f; // Radians
};

struct GeneratedTrack {
    uint32_t seed = 0;
    float road_width = 72.0f;
    float total_length = 0.0f;
    glm::vec2 start_position{0.0f, 0.0f};
    float start_heading = 0.0f; // Radians

    std::vector<glm::vec2> centerline{};
    std::vector<LineSegment> left_walls{};
    std::vector<LineSegment> right_walls{};
    std::vector<Checkpoint> checkpoints{};

    // Procedural continuous meshes for smooth visual rendering
    engine::render::MeshDesc road_mesh{};
    engine::render::MeshDesc left_wall_mesh{};
    engine::render::MeshDesc right_wall_mesh{};
    engine::render::MeshDesc center_line_mesh{};

    // Fallback quads
    std::vector<TrackSegmentQuad> road_quads{};
    std::vector<TrackSegmentQuad> wall_quads{};
};

class TrackGenerator {
public:
    static GeneratedTrack generate(
            uint32_t seed,
            float road_width = 72.0f,
            int control_point_count = 12,
            float base_radius = 310.0f);
};

} // namespace game::track
