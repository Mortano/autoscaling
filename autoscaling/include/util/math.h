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

/// <summary>
///  Combines hash values
/// </summary>
template <typename T>
inline void hash_combine(size_t& hash, T const& val) {
  // Stole this from
  // https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
  std::hash<T> hasher;
  hash ^= hasher(val) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
}

}  // namespace math

}  // namespace as
