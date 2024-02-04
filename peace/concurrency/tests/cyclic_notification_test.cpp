#include "peace/concurrency/cyclic_notification.hpp"

#include <chrono>
#include <ios>
#include <iostream>
#include <ostream>
#include <thread>

int main()
{
  peace::concurrency::CyclicNotification event1(true);
  peace::concurrency::CyclicNotification event2;
  peace::concurrency::CyclicNotification event3;

  constexpr unsigned int number = 10;

  std::thread thrd1(
      [&event1, &event2]
      {
        for (unsigned int i = 0; i < number; ++i)
        {
          event1.Wait();

          std::cout << 1 << std::flush;
          std::this_thread::sleep_for(std::chrono::seconds(1));

          event2.Notify();
        }
      });

  std::thread thrd2(
      [&event2, &event3]
      {
        for (unsigned int i = 0; i < number; ++i)
        {
          event2.Wait();

          std::cout << 2 << std::flush;
          std::this_thread::sleep_for(std::chrono::seconds(1));

          event3.Notify();
        }
      });

  std::thread thrd3(
      [&event3, &event1]
      {
        for (unsigned int i = 0; i < number; ++i)
        {
          event3.Wait();

          std::cout << 3 << std::endl;
          std::this_thread::sleep_for(std::chrono::seconds(1));

          event1.Notify();
        }
      });

  thrd1.join();
  thrd2.join();
  thrd3.join();

  std::cout << std::boolalpha;

  peace::concurrency::CyclicNotification event4;
  std::thread thrd4([&event4]
                    { std::cout << event4.WaitFor(std::chrono::seconds(10)) << std::endl; });
  std::this_thread::sleep_for(std::chrono::seconds(3));
  event4.Notify();
  thrd4.join();

  peace::concurrency::CyclicNotification event5;
  std::thread thrd5(
      [&event5]
      {
        std::cout << event5.WaitUntil(std::chrono::system_clock::now() + std::chrono::seconds(10))
                  << std::endl;
      });
  std::this_thread::sleep_for(std::chrono::seconds(7));
  event5.Notify();
  thrd5.join();

  peace::concurrency::CyclicNotification event6;
  std::cout << event6.WaitFor(std::chrono::seconds(10)) << std::endl;

  peace::concurrency::CyclicNotification event7;
  std::cout << event7.WaitUntil(std::chrono::system_clock::now() + std::chrono::seconds(10))
            << std::endl;

  std::cout << std::noboolalpha;

  return 0;
}