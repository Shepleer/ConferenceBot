#include <ConferenceBot/BotServer/BotServer.hpp>

#include <ConferenceBot/Resources/Constants.hpp>

#include <drogon/drogon.h>

#include <format>

namespace ConferenceBot {

BotServer::BotServer(TgBot::Bot &bot, Config config)
    : _bot(bot)
    , _config(std::move(config)) {}

std::string BotServer::webhookPath() const {
  return "/" + _config.botToken;
}

void BotServer::registerWebhook() {
  try {
    const std::string webhookUrl =
        std::format("{}/{}", _config.webhookUrl, _config.botToken);

    const auto me = _bot.getApi().getMe();
    LOG_INFO << "[bot] Authenticated as @" << me->username
             << " (id=" << me->id << ")";
    LOG_INFO << "[bot] Webhook URL: " << webhookUrl;

    auto allowedListPtr = std::make_shared<std::vector<std::string>>(
        ConferenceBot::Constants::ALLOWED_UPDATES
    );

    _bot.getApi().deleteWebhook();
    LOG_DEBUG << "[bot] Previous webhook deleted.";
    _bot.getApi().setWebhook(webhookUrl, nullptr, 40, allowedListPtr);
    LOG_INFO << "[bot] Webhook registered with Telegram (max_connections=40)";
  } catch (const TgBot::TgException &e) {
    LOG_ERROR << "[bot] TgException while registering webhook: " << e.what();
  } catch (const std::exception &e) {
    LOG_ERROR << "[bot] Unexpected exception while registering webhook: "
              << e.what();
  }
}

} // namespace ConferenceBot
