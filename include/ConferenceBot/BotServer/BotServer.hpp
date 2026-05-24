#pragma once

#include <ConferenceBot/Config/Config.hpp>

#include <tgbot/tgbot.h>

namespace ConferenceBot {

/// Sets up the Telegram webhook for the bot. All incoming updates are
/// received by Drogon's HTTP listener (see main.cpp); this class is only
/// responsible for telling Telegram where to deliver them.
class BotServer {
public:
  explicit BotServer(TgBot::Bot &bot, Config config);

  /// Calls Telegram setWebhook so the bot starts receiving updates at
  /// <webhookUrl>/<botToken>. Safe to call from any non-event-loop thread;
  /// blocks until Telegram acknowledges.
  void registerWebhook();

  /// Path (with leading '/') that the Drogon HTTP server should listen on
  /// to receive Telegram updates for this bot.
  std::string webhookPath() const;

private:
  TgBot::Bot &_bot;
  Config _config;
};

} // namespace ConferenceBot
