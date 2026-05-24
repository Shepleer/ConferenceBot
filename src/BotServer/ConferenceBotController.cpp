#include <ConferenceBot/BotServer/ConferenceBotController.hpp>

#include <ConferenceBot/Resources/Callbacks.hpp>
#include <ConferenceBot/Resources/Commands.hpp>
#include <ConferenceBot/Services/RegistrationRepository.hpp>

#include <drogon/drogon.h>

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
  LOG_INFO << "[controller] Registering bot handlers...";

  _callbackQueryRouter.registerHandler(_bot);
  _callbackQueryRouter.registerRoute(
      Callbacks::CheckSubscriptionCallbackData,
      [this](const TgBot::CallbackQuery::Ptr query) {
        LOG_DEBUG << "[controller] Routing 'check_subscription' callback";
        _checkSubscriptionWorkflow.checkSubscription(query, _config.channelId);
      }
  );

  _callbackQueryRouter.registerRoute(
      Callbacks::WantParticipateCallbackData,
      [this](const TgBot::CallbackQuery::Ptr query) {
        LOG_DEBUG << "[controller] Routing 'want_participate' callback";
        drogon::async_run([this, query] {
          return _registrationWorkflow.answerWantParticipateQuery(query);
        });
      }
  );

  _callbackQueryRouter.registerRoute(
      Callbacks::FormBackButtonCallbackData,
      [this](const TgBot::CallbackQuery::Ptr query) {
        LOG_DEBUG << "[controller] Routing 'form_back' callback";
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

  _bot.getEvents().onNonCommandMessage([this](TgBot::Message::Ptr message) {
    if (!message || !message->chat) {
      return;
    }
    if (message->chat->type != TgBot::Chat::Type::Private ||
        message->text.empty()) {
      LOG_DEBUG << "[controller] Skipping non-private/empty message in chat="
                << message->chat->id;
      return;
    }

    LOG_INFO << "[controller] Non-command message chat=" << message->chat->id
             << " textLen=" << message->text.size();

    drogon::async_run([this, message] {
      return _registrationWorkflow.processMessage(message);
    });
  });

  LOG_INFO << "[controller] Handlers registered (callbacks=3, commands=1).";
}

drogon::Task<bool>
ConferenceBotController::resumeRegistrationIfNeeded(int64_t chatId) {
  LOG_DEBUG << "[controller] Checking registration to resume for chat="
            << chatId;
  auto row = co_await _registrationRepository.findByChatId(chatId);
  if (row.has_value()) {
    LOG_INFO << "[controller] Resuming registration for chat=" << chatId
             << " state=" << row->getValueOfState();
    co_await _registrationWorkflow.resumeRegistration(*row);
    co_return true;
  }

  LOG_DEBUG << "[controller] No existing registration for chat=" << chatId;
  co_return false;
}

} // namespace ConferenceBot