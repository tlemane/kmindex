#include <kmindex/threadpool.hpp>

namespace kmq {

  ThreadPool::ThreadPool(size_type threads)
  {
    if (threads < _n) _n = threads;
    for (size_t i = 0; i < _n; i++)
    {
      _pool.push_back(std::thread(&ThreadPool::worker, this, i));
    }
  }

  void ThreadPool::restart(size_type threads)
  {
    _n = threads ? threads : _n;
    _stop = false;
    _pool.clear();
    for (size_t i = 0; i < _n; i++)
    {
      _pool.push_back(std::thread(&ThreadPool::worker, this, i));
    }
  }

  ThreadPool::~ThreadPool()
  {
    {
      std::unique_lock<std::mutex> lock(_queue_mutex);
      _stop = true;
    }
    _condition.notify_all();
    for (std::thread& t : _pool)
      if (t.joinable()) t.join();
  }

  void ThreadPool::join_all()
  {
    {
      std::unique_lock<std::mutex> lock(_queue_mutex);
      _stop = true;
    }
    _condition.notify_all();
    for (std::thread& t : _pool) t.join();
  }

  void ThreadPool::join(int i)
  {
    if (_pool[i].joinable()) _pool[i].join();
  }

  void ThreadPool::worker(int i)
  {
    while (true)
    {
      std::function<void(int)> task;
      {
        std::unique_lock<std::mutex> lock(this->_queue_mutex);
        this->_condition.wait(lock, [this] { return this->_stop || !this->_queue.empty(); });
        if (this->_stop && this->_queue.empty()) return;
        task = std::move(this->_queue.front());
        this->_queue.pop();
      }
      task(i);
    }
  }

} // end of namespace kmdiff

