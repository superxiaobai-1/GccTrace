cmake_minimum_required(VERSION 3.12)
project(gperf)

option(GPERF_BUILD_TEST "Build the compiler plugin test" ON)

if(NOT GPERF_GCC_PLUGIN_DIR)
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=plugin
        OUTPUT_VARIABLE GPERF_GCC_PLUGIN_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()

if(NOT EXISTS ${GPERF_GCC_PLUGIN_DIR}/include/gcc-plugin.h)
    message(FATAL_ERROR "gcc-plugin.h not found under ${GPERF_GCC_PLUGIN_DIR}/include")
endif()

message("GCC plugin headers found in " ${GPERF_GCC_PLUGIN_DIR})

include_directories(${CMAKE_SOURCE_DIR}/include)

set(SOURCES
    src/plugin.cc 
    src/tracking.cc
    src/perf_output.cc
)

add_library(gperf SHARED ${SOURCES})
target_include_directories(gperf PRIVATE ${GPERF_GCC_PLUGIN_DIR}/include)



set_target_properties(gperf PROPERTIES CXX_STANDARD 20)
set_target_properties(gperf PROPERTIES COMPILE_FLAGS "-fno-rtti -g -Wall")
set_target_properties(gperf PROPERTIES PREFIX "" OUTPUT_NAME "gperf")

set(GPERF_PLUGIN_PATH ${CMAKE_BINARY_DIR}/gperf.so)
install(TARGETS gperf DESTINATION ${GPERF_GCC_PLUGIN_DIR})

if(GPERF_BUILD_TEST)
    add_subdirectory(test)
    add_dependencies(test gperf)
endif()