#pragma once

#include <atomic>
#include <cassert>
#include <chrono>
#include <cinttypes>
#include <cstddef>
#include <optional>
#include <thread>
#include <vector>

// https://codereview.stackexchange.com/questions/221854/yet-another-multi-producer-single-consumer-queue-in-c17

namespace ex {
template <typename T, size_t capacity> class lock_free_mpsc {
public:
  lock_free_mpsc();
  bool enqueue(const T &obj);
  std::optional<T>
  try_dequeue(const std::chrono::duration<long long, std::ratio<1, 1000000>>
                  &sleep_duration_if_failed);
  size_t size() const;

private:
  std::atomic<size_t> m_size{0};
  std::atomic<size_t> m_head{0};
  size_t m_tail{0};
  std::atomic<bool> m_is_readable[capacity];
  T m_buffer[capacity];
};

template <typename T, size_t capacity>
lock_free_mpsc<T, capacity>::lock_free_mpsc() {
  assert(capacity >= 1);
  for (auto &i : m_is_readable) {
    i = false;
  }
}

/// Attempt to enqueue without blocking. This is safe to call from multiple
/// threads. \return true on success and false if queue is full.
template <typename T, size_t capacity>
bool lock_free_mpsc<T, capacity>::enqueue(const T &obj) {
  auto count = m_size.fetch_add(1, std::memory_order_acquire);
  if (count >= capacity) {
    // back off, queue is full
    m_size.fetch_sub(1, std::memory_order_release);
    return false;
  }

  // increment the head, which gives us 'exclusive' access to that element until
  // is_reabable_ flag is set
  const auto head = m_head.fetch_add(1, std::memory_order_acquire) % capacity;
  m_buffer[head] = obj;
  assert(m_is_readable[head] == false);
  m_is_readable[head].store(true, std::memory_order_release);
  return true;
}

/// Attempt to dequeue without blocking
/// \note: This is not safe to call from multiple threads.
/// \return A valid item from queue if the operation won't block, else nothing
template <typename T, size_t capacity>
std::optional<T> lock_free_mpsc<T, capacity>::try_dequeue(
    const std::chrono::duration<long long, std::ratio<1, 1000000>>
        &sleep_duration_if_failed) {
  if (!m_is_readable[m_tail].load(std::memory_order_acquire)) {
    // A thread could still be writing to this location
    std::this_thread::sleep_for(sleep_duration_if_failed);
    return {};
  }

  assert(m_is_readable[m_tail]);
  auto ret = std::move(m_buffer[m_tail]);
  m_is_readable[m_tail].store(false, std::memory_order_release);

  if (++m_tail >= capacity) {
    m_tail = 0;
  }

  const auto count = m_size.fetch_sub(1, std::memory_order_release);
  assert(count > 0);
  return ret;
}

/// \return The number of items in queue
template <typename T, size_t capacity>
size_t lock_free_mpsc<T, capacity>::size() const {
  return m_size.load(std::memory_order_relaxed);
}

} // namespace ex