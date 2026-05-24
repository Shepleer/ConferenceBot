#include <ConferenceBot/Config/ConfigProvider.hpp>

namespace ConferenceBot {
const Config ConfigProvider::getConfig() {
  const Config config =
      {getenv("BOT_TOKEN"), getenv("CHANNEL_ID"), getenv("WEBHOOK_URL"), getenv("EXPORT_TOKEN")};
  return config;
}
} // namespace ConferenceBot