#ifndef SPINLOCK_HPP_OWQGS7C3
#define SPINLOCK_HPP_OWQGS7C3

#include <thread>
#include <atomic>


#if defined(__x86_64__) || defined(_M_X64)
#  define CPU_PAUSE() asm volatile("pause")
#elif defined(__aarch64__) || defined(__arm__)
#  define CPU_PAUSE() asm volatile("yield")
#else
#  define CPU_PAUSE() asm volatile("" ::: "memory")
#endif

namespace kmq {

  class spinlock
  {
    public:
      void lock() noexcept
      {
        static const std::size_t iters = 1024;

        while (true)
        {
          if (!m_lock.exchange(true, std::memory_order_acquire))
            return;
          else
          {
            for (std::size_t i=0; i<iters; i++)
              CPU_PAUSE();
          }

          std::size_t n {0};

          while (m_lock.load(std::memory_order_relaxed))
          {
            if (n < iters)
            {
              CPU_PAUSE();
              n++;
            }
            else
            {
              std::this_thread::yield();
            }
          }
        }
      }

      void unlock() noexcept
      {
        m_lock.store(false, std::memory_order_release);
      }

      bool try_lock() noexcept
      {
        return !m_lock.load(std::memory_order_relaxed) && !m_lock.exchange(true, std::memory_order_acquire);
      }

    private:
      std::atomic<bool> m_lock {false};
  };

}

#endif /* end of include guard: SPINLOCK_HPP_OWQGS7C3 */
