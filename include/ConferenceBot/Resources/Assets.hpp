#pragma once

#include <filesystem>

namespace ConferenceBot::Assets {

inline std::string asset(std::string_view path) {
  return (std::filesystem::path(ASSETS_DIR) / path).string();
};

} // namespace ConferenceBot