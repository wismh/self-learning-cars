#include <gtest/gtest.h>

#include "help/help_topic_table.h"

#include <string_view>
#include <unordered_set>

TEST(HelpTopicTableTest, ValidateTopicsAndSections) {
    EXPECT_EQ(game::kHelpTopicCount, 10);

    const std::unordered_set<std::string_view> expected_sections{
            "help.section.basics",
            "help.section.ai",
            "help.section.environment",
            "help.section.architecture",
    };

    for (std::size_t i = 0; i < game::kHelpTopicCount; ++i) {
        const auto& topic = game::kHelpTopics[i];
        EXPECT_NE(topic.section, nullptr);
        EXPECT_NE(topic.title, nullptr);

        const std::string_view section_sv(topic.section);
        const std::string_view title_sv(topic.title);

        EXPECT_FALSE(section_sv.empty());
        EXPECT_FALSE(title_sv.empty());
        EXPECT_TRUE(expected_sections.contains(section_sv));
        EXPECT_NE(topic.open, game::HelpOpen::None);
    }
}

TEST(HelpTopicTableTest, SectionsAreGroupedContiguously) {
    std::unordered_set<std::string_view> seen_sections;
    std::string_view current_section;

    for (std::size_t i = 0; i < game::kHelpTopicCount; ++i) {
        const std::string_view section = game::kHelpTopics[i].section;
        if (section != current_section) {
            EXPECT_FALSE(seen_sections.contains(section))
                    << "Section " << section << " was encountered again non-contiguously!";
            seen_sections.insert(section);
            current_section = section;
        }
    }
}
