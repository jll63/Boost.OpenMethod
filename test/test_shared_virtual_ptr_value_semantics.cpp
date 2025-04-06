// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/openmethod/shared_ptr.hpp>

#define BOOST_TEST_MODULE openmethod
#include <boost/test/included/unit_test.hpp>

#include "test_virtual_ptr_value_semantics.hpp"

#include <memory>

BOOST_AUTO_TEST_CASE_TEMPLATE(shared_virtual_ptr_value, Policy, test_policies) {
    static_assert(
        std::is_same_v<
            typename shared_virtual_ptr<Animal, Policy>::element_type, Animal>);
    static_assert(
        std::is_same_v<
            decltype(std::declval<shared_virtual_ptr<Animal, Policy>>().get()),
            Animal*>);
    static_assert(shared_virtual_ptr<Animal, Policy>::is_smart_ptr);
    static_assert(shared_virtual_ptr<const Animal, Policy>::is_smart_ptr);
    static_assert(std::is_same_v<
                  decltype(*std::declval<shared_virtual_ptr<Animal, Policy>>()),
                  Animal&>);

    init_test<Policy>();

    {
        // shared_virtual_ptr<Dog>(std::shared_ptr<Dog>&)
        auto dog = std::make_shared<Dog>();
        shared_virtual_ptr<Dog, Policy> p(dog);
        BOOST_TEST(p.get() == dog.get());
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // shared_virtual_ptr<Dog>(const std::shared_ptr<Dog>&)
        const auto dog = std::make_shared<Dog>();
        shared_virtual_ptr<Dog, Policy> p(dog);
        BOOST_TEST(p.get() == dog.get());
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // shared_virtual_ptr<Animal>(std::shared_ptr<Dog>&&)
        auto s = std::make_shared<Dog>();
        auto p = s;
        shared_virtual_ptr<Animal, Policy> q(std::move(p));
        BOOST_TEST(q.get() == s.get());
        BOOST_TEST(q.vptr() == Policy::template static_vptr<Dog>);
        BOOST_TEST(p.get() == nullptr);
    }

    {
        // shared_virtual_ptr<Dog>(std::shared_ptr<Dog>&&)
        auto s = std::make_shared<Dog>();
        auto p = s;
        shared_virtual_ptr<Dog, Policy> q(std::move(p));
        BOOST_TEST(q.get() == s.get());
        BOOST_TEST(q.vptr() == Policy::template static_vptr<Dog>);
        BOOST_TEST(p.get() == nullptr);
    }

    {
        // shared_virtual_ptr<Dog>(shared_virtual_ptr<Dog>&)
        auto dog = std::make_shared<Dog>();
        shared_virtual_ptr<Dog, Policy> p(dog);
        shared_virtual_ptr<Dog, Policy> copy(p);
        BOOST_TEST(copy.get() == dog.get());
        BOOST_TEST(copy.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // shared_virtual_ptr<Animal>(const shared_virtual_ptr<Dog>&)
        auto dog = std::make_shared<Dog>();
        const shared_virtual_ptr<Dog, Policy> p(dog);
        shared_virtual_ptr<Animal, Policy> base(p);
        BOOST_TEST(base.get() == dog.get());
        BOOST_TEST(base.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // shared_virtual_ptr<const Dog>(std::shared_ptr<const Dog>)
        auto dog = std::make_shared<const Dog>();
        shared_virtual_ptr<const Dog, Policy> p(dog);
        BOOST_TEST(p.get() == dog.get());
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // shared_virtual_ptr<const Dog>(const shared_virtual_ptr<Dog>&)
        auto dog = std::make_shared<Dog>();
        const shared_virtual_ptr<Dog, Policy> p(dog);
        shared_virtual_ptr<const Dog, Policy> const_copy(p);
        BOOST_TEST(const_copy.get() == dog.get());
        BOOST_TEST(const_copy.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // shared_virtual_ptr<const Animal>(const shared_virtual_ptr<Dog>&)
        auto dog = std::make_shared<Dog>();
        const shared_virtual_ptr<Dog, Policy> p(dog);
        shared_virtual_ptr<const Animal, Policy> const_base_copy(p);
        BOOST_TEST(const_base_copy.get() == dog.get());
        BOOST_TEST(const_base_copy.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // shared_virtual_ptr<Dog>(shared_virtual_ptr<Dog>&&)
        auto s = std::make_shared<Dog>();
        shared_virtual_ptr<Dog, Policy> p(s);
        shared_virtual_ptr<Dog, Policy> q(std::move(p));
        BOOST_TEST(q.get() == s.get());
        BOOST_TEST(q.vptr() == Policy::template static_vptr<Dog>);
        BOOST_TEST(p.get() == nullptr);
        BOOST_TEST(p.vptr() == nullptr);
    }

    {
        // shared_virtual_ptr<Animal>(shared_virtual_ptr<Dog>&&)
        auto s = std::make_shared<Dog>();
        shared_virtual_ptr<Dog, Policy> p(s);
        shared_virtual_ptr<Animal, Policy> q(std::move(p));
        BOOST_TEST(q.get() == s.get());
        BOOST_TEST(q.vptr() == Policy::template static_vptr<Dog>);
        BOOST_TEST(p.get() == nullptr);
        BOOST_TEST(p.vptr() == nullptr);
    }

    {
        // shared_virtual_ptr<Dog>()
        shared_virtual_ptr<Dog, Policy> p;
        BOOST_TEST(p.get() == nullptr);
        BOOST_TEST(p.vptr() == nullptr);
    }

    {
        // shared_virtual_ptr<Dog> = std::shared_ptr<Dog>&
        shared_virtual_ptr<Dog, Policy> p;
        auto s = std::make_shared<Dog>();
        p = s;
        BOOST_TEST(p.get() == s.get());
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // shared_virtual_ptr<Dog> = const std::shared_ptr<Dog>&
        shared_virtual_ptr<Dog, Policy> p;
        const auto s = std::make_shared<Dog>();
        p = s;
        BOOST_TEST(p.get() == s.get());
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // shared_virtual_ptr<Dog> = std::shared_ptr<Dog>&
        shared_virtual_ptr<Dog, Policy> p;
        auto s = std::make_shared<Dog>();
        p = s;
        BOOST_TEST(p.get() == s.get());
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // shared_virtual_ptr<Dog> = std::shared_ptr<Dog>&&
        auto s = std::make_shared<Dog>();
        auto p = s;
        shared_virtual_ptr<Dog, Policy> q;
        q = std::move(p);
        BOOST_TEST(q.get() == s.get());
        BOOST_TEST(q.vptr() == Policy::template static_vptr<Dog>);
        BOOST_TEST(p.get() == nullptr);
    }

    {
        // shared_virtual_ptr<Animal> = std::shared_ptr<Dog>&&
        auto s = std::make_shared<Dog>();
        auto p = s;
        shared_virtual_ptr<Animal, Policy> q;
        q = std::move(p);
        BOOST_TEST(q.get() == s.get());
        BOOST_TEST(q.vptr() == Policy::template static_vptr<Dog>);
        BOOST_TEST(p.get() == nullptr);
    }

    {
        // shared_virtual_ptr<Dog> = shared_virtual_ptr<Dog>&&
        auto s = std::make_shared<Dog>();
        shared_virtual_ptr<Dog, Policy> p(s);
        shared_virtual_ptr<Dog, Policy> q;
        q = std::move(p);
        BOOST_TEST(q.get() == s.get());
        BOOST_TEST(q.vptr() == Policy::template static_vptr<Dog>);
        BOOST_TEST(p.get() == nullptr);
        BOOST_TEST(p.vptr() == nullptr);
    }

    {
        // shared_virtual_ptr<Animal> = shared_virtual_ptr<Dog>&&
        auto s = std::make_shared<Dog>();
        shared_virtual_ptr<Dog, Policy> p(s);
        shared_virtual_ptr<Animal, Policy> q;
        q = std::move(p);
        BOOST_TEST(q.get() == s.get());
        BOOST_TEST(q.vptr() == Policy::template static_vptr<Dog>);
        BOOST_TEST(p.get() == nullptr);
        BOOST_TEST(p.vptr() == nullptr);
    }

    {
        // virtual_ptr<Dog>(shared_virtual_ptr<Dog>&)
        auto p = make_shared_virtual<Dog, Policy>();
        virtual_ptr<Dog, Policy> q(p);
        BOOST_TEST(q.get() == p.get());
        BOOST_TEST(q.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // virtual_ptr<Dog> = shared_virtual_ptr<Dog>&
        const auto p = make_shared_virtual<Dog, Policy>();
        virtual_ptr<Dog, Policy> q;
        q = p;
        BOOST_TEST(q.get() == p.get());
        BOOST_TEST(q.vptr() == Policy::template static_vptr<Dog>);
    }

    // illegal constructions and assignments
    static_assert(
        !std::is_constructible_v<shared_virtual_ptr<Dog, Policy>, Dog>);
    static_assert(
        !std::is_constructible_v<shared_virtual_ptr<Dog, Policy>, Dog&&>);
    static_assert(
        !std::is_constructible_v<shared_virtual_ptr<Dog, Policy>, const Dog&>);
    static_assert(
        !std::is_constructible_v<shared_virtual_ptr<Dog, Policy>, const Dog*>);

    static_assert(!std::is_assignable_v<shared_virtual_ptr<Dog, Policy>, Dog>);
    static_assert(
        !std::is_assignable_v<shared_virtual_ptr<Dog, Policy>, Dog&&>);
    static_assert(
        !std::is_assignable_v<shared_virtual_ptr<Dog, Policy>, const Dog&>);
    static_assert(
        !std::is_assignable_v<shared_virtual_ptr<Dog, Policy>, const Dog*>);
}

template struct check_illegal_smart_ops<
    std::shared_ptr, std::unique_ptr, direct_vector_policy>;
