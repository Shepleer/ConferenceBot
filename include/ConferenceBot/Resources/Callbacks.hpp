#pragma once
#include <string_view>

namespace ConferenceBot::Callbacks {

inline constexpr std::string_view CheckSubscriptionCallbackData =
    "check_subscription";

inline constexpr std::string_view WantParticipateCallbackData =
    "want_participate";

inline constexpr std::string_view FormBackButtonCallbackData =
    "form_back_button";

} // namespace ConferenceBot::Callbacks