#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <thread>

class CV
{
 public:
  CV() : flag_(false) {}

  void Notify()
  {
    {
      std::lock_guard<std::mutex> lock(mtx_);
      flag_ = true;
    }  // Unlock.
    cv_.notify_one();
  }

  void Wait()
  {
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock, [this] { return flag_; });
  }

 private:
  bool flag_;
  std::mutex mtx_;
  std::condition_variable cv_;
};

int main()
{
  CV cv;

  std::thread thrd1(
      [&cv]
      {
        std::printf("1");
        std::fflush(stdout);

        cv.Notify();
      });

  std::thread thrd2(
      [&cv]
      {
        cv.Wait();

        std::printf("2");
        std::fflush(stdout);
      });

  thrd1.join();
  thrd2.join();

  return 0;
}
