#include <ConferenceBot/BotServer/DbClientFactory.hpp>

namespace ConferenceBot {
drogon::orm::DbClientPtr DbClientFactory::make() {
  return drogon::orm::DbClient::newSqlite3Client(
      std::format("filename={}", "conference.db"),
      1
  );
}
} // namespace ConferenceBot