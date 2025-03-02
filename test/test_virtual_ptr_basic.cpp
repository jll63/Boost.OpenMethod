// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/openmethod.hpp>
#include <boost/openmethod/shared_ptr.hpp>
#include <boost/openmethod/unique_ptr.hpp>
#include <boost/openmethod/compiler.hpp>

#include <iostream>
#include <memory>
#include <string>

#define BOOST_TEST_MODULE openmethod
#include <boost/test/included/unit_test.hpp>
#include <boost/test/tools/output_test_stream.hpp>

#include "test_util.hpp"

using boost::mp11::mp_list;
using std::cout;
using namespace boost::openmethod;
using namespace boost::openmethod::detail;

struct base {
    virtual ~base() {
    }
};

struct a : base {};
struct b : base {};
struct c : base {};
struct d : base {};
struct e : base {};
struct f : base {};

static_assert(
    std::is_same_v<
        virtual_types<mp_list<
            virtual_<std::shared_ptr<a>>, b, virtual_<std::shared_ptr<c>>>>,
        mp_list<std::shared_ptr<a>, std::shared_ptr<c>>>);

static_assert(std::is_same_v<
              overrider_virtual_types<
                  mp_list<virtual_<a&>, b, virtual_<c&>>, mp_list<d&, e, f&>,
                  default_policy>,
              mp_list<d, f>>);

static_assert(
    std::is_same_v<virtual_type<std::shared_ptr<a>, default_policy>, a>);

static_assert(std::is_same_v<
              virtual_traits<virtual_ptr<a>, default_policy>::virtual_type, a>);

static_assert(std::is_same_v<
              select_overrider_virtual_type_aux<
                  virtual_ptr<base>, virtual_ptr<a>, default_policy>::type,
              a>);

static_assert(std::is_same_v<
              overrider_virtual_types<
                  mp_list<virtual_ptr<a>, b, virtual_ptr<c>>,
                  mp_list<virtual_ptr<d>, e, virtual_ptr<f>>, default_policy>,
              mp_list<d, f>>);

static_assert(
    std::is_same_v<
        overrider_virtual_types<
            mp_list<const virtual_ptr<base>&, b, const virtual_ptr<base>&>,
            mp_list<const virtual_ptr<d>&, e, const virtual_ptr<f>&>,
            default_policy>,
        mp_list<d, f>>);

static_assert(
    std::is_same_v<
        overrider_virtual_types<
            mp_list<
                virtual_<std::shared_ptr<a>>, b, virtual_<std::shared_ptr<c>>>,
            mp_list<std::shared_ptr<d>, e, std::shared_ptr<f>>, default_policy>,
        mp_list<d, f>>);

namespace using_polymorphic_classes {

struct Animal {
    virtual ~Animal() {
    }
};

struct Dog : Animal {};

BOOST_OPENMETHOD_CLASSES(Animal, Dog);

namespace BOOST_OPENMETHOD_GENSYM {

BOOST_OPENMETHOD(poke, (const virtual_ptr<Animal>&, std::ostream&), void);

BOOST_OPENMETHOD_OVERRIDE(
    poke, (const virtual_ptr<Dog>&, std::ostream& os), void) {
    os << "bark";
}

static_assert(sizeof(virtual_ptr<Animal>) == 2 * sizeof(void*));

BOOST_AUTO_TEST_CASE(test_virtual_ptr_by_ref) {
    boost::openmethod::initialize();

    {
        boost::test_tools::output_test_stream os;
        Dog dog;
        virtual_ptr<Animal> vptr(dog);
        poke(vptr, os);
    }

    {
        // Using  deduction guide.
        boost::test_tools::output_test_stream os;
        Animal&& animal = Dog();
        auto vptr = virtual_ptr(animal);
        poke(vptr, os);
        BOOST_CHECK(os.is_equal("bark"));
    }

    {
        // Using conversion ctor.
        boost::test_tools::output_test_stream os;
        Animal&& animal = Dog();
        poke(animal, os);
        BOOST_CHECK(os.is_equal("bark"));
    }
}

} // namespace BOOST_OPENMETHOD_GENSYM

namespace BOOST_OPENMETHOD_GENSYM {

BOOST_OPENMETHOD(poke, (virtual_ptr<Animal>, std::ostream&), void);

BOOST_OPENMETHOD_OVERRIDE(poke, (virtual_ptr<Dog>, std::ostream& os), void) {
    os << "bark";
}

BOOST_AUTO_TEST_CASE(test_virtual_shared_by_value) {
    boost::openmethod::initialize();

    {
        boost::test_tools::output_test_stream os;
        shared_virtual_ptr<Animal> animal = make_shared_virtual<Dog>();
        poke(animal, os);
        BOOST_CHECK(os.is_equal("bark"));
    }
}

} // namespace BOOST_OPENMETHOD_GENSYM

namespace BOOST_OPENMETHOD_GENSYM {

BOOST_OPENMETHOD(
    poke, (const shared_virtual_ptr<Animal>&, std::ostream&), void);

BOOST_OPENMETHOD_OVERRIDE(
    poke, (const shared_virtual_ptr<Dog>&, std::ostream& os), void) {
    os << "bark";
}

BOOST_AUTO_TEST_CASE(test_virtual_shared_by_const_reference) {
    boost::openmethod::initialize();

    {
        boost::test_tools::output_test_stream os;
        shared_virtual_ptr<Animal> animal = make_shared_virtual<Dog>();
        poke(animal, os);
        BOOST_CHECK(os.is_equal("bark"));
    }
}

} // namespace BOOST_OPENMETHOD_GENSYM

namespace BOOST_OPENMETHOD_GENSYM {

BOOST_OPENMETHOD(poke, (unique_virtual_ptr<Animal>, std::ostream&), void);

BOOST_OPENMETHOD_OVERRIDE(
    poke, (unique_virtual_ptr<Dog>, std::ostream& os), void) {
    os << "bark";
}

BOOST_AUTO_TEST_CASE(test_virtual_unique) {
    boost::openmethod::initialize();

    {
        boost::test_tools::output_test_stream os;
        poke(make_unique_virtual<Dog>(), os);
        BOOST_CHECK(os.is_equal("bark"));
    }
}

} // namespace BOOST_OPENMETHOD_GENSYM
} // namespace using_polymorphic_classes

namespace using_non_polymorphic_classes {

struct Animal {};

struct Dog : Animal {};

BOOST_OPENMETHOD_CLASSES(Animal, Dog);

BOOST_OPENMETHOD(poke, (virtual_ptr<Animal>, std::ostream&), void);

BOOST_OPENMETHOD_OVERRIDE(poke, (virtual_ptr<Dog>, std::ostream& os), void) {
    os << "bark";
}

BOOST_AUTO_TEST_CASE(test_virtual_ptr_non_polymorphic) {
    boost::openmethod::initialize();

    {
        boost::test_tools::output_test_stream os;
        Dog dog;
        auto vptr = virtual_ptr<Dog>::final(dog);
        poke(vptr, os);
        BOOST_CHECK(os.is_equal("bark"));
    }
}

} // namespace using_non_polymorphic_classes
