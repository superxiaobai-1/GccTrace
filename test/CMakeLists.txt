add_executable(test test.cc)
set_target_properties(
    test
    PROPERTIES
        COMPILE_FLAGS
        "-fplugin=${CMAKE_BINARY_DIR}/gperf.so -fplugin-arg-gperf-trace=${CMAKE_CURRENT_BINARY_DIR}/trace.json"
)

add_custom_target(
    remove_test ALL
    COMMAND rm ${CMAKE_CURRENT_BINARY_DIR}/test
    COMMAND rm ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/test.dir/test.cc.o
    COMMAND echo "Removed!"
    DEPENDS test
)
