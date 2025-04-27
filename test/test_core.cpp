// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <type_traits>

#include <boost/openmethod.hpp>
#include <boost/openmethod/shared_ptr.hpp>

#include "test_util.hpp"

#define BOOST_TEST_MODULE core
#include <boost/test/included/unit_test.hpp>

using namespace boost::openmethod;
using namespace boost::openmethod::detail;
namespace mp11 = boost::mp11;

namespace test_virtual {

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
    std::is_same_v<virtual_traits<base&, default_policy>::virtual_type, base>);

static_assert(std::is_same_v<
              virtual_traits<const base&, default_policy>::virtual_type, base>);

static_assert(
    std::is_same_v<virtual_traits<base&&, default_policy>::virtual_type, base>);

static_assert(
    std::is_same_v<virtual_traits<int, default_policy>::virtual_type, void>);

static_assert(std::is_same_v<
              boost::mp11::mp_filter<
                  is_virtual, mp11::mp_list<virtual_<a&>, b, virtual_<c&>>>,
              mp11::mp_list<virtual_<a&>, virtual_<c&>>>);

static_assert(std::is_same_v<remove_virtual<virtual_<a&>>, a&>);

static_assert(std::is_same_v<virtual_type<a&, default_policy>, a>);

static_assert(
    std::is_same_v<
        virtual_types<mp11::mp_list<
            virtual_<std::shared_ptr<a>>, b, virtual_<std::shared_ptr<c>>>>,
        mp11::mp_list<std::shared_ptr<a>, std::shared_ptr<c>>>);

static_assert(std::is_same_v<
              overrider_virtual_types<
                  mp11::mp_list<virtual_<a&>, b, virtual_<c&>>,
                  mp11::mp_list<d&, e, f&>, default_policy>,
              mp11::mp_list<d, f>>);

static_assert(
    std::is_same_v<virtual_type<std::shared_ptr<a>, default_policy>, a>);

static_assert(std::is_same_v<
              virtual_traits<virtual_ptr<a>, default_policy>::virtual_type, a>);

static_assert(std::is_same_v<
              select_overrider_virtual_type_aux<
                  virtual_ptr<base>, virtual_ptr<a>, default_policy>::type,
              a>);

static_assert(
    std::is_same_v<
        overrider_virtual_types<
            mp11::mp_list<virtual_ptr<a>, b, virtual_ptr<c>>,
            mp11::mp_list<virtual_ptr<d>, e, virtual_ptr<f>>, default_policy>,
        mp11::mp_list<d, f>>);

static_assert(
    std::is_same_v<
        overrider_virtual_types<
            mp11::mp_list<
                const virtual_ptr<base>&, b, const virtual_ptr<base>&>,
            mp11::mp_list<const virtual_ptr<d>&, e, const virtual_ptr<f>&>,
            default_policy>,
        mp11::mp_list<d, f>>);

static_assert(
    std::is_same_v<
        overrider_virtual_types<
            mp11::mp_list<
                virtual_<std::shared_ptr<a>>, b, virtual_<std::shared_ptr<c>>>,
            mp11::mp_list<std::shared_ptr<d>, e, std::shared_ptr<f>>,
            default_policy>,
        mp11::mp_list<d, f>>);

static_assert(std::is_same_v<
              boost::mp11::mp_transform<
                  remove_virtual, mp11::mp_list<virtual_<a&>, virtual_<c&>>>,
              mp11::mp_list<a&, c&>>);

static_assert(
    std::is_same_v<
        boost::mp11::mp_transform_q<
            boost::mp11::mp_bind_back<virtual_type, default_policy>,
            boost::mp11::mp_transform<
                remove_virtual, mp11::mp_list<virtual_<a&>, virtual_<c&>>>>,
        mp11::mp_list<a, c>>);

static_assert(
    std::is_same_v<
        boost::mp11::mp_transform_q<
            boost::mp11::mp_bind_back<virtual_type, default_policy>,
            boost::mp11::mp_transform<
                remove_virtual,
                boost::mp11::mp_filter<
                    is_virtual, mp11::mp_list<virtual_<a&>, b, virtual_<c&>>>>>,
        mp11::mp_list<a, c>>);

// clang-format on

static_assert(std::is_same_v<
              virtual_types<mp11::mp_list<virtual_<a&>, b, virtual_<c&>>>,
              mp11::mp_list<a&, c&>>);

static_assert(detail::is_policy<default_policy>);

struct not_a_policy {};
static_assert(!detail::is_policy<not_a_policy>);

BOOST_AUTO_TEST_CASE(test_policy) {
    {
        // test is_policy_compatible
        struct policy1 : default_policy::fork<policy1> {};
        struct policy2 : default_policy::fork<policy2> {};
        static_assert(detail::is_policy_compatible<
                      policy1, virtual_ptr<a, policy1>>::value);
        static_assert(detail::is_policy_compatible<policy1, int>::value);
        static_assert(!detail::is_policy_compatible<
                      policy1, virtual_ptr<a, policy2>>::value);
    }

    {
        // check that forked policy does not share static data with original
        struct policy : default_policy::fork<policy> {};
        BOOST_TEST(&policy::methods != &default_policy::methods);
        BOOST_TEST(&policy::classes != &default_policy::classes);
        BOOST_TEST(
            &policy::static_vptr<void> != &default_policy::static_vptr<void>);
        BOOST_TEST(&policy::dispatch_data != &default_policy::dispatch_data);
    }

    {
        // check that adding a facet keeps static data from original
        struct policy : default_policy::add<policies::indirect_vptr> {};
        BOOST_TEST(&policy::methods == &default_policy::methods);
        BOOST_TEST(&policy::classes == &default_policy::classes);
        BOOST_TEST(
            &policy::static_vptr<void> == &default_policy::static_vptr<void>);
        BOOST_TEST(&policy::dispatch_data == &default_policy::dispatch_data);
    }
}

