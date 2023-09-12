#include <chrono>
#include <ex/lock_free_queue.h>
#include <ex/safe_queue.h>
#include <iostream>
#include <string>
#include <thread>

using namespace std::chrono_literals;

ex::safe_queue<std::string> q;
ex::safe_queue<std::string> q2;

ex::lock_free_queue<std::string> lq;

int main() {
  std::thread t1([]() {
    while (true) {
      std::this_thread::sleep_for(1s);
      std::cout << "t1: I'm sending a message" << std::endl;
      q.enqueue("t1");
    }
  });

  std::thread t2([]() {
    while (true) {
      auto msg = q.dequeue();
      std::cout << "t2: I got a message from " << msg << std::endl;
    }
  });

  std::thread t3([]() {
    while (true) {
      auto msg = q2.try_dequeue(1s);
      if (msg.has_value())
        std::cout << *msg << std::endl;
      else
        std::cout << "t3: Timeout" << std::endl;
    }
  });

  std::thread t4([]() {
    while (true) {
      std::this_thread::sleep_for(1s);
      std::cout << "t4: I'm sending a message" << std::endl;
      std::string msg = "t4";
      auto ok = lq.enqueue("t4");
      if (ok) {
        std::cout << "t4: I sent a message" << std::endl;
      } else {
        std::cout << "t4: I'm failed to send a message" << std::endl;
      }
    }
  });

  std::thread t5([]() {
    std::this_thread::sleep_for(5s);
    while (true) {
      auto msg = lq.try_dequeue(1ms);
      if (msg.has_value()) {
        std::cout << "t5: I got a message from " << *msg << std::endl;
      }
    }
  });

  t1.join();
  t2.join();
  t3.join();
  t4.join();
  t5.join();
  return 0;
}