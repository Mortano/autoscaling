#include "pch.h"

#include "measuring/measurement.h"

void api_description() {
  // Some use cases
  // Assumption: All measurements are TIMESTAMPED! So everything is realted to
  // time, no samples or anything

  // 1) Tracking function calls
  {
    // Measure that a function 'name' was called at the current time
    as::add_measurement<as::function_call>("name");
    MEASURE_FUNCTION_CALL;

    // Measure the timespan spent in a function by measuring the timepoints
    // of function enter and exit
    as::add_measurement<as::function_timing>("name",
                                             std::chrono::nanoseconds{12345});
    MEASURE_FUNCTION_TIMING;

    /*
    Use case:
    void some_func() {
                    auto t_start = as::now();
                    work_1();
                    work_2();
                    as::add_measurement<function_timing>("func", as::now() -
    start);
    }

    Because this is inconvenient, we can use a macro

    void some_func() {
                    MEASURE_FUNCTION_TIMING;
                    work_1();
                    work_2();
    }

    This expands to a local variable declaration with a sentinel type that adds
    the measurement upon it's destruction when the function is exited

    */
  }

  // 2) Tracking recurring events
  {
    as::add_measurement<as::periodic_event>("name");

    //?
  }

  // 3) Tracking other resources (like memory)
  {
    using namespace as::literals;

    as::add_measurement<as::memory>("name", 1024_KiB);
  }

  // 4) Set cache sizes
  {
    // Periodic events but only the latest N are tracked
    as::set_cache_size<as::periodic_event>("name", 128);
    // Go back to tracking all events (of course the ones older than the cache
    // are gone)
    as::set_cache_size<as::periodic_event>("name", as::cache_size_infinite);
  }

  // 5) Make thread local
  {
    // function calls of "name" are now tracked uniquely for each thread
    // this can't be undone after it was set!
    as::measure_for_each_thread<as::function_call>("name");

    // should work for multiple types of measurements
    as::measure_for_each_thread<as::memory>("name");
    // etc.
  }

  // 6) Get some data back
  {
    // Access all measurements of type (e.g. function_call, memory
    // etc.)
    using type = as::function_timing;
    auto& measurements = as::get_measurements<type>("name");
    // this would be std::vector<measurement<type>>

    // Get all measurements that occurred >= start_time
    auto start_time = as::now() - std::chrono::seconds{10};
    auto& measurements_after = as::get_measurements<type>("name", start_time);
    // Get all measurements that occured >= start_time and <= end_time
    auto end_time = as::now() - std::chrono::seconds{5};
    auto& measurements_in_interval =
        as::get_measurements<type>("name", start_time, end_time);

    // Of course, if the measurement is cached, only the cached data is returned

    // If the data is captured for each thread, we would have to call different
    // functions:

    auto thread_id = std::this_thread::get_id();
    auto& measurements_for_thread =
        as::get_measurements_for_thread<type>("name", thread_id);
    // vector<measurement<type>>

    auto& measurements_all_threads =
        as::get_measurements_for_all_threads<type>("name");
    // unordered_map<thread_id, vector<measurement<type>>
  }

  // 7) Analyze data
  {
    // TODO
  }
}

TEST(measurement, get_measurements_primitive_type_empty) {
  using type = int;
  std::string name{"test"};
  auto measurements = as::get_measurements<type>(name);

  ASSERT_EQ(measurements.size(), 0ull);
}

TEST(measurement, get_measurements_complex_type_empty) {
  using type = std::string;
  std::string name{"test"};
  auto measurements = as::get_measurements<type>(name);

  ASSERT_EQ(measurements.size(), 0ull);
}

TEST(measurement, add_measurement_primitive_type) {
  using type = int;
  std::string name{"test"};

  auto timestmap_before = as::now();

  as::add_measurement<type>(name, 42);

  auto timestamp_after = as::now();

  auto measurements = as::get_measurements<type>(name);
  ASSERT_EQ(measurements.size(), 1ull);
  ASSERT_EQ(measurements[0].data, 42);
  ASSERT_TRUE(timestmap_before <= measurements[0].timestamp);
  ASSERT_TRUE(timestamp_after >= measurements[0].timestamp);

  as::clear_measurements<type>();
}

TEST(measurement, add_measurement_complex_type) {
  using type = std::string;
  std::string name{"test"};

  auto timestmap_before = as::now();

  std::string measurement_data{"richard_parker"};
  as::add_measurement<type>(name, measurement_data);

  auto timestamp_after = as::now();

  auto measurements = as::get_measurements<type>(name);
  ASSERT_EQ(measurements.size(), 1ull);
  ASSERT_EQ(measurements[0].data, measurement_data);
  ASSERT_TRUE(timestmap_before <= measurements[0].timestamp);
  ASSERT_TRUE(timestamp_after >= measurements[0].timestamp);

  as::clear_measurements<type>();
}

