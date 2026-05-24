#include <ConferenceBot/Workflows/StartWorkflow.hpp>

#include <ConferenceBot/Keyboards/Keyboards.hpp>
#include <ConferenceBot/Resources/Assets.hpp>
#include <ConferenceBot/Resources/Strings.hpp>

#include <drogon/drogon.h>

namespace ConferenceBot {

drogon::Task<void> StartWorkflow::replyStartCommand(int64_t chatId) {
  LOG_INFO << "[start] /start invoked for chat=" << chatId;

  try {
    bool isResumed = co_await _delegate.resumeRegistrationIfNeeded(chatId);

    if (isResumed) {
      LOG_INFO << "[start] Existing registration resumed for chat=" << chatId;
      co_return;
    }

    LOG_INFO << "[start] New session - sending welcome flow to chat="
             << chatId;

    TgBot::InlineKeyboardMarkup::Ptr keyboard =
        Keyboards::makeCheckSubscriptionKeyboard();
    _bot.getApi().sendMessage(
        chatId,
        std::string(Strings::BotStartMessageText),
        nullptr,
        nullptr,
        keyboard,
        "HTML"
    );

    auto result = _bot.getApi().sendPhoto(
        chatId,
        "AgACAgIAAxkDAAIBX2oS_yvHm862H-ydLpWN6pFRgtGqAAJ0HGsboJ-"
        "ZSMk78efXcvu8AQADAgADdwADOwQ"
    );

    LOG_DEBUG << "[start] Welcome sent for chat=" << chatId;
  } catch (const TgBot::TgException &e) {
    LOG_ERROR << "[start] Telegram error for chat=" << chatId << ": "
              << e.what();
  } catch (const std::exception &e) {
    LOG_ERROR << "[start] Unexpected error for chat=" << chatId << ": "
              << e.what();
  }
}

} // namespace ConferenceBot