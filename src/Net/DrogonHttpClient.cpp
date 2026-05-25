//
//  DrogonHttpClient.cpp
//  ConferenceBot
//
//  Created by Ivan Shpileuski on 25/05/2026.
//
//

#include <ConferenceBot/Net/DrogonHttpClient.hpp>

#include <tgbot/TgException.h>
#include <tgbot/net/Url.h>

#include <drogon/HttpRequest.h>
#include <drogon/HttpTypes.h>
#include <drogon/utils/Utilities.h>
#include <trantor/utils/Logger.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <format>
#include <string>

namespace ConferenceBot {
namespace {

std::string buildFormUrlEncodedBody(
    const std::vector<TgBot::HttpReqArg> &args
) {
  std::string body;
  bool first = true;
  for (const auto &arg : args) {
    if (!first) {
      body.push_back('&');
    }
    first = false;
    body.append(drogon::utils::urlEncodeComponent(arg.name));
    body.push_back('=');
    body.append(drogon::utils::urlEncodeComponent(arg.value));
  }
  return body;
}

std::string generateBoundary() {
  static std::atomic<uint64_t> counter{0};
  const auto now =
      std::chrono::steady_clock::now().time_since_epoch().count();
  return std::format("----TgBotBoundary{}{}", now, counter.fetch_add(1));
}

std::pair<std::string, std::string> buildMultipartBody(
    const std::vector<TgBot::HttpReqArg> &args
) {
  const std::string boundary = generateBoundary();
  std::string body;
  body.reserve(1024);
  for (const auto &arg : args) {
    body.append("--").append(boundary).append("\r\n");
    body.append("Content-Disposition: form-data; name=\"")
        .append(arg.name)
        .append("\"");
    if (arg.isFile && !arg.fileName.empty()) {
      body.append("; filename=\"").append(arg.fileName).append("\"");
    }
    body.append("\r\n");
    if (arg.isFile) {
      body.append("Content-Type: ").append(arg.mimeType).append("\r\n");
    }
    body.append("\r\n");
    body.append(arg.value);
    body.append("\r\n");
  }
  body.append("--").append(boundary).append("--\r\n");

  return {std::move(body), "multipart/form-data; boundary=" + boundary};
}

} // namespace

DrogonHttpClient::DrogonHttpClient()
    : _loopThread(
          std::make_unique<trantor::EventLoopThread>("TgBotHttpLoop")
      ) {
  _loopThread->run();
  LOG_INFO << "[http-client] DrogonHttpClient initialised on dedicated loop "
              "thread 'TgBotHttpLoop' (pool size per host = "
           << kPoolSize << ")";
}

DrogonHttpClient::~DrogonHttpClient() {
  {
    std::lock_guard<std::mutex> lock(_poolsMutex);
    _pools.clear();
  }
  if (_loopThread && _loopThread->getLoop()) {
    _loopThread->getLoop()->quit();
  }
}

void DrogonHttpClient::applyKeepAlive(int fd) {
  int yes = 1;
  if (::setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes)) != 0) {
    LOG_WARN << "[http-client] SO_KEEPALIVE failed (errno=" << errno << ")";
    return;
  }

#if defined(__linux__)
  int idle = 60;
  int intvl = 10;
  int cnt = 3;
  ::setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(idle));
  ::setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &intvl, sizeof(intvl));
  ::setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &cnt, sizeof(cnt));
#elif defined(__APPLE__)
  int idle = 60;
  ::setsockopt(fd, IPPROTO_TCP, TCP_KEEPALIVE, &idle, sizeof(idle));
#endif
}

drogon::HttpClientPtr DrogonHttpClient::leaseClient(
    const std::string &protocol,
    const std::string &host
) const {
  const std::string key = protocol + "://" + host;

  std::lock_guard<std::mutex> lock(_poolsMutex);
  auto it = _pools.find(key);
  if (it == _pools.end()) {
    std::vector<drogon::HttpClientPtr> pool;
    pool.reserve(kPoolSize);
    for (std::size_t i = 0; i < kPoolSize; ++i) {
      auto client = drogon::HttpClient::newHttpClient(
          protocol + "://" + host,
          _loopThread->getLoop()
      );
      client->setUserAgent("ConferenceBot/1.0");
      client->setSockOptCallback(&DrogonHttpClient::applyKeepAlive);
      pool.push_back(std::move(client));
    }
    LOG_INFO << "[http-client] Created connection pool for " << key
             << " (size=" << kPoolSize << ")";
    it = _pools.emplace(key, std::move(pool)).first;
  }

  const std::size_t index = _roundRobin.fetch_add(1, std::memory_order_relaxed)
                            % it->second.size();
  return it->second[index];
}

std::string DrogonHttpClient::makeRequest(
    const TgBot::Url &url,
    const std::vector<TgBot::HttpReqArg> &args
) const {
  auto client = leaseClient(url.protocol, url.host);

  auto request = drogon::HttpRequest::newHttpRequest();
  std::string path = url.path;
  if (!url.query.empty()) {
    path.push_back('?');
    path.append(url.query);
  }
  request->setPath(path);
  request->setPathEncode(false);

  if (args.empty()) {
    request->setMethod(drogon::Get);
  } else {
    request->setMethod(drogon::Post);
    const bool hasFile = std::ranges::any_of(args, [](const auto &arg) {
      return arg.isFile;
    });
    if (hasFile) {
      auto [body, contentType] = buildMultipartBody(args);
      request->setContentTypeString(contentType);
      request->setBody(std::move(body));
    } else {
      request->setContentTypeCode(drogon::CT_APPLICATION_X_FORM);
      request->setBody(buildFormUrlEncodedBody(args));
    }
  }

  const double timeout = static_cast<double>(_timeout);
  auto [result, response] = client->sendRequest(request, timeout);

  if (result != drogon::ReqResult::Ok || !response) {
    const std::string description =
        std::format("Drogon HTTP transport error: {}", to_string(result));
    LOG_ERROR << "[http-client] " << description << " (url=" << url.protocol
              << url.host << url.path << ")";
    throw TgBot::TgException(
        description,
        TgBot::TgException::ErrorCode::Undefined
    );
  }

  return std::string(response->getBody());
}

} // namespace ConferenceBot
