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

drogon::Task<void> CheckSubscriptionWorkflow::showNotSubscribedUI(
    const TgBot::CallbackQuery::Ptr &query
) {
  LOG_INFO << "[subscription] User user=" << query->from->id
           << " is NOT subscribed";
  _bot.getApi().answerCallbackQuery(
      query->id,
      std::string(Strings::YouNotSubcribedMessageText)
  );

  co_return;
}

drogon::Task<void> CheckSubscriptionWorkflow::showSubscribedUI(
    const TgBot::CallbackQuery::Ptr &query
) {
  const std::int64_t chatId = query->message->chat->id;
  LOG_INFO << "[subscription] User user=" << query->from->id
           << " IS subscribed (chat=" << chatId << ")";

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

  co_return;
}

drogon::Task<void> CheckSubscriptionWorkflow::checkSubscription(
    const TgBot::CallbackQuery::Ptr &query,
    const std::string_view channelId
) {
  const int64_t userId = query->from ? query->from->id : 0;
  LOG_INFO << "[subscription] Checking subscription for user=" << userId
           << " in channel=" << channelId;
  try {
    const auto member =
        _bot.getApi().getChatMember(std::string(channelId), query->from->id);
    LOG_DEBUG << "[subscription] user=" << userId
              << " status=" << member->status;
    if (isSubscribed(member)) {
      co_await showSubscribedUI(query);
    } else {
      co_await showNotSubscribedUI(query);
    }
  } catch (const TgBot::TgException &e) {
    LOG_ERROR << "[subscription] Telegram error checking user=" << userId
              << " in channel=" << channelId << ": " << e.what();
  }
}

} // namespace ConferenceBot