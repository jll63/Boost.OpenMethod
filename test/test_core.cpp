// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <type_traits>

#include <boost/openmethod.hpp>

#include "test_util.hpp"

#define BOOST_TEST_MODULE core
#include <boost/test/included/unit_test.hpp>

using namespace boost::openmethod;
using namespace boost::openmethod::detail;

// clang-format off

namespace test_virtual {

struct base {
    virtual ~base() {}
};

struct a : base {};
struct b : base {};
struct c : base {};
struct d : base {};
struct e : base {};
struct f : base {};

static_assert(
    std::is_same_v<
        virtual_traits<policies::default_, base&>::polymorphic_type, base>);

static_assert(
    std::is_same_v<
        virtual_traits<policies::default_, const base&>::polymorphic_type, base>);

static_assert(
    std::is_same_v<
        virtual_traits<policies::default_, base*>::polymorphic_type, base>);

static_assert(
    std::is_same_v<
        virtual_traits<policies::default_, const base*>::polymorphic_type, base>);

static_assert(
    std::is_same_v<
        virtual_traits<policies::default_, base&&>::polymorphic_type, base>);

static_assert(
    std::is_same_v<
        virtual_traits<policies::default_, int>::polymorphic_type, void>);

static_assert(
    std::is_same_v<
        boost::mp11::mp_filter<
            is_virtual,
            types< virtual_<a&>, b, virtual_<c&> >
        >,
        types< virtual_<a&>, virtual_<c&> >
    >);

static_assert(
    std::is_same_v<
        remove_virtual<virtual_<a&>>,
        a&
    >);

static_assert(
    std::is_same_v<
        polymorphic_type<policies::default_, a&>,
        a
    >);

static_assert(
    std::is_same_v<
        boost::mp11::mp_transform<
            remove_virtual,
            types< virtual_<a&>, virtual_<c&> >
        >,
        types<a&, c&>
    >);

static_assert(
    std::is_same_v<
        boost::mp11::mp_transform_q<
            boost::mp11::mp_bind_front<polymorphic_type, policies::default_>,
            boost::mp11::mp_transform<
                remove_virtual,
                types< virtual_<a&>, virtual_<c&> >
            >
        >,
        types<a, c>
    >);

static_assert(
    std::is_same_v<
        boost::mp11::mp_transform_q<
            boost::mp11::mp_bind_front<polymorphic_type, policies::default_>,
            boost::mp11::mp_transform<
                remove_virtual,
                boost::mp11::mp_filter<
                    is_virtual,
                    types< virtual_<a&>, b, virtual_<c&> >
                >
            >
        >,
        types<a, c>
    >);

static_assert(
    std::is_same_v<
        polymorphic_types<types<virtual_<a&>, b, virtual_<c&>>>,
        types<a&, c&>>);

BOOST_AUTO_TEST_CASE(test_type_id_list) {
    type_id expected[] = {type_id(&typeid(a)), type_id(&typeid(b))};
    auto iter = type_id_list<policies::default_, types<a&, b&>>::begin;
    auto last = type_id_list<policies::default_, types<a&, b&>>::end;
    BOOST_TEST_REQUIRE(last - iter == 2);
    BOOST_TEST_REQUIRE(*iter++ == type_id(&typeid(a)));
    BOOST_TEST_REQUIRE(*iter++ == type_id(&typeid(b)));
}

} // namespace BOOST_OPENMETHOD_GENSYM

