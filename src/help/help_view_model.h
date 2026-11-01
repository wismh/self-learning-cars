#pragma once

#include <engine/ui/command.h>
#include <engine/ui/view_model.h>

#include <string>

namespace game {

// One row of the topic list on the left of the Help tab: a section heading, a topic, or the active topic.
class HelpNavItem final : public engine::ui::ViewModel {
public:
    HelpNavItem() {
        property(engine::ui::intern("title"), title);
        property(engine::ui::intern("isHeader"), isHeader);
        property(engine::ui::intern("isTopic"), isTopic);
        property(engine::ui::intern("isActive"), isActive);
        command(engine::ui::intern("select"), select);
    }

    engine::ui::Bindable<std::string> title;
    engine::ui::Bindable<bool> isHeader{false};
    engine::ui::Bindable<bool> isTopic{false};
    engine::ui::Bindable<bool> isActive{false};
    engine::ui::RelayCommand select;
};

// What a topic document binds: its tryIt button.
class HelpTopicViewModel final : public engine::ui::ViewModel {
public:
    HelpTopicViewModel() {
        command(engine::ui::intern("tryIt"), tryIt);
    }

    engine::ui::RelayCommand tryIt;
};

} // namespace game
