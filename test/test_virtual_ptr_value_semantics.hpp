// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_VIRTUAL_PTR_VALUE_SEMANTICS_HPP
#define TEST_VIRTUAL_PTR_VALUE_SEMANTICS_HPP

#include <boost/openmethod.hpp>
#include <boost/openmethod/policies/vptr_map.hpp>
#include <boost/openmethod/compiler.hpp>
#include <boost/openmethod/unique_ptr.hpp>

using namespace boost::openmethod;
using namespace boost::openmethod::policies;

struct Animal {
    virtual ~Animal() {
    }

    Animal() = default;
    Animal(const Animal&) = delete;
};

struct Cat : virtual Animal {};

struct Dog : Animal {};

template<class Policy>
void init_test() {
    BOOST_OPENMETHOD_REGISTER(use_classes<Animal, Cat, Dog, Policy>);
    struct id;
    (void)&method<id(virtual_ptr<Animal, Policy>), void, Policy>::fn;
    boost::openmethod::initialize<Policy>();
}

struct direct_vector_policy : default_policy::fork<direct_vector_policy> {};

struct indirect_vector_policy
    : default_policy::fork<indirect_vector_policy>::replace<
          extern_vptr, vptr_vector<indirect_vector_policy, indirect_vptr>> {};

struct direct_map_policy : default_policy::fork<direct_map_policy>::replace<
                               extern_vptr, vptr_map<direct_map_policy>> {};

struct indirect_map_policy
    : default_policy::fork<indirect_map_policy>::replace<
          extern_vptr, vptr_map<indirect_map_policy, indirect_vptr>> {};

using test_policies = boost::mp11::mp_list<
    direct_vector_policy, indirect_vector_policy, direct_map_policy,
    indirect_map_policy>;

template<
    template<class... Class> class smart_ptr,
    template<class... Class> class other_smart_ptr, class Policy>
struct check_illegal_smart_ops {

    // a virtual_ptr cannot be constructed from a smart_ptr to a different class
    static_assert(!std::is_constructible_v<
                  virtual_ptr<smart_ptr<Cat>, Policy>, smart_ptr<Dog>>);

    // a virtual_ptr cannot be constructed from const smart_ptr
    static_assert(
        !std::is_constructible_v<
            virtual_ptr<smart_ptr<Animal>, Policy>, smart_ptr<const Animal>>);

    // policies must be the same
    static_assert(!std::is_constructible_v<
                  virtual_ptr<smart_ptr<Animal>, policies::debug>,
                  virtual_ptr<smart_ptr<Animal>, policies::release>>);

    // a smart virtual_ptr cannot be constructed from a plain reference or
    // pointer
    static_assert(!std::is_constructible_v<
                  virtual_ptr<smart_ptr<Animal>, Policy>, Animal&>);
    static_assert(!std::is_constructible_v<
                  virtual_ptr<smart_ptr<Animal>, Policy>, Animal*>);

    static_assert(!std::is_constructible_v<
                  smart_ptr<Animal>, const other_smart_ptr<Animal>&>);
    // smart_ptr<Animal> p{other_smart_ptr<Animal>()};

    static_assert(!std::is_constructible_v<
                  virtual_ptr<smart_ptr<Animal>, Policy>,
                  virtual_ptr<Animal, Policy>>);

    // ---------------------
    // test other properties

    static_assert(virtual_ptr<smart_ptr<Animal>, Policy>::is_smart_ptr);
    static_assert(virtual_ptr<smart_ptr<const Animal>, Policy>::is_smart_ptr);

    static_assert(std::is_same_v<
                  typename virtual_ptr<smart_ptr<Animal>, Policy>::element_type,
                  Animal>);

    static_assert(
        std::is_same_v<
            decltype(std::declval<virtual_ptr<smart_ptr<Animal>, Policy>>()
                         .get()),
            Animal*>);

    static_assert(
        std::is_same_v<
            decltype(*std::declval<virtual_ptr<smart_ptr<Animal>, Policy>>()),
            Animal&>);

    static_assert(
        std::is_same_v<
            decltype(std::declval<virtual_ptr<smart_ptr<Animal>, Policy>>()
                         .pointer()),
            const smart_ptr<Animal>&>);

    static_assert(
        std::is_same_v<
            decltype(*std::declval<virtual_ptr<smart_ptr<Animal>, Policy>>()),
            Animal&>);
};

template<class Left, class Right>
constexpr bool construct_assign_ok =
    std::is_constructible_v<Left, Right> && std::is_assignable_v<Left, Right>;

#endif // TEST_VIRTUAL_PTR_VALUE_SEMANTICS_HPP
