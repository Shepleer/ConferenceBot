#pragma once

#include <ConferenceBot/Config/Config.hpp>

#include <tgbot/tgbot.h>

namespace ConferenceBot {
class BotServer {
public:
  explicit BotServer(TgBot::Bot &bot, Config config);
  void start();

private:
  TgBot::Bot &_bot;
  Config _config;
};
} // namespace ConferenceBot