// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <any>

#include <boost/openmethod.hpp>
#include <boost/openmethod/shared_ptr.hpp>
#include <boost/openmethod/unique_ptr.hpp>
#include <boost/openmethod/compiler.hpp>
#include <boost/openmethod/policies/vptr_vector.hpp>
#include <boost/openmethod/policies/throw_error_handler.hpp>

#include "test_util.hpp"

#define BOOST_TEST_MODULE openmethod
#include <boost/test/included/unit_test.hpp>

using namespace boost::openmethod;

namespace animals {

struct Animal {
    explicit Animal(std::string str) {
        name = std::move(str);
    }

    Animal(const Animal&) = delete;

    Animal(Animal&&) = default;

    virtual ~Animal() {
    }

    std::string name;
};

struct Property {
    std::string owner = "Bill";
};

struct Dog : Property, Animal {
    using Animal::Animal;
};

struct Cat : Property, virtual Animal {
    using Animal::Animal;
};

} // namespace animals

// -----------------------------------------------------------------------------
// pass virtual args by lvalue references

namespace TEST_NS {

using test_registry = test_registry_<__COUNTER__>;
using namespace animals;

BOOST_OPENMETHOD_CLASSES(Animal, Dog, Cat, test_registry);

BOOST_OPENMETHOD(name, (virtual_<const Animal&>), std::string, test_registry);

BOOST_OPENMETHOD_OVERRIDE(name, (const Cat& cat), std::string) {
    return cat.owner + "'s cat " + cat.name;
}

BOOST_OPENMETHOD_OVERRIDE(name, (const Dog& dog), std::string) {
    return dog.owner + "'s dog " + dog.name;
}

BOOST_AUTO_TEST_CASE(cast_args_lvalue_refs) {
    initialize<test_registry>();

    Dog spot("Spot");
    BOOST_TEST(name(spot) == "Bill's dog Spot");

    Cat felix("Felix");
    BOOST_TEST(name(felix) == "Bill's cat Felix");
}
} // namespace TEST_NS

namespace TEST_NS {

using test_registry = test_registry_<__COUNTER__>;
using namespace animals;

} // namespace TEST_NS

// -----------------------------------------------------------------------------
// pass virtual args by rvalue references

namespace TEST_NS {

using test_registry = test_registry_<__COUNTER__>;
using namespace animals;

BOOST_OPENMETHOD_CLASSES(Animal, Dog, Cat, test_registry);

BOOST_OPENMETHOD(
    teleport, (virtual_<Animal&&>), std::unique_ptr<Animal>, test_registry);

BOOST_OPENMETHOD_OVERRIDE(teleport, (Cat && cat), std::unique_ptr<Animal>) {
    return std::make_unique<Cat>(std::move(cat));
}

BOOST_OPENMETHOD_OVERRIDE(teleport, (Dog && dog), std::unique_ptr<Animal>) {
    return std::make_unique<Dog>(std::move(dog));
}

BOOST_AUTO_TEST_CASE(cast_args_rvalue_refs) {
    initialize<test_registry>();

    {
        Dog spot("Spot");
        auto animal = teleport(std::move(spot));
        BOOST_TEST(animal->name == "Spot");
        BOOST_TEST(spot.name == "");
    }

    {
        Cat felix("Felix");
        auto animal = teleport(std::move(felix));
        BOOST_TEST(animal->name == "Felix");
        BOOST_TEST(felix.name == "");
    }
}
} // namespace TEST_NS

// -----------------------------------------------------------------------------
// pass virtual args by pointer

namespace TEST_NS {

using test_registry = test_registry_<__COUNTER__>;
using namespace animals;

BOOST_OPENMETHOD_CLASSES(Animal, Dog, Cat, test_registry);

BOOST_OPENMETHOD(name, (virtual_<const Animal*>), std::string, test_registry);

BOOST_OPENMETHOD_OVERRIDE(name, (const Cat* cat), std::string) {
    return cat->owner + "'s cat " + cat->name;
}

BOOST_OPENMETHOD_OVERRIDE(name, (const Dog* dog), std::string) {
    return dog->owner + "'s dog " + dog->name;
}

BOOST_AUTO_TEST_CASE(cast_args_pointer) {
    initialize<test_registry>();

    Dog spot("Spot");
    BOOST_TEST(name(&spot) == "Bill's dog Spot");

    Cat felix("Felix");
    BOOST_TEST(name(&felix) == "Bill's cat Felix");
}
} // namespace TEST_NS

