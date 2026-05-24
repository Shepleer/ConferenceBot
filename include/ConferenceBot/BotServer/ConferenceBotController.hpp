#pragma once

#include <ConferenceBot/Config/Config.hpp>
#include <ConferenceBot/Events/CallbackQueryRouter.hpp>
#include <ConferenceBot/Events/CommandRegistry.hpp>
#include <ConferenceBot/Workflows/CheckSubscriptionWorkflow.hpp>
#include <ConferenceBot/Workflows/RegistrationWorkflow.hpp>
#include <ConferenceBot/Workflows/StartWorkflow.hpp>

#include <tgbot/Bot.h>

namespace ConferenceBot {
class ConferenceBotController : public StartWorkflowDelegate {
public:
  explicit ConferenceBotController(
      TgBot::Bot &bot,
      drogon::orm::DbClientPtr dbClient,
      Config config
  );
  void registerHandlers();

  drogon::Task<bool> resumeRegistrationIfNeeded(int64_t chatId) override;

private:
  TgBot::Bot &_bot;

  RegistrationRepository _registrationRepository;

  CommandRegistry _commandRouter;
  CallbackQueryRouter _callbackQueryRouter;

  StartWorkflow _startWorkflow;
  CheckSubscriptionWorkflow _checkSubscriptionWorkflow;
  RegistrationWorkflow _registrationWorkflow;

  const Config _config;
};
} // namespace ConferenceBot