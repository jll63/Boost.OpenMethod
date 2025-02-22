// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>

#include <boost/openmethod.hpp>
#include <boost/openmethod/compiler.hpp>

using boost::openmethod::virtual_;

// clang-format off

namespace just_virtual {

struct Animal {
    virtual ~Animal() = default;
};

struct Cat : Animal {};

// tag::virtual_parameter[]
using boost::openmethod::virtual_;

BOOST_OPENMETHOD(poke, (std::ostream&, virtual_<Animal&>), void);

BOOST_OPENMETHOD_OVERRIDE(poke, (std::ostream & os, Cat& cat), void) {
    os << "hiss\n";
}

BOOST_OPENMETHOD_CLASSES(Animal, Cat);

// end::virtual_parameter[]
} // namespace just_virtual

namespace virtual_intrusive {

// tag::virtual_intrusive[]

class Animal {
  protected:
    boost::openmethod::vptr_type vptr;
    friend auto boost_openmethod_vptr(const Animal& a) {
        return a.vptr;
    }

  public:
    Animal() {
        vptr = boost::openmethod::default_policy::static_vptr<Animal>;
    }
};

class Cat : public Animal {
  public:
    Cat() {
        vptr = boost::openmethod::default_policy::static_vptr<Cat>;
    }
};

// end::virtual_intrusive[]

using boost::openmethod::virtual_;

BOOST_OPENMETHOD(poke, (std::ostream&, virtual_<Animal&>), void);

BOOST_OPENMETHOD_OVERRIDE(poke, (std::ostream & os, Cat& cat), void) {
    os << "hiss\n";
}

BOOST_OPENMETHOD_CLASSES(Animal, Cat);
} // namespace virtual_intrusive

namespace with_vptr {

// tag::with_vptr[]

class Animal : public boost::openmethod::with_vptr<Animal> {};

class Cat : public Animal, public boost::openmethod::with_vptr<Cat, Animal> {};

using boost::openmethod::virtual_;

BOOST_OPENMETHOD(poke, (std::ostream&, virtual_<Animal&>), void);

BOOST_OPENMETHOD_OVERRIDE(poke, (std::ostream & os, Cat& cat), void) {
    os << "hiss\n";
}

// end::with_vptr[]

} // namespace with_vptr

int main() {
    boost::openmethod::initialize();

    {
        using namespace just_virtual;
        // tag::call_ref[]
        Cat cat;
        poke(std::cout, cat); // hiss
        // end::call_ref[]
    }

    {
        using namespace virtual_intrusive;
        Cat cat;
        poke(std::cout, cat); // hiss
    }

    {
        using namespace with_vptr;
        Cat cat;
        poke(std::cout, cat); // hiss
    }
}
