// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <string>

#include <boost/openmethod/policies.hpp>

namespace bom = boost::openmethod;
struct test_policy : bom::default_policy::remove<bom::policies::extern_vptr> {};
#define BOOST_OPENMETHOD_DEFAULT_POLICY test_policy

#include <boost/openmethod.hpp>
#include <boost/openmethod/with_vptr.hpp>
#include <boost/openmethod/compiler.hpp>

#define BOOST_TEST_MODULE instrusive
#include <boost/test/included/unit_test.hpp>
#include <boost/test/tools/output_test_stream.hpp>

#include "test_util.hpp"

namespace bom = boost::openmethod;
using bom::virtual_;

struct Animal : bom::with_vptr<Animal> {
    Animal();
    ~Animal();
};

struct Cat : Animal, bom::with_vptr<Cat, Animal> {
    Cat();
    ~Cat();
};

struct Pet : bom::with_vptr<Pet> {
    Pet();
    ~Pet();
    std::string name;
};

struct DomesticCat : Cat, Pet, bom::with_vptr<DomesticCat, Cat, Pet> {
    DomesticCat();
    ~DomesticCat();
};

BOOST_OPENMETHOD(
    speak, (virtual_<const Animal&> animal, std::ostream& os), void);
BOOST_OPENMETHOD(describe, (virtual_<const Pet&> pet, std::ostream& os), void);

Animal::Animal() {
    speak(*this, std::cout);
}

Animal::~Animal() {
    speak(*this, std::cout);
}

Cat::Cat() {
    speak(*this, std::cout);
}

Cat::~Cat() {
    speak(*this, std::cout);
}

Pet::Pet() {
    describe(*this, std::cout);
}

Pet::~Pet() {
    describe(*this, std::cout);
}

DomesticCat::DomesticCat() {
    name = "Felix";
    describe(*this, std::cout);
}

DomesticCat::~DomesticCat() {
    name = "Felix";
    describe(*this, std::cout);
}

BOOST_OPENMETHOD_OVERRIDE(
    speak, (const Animal& animal, std::ostream& os), void) {
    os << "???\n";
}

BOOST_OPENMETHOD_OVERRIDE(speak, (const Cat& animal, std::ostream& os), void) {
    os << "meow\n";
}

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
    cat_influencer, (virtual_<const DomesticCat&> cat, std::ostream& os), void);

BOOST_OPENMETHOD_OVERRIDE(
    cat_influencer, (const DomesticCat& cat, std::ostream& os), void) {
    os << "Follow " << cat.name << " the cat on YouTube\n";
}

BOOST_AUTO_TEST_CASE(intrusive_mode) {
    bom::initialize();

    std::unique_ptr<DomesticCat> cat;

    {
        boost::test_tools::output_test_stream output;
        {
            capture_cout capture(output.rdbuf());
            cat = std::make_unique<DomesticCat>();
        }

        BOOST_CHECK(output.is_equal(
            "???\n"
            "meow\n"
            "I am a pet\n"
            "I am Felix the cat\n"));
    }

    {
        boost::test_tools::output_test_stream output;

        {
            capture_cout capture(output.rdbuf());
            describe(*cat, std::cout);
        }

        BOOST_CHECK(output.is_equal("I am Felix the cat\n"));
    }

    {
        boost::test_tools::output_test_stream output;

        {
            capture_cout capture(output.rdbuf());
            cat_influencer(*cat, std::cout);
        }

        BOOST_CHECK(output.is_equal("Follow Felix the cat on YouTube\n"));
    }

    {
        boost::test_tools::output_test_stream output;
        {
            capture_cout capture(output.rdbuf());
            cat.reset();
        }

        BOOST_CHECK(output.is_equal(
            "I am Felix the cat\n"
            "I am a pet\n"
            "meow\n"
            "???\n"));
    }
}

struct indirect_policy : test_policy::add<bom::policies::indirect_vptr> {};

struct Indirect : bom::with_vptr<Indirect, indirect_policy> {
    using bom::with_vptr<Indirect, indirect_policy>::boost_openmethod_vptr;
};

BOOST_OPENMETHOD(whatever, (virtual_<Indirect&>), void, indirect_policy);

BOOST_OPENMETHOD_OVERRIDE(whatever, (Indirect&), void) {
}

BOOST_AUTO_TEST_CASE(core_intrusive_vptr) {
    bom::initialize<indirect_policy>();
    Indirect i;
    BOOST_TEST(
        i.boost_openmethod_vptr == &indirect_policy::static_vptr<Indirect>);
}
