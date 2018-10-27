#include "pch.h"

#include "util/cache.h"

using namespace as;

TEST(cache, construct) {
  const size_t capacity = 4;

  as::cache<int> cache{capacity};

  EXPECT_EQ(cache.capacity(), capacity);
  EXPECT_EQ(cache.size(), 0ull);
  EXPECT_FALSE(cache.is_full());

  EXPECT_THROW(cache.at(0), std::out_of_range);
}

TEST(cache, construct_no_capacity) {
  // A cache with no capacity makes little sense, but it is still a possibility
  const size_t capacity = 0;
  as::cache<int> cache{capacity};

  EXPECT_TRUE(cache.is_full());
}

TEST(cache, insert_const_reference) {
  const size_t capacity = 4;
  as::cache<int> cache{capacity};

  const int value = 42;
  cache.insert(value);

  EXPECT_EQ(cache.size(), 1ull);
  EXPECT_EQ(cache.at(0), value);
  EXPECT_EQ(cache[0], value);
  EXPECT_EQ(*cache.begin(), value);
}

TEST(cache, insert_rvalue) {
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

TEST(cache, clear) {
  const size_t capacity = 4;
  as::cache<int> cache{capacity};

  cache.insert(1);
  cache.insert(2);

  cache.clear();

  EXPECT_EQ(cache.size(), 0ull);
  EXPECT_EQ(cache.capacity(), capacity);
}

TEST(cache, is_full) {
  const size_t capacity = 4;
  as::cache<size_t> cache{capacity};

  for (size_t idx = 0; idx < capacity; ++idx) {
    cache.insert(idx);
  }

  EXPECT_TRUE(cache.is_full());
}

TEST(cache, youngest) {
  const size_t capacity = 4;

  as::cache<int> cache{capacity};

  cache.insert(1);

  EXPECT_EQ(cache.youngest(), 1);

  cache.insert(2);

  EXPECT_EQ(cache.youngest(), 2);

  cache.insert(3);

  EXPECT_EQ(cache.youngest(), 3);
}

TEST(cache, oldest) {
  const size_t capacity = 4;

  as::cache<int> cache{capacity};

  cache.insert(1);

  EXPECT_EQ(cache.oldest(), 1);

  cache.insert(2);

  EXPECT_EQ(cache.oldest(), 1);

  cache.insert(3);

  EXPECT_EQ(cache.oldest(), 1);
}

TEST(cache, access_elements_non_full) {
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

TEST(cache, access_elements_full) {
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

TEST(cache, empty_iterators) {
  const size_t capacity = 4;

  as::cache<int> cache{capacity};

  EXPECT_TRUE(cache.begin() == cache.end());
}

TEST(cache, for_each_non_full) {
  const size_t capacity = 4;

  as::cache<int> cache{capacity};

  cache.insert(1);
  cache.insert(2);
  cache.insert(3);

  std::vector<int> for_each_result;
  for (auto elem : cache) for_each_result.push_back(elem);

  std::vector<int> expected_elements{3, 2, 1};

  EXPECT_EQ(for_each_result, expected_elements);
}

TEST(cache, for_each_full) {
  const size_t capacity = 4;

  as::cache<int> cache{capacity};

  cache.insert(1);
  cache.insert(2);
  cache.insert(3);
  cache.insert(4);
  cache.insert(5);

  std::vector<int> for_each_result;
  for (auto elem : cache) for_each_result.push_back(elem);

  std::vector<int> expected_elements{5, 4, 3, 2};

  EXPECT_EQ(for_each_result, expected_elements);
}

TEST(cache, mutate_through_iterator) {
  const size_t capacity = 4;

  as::cache<int> cache{capacity};

  cache.insert(1);

  *cache.begin() = 42;

  EXPECT_EQ(cache[0], 42);
}
