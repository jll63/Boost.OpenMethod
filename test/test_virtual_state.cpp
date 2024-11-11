// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <memory>
#include <string>

#define BOOST_TEST_MODULE openmethod
#include <boost/test/included/unit_test.hpp>

#include <boost/utility/identity_type.hpp>

#include <boost/openmethod.hpp>
#include <boost/openmethod/compiler.hpp>

using namespace boost::openmethod;

struct Animal {
    virtual ~Animal() {
    }

    Animal() = default;
    Animal(const Animal&) = delete;

    int age;
};

struct Dog : virtual Animal {
    Dog() = default;
    Dog(const Dog&) = delete;
};

struct direct_policy : policies::default_::fork<direct_policy> {};

BOOST_OPENMETHOD_CLASSES(Animal, Dog, direct_policy);
BOOST_OPENMETHOD(
    direct_method, (virtual_ptr<Animal, direct_policy>), void, direct_policy);

struct indirect_policy
    : policies::default_::fork<indirect_policy>::replace<
          policies::vptr,
          policies::vptr_vector<indirect_policy, indirect_vptr_type>> {};

BOOST_OPENMETHOD_CLASSES(Animal, Dog, indirect_policy);
BOOST_OPENMETHOD(
    indirect_method, (virtual_ptr<Animal, indirect_policy>), void,
    indirect_policy);

void instantiate_methods() {
    Dog dog;
    direct_method(dog);
    indirect_method(dog);
}

using test_policies = boost::mp11::mp_list<direct_policy, indirect_policy>;

