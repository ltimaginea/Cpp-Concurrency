#ifndef PEACE_CONCURRENCY_BOUNDED_BLOCKING_QUEUE_HPP_
#define PEACE_CONCURRENCY_BOUNDED_BLOCKING_QUEUE_HPP_

#include <condition_variable>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <utility>

#include "peace/exception/exception.hpp"

namespace peace
{
namespace concurrency
{

template <typename T>
class BoundedBlockingQueue
{
 public:
  using ContainerType = std::queue<T>;
  using ValueType     = typename ContainerType::value_type;
  using SizeType      = typename ContainerType::size_type;

  explicit BoundedBlockingQueue(SizeType max_queue_size) noexcept(false)
      : closed_(false), max_queue_size_(max_queue_size)
  {
    if (max_queue_size_ == 0)
    {
      throw std::invalid_argument("The maximum queue size cannot be zero!");
    }
  }

  BoundedBlockingQueue(const BoundedBlockingQueue&)            = delete;
  BoundedBlockingQueue(BoundedBlockingQueue&&)                 = delete;
  BoundedBlockingQueue& operator=(const BoundedBlockingQueue&) = delete;
  BoundedBlockingQueue& operator=(BoundedBlockingQueue&&)      = delete;

  ~BoundedBlockingQueue() = default;

  bool Full() const
  {
    std::lock_guard<std::mutex> lock(mtx_);
    return queue_.size() == max_queue_size_;
  }

  bool Empty() const
  {
    std::lock_guard<std::mutex> lock(mtx_);
    return queue_.empty();
  }

  SizeType Size() const
  {
    std::lock_guard<std::mutex> lock(mtx_);
    return queue_.size();
  }

  bool IsClosed() const
  {
    std::lock_guard<std::mutex> lock(mtx_);
    return closed_;
  }

  void Push(const ValueType& element) noexcept(false)
  {
    std::unique_lock<std::mutex> lock(mtx_);
    not_full_.wait(lock, [this] { return queue_.size() < max_queue_size_ || closed_; });
    if (closed_)
    {
      throw peace::exception::ClosedConcurrentQueue();
    }
    queue_.push(element);
    lock.unlock();
    not_empty_.notify_one();
  }

  void Push(ValueType&& element) noexcept(false)
  {
    std::unique_lock<std::mutex> lock(mtx_);
    not_full_.wait(lock, [this] { return queue_.size() < max_queue_size_ || closed_; });
    if (closed_)
    {
      throw peace::exception::ClosedConcurrentQueue();
    }
    queue_.push(std::move(element));
    lock.unlock();
    not_empty_.notify_one();
  }

  ValueType Pop() noexcept(false)
  {
    std::unique_lock<std::mutex> lock(mtx_);
    not_empty_.wait(lock, [this] { return !queue_.empty() || closed_; });
    if (closed_ && queue_.empty())
    {
      throw peace::exception::ClosedConcurrentQueue();
    }
    ValueType element(std::move(queue_.front()));
    queue_.pop();
    lock.unlock();
    not_full_.notify_one();
    return element;
  }

  void Close()
  {
    {
      std::lock_guard<std::mutex> lock(mtx_);
      closed_ = true;
    }
    not_full_.notify_all();
    not_empty_.notify_all();
  }

 private:
  bool closed_;
  const SizeType max_queue_size_;
  ContainerType queue_;
  mutable std::mutex mtx_;
  std::condition_variable not_full_;
  std::condition_variable not_empty_;
};

}  // namespace concurrency
}  // namespace peace

#endif  // PEACE_CONCURRENCY_BOUNDED_BLOCKING_QUEUE_HPP_
