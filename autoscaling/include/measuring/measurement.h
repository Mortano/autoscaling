#pragma once

#include "api.h"

#include <chrono>
#include <string_view>
#include <thread>
#include <unordered_map>

namespace as {

#pragma region time

using timestamp_t = std::chrono::high_resolution_clock::time_point;
using timespan_t = std::chrono::nanoseconds;

/// <summary>
/// Returns a timestamp for the current point in time upon calling this function
/// </summary>
/// <returns>Timestamp for the current point in time</returns>
timestamp_t now() { return std::chrono::high_resolution_clock::now(); }

#pragma endregion

#pragma region thread

using thread_id_t = std::thread::id;

#pragma endregion

#pragma region types

template <typename T>
struct measurement {
  T data;
  timestamp_t timestamp;
};

struct AS_API function_call {};
struct AS_API periodic_event {};

#pragma region resources

template <typename T>
struct resource {
  resource(T&& val) : _val(std::move(val)) {}

 private:
  T _val;
};

struct AS_API memory {
  memory();
  explicit memory(size_t size);

  memory(const memory&) = default;
  memory& operator=(const memory&) = default;

  operator size_t() const;

  size_t get_size() const;

 private:
  size_t _bytes;
};

memory operator+(const memory& l, const memory& r);
memory operator-(const memory& l, const memory& r);
memory operator*(const memory& m, size_t s);

namespace literals {
/// <summary>
/// Creates a memory structure representing the given number of bytes
/// </summary>
/// <param name="size">Number of bytes</param>
/// <returns>memory structure</returns>
memory AS_API operator"" _B(size_t size);

/// <summary>
/// Creates a memory structure representing the given number of kibibytes
/// </summary>
/// <param name="size">Number of bytes</param>
/// <returns>memory structure</returns>
memory AS_API operator"" _KiB(size_t size);

/// <summary>
/// Creates a memory structure representing the given number of kilobytes
/// </summary>
/// <param name="size">Number of bytes</param>
/// <returns>memory structure</returns>
memory AS_API operator"" _KB(size_t size);

/// <summary>
/// Creates a memory structure representing the given number of mebibytes
/// </summary>
/// <param name="size">Number of bytes</param>
/// <returns>memory structure</returns>
memory AS_API operator"" _MiB(size_t size);

/// <summary>
/// Creates a memory structure representing the given number of megabytes
/// </summary>
/// <param name="size">Number of bytes</param>
/// <returns>memory structure</returns>
memory AS_API operator"" _MB(size_t size);

/// <summary>
/// Creates a memory structure representing the given number of gibibytes
/// </summary>
/// <param name="size">Number of bytes</param>
/// <returns>memory structure</returns>
memory AS_API operator"" _GiB(size_t size);

/// <summary>
/// Creates a memory structure representing the given number of mebibytes
/// </summary>
/// <param name="size">Number of bytes</param>
/// <returns>memory structure</returns>
memory AS_API operator"" _GB(size_t size);

}  // namespace literals

#pragma endregion

using function_timing = resource<timespan_t>;

namespace detail {
struct AS_API FunctionTimingHelper {
  explicit FunctionTimingHelper(const char* name);
  ~FunctionTimingHelper();

 private:
  const char* _name;
  timestamp_t _start_time;
};

}  // namespace detail

#pragma endregion

#pragma region measure

template <typename T>
void add_measurement(std::string_view name, T measurement = T{}) {}

template <typename T>
std::vector<measurement<T>> get_measurements(std::string_view name,
                                             timestamp_t begin = timestamp_t{},
                                             timestamp_t end = now()) {
  return {};
}

template <typename T>
std::vector<measurement<T>> get_measurements_for_thread(
    std::string_view name, thread_id_t thread_id,
    timestamp_t begin = timestamp_t{}, timestamp_t end = now()) {
  return {};
}

template <typename T>
std::unordered_map<thread_id_t, std::vector<measurement<T>>>
get_measurements_for_all_threads(std::string_view name,
                                 timestamp_t begin = timestamp_t{},
                                 timestamp_t end = now()) {
  return {};
}

#pragma endregion

#pragma region properties

template <typename T>
void set_cache_size(std::string_view name, size_t cache_size) {}

template <typename T>
void measure_for_each_thread(std::string_view name) {}

/// <summary>
/// Infinite cache size for measurements
/// </summary>
constexpr size_t cache_size_infinite = std::numeric_limits<size_t>::max();

#pragma endregion

#pragma region helper_macros

#define MEASURE_FUNCTION_CALL \
  as::add_measurement<as::function_call>(__FUNCTION__)

#define MEASURE_FUNCTION_TIMING             \
  volatile as::detail::FunctionTimingHelper \
      __measure_function_timing_##__LINE##{ \
    __FUNCTION__                            \
  }

#pragma endregion

}  // namespace as
