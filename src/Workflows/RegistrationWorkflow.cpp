#include <ConferenceBot/Workflows/RegistrationWorkflow.hpp>

#include <ConferenceBot/Keyboards/Keyboards.hpp>
#include <ConferenceBot/Resources/Assets.hpp>
#include <ConferenceBot/Resources/Constants.hpp>
#include <ConferenceBot/Resources/Strings.hpp>
#include <ConferenceBot/Services/RegistrationState.hpp>
#include <Registrations.h>

#include <drogon/drogon.h>

namespace ConferenceBot {
namespace {

std::string
buildFormText(const drogon_model::sqlite3::Registrations &row) {
  const std::string name = row.getValueOfName();
  const std::string companyName = row.getValueOfCompany();
  const std::string companyPosition = row.getValueOfCompanyPosition();
  const std::string phrase = row.getValueOfPhrase();

  return std::format(
      Strings::RegistrationFormTemplate,
      name.empty() ? Strings::RegistrationFormEmptyFieldPlacehplderText : name,
      companyName.empty() ? Strings::RegistrationFormEmptyFieldPlacehplderText
                          : companyName,
      companyPosition.empty()
          ? Strings::RegistrationFormEmptyFieldPlacehplderText
          : companyPosition,
      phrase.empty() ? Strings::RegistrationFormEmptyFieldPlacehplderText
                     : phrase
  );
}

drogon::Task<void> showFormPrompt(
    TgBot::Bot &bot,
    RegistrationRepository &repository,
    int64_t chatId,
    RegistrationState state
) {
  LOG_INFO << "[registration] showFormPrompt chat=" << chatId
           << " state=" << rawValue(state);
  auto row = co_await repository.findByChatId(chatId);
  if (!row) {
    LOG_WARN << "[registration] showFormPrompt: no registration row for chat="
             << chatId;
    co_return;
  }

  row->setState(rawValue(state));
  co_await repository.update(*row);

  const auto formText = buildFormText(*row);
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
  const int64_t chatId = query->message->chat->id;
  const int64_t userId = query->from ? query->from->id : 0;
  LOG_INFO << "[registration] 'want participate' chat=" << chatId
           << " user=" << userId << " (@"
           << (query->from ? query->from->username : "") << ")";

  try {
    _bot.getApi().answerCallbackQuery(query->id);
    _bot.getApi().editMessageReplyMarkup(
        chatId,
        query->message->messageId,
        query->inlineMessageId,
        nullptr
    );
    _bot.getApi().sendMessage(
        chatId,
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
          auto document = std::make_shared<TgBot::InputMediaPhoto>();
          document->media = fileId;
          return document;
        }
    );

    _bot.getApi().sendMediaGroup(chatId, documents);
    LOG_DEBUG << "[registration] Sent contest media group (count="
              << documents.size() << ") to chat=" << chatId;

    co_await requestFullName(chatId, query->from->username);
  } catch (const TgBot::TgException &e) {
    LOG_ERROR << "[registration] Telegram error in answerWantParticipateQuery"
              << " chat=" << chatId << ": " << e.what();
  }
}

drogon::Task<void>
RegistrationWorkflow::processMessage(TgBot::Message::Ptr message) {
  using enum ConferenceBot::RegistrationState;

  const int64_t chatId = message->chat->id;
  auto row = co_await _repository.findByChatId(chatId);
  if (!row) {
    LOG_DEBUG << "[registration] processMessage: no row for chat=" << chatId
              << " - ignoring";
    co_return;
  }

  RegistrationState state;
  try {
    state = getState(*row);
  } catch (const std::exception &e) {
    LOG_ERROR << "[registration] Invalid state for chat=" << chatId
              << " raw=" << row->getValueOfState() << ": " << e.what();
    co_return;
  }

  LOG_INFO << "[registration] processMessage chat=" << chatId
           << " state=" << rawValue(state)
           << " textLen=" << message->text.size();

  switch (state) {
  case WaitingName:
    co_await saveFullName(chatId, message->text);
    break;
  case WaitingCompanyName:
    co_await saveCompanyName(chatId, message->text);
    break;
  case WaitingCompanyPosition:
    co_await saveCompanyPosition(chatId, message->text);
    break;
  case WaitingPhrase:
    co_await savePhrase(chatId, message->text);
    break;
  case Completed:
    LOG_DEBUG << "[registration] processMessage chat=" << chatId
              << " already Completed - ignoring";
    co_return;
  }
}

drogon::Task<void>
RegistrationWorkflow::replyBackQuery(TgBot::CallbackQuery::Ptr query) {
  using enum RegistrationState;

  if (!query || !query->message || !query->message->chat) {
    LOG_WARN << "[registration] replyBackQuery: malformed callback query";
    co_return;
  }

  const int64_t chatId = query->message->chat->id;
  const int64_t clickedMessageId = query->message->messageId;

  auto row = co_await _repository.findByChatId(chatId);

  if (!row) {
    LOG_WARN << "[registration] replyBackQuery: no row for chat=" << chatId;
    _bot.getApi().answerCallbackQuery(query->id);
    co_return;
  }

  if (clickedMessageId != row->getValueOfMessageId()) {
    LOG_INFO << "[registration] replyBackQuery: stale form click chat="
             << chatId << " clicked=" << clickedMessageId
             << " current=" << row->getValueOfMessageId();
    _bot.getApi().answerCallbackQuery(
        query->id,
        std::string(Strings::FormOutdatedMessageText)
    );
    co_return;
  }

  _bot.getApi().answerCallbackQuery(query->id);

  RegistrationState state = getState(*row);
  if (!canGoBack(state)) {
    LOG_DEBUG << "[registration] replyBackQuery: cannot go back from state="
              << rawValue(state) << " chat=" << chatId;
    co_return;
  }

  RegistrationState newState = prev(state);
  LOG_INFO << "[registration] Going back chat=" << chatId
           << " from=" << rawValue(state) << " to=" << rawValue(newState);
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
  using enum RegistrationState;

  const int64_t chatId = model.getValueOfChatId();

  RegistrationState state;
  try {
    state = getState(model);
  } catch (const std::exception &e) {
    LOG_ERROR << "[registration] resumeRegistration: invalid state for chat="
              << chatId << " raw=" << model.getValueOfState() << ": "
              << e.what();
    co_return;
  }

  LOG_INFO << "[registration] resumeRegistration chat=" << chatId
           << " state=" << rawValue(state);

  if (state == Completed) {
    LOG_DEBUG << "[registration] resumeRegistration: already completed chat="
              << chatId;
    _bot.getApi().sendMessage(
        chatId,
        std::string(Strings::ThankYouMessageText),
        nullptr,
        nullptr,
        nullptr,
        "HTML"
    );
    co_return;
  }

  const int64_t staleMessageId = model.getValueOfMessageId();
  if (staleMessageId != 0) {
    try {
      _bot.getApi().editMessageReplyMarkup(
          chatId,
          staleMessageId,
          "",
          nullptr
      );
    } catch (const TgBot::TgException &e) {
      LOG_DEBUG << "[registration] resumeRegistration: could not clear "
                   "keyboard on stale message="
                << staleMessageId << " chat=" << chatId << ": " << e.what();
    }
  }

  try {
    const auto formText = buildFormText(model);
    auto keyboard = canGoBack(state) ? Keyboards::formBackKeyboard() : nullptr;

    auto sent = _bot.getApi().sendMessage(
        chatId,
        formText,
        nullptr,
        nullptr,
        keyboard
    );

    model.setMessageId(sent->messageId);
    co_await _repository.update(model);

    _bot.getApi().sendMessage(chatId, std::string(prompt(state)));
    LOG_DEBUG << "[registration] resumeRegistration: re-sent form chat="
              << chatId << " messageId=" << sent->messageId;
  } catch (const TgBot::TgException &e) {
    LOG_ERROR << "[registration] Telegram error in resumeRegistration chat="
              << chatId << ": " << e.what();
  }
}

drogon::Task<void>
RegistrationWorkflow::requestFullName(int64_t chatId, std::string username) {
  LOG_INFO << "[registration] Creating new registration chat=" << chatId
           << " username=@" << username;

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
  LOG_DEBUG << "[registration] Prompted for full name chat=" << chatId
            << " messageId=" << message->messageId;
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
    LOG_WARN << "[registration] saveFullName: invalid state for chat="
             << chatId;
    co_return;
  }

