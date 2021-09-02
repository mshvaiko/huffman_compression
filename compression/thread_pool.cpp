#include "thread_pool.h"

#include <iostream>
#include <future>

namespace compression {

thread_pool::thread_pool(size_t pool_size = std::thread::hardware_concurrency()) : pool_size_(pool_size), running_(false) {
}

void thread_pool::run() {
  running_ = true;
  for (size_t i = 0; i < pool_size_; ++i)
    workers_.emplace_back([this] {
      for (;;) {
        std::function<void()> task;
        {
          std::unique_lock<std::mutex> lock(this->mutex_);
          this->dispatching_notifier_.wait(lock, [this] { return !this->running_ || !this->tasks_.empty(); });

          if (!this->running_ && this->tasks_.empty()) return;

          task = std::move(this->tasks_.front());
          this->tasks_.pop();
        }
        task();
      }
    });
}

void thread_pool::dispatch(std::function<void()> task) {
  {
    std::unique_lock<std::mutex> lock(mutex_);

    // don't allow enqueueing after stopping the pool
    if (!running_) {
      std::cout << "Failed to dispatch task - pool has already stopped" << std::endl;
      return;
    }

    tasks_.emplace(task);
  }
  dispatching_notifier_.notify_one();
}

void thread_pool::async_job(std::function<void()> task) {
  auto result = std::async(std::launch::deferred, task);
  result.wait();
}

thread_pool::~thread_pool() {
  {
    std::unique_lock<std::mutex> lock(mutex_);
    running_ = false;
  }
  dispatching_notifier_.notify_all();
  for (std::thread& worker : workers_) worker.join();
}

}  // namespace compression
