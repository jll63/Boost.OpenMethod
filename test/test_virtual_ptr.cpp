// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <memory>
#include <string>

#define BOOST_TEST_MODULE openmethod
#include <boost/test/included/unit_test.hpp>

#include <boost/openmethod.hpp>
#include <boost/openmethod/compiler.hpp>
#include <boost/openmethod/virtual_shared_ptr.hpp>
#include <boost/openmethod/virtual_unique_ptr.hpp>

struct Animal {
    virtual ~Animal() {
    }

    Animal() = default;
    Animal(const Animal&) = delete;

    int age;
};

using std::cout;
using namespace boost::openmethod;
using policy = policies::default_;

struct Dog : virtual Animal {
    Dog() = default;
    Dog(const Dog&) = delete;
};

BOOST_OPENMETHOD_CLASSES(Animal, Dog);

BOOST_AUTO_TEST_CASE(test_virtual_ptr_ctors) {
    boost::openmethod::initialize();

    using policy = policies::default_;

    {
        Dog dog;

        {
            auto p = virtual_ptr<Dog>(dog);
            BOOST_TEST(p.vptr() == policy::static_vptr<Dog>);
            virtual_ptr<Dog> copy(p);
            virtual_ptr<Animal> base(p);
            virtual_ptr<const Dog> const_copy(p);
            virtual_ptr<const Animal> const_base_copy(p);
        }

        {
            const auto p = virtual_ptr<Dog>(dog);
            virtual_ptr<Dog> const_copy(p);
            virtual_ptr<Animal> const_base_copy(p);
        }

        { auto p = virtual_ptr<const Animal>(dog); }

// #define BOOST_OPENMETHOD_SHOULD_NOT_COMPILE
// should not compile
#ifdef BOOST_OPENMETHOD_SHOULD_NOT_COMPILE
        { auto vptr = virtual_ptr<Dog>(Dog()); }
#endif
    }
}

static_assert(std::is_same_v<
              decltype(std::declval<virtual_shared_ptr<Dog>>().inferior()),
              const std::shared_ptr<Dog>&>);

static_assert(
    std::is_same_v<decltype(*std::declval<virtual_shared_ptr<Dog>>()), Dog&>);

static_assert(std::is_same_v<
              decltype(std::declval<virtual_shared_ptr<Dog>>()->age),
              decltype(std::declval<std::shared_ptr<Dog>>()->age)>);

BOOST_AUTO_TEST_CASE(test_virtual_shared_ptr) {
    {
        boost::openmethod::initialize();

        auto dog = std::make_shared<Dog>();

        {
            virtual_shared_ptr<Dog> ptr(dog);
            BOOST_TEST((ptr.get() == dog.get()));
            BOOST_TEST(ptr.inferior() == dog);
            BOOST_TEST(ptr.vptr() == policy::static_vptr<Dog>);

            virtual_shared_ptr<Dog> copy(ptr);
            BOOST_TEST((copy.get() == dog.get()));
            BOOST_TEST(copy.inferior() == dog);
            BOOST_TEST(copy.vptr() == policy::static_vptr<Dog>);

            virtual_shared_ptr<Animal> base(ptr);
            BOOST_TEST((base.get() == dog.get()));

            virtual_shared_ptr<Dog> downcast = base.cast<Dog>();
            BOOST_TEST((downcast.get() == dog.get()));
            BOOST_TEST(base.vptr() == policy::static_vptr<Dog>);

            virtual_shared_ptr<const Dog> const_copy(ptr);
            virtual_shared_ptr<const Animal> base_const_copy(ptr);

            virtual_shared_ptr<Animal> move_ptr(std::move(ptr));
            BOOST_TEST(ptr.inferior().get() == nullptr);
            BOOST_TEST(move_ptr.inferior().get() == dog.get());
        }

        {
            virtual_shared_ptr<Dog> ptr(std::make_shared<Dog>());
            virtual_ptr<Dog> dumb_vptr(ptr);
        }

        {
            virtual_shared_ptr<Dog> ptr(dog);
            virtual_shared_ptr<const Animal> move_const_ptr(std::move(ptr));
            BOOST_TEST(ptr.inferior().get() == nullptr);
            BOOST_TEST(move_const_ptr.inferior().get() == dog.get());
        }

        { virtual_shared_ptr<Animal> ptr(dog); }

        {
            // does not compile:
            // virtual_unique_ptr<Dog> unique_dog(dog);
        }
    }

    {
        auto dog = std::make_shared<const Dog>();

        virtual_shared_ptr<const Dog> ptr(dog);
        BOOST_TEST(ptr.inferior().get() == dog.get());
        BOOST_TEST(ptr.vptr() == policy::static_vptr<Dog>);

        virtual_shared_ptr<const Dog> copy(ptr);
        virtual_shared_ptr<const Animal> base(ptr);
    }
}

BOOST_AUTO_TEST_CASE(test_virtual_unique_ptr) {
    boost::openmethod::initialize();

    {
        // a virtual_unique_ptr can be created from a std::unique_ptr
        std::unique_ptr<Dog> smart_ptr = std::make_unique<Dog>();
        auto dumb_ptr = smart_ptr.get();
        virtual_unique_ptr<Dog> virtual_smart_ptr(std::move(smart_ptr));

        // and ownership is transferred
        BOOST_TEST(smart_ptr.get() == nullptr);
        BOOST_TEST(virtual_smart_ptr.get() == dumb_ptr);
    }

    {
        // a virtual_ptr can be constructed from a virtual_unique_ptr
        auto virtual_smart_ptr = make_virtual_unique<Dog>();
        auto dumb_ptr = virtual_smart_ptr.get();
        virtual_ptr<Dog> virtual_dumb_ptr = virtual_smart_ptr;
        BOOST_TEST(virtual_dumb_ptr.get() == dumb_ptr);
        // ownership is not transferred
        BOOST_TEST(virtual_smart_ptr.get() == dumb_ptr);
    }

    {
        // virtual_unique_ptr can be moved
        auto ptr1 = make_virtual_unique<Dog>();
        auto dumb_ptr = ptr1.get();
        auto ptr2 = std::move(ptr1);
        BOOST_TEST(ptr1.get() == nullptr);
        BOOST_TEST(ptr2.get() == dumb_ptr);
    }
}
