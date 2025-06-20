// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_DEFAULT_REGISTRY_HPP
#define BOOST_OPENMETHOD_DEFAULT_REGISTRY_HPP

#include <boost/openmethod/registry.hpp>
#include <boost/openmethod/policies/std_rtti.hpp>
#include <boost/openmethod/policies/vptr_vector.hpp>
#include <boost/openmethod/policies/stderr_output.hpp>
#include <boost/openmethod/policies/fast_perfect_hash.hpp>
#include <boost/openmethod/policies/default_error_handler.hpp>

namespace boost::openmethod {

struct release_registry
    : registry<
          policies::std_rtti, policies::fast_perfect_hash,
          policies::vptr_vector, policies::default_error_handler,
          policies::stderr_output> {};

struct debug_registry
    : registry<
          policies::std_rtti, policies::fast_perfect_hash,
          policies::vptr_vector, policies::default_error_handler,
          policies::runtime_checks, policies::stderr_output, policies::trace> {
};

#ifdef NDEBUG
using default_registry = release_registry;
#else
using default_registry = debug_registry;
#endif // BOOST_OPENMETHOD_DEFAULT_REGISTRY_HPP

} // namespace boost::openmethod

#endif
