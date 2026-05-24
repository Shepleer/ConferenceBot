#pragma once

#include <tgbot/tgbot.h>

namespace ConferenceBot {
class CommandRegistry {
  using RouteProvider = std::function<void(const TgBot::Message::Ptr &message)>;

public:
  explicit CommandRegistry(TgBot::Bot &bot)
      : _bot(bot) {}
  void registerCommand(std::string_view command, RouteProvider provider);

private:
  TgBot::Bot &_bot;
  std::unordered_map<std::string, RouteProvider> _routes;
};
} // namespace ConferenceBot