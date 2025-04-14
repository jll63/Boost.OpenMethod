// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/openmethod/unique_ptr.hpp>

#define BOOST_TEST_MODULE openmethod
#include <boost/test/included/unit_test.hpp>

#include "test_virtual_ptr_value_semantics.hpp"

#include <memory>

BOOST_AUTO_TEST_CASE_TEMPLATE(unique_virtual_ptr_value, Policy, test_policies) {
    init_test<Policy>();

    static_assert(
        std::is_same_v<
            typename unique_virtual_ptr<Animal, Policy>::element_type, Animal>);
    static_assert(
        std::is_same_v<
            decltype(std::declval<unique_virtual_ptr<Animal, Policy>>().get()),
            Animal*>);
    static_assert(unique_virtual_ptr<Animal, Policy>::is_smart_ptr);
    static_assert(unique_virtual_ptr<const Animal, Policy>::is_smart_ptr);
    static_assert(std::is_same_v<
                  decltype(*std::declval<unique_virtual_ptr<Animal, Policy>>()),
                  Animal&>);

    {
        // unique_virtual_ptr<Dog>(nullptr)
        unique_virtual_ptr<Dog, Policy> p{nullptr};
        BOOST_TEST(p.get() == nullptr);
        BOOST_TEST(p.vptr() == nullptr);
    }

    static_assert(!construct_assign_ok<unique_virtual_ptr<Dog, Policy>, Dog>);

    static_assert(!construct_assign_ok<unique_virtual_ptr<Dog, Policy>, Dog&>);

    static_assert(!construct_assign_ok<unique_virtual_ptr<Dog, Policy>, Dog*>);

    static_assert(!construct_assign_ok<
                  unique_virtual_ptr<Dog, Policy>, std::unique_ptr<Dog>&>);

    static_assert(
        !construct_assign_ok<
            unique_virtual_ptr<Dog, Policy>, const std::unique_ptr<Dog>&>);

    static_assert(!construct_assign_ok<
                  unique_virtual_ptr<Dog, Policy>, unique_virtual_ptr<Dog>>);

    static_assert(!construct_assign_ok<
                  unique_virtual_ptr<Dog, Policy>, unique_virtual_ptr<Dog>&>);

    static_assert(
        !construct_assign_ok<
            unique_virtual_ptr<Dog, Policy>, const unique_virtual_ptr<Dog>&>);

    {
        // construct from unique_ptr temporary
        unique_virtual_ptr<Dog, Policy> p(std::make_unique<Dog>());
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // derived-to-base ok?
        unique_virtual_ptr<Animal, Policy> p(std::make_unique<Dog>());
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    }

    static_assert(
        !construct_assign_ok<
            unique_virtual_ptr<Dog, Policy>, const std::unique_ptr<Dog>&>);

    static_assert(!construct_assign_ok<
                  unique_virtual_ptr<Dog, Policy>, std::unique_ptr<Dog>&>);

    static_assert(!construct_assign_ok<
                  unique_virtual_ptr<Dog, Policy>,
                  const unique_virtual_ptr<Dog, Policy>&>);

    static_assert(
        !construct_assign_ok<
            unique_virtual_ptr<Dog, Policy>, unique_virtual_ptr<Dog, Policy>&>);

    static_assert(
        !construct_assign_ok<
            unique_virtual_ptr<Dog, Policy>, unique_virtual_ptr<Dog, Policy>&>);

    {
        // assign from smart ptr temporary
        unique_virtual_ptr<Dog, Policy> p{nullptr};
        p = std::make_unique<Dog>();
        BOOST_TEST(p.get() != nullptr);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // derived-to-base ok?
        unique_virtual_ptr<Animal, Policy> p{nullptr};
        p = std::make_unique<Dog>();
        BOOST_TEST(p.get() != nullptr);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // unique_virtual_ptr<Dog>(unique_virtual_ptr<Dog>)
        unique_virtual_ptr<Dog, Policy> p(std::make_unique<Dog>());
        auto dog = p.get();
        unique_virtual_ptr<Dog, Policy> q(std::move(p));
        BOOST_TEST(q.get() == dog);
        BOOST_TEST(q.vptr() == Policy::template static_vptr<Dog>);
        BOOST_TEST(p.get() == nullptr);
        BOOST_TEST(p.vptr() == nullptr);
    }

    {
        // derived-to-base ok?
        unique_virtual_ptr<Dog, Policy> p(std::make_unique<Dog>());
        auto dog = p.get();
        unique_virtual_ptr<Animal, Policy> q(std::move(p));
        BOOST_TEST(q.get() == dog);
        BOOST_TEST(q.vptr() == Policy::template static_vptr<Dog>);
        BOOST_TEST(p.get() == nullptr);
        BOOST_TEST(p.vptr() == nullptr);
    }

    {
        // virtual_ptr<Dog>(std::unique_ptr<Dog>())
        unique_virtual_ptr<Dog, Policy> p = std::unique_ptr<Dog>();
        BOOST_TEST(p.get() == nullptr);
        BOOST_TEST(p.vptr() == nullptr);
    }

    {
        unique_virtual_ptr<Dog, Policy> p(std::make_unique<Dog>());
        p = nullptr;
        BOOST_TEST(p.get() == nullptr);
        BOOST_TEST(p.vptr() == nullptr);
    }

    {
        unique_virtual_ptr<Dog, Policy> p(std::make_unique<Dog>());
        p = std::unique_ptr<Dog>();
        BOOST_TEST(p.get() == nullptr);
        BOOST_TEST(p.vptr() == nullptr);
    }

#if 0

    {
        // unique_virtual_ptr<Dog> = const std::unique_ptr<Dog>&
        unique_virtual_ptr<Dog, Policy> p;
        const auto s = std::make_unique<Dog>();
        p = s;
        BOOST_TEST(p.get() == s.get());
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // unique_virtual_ptr<Dog> = std::unique_ptr<Dog>&
        unique_virtual_ptr<Dog, Policy> p;
        auto s = std::make_unique<Dog>();
        p = s;
        BOOST_TEST(p.get() == s.get());
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // unique_virtual_ptr<Dog> = std::unique_ptr<Dog>&&
        auto s = std::make_unique<Dog>();
        auto p = s;
        unique_virtual_ptr<Dog, Policy> q;
        q = std::move(p);
        BOOST_TEST(q.get() == s.get());
        BOOST_TEST(q.vptr() == Policy::template static_vptr<Dog>);
        BOOST_TEST(p.get() == nullptr);
    }

    {
        // unique_virtual_ptr<Animal> = std::unique_ptr<Dog>&&
        auto s = std::make_unique<Dog>();
        auto p = s;
        unique_virtual_ptr<Animal, Policy> q;
        q = std::move(p);
        BOOST_TEST(q.get() == s.get());
        BOOST_TEST(q.vptr() == Policy::template static_vptr<Dog>);
        BOOST_TEST(p.get() == nullptr);
    }

    {
        // unique_virtual_ptr<Dog> = unique_virtual_ptr<Dog>&&
        auto s = std::make_unique<Dog>();
        unique_virtual_ptr<Dog, Policy> p(s);
        unique_virtual_ptr<Dog, Policy> q;
        q = std::move(p);
        BOOST_TEST(q.get() == s.get());
        BOOST_TEST(q.vptr() == Policy::template static_vptr<Dog>);
        BOOST_TEST(p.get() == nullptr);
        BOOST_TEST(p.vptr() == nullptr);
    }

    {
        // unique_virtual_ptr<Animal> = unique_virtual_ptr<Dog>&&
        auto s = std::make_unique<Dog>();
        unique_virtual_ptr<Dog, Policy> p(s);
        unique_virtual_ptr<Animal, Policy> q;
        q = std::move(p);
        BOOST_TEST(q.get() == s.get());
        BOOST_TEST(q.vptr() == Policy::template static_vptr<Dog>);
        BOOST_TEST(p.get() == nullptr);
        BOOST_TEST(p.vptr() == nullptr);
    }

    {
        // virtual_ptr<Dog>(unique_virtual_ptr<Dog>&)
        auto p = make_unique_virtual<Dog, Policy>();
        virtual_ptr<Dog, Policy> q(p);
        BOOST_TEST(q.get() == p.get());
        BOOST_TEST(q.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // virtual_ptr<Dog> = unique_virtual_ptr<Dog>&
        const auto p = make_unique_virtual<Dog, Policy>();
        virtual_ptr<Dog, Policy> q;
        q = p;
        BOOST_TEST(q.get() == p.get());
        BOOST_TEST(q.vptr() == Policy::template static_vptr<Dog>);
    }

    // illegal constructions and assignments
    static_assert(
        !construct_assign_ok<unique_virtual_ptr<Dog, Policy>, Dog>);
    static_assert(
        !construct_assign_ok<unique_virtual_ptr<Dog, Policy>, Dog&&>);
    static_assert(
        !construct_assign_ok<unique_virtual_ptr<Dog, Policy>, const Dog&>);
    static_assert(
        !construct_assign_ok<unique_virtual_ptr<Dog, Policy>, const Dog*>);

    static_assert(!std::is_assignable_v<unique_virtual_ptr<Dog, Policy>, Dog>);
    static_assert(
        !std::is_assignable_v<unique_virtual_ptr<Dog, Policy>, Dog&&>);
    static_assert(
        !std::is_assignable_v<unique_virtual_ptr<Dog, Policy>, const Dog&>);
    static_assert(
        !std::is_assignable_v<unique_virtual_ptr<Dog, Policy>, const Dog*>);

#endif
}

template struct check_illegal_smart_ops<
    std::unique_ptr, std::shared_ptr, direct_vector_policy>;
