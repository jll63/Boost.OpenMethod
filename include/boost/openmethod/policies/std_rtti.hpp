// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_POLICY_STD_RTTI_HPP
#define BOOST_OPENMETHOD_POLICY_STD_RTTI_HPP

#include <boost/openmethod/registry.hpp>

#ifndef BOOST_NO_RTTI
#include <typeindex>
#include <typeinfo>
#include <boost/core/demangle.hpp>
#endif

namespace boost::openmethod::policies {

//! Implements the  `rtti` policy with RTTI.
//!
//! This implementation of `rtti` uses the standard C++ RTTI system. It is the
//! default RTTI policy.
//!

struct std_rtti : rtti {
    template<class Registry>
    struct fn {
#ifndef BOOST_NO_RTTI
        //! Returns `true` if `Class` is polymorphic.
        //!
        //! A polymorphic class, as defined by the C++ standard, is a class that
        //! contains at least one virtual function.
        //!
        //! @tparam Class A class.

        template<class Class>
        static constexpr bool is_polymorphic = std::is_polymorphic_v<Class>;

        //! Returns the @ref type_id of `Class` is polymorphic.
        //!
        //! Returns the address of the `std::type_info` object for `Class`, cast
        //! to `type_id`.
        //!
        //! @note `Class` is not necessarily a @e registered class. This
        //! function is also called to acquire the type_id of non-virtual
        //! parameters, library types, etc, for diagnostic and trace purposes.
        //!
        //! @tparam Class A class.

        template<class Class>
        static auto static_type() -> type_id {
            return &typeid(Class);
        }

        //! Returns the @ref type_id of `obj`.
        //!
        //! Returns the address of the `std::type_info` object for `obj`, cast to `type_id`.

        template<class Class>
        static auto dynamic_type(const Class& obj) -> type_id {
            return &typeid(obj);
        }

        //! Writes the demangled name of the class identified by `type` to
        //! `stream`.

        template<typename Stream>
        static auto type_name(type_id type, Stream& stream) -> void {
            stream << boost::core::demangle(
                reinterpret_cast<const std::type_info*>(type)->name());
        }

        //! Returns the `std::type_index` of `type`.
        //!
        //! This function is required because C++ does *not* guarantee that
        //! there is a single instance of `std::type_info` per type.

        static auto type_index(type_id type) -> std::type_index {
            return std::type_index(
                *reinterpret_cast<const std::type_info*>(type));
        }

        //! Casts `obj` to type `D`.
        //!
        //! Casts `obj` to `D` using `dynamic_cast`.

        template<typename D, typename B>
        static auto dynamic_cast_ref(B&& obj) -> D {
            return dynamic_cast<D>(obj);
        }
#endif
    };
};

} // namespace boost::openmethod::policies

#endif
