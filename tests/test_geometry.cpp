#include <gtest/gtest.h>

#include "track/geometry.h"

using namespace game::track::geometry;

TEST(GeometryTest, IntersectRaySegmentCrossingMiddle) {
    // Ray from (0, 0) to (10, 0)
    const glm::vec2 p1{0.0f, 0.0f};
    const glm::vec2 p2{10.0f, 0.0f};

    // Segment from (5, -5) to (5, 5)
    const glm::vec2 p3{5.0f, -5.0f};
    const glm::vec2 p4{5.0f, 5.0f};

    const auto hit = intersect_ray_segment(p1, p2, p3, p4);
    ASSERT_TRUE(hit.has_value());
    EXPECT_NEAR(*hit, 0.5f, 0.0001f);
}

TEST(GeometryTest, IntersectRaySegmentParallelNoHit) {
    const glm::vec2 p1{0.0f, 0.0f};
    const glm::vec2 p2{10.0f, 0.0f};

    const glm::vec2 p3{0.0f, 2.0f};
    const glm::vec2 p4{10.0f, 2.0f};

    const auto hit = intersect_ray_segment(p1, p2, p3, p4);
    EXPECT_FALSE(hit.has_value());
}

TEST(GeometryTest, IntersectRaySegmentCollinearNoHit) {
    const glm::vec2 p1{0.0f, 0.0f};
    const glm::vec2 p2{10.0f, 0.0f};

    const glm::vec2 p3{20.0f, 0.0f};
    const glm::vec2 p4{30.0f, 0.0f};

    const auto hit = intersect_ray_segment(p1, p2, p3, p4);
    EXPECT_FALSE(hit.has_value());
}

TEST(GeometryTest, IntersectRaySegmentMissRayTooShort) {
    // Ray ends at x = 4, segment is at x = 5
    const glm::vec2 p1{0.0f, 0.0f};
    const glm::vec2 p2{4.0f, 0.0f};

    const glm::vec2 p3{5.0f, -5.0f};
    const glm::vec2 p4{5.0f, 5.0f};

    const auto hit = intersect_ray_segment(p1, p2, p3, p4);
    EXPECT_FALSE(hit.has_value());
}

TEST(GeometryTest, IntersectRaySegmentMissBehindRay) {
    // Ray points to +x, segment is at x = -5
    const glm::vec2 p1{0.0f, 0.0f};
    const glm::vec2 p2{10.0f, 0.0f};

    const glm::vec2 p3{-5.0f, -5.0f};
    const glm::vec2 p4{-5.0f, 5.0f};

    const auto hit = intersect_ray_segment(p1, p2, p3, p4);
    EXPECT_FALSE(hit.has_value());
}

TEST(GeometryTest, IntersectRaySegmentMissPastEndpoints) {
    const glm::vec2 p1{0.0f, 0.0f};
    const glm::vec2 p2{10.0f, 0.0f};

    // Segment line passes through x = 5, but y range is [2, 10]
    const glm::vec2 p3{5.0f, 2.0f};
    const glm::vec2 p4{5.0f, 10.0f};

    const auto hit = intersect_ray_segment(p1, p2, p3, p4);
    EXPECT_FALSE(hit.has_value());
}

TEST(GeometryTest, IntersectRaySegmentHitEndpoint) {
    const glm::vec2 p1{0.0f, 0.0f};
    const glm::vec2 p2{10.0f, 0.0f};

    // Endpoint touches y = 0 at x = 5
    const glm::vec2 p3{5.0f, 0.0f};
    const glm::vec2 p4{5.0f, 5.0f};

    const auto hit = intersect_ray_segment(p1, p2, p3, p4);
    ASSERT_TRUE(hit.has_value());
    EXPECT_NEAR(*hit, 0.5f, 0.0001f);
}

TEST(GeometryTest, DistanceToSegmentPerpendicular) {
    const glm::vec2 a{0.0f, 0.0f};
    const glm::vec2 b{10.0f, 0.0f};
    const glm::vec2 p{5.0f, 3.0f};

    const float dist = distance_to_segment(p, a, b);
    EXPECT_NEAR(dist, 3.0f, 0.0001f);
}

TEST(GeometryTest, DistanceToSegmentClosestToEndpointA) {
    const glm::vec2 a{0.0f, 0.0f};
    const glm::vec2 b{10.0f, 0.0f};
    const glm::vec2 p{-3.0f, 4.0f};

    // Distance to (0,0) is 5.0
    const float dist = distance_to_segment(p, a, b);
    EXPECT_NEAR(dist, 5.0f, 0.0001f);
}

TEST(GeometryTest, DistanceToSegmentClosestToEndpointB) {
    const glm::vec2 a{0.0f, 0.0f};
    const glm::vec2 b{10.0f, 0.0f};
    const glm::vec2 p{13.0f, 4.0f};

    // Distance to (10,0) is 5.0
    const float dist = distance_to_segment(p, a, b);
    EXPECT_NEAR(dist, 5.0f, 0.0001f);
}

TEST(GeometryTest, DistanceToSegmentDegenerate) {
    const glm::vec2 a{2.0f, 2.0f};
    const glm::vec2 b{2.0f, 2.0f};
    const glm::vec2 p{5.0f, 6.0f};

    // Distance to (2,2) is sqrt(3^2 + 4^2) = 5.0
    const float dist = distance_to_segment(p, a, b);
    EXPECT_NEAR(dist, 5.0f, 0.0001f);
}
