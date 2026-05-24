#include <ConferenceBot/Services/RegistrationState.hpp>

namespace ConferenceBot {

RegistrationState
getState(const drogon_model::sqlite3::Registrations &registration) {
  const auto raw = registration.getValueOfState();

  switch (raw) {
  case 0:
  case 10:
  case 20:
  case 30:
  case 40:
    return static_cast<RegistrationState>(raw);
  default:
    throw std::runtime_error("Invalid state");
  }
}

} // namespace ConferenceBot