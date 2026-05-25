//
//  BotApiExecutor.cpp
//  ConferenceBot
//
//  Created by Ivan Shpileuski on 25/05/2026.
//
//

#include <ConferenceBot/Net/BotApiExecutor.hpp>

#include <trantor/utils/Logger.h>

#include <algorithm>
#include <charconv>
#include <cstdlib>
#include <string_view>

namespace ConferenceBot {
namespace {

constexpr std::size_t kDefaultBotWorkerThreads = 32;
constexpr std::size_t kMaxBotWorkerThreads = 256;

std::size_t resolveBotWorkerThreadCount() {
  const char *env = std::getenv("BOT_WORKER_THREADS");
  if (env == nullptr) {
    return kDefaultBotWorkerThreads;
  }
  const std::string_view view{env};
  std::size_t value = 0;
  auto [ptr, ec] = std::from_chars(view.data(), view.data() + view.size(), value);
  if (ec != std::errc{} || value == 0) {
    LOG_WARN << "[bot-worker] Invalid BOT_WORKER_THREADS='" << env
             << "', falling back to default " << kDefaultBotWorkerThreads;
    return kDefaultBotWorkerThreads;
  }
  return std::min(value, kMaxBotWorkerThreads);
}

} // namespace

WorkerPool::WorkerPool(std::size_t numThreads) {
  _threads.reserve(numThreads);
  for (std::size_t i = 0; i < numThreads; ++i) {
    _threads.emplace_back([this] { workerLoop(); });
  }
  LOG_INFO << "[bot-worker] WorkerPool started with " << numThreads
           << " threads";
}

WorkerPool::~WorkerPool() {
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _stopped = true;
  }
  _cv.notify_all();
  for (auto &thread : _threads) {
    if (thread.joinable()) {
      thread.join();
    }
  }
}

void WorkerPool::post(std::function<void()> task) {
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _tasks.push(std::move(task));
  }
  _cv.notify_one();
}

void WorkerPool::workerLoop() {
  for (;;) {
    std::function<void()> task;
    {
      std::unique_lock<std::mutex> lock(_mutex);
      _cv.wait(lock, [this] { return _stopped || !_tasks.empty(); });
      if (_stopped && _tasks.empty()) {
        return;
      }
      task = std::move(_tasks.front());
      _tasks.pop();
    }
    try {
      task();
    } catch (const std::exception &e) {
      LOG_ERROR << "[bot-worker] Uncaught exception in worker task: "
                << e.what();
    } catch (...) {
      LOG_ERROR << "[bot-worker] Uncaught non-std exception in worker task";
    }
  }
}

WorkerPool &botWorkerPool() {
  static WorkerPool pool{resolveBotWorkerThreadCount()};
  return pool;
}

} // namespace ConferenceBot
