#pragma once

#include <tgbot/Bot.h>

namespace ConferenceBot {
class CheckSubscriptionWorkflow {
public:
  explicit CheckSubscriptionWorkflow(TgBot::Bot &bot)
      : _bot(bot) {}
  void checkSubscription(
      const TgBot::CallbackQuery::Ptr &query,
      const std::string_view channelId
  );

private:
  TgBot::Bot &_bot;

  void showSubscribedUI(const TgBot::CallbackQuery::Ptr &query);
  void showNotSubscribedUI(const TgBot::CallbackQuery::Ptr &query);
};
} // namespace ConferenceBot