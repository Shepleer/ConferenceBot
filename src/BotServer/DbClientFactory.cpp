#include <ConferenceBot/BotServer/DbClientFactory.hpp>

#include <drogon/drogon.h>

namespace ConferenceBot {
drogon::orm::DbClientPtr DbClientFactory::make() {
  constexpr const char *kDbFile = "conference.db";
  constexpr std::size_t kPoolSize = 16;
  LOG_INFO << "[db] Creating SQLite3 client (file=" << kDbFile
           << ", pool=" << kPoolSize << ")";

  auto client = drogon::orm::DbClient::newSqlite3Client(
      std::format("filename={}", kDbFile),
      kPoolSize
  );

  if (client) {
    LOG_INFO << "[db] SQLite3 client ready.";
  } else {
    LOG_ERROR << "[db] Failed to create SQLite3 client for " << kDbFile;
  }

  return client;
}
} // namespace ConferenceBot