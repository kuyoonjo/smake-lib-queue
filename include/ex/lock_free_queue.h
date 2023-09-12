#pragma once

#include <atomic>
#include <cstddef>
#include <optional>
#include <thread>
#include <type_traits>

namespace ex {
template <typename T, size_t capacity = 1024> class lock_free_queue {
private:
  struct slot {
    T val;
    std::atomic_size_t pop_count;
    std::atomic_size_t push_count;

    slot() : pop_count(0U), push_count(0U) {}
  };

  slot m_data[capacity];
  std::atomic_size_t m_r_count;
  std::atomic_size_t m_w_count;

public:
  lock_free_queue() : m_r_count(0U), m_w_count(0U) {}

  bool enqueue(const T &element) {
    size_t w_count = m_w_count.load(std::memory_order_relaxed);

    while (true) {
      const size_t index = w_count % capacity;

      const size_t push_count =
          m_data[index].push_count.load(std::memory_order_acquire);
      const size_t pop_count =
          m_data[index].pop_count.load(std::memory_order_relaxed);

      if (push_count > pop_count) {
        return false;
      }

      const size_t revolution_count = w_count / capacity;
      const bool our_turn = revolution_count == push_count;

      if (our_turn) {
        /* Try to acquire the slot by bumping the monotonic write counter */
        if (m_w_count.compare_exchange_weak(w_count, w_count + 1U,
                                            std::memory_order_relaxed)) {
          m_data[index].val = element;
          m_data[index].push_count.store(push_count + 1U,
                                         std::memory_order_release);
          return true;
        }
      } else {
        w_count = m_w_count.load(std::memory_order_relaxed);
      }
    }
  }

  std::optional<T>
  try_dequeue(const std::chrono::duration<long long, std::ratio<1, 1000000>>
                  &sleep_duration_if_failed) {
    size_t r_count = m_r_count.load(std::memory_order_relaxed);

    while (true) {
      const size_t index = r_count % capacity;

      const size_t pop_count =
          m_data[index].pop_count.load(std::memory_order_acquire);
      const size_t push_count =
          m_data[index].push_count.load(std::memory_order_relaxed);

      if (pop_count == push_count) {
        std::this_thread::sleep_for(sleep_duration_if_failed);
        return {};
      }

      const size_t revolution_count = r_count / capacity;
      const bool our_turn = revolution_count == pop_count;

      if (our_turn) {
        /* Try to acquire the slot by bumping the monotonic read counter. */
        if (m_r_count.compare_exchange_weak(r_count, r_count + 1U,
                                            std::memory_order_relaxed)) {
          T element = m_data[index].val;
          m_data[index].pop_count.store(pop_count + 1U,
                                        std::memory_order_release);
          return element;
        }
      } else {
        r_count = m_r_count.load(std::memory_order_relaxed);
      }
    }
  }
};
} // namespace ex
