# Copyright (c) 2018-2025 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt
# or copy at http://www.boost.org/LICENSE_1_0.txt)

if(NOT TARGET tests)
    add_custom_target(tests ${EXCLUDE_TESTS_FROM_ALL})
    set_property(TARGET tests PROPERTY FOLDER _deps)
endif()

file(GLOB test_cpp_files "test_*.cpp")

foreach(test_cpp ${test_cpp_files})
    cmake_path(REMOVE_EXTENSION test_cpp LAST_ONLY OUTPUT_VARIABLE test)
    string(REGEX REPLACE ".*/" "" test ${test})
    add_executable(${test} ${test_cpp})
    target_link_libraries(${test} Boost::openmethod Boost::unit_test_framework)
    add_test(NAME ${test} COMMAND ${test})
    add_dependencies(tests ${test})
endforeach()
