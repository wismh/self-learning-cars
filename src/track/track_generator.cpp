#include "track/track_generator.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <random>

namespace game::track {

namespace {

glm::vec2 catmull_rom(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, float t) {
    const float t2 = t * t;
    const float t3 = t2 * t;
    return 0.5f * ((2.0f * p1) +
                   (-p0 + p2) * t +
                   (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
                   (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
}

glm::vec2 normalize_safe(glm::vec2 v) {
    const float len = std::hypot(v.x, v.y);
    if (len > 0.0001f) {
        return v / len;
    }
    return {1.0f, 0.0f};
}

void build_ribbon_mesh(
        const std::vector<glm::vec2>& inner_pts,
        const std::vector<glm::vec2>& outer_pts,
        engine::render::MeshDesc& mesh) {
    const std::size_t count = inner_pts.size();
    mesh.vertices.clear();
    mesh.indices.clear();
    mesh.vertices.reserve(count * 2);
    mesh.indices.reserve(count * 6);

    for (std::size_t i = 0; i < count; ++i) {
        const float v = static_cast<float>(i) / static_cast<float>(count);
        mesh.vertices.push_back({
                .position = {inner_pts[i].x, inner_pts[i].y, 0.0f},
                .normal = {0.0f, 0.0f, 1.0f},
                .uv = {0.0f, v},
        });
        mesh.vertices.push_back({
                .position = {outer_pts[i].x, outer_pts[i].y, 0.0f},
                .normal = {0.0f, 0.0f, 1.0f},
                .uv = {1.0f, v},
        });
    }

    for (std::size_t i = 0; i < count; ++i) {
        const auto i0 = static_cast<std::uint32_t>(i * 2);
        const auto i1 = static_cast<std::uint32_t>(i * 2 + 1);
        const auto next = static_cast<std::uint32_t>((i + 1) % count);
        const auto i2 = static_cast<std::uint32_t>(next * 2);
        const auto i3 = static_cast<std::uint32_t>(next * 2 + 1);

        mesh.indices.push_back(i0);
        mesh.indices.push_back(i2);
        mesh.indices.push_back(i1);

        mesh.indices.push_back(i2);
        mesh.indices.push_back(i3);
        mesh.indices.push_back(i1);
    }
}

} // namespace

GeneratedTrack TrackGenerator::generate(
        uint32_t seed,
        float road_width,
        int control_point_count,
        float base_radius) {
    GeneratedTrack track;
    track.seed = seed;
    track.road_width = road_width;

    std::mt19937 rng{seed};
    const float pi = std::numbers::pi_v<float>;
    const int n = std::max(18, control_point_count);

    // 1. Base Elliptical Frame shifted to the right of the HUD sidebar
    const float rx = base_radius * 1.35f;
    const float ry = base_radius * 0.82f;
    constexpr float kTrackCenterOffsetX = 170.0f;

    std::uniform_real_distribution<float> phase_dist(0.0f, 2.0f * pi);
    const float phase2 = phase_dist(rng);
    const float phase3 = phase_dist(rng);

    // Sector anchors for procedural variety:
    // - One serpentine / chicane sector (S-curves)
    // - One sharp hairpin technical bend
    // - One narrow chokepoint / bottleneck sector
    const int chicane_start_cp = 2 + (seed % 4);
    const int hairpin_cp = (chicane_start_cp + 8 + (seed % 3)) % n;

    std::vector<glm::vec2> control_points(n);

    for (int i = 0; i < n; ++i) {
        const float theta = (2.0f * pi * static_cast<float>(i)) / static_cast<float>(n);

        // General organic shape
        float r = 1.0f + 0.14f * std::sin(theta * 2.0f + phase2) + 0.08f * std::cos(theta * 3.0f + phase3);

        // A. Serpentine / Chicane Sector (flicking S-curves)
        const int dist_to_chicane = (i >= chicane_start_cp) ? (i - chicane_start_cp) : (i + n - chicane_start_cp);
        if (dist_to_chicane <= 4) {
            // Alternating inward/outward wiggle
            const float chicane_sign = (dist_to_chicane % 2 == 1) ? 1.0f : -1.0f;
            const float chicane_factor = std::sin((static_cast<float>(dist_to_chicane) / 4.0f) * pi);
            r += chicane_sign * 0.22f * chicane_factor;
        }

        // B. Hairpin Technical Turn (sharp apex pulled inwards or outwards)
        const int dist_to_hairpin = std::abs(i - hairpin_cp);
        if (dist_to_hairpin <= 1) {
            const float hairpin_dip = (dist_to_hairpin == 0) ? -0.26f : -0.12f;
            r += hairpin_dip;
        }

        const float x = std::cos(theta) * rx * r + kTrackCenterOffsetX;
        const float y = std::sin(theta) * ry * r;
        control_points[i] = {x, y};
    }

    // 2. Gentle smoothing pass preserving sharp bends while ensuring C1 continuity
    std::vector<glm::vec2> smoothed(n);
    for (int i = 0; i < n; ++i) {
        const glm::vec2 p_prev = control_points[(i + n - 1) % n];
        const glm::vec2 p_curr = control_points[i];
        const glm::vec2 p_next = control_points[(i + 1) % n];
        smoothed[i] = 0.15f * p_prev + 0.70f * p_curr + 0.15f * p_next;
    }
    control_points = std::move(smoothed);

    // 3. High-density Catmull-Rom spline sampling (24 steps per segment -> ~432 points)
    constexpr int kStepsPerSegment = 24;
    std::vector<glm::vec2> spline_points;
    spline_points.reserve(n * kStepsPerSegment);

    for (int i = 0; i < n; ++i) {
        const glm::vec2 p0 = control_points[(i + n - 1) % n];
        const glm::vec2 p1 = control_points[i];
        const glm::vec2 p2 = control_points[(i + 1) % n];
        const glm::vec2 p3 = control_points[(i + 2) % n];

        for (int step = 0; step < kStepsPerSegment; ++step) {
            const float t = static_cast<float>(step) / static_cast<float>(kStepsPerSegment);
            spline_points.push_back(catmull_rom(p0, p1, p2, p3, t));
        }
    }

    const std::size_t count = spline_points.size();

    // 4. Position Start Line on the Long Bottom Straightaway
    // Real racing tracks place the start/finish line and starting grid on the longest,
    // straightest section. On our CCW circuit, this is the bottom straight (y < -40.0)
    // where traffic moves smoothly left-to-right (+x).
    std::size_t best_start_idx = 0;
    float best_score = -1e9f;

    for (std::size_t i = 0; i < count; ++i) {
        if (spline_points[i].y < -40.0f) {
            const std::size_t i_prev = (i + count - 12) % count;
            const std::size_t i_next = (i + 12) % count;
            const glm::vec2 fwd = normalize_safe(spline_points[i_next] - spline_points[i_prev]);

            // Ensure motion is firmly rightwards along the straight
            if (fwd.x > 0.6f) {
                const std::size_t i_far_back = (i + count - 20) % count;
                const std::size_t i_far_fwd = (i + 20) % count;
                const glm::vec2 dir_back = normalize_safe(spline_points[i] - spline_points[i_far_back]);
                const glm::vec2 dir_fwd = normalize_safe(spline_points[i_far_fwd] - spline_points[i]);
                const float alignment = dir_back.x * dir_fwd.x + dir_back.y * dir_fwd.y;

                // Center bonus around x in [100, 200]
                const float center_bonus = -std::abs(spline_points[i].x - 140.0f) * 0.005f;
                const float score = alignment * 10.0f + center_bonus;
                if (score > best_score) {
                    best_score = score;
                    best_start_idx = i;
                }
            }
        }
    }

    if (best_start_idx != 0) {
        std::rotate(spline_points.begin(), spline_points.begin() + best_start_idx, spline_points.end());
    }

    track.centerline = spline_points;

    // 5. Tangents and Normals
    std::vector<glm::vec2> tangents(count);
    std::vector<glm::vec2> normals(count);
    for (std::size_t i = 0; i < count; ++i) {
        const glm::vec2 prev = spline_points[(i + count - 1) % count];
        const glm::vec2 next = spline_points[(i + 1) % count];
        const glm::vec2 tangent = normalize_safe(next - prev);
        tangents[i] = tangent;
        normals[i] = glm::vec2{-tangent.y, tangent.x};
    }

    // 6. Variable Road Width (Bottlenecks placed in the mid-lap technical sector)
    // Keep the starting straight wide and safe for grid starts
    const std::size_t bottleneck_min = count / 4;
    const std::size_t bottleneck_span = count / 2;
    const std::size_t bottleneck_center = bottleneck_min + ((seed * 43) % bottleneck_span);
    constexpr float kBottleneckRadius = 45.0f; // Slices over which narrowing occurs

    const float base_half_w = road_width * 0.5f; // e.g. 36.0f
    std::vector<float> local_half_widths(count);

    for (std::size_t i = 0; i < count; ++i) {
        // Distance along loop to bottleneck
        const float diff = std::abs(static_cast<float>(i) - static_cast<float>(bottleneck_center));
        const float dist_to_bottleneck = std::min(diff, static_cast<float>(count) - diff);

        float width_multiplier = 1.0f;
        if (dist_to_bottleneck < kBottleneckRadius) {
            // Smooth cosine constriction down to 55% width (~40px total road width!)
            const float t = dist_to_bottleneck / kBottleneckRadius;
            const float factor = 0.5f * (1.0f + std::cos(pi * t));
            width_multiplier = 1.0f - (0.45f * factor);
        } else {
            // Subtle breathing on wide straights (up to 110% width)
            const float theta = (2.0f * pi * static_cast<float>(i)) / static_cast<float>(count);
            width_multiplier = 1.0f + 0.10f * std::cos(theta * 2.0f);
        }

        local_half_widths[i] = base_half_w * width_multiplier;
    }

    // 6. Curvature-Adaptive Boundary Points (Cusp-free offsets on hairpins and chicanes)
    std::vector<glm::vec2> left_pts(count);
    std::vector<glm::vec2> right_pts(count);
    float accumulated_dist = 0.0f;
    track.checkpoints.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
        const glm::vec2 curr = spline_points[i];
        const glm::vec2 prev = spline_points[(i + count - 1) % count];
        const glm::vec2 next = spline_points[(i + 1) % count];

        const glm::vec2 t_curr = tangents[i];
        const glm::vec2 t_next = tangents[(i + 1) % count];

        // Signed angular turn rate
        const float cross_z = t_curr.x * t_next.y - t_curr.y * t_next.x;
        const float dot_t = std::clamp(t_curr.x * t_next.x + t_curr.y * t_next.y, -1.0f, 1.0f);
        const float d_theta = std::atan2(cross_z, dot_t);
        const float seg_len = std::hypot(next.x - curr.x, next.y - curr.y);
        const float kappa = (seg_len > 0.001f) ? (d_theta / seg_len) : 0.0f;

        const float target_half_w = local_half_widths[i];
        float left_offset = target_half_w;
        float right_offset = target_half_w;

        // On sharp inside bends, clamp offset so d < 0.82 * R to avoid swallowtail cusps
        if (kappa > 0.0005f) {
            // Left is inside
            const float max_inside = 0.82f / kappa;
            left_offset = std::min(target_half_w, std::max(12.0f, max_inside));
        } else if (kappa < -0.0005f) {
            // Right is inside
            const float max_inside = 0.82f / (-kappa);
            right_offset = std::min(target_half_w, std::max(12.0f, max_inside));
        }

        left_pts[i] = curr + normals[i] * left_offset;
        right_pts[i] = curr - normals[i] * right_offset;

        const float step_dist = std::hypot(curr.x - prev.x, curr.y - prev.y);
        accumulated_dist += step_dist;

        track.checkpoints.push_back(Checkpoint{
                .left = left_pts[i],
                .right = right_pts[i],
                .center = curr,
                .forward = t_curr,
                .accumulated_distance = accumulated_dist,
                .index = static_cast<int>(i),
        });
    }

    track.total_length = accumulated_dist;

    // Start line setup at checkpoint 0
    track.start_position = spline_points[0];
    track.start_heading = std::atan2(tangents[0].y, tangents[0].x);

    // 7. Collision Wall Segments
    track.left_walls.reserve(count);
    track.right_walls.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        const std::size_t next_i = (i + 1) % count;
        track.left_walls.push_back(LineSegment{left_pts[i], left_pts[next_i]});
        track.right_walls.push_back(LineSegment{right_pts[i], right_pts[next_i]});
    }

    // 8. Procedural Continuous Meshes
    // A. Smooth Road Asphalt Ribbon with variable width
    build_ribbon_mesh(left_pts, right_pts, track.road_mesh);

    // B. Left Wall Barrier / Curb Mesh (red outer racing curb)
    std::vector<glm::vec2> left_outer(count);
    for (std::size_t i = 0; i < count; ++i) {
        left_outer[i] = left_pts[i] + normals[i] * 4.5f;
    }
    build_ribbon_mesh(left_pts, left_outer, track.left_wall_mesh);

    // C. Right Wall Barrier / Curb Mesh (neon cyan inner racing curb)
    std::vector<glm::vec2> right_outer(count);
    for (std::size_t i = 0; i < count; ++i) {
        right_outer[i] = right_pts[i] - normals[i] * 4.5f;
    }
    build_ribbon_mesh(right_outer, right_pts, track.right_wall_mesh);

    // D. Dashed Centerline Mesh
    std::vector<glm::vec2> center_left(count);
    std::vector<glm::vec2> center_right(count);
    for (std::size_t i = 0; i < count; ++i) {
        center_left[i] = spline_points[i] + normals[i] * 1.0f;
        center_right[i] = spline_points[i] - normals[i] * 1.0f;
    }

    // Dash pattern: 3 steps on, 3 steps off
    track.center_line_mesh.vertices.clear();
    track.center_line_mesh.indices.clear();
    for (std::size_t i = 0; i < count; ++i) {
        if ((i / 4) % 2 == 0) {
            const std::size_t next_i = (i + 1) % count;
            const auto base_idx = static_cast<std::uint32_t>(track.center_line_mesh.vertices.size());

            track.center_line_mesh.vertices.push_back({
                    .position = {center_left[i].x, center_left[i].y, 0.0f},
                    .normal = {0.0f, 0.0f, 1.0f},
                    .uv = {0.0f, 0.0f},
            });
            track.center_line_mesh.vertices.push_back({
                    .position = {center_right[i].x, center_right[i].y, 0.0f},
                    .normal = {0.0f, 0.0f, 1.0f},
                    .uv = {1.0f, 0.0f},
            });
            track.center_line_mesh.vertices.push_back({
                    .position = {center_left[next_i].x, center_left[next_i].y, 0.0f},
                    .normal = {0.0f, 0.0f, 1.0f},
                    .uv = {0.0f, 1.0f},
            });
            track.center_line_mesh.vertices.push_back({
                    .position = {center_right[next_i].x, center_right[next_i].y, 0.0f},
                    .normal = {0.0f, 0.0f, 1.0f},
                    .uv = {1.0f, 1.0f},
            });

            track.center_line_mesh.indices.push_back(base_idx);
            track.center_line_mesh.indices.push_back(base_idx + 2);
            track.center_line_mesh.indices.push_back(base_idx + 1);

            track.center_line_mesh.indices.push_back(base_idx + 2);
            track.center_line_mesh.indices.push_back(base_idx + 3);
            track.center_line_mesh.indices.push_back(base_idx + 1);
        }
    }

    return track;
}

} // namespace game::track
