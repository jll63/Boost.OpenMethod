// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_CORE_HPP
#define BOOST_OPENMETHOD_CORE_HPP

#include <stdint.h>
#include <algorithm>
#include <cstdlib>
#include <tuple>
#include <type_traits>
#include <utility>

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/bind.hpp>
#include <boost/mp11/integral.hpp>
#include <boost/mp11/list.hpp>

#include <boost/openmethod/registry.hpp>
#include <boost/openmethod/default_registry.hpp>

#ifndef BOOST_OPENMETHOD_DEFAULT_REGISTRY
#define BOOST_OPENMETHOD_DEFAULT_REGISTRY ::boost::openmethod::default_registry
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4646)
#endif

namespace boost::openmethod {

// =============================================================================
// Helpers

namespace detail {

template<class Registry, class Class>
constexpr bool is_polymorphic = Registry::rtti::template is_polymorphic<Class>;

using macro_default_registry = BOOST_OPENMETHOD_DEFAULT_REGISTRY;

template<typename...>
struct extract_registry;

template<>
struct extract_registry<> {
    using registry = BOOST_OPENMETHOD_DEFAULT_REGISTRY;
    using others = mp11::mp_list<>;
};

template<typename Type>
struct extract_registry<Type> {
    using registry = std::conditional_t<
        is_registry<Type>, Type, BOOST_OPENMETHOD_DEFAULT_REGISTRY>;
    using others = std::conditional_t<
        is_registry<Type>, mp11::mp_list<>, mp11::mp_list<Type>>;
};

template<typename Type1, typename Type2, typename... MoreTypes>
struct extract_registry<Type1, Type2, MoreTypes...> {
    static_assert(!is_registry<Type1>, "policy must be the last in the list");
    using registry = typename extract_registry<Type2, MoreTypes...>::registry;
    using others = mp11::mp_push_front<
        typename extract_registry<Type2, MoreTypes...>::others, Type1>;
};

template<class Registry, class... Class>
struct init_type_ids;

template<class Registry, class... Class>
struct init_type_ids<Registry, mp11::mp_list<Class...>> {
    static auto fn(type_id* ids) {
        (..., (*ids++ = Registry::rtti::template static_type<Class>()));
        return ids;
    }
};

template<class...>
struct use_class_aux;

template<class Registry, class Class, typename... Bases>
struct use_class_aux<Registry, mp11::mp_list<Class, Bases...>>
    : std::conditional_t<
          Registry::deferred_static_rtti, detail::deferred_class_info,
          detail::class_info> {
    inline static type_id bases[sizeof...(Bases)];
    use_class_aux() {
        this->first_base = bases;
        this->last_base = bases + sizeof...(Bases);
        this->is_abstract = std::is_abstract_v<Class>;
        this->static_vptr = &Registry::template static_vptr<Class>;

        if constexpr (!Registry::deferred_static_rtti) {
            resolve_type_ids();
        }

        Registry::classes.push_back(*this);
    }

    void resolve_type_ids() {
        this->type = Registry::rtti::template static_type<Class>();
        auto iter = bases;
        (..., (*iter++ = Registry::rtti::template static_type<Bases>()));
    }

    ~use_class_aux() {
        Registry::classes.remove(*this);
    }
};

// Collect the base classes of a list of classes. The result is a mp11 map that
// associates each class to a list starting with the class itself, followed by
// all its bases, as per std::is_base_of. Thus the list includes the class
// itself at least twice: at the front, and down the list, as its own improper
// base. The direct and indirect bases are all included. The runtime will
// extract the direct proper bases.
template<typename... Cs>
using inheritance_map = mp11::mp_list<boost::mp11::mp_push_front<
    boost::mp11::mp_filter_q<
        boost::mp11::mp_bind_back<std::is_base_of, Cs>, mp11::mp_list<Cs...>>,
    Cs>...>;

// =============================================================================
// optimal_cast

template<typename B, typename D, typename = void>
struct requires_dynamic_cast_ref_aux : std::true_type {};

template<typename B, typename D>
struct requires_dynamic_cast_ref_aux<
    B, D, std::void_t<decltype(static_cast<D>(std::declval<B>()))>>
    : std::false_type {};

template<class B, class D>
constexpr bool requires_dynamic_cast =
    detail::requires_dynamic_cast_ref_aux<B, D>::value;

template<class Registry, class D, class B>
auto optimal_cast(B&& obj) -> decltype(auto) {
    if constexpr (requires_dynamic_cast<B, D>) {
        return Registry::rtti::template dynamic_cast_ref<D>(
            std::forward<B>(obj));
    } else {
        return static_cast<D>(obj);
    }
}

// =============================================================================
// Common details

template<typename T>
struct is_virtual : std::false_type {};

template<typename T>
struct is_virtual<virtual_<T>> : std::true_type {};

template<typename T>
struct remove_virtual_aux {
    using type = T;
};

template<typename T>
struct remove_virtual_aux<virtual_<T>> {
    using type = T;
};

template<typename T>
using remove_virtual = typename remove_virtual_aux<T>::type;

template<typename T, class Registry>
using virtual_type = typename virtual_traits<T, Registry>::virtual_type;

template<typename MethodArgList>
using virtual_types = boost::mp11::mp_transform<
    remove_virtual, boost::mp11::mp_filter<detail::is_virtual, MethodArgList>>;

template<typename T, class Registry>
struct parameter_traits {
    static auto peek(const T&) {
        return nullptr;
    }

    template<typename>
    static auto cast(T value) -> T {
        return value;
    }
};

template<typename T, class Registry>
struct parameter_traits<virtual_<T>, Registry> : virtual_traits<T, Registry> {};

template<class Class, class Registry>
struct parameter_traits<virtual_ptr<Class, Registry>, Registry>
    : virtual_traits<virtual_ptr<Class, Registry>, Registry> {};

template<class Class, class Registry>
struct parameter_traits<const virtual_ptr<Class, Registry>&, Registry>
    : virtual_traits<const virtual_ptr<Class, Registry>&, Registry> {};

} // namespace detail

// =============================================================================
// virtual_traits

template<typename T, class Registry>
struct virtual_traits {
    using virtual_type = void;
};

template<typename T, class Registry>
struct virtual_traits<T&, Registry> {
    using virtual_type = std::remove_cv_t<T>;

