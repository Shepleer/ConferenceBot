#include <ConferenceBot/Services/RegistrationRepository.hpp>

namespace ConferenceBot {

RegistrationRepository::RegistrationRepository(drogon::orm::DbClientPtr client)
    : _client(client), _mapper(client) {
        assert(client);
    }

drogon::Task<std::optional<drogon_model::sqlite3::Registrations>>
RegistrationRepository::findByChatId(int64_t chatId) {
  try {
    assert(_client);
    auto reg = co_await _mapper.findOne(
        drogon::orm::Criteria(
            drogon_model::sqlite3::Registrations::Cols::_chat_id,
            drogon::orm::CompareOperator::EQ,
            chatId
        )
    );
    LOG_DEBUG << "[repo] findByChatId hit chat=" << chatId;
    co_return reg;
  } catch (const drogon::orm::UnexpectedRows &e) {
    LOG_DEBUG << "[repo] findByChatId miss chat=" << chatId;
    co_return std::nullopt;
  } catch (const drogon::orm::DrogonDbException &e) {
    LOG_ERROR << "[repo] findByChatId failed chat=" << chatId << ": "
              << e.base().what();
    throw;
  }
}

drogon::Task<drogon_model::sqlite3::Registrations> RegistrationRepository::save(
    const drogon_model::sqlite3::Registrations &registration
) {
    assert(this);
    assert(_client);
  try {
    auto reg = co_await _mapper.insert(registration);
    LOG_INFO << "[repo] Inserted registration chat="
             << registration.getValueOfChatId();
    co_return reg;
  } catch (const drogon::orm::DrogonDbException &e) {
    LOG_ERROR << "[repo] save failed chat="
              << registration.getValueOfChatId() << ": " << e.base().what();
    throw;
  }
}

drogon::Task<void> RegistrationRepository::update(
    const drogon_model::sqlite3::Registrations &registration
) {
  try {
    co_await _mapper.update(registration);
    LOG_DEBUG << "[repo] Updated registration chat="
              << registration.getValueOfChatId()
              << " state=" << registration.getValueOfState();
  } catch (const drogon::orm::DrogonDbException &e) {
    LOG_ERROR << "[repo] update failed chat="
              << registration.getValueOfChatId() << ": " << e.base().what();
    throw;
  }
}

drogon::Task<void> RegistrationRepository::remove(int64_t chatId) {
  try {
    co_await _mapper.deleteByPrimaryKey(chatId);
    LOG_INFO << "[repo] Removed registration chat=" << chatId;
  } catch (const drogon::orm::DrogonDbException &e) {
    LOG_ERROR << "[repo] remove failed chat=" << chatId << ": "
              << e.base().what();
    throw;
  }
}

} // namespace ConferenceBot