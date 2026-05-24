#include <ConferenceBot/Workflows/RegistrationWorkflow.hpp>

#include <ConferenceBot/Keyboards/Keyboards.hpp>
#include <ConferenceBot/Resources/Assets.hpp>
#include <ConferenceBot/Resources/Constants.hpp>
#include <ConferenceBot/Resources/Strings.hpp>
#include <ConferenceBot/Services/RegistrationState.hpp>
#include <Registrations.h>

namespace ConferenceBot {
namespace {

drogon::Task<void> showFormPrompt(
    TgBot::Bot &bot,
    RegistrationRepository &repository,
    int64_t chatId,
    RegistrationState state
) {
  auto row = co_await repository.findByChatId(chatId);
  if (!row) {
    co_return;
  }

  row->setState(rawValue(state));

  std::string name = row->getValueOfName();
  std::string companyName = row->getValueOfCompany();
  std::string companyPosition = row->getValueOfCompanyPosition();
  std::string phrase = row->getValueOfPhrase();

  co_await repository.update(*row);

  const auto formText = std::format(
      Strings::RegistrationFormTemplate,
      name.empty() ? Strings::RegistrationFormEmptyFieldPlacehplderText : name,
      companyName.empty() ? Strings::RegistrationFormEmptyFieldPlacehplderText
                          : companyName,
      companyPosition.empty()
          ? Strings::RegistrationFormEmptyFieldPlacehplderText
          : companyPosition,
      phrase.empty() ? Strings::RegistrationFormEmptyFieldPlacehplderText
                     : row->getValueOfPhrase()
  );

  auto keyboard = canGoBack(state) ? Keyboards::formBackKeyboard() : nullptr;

  bot.getApi().editMessageText(
      formText,
      chatId,
      row->getValueOfMessageId(),
      "",
      "",
      nullptr,
      keyboard
  );
  bot.getApi().sendMessage(chatId, std::string(prompt(state)));
}

} // namespace

drogon::Task<void> RegistrationWorkflow::answerWantParticipateQuery(
    const TgBot::CallbackQuery::Ptr &query
) {
  assert(this);
  assert(&_bot);
  _bot.getApi().answerCallbackQuery(query->id);
  _bot.getApi().editMessageReplyMarkup(
      query->message->chat->id,
      query->message->messageId,
      query->inlineMessageId,
      nullptr
  );
  _bot.getApi().sendMessage(
      query->message->chat->id,
      std::string(Strings::ContestDescriptionMessageText),
      nullptr,
      nullptr,
      nullptr,
      "HTML"
  );

  std::vector<TgBot::InputMedia::Ptr> documents;

  std::ranges::transform(
      Constants::CONTEST_FILE_IDS,
      std::back_inserter(documents),
      [](const auto &fileId) {
        auto document = std::make_shared<TgBot::InputMediaDocument>();
        document->media = fileId;
        return document;
      }
  );

  _bot.getApi().sendMediaGroup(query->message->chat->id, documents);

  co_await requestFullName(query->message->chat->id, query->from->username);
}

drogon::Task<void>
RegistrationWorkflow::processMessage(TgBot::Message::Ptr message) {
  using enum ConferenceBot::RegistrationState;

  auto row = co_await _repository.findByChatId(message->chat->id);
  if (!row) {
    co_return;
  }

  RegistrationState state = getState(*row);

  switch (state) {
  case WaitingName:
    co_await saveFullName(message->chat->id, message->text);
    break;
  case WaitingCompanyName:
    co_await saveCompanyName(message->chat->id, message->text);
    break;
  case WaitingCompanyPosition:
    co_await saveCompanyPosition(message->chat->id, message->text);
    break;
  case WaitingPhrase:
    co_await savePhrase(message->chat->id, message->text);
    break;
  case Completed:
    co_return;
  }
}

drogon::Task<void>
RegistrationWorkflow::replyBackQuery(TgBot::CallbackQuery::Ptr query) {
  using enum RegistrationState;
  auto row = co_await _repository.findByChatId(query->message->chat->id);

  if (!row) {
    co_return;
  }

  RegistrationState state = getState(*row);
  if (!canGoBack(state)) {
    co_return;
  }

  RegistrationState newState = prev(state);
  row->setState(rawValue(newState));
  switch (newState) {
  case WaitingName:
    row->setName("");
    break;
  case WaitingCompanyName:
    row->setCompany("");
    break;
  case WaitingCompanyPosition:
    row->setCompanyPosition("");
    break;
  default:
    break;
  }

  co_await _repository.update(*row);
  co_await showFormPrompt(_bot, _repository, row->getValueOfChatId(), newState);
}

drogon::Task<void> RegistrationWorkflow::resumeRegistration(
    drogon_model::sqlite3::Registrations &model
) {
  auto state = getState(model);
  co_return;
}

drogon::Task<void>
RegistrationWorkflow::requestFullName(int64_t chatId, std::string username) {
  drogon_model::sqlite3::Registrations model;
  model.setChatId(chatId);
  model.setState(rawValue(RegistrationState::WaitingName));
  model.setTelegramNickname(username);

  auto row = co_await _repository.save(model);

  auto message = _bot.getApi().sendMessage(
      chatId,
      std::string(prompt(RegistrationState::WaitingName))
  );

  row.setMessageId(message->messageId);

  co_await _repository.update(row);
}

drogon::Task<void> RegistrationWorkflow::requestCompanyName(int64_t chatId) {
  co_await showFormPrompt(
      _bot,
      _repository,
      chatId,
      RegistrationState::WaitingCompanyName
  );
}

drogon::Task<void>
RegistrationWorkflow::requestCompanyPosition(int64_t chatId) {
  co_await showFormPrompt(
      _bot,
      _repository,
      chatId,
      RegistrationState::WaitingCompanyPosition
  );
}

drogon::Task<void> RegistrationWorkflow::requestPhrase(int64_t chatId) {
  co_await showFormPrompt(
      _bot,
      _repository,
      chatId,
      RegistrationState::WaitingPhrase
  );
}

drogon::Task<void>
RegistrationWorkflow::saveFullName(int64_t chatId, std::string fullName) {
  auto row = co_await _repository.findByChatId(chatId);
  if (!row || getState(*row) != RegistrationState::WaitingName) {
    co_return;
  }

  row->setName(std::move(fullName));
  co_await _repository.update(*row);
  co_await requestCompanyName(chatId);
}

drogon::Task<void>
RegistrationWorkflow::saveCompanyName(int64_t chatId, std::string companyName) {
  auto row = co_await _repository.findByChatId(chatId);
  if (!row || getState(*row) != RegistrationState::WaitingCompanyName) {
    co_return;
  }

  row->setCompany(std::move(companyName));
  co_await _repository.update(*row);
  co_await requestCompanyPosition(chatId);
}

drogon::Task<void> RegistrationWorkflow::saveCompanyPosition(
    int64_t chatId,
    std::string companyPosition
) {
  auto row = co_await _repository.findByChatId(chatId);
  if (!row || getState(*row) != RegistrationState::WaitingCompanyPosition) {
    co_return;
  }

  row->setCompanyPosition(std::move(companyPosition));
  co_await _repository.update(*row);
  co_await requestPhrase(chatId);
}

drogon::Task<void>
RegistrationWorkflow::savePhrase(int64_t chatId, std::string phrase) {
  auto row = co_await _repository.findByChatId(chatId);
  if (!row || getState(*row) != RegistrationState::WaitingPhrase) {
    co_return;
  }

  row->setPhrase(std::move(phrase));
  row->setState(rawValue(RegistrationState::Completed));
  co_await _repository.update(*row);

  co_await showFormPrompt(
      _bot,
      _repository,
      chatId,
      RegistrationState::Completed
  );
  co_await completeFormCreation(chatId);
}

drogon::Task<void> RegistrationWorkflow::completeFormCreation(int64_t chatId) {
  _bot.getApi().sendMessage(chatId, std::string(Strings::ThankYouMessageText));
  co_return;
}

} // namespace ConferenceBot
