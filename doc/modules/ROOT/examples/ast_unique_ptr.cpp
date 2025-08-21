// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

// clang-format off

// tag::ast[]

#include <iostream>
#include <memory>

#include <boost/openmethod.hpp>
#include <boost/openmethod/unique_ptr.hpp>
#include <boost/openmethod/initialize.hpp>

using namespace boost::openmethod::aliases;

struct Node {
    virtual ~Node() {}
};

struct Literal : Node {
    Literal(int value) : value(value) {}

    int value;
};

struct Plus : Node {
    Plus(unique_virtual_ptr<Node> left, unique_virtual_ptr<Node> right)
        : left(std::move(left)), right(std::move(right)) {}

    unique_virtual_ptr<Node> left, right;
};

struct Negate : Node {
    Negate(unique_virtual_ptr<Node> node) : child(std::move(node)) {}

    unique_virtual_ptr<Node> child;
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

    auto expr = make_unique_virtual<Negate>(
        make_unique_virtual<Plus>(
            make_unique_virtual<Literal>(1),
            make_unique_virtual<Literal>(2)));

    std::cout << value(expr) << "\n"; // -3

    return 0;
}
// end::ast[]
