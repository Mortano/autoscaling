#pragma once

#include "api.h"
#include "util/cache.h"
#include "util/math.h"

#include <stdint.h>
#include <algorithm>
#include <any>
#include <atomic>
#include <chrono>
#include <mutex>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <variant>

namespace as {

#pragma region forward_declarations
template <typename T>
bool is_measured_for_each_thread(std::string_view name);
#pragma endregion

#pragma region time

using timestamp_t = std::chrono::high_resolution_clock::time_point;
using timespan_t = std::chrono::nanoseconds;

/// <summary>
/// Returns a timestamp for the current point in time upon calling this function
/// </summary>
/// <returns>Timestamp for the current point in time</returns>
timestamp_t now();

#pragma endregion

#pragma region thread

using thread_id_t = std::thread::id;

static const thread_id_t thread_id_all_threads = {};

#pragma endregion

#pragma region types

template <typename T>
struct measurement {
  timestamp_t timestamp;
  T data;

  measurement(timestamp_t timestamp, T data)
      : timestamp(timestamp), data(std::move(data)) {}
};

#pragma region resources

struct AS_API function_call {};
struct AS_API periodic_event {};

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

using function_timing = timespan_t;

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

#pragma region type_id

using type_id_t = size_t;

namespace detail {

template <typename T>
struct TypeIDHelper;

class TypeIDBase {
  template <typename T>
  friend struct TypeIDHelper;

  static type_id_t next();
};

template <typename T>
struct TypeIDHelper : public TypeIDBase {
  static type_id_t get_type_id() {
    static type_id_t s_type_id = TypeIDBase::next();
    return s_type_id;
  }
};
}  // namespace detail

template <typename T>
type_id_t get_type_id() {
  return detail::TypeIDHelper<T>::get_type_id();
}

#pragma endregion

#pragma endregion

#pragma region measure

namespace detail {

struct measurement_lookup {
  thread_id_t thread_id;
  std::string_view name;

  measurement_lookup(thread_id_t thread_id, std::string_view name)
      : thread_id(thread_id), name(std::move(name)) {}
};

}  // namespace detail

}  // namespace as

template <>
struct std::hash<as::detail::measurement_lookup> {
  size_t operator()(const as::detail::measurement_lookup& lookup) const
      noexcept {
    size_t hash = 0;
    as::math::hash_combine(hash, lookup.name);
    as::math::hash_combine(hash, lookup.thread_id);
    return hash;
  }
};

template <>
struct std::equal_to<as::detail::measurement_lookup> {
  constexpr bool operator()(const as::detail::measurement_lookup& l,
                            const as::detail::measurement_lookup& r) const {
    return l.name == r.name && l.thread_id == r.thread_id;
  }
};

namespace as {

namespace detail {

template <typename T>
struct measurement_storage {
  void add_measurement(measurement<T> measurement, std::string_view name,
                       thread_id_t thread_id = thread_id_all_threads) {
    measurement_lookup lookup{thread_id, name};

    std::lock_guard<std::mutex> guard{_measurements_lock};
    auto& container = _measurements[lookup];
    insert_measurement(std::move(measurement), container);
  }

  std::vector<measurement<T>> get_copy_of_measurements(
      std::string_view name, thread_id_t thread_id = thread_id_all_threads) {
    measurement_lookup lookup{thread_id, name};

    std::lock_guard<std::mutex> guard{_measurements_lock};
    auto& measurements = _measurements[lookup];
    return container_to_vector(measurements);
  }

  std::unordered_map<thread_id_t, std::vector<measurement<T>>>
  get_copy_of_measurements_for_all_threads(std::string_view name) {
    std::unordered_map<thread_id_t, std::vector<measurement<T>>> ret;

    std::lock_guard<std::mutex> guard{_measurements_lock};
    for (auto& kv : _measurements) {
      if (kv.first.name != name) continue;
      ret[kv.first.thread_id] = container_to_vector(kv.second);
    }
    return ret;
  }

  void clear() {
    std::lock_guard<std::mutex> guard{_measurements_lock};
    _measurements.clear();
  }

  void clear(std::string_view name) {
    std::lock_guard<std::mutex> guard{_measurements_lock};
    auto iter = std::remove_if(
        _measurements.begin(), _measurements.end(),
        [&name](const auto& kv) { return kv.first.name == name; });
    _measurements.erase(iter, _measurements.end());
  }

  bool is_measured_for_each_thread(std::string_view name) {
    std::lock_guard<std::mutex> guard{_measured_for_each_thread_lock};
    return _measured_for_each_thread[name];
  }

