#include <ConferenceBot/BotServer/BotServer.hpp>

#include <ConferenceBot/Resources/Constants.hpp>

#include <drogon/drogon.h>

namespace ConferenceBot {
namespace {

constexpr uint16_t kWebhookPort = 8080;

} // namespace

BotServer::BotServer(TgBot::Bot &bot, Config config)
    : _bot(bot)
    , _config(config) {}

void BotServer::start() {
  try {
    const std::string webhookUrl =
        std::format("{}/{}", _config.webhookUrl, _config.botToken);
    TgBot::TgWebhookTcpServer webhookServer(kWebhookPort, _bot);

    const auto me = _bot.getApi().getMe();
    LOG_INFO << "[bot] Authenticated as @" << me->username
             << " (id=" << me->id << ")";
    LOG_INFO << "[bot] Webhook URL: " << webhookUrl;
    LOG_INFO << "[bot] Webhook listener starting on 0.0.0.0:" << kWebhookPort;

    auto allowedListPtr = std::make_shared<std::vector<std::string>>(
        ConferenceBot::Constants::ALLOWED_UPDATES
    );

    _bot.getApi().deleteWebhook();
    LOG_DEBUG << "[bot] Previous webhook deleted.";
    _bot.getApi().setWebhook(webhookUrl, nullptr, 40, allowedListPtr);
    LOG_INFO << "[bot] Webhook registered with Telegram (max_connections=40)";

    webhookServer.start();
  } catch (TgBot::TgException &e) {
    LOG_ERROR << "[bot] TgException while starting bot server: " << e.what();
  } catch (const std::exception &e) {
    LOG_ERROR << "[bot] Unexpected exception while starting bot server: "
              << e.what();
  }
}

} // namespace ConferenceBot