namespace casts {

struct Animal {
    virtual ~Animal() {}
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

const void* mammal_this(const Mammal& obj) {
    return &obj;
}

const void* carnivore_this(const Carnivore& obj) {
    return &obj;
}

const void* dog_this(const Dog& obj) {
    return &obj;
}

BOOST_AUTO_TEST_CASE(casts) {
    Dog dog;
    const Animal& animal = dog;
    const Mammal& mammal = dog;
    const Carnivore& carnivore = dog;

    BOOST_TEST(
        (&virtual_traits<policies::default_, const Animal&>::cast<const Mammal&>(animal).m)
        == &dog.m);
    BOOST_TEST(
        (&virtual_traits<policies::default_, const Animal&>::cast<const Carnivore&>(animal).c)
        == &dog.c);
    BOOST_TEST(
        (&virtual_traits<policies::default_, const Animal&>::cast<const Mammal&>(animal).m)
        == &dog.m);
    BOOST_TEST(
        (&virtual_traits<policies::default_, const Animal&>::cast<const Dog&>(animal).d)
        == &dog.d);
    BOOST_TEST(
        (&virtual_traits<policies::default_, const Mammal&>::cast<const Dog&>(mammal).d)
        == &dog.d);
    BOOST_TEST(
        (&virtual_traits<policies::default_, const Carnivore&>::cast<const Dog&>(carnivore).c)
        == &dog.c);

    using voidp = const void*;
    using virtual_animal_t = polymorphic_type<policies::default_, const Animal&>;
    static_assert(std::is_same_v<virtual_animal_t, Animal>, "animal");
    using virtual_mammal_t = polymorphic_type<policies::default_, const Mammal&>;
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
        types<
            types<Animal, Animal>,
            types<Dog, Animal, Dog>,
            types<Bulldog, Animal, Dog, Bulldog>,
            types<Cat, Animal, Cat>,
            types<Dolphin, Animal, Dolphin>
        >
>);

static_assert(
    std::is_same_v<
        use_classes<Animal, Dog, Bulldog, Cat, Dolphin>,
        std::tuple<
            class_declaration_aux<policies::default_, types<Animal, Animal>>,
            class_declaration_aux<policies::default_, types<Dog, Animal, Dog>>,
            class_declaration_aux<policies::default_, types<Bulldog, Animal, Dog, Bulldog>>,
            class_declaration_aux<policies::default_, types<Cat, Animal, Cat>>,
            class_declaration_aux<policies::default_, types<Dolphin, Animal, Dolphin>>
        >
>);

struct my_policy : policies::abstract_policy {};

static_assert(
    std::is_same_v<
        use_classes<Animal, Dog>,
        use_classes_aux<policies::default_, types<Animal, Dog>>::type
>);

static_assert(
    std::is_same_v<
        use_classes<Animal, Dog, my_policy, policies::default_>,
        use_classes_aux<my_policy, types<Animal, Dog>>::type
    >);

} // namespace test_use_classes

namespace facets {

using namespace policies;

struct key1;
struct key2;
struct alt_rtti {};

static_assert(std::is_same_v<
    rebind_facet<key2, basic_domain<key1>>::type,
    basic_domain<key2>
>);

// boost::openmethod::policies::basic_policy<facets::key2, boost::openmethod::policies::std_rtti>,
// boost::openmethod::policies::basic_policy<boost::openmethod::policies::basic_domain<facets::key2>, boost::openmethod::policies::std_rtti>

struct policy1 : basic_policy<policy1, std_rtti> {};
struct policy2 : policy1::fork<policy2> {};
struct policy3 : policy1::fork<policy3>::replace<std_rtti, alt_rtti> {};

static_assert(std::is_same_v<
    policy2::facets,
    types<std_rtti>
>);

static_assert(std::is_same_v<
    policy3::facets,
    types<alt_rtti>
>);

// static_assert(std::is_same_v<
//     basic_policy<basic_domain<key1>, std_rtti>::replace<std_rtti, alt_rtti>,
//     basic_policy<basic_domain<key1>, alt_rtti>
// >);

}

// -----------------------------------------------------------------------------
// static_slots

namespace test_static_slots {
struct Animal;
}

namespace boost {
namespace openmethod {
namespace detail {

template<>
struct static_offsets<
    method<
        void,
        void (
            virtual_<test_static_slots::Animal&>,
            virtual_<test_static_slots::Animal&>)>
> {
    static constexpr std::size_t slots[] = { 0, 1 };
};

}
}
}

namespace test_static_slots {

struct Animal {
    virtual ~Animal() {}
};

using poke = method<void, void (virtual_<Animal&>)>;
static_assert(!has_static_offsets<poke>::value);

using meet = method<void, void (virtual_<Animal&>, virtual_<Animal&>)>;
static_assert(has_static_offsets<meet>::value);

}