namespace TEST_NS {

// -----------------------------------------------------------------------------
// pass virtual args by shared_ptr by value

using test_registry = test_registry_<__COUNTER__>;
using namespace animals;

BOOST_OPENMETHOD_CLASSES(Animal, Dog, Cat, test_registry);

BOOST_OPENMETHOD(
    name, (virtual_<std::shared_ptr<const Animal>>), std::string,
    test_registry);

BOOST_OPENMETHOD_OVERRIDE(name, (std::shared_ptr<const Cat> cat), std::string) {
    return cat->owner + "'s cat " + cat->name;
}

BOOST_OPENMETHOD_OVERRIDE(name, (std::shared_ptr<const Dog> dog), std::string) {
    return dog->owner + "'s dog " + dog->name;
}

BOOST_AUTO_TEST_CASE(cast_args_shared_ptr_by_value) {
    initialize<test_registry>();

    auto spot = std::make_shared<Dog>("Spot");
    BOOST_TEST(name(spot) == "Bill's dog Spot");

    auto felix = std::make_shared<Cat>("Felix");
    BOOST_TEST(name(felix) == "Bill's cat Felix");
}
} // namespace TEST_NS

namespace TEST_NS {

using test_registry = test_registry_<__COUNTER__>;
using namespace animals;
} // namespace TEST_NS

namespace TEST_NS {

// -----------------------------------------------------------------------------
// pass virtual args by shared_ptr by ref

using test_registry = test_registry_<__COUNTER__>;
using namespace animals;

BOOST_OPENMETHOD_CLASSES(Animal, Dog, Cat, test_registry);

BOOST_OPENMETHOD(
    name, (virtual_<const std::shared_ptr<const Animal>&>), std::string,
    test_registry);

BOOST_OPENMETHOD_OVERRIDE(
    name, (const std::shared_ptr<const Cat>& cat), std::string) {
    return cat->owner + "'s cat " + cat->name;
}

BOOST_OPENMETHOD_OVERRIDE(
    name, (const std::shared_ptr<const Dog>& dog), std::string) {
    return dog->owner + "'s dog " + dog->name;
}

BOOST_AUTO_TEST_CASE(cast_args_shared_ptr_by_ref) {
    initialize<test_registry>();

    auto spot = std::make_shared<Dog>("Spot");
    BOOST_TEST(name(spot) == "Bill's dog Spot");

    auto felix = std::make_shared<Cat>("Felix");
    BOOST_TEST(name(felix) == "Bill's cat Felix");
}

} // namespace TEST_NS

namespace TEST_NS {

// -----------------------------------------------------------------------------
// pass virtual args by unique_ptr

using test_registry = test_registry_<__COUNTER__>;
using namespace animals;

BOOST_OPENMETHOD_CLASSES(Animal, Dog, Cat, test_registry);

BOOST_OPENMETHOD(
    name, (virtual_<std::unique_ptr<Animal>>), std::string, test_registry);

BOOST_OPENMETHOD_OVERRIDE(name, (std::unique_ptr<Cat> cat), std::string) {
    return cat->owner + "'s cat " + cat->name;
}

BOOST_OPENMETHOD_OVERRIDE(name, (std::unique_ptr<Dog> dog), std::string) {
    return dog->owner + "'s dog " + dog->name;
}

BOOST_AUTO_TEST_CASE(cast_args_unique_ptr) {
    initialize<test_registry>();

    auto spot = std::make_unique<Dog>("Spot");
    BOOST_TEST(name(std::move(spot)) == "Bill's dog Spot");
    BOOST_TEST(spot.get() == nullptr);

    auto felix = std::make_unique<Cat>("Felix");
    BOOST_TEST(name(std::move(felix)) == "Bill's cat Felix");
    BOOST_TEST(felix.get() == nullptr);
}
} // namespace TEST_NS

