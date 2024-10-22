// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/openmethod.hpp>
#include <boost/openmethod/compiler.hpp>
#include <boost/openmethod/virtual_shared_ptr.hpp>

#include <iostream>
#include <memory>
#include <string>

#define BOOST_TEST_MODULE openmethod
#include <boost/test/included/unit_test.hpp>
#include <boost/test/tools/output_test_stream.hpp>

#include "test_util.hpp"

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
        polymorphic_types<types<
            virtual_<std::shared_ptr<a>>, b, virtual_<std::shared_ptr<c>>>>,
        types<std::shared_ptr<a>, std::shared_ptr<c>>>);

static_assert(std::is_same_v<
              spec_polymorphic_types<
                  policies::default_, types<virtual_<a&>, b, virtual_<c&>>,
                  types<d&, e, f&>>,
              types<d, f>>);

static_assert(std::is_same_v<
              polymorphic_type<policies::default_, std::shared_ptr<a>>, a>);

static_assert(
    std::is_same_v<
        spec_polymorphic_types<
            policies::default_,
            types<
                virtual_<std::shared_ptr<a>>, b, virtual_<std::shared_ptr<c>>>,
            types<std::shared_ptr<d>, e, std::shared_ptr<f>>>,
        types<d, f>>);

namespace BOOST_OPENMETHOD_GENSYM {

struct Animal {
    virtual ~Animal() {
    }
};

struct Dog : Animal {};

static_assert(std::is_same_v<virtual_ptr<Animal>::pointer_type, Animal*>);
static_assert(std::is_same_v<virtual_ptr<Animal>::element_type, Animal>);
static_assert(std::is_same_v<
              decltype(std::declval<virtual_ptr<Animal>>().get()), Animal*>);
static_assert(
    std::is_same_v<decltype(*std::declval<virtual_ptr<Animal>>()), Animal&>);

static_assert(
    std::is_same_v<
        virtual_shared_ptr<Animal>::pointer_type, std::shared_ptr<Animal>>);
static_assert(std::is_same_v<virtual_shared_ptr<Animal>::element_type, Animal>);
static_assert(std::is_same_v<
              decltype(std::declval<virtual_shared_ptr<Animal>>().get()),
              std::shared_ptr<Animal>>);
static_assert(std::is_same_v<
              decltype(*std::declval<virtual_shared_ptr<Animal>>()), Animal&>);

BOOST_OPENMETHOD_CLASSES(Animal, Dog);

namespace BOOST_OPENMETHOD_GENSYM {

void poke_dog(virtual_ptr<Dog>, std::ostream& os) {
    os << "bark";
}

struct BOOST_OPENMETHOD_NAME(poke);
using poke = method<
    BOOST_OPENMETHOD_NAME(poke)(virtual_ptr<Animal>, std::ostream&), void>;
BOOST_OPENMETHOD_REGISTER(poke::override<poke_dog>);

BOOST_AUTO_TEST_CASE(test_virtual_ptr_by_ref) {
    boost::openmethod::initialize();

    {
        boost::test_tools::output_test_stream os;
        Dog dog;
        virtual_ptr<Animal> vptr(dog);
        poke::fn(vptr, os);
    }

    {
        // Using  deduction guide.
        boost::test_tools::output_test_stream os;
        Animal&& animal = Dog();
        auto vptr = virtual_ptr(animal);
        poke::fn(vptr, os);
        BOOST_CHECK(os.is_equal("bark"));
    }

    {
        // Using conversion ctor.
        boost::test_tools::output_test_stream os;
        Animal&& animal = Dog();
        poke::fn(animal, os);
        BOOST_CHECK(os.is_equal("bark"));
    }
}

BOOST_AUTO_TEST_CASE(test_final_error) {
    auto prev_handler = policies::default_::set_error_handler(
        [](const policies::default_::error_variant& ev) {
            if (auto error = std::get_if<method_table_error>(&ev)) {
                static_assert(
                    std::is_same_v<decltype(error), const method_table_error*>);
                throw *error;
            }
        });

    boost::openmethod::initialize();
    bool threw = false;

    try {
        Dog snoopy;
        Animal& animal = snoopy;
        virtual_ptr<Animal>::final(animal);
    } catch (const method_table_error& error) {
        policies::default_::set_error_handler(prev_handler);
        BOOST_TEST(error.type == reinterpret_cast<type_id>(&typeid(Dog)));
        threw = true;
    } catch (...) {
        policies::default_::set_error_handler(prev_handler);
        BOOST_FAIL("wrong exception");
        return;
    }

    if constexpr (policies::default_::has_facet<policies::runtime_checks>) {
        if (!threw) {
            BOOST_FAIL("should have thrown");
        }
    } else {
        if (threw) {
            BOOST_FAIL("should not have thrown");
        }
    }
}
} // namespace BOOST_OPENMETHOD_GENSYM

namespace BOOST_OPENMETHOD_GENSYM {

void poke_dog(virtual_shared_ptr<Dog>, std::ostream& os) {
    os << "bark";
}

struct BOOST_OPENMETHOD_NAME(poke);
using poke = method<
    BOOST_OPENMETHOD_NAME(poke)(virtual_shared_ptr<Animal>, std::ostream&),
    void>;
BOOST_OPENMETHOD_REGISTER(poke::override<poke_dog>);

BOOST_AUTO_TEST_CASE(test_virtual_shared_by_value) {
    boost::openmethod::initialize();

    {
        boost::test_tools::output_test_stream os;
        virtual_shared_ptr<Animal> animal = make_virtual_shared<Dog>();
        poke::fn(animal, os);
        BOOST_CHECK(os.is_equal("bark"));
    }
}
} // namespace BOOST_OPENMETHOD_GENSYM

namespace BOOST_OPENMETHOD_GENSYM {

static_assert(
    detail::virtual_ptr_traits<
        const std::shared_ptr<Animal>&, policies::default_>::is_smart_ptr);

void poke_dog(const virtual_shared_ptr<Dog>&, std::ostream& os) {
    os << "bark";
}

struct BOOST_OPENMETHOD_NAME(poke);
using poke = method<
    BOOST_OPENMETHOD_NAME(poke)(
        const virtual_shared_ptr<Animal>&, std::ostream&),
    void>;
BOOST_OPENMETHOD_REGISTER(poke::override<poke_dog>);

BOOST_AUTO_TEST_CASE(test_virtual_shared_by_const_reference) {
    boost::openmethod::initialize();

    {
        boost::test_tools::output_test_stream os;
        virtual_shared_ptr<Animal> animal = make_virtual_shared<Dog>();
        poke::fn(animal, os);
        BOOST_CHECK(os.is_equal("bark"));
    }
}

BOOST_AUTO_TEST_CASE(test_final_virtual_shared) {
    boost::openmethod::initialize();

    auto ptr = make_virtual_shared<Dog>();
    BOOST_TEST(ptr._vptr() == policies::default_::static_vptr<Dog>);
}

} // namespace BOOST_OPENMETHOD_GENSYM
} // namespace BOOST_OPENMETHOD_GENSYM

namespace BOOST_OPENMETHOD_GENSYM {

struct Animal {};

struct Dog : Animal {};

BOOST_OPENMETHOD_CLASSES(Animal, Dog);

void poke_dog(virtual_ptr<Dog>, std::ostream& os) {
    os << "bark";
}

struct BOOST_OPENMETHOD_NAME(poke);
using poke = method<
    BOOST_OPENMETHOD_NAME(poke)(virtual_ptr<Animal>, std::ostream&), void>;
BOOST_OPENMETHOD_REGISTER(poke::override<poke_dog>);

BOOST_AUTO_TEST_CASE(test_virtual_ptr_non_polymorphic) {
    boost::openmethod::initialize();

    {
        boost::test_tools::output_test_stream os;
        Dog dog;
        auto vptr = virtual_ptr<Dog>::final(dog);
        poke::fn(vptr, os);
        BOOST_CHECK(os.is_equal("bark"));
    }
}

} // namespace BOOST_OPENMETHOD_GENSYM
