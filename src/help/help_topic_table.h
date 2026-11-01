#pragma once

#include <array>
#include <cstddef>

namespace game {

enum class HelpOpen { None, Simulation, Stats };

struct HelpTopicInfo {
    const char* section;
    const char* title;
    HelpOpen open;
};

inline constexpr std::size_t kHelpTopicCount = 10;

inline constexpr std::array<HelpTopicInfo, kHelpTopicCount> kHelpTopics{{
    {"help.section.basics", "help.intro.title", HelpOpen::Simulation},
    {"help.section.basics", "help.sensors.title", HelpOpen::Simulation},
    {"help.section.basics", "help.kinematics.title", HelpOpen::Simulation},
    {"help.section.ai", "help.neural_network.title", HelpOpen::Stats},
    {"help.section.ai", "help.fitness.title", HelpOpen::Simulation},
    {"help.section.ai", "help.genetics.title", HelpOpen::Simulation},
    {"help.section.environment", "help.spline_generation.title", HelpOpen::Simulation},
    {"help.section.environment", "help.road_mesh.title", HelpOpen::Simulation},
    {"help.section.environment", "help.checkpoints.title", HelpOpen::Simulation},
    {"help.section.architecture", "help.ecs.title", HelpOpen::Simulation},
}};

} // namespace game
