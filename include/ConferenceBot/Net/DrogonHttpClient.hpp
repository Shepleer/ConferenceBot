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

#include <atomic>
#include <cstddef>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace ConferenceBot {

/// TgBot::HttpClient implementation that delegates outgoing Telegram Bot API
/// calls to a pool of Drogon HttpClient instances.
///
/// Why a pool? Each drogon::HttpClient owns exactly one persistent TCP
/// connection. With a single client, all concurrent API calls serialize on
/// that one socket, so multiple concurrent users get stuck waiting for the
/// previous response. The pool gives us N parallel connections per host,
/// round-robined across calls, all multiplexed by a single dedicated event
/// loop thread.
///
/// Outbound sockets are configured with aggressive TCP keepalive so dead
/// connections (silently dropped by NATs / cloud middleboxes) are evicted
/// within seconds instead of relying on the kernel's tcp_retries2 default
/// (~13–15 minutes).
class DrogonHttpClient final : public TgBot::HttpClient {
public:
  /// Number of parallel persistent connections kept per remote host.
  static constexpr std::size_t kPoolSize = 32;

  DrogonHttpClient();
  ~DrogonHttpClient() override;

  DrogonHttpClient(const DrogonHttpClient &) = delete;
  DrogonHttpClient &operator=(const DrogonHttpClient &) = delete;

  std::string makeRequest(
      const TgBot::Url &url,
      const std::vector<TgBot::HttpReqArg> &args
  ) const override;

private:
  drogon::HttpClientPtr leaseClient(
      const std::string &protocol,
      const std::string &host
  ) const;

  static void applyKeepAlive(int fd);

  std::unique_ptr<trantor::EventLoopThread> _loopThread;

  mutable std::mutex _poolsMutex;
  mutable std::unordered_map<std::string, std::vector<drogon::HttpClientPtr>>
      _pools;

  mutable std::atomic<std::size_t> _roundRobin{0};
};

} // namespace ConferenceBot
