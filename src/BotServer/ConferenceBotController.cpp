#include <ConferenceBot/BotServer/ConferenceBotController.hpp>

#include <ConferenceBot/Resources/Callbacks.hpp>
#include <ConferenceBot/Resources/Commands.hpp>
#include <ConferenceBot/Services/RegistrationRepository.hpp>

namespace ConferenceBot {
ConferenceBotController::ConferenceBotController(
    TgBot::Bot &bot,
    drogon::orm::DbClientPtr dbClient,
    Config config
)
    : _bot(bot)
    , _registrationRepository(dbClient)
    , _commandRouter(bot)
    , _callbackQueryRouter()
    , _startWorkflow(bot, *this)
    , _checkSubscriptionWorkflow(bot)
    , _registrationWorkflow(bot, _registrationRepository)
    , _config(std::move(config)) {}

void ConferenceBotController::registerHandlers() {
  _callbackQueryRouter.registerHandler(_bot);
  _callbackQueryRouter.registerRoute(
      Callbacks::CheckSubscriptionCallbackData,
      [this](const TgBot::CallbackQuery::Ptr query) {
        _checkSubscriptionWorkflow.checkSubscription(query, _config.channelId);
      }
  );

  _callbackQueryRouter.registerRoute(
      Callbacks::WantParticipateCallbackData,
      [this](const TgBot::CallbackQuery::Ptr query) {
        drogon::async_run([this, query] {
          return _registrationWorkflow.answerWantParticipateQuery(query);
        });
      }
  );

  _callbackQueryRouter.registerRoute(
      Callbacks::FormBackButtonCallbackData,
      [this](const TgBot::CallbackQuery::Ptr query) {
        drogon::async_run([this, query] {
          return _registrationWorkflow.replyBackQuery(query);
        });
      }
  );

  _commandRouter.registerCommand(
      Commands::Start,
      [this](const TgBot::Message::Ptr message) {
        drogon::async_run([this, message] {
          return _startWorkflow.replyStartCommand(message->chat->id);
        });
      }
  );

  _bot.getEvents().onAnyMessage([this](TgBot::Message::Ptr message) {
    if (message->chat->type != TgBot::Chat::Type::Private ||
        message->text.empty() || message->text.starts_with("/")) {
      return;
    }

    drogon::async_run([this, message] {
      return _registrationWorkflow.processMessage(message);
    });
  });
}

drogon::Task<bool>
ConferenceBotController::resumeRegistrationIfNeeded(int64_t chatId) {
  auto row = co_await _registrationRepository.findByChatId(chatId);
  if (row.has_value()) {
    co_await _registrationWorkflow.resumeRegistration(*row);
    co_return true;
  }

  co_return false;
}

} // namespace ConferenceBot