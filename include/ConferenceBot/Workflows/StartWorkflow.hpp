#pragma once

#include <ConferenceBot/Services/RegistrationRepository.hpp>

#include <drogon/drogon.h>
#include <tgbot/Bot.h>

namespace ConferenceBot {
class StartWorkflowDelegate {
public:
  virtual ~StartWorkflowDelegate() = default;

  virtual drogon::Task<bool> resumeRegistrationIfNeeded(int64_t chatId) = 0;
};

class StartWorkflow {
public:
  explicit StartWorkflow(TgBot::Bot &bot, StartWorkflowDelegate &delegate)
      : _bot(bot)
      , _delegate(delegate) {}

  drogon::Task<void> replyStartCommand(int64_t chatId);

private:
  TgBot::Bot &_bot;
  StartWorkflowDelegate &_delegate;
};
} // namespace ConferenceBot