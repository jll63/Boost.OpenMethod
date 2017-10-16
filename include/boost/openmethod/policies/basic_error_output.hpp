// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_POLICY_BASIC_ERROR_OUTPUT_HPP
#define BOOST_OPENMETHOD_POLICY_BASIC_ERROR_OUTPUT_HPP

#include <boost/openmethod/policies/basic_policy.hpp>
#include <boost/openmethod/detail/ostdstream.hpp>

namespace boost {
namespace openmethod {
namespace policies {

template<class Policy, typename Stream = detail::ostderr>
struct basic_error_output : virtual error_output {
    static Stream error_stream;
};

template<class Policy, typename Stream>
Stream basic_error_output<Policy, Stream>::error_stream;

} // namespace policies
} // namespace openmethod
} // namespace boost

#endif
