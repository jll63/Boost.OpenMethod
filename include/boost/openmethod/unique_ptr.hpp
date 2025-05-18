// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_UNIQUE_PTR_HPP
#define BOOST_OPENMETHOD_UNIQUE_PTR_HPP

#include <boost/openmethod/core.hpp>

#include <memory>

namespace boost::openmethod {

template<class Class, class Registry>
struct virtual_traits<std::unique_ptr<Class>, Registry> {
    using virtual_type = std::remove_cv_t<Class>;

    static auto peek(const std::unique_ptr<Class>& arg) -> const Class& {
        return *arg;
    }

    template<class Other>
    using rebind = std::unique_ptr<Other>;

    template<typename Other>
    static auto cast(std::unique_ptr<Class>&& ptr) {
        if constexpr (detail::requires_dynamic_cast<Class&, Other&>) {
            return Other(
                &dynamic_cast<typename Other::element_type&>(*ptr.release()));
        } else {
            return Other(
                &static_cast<typename Other::element_type&>(*ptr.release()));
        }
    }
};

template<class Class, class Registry = BOOST_OPENMETHOD_DEFAULT_REGISTRY>
using unique_virtual_ptr = virtual_ptr<std::unique_ptr<Class>, Registry>;

template<
    class Class, class Registry = BOOST_OPENMETHOD_DEFAULT_REGISTRY,
    typename... T>
inline auto make_unique_virtual(T&&... args)
    -> unique_virtual_ptr<Class, Registry> {
    return unique_virtual_ptr<Class, Registry>::final(
        std::make_unique<Class>(std::forward<T>(args)...));
}

} // namespace boost::openmethod

#endif
