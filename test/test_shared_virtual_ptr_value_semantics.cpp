// qright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or q at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/openmethod/shared_ptr.hpp>

#define BOOST_TEST_MODULE openmethod
#include <boost/test/unit_test.hpp>

#include "test_virtual_ptr_value_semantics.hpp"

#include <memory>

static_assert(detail::same_smart_ptr<
              std::shared_ptr<Animal>, std::shared_ptr<Dog>, default_registry>);

static_assert(!detail::same_smart_ptr<
              std::shared_ptr<Animal>, std::unique_ptr<Dog>, default_registry>);

static_assert(!detail::same_smart_ptr<
              std::shared_ptr<Animal>, shared_virtual_ptr<std::unique_ptr<Dog>>,
              default_registry>);

BOOST_AUTO_TEST_CASE_TEMPLATE(
    shared_virtual_ptr_value, Registry, test_policies) {
    static_assert(std::is_same_v<
                  typename shared_virtual_ptr<Animal, Registry>::element_type,
                  Animal>);
    static_assert(std::is_same_v<
                  decltype(std::declval<shared_virtual_ptr<Animal, Registry>>()
                               .get()),
                  Animal*>);
    static_assert(shared_virtual_ptr<Animal, Registry>::is_smart_ptr);
    static_assert(shared_virtual_ptr<const Animal, Registry>::is_smart_ptr);
    static_assert(
        std::is_same_v<
            decltype(*std::declval<shared_virtual_ptr<Animal, Registry>>()),
            Animal&>);

    init_test<Registry>();

    // construction and assignment from a plain pointer or reference is not
    // allowed

    static_assert(!construct_assign_ok<shared_virtual_ptr<Dog, Registry>, Dog>);
    static_assert(
        !construct_assign_ok<shared_virtual_ptr<Dog, Registry>, Dog&&>);
    static_assert(
        !construct_assign_ok<shared_virtual_ptr<Dog, Registry>, const Dog&>);
    static_assert(
        !construct_assign_ok<shared_virtual_ptr<Dog, Registry>, const Dog*>);

    // -------------------------------------------------------------------------
    // construction and assignment from plain references and pointers

    {
        shared_virtual_ptr<Dog, Registry> p{nullptr};
        BOOST_TEST(p.get() == nullptr);
        BOOST_TEST(p.vptr() == nullptr);
    }

    {
        auto snoopy = std::make_shared<Dog>();
        shared_virtual_ptr<Dog, Registry> p(snoopy);
        BOOST_TEST(p.get() == snoopy.get());
        BOOST_TEST(p.vptr() == Registry::template static_vptr<Dog>);

        auto hector = std::make_shared<Dog>();
        p = hector;
        BOOST_TEST(p.get() == hector.get());
        BOOST_TEST(p.vptr() == Registry::template static_vptr<Dog>);
    }

    {
        auto snoopy = std::make_shared<Dog>();
        shared_virtual_ptr<Animal, Registry> p(snoopy);
        BOOST_TEST(p.get() == snoopy.get());
        BOOST_TEST(p.vptr() == Registry::template static_vptr<Dog>);

        auto felix = std::make_shared<Cat>();
        p = felix;
        BOOST_TEST(p.get() == felix.get());
        BOOST_TEST(p.vptr() == Registry::template static_vptr<Cat>);
    }

    {
        auto snoopy = std::make_shared<const Dog>();
        shared_virtual_ptr<const Dog, Registry> p(snoopy);
        BOOST_TEST(p.get() == snoopy.get());
        BOOST_TEST(p.vptr() == Registry::template static_vptr<Dog>);

        auto hector = std::make_shared<const Dog>();
        p = hector;
        BOOST_TEST(p.get() == hector.get());
        BOOST_TEST(p.vptr() == Registry::template static_vptr<Dog>);
    }

    {
        auto snoopy = std::make_shared<const Dog>();
        shared_virtual_ptr<const Animal, Registry> p(snoopy);
        BOOST_TEST(p.get() == snoopy.get());
        BOOST_TEST(p.vptr() == Registry::template static_vptr<Dog>);

        auto felix = std::make_shared<const Cat>();
        p = felix;
        BOOST_TEST(p.get() == felix.get());
        BOOST_TEST(p.vptr() == Registry::template static_vptr<Cat>);
    }

    {
        auto snoopy = std::make_shared<Dog>();
        shared_virtual_ptr<Dog, Registry> p(snoopy);
        BOOST_TEST(p.get() == snoopy.get());
        BOOST_TEST(p.vptr() == Registry::template static_vptr<Dog>);

        auto hector = std::make_shared<Dog>();
        p = hector;
        BOOST_TEST(p.get() == hector.get());
        BOOST_TEST(p.vptr() == Registry::template static_vptr<Dog>);
    }

    {
        auto snoopy = std::make_shared<Dog>();
        shared_virtual_ptr<Animal, Registry> p(snoopy);
        BOOST_TEST(p.get() == snoopy.get());
        BOOST_TEST(p.vptr() == Registry::template static_vptr<Dog>);

        auto felix = std::make_shared<Cat>();
        p = felix;
        BOOST_TEST(p.get() == felix.get());
        BOOST_TEST(p.vptr() == Registry::template static_vptr<Cat>);
    }

    {
        auto snoopy = std::make_shared<const Dog>();
        shared_virtual_ptr<const Dog, Registry> p(snoopy);
        BOOST_TEST(p.get() == snoopy.get());
        BOOST_TEST(p.vptr() == Registry::template static_vptr<Dog>);

        auto hector = std::make_shared<const Dog>();
        p = hector;
        BOOST_TEST(p.get() == hector.get());
        BOOST_TEST(p.vptr() == Registry::template static_vptr<Dog>);
    }

    {
        auto snoopy = std::make_shared<const Dog>();
        shared_virtual_ptr<const Animal, Registry> p(snoopy);
        BOOST_TEST(p.get() == snoopy.get());
        BOOST_TEST(p.vptr() == Registry::template static_vptr<Dog>);

        auto felix = std::make_shared<const Cat>();
        p = felix;
        BOOST_TEST(p.get() == felix.get());
        BOOST_TEST(p.vptr() == Registry::template static_vptr<Cat>);
    }

    // shared_virtual_ptr<Dog, Registry> p{Dog()};
    static_assert(
        !construct_assign_ok<shared_virtual_ptr<Dog, Registry>, Dog&&>);

    // -------------------------------------------------------------------------
    // construction and assignment from other shared_virtual_ptr

    {
        // shared_virtual_ptr<Dog>(const shared_virtual_ptr<Dog>&)
        auto snoopy = std::make_shared<Dog>();
        const shared_virtual_ptr<Dog, Registry> p(snoopy);
        shared_virtual_ptr<Dog, Registry> q(p);
        BOOST_TEST(q.get() == snoopy.get());
        BOOST_TEST(q.vptr() == Registry::template static_vptr<Dog>);
    }

    {
        // shared_virtual_ptr<Dog>(shared_virtual_ptr<Dog>&)
        auto snoopy = std::make_shared<Dog>();
        shared_virtual_ptr<Dog, Registry> p(snoopy);
        shared_virtual_ptr<Dog, Registry> q(p);
        BOOST_TEST(q.get() == snoopy.get());
        BOOST_TEST(q.vptr() == Registry::template static_vptr<Dog>);
    }

    {
        // shared_virtual_ptr<Dog>(shared_virtual_ptr<Dog>&&)
        auto snoopy = std::make_shared<Dog>();
        shared_virtual_ptr<Dog, Registry> p(snoopy);
        shared_virtual_ptr<Dog, Registry> q(std::move(p));
        BOOST_TEST(q.get() == snoopy.get());
        BOOST_TEST(q.vptr() == Registry::template static_vptr<Dog>);
        BOOST_TEST(p.get() == nullptr);
        BOOST_TEST(p.vptr() == nullptr);
    }

    {
        // shared_virtual_ptr<Animal>(const shared_virtual_ptr<Dog>&)
        auto snoopy = std::make_shared<Dog>();
        const shared_virtual_ptr<Dog, Registry> p(snoopy);
        shared_virtual_ptr<Animal, Registry> base(p);
        BOOST_TEST(base.get() == snoopy.get());
        BOOST_TEST(base.vptr() == Registry::template static_vptr<Dog>);
    }

    {
        // shared_virtual_ptr<const Dog>(const shared_virtual_ptr<Dog>&)
        auto snoopy = std::make_shared<Dog>();
        const shared_virtual_ptr<Dog, Registry> p(snoopy);
        shared_virtual_ptr<const Dog, Registry> const_q(p);
        BOOST_TEST(const_q.get() == snoopy.get());
        BOOST_TEST(const_q.vptr() == Registry::template static_vptr<Dog>);
    }

    {
        // shared_virtual_ptr<const Animal>(const shared_virtual_ptr<Dog>&)
        auto snoopy = std::make_shared<Dog>();
        const shared_virtual_ptr<Dog, Registry> p(snoopy);
        shared_virtual_ptr<const Animal, Registry> const_base_q(p);
        BOOST_TEST(const_base_q.get() == snoopy.get());
        BOOST_TEST(const_base_q.vptr() == Registry::template static_vptr<Dog>);
    }

    {
        // shared_virtual_ptr<Dog>()
        shared_virtual_ptr<Dog, Registry> p{nullptr};
        BOOST_TEST(p.get() == nullptr);
        BOOST_TEST(p.vptr() == nullptr);
    }

    {
        shared_virtual_ptr<Dog, Registry> p{std::shared_ptr<Dog>()};
        BOOST_TEST(p.get() == nullptr);
        BOOST_TEST(p.vptr() == nullptr);
    }

    // -------------------------------------------------------------------------
    // assignment

    {
        shared_virtual_ptr<Dog, Registry> p;
        auto snoopy = std::make_shared<Dog>();
        p = snoopy;
        BOOST_TEST(p.get() == snoopy.get());
        BOOST_TEST(p.vptr() == Registry::template static_vptr<Dog>);
    }

    {
        shared_virtual_ptr<Dog, Registry> p;
        auto snoopy = std::make_shared<Dog>();
        p = snoopy;
        BOOST_TEST(p.get() == snoopy.get());
        BOOST_TEST(p.vptr() == Registry::template static_vptr<Dog>);
    }

    {
        auto p = make_shared_virtual<Dog>();
        p = nullptr;
        BOOST_TEST(p.get() == nullptr);
        BOOST_TEST(p.vptr() == nullptr);
    }

    {
        auto p = make_shared_virtual<Dog>();
        p = std::shared_ptr<Dog>();
        BOOST_TEST(p.get() == nullptr);
        BOOST_TEST(p.vptr() == nullptr);
    }

    static_assert(
        !construct_assign_ok<shared_virtual_ptr<Dog, Registry>, const Dog&>);
    static_assert(
        !construct_assign_ok<shared_virtual_ptr<Dog, Registry>, const Dog*>);
}