    static auto peek(const T& arg) -> const T& {
        return arg;
    }

    template<typename D>
    static auto cast(T& obj) -> D& {
        return detail::optimal_cast<Registry, D&>(obj);
    }
};

template<typename T, class Registry>
struct virtual_traits<T&&, Registry> {
    using virtual_type = T;

    static auto peek(const T& arg) -> const T& {
        return arg;
    }

    template<typename D>
    static auto cast(T&& obj) -> D&& {
        return detail::optimal_cast<Registry, D&&>(obj);
    }
};

template<typename T, class Registry>
struct virtual_traits<T*, Registry> {
    using virtual_type = std::remove_cv_t<T>;

    static auto peek(T* arg) -> const T& {
        return *arg;
    }

    template<typename D>
    static auto cast(T* ptr) {
        static_assert(
            std::is_base_of_v<
                virtual_type, std::remove_pointer_t<std::remove_cv_t<D>>>);
        if constexpr (detail::requires_dynamic_cast<T*, D>) {
            return dynamic_cast<D>(ptr);
        } else {
            return static_cast<D>(ptr);
        }
    }
};

template<class... Classes>
struct use_classes {
    using tuple_type = boost::mp11::mp_apply<
        std::tuple,
        boost::mp11::mp_transform_q<
            boost::mp11::mp_bind_front<
                detail::use_class_aux,
                typename detail::extract_registry<Classes...>::registry>,
            boost::mp11::mp_apply<
                detail::inheritance_map,
                typename detail::extract_registry<Classes...>::others>>>;
    tuple_type tuple;
};

// =============================================================================
// virtual_ptr

namespace detail {

template<class Class, class Registry>
struct is_virtual<virtual_ptr<Class, Registry>> : std::true_type {};

template<class Class, class Registry>
struct is_virtual<const virtual_ptr<Class, Registry>&> : std::true_type {};

template<typename>
struct is_virtual_ptr_aux : std::false_type {};

template<class Class, class Registry>
struct is_virtual_ptr_aux<virtual_ptr<Class, Registry>> : std::true_type {};

template<class Class, class Registry>
struct is_virtual_ptr_aux<const virtual_ptr<Class, Registry>&>
    : std::true_type {};

template<typename T>
constexpr bool is_virtual_ptr = detail::is_virtual_ptr_aux<T>::value;

void boost_openmethod_vptr(...);

template<class Class, class Registry>
constexpr bool has_vptr_fn = std::is_same_v<
    decltype(boost_openmethod_vptr(
        std::declval<const Class&>(), std::declval<Registry*>())),
    vptr_type>;

template<class Registry, class ArgType>
decltype(auto) acquire_vptr(const ArgType& arg) {
    Registry::check_initialized();

    if constexpr (detail::has_vptr_fn<ArgType, Registry>) {
        return boost_openmethod_vptr(arg, static_cast<Registry*>(nullptr));
    } else {
        return Registry::template policy<policies::vptr>::dynamic_vptr(arg);
    }
}

template<bool Indirect>
inline auto box_vptr(const vptr_type& vp) {
    if constexpr (Indirect) {
        return &vp;
    } else {
        return vp;
    }
}

inline auto unbox_vptr(vptr_type vp) {
    return vp;
}

inline auto unbox_vptr(const vptr_type* vpp) {
    return *vpp;
}

inline vptr_type null_vptr = nullptr;

template<class Class, class Registry, typename = void>
class virtual_ptr_impl {
  protected:
    static constexpr bool use_indirect_vptrs =
        Registry::template has_policy<policies::indirect_vptr>;

    std::conditional_t<use_indirect_vptrs, const vptr_type*, vptr_type> vp;
    Class* obj;

  public:
    using traits = virtual_traits<Class&, Registry>;
    using element_type = Class;
    static constexpr bool is_smart_ptr = false;

    virtual_ptr_impl() = default;

    explicit virtual_ptr_impl(std::nullptr_t)
        : vp(box_vptr<use_indirect_vptrs>(null_vptr)), obj(nullptr) {
    }

    template<
        class Other,
        typename = std::enable_if_t<
            std::is_constructible_v<Class*, Other*> &&
            is_polymorphic<Registry, Class>>>
    virtual_ptr_impl(Other& other)
        : vp(box_vptr<use_indirect_vptrs>(acquire_vptr<Registry>(other))),
          obj(&other) {
    }

    template<
        class Other,
        typename = std::enable_if_t<
            std::is_constructible_v<
                Class*,
                decltype(std::declval<virtual_ptr<Other, Registry>>().get())> &&
            is_polymorphic<Registry, Class>>>
    virtual_ptr_impl(Other* other)
        : vp(box_vptr<use_indirect_vptrs>(acquire_vptr<Registry>(*other))),
          obj(other) {
    }

    template<
        class Other,
        typename = std::enable_if_t<std::is_constructible_v<
            Class*,
            decltype(std::declval<virtual_ptr<Other, Registry>>().get())>>>
    virtual_ptr_impl(const virtual_ptr<Other, Registry>& other)
        : vp(other.vp), obj(other.get()) {
    }

    template<
        class Other,
        typename = std::enable_if_t<std::is_constructible_v<
            Class*,
            decltype(std::declval<virtual_ptr<Other, Registry>>().get())>>>
    virtual_ptr_impl(virtual_ptr_impl<Other, Registry>& other)
        : vp(other.vp), obj(other.get()) {
        // Why is this needed? Consider this conversion conversion from
        // smart to dumb pointer:
        //      virtual_ptr<std::shared_ptr<const Node>> p = ...;
        //      virtual_ptr<const Node> q = p;
        // Since 'p' is not const, in the absence of this ctor,
        // virtual_ptr_impl(Other&) would be preferred to
        // virtual_ptr_impl(const virtual_ptr<Other, Registry>& other), and
        // that is incorrect.
    }