TEST(measurement, add_multiple_measurements_primitive_type) {
  using type = int;
  std::string name{"test"};

  std::vector<as::timestamp_t> timestamps_before, timestamps_after;
  for (type data = 0; data < 10; ++data) {
    timestamps_before.push_back(as::now());
    as::add_measurement<type>(name, data);
    timestamps_after.push_back(as::now());
  }

  auto measurements = as::get_measurements<type>(name);
  ASSERT_EQ(measurements.size(), 10ull);

  for (type idx = 0; idx < 10; ++idx) {
    ASSERT_EQ(measurements[idx].data, idx);
    ASSERT_TRUE(timestamps_before[idx] <= measurements[idx].timestamp);
    ASSERT_TRUE(timestamps_after[idx] >= measurements[idx].timestamp);
  }

  as::clear_measurements<type>();
}

TEST(measurement, add_multiple_measurements_complex_type) {
  using type = std::string;
  std::string name{"test"};

  std::vector<std::string> data{"first", "second", "third", "fourth"};

  std::vector<as::timestamp_t> timestamps_before, timestamps_after;
  for (auto& item : data) {
    timestamps_before.push_back(as::now());
    as::add_measurement<type>(name, item);
    timestamps_after.push_back(as::now());
  }

  auto measurements = as::get_measurements<type>(name);
  ASSERT_EQ(measurements.size(), data.size());

  for (size_t idx = 0; idx < 4; ++idx) {
    ASSERT_EQ(measurements[idx].data, data[idx]);
    ASSERT_TRUE(timestamps_before[idx] <= measurements[idx].timestamp);
    ASSERT_TRUE(timestamps_after[idx] >= measurements[idx].timestamp);
  }

  as::clear_measurements<type>();
}

TEST(measurement, add_multiple_measurements_different_types) {
  std::string name{"test"};

  as::add_measurement<int>(name, 42);
  as::add_measurement<int>(name, 43);
  as::add_measurement<std::string>(name, "the question");
  as::add_measurement<std::string>(name, "the answer");

  auto int_measurements = as::get_measurements<int>(name);
  auto string_measurements = as::get_measurements<std::string>(name);

  ASSERT_EQ(int_measurements.size(), 2ull);
  ASSERT_EQ(string_measurements.size(), 2ull);

  ASSERT_EQ(int_measurements[0].data, 42);
  ASSERT_EQ(int_measurements[1].data, 43);
  ASSERT_EQ(string_measurements[0].data, "the question");
  ASSERT_EQ(string_measurements[1].data, "the answer");

  as::clear_measurements<int>();
  as::clear_measurements<std::string>();
}

TEST(measurement, add_multiple_measurements_different_names) {
  std::string name_1{"test_1"};
  std::string name_2{"test_2"};

  as::add_measurement<int>(name_1, 42);
  as::add_measurement<int>(name_1, 43);

  as::add_measurement<int>(name_2, 84);
  as::add_measurement<int>(name_2, 85);

  auto measurements_1 = as::get_measurements<int>(name_1);
  auto measurements_2 = as::get_measurements<int>(name_2);

  ASSERT_EQ(measurements_1.size(), 2ull);
  ASSERT_EQ(measurements_2.size(), 2ull);

  ASSERT_EQ(measurements_1[0].data, 42);
  ASSERT_EQ(measurements_1[1].data, 43);
  ASSERT_EQ(measurements_2[0].data, 84);
  ASSERT_EQ(measurements_2[1].data, 85);

  as::clear_measurements<int>();
}

TEST(measurement, is_thread_local_false) {
  ASSERT_FALSE(as::is_measured_for_each_thread<int>("test"));
}

TEST(measurement, is_thread_local_true) {
  as::measure_for_each_thread<int>("_test_");
  ASSERT_TRUE(as::is_measured_for_each_thread<int>("_test_"));
}

TEST(measurement, thread_local_primitive_type) {
  as::measure_for_each_thread<int>("thread_local");

  std::vector<std::thread> threads;
  for (auto idx = 0; idx < 8; ++idx) {
    threads.emplace_back([idx]() {
      auto offset = idx * 2;

      auto ts_1 = as::now();
      as::add_measurement<int>("thread_local", offset);
      auto ts_2 = as::now();
      as::add_measurement<int>("thread_local", offset + 1);
      auto ts_3 = as::now();

      auto this_thread_id = std::this_thread::get_id();
      auto measurements_this_thread =
          as::get_measurements_for_thread<int>("thread_local", this_thread_id);

      ASSERT_EQ(measurements_this_thread.size(), 2ull);
      ASSERT_EQ(measurements_this_thread[0].data, offset);
      ASSERT_EQ(measurements_this_thread[1].data, offset + 1);

      ASSERT_TRUE(ts_1 <= measurements_this_thread[0].timestamp);
      ASSERT_TRUE(ts_2 >= measurements_this_thread[0].timestamp);
      ASSERT_TRUE(ts_2 <= measurements_this_thread[1].timestamp);
      ASSERT_TRUE(ts_3 >= measurements_this_thread[1].timestamp);
    });
  }

  for (auto& t : threads) t.join();

  // The measurements should be valid even after the threads died
  auto measurements_all_threads =
      as::get_measurements_for_all_threads<int>("thread_local");

  ASSERT_EQ(measurements_all_threads.size(), 8ull);
  for (auto& kv : measurements_all_threads) {
    ASSERT_EQ(kv.second.size(), 2ull);
  }

  as::clear_measurements<int>();
}

TEST(measurement, thread_local_complex_type) {}