  void set_measured_for_each_thread(std::string_view name) {
    std::lock_guard<std::mutex> guard{_measured_for_each_thread_lock};
    _measured_for_each_thread[name] = true;
  }

 private:
  using measurement_container_t =
      std::variant<std::vector<measurement<T>>, as::cache<measurement<T>>>;

  static void insert_measurement(measurement<T>&& measurement,
                                 measurement_container_t& container) {
    std::visit(
        [m = std::move(measurement)](auto&& arg) mutable {
          using U = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<U, std::vector<as::measurement<T>>>) {
            arg.push_back(std::move(m));
          } else if constexpr (std::is_same_v<U,
                                              as::cache<as::measurement<T>>>) {
            arg.insert(std::move(m));
          } else {
            static_assert(false, "Non-exhaustive visitor!");
          }
        },
        container);
  }

  static std::vector<as::measurement<T>> container_to_vector(
      const measurement_container_t& container) {
    std::vector<as::measurement<T>> ret;
    std::visit(
        [&ret](auto&& arg) {
          using U = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<U, std::vector<as::measurement<T>>>) {
            ret = arg;
          } else if constexpr (std::is_same_v<U,
                                              as::cache<as::measurement<T>>>) {
            ret.reserve(arg.size());
            std::copy(arg.begin(), arg.end(), std::back_inserter(ret));
          } else {
            static_assert(false, "Non-exhaustive visitor!");
          }
        },
        container);
    return ret;
  }

  std::mutex _measurements_lock;
  std::unordered_map<measurement_lookup, measurement_container_t> _measurements;

  std::mutex _measured_for_each_thread_lock;
  std::unordered_map<std::string_view, bool> _measured_for_each_thread;
};

/// <summary>
///  Returns the measurement_storage for objects of type T
/// </summary>
template <typename T>
measurement_storage<T>& get_measurement_storage() {
  static measurement_storage<T> s_storage;
  return s_storage;
}

}  // namespace detail

template <typename T>
void add_measurement(std::string_view name, T measurement_value = T{}) {
  auto timestamp = now();

  auto& storage = detail::get_measurement_storage<T>();
  auto thread_id = (is_measured_for_each_thread<T>(name))
                       ? std::this_thread::get_id()
                       : thread_id_all_threads;
  storage.add_measurement(
      measurement<T>{timestamp, std::move(measurement_value)}, name, thread_id);
}

template <typename T>
std::vector<measurement<T>> get_measurements(std::string_view name,
                                             timestamp_t begin = timestamp_t{},
                                             timestamp_t end = now()) {
  if (is_measured_for_each_thread<T>(name))
    throw std::runtime_error{
        "Type is measured for each thread, call 'get_measurements_for_thread' "
        "instead!"};
  // TODO Filter
  return detail::get_measurement_storage<T>().get_copy_of_measurements(name);
}

template <typename T>
std::vector<measurement<T>> get_measurements_for_thread(
    std::string_view name, thread_id_t thread_id,
    timestamp_t begin = timestamp_t{}, timestamp_t end = now()) {
  if (!is_measured_for_each_thread<T>(name))
    throw std::runtime_error{
        "Type is not measured for each thread, call 'get_measurements' "
        "instead!"};
  return detail::get_measurement_storage<T>().get_copy_of_measurements(
      name, thread_id);
}

template <typename T>
std::unordered_map<thread_id_t, std::vector<measurement<T>>>
get_measurements_for_all_threads(std::string_view name,
                                 timestamp_t begin = timestamp_t{},
                                 timestamp_t end = now()) {
  if (!is_measured_for_each_thread<T>(name))
    throw std::runtime_error{
        "Type is not measured for each thread, call 'get_measurements' "
        "instead!"};
  return detail::get_measurement_storage<T>()
      .get_copy_of_measurements_for_all_threads(name);
}

template <typename T>
void clear_measurements() {
  detail::get_measurement_storage<T>().clear();
}

template <typename T>
void clear_measurements(std::string_view name) {
  detail::get_measurement_storage<T>().clear(name);
}

#pragma endregion

#pragma region properties

template <typename T>
void set_cache_size(std::string_view name, size_t cache_size) {}

template <typename T>
void measure_for_each_thread(std::string_view name) {
  detail::get_measurement_storage<T>().set_measured_for_each_thread(name);
}

template <typename T>
bool is_measured_for_each_thread(std::string_view name) {
  return detail::get_measurement_storage<T>().is_measured_for_each_thread(name);
}

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
