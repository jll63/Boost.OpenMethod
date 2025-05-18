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
// Registering classes

namespace detail {

// =============================================================================
// Helpers

template<class Registry, class Class>
constexpr bool is_polymorphic = Registry::Rtti::template is_polymorphic<Class>;

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

template<class Registry, class Class>
auto collect_static_type_id() -> type_id {
    using Rtti = typename Registry::Rtti;

    if constexpr (Registry::template has_policy<
                      policies::deferred_static_rtti>) {
        return reinterpret_cast<type_id>(Rtti::template static_type<Class>);
    } else {
        return Rtti::template static_type<Class>();
    }
}

template<class TypeList, class Registry>
struct type_id_list;

template<typename... T, class Registry>
struct type_id_list<mp11::mp_list<T...>, Registry> {
    // If using deferred 'static_type', add an extra element in 'value',
    // default-initialized to zero, indicating the ids need to be resolved. Set
    // to 1 after this is done.
    static constexpr std::size_t values = sizeof...(T) +
        Registry::template has_policy<policies::deferred_static_rtti>;
    static type_id value[values];
    static type_id* begin;
    static type_id* end;
};

template<typename... T, class Registry>
type_id type_id_list<mp11::mp_list<T...>, Registry>::value[values] = {
    collect_static_type_id<Registry, T>()...};

template<typename... T, class Registry>
type_id* type_id_list<mp11::mp_list<T...>, Registry>::begin = value;

template<typename... T, class Registry>
type_id* type_id_list<mp11::mp_list<T...>, Registry>::end =
    value + sizeof...(T);

template<class Registry>
struct type_id_list<mp11::mp_list<>, Registry> {
    static constexpr type_id* const begin = nullptr;
    static constexpr auto end = begin;
};

template<class...>
struct class_declaration_aux;

template<class Registry, class Class, typename... Bases>
struct class_declaration_aux<Registry, mp11::mp_list<Class, Bases...>>
    : class_info {
    class_declaration_aux() {
        this->type = collect_static_type_id<Registry, Class>();
        this->first_base =
            type_id_list<mp11::mp_list<Bases...>, Registry>::begin;
        this->last_base = type_id_list<mp11::mp_list<Bases...>, Registry>::end;
        Registry::classes.push_back(*this);
        this->is_abstract = std::is_abstract_v<Class>;
        this->static_vptr = &Registry::template static_vptr<Class>;
    }

    ~class_declaration_aux() {
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
        return Registry::Rtti::template dynamic_cast_ref<D>(
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
                detail::class_declaration_aux,
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

template<class Class>
constexpr bool has_vptr_fn = std::is_same_v<
    decltype(boost_openmethod_vptr(std::declval<const Class&>())), vptr_type>;

template<class Registry, class ArgType>
decltype(auto) acquire_vptr(const ArgType& arg) {
    if constexpr (detail::has_vptr_fn<ArgType>) {
        return boost_openmethod_vptr(arg);
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
  public:
    using traits = virtual_traits<Class&, Registry>;
    using element_type = Class;
    static constexpr bool is_smart_ptr = false;

    static constexpr bool use_indirect_vptrs =
        Registry::template has_policy<policies::indirect_vptr>;

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
    virtual_ptr_impl(Other& other, const vptr_type& vp)
        : vp(box_vptr<use_indirect_vptrs>(vp)), obj(&other) {
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
            traits::template cast<Other&>(*obj), unbox_vptr(vp));
    }

    template<class, class>
    friend struct virtual_traits;

  protected:
    std::conditional_t<use_indirect_vptrs, const vptr_type*, vptr_type> vp;
    Class* obj;
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

    template<typename Arg>
    virtual_ptr_impl(Arg&& obj, decltype(vp) other_vp)
        : vp(other_vp), obj(std::forward<Arg>(obj)) {
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
        using Rtti = typename Registry::Rtti;

        static_assert(
            std::is_base_of_v<element_type, other_class> ||
            std::is_base_of_v<other_class, element_type>);

        if constexpr (
            Registry::RuntimeChecks &&
            is_polymorphic<Registry, typename impl::traits::virtual_type> &&
            is_polymorphic<Registry, other_class>) {

            // check that dynamic type == static type
            auto static_type = Rtti::template static_type<other_class>();
            type_id dynamic_type = Rtti::dynamic_type(other_traits::peek(obj));

            if (dynamic_type != static_type) {
                type_mismatch_error error;
                error.type = dynamic_type;
                using ErrorHandler = typename Registry::ErrorHandler;

                if constexpr (is_not_void<ErrorHandler>) {
                    ErrorHandler::error(error);
                }

                abort();
            }
        }

        return virtual_ptr(
            std::forward<Other>(obj),
            detail::box_vptr<impl::use_indirect_vptrs>(
                Registry::template static_vptr<other_class>));
    }

    auto vptr() const {
        return detail::unbox_vptr(this->vp);
    }
};

template<class Class>
virtual_ptr(Class&) -> virtual_ptr<Class, BOOST_OPENMETHOD_DEFAULT_REGISTRY>;

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

} // namespace detail

template<
    typename Method, typename ReturnType,
    class Registry = BOOST_OPENMETHOD_DEFAULT_REGISTRY>
class method;

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
class method<Name(Parameters...), ReturnType, Registry>
    : public detail::method_info {
    // Aliases used in implementation only. Everything extracted from template
    // arguments is capitalized like the arguments themselves.
    using Rtti = typename Registry::Rtti;
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
    static_assert(Arity > 0, "method must have at least one virtual argument");
    static_assert(
        (true && ... &&
         detail::using_same_registry<Registry, Parameters>::value));

    static std::size_t slots_strides[2 * Arity - 1];
    // Slots followed by strides. No stride for first virtual argument.
    // For 1-method: the offset of the method in the method table, which
    // contains a pointer to a function.
    // For multi-methods: the offset of the first virtual argument in the
    // method table, which contains a pointer to the corresponding cell in
    // the dispatch table, followed by the offset of the second argument and
    // the stride in the second dimension, etc.

    template<typename ArgType>
    auto vptr(const ArgType& arg) const -> vptr_type;

    template<class Error>
    auto check_static_offset(std::size_t actual, std::size_t expected) const
        -> void;

    template<typename MethodArgList, typename ArgType, typename... MoreArgTypes>
    auto resolve_uni(const ArgType& arg, const MoreArgTypes&... more_args) const
        -> std::uintptr_t;

    template<typename MethodArgList, typename ArgType, typename... MoreArgTypes>
    auto resolve_multi_first(
        const ArgType& arg, const MoreArgTypes&... more_args) const
        -> std::uintptr_t;

    template<
        std::size_t VirtualArg, typename MethodArgList, typename ArgType,
        typename... MoreArgTypes>
    auto resolve_multi_next(
        vptr_type dispatch, const ArgType& arg,
        const MoreArgTypes&... more_args) const -> std::uintptr_t;

    template<typename... ArgType>
    FunctionPointer resolve(const ArgType&... args) const;

    static BOOST_NORETURN auto
    not_implemented_handler(detail::remove_virtual<Parameters>... args)
        -> ReturnType;

    template<auto, typename>
    struct thunk;

    friend class generator;

    method();
    method(const method&) = delete;
    method(method&&) = delete;
    ~method();

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

  private:
    template<
        auto Overrider, typename OverriderReturn,
        typename... OverriderParameters>
    struct thunk<Overrider, OverriderReturn (*)(OverriderParameters...)> {
        static auto fn(detail::remove_virtual<Parameters>... arg) -> ReturnType;
        using OverriderParameterTypeIds = detail::type_id_list<
            detail::overrider_virtual_types<
                DeclaredParameters, mp11::mp_list<OverriderParameters...>,
                Registry>,
            Registry>;
    };

    template<auto Function, typename FnReturnType>
    struct override_impl {
        explicit override_impl(FunctionPointer* next = nullptr);
    };

    template<auto Function, typename FunctionType>
    struct override_aux;

    template<auto Function, typename FnReturnType, typename... FnParameters>
    struct override_aux<Function, FnReturnType (*)(FnParameters...)>
        : override_impl<Function, FnReturnType> {};

    template<
        auto Function, class FnClass, typename FnReturnType,
        typename... FnParameters>
    struct override_aux<Function, FnReturnType (FnClass::*)(FnParameters...)> {
        static auto fn(FnClass* this_, FnParameters&&... args) -> FnReturnType {
            return (this_->*Function)(std::forward<FnParameters>(args)...);
        }

        override_impl<fn, FnReturnType> impl{&next<Function>};
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
method<Name(Parameters...), ReturnType, Registry>
    method<Name(Parameters...), ReturnType, Registry>::fn;

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
template<auto>
typename method<Name(Parameters...), ReturnType, Registry>::FunctionPointer
    method<Name(Parameters...), ReturnType, Registry>::next;

template<typename T>
constexpr bool is_method = std::is_base_of_v<detail::method_info, T>;

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
method<Name(Parameters...), ReturnType, Registry>::method() {
    method_info::slots_strides_ptr = slots_strides;

    using virtual_type_ids = detail::type_id_list<
        boost::mp11::mp_transform_q<
            boost::mp11::mp_bind_back<detail::virtual_type, Registry>,
            VirtualParameters>,
        Registry>;
    method_info::vp_begin = virtual_type_ids::begin;
    method_info::vp_end = virtual_type_ids::end;
    method_info::not_implemented = (void*)not_implemented_handler;
    method_info::method_type = Rtti::template static_type<method>();
    method_info::return_type = Rtti::template static_type<
        typename virtual_traits<ReturnType, Registry>::virtual_type>();
    Registry::methods.push_back(*this);
}

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
std::size_t method<
    Name(Parameters...), ReturnType, Registry>::slots_strides[2 * Arity - 1];

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
method<Name(Parameters...), ReturnType, Registry>::~method() {
    Registry::methods.remove(*this);
}

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
template<class Error>
auto method<Name(Parameters...), ReturnType, Registry>::check_static_offset(
    std::size_t actual, std::size_t expected) const -> void {
    using namespace detail;
    using ErrorHandler =
        typename Registry::template policy<policies::error_handler>;

    if constexpr (is_not_void<ErrorHandler>) {
        if (actual != expected) {
            Error error;
            error.method = Registry::Rtti::template static_type<method>();
            error.expected = this->slots_strides[0];
            error.actual = actual;
            ErrorHandler::error(error);

            abort();
        }
    }
}

// -----------------------------------------------------------------------------
// method dispatch

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
BOOST_FORCEINLINE auto
method<Name(Parameters...), ReturnType, Registry>::operator()(
    detail::remove_virtual<Parameters>... args) const -> ReturnType {
    using namespace detail;
    auto pf = resolve(parameter_traits<Parameters, Registry>::peek(args)...);

    return pf(std::forward<remove_virtual<Parameters>>(args)...);
}

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
template<typename... ArgType>
BOOST_FORCEINLINE
    typename method<Name(Parameters...), ReturnType, Registry>::FunctionPointer
    method<Name(Parameters...), ReturnType, Registry>::resolve(
        const ArgType&... args) const {
    using namespace detail;

    std::uintptr_t pf;

    if constexpr (Arity == 1) {
        pf = resolve_uni<mp11::mp_list<Parameters...>, ArgType...>(args...);
    } else {
        pf = resolve_multi_first<mp11::mp_list<Parameters...>, ArgType...>(
            args...);
    }

    return reinterpret_cast<FunctionPointer>(pf);
}

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
template<typename ArgType>
BOOST_FORCEINLINE auto method<Name(Parameters...), ReturnType, Registry>::vptr(
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
method<Name(Parameters...), ReturnType, Registry>::resolve_uni(
    const ArgType& arg, const MoreArgTypes&... more_args) const
    -> std::uintptr_t {

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
method<Name(Parameters...), ReturnType, Registry>::resolve_multi_first(
    const ArgType& arg, const MoreArgTypes&... more_args) const
    -> std::uintptr_t {

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
        auto dispatch = reinterpret_cast<vptr_type>(vtbl[slot]);
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
method<Name(Parameters...), ReturnType, Registry>::resolve_multi_next(
    vptr_type dispatch, const ArgType& arg,
    const MoreArgTypes&... more_args) const -> std::uintptr_t {

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

        dispatch = dispatch + vtbl[slot] * stride;
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

namespace detail {

template<class Registry, class Class>
auto error_type_id(const Class& obj) {
    if constexpr (Registry::Rtti::template is_polymorphic<Class>) {
        return Registry::Rtti::template dynamic_type<Class>(obj);
    } else {
        return Registry::Rtti::template static_type<void>();
    }
}

} // namespace detail

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
BOOST_NORETURN auto
method<Name(Parameters...), ReturnType, Registry>::not_implemented_handler(
    detail::remove_virtual<Parameters>... args) -> ReturnType {
    if constexpr (Registry::template has_policy<policies::error_handler>) {
        not_implemented_error error;
        error.method = Registry::Rtti::template static_type<method>();
        error.arity = Arity;
        type_id types[sizeof...(args)];
        auto ti_iter = types;
        (...,
         (*ti_iter++ = detail::error_type_id<Registry>(
              detail::parameter_traits<Parameters, Registry>::peek(args))));
        std::copy_n(
            types,
            (std::min)(sizeof...(args), not_implemented_error::max_types),
            &error.types[0]);
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
auto method<Name(Parameters...), ReturnType, Registry>::
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
method<Name(Parameters...), ReturnType, Registry>::override_impl<
    Function, FnReturnType>::override_impl(FunctionPointer* p_next) {
    using namespace detail;

    // Work around MSVC bug: using &next<Function> as a default value
    // for 'next' confuses it about Parameters not being expanded.
    if (!p_next) {
        p_next = &next<Function>;
    }

    static overrider_info info;

    if (info.method) {
        BOOST_ASSERT(info.method == &fn);
        return;
    }

    info.method = &fn;
    info.return_type = Registry::Rtti::template static_type<
        typename virtual_traits<FnReturnType, Registry>::virtual_type>();
    info.type = Registry::Rtti::template static_type<decltype(Function)>();
    info.next = reinterpret_cast<void**>(p_next);
    using Thunk = thunk<Function, decltype(Function)>;
    info.pf = (void*)Thunk::fn;
    info.vp_begin = Thunk::OverriderParameterTypeIds::begin;
    info.vp_end = Thunk::OverriderParameterTypeIds::end;
    fn.specs.push_back(info);
}

} // namespace boost::openmethod

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
