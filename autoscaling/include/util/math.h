#pragma once

#include <type_traits>

namespace as {

namespace math {

/// <summary>
/// Increments the given value and wraps around when the incremented value is
/// greater than or equal to the given threshold value
/// </summary>
template <typename T>
void inc_wrap(T& val, T threshold) {
  static_assert(std::is_arithmetic_v<T>, "inc_wrap requires arithmetic type");
  ++val;
  if (val >= threshold) val = T{0};
}

/// <summary>
/// Decrements the given value and wraps around to (threshold - 1) if the value
/// would become less than zero
/// </summary>
template <typename T>
void dec_wrap(T& val, T threshold) {
  if (val <= T{0})
    val = (threshold - 1);
  else
    --val;
}

}  // namespace math

}  // namespace as
