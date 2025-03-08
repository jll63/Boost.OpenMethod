// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_TEST_HELPERS_HPP
#define BOOST_OPENMETHOD_TEST_HELPERS_HPP

#include <iostream>

#include <boost/openmethod/core.hpp>
#include <boost/openmethod/compiler.hpp>

template<int Name>
struct test_policy_ :
#ifdef NDEBUG
    boost::openmethod::policies::release::fork<test_policy_<Name>>
#else
    boost::openmethod::policies::debug::fork<test_policy_<Name>>
#endif
{
};

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
