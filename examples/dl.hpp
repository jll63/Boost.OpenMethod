// dl.hpp
// Copyright (c) 2018-2021 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// clang-format on

#ifndef DL_DEFINED
#define DL_DEFINED

// tag::header[]
// dl.hpp

#include <string>

#include <boost/openmethod.hpp>

struct Animal {
    virtual ~Animal() {
    }
};

struct Herbivore : Animal {};
struct Carnivore : Animal {};
struct Cow : Herbivore {};
struct Wolf : Carnivore {};

struct dynamic_policy
    : boost::openmethod::default_policy::fork<dynamic_policy>::with<
          boost::openmethod::policies::indirect_vptr> {};

template<class Class>
using dyn_vptr = boost::openmethod::virtual_ptr<Class, dynamic_policy>;

BOOST_OPENMETHOD(
    encounter, (dyn_vptr<Animal>, dyn_vptr<Animal>), std::string,
    dynamic_policy);

// end::header[]

#endif
