#ifndef SCOPION_ERROR_H_
#define SCOPION_ERROR_H_

#include <algorithm>

#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/iterator_range.hpp>

namespace scopion
{
class error : public std::runtime_error
{
  using str_range_t = boost::iterator_range<std::string::const_iterator>;

  static str_range_t line_range(str_range_t const where, str_range_t const code)
  {
    auto range = boost::make_iterator_range(code.begin(), where.begin());
    auto sol   = boost::find(range | boost::adaptors::reversed, '\n').base();
    auto eol   = std::find(where.begin(), code.end(), '\n');
    return boost::make_iterator_range(sol, eol);
  }
  uint64_t line_number_;
  uint64_t distance_;
  std::string line_;
  uint8_t level_;

public:
  error(std::string const& message,
        str_range_t const where,
        str_range_t const code,
        uint8_t level = 0)
      : std::runtime_error(message), level_(level)
  {
    line_number_ = std::count(code.begin(), where.begin(), '\n');
    auto r       = line_range(where, code);
    line_        = std::string(r.begin(), r.end());
    distance_    = std::distance(r.begin(), where.begin());
  }

  uint64_t line_number() const { return line_number_; }
  std::string line() const { return line_; }
  uint64_t distance() const { return distance_; }
  uint8_t level() const { return level_; }
};

};  // namespace scopion

#endif  // SCOPION_EXCEPTIONS_H_
