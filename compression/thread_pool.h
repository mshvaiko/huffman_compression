#ifndef COMPRESSION_THREAD_POOL_H_
#define COMPRESSION_THREAD_POOL_H_

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <future>
#include <vector>

namespace compression {

class thread_pool {
 private:
  // need to keep track of threads so we can join them
  std::vector<std::thread> workers_;
  // the task queue
  std::queue<std::function<void()>> tasks_;

  // synchronization
  std::mutex mutex_;
  std::condition_variable dispatching_notifier_;
  size_t pool_size_;
  std::atomic_bool running_;

 public:
  explicit thread_pool(size_t pool_size);
  ~thread_pool();

  // this object shouldn't be copied
  thread_pool(const thread_pool&) = delete;
  thread_pool& operator=(const thread_pool&) = delete;

  void run();

  void dispatch(std::function<void()> task);
  void async_job(std::function<void()> task);
};

}  // namespace compression

#endif