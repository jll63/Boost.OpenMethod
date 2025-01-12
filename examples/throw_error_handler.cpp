// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>

#include <boost/openmethod/policies.hpp>
#include <boost/openmethod/policies/throw_error_handler.hpp>

struct Animal {
    virtual ~Animal() = default;
};

struct Cat : Animal {};
struct Dog : Animal {};

namespace bom = boost::openmethod;

struct throwing_policy
    : bom::default_policy::fork<throwing_policy>::replace<
          bom::policies::error_handler, bom::policies::throw_error_handler> {};

#define BOOST_OPENMETHOD_DEFAULT_POLICY throwing_policy

#include <boost/openmethod.hpp>
#include <boost/openmethod/compiler.hpp>

BOOST_OPENMETHOD_CLASSES(Animal, Cat, Dog, throwing_policy);

BOOST_OPENMETHOD(
    trick, (std::ostream&, virtual_ptr<Animal>), void);

BOOST_OPENMETHOD_OVERRIDE(
    trick, (std::ostream & os, virtual_ptr<Dog> dog), void) {
    os << "spin\n";
}

int main() {
    bom::initialize<throwing_policy>();

    Cat felix;
    Dog hector, snoopy;
    std::vector<Animal*> animals = {&hector, &felix, &snoopy};

    for (auto animal : animals) {
        try {
            trick(std::cout, *animal);
        } catch (bom::not_implemented_error) {
            std::cerr << boost::core::demangle(typeid(*animal).name())
                      << "s don't perform tricks\n";
        }
    }

    return 0;
}
