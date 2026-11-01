#include <gtest/gtest.h>

#include "track/track_generator.h"

#include <cmath>

TEST(TrackGeneratorTest, GeneratesClosedCircuit) {
    const auto track1 = game::track::TrackGenerator::generate(101, 70.0f);
    const auto track2 = game::track::TrackGenerator::generate(102, 70.0f);

    EXPECT_GT(track1.total_length, 1000.0f);
    EXPECT_GT(track2.total_length, 1000.0f);

    EXPECT_FALSE(track1.checkpoints.empty());
    EXPECT_EQ(track1.left_walls.size(), track1.right_walls.size());
    EXPECT_FALSE(track1.road_mesh.vertices.empty());
    EXPECT_FALSE(track1.road_mesh.indices.empty());
    EXPECT_FALSE(track1.left_wall_mesh.vertices.empty());
    EXPECT_FALSE(track1.right_wall_mesh.vertices.empty());

    // Different seeds produce different start points or geometries
    const float dist_diff = std::abs(track1.total_length - track2.total_length);
    const float start_diff = std::hypot(
            track1.start_position.x - track2.start_position.x,
            track1.start_position.y - track2.start_position.y);

    EXPECT_TRUE(dist_diff > 1.0f || start_diff > 1.0f);
}

TEST(TrackGeneratorTest, CheckpointsDistanceMonotonicity) {
    const auto track = game::track::TrackGenerator::generate(101, 72.0f);
    ASSERT_GT(track.checkpoints.size(), 10u);

    for (std::size_t i = 1; i < track.checkpoints.size(); ++i) {
        EXPECT_GT(
                track.checkpoints[i].accumulated_distance,
                track.checkpoints[i - 1].accumulated_distance);
    }

    const float last_dist = track.checkpoints.back().accumulated_distance;
    EXPECT_NEAR(last_dist, track.total_length, 50.0f);
}

TEST(TrackGeneratorTest, CheckpointsGeometryIntegrity) {
    const auto track = game::track::TrackGenerator::generate(105, 72.0f);

    for (const auto& cp : track.checkpoints) {
        // Forward vector should be roughly unit length
        const float fwd_len = std::hypot(cp.forward.x, cp.forward.y);
        EXPECT_NEAR(fwd_len, 1.0f, 0.01f);

        // Road width between left and right boundary points should be positive and reasonable
        const float span = std::hypot(cp.right.x - cp.left.x, cp.right.y - cp.left.y);
        EXPECT_GT(span, 20.0f);
        EXPECT_LT(span, 160.0f);
    }
}

TEST(TrackGeneratorTest, WallSegmentsFormClosedContinuousChain) {
    const auto track = game::track::TrackGenerator::generate(110, 72.0f);
    ASSERT_FALSE(track.left_walls.empty());
    const std::size_t count = track.left_walls.size();

    for (std::size_t i = 0; i < count; ++i) {
        const std::size_t next = (i + 1) % count;
        // p2 of segment i must connect to p1 of segment (i+1)
        EXPECT_NEAR(track.left_walls[i].p2.x, track.left_walls[next].p1.x, 0.001f);
        EXPECT_NEAR(track.left_walls[i].p2.y, track.left_walls[next].p1.y, 0.001f);

        EXPECT_NEAR(track.right_walls[i].p2.x, track.right_walls[next].p1.x, 0.001f);
        EXPECT_NEAR(track.right_walls[i].p2.y, track.right_walls[next].p1.y, 0.001f);
    }
}

TEST(TrackGeneratorTest, StartPositionMatchesCheckpointZero) {
    const auto track = game::track::TrackGenerator::generate(115, 72.0f);
    ASSERT_FALSE(track.checkpoints.empty());

    EXPECT_NEAR(track.start_position.x, track.checkpoints[0].center.x, 0.001f);
    EXPECT_NEAR(track.start_position.y, track.checkpoints[0].center.y, 0.001f);
}
