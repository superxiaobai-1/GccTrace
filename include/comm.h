#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace GccTrace {

template <class Key, class Value> 
using map_t = std::unordered_map<Key, Value>;

template <class Value> 
using set_t = std::unordered_set<Value>;

using clock_t = std::chrono::high_resolution_clock;
using time_point_t = std::chrono::time_point<clock_t>;
extern time_point_t COMPILATION_START;
using TimeStamp = int64_t;

struct TimeSpan {
  int64_t start;
  int64_t end;
};

inline TimeStamp ns_from_start() {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(clock_t::now() -
                                                              COMPILATION_START)
      .count();
}

enum EventCategory {
  TU,
  PREPROCESS,
  FUNCTION,
  STRUCT,
  NAMESPACE,
  GIMPLE_PASS,
  RTL_PASS,
  SIMPLE_IPA_PASS,
  IPA_PASS,
  UNKNOWN
};

struct TraceEvent {
  const char *name;
  EventCategory category;
  TimeSpan ts;
  std::optional<map_t<std::string, std::string>> args;
};

struct FinishedFunction {
  void *decl;
  const char *name;
  const char *file_name;
  const char *scope_name;
  EventCategory scope_type;
};
}