#pragma once

#include <tgbot/Bot.h>

namespace ConferenceBot {
class CallbackQueryRouter {
  using RouteProvider =
      std::function<void(const TgBot::CallbackQuery::Ptr &query)>;

public:
  void registerHandler(TgBot::Bot &bot);

  template <typename T>
    requires std::invocable<T, const TgBot::CallbackQuery::Ptr &>
  void registerRoute(std::string_view route, T &&provider) {
    _routes.emplace(route, std::forward<T>(provider));
  };

private:
  std::unordered_map<std::string, RouteProvider> _routes;
};
} // namespace ConferenceBot