BOOST_AUTO_TEST_CASE(test_type_id_list) {
    auto iter = type_id_list<mp11::mp_list<a&, b&>, default_policy>::begin;
    auto last = type_id_list<mp11::mp_list<a&, b&>, default_policy>::end;
    BOOST_TEST_REQUIRE(last - iter == 2);
    BOOST_TEST_REQUIRE(*iter++ == type_id(&typeid(a)));
    BOOST_TEST_REQUIRE(*iter++ == type_id(&typeid(b)));
}

} // namespace test_virtual

namespace test_macros {

// Check that macros can handle commas in parameter and return types.

struct Animal {
    virtual ~Animal() = default;
};

BOOST_OPENMETHOD(poke, (virtual_<Animal&>), std::tuple<int, int>);

} // namespace test_macros

namespace casts {

struct Animal {
    virtual ~Animal() {
    }
    int a{1};
};

struct Mammal : virtual Animal {
    int m{2};
};

struct Carnivore : virtual Animal {
    int c{3};
};

struct Dog : Mammal, Carnivore {
    int d{4};
};

auto mammal_this(const Mammal& obj) -> const void* {
    return &obj;
}

auto carnivore_this(const Carnivore& obj) -> const void* {
    return &obj;
}

auto dog_this(const Dog& obj) -> const void* {
    return &obj;
}

BOOST_AUTO_TEST_CASE(casts) {
    Dog dog;
    const Animal& animal = dog;
    const Mammal& mammal = dog;
    const Carnivore& carnivore = dog;

    BOOST_TEST(
        (&virtual_traits<const Animal&, default_policy>::cast<const Mammal&>(
              animal)
              .m) == &dog.m);
    BOOST_TEST(
        (&virtual_traits<const Animal&, default_policy>::cast<const Carnivore&>(
              animal)
              .c) == &dog.c);
    BOOST_TEST(
        (&virtual_traits<const Animal&, default_policy>::cast<const Mammal&>(
              animal)
              .m) == &dog.m);
    BOOST_TEST(
        (&virtual_traits<const Animal&, default_policy>::cast<const Dog&>(
              animal)
              .d) == &dog.d);
    BOOST_TEST(
        (&virtual_traits<const Mammal&, default_policy>::cast<const Dog&>(
              mammal)
              .d) == &dog.d);
    BOOST_TEST(
        (&virtual_traits<const Carnivore&, default_policy>::cast<const Dog&>(
              carnivore)
              .c) == &dog.c);

    using virtual_animal_t = virtual_type<const Animal&, default_policy>;
    static_assert(std::is_same_v<virtual_animal_t, Animal>, "animal");
    using virtual_mammal_t = virtual_type<const Mammal&, default_policy>;
    static_assert(std::is_same_v<virtual_mammal_t, Mammal>, "mammal");
}

} // namespace casts

namespace test_use_classes {

struct Animal {};
struct Dog : public Animal {};
struct Bulldog : public Dog {};
struct Cat : public Animal {};
struct Dolphin : public Animal {};

static_assert(
    std::is_same_v<
        inheritance_map<Animal, Dog, Bulldog, Cat, Dolphin>,
        mp11::mp_list<
            mp11::mp_list<Animal, Animal>, mp11::mp_list<Dog, Animal, Dog>,
            mp11::mp_list<Bulldog, Animal, Dog, Bulldog>,
            mp11::mp_list<Cat, Animal, Cat>,
            mp11::mp_list<Dolphin, Animal, Dolphin>>>);

static_assert(
    std::is_same_v<
        use_classes<Animal, Dog, Bulldog, Cat, Dolphin>::tuple_type,
        std::tuple<
            class_declaration_aux<
                default_policy, mp11::mp_list<Animal, Animal>>,
            class_declaration_aux<
                default_policy, mp11::mp_list<Dog, Animal, Dog>>,
            class_declaration_aux<
                default_policy, mp11::mp_list<Bulldog, Animal, Dog, Bulldog>>,
            class_declaration_aux<
                default_policy, mp11::mp_list<Cat, Animal, Cat>>,
            class_declaration_aux<
                default_policy, mp11::mp_list<Dolphin, Animal, Dolphin>>>>);

} // namespace test_use_classes

namespace facets {

using namespace policies;

struct key1;
struct key2;
struct alt_rtti {};

static_assert(
    std::is_same_v<fork_facet<key2, domain<key1>>::type, domain<key2>>);

struct policy1 : basic_policy<policy1, std_rtti> {};
struct policy2 : policy1::fork<policy2> {};
struct policy3 : policy1::fork<policy3>::replace<std_rtti, alt_rtti> {};

static_assert(std::is_same_v<policy2::facets, mp11::mp_list<std_rtti>>);

static_assert(std::is_same_v<policy3::facets, mp11::mp_list<alt_rtti>>);

} // namespace facets

// -----------------------------------------------------------------------------
// static_slots

namespace test_static_slots {
struct Animal;
}

namespace boost::openmethod::detail {

template<>
struct static_offsets<method<
    void,
    void(
        virtual_<test_static_slots::Animal&>,
        virtual_<test_static_slots::Animal&>)>> {
    static constexpr std::size_t slots[] = {0, 1};
};

} // namespace boost::openmethod::detail

namespace test_static_slots {

struct Animal {
    virtual ~Animal() {
    }
};

using poke = method<void, void(virtual_<Animal&>)>;
static_assert(!has_static_offsets<poke>::value);

using meet = method<void, void(virtual_<Animal&>, virtual_<Animal&>)>;
static_assert(has_static_offsets<meet>::value);

} // namespace test_static_slots
