#include "pch.h"

#include "measuring/measurement.h"

TEST(measurement, base) {
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
            as::add_measurement<function_timing>("func", as::now() - start);
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

    /*
    The types 'function_call' and 'function_timing' are typedefs for the types
    counter and resource<time>

    TODO Could everything be resource<T>? Like, resource<count>? That sounds
    constructed...
    TODO Do we need tagged resources, like resource<T, user_tag>? I think the
    names of add_measurement should be sufficient
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

    as::add_measurement<as::resource<as::memory>>("name", 1024_KiB);
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
    as::measure_for_each_thread<as::resource<as::memory>>("name");
    // etc.
  }

  // 6) Get some data back
  {
    // Access all measurements of type (e.g. function_call, resource<memory>
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
