
// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_POLICY_BASIC_TRACE_OUTPUT_HPP
#define BOOST_OPENMETHOD_POLICY_BASIC_TRACE_OUTPUT_HPP

#include <boost/openmethod/policies/basic_policy.hpp>
#include <boost/openmethod/detail/ostdstream.hpp>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

namespace boost::openmethod::policies {

template<class Policy, typename Stream = detail::ostderr>
struct basic_trace_output : virtual trace_output {
    static bool trace_enabled;
    static Stream trace_stream;
};

template<class Policy, typename Stream>
Stream basic_trace_output<Policy, Stream>::trace_stream;

template<class Policy, typename Stream>
bool basic_trace_output<Policy, Stream>::trace_enabled([]() {
    auto env = getenv("BOOST_OPENMETHOD_TRACE");
    return env && *env++ == '1' && *env++ == 0;
}());

} // namespace boost::openmethod::policies

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
