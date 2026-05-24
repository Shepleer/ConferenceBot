#include <ConferenceBot/Events/MessageHandler.hpp>

#include <drogon/drogon.h>

namespace ConferenceBot {

void MessageHandler::registerHandler(TgBot::Bot &bot) {
  bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) {
    if (!message || !message->chat) {
      return;
    }
    if (message->chat->type != TgBot::Chat::Type::Private) {
      LOG_DEBUG << "[messages] ignoring non-private chat type for chat="
                << message->chat->id;
      return;
    }
    LOG_DEBUG << "[messages] private message chat=" << message->chat->id
              << " textLen=" << message->text.size();
  });
}

} // namespace ConferenceBot