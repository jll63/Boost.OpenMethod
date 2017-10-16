# Copyright (c) 2018-2025 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt
# or copy at http://www.boost.org/LICENSE_1_0.txt)

file(GLOB test_cpp_files "test_*.cpp")
foreach(test_cpp ${test_cpp_files})
    cmake_path(REMOVE_EXTENSION test_cpp LAST_ONLY OUTPUT_VARIABLE test)
    string(REGEX REPLACE ".*/" "" test ${test})
    add_executable(${test} ${test_cpp})
    target_link_libraries(${test} Boost::openmethod)
    add_test(NAME ${test} COMMAND ${test})
endforeach()
