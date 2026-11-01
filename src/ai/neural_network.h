#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <span>

namespace game::ai {

// Multi-layer perceptron with 8 inputs, 12 hidden neurons, and 2 outputs.
// Activation is hyperbolic tangent for smooth $[-1, 1]$ vehicle control.
class NeuralNetwork {
public:
    static constexpr std::size_t kInputs = 8;
    static constexpr std::size_t kHidden = 12;
    static constexpr std::size_t kOutputs = 2;

    static constexpr std::size_t kWeights1 = kInputs * kHidden;
    static constexpr std::size_t kBiases1 = kHidden;
    static constexpr std::size_t kWeights2 = kHidden * kOutputs;
    static constexpr std::size_t kBiases2 = kOutputs;

    static constexpr std::size_t kTotalWeights = kWeights1 + kBiases1 + kWeights2 + kBiases2;

    struct Output {
        float steering = 0.0f; // [-1.0 (left), 1.0 (right)]
        float throttle = 0.0f; // [-1.0 (brake), 1.0 (full forward)]
    };

    NeuralNetwork() {
        weights_.fill(0.0f);
    }

    [[nodiscard]] std::span<float> weights() {
        return weights_;
    }

    [[nodiscard]] std::span<const float> weights() const {
        return weights_;
    }

    void set_weights(std::span<const float> src) {
        const std::size_t count = std::min(weights_.size(), src.size());
        std::copy_n(src.data(), count, weights_.data());
    }

    [[nodiscard]] Output forward(std::span<const float, kInputs> inputs) const {
        std::array<float, kHidden> hidden{};

        // Layer 1: Inputs -> Hidden
        std::size_t offset = 0;
        for (std::size_t h = 0; h < kHidden; ++h) {
            float sum = 0.0f;
            for (std::size_t i = 0; i < kInputs; ++i) {
                sum += inputs[i] * weights_[offset++];
            }
            sum += weights_[kWeights1 + h]; // Bias
            hidden[h] = std::tanh(sum);
        }

        // Layer 2: Hidden -> Outputs
        offset = kWeights1 + kBiases1;
        std::array<float, kOutputs> raw_outputs{};
        for (std::size_t o = 0; o < kOutputs; ++o) {
            float sum = 0.0f;
            for (std::size_t h = 0; h < kHidden; ++h) {
                sum += hidden[h] * weights_[offset++];
            }
            sum += weights_[kWeights1 + kBiases1 + kWeights2 + o]; // Bias
            raw_outputs[o] = std::tanh(sum);
        }

        return Output{
                .steering = raw_outputs[0],
                .throttle = raw_outputs[1],
        };
    }

private:
    std::array<float, kTotalWeights> weights_{};
};

} // namespace game::ai
