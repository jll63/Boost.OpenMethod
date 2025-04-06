// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/openmethod/unique_ptr.hpp>

#define BOOST_TEST_MODULE openmethod
#include <boost/test/included/unit_test.hpp>

#include "test_virtual_ptr_value_semantics.hpp"

#include <memory>

BOOST_AUTO_TEST_CASE_TEMPLATE(
    unique_plain_virtual_ptr_value, Policy, test_policies) {
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

template struct check_illegal_smart_ops<
    std::unique_ptr, std::shared_ptr, direct_vector_policy>;