    template<
        class Other,
        typename = std::enable_if_t<std::is_constructible_v<Class*, Other*>>>
    virtual_ptr_impl(Other& other, decltype(vp) vp) : vp(vp), obj(&other) {
    }

    template<
        class Other,
        typename = std::enable_if_t<
            std::is_assignable_v<Class*, Other*> &&
            is_polymorphic<Registry, Class>>>
    virtual_ptr_impl& operator=(Other& other) {
        obj = &other;
        vp = box_vptr<use_indirect_vptrs>(acquire_vptr<Registry>(other));
        return *this;
    }

    template<
        class Other,
        typename = std::enable_if_t<
            std::is_assignable_v<Class*, Other*> &&
            is_polymorphic<Registry, Class>>>
    virtual_ptr_impl& operator=(Other* other) {
        obj = other;
        vp = box_vptr<use_indirect_vptrs>(acquire_vptr<Registry>(*other));
        return *this;
    }

    template<
        class Other,
        typename = std::enable_if_t<std::is_assignable_v<
            Class*,
            decltype(std::declval<virtual_ptr<Other, Registry>>().get())>>>
    virtual_ptr_impl&
    operator=(const virtual_ptr_impl<Other, Registry>& other) {
        obj = other.get();
        vp = other.vp;
        return *this;
    }

    virtual_ptr_impl& operator=(std::nullptr_t) {
        obj = nullptr;
        vp = box_vptr<use_indirect_vptrs>(null_vptr);
        return *this;
    }

    auto get() const -> Class* {
        return obj;
    }

    auto operator->() const {
        return get();
    }

    auto operator*() const -> element_type& {
        return *get();
    }

    auto pointer() const -> const Class*& {
        return obj;
    }

    template<class Other>
    auto cast() const -> decltype(auto) {
        static_assert(
            std::is_base_of_v<Class, Other> || std::is_base_of_v<Other, Class>);

        return virtual_ptr<Other, Registry>(
            traits::template cast<Other&>(*obj), vp);
    }

