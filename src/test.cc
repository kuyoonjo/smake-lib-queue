#include <chrono>
#include <ex/safe_queue.h>
#include <iostream>
#include <string>
#include <thread>

using namespace std::chrono_literals;

ex::safe_queue<std::string> q;
ex::safe_queue<std::string> q2;

int main() {
  std::thread t1([]() {
    while (true) {
      std::this_thread::sleep_for(1s);
      q.enqueue("I will kill you!");
    }
  });

  std::thread t2([]() {
    while (true) {
      auto msg = q.dequeue();
      std::cout << msg << std::endl;
    }
  });

  std::thread t3([]() {
    while (true) {
      auto msg = q2.try_dequeue(1s);
      if (msg.has_value())
        std::cout << *msg << std::endl;
      else
        std::cout << "Timeout" << std::endl;
    }
  });

  t1.join();
  t2.join();
  t3.join();
  return 0;
}