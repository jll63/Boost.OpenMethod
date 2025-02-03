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

// static_assert(
//     std::is_same_v<decltype(bases((Animal*)0)), boost::mp11::mp_list<>>);

// static_assert(std::is_same_v<
//               decltype(bases((Cat*)0)), boost::mp11::mp_list<Animal>>);

// static_assert(std::is_same_v<
//               decltype(bases((DomesticCat*)0)),
//               boost::mp11::mp_list<Cat, Pet>>);

namespace bom = boost::openmethod;

struct Animal : bom::set_vptr<Animal> {};

struct Cat : Animal, bom::set_vptr<Cat, Animal> {};

struct Pet : bom::set_vptr<Pet> {
    std::string name;
};

struct DomesticCat : Cat, Pet, bom::set_vptr<DomesticCat, Cat, Pet> {};

BOOST_OPENMETHOD_CLASSES(Animal, Pet, Cat, DomesticCat);

BOOST_OPENMETHOD(
    describe, (bom::virtual_<const Pet&> pet, std::ostream& os), void);

BOOST_OPENMETHOD_OVERRIDE(describe, (const Pet& pet, std::ostream& os), void) {
    os << "I am a pet\n";
}

BOOST_OPENMETHOD_OVERRIDE(
    describe, (const DomesticCat& pet, std::ostream& os), void) {
    os << "I am " << pet.name << " the cat\n";
}

// Check that we pick one of the vptrs in presence of MI, dodging ambiguity
// issues.
BOOST_OPENMETHOD(
    cat_influencer, (bom::virtual_<const DomesticCat&> cat, std::ostream& os),
    void);

BOOST_OPENMETHOD_OVERRIDE(
    cat_influencer, (const DomesticCat& cat, std::ostream& os), void) {
    os << "Follow " << cat.name << " the cat on YouTube\n";
}

BOOST_AUTO_TEST_CASE(intrusive_mode) {
    bom::initialize();

    DomesticCat cat;
    cat.name = "Felix";

    {
        boost::test_tools::output_test_stream output;

        {
            capture_cout capture(output.rdbuf());
            describe(cat, std::cout);
        }

        BOOST_CHECK(output.is_equal("I am Felix the cat\n"));
    }

    {
        boost::test_tools::output_test_stream output;

        {
            capture_cout capture(output.rdbuf());
            cat_influencer(cat, std::cout);
        }

        BOOST_CHECK(output.is_equal("Follow Felix the cat on YouTube\n"));
    }
}
