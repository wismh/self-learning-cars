#include <gtest/gtest.h>

#include "loc/locale_prefs.h"

#include <filesystem>
#include <fstream>

namespace {

class LocalePrefsTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_dir_ = std::filesystem::temp_directory_path() / "self_learning_cars_test_prefs";
        std::error_code ec;
        std::filesystem::remove_all(test_dir_, ec);
        std::filesystem::create_directories(test_dir_, ec);
    }

    void TearDown() override {
        std::error_code ec;
        std::filesystem::remove_all(test_dir_, ec);
    }

    std::filesystem::path test_dir_;
};

} // namespace

TEST_F(LocalePrefsTest, EmptyDirectoryReturnsFallback) {
    const std::string result = game::read_locale({}, "uk");
    EXPECT_EQ(result, "uk");
}

TEST_F(LocalePrefsTest, NonExistentFileReturnsFallback) {
    const std::string result = game::read_locale(test_dir_ / "nonexistent", "en");
    EXPECT_EQ(result, "en");
}

TEST_F(LocalePrefsTest, InvalidContentReturnsFallback) {
    const auto file_path = test_dir_ / "locale.txt";
    {
        std::ofstream out(file_path);
        out << "es\n";
    }

    const std::string result = game::read_locale(test_dir_, "uk");
    EXPECT_EQ(result, "uk");
}

TEST_F(LocalePrefsTest, WriteAndReadRoundtrip) {
    game::write_locale(test_dir_, "en");
    EXPECT_EQ(game::read_locale(test_dir_, "uk"), "en");

    game::write_locale(test_dir_, "uk");
    EXPECT_EQ(game::read_locale(test_dir_, "en"), "uk");
}

TEST_F(LocalePrefsTest, WriteWithEmptyDirectoryDoesNotCrash) {
    EXPECT_NO_THROW(game::write_locale({}, "en"));
}
