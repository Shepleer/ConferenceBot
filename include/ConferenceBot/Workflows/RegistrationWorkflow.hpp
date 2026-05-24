#pragma once

#include <ConferenceBot/Services/RegistrationRepository.hpp>
#include <tgbot/tgbot.h>

namespace ConferenceBot {
class RegistrationWorkflow {
public:
  explicit RegistrationWorkflow(
      TgBot::Bot &bot,
      RegistrationRepository &repository
  )
      : _bot(bot)
      , _repository(repository) {}
  drogon::Task<void>
  answerWantParticipateQuery(const TgBot::CallbackQuery::Ptr &query);

  drogon::Task<void> processMessage(TgBot::Message::Ptr message);
  drogon::Task<void> replyBackQuery(TgBot::CallbackQuery::Ptr query);

  drogon::Task<void> resumeRegistration(drogon_model::sqlite3::Registrations &model);

private:
  TgBot::Bot &_bot;
  RegistrationRepository &_repository;

  drogon::Task<void> saveFullName(int64_t chatId, std::string fullName);
  drogon::Task<void> saveCompanyName(int64_t chatId, std::string companyName);
  drogon::Task<void>
  saveCompanyPosition(int64_t chatId, std::string companyPosition);
  drogon::Task<void> savePhrase(int64_t chatId, std::string phrase);

  drogon::Task<void> requestFullName(int64_t chatId, std::string username);
  drogon::Task<void> requestCompanyName(int64_t chatId);
  drogon::Task<void> requestCompanyPosition(int64_t chatId);
  drogon::Task<void> requestPhrase(int64_t chatId);

  drogon::Task<void> completeFormCreation(int64_t chatId);
};
} // namespace ConferenceBot