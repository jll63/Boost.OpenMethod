// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

// clang-format off

#include <iostream>
#include <string>
#include <boost/openmethod.hpp>

#ifdef FRIEND_ALL

// tag::friend_all[]
class Cat;
class Dog;

class Animal {
// end::friend_all[]

  public:
    Animal(std::string name) : name(name) {
    }
    virtual ~Animal() = default;
// tag::friend_all[]
    // ...
  private:
    std::string name;

    template<typename...> friend class BOOST_OPENMETHOD_OVERRIDERS(poke);
};
// end::friend_all[]

#else

// tag::forward[]
template<typename...> struct BOOST_OPENMETHOD_OVERRIDERS(poke);
// end::forward[]

// tag::friend[]
class Cat;
class Dog;

class Animal {
    // ...
// end::friend[]
  public:
    Animal(std::string name) : name(name) {
    }
    virtual ~Animal() = default;

// tag::friend[]
  private:
    std::string name;

    friend class BOOST_OPENMETHOD_OVERRIDERS(poke)<void(std::ostream&, virtual_ptr<Cat>)>;
    friend class BOOST_OPENMETHOD_OVERRIDERS(poke)<void(std::ostream&, virtual_ptr<Dog>)>;
};
// end::friend[]

#endif

struct Cat : Animal {
    using Animal::Animal;
};

struct Dog : Animal {
    using Animal::Animal;
};

class Animal;

BOOST_OPENMETHOD(poke, (std::ostream&, virtual_ptr<Animal>), void);

BOOST_OPENMETHOD_OVERRIDE(
    poke, (std::ostream& os, virtual_ptr<Cat> cat), void) {
    os << cat->name << " hisses";
}

BOOST_OPENMETHOD_OVERRIDE(
    poke, (std::ostream& os, virtual_ptr<Dog> dog), void) {
    os << dog->name << " barks";
}

BOOST_OPENMETHOD_CLASSES(Animal, Cat, Dog);

#include <boost/openmethod/compiler.hpp>

int main() {
    boost::openmethod::initialize();

    std::unique_ptr<Animal> a(new Cat("Felix"));
    std::unique_ptr<Animal> b(new Dog("Snoopy"));

    poke(std::cout, *a); // Felix hisses
    std::cout << ".\n";

    poke(std::cout, *b); // Snoopy barks
    std::cout << ".\n";

    return 0;
}