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
#include <boost/test/unit_test.hpp>

using namespace boost::openmethod;
using namespace test_matrices;

struct capture_output : policies::output {
    template<class Registry>
    struct fn {
        inline static std::ostringstream os;
    };
};

template<int N>
struct errors_ : test_registry_<N, capture_output> {
    struct capture {
        using error_handler =
            typename test_registry_<N, capture_output>::error_handler;
        using output = typename test_registry_<N, capture_output>::output;

        capture() {
            prev = error_handler::set(
                [this](
                    const policies::default_error_handler::error_variant&
                        error) {
                    prev(error);
                    std::visit([](auto&& arg) { throw arg; }, error);
                });
        }

        ~capture() {
            error_handler::set(prev);
            output::os.clear();
        }

        auto operator()() const {
            return output::os.str();
        }

        policies::default_error_handler::function_type prev;
    };
};

namespace TEST_NS {

using registry = errors_<__COUNTER__>;

BOOST_OPENMETHOD(
    transpose, (virtual_ptr<const matrix, registry>), void, registry);

BOOST_OPENMETHOD_OVERRIDE(
    transpose, (virtual_ptr<const diagonal_matrix, registry>), void) {
}

BOOST_AUTO_TEST_CASE(not_initialized) {
    if constexpr (registry::runtime_checks) {
        // throw during virtual_ptr construction, because of hash table lookup
        {
            registry::capture capture;
            BOOST_CHECK_THROW(
                (unique_virtual_ptr<matrix, registry>{
                    std::make_unique<diagonal_matrix>()}),
                not_initialized_error);
            BOOST_TEST(capture() == "not initialized\n");
        }

        // throw during method call
        {
            registry::capture capture;
            BOOST_CHECK_THROW(
                transpose(make_unique_virtual<diagonal_matrix, registry>()),
                not_initialized_error);
        }
    } else {
        try {
            registry::check_initialized();
        } catch (not_initialized_error&) {
            BOOST_TEST_FAIL("should have not thrown in release variant");
        }
    }
}

} // namespace TEST_NS

namespace TEST_NS {

using registry = errors_<__COUNTER__>;

BOOST_OPENMETHOD(
    transpose, (virtual_ptr<const matrix, registry>), void, registry);

// without any overrider initialize() would do nothing
BOOST_OPENMETHOD_OVERRIDE(
    transpose, (virtual_ptr<const matrix, registry>), void) {
}

BOOST_AUTO_TEST_CASE(initialize_unknown_class) {
    if constexpr (registry::runtime_checks) {
        {
            registry::capture capture;
            BOOST_CHECK_THROW(initialize<registry>(), unknown_class_error);
            BOOST_TEST(capture().find("unknown class") != std::string::npos);
        }
    }
}

} // namespace TEST_NS

namespace TEST_NS {

using registry = errors_<__COUNTER__>;

// don't register dense_matrix
BOOST_OPENMETHOD_CLASSES(matrix, diagonal_matrix, registry);

BOOST_OPENMETHOD(transpose, (virtual_<const matrix&>), void, registry);

// without any overrider initialize() would do nothing
BOOST_OPENMETHOD_OVERRIDE(transpose, (const matrix&), void) {
}

BOOST_AUTO_TEST_CASE(call_unknown_class) {
    if constexpr (registry::runtime_checks) {
        {
            initialize<registry>();

            registry::capture capture;
            BOOST_CHECK_THROW(transpose(dense_matrix()), unknown_class_error);
            BOOST_TEST(capture().find("unknown class") != std::string::npos);
        }
    }
}

} // namespace TEST_NS

namespace TEST_NS {

using namespace test_matrices;
using registry = errors_<__COUNTER__>;

BOOST_OPENMETHOD_CLASSES(matrix, dense_matrix, diagonal_matrix, registry);

BOOST_OPENMETHOD(
    times, (virtual_<const matrix&>, virtual_<const matrix&>), void, registry);

BOOST_OPENMETHOD_OVERRIDE(
    times, (const matrix&, const diagonal_matrix&), void) {
}

BOOST_OPENMETHOD_OVERRIDE(
    times, (const diagonal_matrix&, const matrix&), void) {
}

BOOST_AUTO_TEST_CASE(call_error) {
    auto report = initialize<registry>().report;
    BOOST_TEST(report.not_implemented == 1u);
    BOOST_TEST(report.ambiguous == 1u);

    {
        registry::capture capture;
        BOOST_CHECK_THROW(times(matrix(), matrix()), not_implemented_error);
        BOOST_TEST(capture().find("not implemented") != std::string::npos);
    }

    {
        registry::capture capture;
        BOOST_CHECK_THROW(
            times(diagonal_matrix(), diagonal_matrix()), ambiguous_error);
        BOOST_TEST(capture().find("ambiguous") != std::string::npos);
    }
}

} // namespace TEST_NS

namespace TEST_NS {

using namespace test_matrices;
struct registry
    : test_registry_<__COUNTER__>::with<policies::throw_error_handler> {};

BOOST_OPENMETHOD_CLASSES(matrix, dense_matrix, diagonal_matrix, registry);

BOOST_OPENMETHOD(
    times, (virtual_<const matrix&>, virtual_<const matrix&>), void, registry);

BOOST_AUTO_TEST_CASE(throw_error) {
    initialize<registry>();

    try {
        times(matrix(), matrix());
        BOOST_FAIL("should have thrown");
    } catch (const std::runtime_error& error) {
        BOOST_TEST(
            std::string(error.what()).find("not implemented") !=
            std::string::npos);
    } catch (...) {
        BOOST_FAIL("wrong exception");
    }
}

} // namespace TEST_NS
