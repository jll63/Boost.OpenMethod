// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <string>

#include <boost/openmethod.hpp>
#include <boost/openmethod/compiler.hpp>

#include "test_util.hpp"

#define BOOST_TEST_MODULE instrusive
#include <boost/test/included/unit_test.hpp>

#include <boost/test/output_test_stream.hpp>

namespace bom = boost::openmethod;

struct Animal : bom::set_vptr<Animal> {
    bom::vptr_type boost_openmethod_vptr;
};

struct Cat : Animal, bom::set_vptr<Cat> {};

struct Pet : bom::set_vptr<Pet> {
    bom::vptr_type boost_openmethod_vptr;
    std::string name;
};

struct DomesticCat : Animal, Pet, bom::set_vptr<DomesticCat> {
    //using Animal::boost_openmethod_vptr;
};

BOOST_OPENMETHOD_CLASSES(Animal, Pet, Cat, DomesticCat);

BOOST_OPENMETHOD(
    describe, (bom::virtual_<const Pet&> pet, std::ostream& os), void);

BOOST_OPENMETHOD_OVERRIDE(
    describe, (const Pet& pet, std::ostream& os), void) {
    os << "I am a pet\n";
}

BOOST_OPENMETHOD_OVERRIDE(
    describe, (const DomesticCat& pet, std::ostream& os), void) {
    os << "I am " << pet.name << " the cat\n";
}

BOOST_AUTO_TEST_CASE(intrusive_mode) {
    bom::initialize();

    DomesticCat cat;
    cat.name = "Felix";

    {
        boost::test_tools::output_test_stream output;
        capture_cout capture(output.rdbuf());
        describe(cat, std::cout);
        BOOST_CHECK_EQUAL(output.str(), "I am Felix the cat\n");
    }
}