  LOG_INFO << "[registration] Saved name chat=" << chatId
           << " len=" << fullName.size();
  row->setName(std::move(fullName));
  co_await _repository.update(*row);
  co_await requestCompanyName(chatId);
}

drogon::Task<void>
RegistrationWorkflow::saveCompanyName(int64_t chatId, std::string companyName) {
  auto row = co_await _repository.findByChatId(chatId);
  if (!row || getState(*row) != RegistrationState::WaitingCompanyName) {
    LOG_WARN << "[registration] saveCompanyName: invalid state for chat="
             << chatId;
    co_return;
  }

  LOG_INFO << "[registration] Saved company chat=" << chatId
           << " len=" << companyName.size();
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
    LOG_WARN << "[registration] saveCompanyPosition: invalid state for chat="
             << chatId;
    co_return;
  }

  LOG_INFO << "[registration] Saved company position chat=" << chatId
           << " len=" << companyPosition.size();
  row->setCompanyPosition(std::move(companyPosition));
  co_await _repository.update(*row);
  co_await requestPhrase(chatId);
}

drogon::Task<void>
RegistrationWorkflow::savePhrase(int64_t chatId, std::string phrase) {
  auto row = co_await _repository.findByChatId(chatId);
  if (!row || getState(*row) != RegistrationState::WaitingPhrase) {
    LOG_WARN << "[registration] savePhrase: invalid state for chat=" << chatId;
    co_return;
  }

  LOG_INFO << "[registration] Saved phrase chat=" << chatId
           << " len=" << phrase.size();
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
  LOG_INFO << "[registration] Registration COMPLETED chat=" << chatId;
  _bot.getApi().sendMessage(
      chatId,
      std::string(Strings::ThankYouMessageText),
      nullptr,
      nullptr,
      nullptr,
      "HTML"
  );
  co_return;
}

} // namespace ConferenceBot
