#include "peace/concurrency/bounded_blocking_queue.hpp"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "peace/exception/exception.hpp"

class ProductionConsumptionTask
{
 public:
  using ContainerType      = peace::concurrency::BoundedBlockingQueue<std::string>;
  using ContainerValueType = ContainerType::ValueType;
  using ContainerSizeType  = ContainerType::SizeType;

  ProductionConsumptionTask(ContainerSizeType max_queue_size, std::size_t producers_number,
                            std::size_t consumers_number)
      : queue_(max_queue_size),
        stopped_(false),
        producers_(producers_number),
        consumers_(consumers_number)
  {
  }

  ProductionConsumptionTask(const ProductionConsumptionTask&)            = delete;
  ProductionConsumptionTask(ProductionConsumptionTask&&)                 = delete;
  ProductionConsumptionTask& operator=(const ProductionConsumptionTask&) = delete;
  ProductionConsumptionTask& operator=(ProductionConsumptionTask&&)      = delete;

  ~ProductionConsumptionTask()
  {
    Stop();
    WaitForCompletion();
  }

  void Produce()
  {
    for (auto& thrd : producers_)
    {
      thrd = std::thread(
          [this]
          {
            ContainerValueType data("apple");
            while (!stopped_)
            {
              try
              {
                queue_.Push(data);
                std::printf("Produce %s.\n", data.c_str());
              }
              catch (const peace::exception::ClosedConcurrentQueue& error)
              {
                std::printf("Catch an exception: %s.\n", error.what());
              }
            }
          });
    }
  }

  void Consume()
  {
    for (auto& thrd : consumers_)
    {
      thrd = std::thread(
          [this]
          {
            while (!stopped_)
            {
              try
              {
                ContainerValueType data(queue_.Pop());
                std::printf("Consume %s.\n", data.c_str());
              }
              catch (const peace::exception::ClosedConcurrentQueue& error)
              {
                std::printf("Catch an exception: %s.\n", error.what());
              }
            }
          });
    }
  }

  void Stop()
  {
    stopped_ = true;
    queue_.Close();
  }

  void WaitForCompletion()
  {
    for (auto& thrd : producers_)
    {
      if (thrd.joinable())
      {
        thrd.join();
      }
    }

    for (auto& thrd : consumers_)
    {
      if (thrd.joinable())
      {
        thrd.join();
      }
    }
  }

 private:
  ContainerType queue_;
  std::atomic_bool stopped_;
  std::vector<std::thread> producers_;
  std::vector<std::thread> consumers_;
};

int main()
{
  ProductionConsumptionTask task(8, 7, 6);
  task.Produce();
  task.Consume();

  std::thread stop(
      [&task]
      {
        std::this_thread::sleep_for(std::chrono::minutes(1));
        task.Stop();
      });

  task.WaitForCompletion();
  stop.join();

  return 0;
}