namespace TEST_NS {

using namespace test_matrices;

BOOST_OPENMETHOD_CLASSES(matrix, dense_matrix, diagonal_matrix, test_registry);

BOOST_OPENMETHOD(
    times, (virtual_<const matrix&>, virtual_<const matrix&>), string_pair,
    test_registry);

BOOST_OPENMETHOD(
    times, (double, virtual_<const matrix&>), string_pair, test_registry);

BOOST_OPENMETHOD(
    times, (virtual_<const matrix&>, double), string_pair, test_registry);

BOOST_OPENMETHOD_OVERRIDE(times, (const matrix&, const matrix&), string_pair) {
    return string_pair(MATRIX_MATRIX, NONE);
}

BOOST_OPENMETHOD_OVERRIDE(
    times, (const diagonal_matrix&, const diagonal_matrix&), string_pair) {
    return string_pair(DIAGONAL_DIAGONAL, MATRIX_MATRIX);
}

BOOST_OPENMETHOD_OVERRIDE(times, (double, const matrix&), string_pair) {
    return string_pair(SCALAR_MATRIX, NONE);
}

BOOST_OPENMETHOD_OVERRIDE(
    times, (double, const diagonal_matrix&), string_pair) {
    return string_pair(SCALAR_DIAGONAL, SCALAR_MATRIX);
}

BOOST_OPENMETHOD_OVERRIDE(
    times, (const diagonal_matrix&, double), string_pair) {
    return string_pair(DIAGONAL_SCALAR, MATRIX_SCALAR);
}

BOOST_OPENMETHOD_OVERRIDE(times, (const matrix&, double), string_pair) {
    return string_pair(MATRIX_SCALAR, NONE);
}

BOOST_AUTO_TEST_CASE(simple) {
    auto report = initialize<test_registry>().report;
    BOOST_TEST(report.not_implemented == 0);
    BOOST_TEST(report.ambiguous == 0);

    {
        // pass by const ref
        const matrix& dense = dense_matrix();
        const matrix& diag = diagonal_matrix();
        BOOST_TEST(times(dense, dense) == string_pair(MATRIX_MATRIX, NONE));
        BOOST_TEST(
            times(diag, diag) == string_pair(DIAGONAL_DIAGONAL, MATRIX_MATRIX));
        BOOST_TEST(times(diag, dense) == string_pair(MATRIX_MATRIX, NONE));
        BOOST_TEST(times(2, dense) == string_pair(SCALAR_MATRIX, NONE));
        BOOST_TEST(times(dense, 2) == string_pair(MATRIX_SCALAR, NONE));
        BOOST_TEST(
            times(diag, 2) == string_pair(DIAGONAL_SCALAR, MATRIX_SCALAR));
    }

    if constexpr (test_registry::has_policy<policies::vptr_vector>) {
        BOOST_TEST(!test_registry::dispatch_data.empty());
        BOOST_TEST(
            !detail::vptr_vector_vptrs<test_registry::registry_type>.empty());
        finalize<test_registry>();
        static_assert(detail::has_finalize_aux<
                      test_registry::policy<policies::vptr>>::value);
        BOOST_TEST(test_registry::dispatch_data.empty());
        BOOST_TEST(
            detail::vptr_vector_vptrs<test_registry::registry_type>.empty());
    }
}

} // namespace TEST_NS

namespace TEST_NS {

using namespace test_matrices;
struct test_registry
    : test_registry_<__COUNTER__>::with<policies::throw_error_handler> {};

BOOST_OPENMETHOD_CLASSES(matrix, dense_matrix, diagonal_matrix, test_registry);

BOOST_OPENMETHOD(
    times, (virtual_<const matrix&>, virtual_<const matrix&>), void,
    test_registry);

BOOST_OPENMETHOD_OVERRIDE(
    times, (const matrix&, const diagonal_matrix&), void) {
}

BOOST_OPENMETHOD_OVERRIDE(
    times, (const diagonal_matrix&, const matrix&), void) {
}

BOOST_AUTO_TEST_CASE(ambiguity) {
    auto report = initialize<test_registry>().report;
    BOOST_TEST(report.not_implemented == 1);
    BOOST_TEST(report.ambiguous == 1u);
    BOOST_CHECK_THROW(
        times(diagonal_matrix(), diagonal_matrix()), ambiguous_error);
}

} // namespace TEST_NS

