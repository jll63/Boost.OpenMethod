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

template<class T>
T& ref(T& value) {
    return value;
}

BOOST_AUTO_TEST_CASE(test_virtual_ptr_ctors) {
    boost::openmethod::initialize();

    {
        Dog dog;

        {
            auto p = virtual_ptr<Dog>(dog);
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
              decltype(std::declval<virtual_shared_ptr<Dog>>().get()),
              virtual_ptr<Dog>>);

static_assert(std::is_same_v<
              decltype(*std::declval<virtual_shared_ptr<Dog>>()),
              decltype(*std::declval<std::shared_ptr<Dog>>())>);

static_assert(std::is_same_v<
              decltype(std::declval<virtual_shared_ptr<Dog>>()->age),
              decltype(std::declval<std::shared_ptr<Dog>>()->age)>);

BOOST_AUTO_TEST_CASE(test_virtual_shared_ptr) {
    {
        auto dog = std::make_shared<Dog>();

        {
            virtual_shared_ptr<Dog> ptr(dog);
            BOOST_TEST((ptr.get() == final_virtual_ptr(*dog)));
            BOOST_TEST(ptr.get().get() == dog.get());
            BOOST_TEST(ptr._vptr() == policy::static_vptr<Dog>);

            virtual_shared_ptr<Dog> copy(ptr);
            virtual_shared_ptr<Animal> base(ptr);
            virtual_shared_ptr<Dog> downcast = base.cast<Dog>();
            BOOST_TEST(base._vptr() == policy::static_vptr<Dog>);

            virtual_shared_ptr<const Dog> const_copy(ptr);
            virtual_shared_ptr<const Animal> base_const_copy(ptr);

            virtual_shared_ptr<Animal> move_ptr(std::move(ptr));
            BOOST_TEST(ptr.get().get() == nullptr);
            BOOST_TEST(move_ptr.get().get() == dog.get());
        }

        { virtual_shared_ptr<Dog> ptr(std::make_shared<Dog>()); }

        {
            virtual_shared_ptr<Dog> ptr(dog);
            virtual_shared_ptr<const Animal> move_const_ptr(std::move(ptr));
            BOOST_TEST(ptr.get().get() == nullptr);
            BOOST_TEST(move_const_ptr.get().get() == dog.get());
        }
    }

    {
        auto dog = std::make_shared<const Dog>();

        virtual_shared_ptr<const Dog> ptr(dog);
        BOOST_TEST(ptr.get().get() == dog.get());
        BOOST_TEST(ptr._vptr() == policy::static_vptr<Dog>);

        virtual_shared_ptr<const Dog> copy(ptr);
        virtual_shared_ptr<const Animal> base(ptr);
    }
}
