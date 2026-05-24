#include <ConferenceBot/Events/CallbackQueryRouter.hpp>

#include <drogon/drogon.h>

namespace ConferenceBot {

void CallbackQueryRouter::registerHandler(TgBot::Bot &bot) {
  bot.getEvents().onCallbackQuery(
      [this](const TgBot::CallbackQuery::Ptr &query) {
        const int64_t userId = query->from ? query->from->id : 0;
        LOG_INFO << "[callback] data='" << query->data
                 << "' from user=" << userId
                 << " (@" << (query->from ? query->from->username : "") << ")";

        const auto it = _routes.find(query->data);

        if (it != _routes.end()) {
          try {
            it->second(query);
          } catch (const std::exception &e) {
            LOG_ERROR << "[callback] handler threw for data='" << query->data
                      << "': " << e.what();
          }
        } else {
          LOG_WARN << "[callback] no handler registered for data='"
                   << query->data << "'";
        }
      }
  );
}

} // namespace ConferenceBot