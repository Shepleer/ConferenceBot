#include <ConferenceBot/Events/CommandRegistry.hpp>

namespace ConferenceBot {
void CommandRegistry::registerCommand(
    std::string_view command,
    RouteProvider provider
) {
  _routes.emplace(command, std::move(provider));
  _bot.getEvents().onCommand(
      std::string(command),
      [this,
       command = std::string(command)](const TgBot::Message::Ptr &message) {
        const auto it = _routes.find(command);
        if (it != _routes.end()) {
          it->second(message);
        }
      }
  );
}
} // namespace ConferenceBot