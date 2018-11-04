#include "measuring/measurement.h"

#pragma region time
as::timestamp_t as::now() { return std::chrono::high_resolution_clock::now(); }
#pragma endregion

#pragma region memory

as::memory::memory() : _bytes(0) {}
as::memory::memory(size_t bytes) : _bytes(bytes) {}

as::memory::operator size_t() const { return _bytes; }
size_t as::memory::get_size() const { return _bytes; }

as::memory as::operator+(const memory& l, const memory& r) {
  return memory(l.get_size() + r.get_size());
}

as::memory as::operator-(const memory& l, const memory& r) {
  // TODO should this be checked and maybe throw or clamp to zero?
  return memory(l.get_size() - r.get_size());
}

as::memory as::operator*(const memory& mem, size_t s) {
  return memory(mem.get_size() * s);
}

as::memory as::literals::operator"" _B(size_t size) { return memory{size}; }

as::memory as::literals::operator"" _KiB(size_t size) {
  return memory{size * (1 << 10)};
}

as::memory as::literals::operator"" _KB(size_t size) {
  return memory{size * static_cast<size_t>(1e3)};
}

as::memory as::literals::operator"" _MiB(size_t size) {
  return memory{size * (1 << 20)};
}

as::memory as::literals::operator"" _MB(size_t size) {
  return memory{size * static_cast<size_t>(1e6)};
}

as::memory as::literals::operator"" _GiB(size_t size) {
  return memory{size * (1 << 30)};
}

as::memory as::literals::operator"" _GB(size_t size) {
  return memory{size * static_cast<size_t>(1e9)};
}

#pragma endregion

#pragma region type_id

as::type_id_t as::detail::TypeIDBase::next() {
  static type_id_t s_next_id = 0;
  return ++s_next_id;
}

#pragma endregion

#pragma region function_timing_helper

as::detail::FunctionTimingHelper::FunctionTimingHelper(const char* name)
    : _name(name), _start_time(as::now()) {}

as::detail::FunctionTimingHelper::~FunctionTimingHelper() {
  add_measurement<function_timing>(_name, now() - _start_time);
}

#pragma endregion
