// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_TEST_HELPERS_HPP
#define BOOST_OPENMETHOD_TEST_HELPERS_HPP

#include <iostream>

#include <boost/openmethod/core.hpp>
#include <boost/openmethod/compiler.hpp>

template<int N>
    struct test_registry_
    : boost::openmethod::default_registry::with <
      boost::openmethod::policies::unique<test_registry_<N>>> {};

#define TEST_NS BOOST_PP_CAT(test, __COUNTER__)

struct capture_cout {
    capture_cout(std::streambuf* new_buffer)
        : old(std::cout.rdbuf(new_buffer)) {
    }

    ~capture_cout() {
        std::cout.rdbuf(old);
    }

  private:
    std::streambuf* old;
};

#endif
