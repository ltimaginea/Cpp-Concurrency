#ifndef PEACE_EXCEPTION_EXCEPTION_HPP_
#define PEACE_EXCEPTION_EXCEPTION_HPP_

#include <exception>

namespace peace
{
namespace exception
{

class ClosedConcurrentQueue : public std::exception
{
 public:
  const char* what() const noexcept override { return "closed concurrent queue"; }
};

}  // namespace exception
}  // namespace peace

#endif  // PEACE_EXCEPTION_EXCEPTION_HPP_
