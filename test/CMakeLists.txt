# Copyright 2018 Peter Dimov
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt

include(BoostTestJamfile OPTIONAL RESULT_VARIABLE HAVE_BOOST_TEST)

if(STANDALONE)
    file(GLOB test_cpp_files "test_*.cpp")
    foreach(test_cpp ${test_cpp_files})
        cmake_path(REMOVE_EXTENSION test_cpp LAST_ONLY OUTPUT_VARIABLE test)
        string(REGEX REPLACE ".*/" "" test ${test})
        add_executable(${test} ${test_cpp})
        target_link_libraries(${test} Boost::openmethod)
        add_test(NAME ${test} COMMAND ${test})
        #message(STATUS "add_test(NAME ${test} COMMAND ${test})")
    endforeach()
elseif(HAVE_BOOST_TEST)
    boost_test_jamfile(FILE Jamfile LINK_LIBRARIES Boost::openmethod Boost::core)
    boost_test(SOURCES check_cmake_version.cpp ARGUMENTS ${PROJECT_VERSION} LINK_LIBRARIES Boost::core Boost::config)
endif()
