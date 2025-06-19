// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_POLICY_THROW_ERROR_HPP
#define BOOST_OPENMETHOD_POLICY_THROW_ERROR_HPP

#include <boost/openmethod/registry.hpp>

#include <sstream>
#include <stdexcept>
#include <string>

namespace boost::openmethod::policies {

struct throw_error_handler : error_handler {
    template<class Registry>
    class fn {
      public:
        template<class Error>
        [[noreturn]] static auto error(const Error& error) -> void {
            struct wrapper : Error, std::runtime_error {
                using std::runtime_error::runtime_error;
            };

            std::ostringstream os;
            error.template write<Registry>(os);

            throw wrapper(os.str());
        }
    };
};

} // namespace boost::openmethod::policies

#endif
