#ifndef BOOST_OPENMETHOD_VIRTUAL_SHARED_PTR_HPP
#define BOOST_OPENMETHOD_VIRTUAL_SHARED_PTR_HPP

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

template<class Class, class Policy>
struct virtual_ptr_traits<std::shared_ptr<Class>, Policy> {
    static bool constexpr is_smart_ptr = true;
    using element_type = Class;

    static auto dynamic_type(const std::shared_ptr<Class>& obj) {
        return Policy::dynamic_type(*obj);
    }

    template<typename Other>
    static auto cast(const virtual_ptr<std::shared_ptr<Class>, Policy>& ptr)
        -> decltype(auto) {

        if constexpr (detail::requires_dynamic_cast<Class&, Other&>) {
            return virtual_ptr<std::shared_ptr<Other>, Policy>(
                std::dynamic_pointer_cast<Other>(ptr.obj), ptr.vp);
        } else {
            return virtual_ptr<std::shared_ptr<Other>, Policy>(
                std::static_pointer_cast<Other>(ptr.obj), ptr.vp);
        }
    }
};

template<class Class, class Policy>
struct virtual_ptr_traits<const std::shared_ptr<Class>&, Policy>
    : virtual_ptr_traits<std::shared_ptr<Class>, Policy> {};

} // namespace detail

template<class Class, class Policy = BOOST_OPENMETHOD_DEFAULT_POLICY>
using virtual_shared_ptr = virtual_ptr<std::shared_ptr<Class>, Policy>;

template<
    class Class, class Policy = BOOST_OPENMETHOD_DEFAULT_POLICY, typename... T>
inline auto make_virtual_shared(T&&... args) {
    return virtual_shared_ptr<Class, Policy>::final(
        std::make_shared<Class>(std::forward<T>(args)...));
}

} // namespace openmethod
} // namespace boost

#endif
