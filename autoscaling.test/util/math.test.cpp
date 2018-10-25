#include "pch.h"

#include "util/math.h"

#include <stdint.h>

using namespace as::math;

TEST(inc_wrap, signed_no_wrap) {
  int val = 0;
  inc_wrap(val, 42);
  EXPECT_EQ(val, 1);
}

TEST(inc_wrap, signed_wrap) {
  int val = 41;
  inc_wrap(val, 42);
  EXPECT_EQ(val, 0);
}

TEST(inc_wrap, unsigned_no_wrap) {
  uint32_t val = 0;
  inc_wrap(val, 42u);
  EXPECT_EQ(val, 1u);
}

TEST(inc_wrap, unsigned_wrap) {
  uint32_t val = 41;
  inc_wrap(val, 42u);
  EXPECT_EQ(val, 0u);
}

TEST(inc_wrap, float_no_wrap) {
  float val = 0;
  inc_wrap(val, 42.f);
  EXPECT_EQ(val, 1.f);
}

TEST(inc_wrap, float_wrap) {
  float val = 41.f;
  inc_wrap(val, 42.f);
  EXPECT_EQ(val, 0.f);
}

TEST(inc_wrap, float_wrap_fractional) {
  float val = 41.5f;
  inc_wrap(val, 42.f);
  EXPECT_EQ(val, 0.f);
}

TEST(dec_wrap, signed_no_wrap) {
  int val = 5;
  dec_wrap(val, 42);
  EXPECT_EQ(val, 4);
}

TEST(dec_wrap, signed_wrap) {
  int val = 0;
  dec_wrap(val, 42);
  EXPECT_EQ(val, 41);
}

TEST(dec_wrap, unsigned_no_wrap) {
  uint32_t val = 5;
  dec_wrap(val, 42u);
  EXPECT_EQ(val, 4u);
}

TEST(dec_wrap, unsigned_wrap) {
  uint32_t val = 0;
  dec_wrap(val, 42u);
  EXPECT_EQ(val, 41u);
}

TEST(dec_wrap, float_no_wrap) {
  float val = 5.f;
  dec_wrap(val, 42.f);
  EXPECT_EQ(val, 4.f);
}

TEST(dec_wrap, float_wrap) {
  float val = 0.f;
  dec_wrap(val, 42.f);
  EXPECT_EQ(val, 41.f);
}
