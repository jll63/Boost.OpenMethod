// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

// clang-format off

// tag::ast[]

#include <iostream>

#include <boost/openmethod.hpp>
#include <boost/openmethod/compiler.hpp>

using boost::openmethod::virtual_ptr;

struct Node {
    virtual ~Node() {}
};

struct Literal : Node {
    explicit Literal(int value) : value(value) {}

    int value;
};

struct Plus : Node {
    Plus(virtual_ptr<Node> left, virtual_ptr<Node> right)
        : left(left), right(right) {}

    virtual_ptr<Node> left, right;
};

struct Negate : Node {
    explicit Negate(virtual_ptr<Node> node) : child(node) {}

    virtual_ptr<Node> child;
};

BOOST_OPENMETHOD(value, (virtual_ptr<Node>), int);

BOOST_OPENMETHOD_OVERRIDE(value, (virtual_ptr<Literal> node), int) {
    return node->value;
}

BOOST_OPENMETHOD_OVERRIDE(value, (virtual_ptr<Plus> node), int) {
    return value(node->left) + value(node->right);
}

BOOST_OPENMETHOD_OVERRIDE(value, (virtual_ptr<Negate> node), int) {
    return -value(node->child);
}

BOOST_OPENMETHOD_CLASSES(Node, Literal, Plus, Negate);

auto main() -> int {
    boost::openmethod::initialize();

    Literal one(1), two(2);
    Plus sum(one, two);
    Negate neg(sum);

    std::cout << value(neg) << "\n"; // -3

    return 0;
}
// end::ast[]

auto negate(virtual_ptr<Node> node) -> int {
    return -value(node);
}

#define main alt_main

auto main() -> int {
// tag::final[]
    Literal one(1);
    Negate neg(boost::openmethod::final_virtual_ptr(one));
// end::final[]

    std::cout << value(boost::openmethod::final_virtual_ptr(neg)) << "\n"; // -3

    return 0;
}
