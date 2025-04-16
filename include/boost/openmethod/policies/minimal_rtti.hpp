// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_POLICY_MINIMAL_RTTI_HPP
#define BOOST_OPENMETHOD_POLICY_MINIMAL_RTTI_HPP

#include <boost/openmethod/policies/basic_policy.hpp>

namespace boost {
namespace openmethod {
namespace policies {

struct minimal_rtti : virtual rtti {
    template<class Class>
    static constexpr bool is_polymorphic = false;

    template<typename T>
    static auto static_type() -> type_id {
        static char id;
        return reinterpret_cast<type_id>(&id);
    }
};

} // namespace policies
} // namespace openmethod
} // namespace boost

#endif