    template<class, class>
    friend struct virtual_traits;
};

template<class Class, class Other, class Registry, typename = void>
struct same_smart_ptr_aux : std::false_type {};

template<class Class, class Other, class Registry>
struct same_smart_ptr_aux<
    Class, Other, Registry,
    std::void_t<typename virtual_traits<Class, Registry>::template rebind<
        typename Other::element_type>>>
    : std::is_same<
          Other,
          typename virtual_traits<Class, Registry>::template rebind<
              typename Other::element_type>> {};

template<class Class, class Other, class Registry>
constexpr bool same_smart_ptr =
    same_smart_ptr_aux<Class, Other, Registry>::value;

template<class Class, class Registry>
class virtual_ptr_impl<
    Class, Registry,
    std::void_t<
        typename virtual_traits<Class, Registry>::template rebind<Class>>> {

  public:
    using traits = virtual_traits<Class, Registry>;
    using element_type = typename Class::element_type;

    template<class, class>
    friend class virtual_ptr;
    template<class, class, typename>
    friend class virtual_ptr_impl;
    template<class, class>
    friend struct virtual_traits;

  protected:
    static constexpr bool use_indirect_vptrs =
        Registry::template has_policy<policies::indirect_vptr>;

    std::conditional_t<use_indirect_vptrs, const vptr_type*, vptr_type> vp;
    Class obj;

  public:
    static constexpr bool is_smart_ptr = true;

    virtual_ptr_impl() : vp(box_vptr<use_indirect_vptrs>(null_vptr)) {
    }

    explicit virtual_ptr_impl(std::nullptr_t)
        : vp(box_vptr<use_indirect_vptrs>(null_vptr)), obj(nullptr) {
    }

    virtual_ptr_impl(const virtual_ptr_impl& other) = default;

    template<
        class Other,
        typename = std::enable_if_t<
            same_smart_ptr<Class, Other, Registry> &&
            std::is_constructible_v<Class, const Other&> &&
            is_polymorphic<Registry, element_type>>>
    virtual_ptr_impl(const Other& other)
        : vp(box_vptr<use_indirect_vptrs>(
              other ? acquire_vptr<Registry>(*other) : null_vptr)),
          obj(other) {
    }

    template<
        class Other,
        typename = std::enable_if_t<
            same_smart_ptr<Class, Other, Registry> &&
            std::is_constructible_v<Class, Other&> &&
            is_polymorphic<Registry, element_type>>>
    virtual_ptr_impl(Other& other)
        : vp(box_vptr<use_indirect_vptrs>(
              other ? acquire_vptr<Registry>(*other) : null_vptr)),
          obj(other) {
    }

    template<
        class Other,
        typename = std::enable_if_t<std::is_constructible_v<Class*, Other*>>>
    virtual_ptr_impl(Other& other, decltype(vp) vp) : vp(vp), obj(&other) {
    }

    template<
        class Other,
        typename = std::enable_if_t<
            same_smart_ptr<Class, Other, Registry> &&
            std::is_constructible_v<Class, Other&&> &&
            is_polymorphic<Registry, element_type>>>
    virtual_ptr_impl(Other&& other)
        : vp(box_vptr<use_indirect_vptrs>(
              other ? acquire_vptr<Registry>(*other) : null_vptr)),
          obj(std::move(other)) {
    }

    template<
        class Other,
        typename = std::enable_if_t<
            same_smart_ptr<Class, Other, Registry> &&
            std::is_constructible_v<Class, const Other&>>>
    virtual_ptr_impl(const virtual_ptr_impl<Other, Registry>& other)
        : vp(other.vp), obj(other.obj) {
    }

    template<
        class Other,
        typename = std::enable_if_t<
            same_smart_ptr<Class, Other, Registry> &&
            std::is_constructible_v<Class, Other&>>>
    virtual_ptr_impl(virtual_ptr_impl<Other, Registry>& other)
        : vp(other.vp), obj(other.obj) {
    }

    virtual_ptr_impl(virtual_ptr_impl&& other)
        : vp(other.vp), obj(std::move(other.obj)) {
        other.vp = box_vptr<use_indirect_vptrs>(null_vptr);
    }

    template<
        class Other,
        typename = std::enable_if_t<
            same_smart_ptr<Class, Other, Registry> &&
            std::is_constructible_v<Class, Other&&>>>
    virtual_ptr_impl(virtual_ptr_impl<Other, Registry>&& other)
        : vp(other.vp), obj(std::move(other.obj)) {
        other.vp = box_vptr<use_indirect_vptrs>(null_vptr);
    }

    template<typename Arg>
    virtual_ptr_impl(Arg&& obj, decltype(vp) vp)
        : vp(vp), obj(std::forward<Arg>(obj)) {
    }

    virtual_ptr_impl& operator=(std::nullptr_t) {
        obj = nullptr;
        vp = box_vptr<use_indirect_vptrs>(null_vptr);
        return *this;
    }

    template<
        class Other,
        typename = std::enable_if_t<
            same_smart_ptr<Class, Other, Registry> &&
            std::is_assignable_v<Class, const Other&> &&
            is_polymorphic<Registry, element_type>>>
    virtual_ptr_impl& operator=(const Other& other) {
        obj = other;
        vp = box_vptr<use_indirect_vptrs>(acquire_vptr<Registry>(*other));
        return *this;
    }

    template<
        class Other,
        typename = std::enable_if_t<
            same_smart_ptr<Class, Other, Registry> &&
            std::is_assignable_v<Class, Other&&> &&
            is_polymorphic<Registry, element_type>>>
    virtual_ptr_impl& operator=(Other&& other) {
        vp = box_vptr<use_indirect_vptrs>(
            other ? acquire_vptr<Registry>(*other) : null_vptr);
        obj = std::move(other);
        return *this;
    }

    template<
        class Other,
        typename = std::enable_if_t<
            same_smart_ptr<Class, Other, Registry> &&
            std::is_assignable_v<Class, Other&>>>
    virtual_ptr_impl& operator=(virtual_ptr_impl<Other, Registry>& other) {
        obj = other.obj;
        vp = other.vp;
        return *this;
    }

    virtual_ptr_impl& operator=(const virtual_ptr_impl& other) = default;

    template<
        class Other,
        typename = std::enable_if_t<
            same_smart_ptr<Class, Other, Registry> &&
            std::is_assignable_v<Class, const Other&>>>
    virtual_ptr_impl&
    operator=(const virtual_ptr_impl<Other, Registry>& other) {
        obj = other.obj;
        vp = other.vp;
        return *this;
    }

    template<
        class Other,
        typename = std::enable_if_t<
            same_smart_ptr<Class, Other, Registry> &&
            std::is_assignable_v<Class, Other&&>>>
    virtual_ptr_impl& operator=(virtual_ptr_impl<Other, Registry>&& other) {
        obj = std::move(other.obj);
        vp = other.vp;
        other.vp = box_vptr<use_indirect_vptrs>(null_vptr);
        return *this;
    }

    auto get() const -> element_type* {
        return obj.get();
    }

    auto operator->() const -> element_type* {
        return get();
    }

    auto operator*() const -> element_type& {
        return *get();
    }

    auto pointer() const -> const Class& {
        return obj;
    }

    template<class Other>
    auto cast() & -> decltype(auto) {
        static_assert(
            std::is_base_of_v<element_type, Other> ||
            std::is_base_of_v<Other, element_type>);

        using other_smart_ptr = typename traits::template rebind<Other>;

        return virtual_ptr<other_smart_ptr, Registry>(
            traits::template cast<other_smart_ptr>(obj), vp);
    }

    template<class Other>
    auto cast() const& -> decltype(auto) {
        static_assert(
            std::is_base_of_v<element_type, Other> ||
            std::is_base_of_v<Other, element_type>);

        using other_smart_ptr = typename traits::template rebind<Other>;

        return virtual_ptr<other_smart_ptr, Registry>(
            traits::template cast<other_smart_ptr>(obj), vp);
    }

    template<class Other>
    auto cast() && -> decltype(auto) {
        static_assert(
            std::is_base_of_v<element_type, Other> ||
            std::is_base_of_v<Other, element_type>);

        using other_smart_ptr = typename traits::template rebind<Other>;

        return virtual_ptr<other_smart_ptr, Registry>(
            traits::template cast<other_smart_ptr>(std::move(obj)), vp);
    }
};

} // namespace detail

template<class Class, class Registry = BOOST_OPENMETHOD_DEFAULT_REGISTRY>
class virtual_ptr : public detail::virtual_ptr_impl<Class, Registry> {
    using impl = detail::virtual_ptr_impl<Class, Registry>;

  public:
    using detail::virtual_ptr_impl<Class, Registry>::virtual_ptr_impl;
    using element_type = typename impl::element_type;

    template<class, class, typename>
    friend class detail::virtual_ptr_impl;

    template<
        typename Other,
        typename = std::enable_if_t<std::is_assignable_v<impl, Other>>>
    virtual_ptr& operator=(Other&& other) {
        impl::operator=(std::forward<Other>(other));
        return *this;
    }

    template<class Other>
    static auto final(Other&& obj) {
        using namespace detail;
        using other_traits = virtual_traits<Other, Registry>;
        using other_class = typename other_traits::virtual_type;

        static_assert(
            std::is_base_of_v<element_type, other_class> ||
            std::is_base_of_v<other_class, element_type>);

        if constexpr (
            Registry::runtime_checks &&
            is_polymorphic<Registry, typename impl::traits::virtual_type> &&
            is_polymorphic<Registry, other_class>) {

            // check that dynamic type == static type
            auto static_type =
                Registry::rtti::template static_type<other_class>();
            type_id dynamic_type =
                Registry::rtti::dynamic_type(other_traits::peek(obj));

            if (dynamic_type != static_type) {
                if constexpr (is_not_void<typename Registry::error_handler>) {
                    final_error error;
                    error.static_type = static_type;
                    error.dynamic_type = dynamic_type;
                    Registry::error_handler::error(error);
                }

                abort();
            }
        }

        return virtual_ptr(
            std::forward<Other>(obj),
            box_vptr<impl::use_indirect_vptrs>(
                Registry::template static_vptr<other_class>));
    }

