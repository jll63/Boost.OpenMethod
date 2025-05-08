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
#include <boost/openmethod/policies/vectored_error_handler.hpp>

namespace boost::openmethod {

namespace policies {

struct release : registry<
                     std_rtti, fast_perfect_hash, vptr_vector,
                     vectored_error_handler, stderr_output> {};

struct debug
    : registry<
          std_rtti, fast_perfect_hash, vptr_vector, vectored_error_handler,
          runtime_checks, stderr_output, trace> {};

} // namespace policies

#ifdef NDEBUG
using default_registry = policies::release;
#else
using default_registry = policies::debug;
#endif // BOOST_OPENMETHOD_DEFAULT_REGISTRY_HPP

} // namespace boost::openmethod

#endif
