#pragma once

#include "ecs/evolution_system.h"
#include "help/help.h"
#include "loc/tr.h"
#include "panels/simulation_panel.h"
#include "view/car_view_model.h"

#include <engine/core/engine_services.h>
#include <engine/core/input_system.h>
#include <engine/igame.h>
#include <engine/resources/assets_db.h>

#include <filesystem>
#include <memory>
#include <optional>
#include <string_view>

namespace game {

// Main game lifecycle owning the simulation world, UI canvas, and input bindings.
class Game final : public engine::GameBase {
public:
    explicit Game(const engine::EngineServices& services);

    [[nodiscard]] engine::WindowDesc primary_window() const override;
    [[nodiscard]] std::optional<engine::AssetId> window_icon() const override;
    void on_start() override;

private:
    void load_locale();
    void set_locale(std::string_view tag);
    void update_input();

    engine::AssetsDb* assets_;
    engine::InputSystem* input_;
    std::filesystem::path preferences_directory_;
    std::optional<Tr> tr_;
    std::shared_ptr<CarViewModel> view_;
    std::optional<ecs::EvolutionManager> evolution_;
    std::optional<SimulationPanel> panel_;
    std::optional<Help> help_;
    engine::ecs::Entity screen_{};

    engine::ActionId pause_action_{};
    engine::ActionId reset_action_{};
    engine::ActionId next_track_action_{};
    engine::ActionId toggle_rays_action_{};
};

} // namespace game