    auto vptr() const {
        return detail::unbox_vptr(this->vp);
    }
};

template<class Class>
virtual_ptr(Class&) -> virtual_ptr<Class, BOOST_OPENMETHOD_DEFAULT_REGISTRY>;

template<class Class>
virtual_ptr(Class&&) -> virtual_ptr<Class, BOOST_OPENMETHOD_DEFAULT_REGISTRY>;

// Alas this is not allowed:
// template<class Registry, class Class>
// virtual_ptr<Registry>(Class&) -> virtual_ptr<Class, Registry>;

template<class Registry, class Class>
inline auto final_virtual_ptr(Class&& obj) {
    return virtual_ptr<std::remove_reference_t<Class>, Registry>::final(
        std::forward<Class>(obj));
}

template<class Class>
inline auto final_virtual_ptr(Class&& obj) {
    return virtual_ptr<std::remove_reference_t<Class>>::final(
        std::forward<Class>(obj));
}

template<class Left, class Right, class Registry>
auto operator==(
    const virtual_ptr<Left, Registry>& left,
    const virtual_ptr<Right, Registry>& right) -> bool {
    return &*left == &*right;
}

template<class Left, class Right, class Registry>
auto operator!=(
    const virtual_ptr<Left, Registry>& left,
    const virtual_ptr<Right, Registry>& right) -> bool {
    return !(left == right);
}

template<class Class, class Registry>
struct virtual_traits<virtual_ptr<Class, Registry>, Registry> {
    using virtual_type = typename virtual_ptr<Class, Registry>::element_type;

    static auto peek(const virtual_ptr<Class, Registry>& ptr)
        -> const virtual_ptr<Class, Registry>& {
        return ptr;
    }

    template<typename Derived>
    static auto cast(const virtual_ptr<Class, Registry>& ptr)
        -> decltype(auto) {
        return ptr.template cast<typename Derived::element_type>();
    }

    template<typename Derived>
    static auto cast(virtual_ptr<Class, Registry>&& ptr) -> decltype(auto) {
        return std::move(ptr).template cast<typename Derived::element_type>();
    }
};

template<class Class, class Registry>
struct virtual_traits<const virtual_ptr<Class, Registry>&, Registry> {
    using virtual_type = typename virtual_ptr<Class, Registry>::element_type;

    static auto peek(const virtual_ptr<Class, Registry>& ptr)
        -> const virtual_ptr<Class, Registry>& {
        return ptr;
    }

    template<typename Derived>
    static auto cast(const virtual_ptr<Class, Registry>& ptr)
        -> decltype(auto) {
        return ptr.template cast<
            typename std::remove_reference_t<Derived>::element_type>();
    }

    template<typename Derived>
    static auto cast(virtual_ptr<Class, Registry>&& ptr) -> decltype(auto) {
        return std::move(ptr).template cast<typename Derived::element_type>();
    }
};

// =============================================================================
// Method

namespace detail {

template<typename P, typename Q, class Registry>
struct select_overrider_virtual_type_aux {
    using type = void;
};

template<typename P, typename Q, class Registry>
struct select_overrider_virtual_type_aux<virtual_<P>, Q, Registry> {
    using type = virtual_type<Q, Registry>;
};

template<typename P, typename Q, class Registry>
struct select_overrider_virtual_type_aux<
    virtual_ptr<P, Registry>, virtual_ptr<Q, Registry>, Registry> {
    using type = typename virtual_traits<
        virtual_ptr<Q, Registry>, Registry>::virtual_type;
};

template<typename P, typename Q, class Registry>
struct select_overrider_virtual_type_aux<
    const virtual_ptr<P, Registry>&, const virtual_ptr<Q, Registry>&,
    Registry> {
    using type = typename virtual_traits<
        const virtual_ptr<Q, Registry>&, Registry>::virtual_type;
};

template<typename P, typename Q, class Registry>
using select_overrider_virtual_type =
    typename select_overrider_virtual_type_aux<P, Q, Registry>::type;

template<
    typename MethodParameters, typename OverriderParameters, class Registry>
using overrider_virtual_types = boost::mp11::mp_remove<
    boost::mp11::mp_transform_q<
        boost::mp11::mp_bind_back<select_overrider_virtual_type, Registry>,
        MethodParameters, OverriderParameters>,
    void>;

template<class Method, class Rtti, std::size_t Index>
struct init_call_error {
    template<typename Arg, typename... Args>
    static auto fn(call_error& error, const Arg& arg, const Args&... args) {
        if constexpr (Index == 0u) {
            error.method = Rtti::template static_type<Method>();
            error.arity = sizeof...(args);
        }

        type_id arg_type_id;

        if constexpr (Rtti::template is_polymorphic<Arg>) {
            arg_type_id = Rtti::template dynamic_type<Arg>(arg);
        } else {
            arg_type_id = Rtti::template static_type<Arg>();
        }

        error.types[Index] = arg_type_id;

        init_call_error<Method, Rtti, Index + 1>::fn(error, args...);
    }

