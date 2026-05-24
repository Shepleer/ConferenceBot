#include <ConferenceBot/Workflows/StartWorkflow.hpp>

#include <ConferenceBot/Keyboards/Keyboards.hpp>
#include <ConferenceBot/Resources/Assets.hpp>
#include <ConferenceBot/Resources/Strings.hpp>

#include <drogon/drogon.h>

namespace ConferenceBot {

drogon::Task<void> StartWorkflow::replyStartCommand(int64_t chatId) {

  bool isResumed = co_await _delegate.resumeRegistrationIfNeeded(chatId);

  if (isResumed) {
    co_return;
  }

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
}

} // namespace ConferenceBot