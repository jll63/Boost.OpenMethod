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

struct std_rtti : rtti {
#ifndef BOOST_NO_RTTI

    template<class Registry>
    struct fn {
        template<class Class>
        static constexpr bool is_polymorphic = std::is_polymorphic_v<Class>;

        template<class Class>
        static auto static_type() -> type_id {
            return &typeid(Class);
        }

        template<class Class>
        static auto dynamic_type(const Class& obj) -> type_id {
            return &typeid(obj);
        }

        template<typename Stream>
        static auto type_name(type_id type, Stream& stream) -> void {
            stream << boost::core::demangle(
                reinterpret_cast<const std::type_info*>(type)->name());
        }

        static auto type_index(type_id type) -> std::type_index {
            return std::type_index(
                *reinterpret_cast<const std::type_info*>(type));
        }

        template<typename D, typename B>
        static auto dynamic_cast_ref(B&& obj) -> D {
            return dynamic_cast<D>(obj);
        }
    };

#endif
};

} // namespace boost::openmethod::policies

#endif
