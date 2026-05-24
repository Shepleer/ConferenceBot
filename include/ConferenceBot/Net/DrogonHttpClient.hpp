//
//  DrogonHttpClient.hpp
//  ConferenceBot
//
//  Created by Ivan Shpileuski on 25/05/2026.
//
//

#pragma once

#include <tgbot/net/HttpClient.h>

#include <drogon/HttpClient.h>
#include <trantor/net/EventLoopThread.h>

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace ConferenceBot {

/// TgBot::HttpClient implementation that delegates outgoing Telegram Bot API
/// calls to a Drogon HttpClient.
///
/// All HTTP I/O runs on a dedicated trantor event loop (one thread), so the
/// underlying TCP/TLS connection to api.telegram.org is reused between calls
/// (HTTP keep-alive) and never blocks Drogon's main worker pool with curl.
///
/// makeRequest() still returns synchronously because TgBot's API surface is
/// synchronous, but the blocking happens on a std::promise rather than on a
/// libcurl handle, and connection setup cost is amortized.
class DrogonHttpClient final : public TgBot::HttpClient {
public:
  DrogonHttpClient();
  ~DrogonHttpClient() override;

  DrogonHttpClient(const DrogonHttpClient &) = delete;
  DrogonHttpClient &operator=(const DrogonHttpClient &) = delete;

  std::string makeRequest(
      const TgBot::Url &url,
      const std::vector<TgBot::HttpReqArg> &args
  ) const override;

private:
  drogon::HttpClientPtr getOrCreateClient(
      const std::string &protocol,
      const std::string &host
  ) const;

  std::unique_ptr<trantor::EventLoopThread> _loopThread;

  mutable std::mutex _clientsMutex;
  mutable std::unordered_map<std::string, drogon::HttpClientPtr> _clients;
};

} // namespace ConferenceBot
