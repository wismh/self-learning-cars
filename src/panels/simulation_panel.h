#pragma once

#include "ecs/evolution_system.h"
#include "loc/tr.h"
#include "view/car_view_model.h"

#include <engine/ecs/world.h>
#include <engine/resources/assets_db.h>

#include <functional>
#include <memory>
#include <string_view>

namespace game {

// HUD panel managing UI commands and metrics synchronization between ECS and ViewModel.
class SimulationPanel {
public:
    using LocaleChangeCallback = std::function<void(std::string_view)>;

    SimulationPanel(
            engine::ecs::World& world,
            engine::AssetsDb& assets,
            CarViewModel& view_model,
            ecs::EvolutionManager& evolution,
            const Tr& tr);

    void update();
    void say_status(std::string_view key, const Tr::Args& args = {});

    void toggle_pause();
    void reset_evolution();
    void next_track();
    void mutate_all();
    void toggle_rays();

    void use_english();
    void use_ukrainian();

    void on_locale_change(LocaleChangeCallback callback) {
        locale_callback_ = std::move(callback);
    }

private:
    void bind_commands();

    void show_simulation();
    void show_stats();
    void show_help();
    void show_theory() { show_help(); }

    void set_speed_1x() { set_speed(1.0f); }
    void set_speed_2x() { set_speed(2.0f); }
    void set_speed_5x() { set_speed(5.0f); }
    void set_speed_10x() { set_speed(10.0f); }
    void set_speed(float s);

    engine::ecs::World* world_;
    engine::AssetsDb* assets_;
    CarViewModel* vm_;
    ecs::EvolutionManager* evolution_;
    const Tr* tr_;
    LocaleChangeCallback locale_callback_{};
};

} // namespace game
