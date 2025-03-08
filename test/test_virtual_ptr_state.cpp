// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <memory>
#include <string>

#define BOOST_TEST_MODULE openmethod
#include <boost/test/included/unit_test.hpp>

#include <boost/core/demangle.hpp>
#include <boost/utility/identity_type.hpp>

#include <boost/openmethod.hpp>
#include <boost/openmethod/policies/vptr_map.hpp>
#include <boost/openmethod/compiler.hpp>
#include <boost/openmethod/shared_ptr.hpp>
#include <boost/openmethod/unique_ptr.hpp>

using namespace boost::openmethod;
using namespace boost::openmethod::policies;

struct Animal {
    virtual ~Animal() {
    }

    Animal() = default;
    Animal(const Animal&) = delete;

    int age;
};

struct Cat : virtual Animal {};

struct Dog : virtual Animal {};

template<class Policy>
void init_test() {
    BOOST_OPENMETHOD_REGISTER(use_classes<Animal, Dog, Policy>);
    struct id;
    (void)&method<id(virtual_ptr<Animal, Policy>), void, Policy>::fn;
    boost::openmethod::initialize<Policy>();
}

struct direct_vector_policy : default_policy::fork<direct_vector_policy> {};

struct indirect_vector_policy
    : default_policy::fork<indirect_vector_policy>::add<indirect_vptr>::replace<
          vptr, vptr_vector<indirect_vector_policy, indirect_vptr>> {};

struct direct_map_policy : default_policy::fork<direct_map_policy>::replace<
                               vptr, vptr_map<direct_map_policy>> {};

struct indirect_map_policy
    : default_policy::fork<indirect_map_policy>::add<indirect_vptr>::replace<
          vptr, vptr_map<indirect_map_policy, indirect_vptr>> {};

using test_policies = boost::mp11::mp_list<
    direct_vector_policy, indirect_vector_policy, direct_map_policy,
    indirect_map_policy>;

BOOST_AUTO_TEST_CASE_TEMPLATE(virtual_ptr_ctors, Policy, test_policies) {
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
    static_assert(!std::is_constructible_v<virtual_ptr<Animal, Policy>, Dog>);

    init_test<Policy>();

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
    }
}

template<
    template<class... Class> class smart_ptr,
    template<class... Class> class other_smart_ptr, class Policy>
struct check_smart_ctors {
    // construction

    // a virtual_ptr can be constructed from a smart_ptr (same class)
    static_assert(std::is_constructible_v<
                  virtual_ptr<smart_ptr<Animal>, Policy>, smart_ptr<Animal>>);

    // a virtual_ptr to const can be constructed from smart_ptr (same class)
    static_assert(
        std::is_constructible_v<
            virtual_ptr<smart_ptr<Animal>, Policy>, const smart_ptr<Animal>&>);

    static_assert(
        std::is_constructible_v<
            virtual_ptr<smart_ptr<const Animal>, Policy>, smart_ptr<Animal>>);

    // a virtual_ptr can be constructed from a smart_ptr (derived class)
    static_assert(std::is_constructible_v<
                  virtual_ptr<smart_ptr<Animal>, Policy>, smart_ptr<Dog>>);

    // a virtual_ptr to const can be constructed from a smart_ptr (derived class)
    static_assert(
        std::is_constructible_v<
            virtual_ptr<smart_ptr<const Animal>, Policy>, smart_ptr<Dog>>);

    // a virtual_ptr cannot be constructed from a smart_ptr to a different class
    static_assert(!std::is_constructible_v<
                  virtual_ptr<smart_ptr<Cat>, Policy>, smart_ptr<Dog>>);

    // a virtual_ptr cannot be constructed from const  smart_ptr
    static_assert(
        !std::is_constructible_v<
            virtual_ptr<smart_ptr<Animal>, Policy>, smart_ptr<const Animal>>);

    // policies must be the same
    static_assert(!std::is_constructible_v<
                  virtual_ptr<smart_ptr<Animal>, policies::debug>,
                  virtual_ptr<smart_ptr<Animal>, policies::release>>);

    // move constructible
    static_assert(
        std::is_move_constructible_v<virtual_ptr<smart_ptr<Animal>, Policy>>);

    // a smart virtual_ptr cannot be constructed from a plain reference or
    // pointer
    static_assert(!std::is_constructible_v<
                  virtual_ptr<smart_ptr<Animal>, Policy>, Animal>);
    static_assert(!std::is_constructible_v<
                  virtual_ptr<smart_ptr<Animal>, Policy>, Animal*>);

    // the smart pointer must be the same (e.g. both shared_ptr)
    static_assert(!std::is_constructible_v<
                  virtual_ptr<smart_ptr<Animal>, Policy>,
                  virtual_ptr<other_smart_ptr<Animal>, Policy>>);

    // a smart virtual_ptr converts to a plain one
    static_assert(std::is_constructible_v<
                  virtual_ptr<Animal, Policy>,
                  virtual_ptr<smart_ptr<Animal>, Policy>>);

