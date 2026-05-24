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

#include <algorithm>
#include <atomic>
#include <chrono>
#include <format>
#include <string>

namespace ConferenceBot {
namespace {

std::string buildHostKey(const std::string &protocol, const std::string &host) {
  return protocol + host;
}

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
              "thread 'TgBotHttpLoop'";
}

DrogonHttpClient::~DrogonHttpClient() {
  {
    std::lock_guard<std::mutex> lock(_clientsMutex);
    _clients.clear();
  }
  if (_loopThread && _loopThread->getLoop()) {
    _loopThread->getLoop()->quit();
  }
}

drogon::HttpClientPtr DrogonHttpClient::getOrCreateClient(
    const std::string &protocol,
    const std::string &host
) const {
  const std::string key = buildHostKey(protocol, host);

  std::lock_guard<std::mutex> lock(_clientsMutex);
  auto it = _clients.find(key);
  if (it != _clients.end()) {
    return it->second;
  }

  auto client = drogon::HttpClient::newHttpClient(
      protocol + host,
      _loopThread->getLoop()
  );
  client->setUserAgent("ConferenceBot/1.0");
  _clients.emplace(key, client);
  LOG_INFO << "[http-client] Created persistent HTTP client for " << key;
  return client;
}

std::string DrogonHttpClient::makeRequest(
    const TgBot::Url &url,
    const std::vector<TgBot::HttpReqArg> &args
) const {
  auto client = getOrCreateClient(url.protocol, url.host);

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
