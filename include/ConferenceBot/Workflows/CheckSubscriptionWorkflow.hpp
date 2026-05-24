#pragma once

#include <drogon/drogon.h>
#include <tgbot/Bot.h>

namespace ConferenceBot {
class CheckSubscriptionWorkflow {
public:
  explicit CheckSubscriptionWorkflow(TgBot::Bot &bot)
      : _bot(bot) {}
  drogon::Task<void> checkSubscription(
      const TgBot::CallbackQuery::Ptr &query,
      const std::string_view channelId
  );

private:
  TgBot::Bot &_bot;

  drogon::Task<void> showSubscribedUI(const TgBot::CallbackQuery::Ptr &query);
  drogon::Task<void>
  showNotSubscribedUI(const TgBot::CallbackQuery::Ptr &query);
};
} // namespace ConferenceBot