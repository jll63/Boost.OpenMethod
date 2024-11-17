
// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_POLICY_THROW_ERROR_HPP
#define BOOST_OPENMETHOD_POLICY_THROW_ERROR_HPP

#include <boost/openmethod/policies/core.hpp>

namespace boost {
namespace openmethod {
namespace policies {

struct throw_error : virtual error_handler {
    template<class Error>
    static auto error(const Error& error) {
        throw error;
    }
};

} // namespace policies
} // namespace openmethod
} // namespace boost

#endif
