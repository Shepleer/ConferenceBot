#include <vector>

namespace ConferenceBot::Constants {

inline const std::vector<std::string> ALLOWED_UPDATES =
    {"message", "callback_query", "chat_member"};

inline const std::vector<std::string> CONTEST_FILE_IDS = {
    "AgACAgIAAxkDAAICA2oTpsVxgJK8qlg_c2cDBG1F0WnXAAIRFmsboJ-hSMnDR_feVsUcAQADAgADdwADOwQ",
    "AgACAgIAAxkDAAICBGoTpsc8MczftEGn0t3G_f9C8okuAAISFmsboJ-hSIRKiMZTJx9pAQADAgADdwADOwQ",
    "AgACAgIAAxkDAAICBWoTpsgtatp5prPHSDKb23wvMJpBAAITFmsboJ-hSIPrEV6k8DIpAQADAgADdwADOwQ",
    "AgACAgIAAxkDAAICBmoTpsvFfu3UV60K8DNRXuRqMgAB-AACFBZrG6CfoUjXDu2T5DMvJAEAAwIAA3cAAzsE",
};

} // namespace ConferenceBot::Constants