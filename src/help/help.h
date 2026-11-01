#pragma once

#include "help/help_topic_table.h"
#include "help/help_view_model.h"
#include "loc/tr.h"

#include <array>
#include <memory>
#include <vector>

namespace game {

class CarViewModel;

// Coordinates the Help tab: the topic directory, active article view, and action navigation.
class Help {
public:
    Help(CarViewModel& view, const Tr& tr);

    Help(const Help&) = delete;
    Help& operator=(const Help&) = delete;

    void apply_locale();

private:
    void select(std::size_t topic);
    void open(HelpOpen tab);

    CarViewModel& view_;
    const Tr& tr_;

    std::vector<std::shared_ptr<HelpNavItem>> nav_;
    std::array<std::size_t, kHelpTopicCount> nav_row_{};
    std::size_t selected_ = 0;
};

} // namespace game
