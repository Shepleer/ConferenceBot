#include <ConferenceBot/BotServer/DbClientFactory.hpp>

#include <drogon/drogon.h>

namespace ConferenceBot {
drogon::orm::DbClientPtr DbClientFactory::make() {
  constexpr const char *kDbFile = "conference.db";
  LOG_INFO << "[db] Creating SQLite3 client (file=" << kDbFile
           << ", pool=4)";

  auto client = drogon::orm::DbClient::newSqlite3Client(
      std::format("filename={}", kDbFile),
      4
  );

  if (client) {
    LOG_INFO << "[db] SQLite3 client ready.";
  } else {
    LOG_ERROR << "[db] Failed to create SQLite3 client for " << kDbFile;
  }

  return client;
}
} // namespace ConferenceBot