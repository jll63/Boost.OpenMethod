// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/openmethod.hpp>
#include <boost/openmethod/compiler.hpp>

#include <exception>

#define BOOST_TEST_MODULE openmethod
#include <boost/test/unit_test.hpp>

struct A {
    virtual ~A() = default;
};

struct B : virtual A {
    virtual ~B() = default;
};

struct B1 : B {};
struct B2 : B {};

struct C : B1, B2 {};

using boost::openmethod::virtual_ptr;

BOOST_OPENMETHOD_CLASSES(A, B, B1, B2, C);

BOOST_OPENMETHOD(wrong, (virtual_ptr<A>), void);

BOOST_OPENMETHOD_OVERRIDE(wrong, (virtual_ptr<B>), void) {
}

BOOST_AUTO_TEST_CASE(repeated_inheritance_throws_bad_cast) {
    boost::openmethod::initialize();

    C obj;
    A& base = static_cast<B1&>(obj);

    BOOST_CHECK_THROW(wrong(base), std::bad_cast);
}