    // but not the other way around
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

template struct check_smart_ctors<
    std::shared_ptr, std::unique_ptr, direct_vector_policy>;

template struct check_smart_ctors<
    std::unique_ptr, std::shared_ptr, direct_vector_policy>;

BOOST_AUTO_TEST_CASE_TEMPLATE(shared_virtual_ptr_ctors, Policy, test_policies) {
    {
        init_test<Policy>();

        auto dog = std::make_shared<Dog>();

        {
            shared_virtual_ptr<Dog, Policy> ptr(dog);
            BOOST_TEST((ptr.get() == dog.get()));
            BOOST_TEST(ptr.pointer() == dog);
            BOOST_TEST(ptr.vptr() == Policy::template static_vptr<Dog>);

            shared_virtual_ptr<Dog, Policy> copy(ptr);
            BOOST_TEST((copy.get() == dog.get()));
            BOOST_TEST(copy.pointer() == dog);
            BOOST_TEST(copy.vptr() == Policy::template static_vptr<Dog>);

            shared_virtual_ptr<Animal, Policy> base(ptr);
            BOOST_TEST((base.get() == dog.get()));

            shared_virtual_ptr<Dog, Policy> downcast =
                base.template cast<Dog>();
            BOOST_TEST((downcast.get() == dog.get()));
            BOOST_TEST(base.vptr() == Policy::template static_vptr<Dog>);

            shared_virtual_ptr<const Dog, Policy> const_copy(ptr);
            shared_virtual_ptr<const Animal, Policy> base_const_copy(ptr);

            shared_virtual_ptr<Animal, Policy> move_ptr(std::move(ptr));
            BOOST_TEST(ptr.pointer().get() == nullptr);
            BOOST_TEST(move_ptr.pointer().get() == dog.get());
        }

        {
            shared_virtual_ptr<Dog, Policy> ptr(std::make_shared<Dog>());
            virtual_ptr<Dog, Policy> dumb_vptr(ptr);
            BOOST_TEST(dumb_vptr.get() == ptr.get());
            BOOST_TEST(dumb_vptr.vptr() == ptr.vptr());
        }

        {
            shared_virtual_ptr<Dog, Policy> ptr(std::make_shared<Dog>());
            virtual_ptr<Dog, Policy> dumb_vptr(ptr);
            BOOST_TEST(dumb_vptr.get() == ptr.get());
            BOOST_TEST(dumb_vptr.vptr() == ptr.vptr());
        }

        {
            shared_virtual_ptr<Dog, Policy> ptr(dog);
            shared_virtual_ptr<const Animal, Policy> move_const_ptr(
                std::move(ptr));
            BOOST_TEST(ptr.pointer().get() == nullptr);
            BOOST_TEST(move_const_ptr.pointer().get() == dog.get());
        }

        {
            shared_virtual_ptr<Animal, Policy> ptr(dog);
        }

        {
            // should not compile:
            // unique_virtual_ptr<Dog, Policy> unique_dog(dog);
        }
    }

    {
        auto dog = std::make_shared<const Dog>();

        shared_virtual_ptr<const Dog, Policy> ptr(dog);
        BOOST_TEST(ptr.pointer().get() == dog.get());
        BOOST_TEST(ptr.vptr() == Policy::template static_vptr<Dog>);

        shared_virtual_ptr<const Dog, Policy> copy(ptr);
        shared_virtual_ptr<const Animal, Policy> base(ptr);
    }
}

BOOST_AUTO_TEST_CASE_TEMPLATE(unique_virtual_ptr_ctors, Policy, test_policies) {
    init_test<Policy>();

    {
        // a unique_virtual_ptr can be created from a std::unique_ptr
        std::unique_ptr<Dog> is_smart_ptr = std::make_unique<Dog>();
        auto dumb_ptr = is_smart_ptr.get();
        unique_virtual_ptr<Dog, Policy> virtual_smart_ptr(
            std::move(is_smart_ptr));

        // and ownership is transferred
        BOOST_TEST(is_smart_ptr.get() == nullptr);
        BOOST_TEST(virtual_smart_ptr.get() == dumb_ptr);
    }

    {
        // a virtual_ptr can be constructed from a unique_virtual_ptr
        auto virtual_smart_ptr = make_unique_virtual<Dog, Policy>();
        auto dumb_ptr = virtual_smart_ptr.get();
        virtual_ptr<Dog, Policy> virtual_dumb_ptr = virtual_smart_ptr;
        BOOST_TEST(virtual_dumb_ptr.get() == dumb_ptr);
        // ownership is not transferred
        BOOST_TEST(virtual_smart_ptr.get() == dumb_ptr);
    }

    {
        // unique_virtual_ptr can be moved
        auto unique = make_unique_virtual<Dog, Policy>();
        Dog* dumb_ptr = unique.get();
        unique_virtual_ptr<Dog, Policy> unique_moved = std::move(unique);
        BOOST_TEST(unique.get() == nullptr);
        BOOST_TEST(unique_moved.get() == dumb_ptr);
    }

    {
        unique_virtual_ptr<Animal, Policy> base(std::make_unique<Dog>());
        auto p = base.get();
        unique_virtual_ptr<Dog, Policy> downcast =
            std::move(base).template cast<Dog>();
        BOOST_TEST((downcast.get() == p));
        BOOST_TEST((base.get() == nullptr));
        BOOST_TEST(downcast.vptr() == Policy::template static_vptr<Dog>);
    }
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