    static auto fn(call_error&) {
    }
};

template<class Method, class Rtti>
struct init_call_error<Method, Rtti, call_error::max_types> {
    static auto fn(call_error&) {
    }
};

template<class Method>
struct static_offsets;

template<class Method, typename = void>
struct has_static_offsets : std::false_type {};

template<class Method>
struct has_static_offsets<
    Method, std::void_t<decltype(static_offsets<Method>::slots)>>
    : std::true_type {};

template<class Registry, class Parameter>
struct using_same_registry : std::true_type {};

template<class Registry, typename Type, class OtherRegistry>
struct using_same_registry<Registry, virtual_ptr<Type, OtherRegistry>>
    : std::is_same<Registry, OtherRegistry> {};

template<class Registry, typename Type, class OtherRegistry>
struct using_same_registry<Registry, const virtual_ptr<Type, OtherRegistry>&>
    : std::is_same<Registry, OtherRegistry> {};

template<typename, class>
struct valid_method_parameter : std::true_type {};

template<typename T, class Registry>
struct valid_method_parameter<virtual_<T>, Registry>
    : std::bool_constant<
          has_vptr_fn<virtual_type<T, Registry>, Registry> ||
          Registry::rtti::template is_polymorphic<virtual_type<T, Registry>>> {
};

} // namespace detail

template<
    typename Method, typename ReturnType,
    class Registry = BOOST_OPENMETHOD_DEFAULT_REGISTRY>
class method;

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
class method<Name, auto(Parameters...)->ReturnType, Registry>
    : public std::conditional_t<
          Registry::deferred_static_rtti, detail::deferred_method_info,
          detail::method_info> {
    // Aliases used in implementation only. Everything extracted from template
    // arguments is capitalized like the arguments themselves.
    using RegistryType = Registry;
    using rtti = typename Registry::rtti;
    using DeclaredParameters = mp11::mp_list<Parameters...>;
    using CallParameters =
        boost::mp11::mp_transform<detail::remove_virtual, DeclaredParameters>;
    using VirtualParameters =
        typename detail::virtual_types<DeclaredParameters>;
    using Signature = auto(Parameters...) -> ReturnType;
    using FunctionPointer = auto (*)(detail::remove_virtual<Parameters>...)
        -> ReturnType;
    static constexpr auto Arity = boost::mp11::mp_count_if<
        mp11::mp_list<Parameters...>, detail::is_virtual>::value;

    // sanity checks
    static_assert(Arity > 0, "method has no virtual parameters");
    static_assert(
        (... && detail::using_same_registry<Registry, Parameters>::value),
        "method and parameters use different registries");
    static_assert(
        (... && detail::valid_method_parameter<Parameters, Registry>::value),
        "virtual_<> parameter is not polymorphic and no boost_openmethod_vptr "
        "function is available");
    //static_assert()

    type_id vp_type_ids[Arity];

    static std::size_t slots_strides[2 * Arity - 1];
    // Slots followed by strides. No stride for first virtual argument.
    // For 1-method: the offset of the method in the method table, which
    // contains a pointer to a function.
    // For multi-methods: the offset of the first virtual argument in the
    // method table, which contains a pointer to the corresponding cell in
    // the dispatch table, followed by the offset of the second argument and
    // the stride in the second dimension, etc.

    void resolve_type_ids();

    template<typename ArgType>
    auto vptr(const ArgType& arg) const -> vptr_type;

    template<class Error>
    auto check_static_offset(std::size_t actual, std::size_t expected) const
        -> void;

    template<typename MethodArgList, typename ArgType, typename... MoreArgTypes>
    auto resolve_uni(const ArgType& arg, const MoreArgTypes&... more_args) const
        -> detail::word;

    template<typename MethodArgList, typename ArgType, typename... MoreArgTypes>
    auto resolve_multi_first(
        const ArgType& arg, const MoreArgTypes&... more_args) const
        -> detail::word;

    template<
        std::size_t VirtualArg, typename MethodArgList, typename ArgType,
        typename... MoreArgTypes>
    auto resolve_multi_next(
        vptr_type dispatch, const ArgType& arg,
        const MoreArgTypes&... more_args) const -> detail::word;

    template<typename... ArgType>
    FunctionPointer resolve(const ArgType&... args) const;

    template<auto, typename>
    struct thunk;

    friend class generator;

    method();
    method(const method&) = delete;
    method(method&&) = delete;
    ~method();

    void resolve(); // perhaps virtual, perhaps not

  public:
    // Public aliases.
    using name_type = Name;
    using return_type = ReturnType;
    using function_type = ReturnType (*)(detail::remove_virtual<Parameters>...);

    static method fn;

    auto operator()(detail::remove_virtual<Parameters>... args) const
        -> ReturnType;

    template<auto>
    static function_type next;

    template<auto>
    static bool has_next();

    static BOOST_NORETURN auto
    fn_not_implemented(detail::remove_virtual<Parameters>... args)
        -> ReturnType;
    static BOOST_NORETURN auto
    fn_ambiguous(detail::remove_virtual<Parameters>... args) -> ReturnType;

  private:
    template<
        auto Overrider, typename OverriderReturn,
        typename... OverriderParameters>
    struct thunk<Overrider, OverriderReturn (*)(OverriderParameters...)> {
        static auto fn(detail::remove_virtual<Parameters>... arg) -> ReturnType;
        using OverriderVirtualParameters = detail::overrider_virtual_types<
            DeclaredParameters, mp11::mp_list<OverriderParameters...>,
            Registry>;
    };

    template<auto Function, typename FnReturnType>
    struct override_impl
        : std::conditional_t<
              Registry::deferred_static_rtti, detail::deferred_overrider_info,
              detail::overrider_info> {
        explicit override_impl(FunctionPointer* next = nullptr);
        void resolve_type_ids();

        inline static type_id vp_type_ids[Arity];
    };

    template<auto Function, typename FunctionType>
    struct override_aux;

    template<auto Function, typename FnReturnType, typename... FnParameters>
    struct override_aux<Function, FnReturnType (*)(FnParameters...)> {
        override_aux() {
            (void)&impl;
        }

        inline static override_impl<Function, FnReturnType> impl;
    };

    template<
        auto Function, class FnClass, typename FnReturnType,
        typename... FnParameters>
    struct override_aux<Function, FnReturnType (FnClass::*)(FnParameters...)> {
        override_aux() {
            (void)&impl;
        }

        static auto fn(FnClass* this_, FnParameters&&... args) -> FnReturnType {
            return (this_->*Function)(std::forward<FnParameters>(args)...);
        }

        inline static override_impl<fn, FnReturnType> impl{&next<Function>};
    };

  public:
    template<auto... Function>
    struct override {
        std::tuple<override_aux<Function, decltype(Function)>...> impl;
    };
};

// Following cannot be `inline static` becaused of MSVC (19.43) bug causing a
// "no appropriate default constructor available". Try this in CE:
//
// template<typename>
// class method {
//         method();
//         method(const method&) = delete;
//         method(method&&) = delete;
//         ~method();
//     public:
//         static inline method instance;
// };
// template method<void>;
// https://godbolt.org/z/GzEn486P7

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
method<Name, auto(Parameters...)->ReturnType, Registry>
    method<Name, auto(Parameters...)->ReturnType, Registry>::fn;

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
template<auto>
typename method<
    Name, auto(Parameters...)->ReturnType, Registry>::FunctionPointer
    method<Name, auto(Parameters...)->ReturnType, Registry>::next;

template<typename T>
constexpr bool is_method = std::is_base_of_v<detail::method_info, T>;

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
method<Name, auto(Parameters...)->ReturnType, Registry>::method() {
    this->slots_strides_ptr = slots_strides;

    if constexpr (!Registry::deferred_static_rtti) {
        resolve_type_ids();
    }

    this->vp_begin = vp_type_ids;
    this->vp_end = vp_type_ids + Arity;
    this->not_implemented = reinterpret_cast<void (*)()>(fn_not_implemented);

    if constexpr (Registry::template has_policy<policies::n2216>) {
        this->ambiguous = nullptr;
    } else {
        this->ambiguous = reinterpret_cast<void (*)()>(fn_ambiguous);
    }

    Registry::methods.push_back(*this);
}

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
void method<
    Name, auto(Parameters...)->ReturnType, Registry>::resolve_type_ids() {
    using namespace detail;
    this->method_type_id = rtti::template static_type<method>();
    this->return_type_id = rtti::template static_type<
        typename virtual_traits<ReturnType, Registry>::virtual_type>();
    init_type_ids<
        Registry,
        mp11::mp_transform_q<
            mp11::mp_bind_back<virtual_type, Registry>,
            VirtualParameters>>::fn(this->vp_type_ids);
}

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
std::size_t method<Name, auto(Parameters...)->ReturnType, Registry>::
    slots_strides[2 * Arity - 1];

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
method<Name, auto(Parameters...)->ReturnType, Registry>::~method() {
    Registry::methods.remove(*this);
}

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
template<class Error>
auto method<Name, auto(Parameters...)->ReturnType, Registry>::
    check_static_offset(std::size_t actual, std::size_t expected) const
    -> void {
    using namespace detail;
    using error_handler =
        typename Registry::template policy<policies::error_handler>;

    if constexpr (is_not_void<error_handler>) {
        if (actual != expected) {
            Error error;
            error.method = Registry::rtti::template static_type<method>();
            error.expected = this->slots_strides[0];
            error.actual = actual;
            error_handler::error(error);

            abort();
        }
    }
}

// -----------------------------------------------------------------------------
// method dispatch

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
BOOST_FORCEINLINE auto
method<Name, auto(Parameters...)->ReturnType, Registry>::operator()(
    detail::remove_virtual<Parameters>... args) const -> ReturnType {
    using namespace detail;
    auto pf = resolve(parameter_traits<Parameters, Registry>::peek(args)...);

    return pf(std::forward<remove_virtual<Parameters>>(args)...);
}

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
template<typename... ArgType>
BOOST_FORCEINLINE typename method<
    Name, auto(Parameters...)->ReturnType, Registry>::FunctionPointer
method<Name, auto(Parameters...)->ReturnType, Registry>::resolve(
    const ArgType&... args) const {
    using namespace detail;

    Registry::check_initialized();

    void (*pf)();

    if constexpr (Arity == 1) {
        pf = resolve_uni<mp11::mp_list<Parameters...>, ArgType...>(args...).pf;
    } else {
        pf = resolve_multi_first<mp11::mp_list<Parameters...>, ArgType...>(
                 args...)
                 .pf;
    }

    return reinterpret_cast<FunctionPointer>(pf);
}

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
template<typename ArgType>
BOOST_FORCEINLINE auto
method<Name, auto(Parameters...)->ReturnType, Registry>::vptr(
    const ArgType& arg) const -> vptr_type {
    if constexpr (detail::is_virtual_ptr<ArgType>) {
        return arg.vptr();
    } else {
        return detail::acquire_vptr<Registry>(arg);
    }
}

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
template<typename MethodArgList, typename ArgType, typename... MoreArgTypes>
BOOST_FORCEINLINE auto
method<Name, auto(Parameters...)->ReturnType, Registry>::resolve_uni(
    const ArgType& arg, const MoreArgTypes&... more_args) const
    -> detail::word {

    using namespace detail;
    using namespace boost::mp11;

    if constexpr (is_virtual<mp_first<MethodArgList>>::value) {
        vptr_type vtbl = vptr<ArgType>(arg);

        if constexpr (has_static_offsets<method>::value) {
            if constexpr (Registry::template has_policy<
                              policies::runtime_checks>) {
                check_static_offset<static_slot_error>(
                    static_offsets<method>::slots[0], this->slots_strides[0]);
            }
            return vtbl[static_offsets<method>::slots[0]];
        } else {
            return vtbl[this->slots_strides[0]];
        }
    } else {
        return resolve_uni<mp_rest<MethodArgList>>(more_args...);
    }
}

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
template<typename MethodArgList, typename ArgType, typename... MoreArgTypes>
BOOST_FORCEINLINE auto
method<Name, auto(Parameters...)->ReturnType, Registry>::resolve_multi_first(
    const ArgType& arg, const MoreArgTypes&... more_args) const
    -> detail::word {

    using namespace detail;
    using namespace boost::mp11;

    if constexpr (is_virtual<mp_first<MethodArgList>>::value) {
        vptr_type vtbl = vptr<ArgType>(arg);
        std::size_t slot;

        if constexpr (has_static_offsets<method>::value) {
            slot = static_offsets<method>::slots[0];
            if constexpr (Registry::template has_policy<
                              policies::runtime_checks>) {
                check_static_offset<static_slot_error>(
                    static_offsets<method>::slots[0], this->slots_strides[0]);
            }
        } else {
            slot = this->slots_strides[0];
        }

        // The first virtual parameter is special.  Since its stride is
        // 1, there is no need to store it. Also, the method table
        // contains a pointer into the multi-dimensional dispatch table,
        // already resolved to the appropriate group.
        auto dispatch = vtbl[slot].pw;
        return resolve_multi_next<1, mp_rest<MethodArgList>, MoreArgTypes...>(
            dispatch, more_args...);
    } else {
        return resolve_multi_first<mp_rest<MethodArgList>, MoreArgTypes...>(
            more_args...);
    }
}

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
template<
    std::size_t VirtualArg, typename MethodArgList, typename ArgType,
    typename... MoreArgTypes>
BOOST_FORCEINLINE auto
method<Name, auto(Parameters...)->ReturnType, Registry>::resolve_multi_next(
    vptr_type dispatch, const ArgType& arg,
    const MoreArgTypes&... more_args) const -> detail::word {

    using namespace detail;
    using namespace boost::mp11;

    if constexpr (is_virtual<mp_first<MethodArgList>>::value) {
        vptr_type vtbl = vptr<ArgType>(arg);
        std::size_t slot, stride;

        if constexpr (has_static_offsets<method>::value) {
            slot = static_offsets<method>::slots[VirtualArg];
            stride = static_offsets<method>::strides[VirtualArg - 1];
            if constexpr (Registry::template has_policy<
                              policies::runtime_checks>) {
                check_static_offset<static_slot_error>(
                    this->slots_strides[VirtualArg], slot);
                check_static_offset<static_stride_error>(
                    this->slots_strides[2 * VirtualArg], stride);
            }
        } else {
            slot = this->slots_strides[VirtualArg];
            stride = this->slots_strides[Arity + VirtualArg - 1];
        }

        dispatch = dispatch + vtbl[slot].i * stride;
    }

    if constexpr (VirtualArg + 1 == Arity) {
        return *dispatch;
    } else {
        return resolve_multi_next<
            VirtualArg + 1, mp_rest<MethodArgList>, MoreArgTypes...>(
            dispatch, more_args...);
    }
}

// -----------------------------------------------------------------------------
// Error handling

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
template<auto Fn>
inline auto method<Name, auto(Parameters...)->ReturnType, Registry>::has_next()
    -> bool {
    if (next<Fn> == fn_not_implemented) {
        return false;
    }

    if constexpr (!Registry::template has_policy<policies::n2216>) {
        if (next<Fn> == fn_ambiguous) {
            return false;
        }
    }

    return true;
}

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
BOOST_NORETURN auto
method<Name, auto(Parameters...)->ReturnType, Registry>::fn_not_implemented(
    detail::remove_virtual<Parameters>... args) -> ReturnType {
    if constexpr (Registry::template has_policy<policies::error_handler>) {
        not_implemented_error error;
        detail::init_call_error<method, rtti, 0u>::fn(
            error,
            detail::parameter_traits<Parameters, Registry>::peek(args)...);
        Registry::template policy<policies::error_handler>::error(error);
    }

    abort(); // in case user handler "forgets" to abort
}

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
BOOST_NORETURN auto
method<Name, auto(Parameters...)->ReturnType, Registry>::fn_ambiguous(
    detail::remove_virtual<Parameters>... args) -> ReturnType {
    if constexpr (Registry::template has_policy<policies::error_handler>) {
        ambiguous_error error;
        detail::init_call_error<method, rtti, 0u>::fn(
            error,
            detail::parameter_traits<Parameters, Registry>::peek(args)...);
        Registry::template policy<policies::error_handler>::error(error);
    }

    abort(); // in case user handler "forgets" to abort
}

// -----------------------------------------------------------------------------
// thunk

namespace detail {
template<typename T, typename U>
constexpr bool is_virtual_ptr_compatible =
    is_virtual_ptr<T> == is_virtual_ptr<U>;
}

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
template<
    auto Overrider, typename OverriderReturn, typename... OverriderParameters>
auto method<Name, auto(Parameters...)->ReturnType, Registry>::
    thunk<Overrider, OverriderReturn (*)(OverriderParameters...)>::fn(
        detail::remove_virtual<Parameters>... arg) -> ReturnType {
    using namespace detail;
    static_assert(
        (true && ... &&
         is_virtual_ptr_compatible<Parameters, OverriderParameters>),
        "virtual_ptr mismatch");
    return Overrider(
        detail::parameter_traits<Parameters, Registry>::template cast<
            OverriderParameters>(
            std::forward<detail::remove_virtual<Parameters>>(arg))...);
}

// -----------------------------------------------------------------------------
// overriders

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
template<auto Function, typename FnReturnType>
method<Name, ReturnType(Parameters...), Registry>::override_impl<
    Function, FnReturnType>::override_impl(FunctionPointer* p_next) {
    using namespace detail;

    // static variable this->method below is zero-initialized but gcc and clang
    // don't always see that.

#ifdef BOOST_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wuninitialized"
#endif

#ifdef BOOST_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

    if (overrider_info::method) {
        BOOST_ASSERT(overrider_info::method == &fn);
        return;
    }

#ifdef BOOST_CLANG
#pragma clang diagnostic pop
#endif

#ifdef BOOST_GCC
#pragma GCC diagnostic pop
#endif

    overrider_info::method = &fn;

    if constexpr (!Registry::deferred_static_rtti) {
        resolve_type_ids();
    }

    this->next = reinterpret_cast<void (**)()>(
        p_next ? p_next : &method::next<Function>);

    using Thunk = thunk<Function, decltype(Function)>;
    this->pf = reinterpret_cast<void (*)()>(Thunk::fn);

    this->vp_begin = vp_type_ids;
    this->vp_end = vp_type_ids + Arity;

    fn.specs.push_back(*this);
}

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
template<auto Function, typename FnReturnType>
void method<Name, auto(Parameters...)->ReturnType, Registry>::override_impl<
    Function, FnReturnType>::resolve_type_ids() {
    using namespace detail;

    this->return_type = Registry::rtti::template static_type<
        typename virtual_traits<FnReturnType, Registry>::virtual_type>();
    this->type = Registry::rtti::template static_type<decltype(Function)>();
    using Thunk = thunk<Function, decltype(Function)>;
    detail::
        init_type_ids<Registry, typename Thunk::OverriderVirtualParameters>::fn(
            this->vp_type_ids);
}

namespace aliases {

using boost::openmethod::final_virtual_ptr;
using boost::openmethod::virtual_;
using boost::openmethod::virtual_ptr;

} // namespace aliases

} // namespace boost::openmethod

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
