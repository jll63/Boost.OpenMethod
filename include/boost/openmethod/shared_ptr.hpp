// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_SHARED_PTR_HPP
#define BOOST_OPENMETHOD_SHARED_PTR_HPP

#include <boost/openmethod/core.hpp>
#include <memory>

namespace boost {
namespace openmethod {
namespace detail {

template<typename Class>
struct shared_ptr_traits {
    static const bool is_shared_ptr = false;
};

template<typename Class>
struct shared_ptr_traits<std::shared_ptr<Class>> {
    static const bool is_shared_ptr = true;
    static const bool is_const_ref = false;
    using virtual_type = Class;
};

template<typename Class>
struct shared_ptr_traits<const std::shared_ptr<Class>&> {
    static const bool is_shared_ptr = true;
    static const bool is_const_ref = true;
    using virtual_type = Class;
};

} // namespace detail

template<class Class, class Policy>
struct virtual_traits<const std::shared_ptr<Class>&, Policy> {
    using virtual_type = std::remove_cv_t<Class>;

    static auto peek(const std::shared_ptr<Class>& arg) -> const Class& {
        return *arg;
    }

    template<class Other>
    using rebind = std::shared_ptr<Other>;

    template<class Other>
    static void check_cast() {
        using namespace boost::openmethod::detail;

        static_assert(shared_ptr_traits<Other>::is_shared_ptr);
        static_assert(
            shared_ptr_traits<Other>::is_const_ref,
            "cannot cast from 'const shared_ptr<base>&' to "
            "'shared_ptr<derived>'");
        static_assert(
            std::is_class_v<typename shared_ptr_traits<Other>::virtual_type>);
    }

    template<class Other>
    static auto cast(const std::shared_ptr<Class>& obj) {
        using namespace boost::openmethod::detail;

        check_cast<Other>();

        if constexpr (detail::requires_dynamic_cast<Class*, Other>) {
            return std::dynamic_pointer_cast<
                typename shared_ptr_traits<Other>::virtual_type>(obj);
        } else {
            return std::static_pointer_cast<
                typename shared_ptr_traits<Other>::virtual_type>(obj);
        }
    }
};

template<typename Class, class Policy>
struct virtual_traits<std::shared_ptr<Class>, Policy> {
    using virtual_type = std::remove_cv_t<Class>;

    static auto peek(const std::shared_ptr<Class>& arg) -> const Class& {
        return *arg;
    }

    template<class Other>
    using rebind = std::shared_ptr<Other>;

    template<class Other>
    static void check_cast() {
        using namespace boost::openmethod::detail;

        static_assert(shared_ptr_traits<Other>::is_shared_ptr);
        static_assert(
            !shared_ptr_traits<Other>::is_const_ref,
            "cannot cast from 'const shared_ptr<base>&' to "
            "'shared_ptr<derived>'");
        static_assert(
            std::is_class_v<typename shared_ptr_traits<Other>::virtual_type>);
    }
    template<class Other>
    static auto cast(const std::shared_ptr<Class>& obj) {
        using namespace boost::openmethod::detail;

        check_cast<Other>();

        if constexpr (detail::requires_dynamic_cast<
                          Class*, typename Other::element_type*>) {
            return std::dynamic_pointer_cast<
                typename shared_ptr_traits<Other>::virtual_type>(obj);
        } else {
            return std::static_pointer_cast<
                typename shared_ptr_traits<Other>::virtual_type>(obj);
        }
    }
};

template<class Class, class Policy = BOOST_OPENMETHOD_DEFAULT_POLICY>
using shared_virtual_ptr = virtual_ptr<std::shared_ptr<Class>, Policy>;

template<
    class Class, class Policy = BOOST_OPENMETHOD_DEFAULT_POLICY, typename... T>
inline auto make_shared_virtual(T&&... args)
    -> shared_virtual_ptr<Class, Policy> {
    return shared_virtual_ptr<Class, Policy>::final(
        std::make_shared<Class>(std::forward<T>(args)...));
}

} // namespace openmethod
} // namespace boost

#endif
