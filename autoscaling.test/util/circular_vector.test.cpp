#include "pch.h"

#include "util/circular_vector.h"

using namespace as;

TEST(circular_vector, construct) {
  const size_t capacity = 4;

  as::cache<int> cache{capacity};

  EXPECT_EQ(cache.capacity(), capacity);
  EXPECT_EQ(cache.size(), 0ull);
  EXPECT_FALSE(cache.is_full());

  EXPECT_THROW(cache.at(0), std::out_of_range);
}

TEST(circular_vector, construct_no_capacity) {
  // A cache with no capacity makes little sense, but it is still a possibility
  const size_t capacity = 0;
  as::cache<int> cache{capacity};

  EXPECT_TRUE(cache.is_full());
}

TEST(circular_vector, insert_const_reference) {
  const size_t capacity = 4;
  as::cache<int> cache{capacity};

  const int value = 42;
  cache.insert(value);

  EXPECT_EQ(cache.size(), 1ull);
  EXPECT_EQ(cache.at(0), value);
  EXPECT_EQ(cache[0], value);
  EXPECT_EQ(*cache.begin(), value);
}

TEST(circular_vector, insert_rvalue) {
  struct S {
    explicit S(int val) : val(val) {}

    S(const S&) {
      throw std::exception("Copy constructor must not be called!");
    }
    S(S&&) = default;

    S& operator=(const S&) {
      throw std::exception("Copy assignment operator must not be called!");
      return *this;
    }

    S& operator=(S&&) = default;

    int val;
  };

  const size_t capacity = 4;
  as::cache<S> cache{capacity};

  S obj{42};

  cache.insert(std::move(obj));

  EXPECT_EQ(cache.size(), 1ull);
  EXPECT_EQ(cache.at(0).val, 42);
  EXPECT_EQ(cache[0].val, 42);
  EXPECT_EQ(cache.begin()->val, 42);
}

TEST(circular_vector, clear) {
  const size_t capacity = 4;
  as::cache<int> cache{capacity};

  cache.insert(1);
  cache.insert(2);

  cache.clear();

  EXPECT_EQ(cache.size(), 0ull);
  EXPECT_EQ(cache.capacity(), capacity);
}

TEST(circular_vector, is_full) {
  const size_t capacity = 4;
  as::cache<size_t> cache{capacity};

  for (size_t idx = 0; idx < capacity; ++idx) {
    cache.insert(idx);
  }

  EXPECT_TRUE(cache.is_full());
}

TEST(circular_vector, youngest) {
  const size_t capacity = 4;

  as::cache<int> cache{capacity};

  cache.insert(1);

  EXPECT_EQ(cache.youngest(), 1);

  cache.insert(2);

  EXPECT_EQ(cache.youngest(), 2);

  cache.insert(3);

  EXPECT_EQ(cache.youngest(), 3);
}

TEST(circular_vector, oldest) {
  const size_t capacity = 4;

  as::cache<int> cache{capacity};

  cache.insert(1);

  EXPECT_EQ(cache.oldest(), 1);

  cache.insert(2);

  EXPECT_EQ(cache.oldest(), 1);

  cache.insert(3);

  EXPECT_EQ(cache.oldest(), 1);
}

TEST(circular_vector, access_elements_non_full) {
  const size_t capacity = 4;

  as::cache<int> cache{capacity};

  cache.insert(1);
  cache.insert(2);
  cache.insert(3);

  EXPECT_EQ(cache.at(0), 3);
  EXPECT_EQ(cache[0], 3);

  EXPECT_EQ(cache.at(1), 2);
  EXPECT_EQ(cache[1], 2);

  EXPECT_EQ(cache.at(2), 1);
  EXPECT_EQ(cache[2], 1);

  EXPECT_EQ(cache.youngest(), 3);
  EXPECT_EQ(cache.oldest(), 1);
}

TEST(circular_vector, access_elements_full) {
  const size_t capacity = 4;

  as::cache<int> cache{capacity};

  cache.insert(1);
  cache.insert(2);
  cache.insert(3);
  cache.insert(4);
  cache.insert(5);
  cache.insert(6);

  // At this point, the cache should contain [6;5;4;3]

  EXPECT_EQ(cache.at(0), 6);
  EXPECT_EQ(cache[0], 6);

  EXPECT_EQ(cache.at(1), 5);
  EXPECT_EQ(cache[1], 5);

  EXPECT_EQ(cache.at(2), 4);
  EXPECT_EQ(cache[2], 4);

  EXPECT_EQ(cache.at(3), 3);
  EXPECT_EQ(cache[3], 3);

  EXPECT_EQ(cache.youngest(), 6);
  EXPECT_EQ(cache.oldest(), 3);
}
