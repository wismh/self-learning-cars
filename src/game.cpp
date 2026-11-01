#include "game.h"

#include <asset_ids.h>

#include "ecs/components.h"
#include "ecs/simulation_systems.h"
#include "ecs/spawner.h"
#include "loc/locale_prefs.h"

#include <engine/core/platform.h>
#include <engine/ecs/events.h>
#include <engine/loc/catalog.h>
#include <engine/log.h>
#include <engine/ui/canvas.h>

namespace game {

namespace {

constexpr glm::vec2 kDesignSize{1600.0f, 900.0f};

} // namespace

Game::Game(const engine::EngineServices& services)
    : engine::GameBase(services.worlds)
    , assets_(&services.assets)
    , input_(&services.input) {}

engine::WindowDesc Game::primary_window() const {
    return engine::WindowDesc{
            .title = "Self-Learning Cars — Neuroevolution 2D",
            .size = {static_cast<int>(kDesignSize.x), static_cast<int>(kDesignSize.y)},
            .style = {.resizable = true},
    };
}

std::optional<engine::AssetId> Game::window_icon() const {
    return assets::textures::app_icon;
}

void Game::on_start() {
    if (const auto directory = engine::user_data_directory("wismh", "self-learning-cars")) {
        preferences_directory_ = *directory;
    } else {
        engine::log::warn("no user data directory found");
    }

    load_locale();

    // 1. Initialize world singletons
    world().ctx<ecs::SimulationConfig>();
    world().ctx<ecs::SimulationStats>();

    // 2. Setup Camera and Ray visualizers
    ecs::spawner::setup_camera(world());
    ecs::spawner::setup_ray_visualizers(world(), *assets_);

    // 3. Initialize Evolution manager and spawn track & cars
    evolution_.emplace(*assets_, 60);
    evolution_->reset(world());

    // 4. Register Simulation Systems
    ecs::register_simulation_systems(world(), *evolution_);

    // 5. Input Bindings
    pause_action_ = input_->intern("pause");
    input_->bind(engine::KeyCode::Space, pause_action_);

    reset_action_ = input_->intern("reset");
    input_->bind(engine::KeyCode::R, reset_action_);

    next_track_action_ = input_->intern("next_track");
    input_->bind(engine::KeyCode::N, next_track_action_);

    toggle_rays_action_ = input_->intern("toggle_rays");
    input_->bind(engine::KeyCode::V, toggle_rays_action_);

    // 6. Setup ViewModel, Panels
    view_ = std::make_shared<CarViewModel>();
    const auto& catalog = world().ctx<engine::loc::Catalog>();
    view_->localeIsEn = catalog.active() == "en";
    view_->localeIsUk = catalog.active() == "uk";
    panel_.emplace(world(), *assets_, *view_, *evolution_, *tr_);
    panel_->on_locale_change([this](std::string_view tag) { set_locale(tag); });
    help_.emplace(*view_, *tr_);
    panel_->say_status("status.ready");

    // 7. Setup UI Canvas
    engine::ui::UiCanvas canvas;
    canvas.document = assets::ui::cars;
    canvas.stylesheet = assets::css::cars;
    canvas.fit = engine::ui::UiFit::ScaleWithScreenSize;
    canvas.reference_size = kDesignSize;
    canvas.data_context = view_;
    screen_ = world().create();
    world().emplace<engine::ui::UiCanvas>(screen_, std::move(canvas));

    // 8. Frame Update System (input and panel update)
    world().add_system(engine::ecs::Schedule::Frame, engine::ecs::Phase::Game, [this](engine::ecs::World&) {
        update_input();
        panel_->update();
    });

    engine::log::info("self-learning cars simulation initialized successfully");
}

void Game::load_locale() {
    auto& catalog = world().ctx<engine::loc::Catalog>();
    catalog.add(*assets_->get<engine::loc::StringTable>(assets::locale::en), engine::loc::Role::Source);
    catalog.add(*assets_->get<engine::loc::StringTable>(assets::locale::uk), engine::loc::Role::Translation);

    const std::string saved = read_locale(preferences_directory_, "uk");
    catalog.set_active(saved);
    tr_.emplace(catalog);
}

void Game::set_locale(std::string_view tag) {
    auto& catalog = world().ctx<engine::loc::Catalog>();
    catalog.set_active(std::string{tag});
    write_locale(preferences_directory_, tag);

    view_->localeIsEn = tag == "en";
    view_->localeIsUk = tag == "uk";
    panel_->update();
    help_->apply_locale();
}

void Game::update_input() {
    engine::ecs::EventReader<engine::InputEvent> reader(
            world(), world().ctx<engine::ecs::EventCursor<engine::InputEvent>>());
    for (const auto& ev : reader) {
        if (ev.kind == engine::InputEvent::Kind::Down) {
            if (ev.action == pause_action_) {
                panel_->toggle_pause();
            } else if (ev.action == reset_action_) {
                panel_->reset_evolution();
            } else if (ev.action == next_track_action_) {
                panel_->next_track();
            } else if (ev.action == toggle_rays_action_) {
                panel_->toggle_rays();
            }
        }
    }
}

} // namespace game
