#pragma once

#include <ConferenceBot/Resources/Strings.hpp>
#include <Registrations.h>

namespace ConferenceBot {
enum class RegistrationState {
  WaitingName = 0,
  WaitingCompanyName = 10,
  WaitingCompanyPosition = 20,
  WaitingPhrase = 30,
  Completed = 40
};

RegistrationState
getState(const drogon_model::sqlite3::Registrations &registration);

constexpr int rawValue(RegistrationState state) {
  return static_cast<int>(state);
}

constexpr std::string_view prompt(RegistrationState state) {
  using enum RegistrationState;
  switch (state) {
  case WaitingName:
    return Strings::FormEnterNamePromptText;
  case WaitingCompanyName:
    return Strings::FormEnterCompanyPromptText;
  case WaitingCompanyPosition:
    return Strings::FormEnterCompanyPositionPromptText;
  case WaitingPhrase:
    return Strings::FormEnterPhrasePromptText;
  case Completed:
    return Strings::FormCompletedText;
  default:
    return "";
  }

  return "";
}

constexpr RegistrationState prev(RegistrationState state) {
  using enum RegistrationState;
  switch (state) {
  case WaitingName:
    return WaitingName;

  case WaitingCompanyName:
    return WaitingName;

  case WaitingCompanyPosition:
    return WaitingCompanyName;

  case WaitingPhrase:
    return WaitingCompanyPosition;

  case Completed:
    return WaitingPhrase;

  default:
    return WaitingName;
  }

  return WaitingName;
}

constexpr RegistrationState next(RegistrationState state) {
  using enum RegistrationState;
  switch (state) {
  case WaitingName:
    return WaitingCompanyName;

  case WaitingCompanyName:
    return WaitingCompanyPosition;

  case WaitingCompanyPosition:
    return WaitingPhrase;

  case WaitingPhrase:
    return Completed;

  case Completed:
    return Completed;

  default:
    return Completed;
  }

  return Completed;
}

constexpr bool canGoBack(RegistrationState state) {
  using enum RegistrationState;

  switch (state) {
  case WaitingName:
  case Completed:
    return false;
  default:
    return true;
  }
}

} // namespace ConferenceBot