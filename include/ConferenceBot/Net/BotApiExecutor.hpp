//
//  BotApiExecutor.hpp
//  ConferenceBot
//
//  Created by Ivan Shpileuski on 25/05/2026.
//
//

#pragma once

#include <condition_variable>
#include <coroutine>
#include <cstddef>
#include <exception>
#include <functional>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace ConferenceBot {

/// Simple fixed-size thread pool with a FIFO task queue.
///
/// Threads in this pool are NOT Drogon event loop threads, so blocking work
/// (e.g. synchronous Telegram Bot API calls via libcurl-style HTTP clients)
/// can run here without stalling Drogon's HTTP listeners or database
/// connection loops.
class WorkerPool {
public:
  explicit WorkerPool(std::size_t numThreads);
  ~WorkerPool();

  WorkerPool(const WorkerPool &) = delete;
  WorkerPool &operator=(const WorkerPool &) = delete;

  void post(std::function<void()> task);
  std::size_t size() const noexcept { return _threads.size(); }

private:
  void workerLoop();

  std::vector<std::thread> _threads;
  std::queue<std::function<void()>> _tasks;
  std::mutex _mutex;
  std::condition_variable _cv;
  bool _stopped = false;
};

/// Process-wide pool used to execute blocking Telegram Bot API calls.
/// Sized via BOT_WORKER_THREADS env var (default 32).
WorkerPool &botWorkerPool();

namespace detail {

template <typename F, typename R>
struct PoolAwaiterValue {
  WorkerPool &pool;
  F func;
  std::optional<R> result;
  std::exception_ptr exception;

  bool await_ready() const noexcept { return false; }

  void await_suspend(std::coroutine_handle<> handle) {
    pool.post([this, handle]() mutable {
      try {
        result.emplace(func());
      } catch (...) {
        exception = std::current_exception();
      }
      handle.resume();
    });
  }

  R await_resume() {
    if (exception) {
      std::rethrow_exception(exception);
    }
    return std::move(*result);
  }
};

template <typename F>
struct PoolAwaiterVoid {
  WorkerPool &pool;
  F func;
  std::exception_ptr exception;

  bool await_ready() const noexcept { return false; }

  void await_suspend(std::coroutine_handle<> handle) {
    pool.post([this, handle]() mutable {
      try {
        func();
      } catch (...) {
        exception = std::current_exception();
      }
      handle.resume();
    });
  }

  void await_resume() {
    if (exception) {
      std::rethrow_exception(exception);
    }
  }
};

} // namespace detail

/// Wrap a synchronous (blocking) callable so it can be co_await-ed from a
/// coroutine without blocking the calling thread.
///
/// The callable runs on the shared bot worker pool. After it returns, the
/// coroutine resumes on the same worker pool thread, so subsequent
/// synchronous calls in the coroutine body also stay off Drogon's loops.
///
/// Usage:
///   auto msg = co_await onBotPool([&] {
///     return _bot.getApi().sendMessage(chatId, text);
///   });
template <typename F>
auto onBotPool(F func) {
  using R = std::invoke_result_t<F>;
  if constexpr (std::is_void_v<R>) {
    return detail::PoolAwaiterVoid<F>{botWorkerPool(), std::move(func), nullptr};
  } else {
    return detail::PoolAwaiterValue<F, R>{
        botWorkerPool(),
        std::move(func),
        std::nullopt,
        nullptr
    };
  }
}

} // namespace ConferenceBot
