#pragma once

#include <ConferenceBot/Config/Config.hpp>

namespace ConferenceBot {
class ConfigProvider {
public:
  static const Config getConfig();
};
} // namespace ConferenceBot