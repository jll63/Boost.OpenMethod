# Copyright 2018, 2019 Peter Dimov
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required(VERSION 3.5...3.16)

project(boost_openmethod VERSION 1.87.0 LANGUAGES CXX)

if(POLICY CMP0167)
   cmake_policy(SET CMP0167 OLD)
endif()

option(STANDALONE "Set to ON to work with vcpkg Boost" OFF)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(boost_openmethod INTERFACE)
add_library(Boost::openmethod ALIAS boost_openmethod)

target_include_directories(boost_openmethod INTERFACE include)
target_compile_features(boost_openmethod INTERFACE cxx_alias_templates cxx_variadic_templates cxx_decltype)

find_package(Boost REQUIRED)

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
