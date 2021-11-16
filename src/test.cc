#include <chrono>
#include <ex/safe_queue.h>
#include <iostream>
#include <string>
#include <thread>

using namespace std::chrono_literals;

ex::safe_queue<std::string> q;

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

  t1.join();
  t2.join();
  return 0;
}