#include <array>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <queue>
#include <stop_token>
#include <string>
#include <thread>
#include <utility>

class ProducerConsumerUnboundedQueueTask
{
 public:
  void Produce(std::stop_token stoken, const std::string& product)
  {
    while (!stoken.stop_requested())
    {
      std::unique_lock<std::mutex> lock(mtx_);
      queue_.push(product);
      lock.unlock();
      not_empty_.notify_one();
      // Use sleep to simulate other production processes.
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }

  void Consume(std::stop_token stoken)
  {
    while (!stoken.stop_requested())
    {
      std::unique_lock<std::mutex> lock(mtx_);
      if (!not_empty_.wait(lock, stoken, [this] { return !queue_.empty(); }))
      {
        break;
      }
      std::string data(std::move(queue_.front()));
      queue_.pop();
      lock.unlock();
      // Use print to simulate consuming products.
      std::printf("Consume %s.\n", data.c_str());
    }
  }

 private:
  std::queue<std::string> queue_;
  std::mutex mtx_;
  std::condition_variable_any not_empty_;
};

int main()
{
  ProducerConsumerUnboundedQueueTask task;

  std::array producers = {
      std::jthread([&task](std::stop_token stoken) { task.Produce(stoken, "apple"); }),
      std::jthread([&task](std::stop_token stoken) { task.Produce(stoken, "orange"); }),
      std::jthread([&task](std::stop_token stoken) { task.Produce(stoken, "banana"); })};

  std::array consumers = {std::jthread([&task](std::stop_token stoken) { task.Consume(stoken); }),
                          std::jthread([&task](std::stop_token stoken) { task.Consume(stoken); })};

  std::this_thread::sleep_for(std::chrono::minutes(1));

  return 0;
}