template struct check_illegal_smart_ops<
    std::shared_ptr, std::unique_ptr, direct_vector>;

// Cannot construct or assign a virtual_ptr from a non-polymorphic object.
static_assert(!construct_assign_ok<
              virtual_ptr<std::shared_ptr<NonPolymorphic>>,
              const std::shared_ptr<NonPolymorphic>&>);
static_assert(!construct_assign_ok<
              virtual_ptr<std::shared_ptr<NonPolymorphic>>,
              std::shared_ptr<NonPolymorphic>&>);
static_assert(!construct_assign_ok<
              virtual_ptr<std::shared_ptr<NonPolymorphic>>,
              std::shared_ptr<NonPolymorphic>&&>);
// OK from another virtual_ptr though, because it can be constructed using
// 'final'.
static_assert(std::is_assignable_v<
              virtual_ptr<std::shared_ptr<NonPolymorphic>>,
              const virtual_ptr<std::shared_ptr<NonPolymorphic>>&>);
static_assert(construct_assign_ok<
              virtual_ptr<std::shared_ptr<NonPolymorphic>>,
              virtual_ptr<std::shared_ptr<NonPolymorphic>>&>);
static_assert(construct_assign_ok<
              virtual_ptr<std::shared_ptr<NonPolymorphic>>,
              virtual_ptr<std::shared_ptr<NonPolymorphic>>&&>);
