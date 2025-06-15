// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <boost/openmethod.hpp>
#include <boost/openmethod/registry.hpp>
#include <boost/openmethod/policies/throw_error_handler.hpp>
#include <boost/openmethod/compiler.hpp>
#include <boost/openmethod/unique_ptr.hpp>

#include "test_util.hpp"

#include <sstream>

#define BOOST_TEST_MODULE openmethod
#include <boost/test/included/unit_test.hpp>

using namespace boost::openmethod;
using namespace test_matrices;

struct capture_output : policies::output {
    template<class Registry>
    struct fn {
        inline static std::ostringstream os;
    };
};

namespace not_initialized {

struct registry : test_registry_<__COUNTER__, capture_output> {

    struct output {
        using error_handler = policy<policies::error_handler>;
        output() {
            prev = error_handler::set(
                [this](
                    const policies::default_error_handler::error_variant&
                        error) {
                    prev(error);
                    std::visit([](auto&& arg) { throw arg; }, error);
                });
        }

        ~output() {
            error_handler::set(prev);
            policy<policies::output>::os.clear();
        }

        auto operator()() const {
            return policy<policies::output>::os.str();
        }

        policies::default_error_handler::function_type prev;
    };
};

BOOST_OPENMETHOD(
    transpose, (virtual_ptr<const matrix, registry>), std::string, registry);

BOOST_OPENMETHOD_OVERRIDE(
    transpose, (virtual_ptr<const diagonal_matrix, registry>), std::string) {
    return DIAGONAL;
}

BOOST_AUTO_TEST_CASE(not_initialized) {
    if constexpr (registry::runtime_checks) {
        // throw during virtual_ptr construction, because of hash table lookup
        {
            registry::output output;
            BOOST_CHECK_THROW(
                (unique_virtual_ptr<matrix, registry>{
                    std::make_unique<diagonal_matrix>()}),
                not_initialized_error);
            BOOST_TEST(output() == "not initialized\n");
        }

        // throw during method call
        {
            registry::output output;
            BOOST_CHECK_THROW(
                transpose(make_unique_virtual<diagonal_matrix, registry>()),
                not_initialized_error);
        }
    } else {
        try {
            registry::check_initialized();
        } catch (not_initialized_error) {
            BOOST_TEST_FAIL("should have not thrown in release variant");
        }
    }
}

} // namespace not_initialized
