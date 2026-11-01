#include "view/car_view_model.h"

namespace game {

CarViewModel::CarViewModel() {
    using engine::ui::intern;

    property(intern("tabSimulation"), tabSimulation);
    property(intern("tabStats"), tabStats);
    property(intern("tabHelp"), tabHelp);
    property(intern("tabTheory"), tabTheory);
    command(intern("showSimulation"), showSimulation);
    command(intern("showStats"), showStats);
    command(intern("showHelp"), showHelp);
    command(intern("showTheory"), showTheory);

    property(intern("localeIsEn"), localeIsEn);
    property(intern("localeIsUk"), localeIsUk);
    command(intern("localeEn"), localeEn);
    command(intern("localeUk"), localeUk);

    property(intern("playPauseText"), playPauseText);
    property(intern("toggleRaysText"), toggleRaysText);
    command(intern("togglePause"), togglePause);
    command(intern("resetEvolution"), resetEvolution);
    command(intern("nextTrack"), nextTrack);
    command(intern("mutateAll"), mutateAll);
    command(intern("setSpeed1x"), setSpeed1x);
    command(intern("setSpeed2x"), setSpeed2x);
    command(intern("setSpeed5x"), setSpeed5x);
    command(intern("setSpeed10x"), setSpeed10x);
    command(intern("toggleRays"), toggleRays);

    property(intern("generationText"), generationText);
    property(intern("aliveCarsText"), aliveCarsText);
    property(intern("bestFitnessText"), bestFitnessText);
    property(intern("allTimeFitnessText"), allTimeFitnessText);
    property(intern("trackSeedText"), trackSeedText);
    property(intern("leaderSpeedText"), leaderSpeedText);
    property(intern("fpsText"), fpsText);
    property(intern("statusMessage"), statusMessage);

    property(intern("helpNav"), helpNav);
    property(intern("helpScroll"), helpScroll);
    for (std::size_t i = 0; i < kHelpTopicCount; ++i) {
        property(intern("helpTopic" + std::to_string(i)), helpTopic[i]);
        property(intern("helpShown" + std::to_string(i)), helpShown[i]);
    }
}

} // namespace game
