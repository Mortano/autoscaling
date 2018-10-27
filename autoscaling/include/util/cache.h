#pragma once

#include "math.h"

#include <array>
#include <vector>

namespace as {

/// <summary>
/// Cache of a fixed (dynamic) size using a FIFO caching scheme. Elements are
/// inserted in a circular fashion, if the cache is full the oldest element is
/// discarded
/// </summary>
template <typename T>
struct cache {
  /// <summary>
  /// Creates a cache with the given capacity
  /// </summary>
  /// <param name="capacity">Maximum number of elements that the cache will
  /// hold</param>
  explicit cache(size_t capacity) : _head_idx(0) { _storage.reserve(capacity); }

  /// <summary>
  /// Inserts an element into the cache
  /// </summary>
  /// <param name="element">Element to insert</param>
  void insert(const T& element) {
    if (is_full()) {
      auto& slot = next_element();
      slot = element;
    } else {
      _storage.push_back(element);
      math::inc_wrap(_head_idx, capacity());
    }
  }

  /// <summary>
  /// Move-inserts an element into the cache
  /// </summary>
  /// <param name="element">Element to insert</param>
  void insert(T&& element) {
    if (is_full()) {
      auto& slot = next_element();
      slot = std::move(element);
    } else {
      _storage.push_back(std::move(element));
      math::inc_wrap(_head_idx, capacity());
    }
  }

  /// <summary>
  /// Clears all elements in this cache
  /// </summary>
  void clear() {
    _storage.clear();
    _head_idx = 0;
  }

  /// <summary>
  /// Returns the number of elements currently in the cache
  /// </summary>
  /// <returns>Number of elements in the cache</returns>
  size_t size() const { return _storage.size(); }

  /// <summary>
  /// Returns the capacity of this cache
  /// </summary>
  /// <returns>Capacity of the cache</returns>
  size_t capacity() const { return _storage.capacity(); }

  /// <summary>
  /// Returns true if the cache is full, i.e. its size equals its capacity
  /// </summary>
  /// <returns>True if the cache is full</returns>
  bool is_full() const { return size() == capacity(); }

  /// <summary>
  /// Returns the element at the given index.
  /// TODO Define what index 0 means like and if we iterate forwards or
  /// backwards
  /// </summary>
  /// <param name="idx"></param>
  /// <returns></returns>
  T& at(size_t idx) {
    if (idx >= size()) throw std::out_of_range{"Index out of range"};
    return _storage[index_from_age(idx)];
  }

  T const& at(size_t idx) const {
    if (idx >= size()) throw std::out_of_range{"Index out of range"};
    return _storage[index_from_age(idx)];
  }

  T& operator[](size_t idx) { return _storage[index_from_age(idx)]; }

  T const& operator[](size_t idx) const {
    return _storage[index_from_age(idx)];
  }

  /// <summary>
  /// Returns a reference to the oldest element in this cache
  /// </summary>
  /// <returns>Reference to the oldest element in this cache</returns>
  T& oldest() { return _storage[oldest_index()]; }

  /// <summary>
  /// Returns a const reference to the oldest element in this cache
  /// </summary>
  /// <returns>Const reference to the oldest element in this cache</returns>
  T const& oldest() const { return _storage[oldest_index()]; }

  /// <summary>
  /// Returns a reference to the youngest element in this cache, i.e. the
  /// element most recently added
  /// </summary>
  /// <returns>Reference to the most recently added element</returns>
  T& youngest() { return _storage[youngest_index()]; }

  /// <summary>
  /// Returns a const reference the youngest element in this cache, i.e. the
  /// element most recently added
  /// </summary>
  /// <returns>Const reference to the most recently added element</returns>
  T const& youngest() const { return _storage[youngest_index()]; }

  /// <summary>
  /// Mutable iterator that iterates the contents of the cache from youngest to
  /// oldest
  /// </summary>
  struct iterator {
    iterator() : _container(nullptr), _step(0), _is_end(true) {}
    iterator(cache* container, size_t offset, size_t step = 0)
        : _container(container), _offset(offset), _step(step) {}

    iterator& operator++() {
      ++_step;
      return *this;
    }
    iterator operator++(int) {
      auto ret = *this;
      ++_step;
      return ret;
    }

    iterator& operator--() {
      --_step;
      return *this;
    }
    iterator operator--(int) {
      auto ret = *this;
      --_step;
      return ret;
    }

    T& operator*() const { return _container->_storage[get_index()]; }

    T* operator->() const { return &_container->_storage[get_index()]; }

    bool operator!=(iterator const& other) const {
      return _container != other._container || _offset != other._offset ||
             _step != other._step;
    }

    bool operator==(iterator const& other) const { return !operator!=(other); }

   private:
    bool is_end() const {
      return !_container || _step == _container->capacity();
    }

    size_t get_index() const {
      const auto capacity = _container->capacity();
      return (capacity + _offset - _step) % _container->capacity();
    }

    cache* _container;
    const size_t _offset;
    size_t _step;
  };

  /// <summary>
  /// Const iterator that iterates the contents of the cache from youngest to
  /// oldest
  /// </summary>
  struct const_iterator {
    const_iterator() : _container(nullptr), _step(0), _is_end(true) {}
    const_iterator(cache const* container, size_t offset, size_t step)
        : _container(container), _offset(offset), _step(step) {}

    const_iterator& operator++() {
      ++_step;
      return *this;
    }
    const_iterator operator++(int) {
      auto ret = *this;
      ++_step;
      return ret;
    }

    const_iterator& operator--() {
      --_step;
      return *this;
    }
    const_iterator operator--(int) {
      auto ret = *this;
      --_step;
      return ret;
    }

    T const& operator*() const { return _container->_storage[get_index()]; }

    T const* operator->() const { return &_container->_storage[get_index()]; }

    bool operator!=(const_iterator const& other) const {
      return _container != other._container || _offset != other._offset ||
             _step != other._step;
    }

    bool operator==(const_iterator const& other) const {
      return !operator==(other);
    }

   private:
    bool is_end() const {
      return !_container || _step == _container->capacity();
    }

    size_t get_index() const {
      const auto capacity = _container->capacity();
      return (capacity + _offset - _step) % _container->capacity();
    }

    cache const* _container;
    const size_t _offset;
    size_t _step;
  };

  iterator begin() { return iterator{this, youngest_index()}; }

  iterator end() { return iterator{this, youngest_index(), size()}; }

  const_iterator begin() const {
    return const_iterator{this, youngest_index()};
  }

  const_iterator end() const {
    return const_iterator{this, youngest_index(), size()};
  }

 private:
  friend struct iterator;
  friend struct const_iterator;

  T& next_element() {
    auto& ret = _storage[_head_idx];
    math::inc_wrap(_head_idx, capacity());
    return ret;
  }

  size_t youngest_index() const {
    auto youngest_idx = _head_idx;
    math::dec_wrap(
        youngest_idx,
        capacity());  // wrap by _capacity_ because if the cache is not
                      // full, we will never underflow expect when it is
                      // empty, in which case calling youngest() is UB
    return youngest_idx;
  }

  size_t oldest_index() const {
    auto oldest_idx = youngest_index();
    math::inc_wrap(oldest_idx, size());
    return oldest_idx;
  }

  size_t index_from_age(size_t age) const {
    return (capacity() + youngest_index() - age) % capacity();
  }

  std::vector<T> _storage;
  size_t _head_idx;
};

template <typename T, size_t N>
struct circular_array {};

}  // namespace as
