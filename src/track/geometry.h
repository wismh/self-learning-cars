#pragma once

#include <glm/vec2.hpp>

#include <algorithm>
#include <cmath>
#include <optional>

namespace game::track::geometry {

// Ray (p1 to p2) vs Line Segment (p3 to p4) intersection.
// Returns normalized parameter t in [0, 1] along the ray if intersecting.
[[nodiscard]] inline std::optional<float> intersect_ray_segment(
        glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec2 p4) {
    const float denom = (p4.y - p3.y) * (p2.x - p1.x) - (p4.x - p3.x) * (p2.y - p1.y);
    if (std::abs(denom) < 0.000001f) {
        return std::nullopt;
    }

    const float ua = ((p4.x - p3.x) * (p1.y - p3.y) - (p4.y - p3.y) * (p1.x - p3.x)) / denom;
    const float ub = ((p2.x - p1.x) * (p1.y - p3.y) - (p2.y - p1.y) * (p1.x - p3.x)) / denom;

    if (ua >= 0.0f && ua <= 1.0f && ub >= 0.0f && ub <= 1.0f) {
        return ua;
    }
    return std::nullopt;
}

// Shortest Euclidean distance from point p to line segment (a to b).
[[nodiscard]] inline float distance_to_segment(glm::vec2 p, glm::vec2 a, glm::vec2 b) {
    const glm::vec2 ab = b - a;
    const float l2 = ab.x * ab.x + ab.y * ab.y;
    if (l2 < 0.0001f) {
        return std::hypot(p.x - a.x, p.y - a.y);
    }
    const float t = std::clamp(((p.x - a.x) * ab.x + (p.y - a.y) * ab.y) / l2, 0.0f, 1.0f);
    const glm::vec2 projection = a + t * ab;
    return std::hypot(p.x - projection.x, p.y - projection.y);
}

} // namespace game::track::geometry
