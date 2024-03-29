#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

namespace ex {
template <class T, size_t capacity = 1024> class safe_queue {
public:
  explicit safe_queue() : m_queue(), m_mutex(), m_cv() {}

  ~safe_queue(void) {}

  bool enqueue(T t) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_queue.size() < capacity) {
      m_queue.push(t);
      m_cv.notify_one();
      return true;
    }
    return false;
  }

  T dequeue() {
    std::unique_lock<std::mutex> lock(m_mutex);
    while (m_queue.empty())
      m_cv.wait(lock);
    T val = m_queue.front();
    m_queue.pop();
    return val;
  }

  std::optional<T> try_dequeue(
      const std::chrono::duration<long long, std::ratio<1, 1000000>> &timeout) {
    std::unique_lock<std::mutex> lock(m_mutex);
    while (m_queue.empty()) {
      auto status = m_cv.wait_for(lock, timeout);
      if (status == std::cv_status::timeout)
        return {};
    }
    T val = m_queue.front();
    m_queue.pop();
    return val;
  }

  std::vector<T> pop_all() {
    std::unique_lock<std::mutex> lock(m_mutex);
    std::vector<T> v;
    while (!m_queue.empty()) {
      T val = m_queue.front();
      m_queue.pop();
      v.push_back(val);
    }
    return v;
  }

private:
  std::queue<T> m_queue;
  mutable std::mutex m_mutex;
  std::condition_variable m_cv;
};
} // namespace ex
