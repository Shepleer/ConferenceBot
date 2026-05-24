#include <string>
#include <thread>

#include <ConferenceBot/BotServer/BotServer.hpp>
#include <ConferenceBot/BotServer/ConferenceBotController.hpp>
#include <ConferenceBot/BotServer/DbClientFactory.hpp>
#include <ConferenceBot/Config/ConfigProvider.hpp>
#include <ConferenceBot/Controllers/ExportController.hpp>
#include <ConferenceBot/Net/DrogonHttpClient.hpp>

#include <drogon/drogon.h>
#include <tgbot/tgbot.h>

namespace {

constexpr uint16_t kWebhookPort = 8080;
constexpr uint16_t kExportPort = 8081;

trantor::Logger::LogLevel resolveLogLevel() {
  const char *envLevel = std::getenv("LOG_LEVEL");
  if (envLevel == nullptr) {
    return trantor::Logger::kInfo;
  }
  const std::string level(envLevel);
  if (level == "TRACE")
    return trantor::Logger::kTrace;
  if (level == "DEBUG")
    return trantor::Logger::kDebug;
  if (level == "INFO")
    return trantor::Logger::kInfo;
  if (level == "WARN")
    return trantor::Logger::kWarn;
  if (level == "ERROR")
    return trantor::Logger::kError;
  if (level == "FATAL")
    return trantor::Logger::kFatal;
  return trantor::Logger::kInfo;
}

} // namespace

int main() {
  drogon::app().setLogLevel(resolveLogLevel());
  drogon::app().setThreadNum(0);

  LOG_INFO << "[main] Starting ConferenceBot...";

  const ConferenceBot::Config config =
      ConferenceBot::ConfigProvider::getConfig();

  ConferenceBot::DrogonHttpClient httpClient;
  TgBot::Bot bot(config.botToken, httpClient);

  auto dbClient = ConferenceBot::DbClientFactory::make();
  if (!dbClient) {
    LOG_FATAL << "[main] Failed to create DB client. Exiting.";
    return 1;
  }

  auto botController = std::make_shared<ConferenceBot::ConferenceBotController>(
      bot,
      dbClient,
      config
  );

  botController->registerHandlers();
  LOG_INFO << "[main] Bot handlers registered.";

  ConferenceBot::BotServer botServer(bot, config);

  TgBot::TgTypeParser tgTypeParser;
  const std::string webhookPath = botServer.webhookPath();

  drogon::app().registerHandler(
      webhookPath,
      [&bot,
       &tgTypeParser](
          const drogon::HttpRequestPtr &request,
          std::function<void(const drogon::HttpResponsePtr &)> &&callback
      ) {
        auto ack = drogon::HttpResponse::newHttpResponse();
        ack->setStatusCode(drogon::HttpStatusCode::k200OK);
        callback(ack);

        std::string body{request->getBody()};
        drogon::async_run(
            [&bot, &tgTypeParser, body = std::move(body)]() -> drogon::Task<> {
              try {
                auto update = tgTypeParser.parseJsonAndGetUpdate(
                    tgTypeParser.parseJson(body)
                );
                bot.getEventHandler().handleUpdate(update);
              } catch (const std::exception &e) {
                LOG_ERROR << "[webhook] Failed to handle update: " << e.what();
              }
              co_return;
            }
        );
      },
      {drogon::Post}
  );
  LOG_INFO << "[main] Webhook route registered at POST " << webhookPath;

  auto exportController =
      ConferenceBot::ExportController(dbClient, config.exportToken);

  drogon::app().registerHandler(
      "/export",
      [exportController](
          const drogon::HttpRequestPtr &request,
          std::function<void(const drogon::HttpResponsePtr &)> &&callback
      ) {
        LOG_INFO << "[http] GET /export from "
                 << request->getPeerAddr().toIpPort();
        drogon::async_run([exportController,
                           request,
                           callback = std::move(callback)]() mutable {
          return exportController.exportCsv(request, callback);
        });
      },
      {drogon::Get}
  );
  LOG_INFO << "[main] HTTP route '/export' registered.";

  drogon::app().registerBeginningAdvice([&botServer] {
    std::thread([&botServer] { botServer.registerWebhook(); }).detach();
  });

  signal(SIGINT, [](int) {
    LOG_WARN << "[main] SIGINT received, shutting down.";
    drogon::app().quit();
  });
  signal(SIGTERM, [](int) {
    LOG_WARN << "[main] SIGTERM received, shutting down.";
    drogon::app().quit();
  });

  LOG_INFO << "[http] Drogon HTTP server listening on 0.0.0.0:"
           << kWebhookPort << " (webhook) and 0.0.0.0:" << kExportPort
           << " (export)";
  drogon::app()
      .addListener("0.0.0.0", kWebhookPort)
      .addListener("0.0.0.0", kExportPort)
      .run();

  LOG_INFO << "[main] Shutdown complete.";
  return 0;
}
