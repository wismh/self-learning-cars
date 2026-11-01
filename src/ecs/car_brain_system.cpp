#include "ecs/car_brain_system.h"

#include "ecs/components.h"

#include <algorithm>
#include <array>

namespace game::ecs {

void update_car_brains(engine::ecs::World& world) {
    auto view = world.view<Car, RaycastSensors, Brain>();

    for (engine::ecs::Entity e : view) {
        auto& car = view.get<Car>(e);
        if (!car.is_alive) {
            continue;
        }

        const auto& sensors = view.get<RaycastSensors>(e);
        const auto& brain = view.get<Brain>(e);

        // 8 inputs: 7 raycast distances [0, 1] + normalized forward speed [-1, 1]
        std::array<float, ai::NeuralNetwork::kInputs> inputs{};
        for (std::size_t i = 0; i < RaycastSensors::kRayCount; ++i) {
            inputs[i] = sensors.distances[i];
        }
        inputs[RaycastSensors::kRayCount] = std::clamp(car.velocity / car.max_speed, -1.0f, 1.0f);

        const auto output = brain.network.forward(inputs);
        car.steering = std::clamp(output.steering, -1.0f, 1.0f);
        car.throttle = std::clamp(output.throttle, -1.0f, 1.0f);
    }
}

} // namespace game::ecs
