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

//! Registry with runtime checks and trace enabled
//!
//! `debug_registry` uses the same policies as @ref release_registry, with the
//! additional policies of @ref policies::runtime_checks and @ref
//! policies::trace.
//!
//! This is the default value of
//! [BOOST_OPENMETHOD_DEFAULT_REGISTRY](../BOOST_OPENMETHOD_DEFAULT_REGISTRY.html)
//! when NDEBUG is not defined.
//!
//! `debug_registry` is derived from `release_registry::with<...>`, instead of
//! being aliased, to avoid creating long symbol names wherever it is used. Its
//! state is entirely distinct from `release_registry`\'s.
struct debug_registry
    : release_registry::with<policies::runtime_checks, policies::trace> {};

#ifdef NDEBUG
using default_registry = release_registry;
#else
using default_registry = debug_registry;
#endif

} // namespace boost::openmethod

#endif
