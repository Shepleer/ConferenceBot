#include <ConferenceBot/Events/CommandRegistry.hpp>

#include <drogon/drogon.h>

namespace ConferenceBot {
void CommandRegistry::registerCommand(
    std::string_view command,
    RouteProvider provider
) {
  LOG_INFO << "[commands] Registering command '/" << command << "'";
  _routes.emplace(command, std::move(provider));
  _bot.getEvents().onCommand(
      std::string(command),
      [this,
       command = std::string(command)](const TgBot::Message::Ptr &message) {
        const int64_t chatId = message->chat ? message->chat->id : 0;
        const int64_t userId = message->from ? message->from->id : 0;
        LOG_INFO << "[commands] '/" << command << "' from chat=" << chatId
                 << " user=" << userId
                 << " (@" << (message->from ? message->from->username : "")
                 << ")";

        const auto it = _routes.find(command);
        if (it != _routes.end()) {
          try {
            it->second(message);
          } catch (const std::exception &e) {
            LOG_ERROR << "[commands] handler threw for '/" << command
                      << "': " << e.what();
          }
        } else {
          LOG_WARN << "[commands] no handler for '/" << command << "'";
        }
      }
  );
}
} // namespace ConferenceBot