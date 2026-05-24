#include <vector>

namespace ConferenceBot::Constants {

inline const std::vector<std::string> ALLOWED_UPDATES =
    {"message", "callback_query", "chat_member"};

inline const std::vector<std::string> CONTEST_FILE_IDS = {
    "BQACAgIAAxkDAAIBf2oTEV8kbxu-r5IKgKiqjEun7fXsAAKVmQACoJ-ZSLwJ194cbvpsOwQ",
    "BQACAgIAAxkDAAIBgGoTEWPc8KyzfhUXmoumbv5_af9WAAKWmQACoJ-ZSApHU0PtWRSDOwQ",
    "BQACAgIAAxkDAAIBgWoTEWV8y21Bfwt7khpoKNwLEzIiAAKXmQACoJ-ZSBsgOqeKtYIDOwQ",
    "BQACAgIAAxkDAAIBgmoTEWVg63jltqQKyQABpwSVzzO4oQACmJkAAqCfmUiD1otWqJY0tzsE",
    "BQACAgIAAxkDAAIBg2oTEWaogWOy8Ls9YHMywjEzWPcvAAKZmQACoJ-ZSEPQDDWeXtVzOwQ"
};

} // namespace ConferenceBot::Constants