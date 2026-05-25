#include <ConferenceBot/Workflows/CheckSubscriptionWorkflow.hpp>

#include <ConferenceBot/Keyboards/Keyboards.hpp>
#include <ConferenceBot/Net/BotApiExecutor.hpp>
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
  const std::string queryId = query->id;
  co_await onBotPool([&] {
    _bot.getApi().answerCallbackQuery(
        queryId,
        std::string(Strings::YouNotSubcribedMessageText)
    );
  });
}

drogon::Task<void> CheckSubscriptionWorkflow::showSubscribedUI(
    const TgBot::CallbackQuery::Ptr &query
) {
  const std::int64_t chatId = query->message->chat->id;
  const std::string queryId = query->id;
  const int32_t messageId = query->message->messageId;
  const std::string inlineMessageId = query->inlineMessageId;

  LOG_INFO << "[subscription] User user=" << query->from->id
           << " IS subscribed (chat=" << chatId << ")";

  co_await onBotPool([&] {
    _bot.getApi().editMessageReplyMarkup(
        chatId,
        messageId,
        inlineMessageId,
        nullptr
    );
  });

  co_await onBotPool([&] {
    _bot.getApi().answerCallbackQuery(
        queryId,
        std::string(Strings::YouSubcribedMessageText)
    );
  });

  auto keyboard = ConferenceBot::Keyboards::wantParticipateKeyboard();

  co_await onBotPool([&] {
    _bot.getApi().sendMessage(
        chatId,
        std::string(Strings::SubscriptionConfirmedMessageText),
        nullptr,
        nullptr,
        keyboard
    );
  });
}

drogon::Task<void> CheckSubscriptionWorkflow::checkSubscription(
    const TgBot::CallbackQuery::Ptr &query,
    const std::string_view channelId
) {
  const int64_t userId = query->from ? query->from->id : 0;
  const std::string channel{channelId};
  LOG_INFO << "[subscription] Checking subscription for user=" << userId
           << " in channel=" << channel;
  try {
    const auto member = co_await onBotPool([&] {
      return _bot.getApi().getChatMember(channel, userId);
    });
    LOG_DEBUG << "[subscription] user=" << userId
              << " status=" << member->status;
    if (isSubscribed(member)) {
      co_await showSubscribedUI(query);
    } else {
      co_await showNotSubscribedUI(query);
    }
  } catch (const TgBot::TgException &e) {
    LOG_ERROR << "[subscription] Telegram error checking user=" << userId
              << " in channel=" << channel << ": " << e.what();
  }
}

} // namespace ConferenceBot