#include <ConferenceBot/Controllers/ExportController.hpp>

namespace ConferenceBot {
namespace {

std::string escapeCsv(std::string value) {
  std::erase_if(value, [](char c) {
    return c == '\n' || c == '\t' || c == '\r';
  });
  std::replace(value.begin(), value.end(), '"', '\'');

  return std::format("\"{}\"", value);
}
} // namespace

ExportController::ExportController(
    drogon::orm::DbClientPtr &dbClient,
    std::string exportToken
)
    : _mapper(
          drogon::orm::CoroMapper<drogon_model::sqlite3::Registrations>(
              dbClient
          )
      )
    , _exportToken(std::move(exportToken)) {}

drogon::Task<void> ExportController::exportCsv(
    const drogon::HttpRequestPtr &request,
    const std::function<void(const drogon::HttpResponsePtr &)> &callback
) {
  const std::string peer = request->getPeerAddr().toIpPort();
  const auto token = request->getParameter("token");

  if (_exportToken != token) {
    LOG_WARN << "[export] Forbidden: invalid token from " << peer
             << " (tokenProvided=" << (token.empty() ? "no" : "yes") << ")";
    drogon::HttpResponsePtr response =
        drogon::HttpResponse::newHttpResponse();
    response->setStatusCode(drogon::HttpStatusCode::k403Forbidden);

    callback(response);
    co_return;
  }

  LOG_INFO << "[export] Authorized request from " << peer
           << "; generating CSV...";
  auto csv = co_await generateCsv();

  auto response = drogon::HttpResponse::newHttpResponse();
  response->setStatusCode(drogon::HttpStatusCode::k200OK);
  response->setContentTypeString("text/csv");
  response->addHeader(
      "Content-Disposition",
      "attachment; filename=registrations.csv"
  );
  response->addHeader("Cache-Control", "no-store");
  response->setBody(csv);

  LOG_INFO << "[export] Responding with CSV (bytes=" << csv.size() << ") to "
           << peer;
  callback(response);
}

drogon::Task<std::string> ExportController::generateCsv() {
  std::stringstream csv;

  csv << "Name,"
      << "Company,"
      << "Company Position,"
      << "Phrase,"
      << "Telegram Nickname\n";

  try {
    auto rows = co_await _mapper.findAll();
    LOG_INFO << "[export] Fetched " << rows.size() << " registration rows";

    for (const auto &row : rows) {
      csv << escapeCsv(row.getValueOfName()) << ",";
      csv << escapeCsv(row.getValueOfCompany()) << ",";
      csv << escapeCsv(row.getValueOfCompanyPosition()) << ",";
      csv << escapeCsv(row.getValueOfPhrase()) << ",";
      csv << escapeCsv(row.getValueOfTelegramNickname()) << "\n";
    }
  } catch (const drogon::orm::SqlError &e) {
    LOG_ERROR << "[export] SQL error while fetching registrations: "
              << e.what();
    co_return csv.str();
  }

  co_return csv.str();
}

} // namespace ConferenceBot