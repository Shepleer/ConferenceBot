#include <ConferenceBot/Events/MessageHandler.hpp>

namespace ConferenceBot {

void MessageHandler::registerHandler(TgBot::Bot &bot) {
  bot.getEvents().onAnyMessage([&bot](TgBot::Message::Ptr message) {
    if (message->chat->type != TgBot::Chat::Type::Private) {
      return;
    }
  });
}

} // namespace ConferenceBot