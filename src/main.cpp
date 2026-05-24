#include <string>

#include <ConferenceBot/BotServer/BotServer.hpp>
#include <ConferenceBot/BotServer/ConferenceBotController.hpp>
#include <ConferenceBot/BotServer/DbClientFactory.hpp>
#include <ConferenceBot/Config/ConfigProvider.hpp>
#include <ConferenceBot/Controllers/ExportController.hpp>

#include <drogon/drogon.h>
#include <tgbot/tgbot.h>

namespace {

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
  TgBot::Bot bot(config.botToken);

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

  signal(SIGINT, [](int) {
    LOG_WARN << "[main] SIGINT received, shutting down.";
    exit(0);
  });
  signal(SIGTERM, [](int) {
    LOG_WARN << "[main] SIGTERM received, shutting down.";
    exit(0);
  });

  std::thread webThread([]() {
    LOG_INFO << "[http] Drogon HTTP server listening on 0.0.0.0:8081";
    drogon::app().addListener("0.0.0.0", 8081).run();
  });

  ConferenceBot::BotServer server(bot, config);
  server.start();

  webThread.join();

  LOG_INFO << "[main] Shutdown complete.";
  return 0;
}