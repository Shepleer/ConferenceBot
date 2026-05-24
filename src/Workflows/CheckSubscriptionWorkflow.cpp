#include <ConferenceBot/Workflows/CheckSubscriptionWorkflow.hpp>

#include <ConferenceBot/Keyboards/Keyboards.hpp>
#include <ConferenceBot/Resources/Strings.hpp>

namespace {
bool isSubscribed(const TgBot::ChatMember::Ptr &member) {
  const std::string &status = member->status;
  return status != TgBot::ChatMemberLeft::STATUS &&
         status != TgBot::ChatMemberBanned::STATUS;
}
} // namespace

namespace ConferenceBot {

void CheckSubscriptionWorkflow::showNotSubscribedUI(
    const TgBot::CallbackQuery::Ptr &query
) {
  _bot.getApi().answerCallbackQuery(
      query->id,
      std::string(Strings::YouNotSubcribedMessageText)
  );
}

void CheckSubscriptionWorkflow::showSubscribedUI(
    const TgBot::CallbackQuery::Ptr &query
) {
  const std::int64_t chatId = query->message->chat->id;

  _bot.getApi().editMessageReplyMarkup(
      chatId,
      query->message->messageId,
      query->inlineMessageId,
      nullptr
  );

  _bot.getApi().answerCallbackQuery(
      query->id,
      std::string(Strings::YouSubcribedMessageText)
  );

  auto keyboard = ConferenceBot::Keyboards::wantParticipateKeyboard();

  _bot.getApi().sendMessage(
      chatId,
      std::string(Strings::SubscriptionConfirmedMessageText),
      nullptr,
      nullptr,
      keyboard
  );
}

void CheckSubscriptionWorkflow::checkSubscription(
    const TgBot::CallbackQuery::Ptr &query,
    const std::string_view channelId
) {
  const auto member =
      _bot.getApi().getChatMember(std::string(channelId), query->from->id);
  if (isSubscribed(member)) {
    showSubscribedUI(query);
  } else {
    showNotSubscribedUI(query);
  }
}

} // namespace ConferenceBot