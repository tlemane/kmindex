#ifndef THREADPOOL_HPP_DMXPPU0Y
#define THREADPOOL_HPP_DMXPPU0Y

#include <future>
#include <iostream>
#include <queue>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace kmq {

  class ThreadPool
  {
    using size_type = std::result_of<decltype (&std::thread::hardware_concurrency)()>::type;

   public:
    ThreadPool(size_type threads);

    ThreadPool() = delete;
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    ~ThreadPool();

    void join_all();

    void join(int i);

    template <typename Callable>
    void add_task(Callable&& f)
    {
      auto task = std::make_shared<std::packaged_task<void(int)>>(std::forward<Callable>(f));
      {
        std::unique_lock<std::mutex> lock(_queue_mutex);
        if (_stop) throw std::runtime_error("Push on stopped Pool.");
        _queue.emplace([task](int thread_id) { (*task)(thread_id); });
      }
      _condition.notify_one();
    }

   private:
    void worker(int i);

   private:
    size_type _n{std::thread::hardware_concurrency()};
    std::vector<std::thread> _pool;
    std::queue<std::function<void(int)>> _queue;
    std::mutex _queue_mutex;
    std::condition_variable _condition;
    bool _stop{false};
  };

}

#endif /* end of include guard: THREADPOOL_HPP_DMXPPU0Y */
