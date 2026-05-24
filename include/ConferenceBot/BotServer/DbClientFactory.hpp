#pragma once

#include <drogon/drogon.h>

namespace ConferenceBot {
class DbClientFactory {
public:
  static drogon::orm::DbClientPtr make();
};
} // namespace ConferenceBot