#include "panels/simulation_panel.h"

#include "ecs/components.h"

#include <format>

namespace game {

SimulationPanel::SimulationPanel(
        engine::ecs::World& world,
        engine::AssetsDb& assets,
        CarViewModel& view_model,
        ecs::EvolutionManager& evolution,
        const Tr& tr)
    : world_(&world)
    , assets_(&assets)
    , vm_(&view_model)
    , evolution_(&evolution)
    , tr_(&tr) {
    bind_commands();

    evolution_->on_new_generation([this](int gen, uint32_t seed) {
        say_status("status.new_generation", {
                {"gen", std::to_string(gen)},
                {"seed", std::to_string(seed)},
        });
    });
}

void SimulationPanel::bind_commands() {
    vm_->showSimulation.bind_to<SimulationPanel, &SimulationPanel::show_simulation>(*this);
    vm_->showStats.bind_to<SimulationPanel, &SimulationPanel::show_stats>(*this);
    vm_->showHelp.bind_to<SimulationPanel, &SimulationPanel::show_help>(*this);
    vm_->showTheory.bind_to<SimulationPanel, &SimulationPanel::show_theory>(*this);

    vm_->togglePause.bind_to<SimulationPanel, &SimulationPanel::toggle_pause>(*this);
    vm_->resetEvolution.bind_to<SimulationPanel, &SimulationPanel::reset_evolution>(*this);
    vm_->nextTrack.bind_to<SimulationPanel, &SimulationPanel::next_track>(*this);
    vm_->mutateAll.bind_to<SimulationPanel, &SimulationPanel::mutate_all>(*this);
    vm_->toggleRays.bind_to<SimulationPanel, &SimulationPanel::toggle_rays>(*this);

    vm_->setSpeed1x.bind_to<SimulationPanel, &SimulationPanel::set_speed_1x>(*this);
    vm_->setSpeed2x.bind_to<SimulationPanel, &SimulationPanel::set_speed_2x>(*this);
    vm_->setSpeed5x.bind_to<SimulationPanel, &SimulationPanel::set_speed_5x>(*this);
    vm_->setSpeed10x.bind_to<SimulationPanel, &SimulationPanel::set_speed_10x>(*this);

    vm_->localeEn.bind_to<SimulationPanel, &SimulationPanel::use_english>(*this);
    vm_->localeUk.bind_to<SimulationPanel, &SimulationPanel::use_ukrainian>(*this);
}

void SimulationPanel::show_simulation() {
    vm_->tabSimulation = true;
    vm_->tabStats = false;
    vm_->tabHelp = false;
    vm_->tabTheory = false;
}

void SimulationPanel::show_stats() {
    vm_->tabSimulation = false;
    vm_->tabStats = true;
    vm_->tabHelp = false;
    vm_->tabTheory = false;
}

void SimulationPanel::show_help() {
    vm_->tabSimulation = false;
    vm_->tabStats = false;
    vm_->tabHelp = true;
    vm_->tabTheory = true;
}

void SimulationPanel::toggle_pause() {
    auto& config = world_->ctx<ecs::SimulationConfig>();
    config.paused = !config.paused;
    say_status(config.paused ? "status.paused" : "status.ready");
}

void SimulationPanel::reset_evolution() {
    evolution_->reset(*world_);
    say_status("status.ready");
}

void SimulationPanel::next_track() {
    evolution_->next_track(*world_);
}

void SimulationPanel::mutate_all() {
    evolution_->mutate_all(0.45f);
}

void SimulationPanel::toggle_rays() {
    auto& config = world_->ctx<ecs::SimulationConfig>();
    config.show_rays = !config.show_rays;
}

void SimulationPanel::use_english() {
    if (locale_callback_) {
        locale_callback_("en");
    }
}

void SimulationPanel::use_ukrainian() {
    if (locale_callback_) {
        locale_callback_("uk");
    }
}

void SimulationPanel::set_speed(float s) {
    auto& config = world_->ctx<ecs::SimulationConfig>();
    config.time_scale = s;
}

void SimulationPanel::say_status(std::string_view key, const Tr::Args& args) {
    vm_->statusMessage = (*tr_)(key, args);
}

void SimulationPanel::update() {
    const auto& config = world_->ctx<ecs::SimulationConfig>();
    const auto& stats = world_->ctx<ecs::SimulationStats>();

    vm_->playPauseText = (*tr_)(config.paused ? "ui.btn.play" : "ui.btn.pause");
    vm_->toggleRaysText = (*tr_)(config.show_rays ? "ui.btn.rays_hide" : "ui.btn.rays_show");

    vm_->generationText = std::format("Gen #{}", config.current_generation);
    vm_->aliveCarsText = std::format("{}/{}", config.alive_cars_count, config.population_size);
    vm_->bestFitnessText = std::format("{:.1f}", config.best_fitness_current_gen);
    vm_->allTimeFitnessText = std::format("{:.1f}", config.all_time_best_fitness);
    vm_->trackSeedText = std::format("#{}", config.track_seed);
    vm_->leaderSpeedText = std::format("{:.0f} px/s", config.leader_speed);
    vm_->fpsText = std::format("{:.0f}", stats.fps);
}

} // namespace game
