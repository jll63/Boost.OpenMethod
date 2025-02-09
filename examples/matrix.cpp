// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <memory>
#include <string>
#include <typeinfo>

#include <boost/openmethod.hpp>
#include <boost/openmethod/compiler.hpp>

using std::make_shared;
using std::shared_ptr;
using std::string;

using boost::openmethod::make_virtual_shared;
using boost::openmethod::virtual_shared_ptr;

struct matrix {
    virtual ~matrix() {
    }
    virtual double at(int row, int col) const = 0;
    // ...
};

struct dense_matrix : matrix {
    virtual double at(int row, int col) const {
        return 0;
    }
};

struct diagonal_matrix : matrix {
    virtual double at(int row, int col) const {
        return 0;
    }
};

BOOST_OPENMETHOD_CLASSES(matrix, dense_matrix, diagonal_matrix);

BOOST_OPENMETHOD(to_json, (virtual_ptr<const matrix>), string);

BOOST_OPENMETHOD_OVERRIDE(
    to_json, (virtual_ptr<const dense_matrix> m), string) {
    return "json for dense matrix...";
}

BOOST_OPENMETHOD_OVERRIDE(
    to_json, (virtual_ptr<const diagonal_matrix> m), string) {
    return "json for diagonal matrix...";
}

// -----------------------------------------------------------------------------
// matrix * matrix

BOOST_OPENMETHOD(
    times, (virtual_shared_ptr<const matrix>, virtual_shared_ptr<const matrix>),
    virtual_shared_ptr<const matrix>);

// catch-all matrix * matrix -> dense_matrix
BOOST_OPENMETHOD_OVERRIDE(
    times,
    (virtual_shared_ptr<const matrix> a, virtual_shared_ptr<const matrix> b),
    virtual_shared_ptr<const dense_matrix>) {
    return make_shared<const dense_matrix>();
}

// diagonal_matrix * diagonal_matrix -> diagonal_matrix
BOOST_OPENMETHOD_OVERRIDE(
    times,
    (virtual_shared_ptr<const diagonal_matrix> a,
     virtual_shared_ptr<const diagonal_matrix> b),
    virtual_shared_ptr<const diagonal_matrix>) {
    return make_virtual_shared<diagonal_matrix>();
}

inline virtual_shared_ptr<const matrix>
operator*(shared_ptr<const matrix> a, shared_ptr<const matrix> b) {
    return times(a, b);
}

// -----------------------------------------------------------------------------
// scalar * matrix

BOOST_OPENMETHOD(
    times, (double, virtual_shared_ptr<const matrix>),
    virtual_shared_ptr<const matrix>);

// catch-all matrix * scalar -> dense_matrix
BOOST_OPENMETHOD_OVERRIDE(
    times, (double a, virtual_shared_ptr<const matrix> b),
    virtual_shared_ptr<const dense_matrix>) {
    return make_virtual_shared<dense_matrix>();
}

BOOST_OPENMETHOD_OVERRIDE(
    times, (double a, virtual_shared_ptr<const diagonal_matrix> b),
    virtual_shared_ptr<const diagonal_matrix>) {
    return make_virtual_shared<diagonal_matrix>();
}

// -----------------------------------------------------------------------------
// matrix * scalar

// just swap
inline virtual_shared_ptr<const matrix>
times(virtual_shared_ptr<const matrix> a, double b) {
    return times(b, a);
}

// -----------------------------------------------------------------------------
// main

#define check(expr)                                                            \
    {                                                                          \
        if (!(expr)) {                                                         \
            cerr << #expr << " failed\n";                                      \
        }                                                                      \
    }

int main() {
    using std::cerr;
    using std::cout;

    boost::openmethod::initialize();

    shared_ptr<const matrix> a = make_shared<dense_matrix>();
    shared_ptr<const matrix> b = make_shared<diagonal_matrix>();
    double s = 1;

#ifndef _MSC_VER
#pragma clang diagnostic ignored "-Wpotentially-evaluated-expression"
#endif

    check(typeid(*times(a, a)) == typeid(dense_matrix));
    check(typeid(*times(a, b)) == typeid(dense_matrix));
    check(typeid(*times(b, b)) == typeid(diagonal_matrix));
    check(typeid(*times(s, a)) == typeid(dense_matrix));
    check(typeid(*times(s, b)) == typeid(diagonal_matrix));

    cout << to_json(*a) << "\n"; // json for dense matrix
    cout << to_json(*b) << "\n"; // json for diagonal matrix

    return 0;
}
