#pragma once

#include <Registrations.h>

namespace ConferenceBot {
class RegistrationRepository {
public:
  explicit RegistrationRepository(drogon::orm::DbClientPtr client);

  drogon::Task<std::optional<drogon_model::sqlite3::Registrations>>
  findByChatId(int64_t chatId);

  drogon::Task<drogon_model::sqlite3::Registrations>
  save(const drogon_model::sqlite3::Registrations &registration);
  drogon::Task<void>
  update(const drogon_model::sqlite3::Registrations &registration);
  drogon::Task<void> remove(int64_t chatId);

private:
  drogon::orm::DbClientPtr _client;
  drogon::orm::CoroMapper<drogon_model::sqlite3::Registrations> _mapper;
};
} // namespace ConferenceBot