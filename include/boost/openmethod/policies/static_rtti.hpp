// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_POLICY_MINIMAL_RTTI_HPP
#define BOOST_OPENMETHOD_POLICY_MINIMAL_RTTI_HPP

#include <boost/openmethod/registry.hpp>

namespace boost::openmethod::policies {

//! Minimal implementation of the `rtti` policy.
//!
//! `static_rtti` implements only the static parts of the `rtti` policy. It uses
//! the addresses of a per-class static variables as `type_id`s. It reports all
//! types as non-polymorphic. As a consequence, the `virtual_ptr` constructors
//! that take a pointer or a reference to a polymorphic object are disabled.
//! `virtual_ptr`s must be constructed using @ref final_virtual_ptr.
//!
//! @par Example
//! TODO
//! include::example$static_rtti.cpp[tag=all]
struct static_rtti : rtti {
    template<class Registry>
    struct fn : rtti::defaults {
        //! Always evaluates to `false`.
        //! @tparam Class A class.
        template<class Class>
        static constexpr bool is_polymorphic = false;

        //! Returns the @ref type_id of `Class`.
        //!
        //! @tparam Class A class.
        template<typename T>
        static auto static_type() -> type_id {
            static char id;
            return &id;
        }
    };
};

} // namespace boost::openmethod::policies

#endif
