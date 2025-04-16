// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_POLICY_HPP
#define BOOST_OPENMETHOD_POLICY_HPP

#include <boost/openmethod/policies/basic_policy.hpp>

#include <boost/openmethod/policies/std_rtti.hpp>
#include <boost/openmethod/policies/vptr_vector.hpp>
#include <boost/openmethod/policies/basic_error_output.hpp>
#include <boost/openmethod/policies/basic_trace_output.hpp>
#include <boost/openmethod/policies/fast_perfect_hash.hpp>
#include <boost/openmethod/policies/vectored_error_handler.hpp>

namespace boost::openmethod {

namespace policies {

struct release : basic_policy<
                     release, std_rtti, fast_perfect_hash<release>,
                     vptr_vector<release>, vectored_error_handler<release>> {};

struct debug : release::add<
                   runtime_checks, basic_error_output<debug>,
                   basic_trace_output<debug>>::
                   replace<error_handler, vectored_error_handler<debug>> {};

} // namespace policies

#ifdef NDEBUG
using default_policy = policies::release;
#else
using default_policy = policies::debug;
#endif

} // namespace boost::openmethod

#endif
