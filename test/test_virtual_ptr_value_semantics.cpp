// qright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or q at http://www.boost.org/LICENSE_1_0.txt)

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

    // -------------------------------------------------------------------------
    // construction and assignment from plain references and pointers

    {
        virtual_ptr<Dog, Policy> p{nullptr};
        BOOST_TEST(p.get() == nullptr);
        BOOST_TEST(p.vptr() == nullptr);
    }

    {
        Dog snoopy;
        virtual_ptr<Dog, Policy> p(snoopy);
        BOOST_TEST(p.get() == &snoopy);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);

        Dog hector;
        p = hector;
        BOOST_TEST(p.get() == &hector);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        Dog snoopy;
        virtual_ptr<Animal, Policy> p(snoopy);
        BOOST_TEST(p.get() == &snoopy);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);

        Cat felix;
        p = felix;
        BOOST_TEST(p.get() == &felix);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Cat>);
    }

    {
        const Dog snoopy;
        virtual_ptr<const Dog, Policy> p(snoopy);
        BOOST_TEST(p.get() == &snoopy);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);

        const Dog hector;
        p = hector;
        BOOST_TEST(p.get() == &hector);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        const Dog snoopy;
        virtual_ptr<const Animal, Policy> p(snoopy);
        BOOST_TEST(p.get() == &snoopy);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);

        const Cat felix;
        p = felix;
        BOOST_TEST(p.get() == &felix);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Cat>);
    }

    {
        Dog snoopy;
        virtual_ptr<Dog, Policy> p(&snoopy);
        BOOST_TEST(p.get() == &snoopy);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);

        Dog hector;
        p = &hector;
        BOOST_TEST(p.get() == &hector);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        Dog snoopy;
        virtual_ptr<Animal, Policy> p(&snoopy);
        BOOST_TEST(p.get() == &snoopy);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);

        Cat felix;
        p = &felix;
        BOOST_TEST(p.get() == &felix);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Cat>);
    }

    {
        const Dog snoopy;
        virtual_ptr<const Dog, Policy> p(&snoopy);
        BOOST_TEST(p.get() == &snoopy);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);

        const Dog hector;
        p = &hector;
        BOOST_TEST(p.get() == &hector);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        const Dog snoopy;
        virtual_ptr<const Animal, Policy> p(&snoopy);
        BOOST_TEST(p.get() == &snoopy);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);

        const Cat felix;
        p = &felix;
        BOOST_TEST(p.get() == &felix);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Cat>);
    }

    // virtual_ptr<Dog, Policy> p{Dog()};
    static_assert(!construct_assign_ok<virtual_ptr<Dog, Policy>, Dog&&>);

    // -------------------------------------------------------------------------
    // construction and assignment from other virtual_ptr

    {
        // virtual_ptr<Dog>(const virtual_ptr<Dog>&)
        Dog snoopy;
        const virtual_ptr<Dog, Policy> p(snoopy);
        virtual_ptr<Dog, Policy> q(p);
        BOOST_TEST(q.get() == &snoopy);
        BOOST_TEST(q.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // virtual_ptr<Dog>(virtual_ptr<Dog>&)
        Dog snoopy;
        virtual_ptr<Dog, Policy> p(snoopy);
        virtual_ptr<Dog, Policy> q(p);
        BOOST_TEST(q.get() == &snoopy);
        BOOST_TEST(q.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // virtual_ptr<Dog>(virtual_ptr<Dog>&&)
        Dog snoopy;
        virtual_ptr<Dog, Policy> p(snoopy);
        virtual_ptr<Dog, Policy> q(std::move(p));
        BOOST_TEST(q.get() == &snoopy);
        BOOST_TEST(q.vptr() == Policy::template static_vptr<Dog>);
        BOOST_TEST(p.get() == &snoopy);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // virtual_ptr<Animal>(const virtual_ptr<Dog>&)
        Dog snoopy;
        const virtual_ptr<Dog, Policy> p(snoopy);
        virtual_ptr<Animal, Policy> base(p);
        BOOST_TEST(base.get() == &snoopy);
        BOOST_TEST(base.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // virtual_ptr<const Dog>(const virtual_ptr<Dog>&)
        Dog snoopy;
        const virtual_ptr<Dog, Policy> p(snoopy);
        virtual_ptr<const Dog, Policy> const_q(p);
        BOOST_TEST(const_q.get() == &snoopy);
        BOOST_TEST(const_q.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // virtual_ptr<const Animal>(const virtual_ptr<Dog>&)
        Dog snoopy;
        const virtual_ptr<Dog, Policy> p(snoopy);
        virtual_ptr<const Animal, Policy> const_base_q(p);
        BOOST_TEST(const_base_q.get() == &snoopy);
        BOOST_TEST(const_base_q.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        // virtual_ptr<Dog>()
        virtual_ptr<Dog, Policy> p{nullptr};
        BOOST_TEST(p.get() == nullptr);
        BOOST_TEST(p.vptr() == nullptr);
    }

    // -------------------------------------------------------------------------
    // assignment

    {
        virtual_ptr<Dog, Policy> p;
        Dog snoopy;
        p = snoopy;
        BOOST_TEST(p.get() == &snoopy);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        virtual_ptr<Dog, Policy> p;
        Dog snoopy;
        p = &snoopy;
        BOOST_TEST(p.get() == &snoopy);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        virtual_ptr<Animal, Policy> p;
        Dog snoopy;
        p = snoopy;
        BOOST_TEST(p.get() == &snoopy);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        virtual_ptr<Animal, Policy> p;
        Dog snoopy;
        p = &snoopy;
        BOOST_TEST(p.get() == &snoopy);
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    }

    {
        Dog snoopy;
        virtual_ptr<Animal, Policy> p(snoopy);
        p = nullptr;
        BOOST_TEST(p.get() == nullptr);
        BOOST_TEST(p.vptr() == nullptr);
    }

    static_assert(!construct_assign_ok<virtual_ptr<Dog, Policy>, const Dog&>);
    static_assert(!construct_assign_ok<virtual_ptr<Dog, Policy>, const Dog*>);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(indirect_virtual_ptr, Policy, test_policies) {
    BOOST_TEST_MESSAGE(
        "Policy = " << boost::core::demangle(typeid(Policy).name()));

    init_test<Policy>();

    Dog snoopy;
    virtual_ptr<Dog, Policy> p(snoopy);

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
