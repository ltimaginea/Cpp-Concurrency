#include <array>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <stop_token>
#include <string>
#include <thread>
#include <utility>

class ProducerConsumerBoundedQueueTask
{
 public:
  using ContainerType      = std::queue<std::string>;
  using ContainerValueType = ContainerType::value_type;
  using ContainerSizeType  = ContainerType::size_type;

  explicit ProducerConsumerBoundedQueueTask(ContainerSizeType max_queue_size) noexcept(false)
      : max_queue_size_(max_queue_size)
  {
    if (max_queue_size_ == 0)
    {
      throw std::invalid_argument("The maximum queue size cannot be zero!");
    }
  }

  void Produce(std::stop_token stoken, const ContainerValueType& product)
  {
    while (!stoken.stop_requested())
    {
      std::unique_lock<std::mutex> lock(mtx_);
      if (!not_full_.wait(lock, stoken, [this] { return queue_.size() < max_queue_size_; }))
      {
        break;
      }
      queue_.push(product);
      lock.unlock();
      not_empty_.notify_one();
      // Use sleep to simulate other production processes.
      std::this_thread::sleep_for(std::chrono::milliseconds(300));
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
      ContainerValueType data(std::move(queue_.front()));
      queue_.pop();
      lock.unlock();
      not_full_.notify_one();
      // Use print to simulate consuming products.
      std::printf("Consume %s.\n", data.c_str());
    }
  }

 private:
  const ContainerSizeType max_queue_size_;
  ContainerType queue_;
  std::mutex mtx_;
  std::condition_variable_any not_full_;
  std::condition_variable_any not_empty_;
};

int main()
{
  ProducerConsumerBoundedQueueTask task(4);

  std::array producers = {
      std::jthread([&task](std::stop_token stoken) { task.Produce(stoken, "apple"); }),
      std::jthread([&task](std::stop_token stoken) { task.Produce(stoken, "orange"); }),
      std::jthread([&task](std::stop_token stoken) { task.Produce(stoken, "banana"); })};

  std::array consumers = {std::jthread([&task](std::stop_token stoken) { task.Consume(stoken); }),
                          std::jthread([&task](std::stop_token stoken) { task.Consume(stoken); })};

  std::this_thread::sleep_for(std::chrono::minutes(1));

  return 0;
}
