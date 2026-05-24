#include <ConferenceBot/Events/CallbackQueryRouter.hpp>

namespace ConferenceBot {

void CallbackQueryRouter::registerHandler(TgBot::Bot &bot) {
  bot.getEvents().onCallbackQuery(
      [this](const TgBot::CallbackQuery::Ptr &query) {
        const auto it = _routes.find(query->data);

        if (it != _routes.end()) {
          it->second(query);
        }
      }
  );
}

} // namespace ConferenceBot