namespace test_next_fn {

struct Animal {
    virtual ~Animal() {
    }
};

struct Dog : Animal {};
struct Bulldog : Dog {};

BOOST_OPENMETHOD_CLASSES(Animal, Dog, Bulldog);

struct BOOST_OPENMETHOD_NAME(poke);
using poke =
    method<BOOST_OPENMETHOD_NAME(poke), auto(virtual_<Animal&>)->std::string>;

auto poke_dog(Dog&) -> std::string {
    return "bark";
}

BOOST_OPENMETHOD_REGISTER(poke::override<poke_dog>);

auto poke_bulldog(Bulldog& dog) -> std::string {
    return poke::next<poke_bulldog>(dog) + " and bite back";
}

BOOST_OPENMETHOD_REGISTER(poke::override<poke_bulldog>);

BOOST_AUTO_TEST_CASE(test_next_fn) {
    initialize();

    std::unique_ptr<Animal> snoopy = std::make_unique<Dog>();
    BOOST_TEST(poke::fn(*snoopy) == "bark");

    std::unique_ptr<Animal> hector = std::make_unique<Bulldog>();
    BOOST_TEST(poke::fn(*hector) == "bark and bite back");
}

} // namespace test_next_fn

namespace errors {

using namespace test_matrices;

BOOST_OPENMETHOD_CLASSES(matrix, dense_matrix, diagonal_matrix, matrix);

BOOST_OPENMETHOD(
    times, (virtual_<const matrix&>, virtual_<const matrix&>), void);

BOOST_OPENMETHOD_OVERRIDE(
    times, (const diagonal_matrix&, const matrix&), void) {
}

BOOST_OPENMETHOD_OVERRIDE(
    times, (const matrix&, const diagonal_matrix&), void) {
}

void test_handler(
    const policies::default_error_handler::error_variant& error_v) {
    if (auto error = std::get_if<not_implemented_error>(&error_v)) {
        throw *error;
    }

    if (auto error = std::get_if<unknown_class_error>(&error_v)) {
        throw *error;
    }

    if (auto error = std::get_if<hash_search_error>(&error_v)) {
        throw *error;
    }

    throw int();
}

} // namespace errors

namespace initialize_error_handling {

using test_registry = test_registry_<__COUNTER__>;

struct base {
    virtual ~base() {
    }
};

BOOST_OPENMETHOD(foo, (virtual_<base&>), void, test_registry);

// instantiate the method
BOOST_OPENMETHOD_OVERRIDE(foo, (base&), void) {
}

BOOST_AUTO_TEST_CASE(test_initialize_error_handling) {
    auto prev_handler = test_registry::error_handler::set(errors::test_handler);

    try {
        initialize<test_registry>();
    } catch (const unknown_class_error& error) {
        test_registry::error_handler::set(prev_handler);
        BOOST_TEST(error.type == reinterpret_cast<type_id>(&typeid(base)));
        return;
    } catch (...) {
        test_registry::error_handler::set(prev_handler);
        BOOST_FAIL("unexpected exception");
    }

    BOOST_FAIL("did not throw");
}

} // namespace initialize_error_handling

namespace across_namespaces {

namespace animals {

class Animal {
  public:
    virtual ~Animal() {
    }
};

BOOST_OPENMETHOD(poke, (virtual_<const Animal&>), std::string);

} // namespace animals

namespace more_animals {

class Dog : public animals::Animal {};

BOOST_OPENMETHOD_CLASSES(Dog, animals::Animal);

BOOST_OPENMETHOD_OVERRIDE(poke, (const Dog&), std::string) {
    return "bark";
}

} // namespace more_animals

BOOST_AUTO_TEST_CASE(across_namespaces) {
    const animals::Animal& animal = more_animals::Dog();
    BOOST_TEST("bark" == poke(animal));
}

} // namespace across_namespaces

namespace test_comma_in_return_type {

using test_registry = test_registry_<__COUNTER__>;

struct Test {
    virtual ~Test() {};
};

BOOST_OPENMETHOD_CLASSES(Test, test_registry);

BOOST_OPENMETHOD(foo, (virtual_<Test&>), std::pair<int, int>, test_registry);

BOOST_OPENMETHOD_OVERRIDE(foo, (Test&), std::pair<int, int>) {
    return {1, 2};
}

BOOST_AUTO_TEST_CASE(comma_in_return_type) {
    initialize<test_registry>();

    Test test;

    BOOST_CHECK(foo(test) == std::pair(1, 2));
}

} // namespace test_comma_in_return_type
