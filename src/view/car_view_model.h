#pragma once

#include "help/help_topic_table.h"
#include "help/help_view_model.h"

#include <engine/ui/command.h>
#include <engine/ui/view_model.h>

#include <array>
#include <memory>
#include <string>

namespace game {

// ViewModel exposing bindable properties and commands to cars.xml.
class CarViewModel final : public engine::ui::ViewModel {
public:
    CarViewModel();

    // Tabs
    engine::ui::Bindable<bool> tabSimulation{true};
    engine::ui::Bindable<bool> tabStats{false};
    engine::ui::Bindable<bool> tabHelp{false};
    engine::ui::Bindable<bool> tabTheory{false};
    engine::ui::MethodCommand showSimulation;
    engine::ui::MethodCommand showStats;
    engine::ui::MethodCommand showHelp;
    engine::ui::MethodCommand showTheory;

    // Locales
    engine::ui::Bindable<bool> localeIsEn{true};
    engine::ui::Bindable<bool> localeIsUk{false};
    engine::ui::MethodCommand localeEn;
    engine::ui::MethodCommand localeUk;

    // Simulation Controls
    engine::ui::Bindable<std::string> playPauseText;
    engine::ui::Bindable<std::string> toggleRaysText;
    engine::ui::MethodCommand togglePause;
    engine::ui::MethodCommand resetEvolution;
    engine::ui::MethodCommand nextTrack;
    engine::ui::MethodCommand mutateAll;
    engine::ui::MethodCommand setSpeed1x;
    engine::ui::MethodCommand setSpeed2x;
    engine::ui::MethodCommand setSpeed5x;
    engine::ui::MethodCommand setSpeed10x;
    engine::ui::MethodCommand toggleRays;

    // Live Metrics
    engine::ui::Bindable<std::string> generationText;
    engine::ui::Bindable<std::string> aliveCarsText;
    engine::ui::Bindable<std::string> bestFitnessText;
    engine::ui::Bindable<std::string> allTimeFitnessText;
    engine::ui::Bindable<std::string> trackSeedText;
    engine::ui::Bindable<std::string> leaderSpeedText;
    engine::ui::Bindable<std::string> fpsText;
    engine::ui::Bindable<std::string> statusMessage;

    // Help tab
    engine::ui::BindableList<std::shared_ptr<HelpNavItem>> helpNav;
    engine::ui::Bindable<float> helpScroll{0.0f};
    std::array<engine::ui::BindableList<std::shared_ptr<HelpTopicViewModel>>, kHelpTopicCount> helpTopic;
    std::array<engine::ui::Bindable<bool>, kHelpTopicCount> helpShown;
};

} // namespace game
