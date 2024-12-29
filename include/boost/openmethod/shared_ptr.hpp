#ifndef BOOST_OPENMETHOD_SHARED_PTR_HPP
#define BOOST_OPENMETHOD_SHARED_PTR_HPP

#include <boost/openmethod/core.hpp>
#include <memory>

namespace boost {
namespace openmethod {
namespace detail {

template<typename T>
struct shared_ptr_traits {
    static const bool is_shared_ptr = false;
};

template<typename T>
struct shared_ptr_traits<std::shared_ptr<T>> {
    static const bool is_shared_ptr = true;
    static const bool is_const_ref = false;
    using virtual_type = T;
};

template<typename T>
struct shared_ptr_traits<const std::shared_ptr<T>&> {
    static const bool is_shared_ptr = true;
    static const bool is_const_ref = true;
    using virtual_type = T;
};

template<typename T, class Policy>
struct virtual_traits<const std::shared_ptr<T>&, Policy> {
    using virtual_type = std::remove_cv_t<T>;

    static auto rarg(const std::shared_ptr<T>& arg) -> const T& {
        return *arg;
    }

    template<class DERIVED>
    static void check_cast() {
        static_assert(shared_ptr_traits<DERIVED>::is_shared_ptr);
        static_assert(
            shared_ptr_traits<DERIVED>::is_const_ref,
            "cannot cast from 'const shared_ptr<base>&' to "
            "'shared_ptr<derived>'");
        static_assert(
            std::is_class_v<typename shared_ptr_traits<DERIVED>::virtual_type>);
    }

    template<class DERIVED>
    static auto cast(const std::shared_ptr<T>& obj) {
        check_cast<DERIVED>();

        if constexpr (detail::requires_dynamic_cast<T*, DERIVED>) {
            return std::dynamic_pointer_cast<
                typename shared_ptr_traits<DERIVED>::virtual_type>(obj);
        } else {
            return std::static_pointer_cast<
                typename shared_ptr_traits<DERIVED>::virtual_type>(obj);
        }
    }
};

template<typename T, class Policy>
struct virtual_traits<std::shared_ptr<T>, Policy> {
    using virtual_type = std::remove_cv_t<T>;

    static auto rarg(const std::shared_ptr<T>& arg) -> const T& {
        return *arg;
    }

    template<class DERIVED>
    static void check_cast() {
        static_assert(shared_ptr_traits<DERIVED>::is_shared_ptr);
        static_assert(
            !shared_ptr_traits<DERIVED>::is_const_ref,
            "cannot cast from 'const shared_ptr<base>&' to "
            "'shared_ptr<derived>'");
        static_assert(
            std::is_class_v<typename shared_ptr_traits<DERIVED>::virtual_type>);
    }
    template<class DERIVED>
    static auto cast(const std::shared_ptr<T>& obj) {
        check_cast<DERIVED>();

        if constexpr (detail::requires_dynamic_cast<T*, DERIVED>) {
            return std::dynamic_pointer_cast<
                typename shared_ptr_traits<DERIVED>::virtual_type>(obj);
        } else {
            return std::static_pointer_cast<
                typename shared_ptr_traits<DERIVED>::virtual_type>(obj);
        }
    }
};

} // namespace detail
} // namespace openmethod
} // namespace boost

#endif
