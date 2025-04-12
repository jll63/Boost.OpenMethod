// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_TEST_MODULE openmethod
#include <boost/test/included/unit_test.hpp>

#include "test_virtual_ptr_value_semantics.hpp"

BOOST_AUTO_TEST_CASE_TEMPLATE(plain_virtual_ptr_value, Policy, test_policies) {
    static_assert(std::is_same_v<
                  typename virtual_ptr<Animal, Policy>::element_type, Animal>);
    static_assert(std::is_same_v<
                  decltype(std::declval<virtual_ptr<Animal, Policy>>().get()),
                  Animal*>);
    static_assert(!virtual_ptr<Animal, Policy>::is_smart_ptr);
    static_assert(!virtual_ptr<const Animal, Policy>::is_smart_ptr);
    static_assert(
        std::is_same_v<
            decltype(*std::declval<virtual_ptr<Animal, Policy>>()), Animal&>);

    init_test<Policy>();

    {
        // virtual_ptr<Dog>(Dog&)
        Dog dog;
        virtual_ptr<Dog, Policy> p(dog);
        BOOST_TEST(p.get() == &dog);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // virtual_ptr<Dog>(virtual_ptr<Dog>&)
        Dog dog;
        virtual_ptr<Dog, Policy> p(dog);
        virtual_ptr<Dog, Policy> copy(p);
        BOOST_TEST(copy.get() == &dog);
        BOOST_TEST(copy.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // virtual_ptr<Dog>(virtual_ptr<Dog>&&)
        Dog dog;
        virtual_ptr<Dog, Policy> p(dog);
        virtual_ptr<Dog, Policy> copy(std::move(p));
        BOOST_TEST(copy.get() == &dog);
        BOOST_TEST(copy.vptr() == Policy::template static_vptr<Dog>);
        BOOST_TEST(p.get() == &dog);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // virtual_ptr<Animal>(const virtual_ptr<Dog>&)
        Dog dog;
        const virtual_ptr<Dog, Policy> p(dog);
        virtual_ptr<Animal, Policy> base(p);
        BOOST_TEST(base.get() == &dog);
        BOOST_TEST(base.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // virtual_ptr<const Dog>(const Dog&)
        const Dog dog;
        const virtual_ptr<const Dog, Policy> p(dog);
        BOOST_TEST(p.get() == &dog);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // virtual_ptr<const Dog>(const Dog*)
        const Dog dog;
        const virtual_ptr<const Dog, Policy> p(&dog);
        BOOST_TEST(p.get() == &dog);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // virtual_ptr<Dog>(nullptr)
        virtual_ptr<Dog, Policy> p{nullptr};
        BOOST_TEST(p.get() == nullptr);
        BOOST_TEST(p.vptr() == nullptr);
    }

    {
        // virtual_ptr<const Dog>(const virtual_ptr<Dog>&)
        Dog dog;
        const virtual_ptr<Dog, Policy> p(dog);
        virtual_ptr<const Dog, Policy> const_copy(p);
        BOOST_TEST(const_copy.get() == &dog);
        BOOST_TEST(const_copy.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // virtual_ptr<const Animal>(const virtual_ptr<Dog>&)
        Dog dog;
        const virtual_ptr<Dog, Policy> p(dog);
        virtual_ptr<const Animal, Policy> const_base_copy(p);
        BOOST_TEST(const_base_copy.get() == &dog);
        BOOST_TEST(const_base_copy.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // virtual_ptr<Dog>()
        virtual_ptr<Dog, Policy> p{nullptr};
        BOOST_TEST(p.get() == nullptr);
        BOOST_TEST(p.vptr() == nullptr);
    }

    {
        // virtual_ptr<Dog> = Dog&
        virtual_ptr<Dog, Policy> p;
        Dog dog;
        p = dog;
        BOOST_TEST(p.get() == &dog);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // virtual_ptr<Dog> = Dog*
        virtual_ptr<Dog, Policy> p;
        Dog dog;
        p = &dog;
        BOOST_TEST(p.get() == &dog);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // virtual_ptr<Animal> = Dog&
        virtual_ptr<Animal, Policy> p;
        Dog dog;
        p = dog;
        BOOST_TEST(p.get() == &dog);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // virtual_ptr<Animal> = Dog*
        virtual_ptr<Animal, Policy> p;
        Dog dog;
        p = &dog;
        BOOST_TEST(p.get() == &dog);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    }

    // illegal constructions and assignments
    static_assert(!construct_assign_ok<virtual_ptr<Dog, Policy>, Dog&&>);
    static_assert(
        !construct_assign_ok<virtual_ptr<Dog, Policy>, const Dog&>);
    static_assert(
        !construct_assign_ok<virtual_ptr<Dog, Policy>, const Dog*>);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(indirect_virtual_ptr, Policy, test_policies) {
    BOOST_TEST_MESSAGE(
        "Policy = " << boost::core::demangle(typeid(Policy).name()));

    init_test<Policy>();

    Dog dog;
    virtual_ptr<Dog, Policy> p(dog);

    BOOST_TEST_MESSAGE("After first call to initialize:");
    BOOST_TEST_MESSAGE("p.vptr() = " << p.vptr());
    BOOST_TEST_MESSAGE(
        "static_vptr<Dog> = " << Policy::template static_vptr<Dog>);
    BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);

    // Add a class, to make sure dispatch data is not re-constructed in the same
    // place with the same values:
    struct Cat : Animal {};
    BOOST_OPENMETHOD_CLASSES(Animal, Cat, Policy);

    init_test<Policy>();

    BOOST_TEST_MESSAGE("After second call to initialize:");
    BOOST_TEST_MESSAGE("p.vptr() = " << p.vptr());
    BOOST_TEST_MESSAGE(
        "static_vptr<Dog> = " << Policy::template static_vptr<Dog>);

    if constexpr (Policy::template has_facet<indirect_vptr>) {
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    } else {
        BOOST_TEST(p.vptr() != Policy::template static_vptr<Dog>);
    }
}

BOOST_AUTO_TEST_CASE(virtual_ptr_final_error) {
    auto prev_handler = default_policy::set_error_handler(
        [](const default_policy::error_variant& ev) {
            if (auto error = std::get_if<type_mismatch_error>(&ev)) {
                static_assert(std::is_same_v<
                              decltype(error), const type_mismatch_error*>);
                throw *error;
            }
        });

    init_test<default_policy>();
    bool threw = false;

    try {
        Dog snoopy;
        Animal& animal = snoopy;
        virtual_ptr<Animal>::final(animal);
    } catch (const type_mismatch_error& error) {
        default_policy::set_error_handler(prev_handler);
        BOOST_TEST(error.type == reinterpret_cast<type_id>(&typeid(Dog)));
        threw = true;
    } catch (...) {
        default_policy::set_error_handler(prev_handler);
        BOOST_FAIL("wrong exception");
        return;
    }

    if constexpr (default_policy::has_facet<runtime_checks>) {
        if (!threw) {
            BOOST_FAIL("should have thrown");
        }
    } else {
        if (threw) {
            BOOST_FAIL("should not have thrown");
        }
    }
}
