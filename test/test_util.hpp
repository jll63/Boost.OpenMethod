#ifndef BOOST_OPENMETHOD_TEST_HELPERS_HPP
#define BOOST_OPENMETHOD_TEST_HELPERS_HPP

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

#endif
