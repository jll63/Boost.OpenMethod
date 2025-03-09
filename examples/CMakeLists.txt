# Copyright (c) 2018-2024 Jean-Louis Leroy
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt
# or copy at http://www.boost.org/LICENSE_1_0.txt)

if (CMAKE_BUILD_TYPE MATCHES "Release")
  if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-save-temps -masm=intel)
  endif()
endif()

add_executable(virtual_func virtual_func.cpp)
target_link_libraries(virtual_func Boost::openmethod)
add_test(NAME virtual_func COMMAND virtual_func)

add_executable(ast ast.cpp)
target_link_libraries(ast Boost::openmethod)
add_test(NAME ast COMMAND ast)

add_executable(ast_unique_ptr ast_unique_ptr.cpp)
target_link_libraries(ast_unique_ptr Boost::openmethod)
add_test(NAME ast_unique_ptr COMMAND ast_unique_ptr)

add_executable(hello_world hello_world.cpp)
target_link_libraries(hello_world Boost::openmethod)
add_test(NAME hello_world COMMAND hello_world)

add_executable(friendship_all friendship.cpp)
target_compile_definitions(friendship_all PRIVATE FRIEND_ALL)
target_link_libraries(friendship_all Boost::openmethod)
add_test(NAME friendship_add COMMAND friendship_all)

add_executable(friendship friendship.cpp)
target_link_libraries(friendship Boost::openmethod)
add_test(NAME friendship COMMAND friendship)

add_executable(friendship_across_namespaces_all friendship_across_namespaces.cpp)
target_compile_definitions(friendship_across_namespaces_all PRIVATE FRIEND_ALL)
target_link_libraries(friendship_across_namespaces_all Boost::openmethod)
add_test(NAME friendship_across_namespaces_all COMMAND friendship_across_namespaces_all)

add_executable(friendship_across_namespaces friendship_across_namespaces.cpp)
target_link_libraries(friendship_across_namespaces Boost::openmethod)
add_test(NAME friendship_across_namespaces COMMAND friendship_across_namespaces)

add_executable(virtual_ptr virtual_ptr.cpp)
target_link_libraries(virtual_ptr Boost::openmethod)
add_test(NAME virtual_ptr COMMAND virtual_ptr)

add_executable(virtual_ virtual_.cpp)
target_link_libraries(virtual_ Boost::openmethod)
add_test(NAME virtual_ COMMAND virtual_)

add_executable(core_api core_api.cpp)
target_link_libraries(core_api Boost::openmethod)
add_test(NAME core_api COMMAND core_api)

add_executable(vectored_error_handler vectored_error_handler.cpp)
target_link_libraries(vectored_error_handler Boost::openmethod)
add_test(NAME vectored_error_handler COMMAND vectored_error_handler)

add_executable(throw_error_handler throw_error_handler.cpp)
target_link_libraries(throw_error_handler Boost::openmethod)
add_test(NAME throw_error_handler COMMAND throw_error_handler)

add_executable(custom_rtti custom_rtti.cpp)
target_link_libraries(custom_rtti Boost::openmethod)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set_target_properties(custom_rtti PROPERTIES COMPILE_FLAGS "-fno-rtti")
endif()
add_test(NAME custom_rtti COMMAND custom_rtti)

add_executable(deferred_custom_rtti deferred_custom_rtti.cpp)
target_link_libraries(deferred_custom_rtti Boost::openmethod)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set_target_properties(deferred_custom_rtti PROPERTIES COMPILE_FLAGS "-fno-rtti")
endif()
add_test(NAME deferred_custom_rtti COMMAND deferred_custom_rtti)

add_executable(slides slides.cpp)
target_link_libraries(slides Boost::openmethod)
add_test(NAME slides COMMAND slides)

add_executable(synopsis synopsis.cpp)
target_link_libraries(synopsis Boost::openmethod)
add_test(NAME synopsis COMMAND synopsis)

add_executable(matrix matrix.cpp)
target_link_libraries(matrix Boost::openmethod)
add_test(NAME matrix COMMAND matrix)

add_executable(accept_no_visitors accept_no_visitors.cpp)
target_link_libraries(accept_no_visitors Boost::openmethod)
add_test(NAME accept_no_visitors COMMAND accept_no_visitors)

add_executable(adventure adventure.cpp)
target_link_libraries(adventure Boost::openmethod)
add_test(NAME adventure COMMAND adventure)

add_executable(next next.cpp)
target_link_libraries(next Boost::openmethod)
add_test(NAME next COMMAND next)

add_executable(asteroids asteroids.cpp)
target_link_libraries(asteroids Boost::openmethod)
add_test(NAME asteroids COMMAND asteroids)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  message(STATUS "Building dlopen example")
  add_executable(dl_main dl_main.cpp)
  add_library(dl_shared SHARED dl_shared.cpp)
  get_target_property(BOOST_OPENMETHOD_INCLUDE_DIRS Boost::openmethod INTERFACE_INCLUDE_DIRECTORIES)
  target_include_directories(dl_shared PUBLIC ${BOOST_OPENMETHOD_INCLUDE_DIRS})
  add_dependencies(dl_main dl_shared)
  set_target_properties(dl_main PROPERTIES LINK_FLAGS "-Wl,-export-dynamic")
  target_link_libraries(dl_main Boost::openmethod dl)
  target_link_libraries(dl_shared Boost::openmethod)
  add_test(NAME dlopen COMMAND dl_main)
endif()
