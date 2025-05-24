// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>

#include <boost/openmethod/compiler.hpp>
#include <boost/openmethod/core.hpp>

struct Animal {
    virtual ~Animal() = default;
};

struct Cat : Animal {};

struct Dog : Animal {};

struct Bulldog : Dog {};

using namespace boost::openmethod;

// tag::method[]

#include <boost/openmethod/macros.hpp>

class BOOST_OPENMETHOD_NAME(poke);

using poke = method<
    BOOST_OPENMETHOD_NAME(poke),
    auto(std::ostream&, virtual_ptr<Animal>)->void>;
// end::method[]

// tag::poke_cat[]
auto poke_cat(std::ostream& os, virtual_ptr<Cat> cat) {
    os << "hiss";
}

static poke::override<poke_cat> override_poke_cat;
// end::poke_cat[]

// tag::poke_dog[]
#include <boost/openmethod/macros.hpp>

auto poke_dog(std::ostream& os, virtual_ptr<Dog> dog) {
    os << "bark";
}

BOOST_OPENMETHOD_REGISTER(poke::override<poke_dog>);
// end::poke_dog[]

// tag::poke_bulldog[]
auto poke_bulldog(std::ostream& os, virtual_ptr<Bulldog> dog) -> void {
    poke::next<poke_bulldog>(os, dog);
    os << " and bite";
}

BOOST_OPENMETHOD_REGISTER(poke::override<poke_bulldog>);
// end::poke_bulldog[]

class BOOST_OPENMETHOD_NAME(pet);

auto pet_cat(std::ostream& os, virtual_ptr<Cat> cat) {
    os << "purr";
}

auto pet_dog(std::ostream& os, virtual_ptr<Dog> dog) {
    os << "wag tail";
}

using pet = method<
    BOOST_OPENMETHOD_NAME(pet), auto(std::ostream&, virtual_ptr<Animal>)->void>;

BOOST_OPENMETHOD_REGISTER(pet::override<pet_cat, pet_dog>);

// tag::use_classes[]
BOOST_OPENMETHOD_REGISTER(use_classes<Animal, Cat, Dog, Bulldog>);
// end::use_classes[]

// tag::main[]
auto main() -> int {
    boost::openmethod::initialize();

    std::unique_ptr<Animal> a(new Cat);
    std::unique_ptr<Animal> b(new Dog);
    std::unique_ptr<Animal> c(new Bulldog);

    poke::fn(std::cout, *a); // prints "hiss"
    std::cout << "\n";

    poke::fn(std::cout, *b); // prints "bark"
    std::cout << "\n";

    poke::fn(std::cout, *c); // prints "bark and bite"
    std::cout << "\n";
    // end::main[]

    pet::fn(std::cout, *a); // prints "purr"
    std::cout << "\n";
    // tag::main[]

    return 0;
    // end::main[]
}
