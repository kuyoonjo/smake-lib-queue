#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

namespace ex {
template <class T> class safe_queue {
public:
  explicit safe_queue() : m_queue(), m_mutex(), m_cv() {}

  ~safe_queue(void) {}

  void enqueue(T t) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(t);
    m_cv.notify_one();
  }

  T dequeue() {
    std::unique_lock<std::mutex> lock(m_mutex);
    while (m_queue.empty())
      m_cv.wait(lock);
    T val = m_queue.front();
    m_queue.pop();
    return val;
  }

private:
  std::queue<T> m_queue;
  mutable std::mutex m_mutex;
  std::condition_variable m_cv;
};
} // namespace ex
