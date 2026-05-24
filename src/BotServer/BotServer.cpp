#include <ConferenceBot/BotServer/BotServer.hpp>

#include <ConferenceBot/Resources/Constants.hpp>

namespace ConferenceBot {

BotServer::BotServer(TgBot::Bot &bot, Config config)
    : _bot(bot)
    , _config(config) {}

void BotServer::start() {
  try {
    const std::string webhookUrl =
        std::format("{}/{}", _config.webhookUrl, _config.botToken);
    TgBot::TgWebhookTcpServer webhookServer(8080, _bot);

    printf("Bot username: %s\n", _bot.getApi().getMe()->username.c_str());
    printf("Server starting...\n");
    printf("Webhook URL: %s\n", webhookUrl.c_str());

    auto allowedListPtr = std::make_shared<std::vector<std::string>>(
        ConferenceBot::Constants::ALLOWED_UPDATES
    );

    _bot.getApi().deleteWebhook();
    _bot.getApi().setWebhook(webhookUrl, nullptr, 40, allowedListPtr);

    webhookServer.start();
  } catch (TgBot::TgException &e) {
    printf("error: %s\n", e.what());
  }
}

} // namespace ConferenceBot