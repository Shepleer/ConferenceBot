#pragma once

#include <ConferenceBot/Config/Config.hpp>
#include <Registrations.h>

#include <drogon/drogon.h>

namespace ConferenceBot {
class ExportController {
public:
  explicit ExportController(
      drogon::orm::DbClientPtr &dbClient,
      std::string exportToken
  );

  drogon::Task<void> exportCsv(
      const drogon::HttpRequestPtr &request,
      const std::function<void(const drogon::HttpResponsePtr &)> &callback
  );

private:
  std::string _exportToken;
  drogon::orm::CoroMapper<drogon_model::sqlite3::Registrations> _mapper;

  drogon::Task<std::string> generateCsv();
};
} // namespace ConferenceBot