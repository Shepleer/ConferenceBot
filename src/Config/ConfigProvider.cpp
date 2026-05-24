#include <ConferenceBot/Config/ConfigProvider.hpp>

#include <drogon/drogon.h>

namespace ConferenceBot {
namespace {

std::string readEnv(const char *name) {
  const char *value = std::getenv(name);
  if (value == nullptr || *value == '\0') {
    LOG_ERROR << "[config] Required environment variable '" << name
              << "' is not set or empty";
    return {};
  }
  return value;
}

} // namespace

const Config ConfigProvider::getConfig() {
  LOG_INFO << "[config] Loading configuration from environment...";

  const Config config = {
      readEnv("BOT_TOKEN"),
      readEnv("CHANNEL_ID"),
      readEnv("WEBHOOK_URL"),
      readEnv("EXPORT_TOKEN")
  };

  LOG_INFO << "[config] Loaded. channelId=" << config.channelId
           << ", webhookUrl=" << config.webhookUrl
           << ", botToken=" << (config.botToken.empty() ? "<missing>" : "<set>")
           << ", exportToken="
           << (config.exportToken.empty() ? "<missing>" : "<set>");

  return config;
}
} // namespace ConferenceBot