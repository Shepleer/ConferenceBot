#include <string>

#include <ConferenceBot/BotServer/BotServer.hpp>
#include <ConferenceBot/BotServer/ConferenceBotController.hpp>
#include <ConferenceBot/BotServer/DbClientFactory.hpp>
#include <ConferenceBot/Config/ConfigProvider.hpp>
#include <ConferenceBot/Controllers/ExportController.hpp>

#include <drogon/drogon.h>
#include <tgbot/tgbot.h>

int main() {
  const ConferenceBot::Config config =
      ConferenceBot::ConfigProvider::getConfig();
  TgBot::Bot bot(config.botToken);

  auto dbClient = ConferenceBot::DbClientFactory::make();

  auto botController = std::make_shared<ConferenceBot::ConferenceBotController>(
      bot, dbClient, config
  );

  botController->registerHandlers();

  auto exportController =
      ConferenceBot::ExportController(dbClient, config.exportToken);

  drogon::app().registerHandler(
      "/export",
      [exportController](
          const drogon::HttpRequestPtr &request,
          std::function<void(const drogon::HttpResponsePtr &)> &&callback
      ) {
        drogon::async_run([exportController,
                           request,
                           callback = std::move(callback)]() mutable {
          return exportController.exportCsv(request, callback);
        });
      },
      {drogon::Get}
  );

  assert(dbClient);
    signal(SIGINT, [](int) {
      printf("SIGINT\n");
      exit(0);
    });

  std::thread webThread([]() {
    drogon::app().addListener("0.0.0.0", 8081).run();
  });
  ConferenceBot::BotServer server(bot, config);
  server.start();

  webThread.join();

  return 0;
}