// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

// clang-format off

// tag::domain_classes[]
#include <string>

struct Animal {
    Animal(std::string name) : name(name) {
    }
    std::string name;
    virtual ~Animal() = default;
};

struct Cat : Animal {
    using Animal::Animal;
};

struct Dog : Animal {
    using Animal::Animal;
};

struct Bulldog : Dog {
    using Dog::Dog;
};
// end::domain_classes[]

// tag::method[]
#include <iostream>
#include <boost/openmethod.hpp>

BOOST_OPENMETHOD(
    poke,                                       // method name
    (std::ostream&, virtual_ptr<Animal>),       // method signature
    void);                                      // return type
// end::method[]

// tag::overriders[]
BOOST_OPENMETHOD_OVERRIDE(
    poke,                                       // method name
    (std::ostream & os, virtual_ptr<Cat> cat),  // overrider signature
    void) {                                     // return type
    os << cat->name << " hisses";               // overrider body
}

BOOST_OPENMETHOD_OVERRIDE(
    poke, (std::ostream & os, virtual_ptr<Dog> dog), void) {
    os << dog->name << " barks";
}
// end::overriders[]

// tag::next[]
BOOST_OPENMETHOD_OVERRIDE(
    poke, (std::ostream & os, virtual_ptr<Bulldog> dog), void) {
    next(os, dog);                              // call super-method
    os << " and bites back";
}
// end::next[]

// tag::classes[]
BOOST_OPENMETHOD_CLASSES(Animal, Cat, Dog, Bulldog);
// end::classes[]

// tag::multi_method[]
BOOST_OPENMETHOD(encounter, (std::ostream&, virtual_ptr<Animal>, virtual_ptr<Animal>), void);

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
// end::multi_method[]

// tag::main1[]
// only needed in the file that calls boost::openmethod::initialize()
#include <boost/openmethod/compiler.hpp>

int main() {
    boost::openmethod::initialize();
// end::main1[]

// tag::main2[]
    std::unique_ptr<Animal> a(new Cat("Felix"));
    std::unique_ptr<Animal> b(new Dog("Snoopy"));
    std::unique_ptr<Animal> c(new Bulldog("Hector"));

    poke(std::cout, *a); // Felix hisses
    std::cout << ".\n";

    poke(std::cout, *b); // Snoopy barks
    std::cout << ".\n";
// end::main2[]

    poke(std::cout, *c); // Hector barks and bites
    std::cout << ".\n";
    // end::call[]

    encounter(std::cout, *a, *b); // Felix runs away from Snoopy
    std::cout << ".\n";

    encounter(std::cout, *b, *a); // Snoopy chases Felix
    std::cout << ".\n";

    return 0;
}

auto make_virtual_ptr(std::ostream& os, Animal& a) {
    return virtual_ptr<Animal>(a);
}

auto make_vinal_virtual_ptr(std::ostream& os, Cat& cat) {
    return boost::openmethod::final_virtual_ptr(cat);
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
