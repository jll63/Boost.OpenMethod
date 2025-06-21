# Copyright (c) 2018-2025 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt
# or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.10)

if(POLICY CMP0167)
  cmake_policy(SET CMP0074 NEW)
endif()

if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

project(boost_openmethod VERSION 1.87.0 LANGUAGES CXX)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(boost_openmethod INTERFACE)
add_library(Boost::openmethod ALIAS boost_openmethod)

target_include_directories(boost_openmethod INTERFACE include)

find_package(Boost COMPONENTS)
target_link_libraries(boost_openmethod INTERFACE Boost::boost)

if(MSVC)
  add_compile_options(/EHsc /FAs /bigobj)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  message(STATUS "Enabling sanitizers")
  add_compile_options(
    -fPIC -Wno-deprecated-declarations -Wall -Wextra -Werror
    -fsanitize=undefined -fsanitize=address -fsanitize=leak)
  add_link_options(-fsanitize=undefined -fsanitize=address -fsanitize=leak)
endif()

if(BUILD_TESTING)
  message(STATUS "Building tests")
  include(CTest)
  enable_testing()
  add_subdirectory(test)
endif()

if(BUILD_EXAMPLES)
  message(STATUS "Building examples")
  add_subdirectory(examples)
endif()
