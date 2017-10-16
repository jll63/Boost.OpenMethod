// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/openmethod.hpp>
#include <boost/openmethod/compiler.hpp>
#include <boost/openmethod/shared_ptr.hpp>

#include "test_util.hpp"

#define BOOST_TEST_MODULE openmethod
#include <boost/test/included/unit_test.hpp>
#include <boost/utility/identity_type.hpp>

using namespace boost::openmethod;

struct Player {
    virtual ~Player() {
    }
};

struct Warrior : Player {};
struct Wizard : Player {};

struct Bear : Player {};

struct Object {
    virtual ~Object() {
    }
};

struct Axe : Object {};

template<class VirtualBearPtr>
auto poke_bear(VirtualBearPtr) {
    return std::string("growl");
}

template<class VirtualWarriorPtr, class VirtualAxePtr, class VirtualBearPtr>
auto fight_bear(VirtualWarriorPtr, VirtualAxePtr, VirtualBearPtr) {
    return "kill bear";
}

template<int N>
struct indirect_test_policy : test_policy_<N> {};

template<int N>
using policy_types =
    boost::mp11::mp_list<test_policy_<N>, indirect_test_policy<N>>;

struct BOOST_OPENMETHOD_NAME(poke);
struct BOOST_OPENMETHOD_NAME(fight);

namespace BOOST_OPENMETHOD_GENSYM {

BOOST_AUTO_TEST_CASE_TEMPLATE(
    test_virtual_ptr, Policy, policy_types<__COUNTER__>) {

    BOOST_OPENMETHOD_REGISTER(
        use_classes<Player, Warrior, Object, Axe, Bear, Policy>);
    ;
    using poke = method<
        BOOST_OPENMETHOD_NAME(poke)(virtual_ptr<Player, Policy>), std::string,
        Policy>;
    BOOST_OPENMETHOD_REGISTER(
        typename poke::template override<
            poke_bear<virtual_ptr<Player, Policy>>>);

    initialize<Policy>();

    using vptr_player = virtual_ptr<Player, Policy>;
    static_assert(detail::is_virtual_ptr<vptr_player>);
    using vptr_bear = virtual_ptr<Bear, Policy>;

    Player player;
    auto virtual_player = vptr_player::final(player);
    BOOST_TEST(&*virtual_player == &player);
    BOOST_TEST((virtual_player.vptr() == Policy::template static_vptr<Player>));

    Bear bear;
    BOOST_TEST((&*vptr_bear::final(bear)) == &bear);
    BOOST_TEST(
        (vptr_bear::final(bear).vptr() == Policy::template static_vptr<Bear>));

    BOOST_TEST(
        (vptr_player(bear).vptr() == Policy::template static_vptr<Bear>));

    vptr_bear virtual_bear_ptr(bear);
    vptr_player virtual_player_ptr = virtual_bear_ptr;

    struct upcast {
        static void fn(vptr_player) {
        }
    };

    upcast::fn(virtual_bear_ptr);

    auto data = Policy::dispatch_data.data();
    std::fill_n(data, Policy::dispatch_data.size(), 0);

    while (data == Policy::dispatch_data.data()) {
        Policy::dispatch_data.resize(2 * Policy::dispatch_data.size());
    }

    initialize<Policy>();

    BOOST_TEST(
        (virtual_bear_ptr.vptr() == Policy::template static_vptr<Bear>) ==
        Policy::template has_facet<policies::indirect_vptr>);
}
} // namespace BOOST_OPENMETHOD_GENSYM

namespace test_virtual_ptr_dispatch {

BOOST_AUTO_TEST_CASE_TEMPLATE(
    test_virtual_ptr_dispatch, Policy, policy_types<__COUNTER__>) {

    BOOST_OPENMETHOD_REGISTER(
        use_classes<Player, Warrior, Object, Axe, Bear, Policy>);

    using poke = method<
        BOOST_OPENMETHOD_NAME(poke)(virtual_ptr<Player, Policy>), std::string,
        Policy>;
    BOOST_OPENMETHOD_REGISTER(
        typename poke::template override<
            poke_bear<virtual_ptr<Player, Policy>>>);

    using fight = method<
        BOOST_OPENMETHOD_NAME(fight)(
            virtual_ptr<Player, Policy>, virtual_ptr<Object, Policy>,
            virtual_ptr<Player, Policy>),
        std::string, Policy>;
    BOOST_OPENMETHOD_REGISTER(
        typename fight::template override<fight_bear<
            virtual_ptr<Player, Policy>, virtual_ptr<Object, Policy>,
            virtual_ptr<Player, Policy>>>);

    initialize<Policy>();

    Bear bear;
    BOOST_TEST(poke::fn(virtual_ptr<Player, Policy>(bear)) == "growl");

    Warrior warrior;
    Axe axe;
    BOOST_TEST(
        fight::fn(
            virtual_ptr<Player, Policy>(warrior),
            virtual_ptr<Object, Policy>(axe),
            virtual_ptr<Player, Policy>(bear)) == "kill bear");
}

} // namespace test_virtual_ptr_dispatch

namespace test_shared_virtual_ptr_dispatch {

BOOST_AUTO_TEST_CASE_TEMPLATE(
    test_virtual_ptr_dispatch, Policy, policy_types<__COUNTER__>) {

    BOOST_OPENMETHOD_REGISTER(
        use_classes<Player, Warrior, Object, Axe, Bear, Policy>);

    using poke = method<
        BOOST_OPENMETHOD_NAME(poke)(shared_virtual_ptr<Player, Policy>),
        std::string, Policy>;

    BOOST_OPENMETHOD_REGISTER(
        typename poke::template override<
            poke_bear<shared_virtual_ptr<Player, Policy>>>);

    using fight = method<
        BOOST_OPENMETHOD_NAME(fight)(
            shared_virtual_ptr<Player, Policy>,
            shared_virtual_ptr<Object, Policy>,
            shared_virtual_ptr<Player, Policy>),
        std::string, Policy>;

    BOOST_OPENMETHOD_REGISTER(
        typename fight::template override<fight_bear<
            shared_virtual_ptr<Player, Policy>,
            shared_virtual_ptr<Object, Policy>,
            shared_virtual_ptr<Player, Policy>>>);

    initialize<Policy>();

    auto bear = make_shared_virtual<Bear, Policy>();
    auto warrior = make_shared_virtual<Warrior, Policy>();
    auto axe = make_shared_virtual<Axe, Policy>();

    BOOST_TEST(poke::fn(bear) == "growl");

    BOOST_TEST(fight::fn(warrior, axe, bear) == "kill bear");
}

} // namespace test_shared_virtual_ptr_dispatch
