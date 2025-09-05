#
# Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
# Copyright (c) 2021 DMitry Arkhipov (grisumbras@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/boostorg/url
#

message(STATUS "Building tests")

# Custom target used by the boost super-project
if(NOT TARGET tests)
    add_custom_target(tests)
    set_property(TARGET tests PROPERTY FOLDER Dependencies)
endif()

# Replicate error flags from Jamfile
if (BOOST_OPENMETHOD_WARNINGS_AS_ERRORS)
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 7)
            set(BOOST_OPENMETHOD_TEST_FLAGS "-Wall -Werror -Wno-unused-but-set-variable -Wno-maybe-uninitialized")
        else()
            set(BOOST_OPENMETHOD_TEST_FLAGS "-Wall -Werror -Wno-unused-but-set-variable")
        endif()
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
        set(BOOST_OPENMETHOD_TEST_FLAGS "-Wall -Werror")
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND NOT CMAKE_CXX_COMPILER_FRONTEND_VARIANT MATCHES "MSVC")
        if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13)
            set(BOOST_OPENMETHOD_TEST_FLAGS "-Wall -Werror -Wno-unused-but-set-variable")
        else()
            set(BOOST_OPENMETHOD_TEST_FLAGS "-Wall -Werror")
        endif()
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR CMAKE_CXX_COMPILER_FRONTEND_VARIANT MATCHES "MSVC")
        set(BOOST_OPENMETHOD_TEST_FLAGS "/W4 /WX /we4265 /wd4251")
    endif()

    # Print test configuration if running in CI
    # This is useful for debugging CI failures related to warnings which might be false positives
    if (DEFINED ENV{CI})
        message(STATUS "Boost.OpenMethod Tests - Compiler ID: ${CMAKE_CXX_COMPILER_ID} / ${CMAKE_CXX_COMPILER_VERSION}")
        if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT)
            message(STATUS "Boost.OpenMethod Tests - Compiler Frontend: ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}")
        endif()
        message(STATUS "Boost.OpenMethod Tests - Platform: ${CMAKE_SYSTEM_NAME} / ${CMAKE_SYSTEM_VERSION}")
        message(STATUS "Boost.OpenMethod Tests - C++ standard: ${CMAKE_CXX_STANDARD}")
        message(STATUS "Boost.OpenMethod Tests - Test error flags: ${BOOST_OPENMETHOD_TEST_FLAGS}")
    endif()
endif()

file(GLOB test_cpp_files "test_*.cpp")

foreach(test_cpp ${test_cpp_files})
    cmake_path(REMOVE_EXTENSION test_cpp LAST_ONLY OUTPUT_VARIABLE test)
    string(REGEX REPLACE ".*/" "" test ${test})
    add_executable(${test} ${test_cpp})
    target_link_libraries(${test} PUBLIC Boost::openmethod Boost::unit_test_framework)
    add_test(NAME ${test} COMMAND ${test})
    add_dependencies(tests ${test})
endforeach()
