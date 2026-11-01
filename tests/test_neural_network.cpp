#include <gtest/gtest.h>

#include "ai/neural_network.h"

#include <algorithm>
#include <array>
#include <vector>

TEST(NeuralNetworkTest, ConstantsConsistency) {
    EXPECT_EQ(game::ai::NeuralNetwork::kWeights1, 8 * 12);
    EXPECT_EQ(game::ai::NeuralNetwork::kBiases1, 12);
    EXPECT_EQ(game::ai::NeuralNetwork::kWeights2, 12 * 2);
    EXPECT_EQ(game::ai::NeuralNetwork::kBiases2, 2);
    EXPECT_EQ(
            game::ai::NeuralNetwork::kTotalWeights,
            (8 * 12) + 12 + (12 * 2) + 2);
}

TEST(NeuralNetworkTest, ForwardPassWithinBounds) {
    game::ai::NeuralNetwork net;

    std::vector<float> weights(game::ai::NeuralNetwork::kTotalWeights, 0.5f);
    net.set_weights(weights);

    std::array<float, game::ai::NeuralNetwork::kInputs> inputs{
            0.5f, 0.7f, 0.2f, 1.0f, 0.3f, 0.6f, 0.9f, 0.8f};

    const auto out = net.forward(inputs);

    EXPECT_GE(out.steering, -1.0f);
    EXPECT_LE(out.steering, 1.0f);
    EXPECT_GE(out.throttle, -1.0f);
    EXPECT_LE(out.throttle, 1.0f);
}

TEST(NeuralNetworkTest, ZeroWeightsProduceZeroOutputs) {
    game::ai::NeuralNetwork net;
    std::array<float, game::ai::NeuralNetwork::kInputs> inputs{1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};

    const auto out = net.forward(inputs);

    EXPECT_NEAR(out.steering, 0.0f, 0.0001f);
    EXPECT_NEAR(out.throttle, 0.0f, 0.0001f);
}

TEST(NeuralNetworkTest, WeightsSpanAccessAndModification) {
    game::ai::NeuralNetwork net;
    auto w_span = net.weights();
    EXPECT_EQ(w_span.size(), game::ai::NeuralNetwork::kTotalWeights);

    w_span[0] = 1.25f;
    w_span[10] = -0.75f;

    const auto& const_net = net;
    auto const_span = const_net.weights();
    EXPECT_FLOAT_EQ(const_span[0], 1.25f);
    EXPECT_FLOAT_EQ(const_span[10], -0.75f);
}

TEST(NeuralNetworkTest, SetWeightsPartialSource) {
    game::ai::NeuralNetwork net;
    std::vector<float> partial{0.1f, 0.2f, 0.3f};
    net.set_weights(partial);

    auto w = net.weights();
    EXPECT_FLOAT_EQ(w[0], 0.1f);
    EXPECT_FLOAT_EQ(w[1], 0.2f);
    EXPECT_FLOAT_EQ(w[2], 0.3f);
    EXPECT_FLOAT_EQ(w[3], 0.0f); // Unmodified
}

TEST(NeuralNetworkTest, ExtremeWeightsProduceSaturatedOutputs) {
    game::ai::NeuralNetwork net;
    std::vector<float> large_weights(game::ai::NeuralNetwork::kTotalWeights, 100.0f);
    net.set_weights(large_weights);

    std::array<float, game::ai::NeuralNetwork::kInputs> inputs{1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    const auto out = net.forward(inputs);

    EXPECT_GE(out.steering, -1.0f);
    EXPECT_LE(out.steering, 1.0f);
    EXPECT_NEAR(out.steering, 1.0f, 0.001f);
    EXPECT_NEAR(out.throttle, 1.0f, 0.001f);
}
