// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>

#include <boost/openmethod.hpp>
#include <boost/openmethod/compiler.hpp>

struct Animal {
    Animal(const char* name) : name(name) {
    }
    const char* name;
    virtual ~Animal() = default;
};

struct Cat : Animal {
    using Animal::Animal;
};

struct Dog : Animal {
    using Animal::Animal;
};

BOOST_OPENMETHOD(poke, (std::ostream&, virtual_ptr<Animal>), void);

BOOST_OPENMETHOD_OVERRIDE(
    poke, (std::ostream & os, virtual_ptr<Cat> cat), void) {
    os << cat->name << " hisses";
}

BOOST_OPENMETHOD_OVERRIDE(
    poke, (std::ostream & os, virtual_ptr<Dog> dog), void) {
    os << dog->name << " barks";
}

BOOST_OPENMETHOD_CLASSES(Animal, Cat, Dog);

// tag::next[]
struct Bulldog : Dog {
    using Dog::Dog;
};

BOOST_OPENMETHOD_CLASSES(Dog, Bulldog);

BOOST_OPENMETHOD_OVERRIDE(
    poke, (std::ostream & os, virtual_ptr<Bulldog> dog), void) {
    next(os, dog); // prints "bark"
    os << " and bites";
}
// end::next[]

BOOST_OPENMETHOD(
    encounter, (std::ostream&, virtual_ptr<Animal>, virtual_ptr<Animal>), void);

// 'encounter' catch-all implementation.
BOOST_OPENMETHOD_OVERRIDE(
    encounter,
    (std::ostream & os, virtual_ptr<Animal> a, virtual_ptr<Animal> b), void) {
    os << a->name << " and " << b->name << " ignore each other";
}

// Add definitions for specific pairs of animals.
BOOST_OPENMETHOD_OVERRIDE(
    encounter,
    (std::ostream & os, virtual_ptr<Dog> dog1, virtual_ptr<Dog> dog2), void) {
    os << "wag tail";
}

BOOST_OPENMETHOD_OVERRIDE(
    encounter, (std::ostream & os, virtual_ptr<Dog> dog, virtual_ptr<Cat> cat),
    void) {
    os << dog->name << " chases " << cat->name;
}

BOOST_OPENMETHOD_OVERRIDE(
    encounter, (std::ostream & os, virtual_ptr<Cat> cat, virtual_ptr<Dog> dog),
    void) {
    os << cat->name << " runs away from " << dog->name;
}

int main() {
    boost::openmethod::initialize();

    Animal&& a = Cat("Felix");
    Animal&& b = Dog("Snoopy");

    poke(std::cout, a); // Felix hisses
    std::cout << "\n";

    poke(std::cout, b); // Snoopy barks
    std::cout << "\n";

    Animal&& c = Bulldog("Hector");
    poke(std::cout, c); // Hector barks and bites
    std::cout << "\n";

    encounter(std::cout, a, b); // Felix runs away from Snoopy
    std::cout << "\n";

    encounter(std::cout, b, a); // Snoopy chases Felix
    std::cout << "\n";

    return 0;
}

void call_poke(std::ostream& os, virtual_ptr<Animal> a) {
    poke(os, a);
}

void call_poke(std::ostream& os, Animal& a) {
    poke(os, a);
}

void call_encounter(
    std::ostream& os, virtual_ptr<Animal> a, virtual_ptr<Animal> b) {
    encounter(os, a, b);
}
