cmake_minimum_required(VERSION 3.5...3.16)

project(boost_openmethod VERSION 1.87.0 LANGUAGES CXX)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED)

add_library(boost_openmethod INTERFACE)
add_library(Boost::openmethod ALIAS boost_openmethod)

target_include_directories(boost_openmethod INTERFACE include)

include(CTest)
enable_testing()
add_subdirectory(examples)
add_subdirectory(test)