BOOST_AUTO_TEST_CASE_TEMPLATE(virtual_ptr_ctors, Policy, test_policies) {
    static_assert(std::is_same_v<
                  typename virtual_ptr<Animal, Policy>::element_type, Animal>);
    static_assert(std::is_same_v<
                  decltype(std::declval<virtual_ptr<Animal, Policy>>().get()),
                  Animal*>);
    static_assert(
        std::is_same_v<
            decltype(*std::declval<virtual_ptr<Animal, Policy>>()), Animal&>);

    boost::openmethod::initialize<Policy>();

    {
        Dog dog;

        {
            virtual_ptr<Dog, Policy> p(dog);
            BOOST_TEST(p.vptr() != nullptr);
            BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
            virtual_ptr<Dog, Policy> copy(p);
            virtual_ptr<Animal, Policy> base(p);
            virtual_ptr<const Dog, Policy> const_copy(p);
            virtual_ptr<const Animal, Policy> const_base_copy(p);
        }

        {
            const auto p = virtual_ptr<Dog, Policy>(dog);
            virtual_ptr<Dog, Policy> const_copy(p);
            virtual_ptr<Animal, Policy> const_base_copy(p);
        }

        {
            auto p = virtual_ptr<const Animal, Policy>(dog);
        }

// #define BOOST_OPENMETHOD_SHOULD_NOT_COMPILE
// should not compile
#ifdef BOOST_OPENMETHOD_SHOULD_NOT_COMPILE
        {
            auto vptr = virtual_ptr<Dog, Policy>(Dog());
        }
#endif
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(virtual_shared_ptr_ctors, Policy, test_policies) {
    {
        static_assert(std::is_same_v<
                      typename virtual_shared_ptr<Animal, Policy>::element_type,
                      Animal>);
        static_assert(
            std::is_same_v<
                decltype(std::declval<virtual_shared_ptr<Animal, Policy>>()
                             .get()),
                Animal*>);
        static_assert(
            std::is_same_v<
                decltype(*std::declval<virtual_shared_ptr<Animal, Policy>>()),
                Animal&>);
        static_assert(
            std::is_same_v<
                decltype(std::declval<virtual_shared_ptr<Animal, Policy>>()
                             .inferior()),
                const std::shared_ptr<Animal>&>);
        static_assert(
            std::is_same_v<
                decltype(*std::declval<virtual_shared_ptr<Animal, Policy>>()),
                Animal&>);

        boost::openmethod::initialize<Policy>();

        auto dog = std::make_shared<Dog>();

        {
            virtual_shared_ptr<Dog, Policy> ptr(dog);
            BOOST_TEST((ptr.get() == dog.get()));
            BOOST_TEST(ptr.inferior() == dog);
            BOOST_TEST(ptr.vptr() == Policy::template static_vptr<Dog>);

            virtual_shared_ptr<Dog, Policy> copy(ptr);
            BOOST_TEST((copy.get() == dog.get()));
            BOOST_TEST(copy.inferior() == dog);
            BOOST_TEST(copy.vptr() == Policy::template static_vptr<Dog>);

            virtual_shared_ptr<Animal, Policy> base(ptr);
            BOOST_TEST((base.get() == dog.get()));

            virtual_shared_ptr<Dog, Policy> downcast =
                base.template cast<Dog>();
            BOOST_TEST((downcast.get() == dog.get()));
            BOOST_TEST(base.vptr() == Policy::template static_vptr<Dog>);

            virtual_shared_ptr<const Dog, Policy> const_copy(ptr);
            virtual_shared_ptr<const Animal, Policy> base_const_copy(ptr);

            virtual_shared_ptr<Animal, Policy> move_ptr(std::move(ptr));
            BOOST_TEST(ptr.inferior().get() == nullptr);
            BOOST_TEST(move_ptr.inferior().get() == dog.get());
        }

        {
            virtual_shared_ptr<Dog, Policy> ptr(std::make_shared<Dog>());
            virtual_ptr<Dog, Policy> dumb_vptr(ptr);
        }

        {
            virtual_shared_ptr<Dog, Policy> ptr(dog);
            virtual_shared_ptr<const Animal, Policy> move_const_ptr(
                std::move(ptr));
            BOOST_TEST(ptr.inferior().get() == nullptr);
            BOOST_TEST(move_const_ptr.inferior().get() == dog.get());
        }

        {
            virtual_shared_ptr<Animal, Policy> ptr(dog);
        }

        {
            // should not compile:
            // virtual_unique_ptr<Dog, Policy> unique_dog(dog);
        }
    }

    {
        auto dog = std::make_shared<const Dog>();

        virtual_shared_ptr<const Dog, Policy> ptr(dog);
        BOOST_TEST(ptr.inferior().get() == dog.get());
        BOOST_TEST(ptr.vptr() == Policy::template static_vptr<Dog>);

        virtual_shared_ptr<const Dog, Policy> copy(ptr);
        virtual_shared_ptr<const Animal, Policy> base(ptr);
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(virtual_unique_ptr_ctors, Policy, test_policies) {
    boost::openmethod::initialize<Policy>();

    {
        // a virtual_unique_ptr can be created from a std::unique_ptr
        std::unique_ptr<Dog> smart_ptr = std::make_unique<Dog>();
        auto dumb_ptr = smart_ptr.get();
        virtual_unique_ptr<Dog, Policy> virtual_smart_ptr(std::move(smart_ptr));

        // and ownership is transferred
        BOOST_TEST(smart_ptr.get() == nullptr);
        BOOST_TEST(virtual_smart_ptr.get() == dumb_ptr);
    }

    {
        // a virtual_ptr can be constructed from a virtual_unique_ptr
        auto virtual_smart_ptr = make_virtual_unique<Dog, Policy>();
        auto dumb_ptr = virtual_smart_ptr.get();
        virtual_ptr<Dog, Policy> virtual_dumb_ptr = virtual_smart_ptr;
        BOOST_TEST(virtual_dumb_ptr.get() == dumb_ptr);
        // ownership is not transferred
        BOOST_TEST(virtual_smart_ptr.get() == dumb_ptr);
    }

    {
        // virtual_unique_ptr can be moved
        auto unique = make_virtual_unique<Dog, Policy>();
        Dog* dumb_ptr = unique.get();
        virtual_unique_ptr<Dog, Policy> unique_moved = std::move(unique);
        BOOST_TEST(unique.get() == nullptr);
        BOOST_TEST(unique_moved.get() == dumb_ptr);
        virtual_shared_ptr<Dog, Policy> shared_moved = std::move(unique_moved);
        BOOST_TEST(unique_moved.get() == nullptr);
        BOOST_TEST(shared_moved.get() == dumb_ptr);
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(indirect_virtual_ptr, Policy, test_policies) {
    boost::openmethod::initialize<Policy>();

    Dog dog;
    virtual_ptr<Dog, Policy> p(dog);
    BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);

    struct Cat : Animal {};
    BOOST_OPENMETHOD_CLASSES(Animal, Cat, Policy);

    boost::openmethod::initialize<Policy>();

    if constexpr (std::is_same_v<Policy, indirect_policy>) {
        BOOST_TEST(p.vptr() == Policy::template static_vptr<Dog>);
    } else {
        BOOST_TEST(p.vptr() != Policy::template static_vptr<Dog>);
    }
}

BOOST_AUTO_TEST_CASE(virtual_ptr_final_error) {
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
