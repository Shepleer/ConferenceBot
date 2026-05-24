#pragma once

#include <string>

namespace ConferenceBot {
struct Config {
  const std::string botToken;
  const std::string channelId;
  const std::string webhookUrl;
  const std::string exportToken;
};
} // namespace ConferenceBot