#include <doctest/doctest.h>

#include <cstdio>
#include <filesystem>
#include <fstream>

#include "config/config.h"

using namespace SW::Config;

namespace {
    // RAII helper: writes `contents` to a uniquely-named temp file and
    // removes it on scope exit, so each test gets an isolated ini path
    // instead of touching the real Scenewright.ini default.
    struct TempIniFile {
        std::filesystem::path path = std::filesystem::temp_directory_path() /
                                      ("scenewright_config_test_" + std::to_string(std::rand()) + ".ini");

        explicit TempIniFile(const std::string& contents) {
            std::ofstream(path) << contents;
        }
        ~TempIniFile() { std::filesystem::remove(path); }
    };
}

TEST_CASE("IsDevModeEnabled returns true when explicitly enabled") {
    TempIniFile ini("[Debug]\nEnableLiveLink=1\n");
    CHECK(IsDevModeEnabled(ini.path.string()));
}

TEST_CASE("IsDevModeEnabled defaults to false when the key is absent") {
    TempIniFile ini("[Debug]\n");
    CHECK_FALSE(IsDevModeEnabled(ini.path.string()));
}

TEST_CASE("IsDevModeEnabled defaults to false when the file doesn't exist") {
    CHECK_FALSE(IsDevModeEnabled("/nonexistent/path/Scenewright.ini"));
}

TEST_CASE("IsDevModeEnabled returns false when explicitly disabled") {
    TempIniFile ini("[Debug]\nEnableLiveLink=0\n");
    CHECK_FALSE(IsDevModeEnabled(ini.path.string()));
}
