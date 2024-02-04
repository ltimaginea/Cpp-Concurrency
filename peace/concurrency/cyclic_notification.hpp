#ifndef PEACE_CONCURRENCY_CYCLIC_NOTIFICATION_HPP_
#define PEACE_CONCURRENCY_CYCLIC_NOTIFICATION_HPP_

#include <chrono>
#include <condition_variable>
#include <mutex>

namespace peace
{
namespace concurrency
{

// The class CyclicNotification is a reusable thread notification.
class CyclicNotification
{
 public:
  // Initialize the notified state to unnotified.
  CyclicNotification() : notified_(false) {}
  explicit CyclicNotification(bool prenotification) : notified_(prenotification) {}

  CyclicNotification(const CyclicNotification&) = delete;
  CyclicNotification(CyclicNotification&&) = delete;
  CyclicNotification& operator=(const CyclicNotification&) = delete;
  CyclicNotification& operator=(CyclicNotification&&) = delete;

  ~CyclicNotification() = default;

  void Notify()
  {
    {
      std::lock_guard<std::mutex> lock(mtx_);
      notified_ = true;
    }  // Unlock.
    cv_.notify_one();
  }

  void Wait()
  {
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock, [this] { return notified_; });

    // Reset the flag.
    notified_ = false;
    lock.unlock();
  }

  // Return false if timeout, otherwise return true.
  template <typename Clock, typename Duration>
  bool WaitUntil(const std::chrono::time_point<Clock, Duration>& abs_time)
  {
    std::unique_lock<std::mutex> lock(mtx_);
    bool is_notified = cv_.wait_until(lock, abs_time, [this] { return notified_; });

    if (is_notified)
    {
      // Reset the flag.
      notified_ = false;
    }
    lock.unlock();
    return is_notified;
  }

  // Return false if timeout, otherwise return true.
  template <typename Rep, typename Period>
  bool WaitFor(const std::chrono::duration<Rep, Period>& rel_time)
  {
    std::unique_lock<std::mutex> lock(mtx_);
    bool is_notified = cv_.wait_for(lock, rel_time, [this] { return notified_; });

    if (is_notified)
    {
      // Reset the flag.
      notified_ = false;
    }
    lock.unlock();
    return is_notified;
  }

 private:
  bool notified_;
  std::mutex mtx_;
  std::condition_variable cv_;
};

}  // namespace concurrency
}  // namespace peace

#endif  // PEACE_CONCURRENCY_CYCLIC_NOTIFICATION_HPP_