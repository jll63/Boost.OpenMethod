#ifndef BOOST_OPENMETHOD_CORE_HPP
#define BOOST_OPENMETHOD_CORE_HPP

#include <functional>
#include <memory>

#include <boost/assert.hpp>

#include <boost/openmethod/policy.hpp>

#ifndef BOOST_OPENMETHOD_DEFAULT_POLICY
#define BOOST_OPENMETHOD_DEFAULT_POLICY ::boost::openmethod::policies::default_
#endif

#include <boost/openmethod/detail/types.hpp>

namespace boost::openmethod {

// =============================================================================
// Registering classes

namespace detail {

template<class Policy, class Class>
auto collect_static_type_id() -> type_id {
    if constexpr (std::is_base_of_v<policies::deferred_static_rtti, Policy>) {
        return reinterpret_cast<type_id>(Policy::template static_type<Class>);
    } else {
        return Policy::template static_type<Class>();
    }
}

template<class TypeList, class Policy>
struct type_id_list;

template<typename... T, class Policy>
struct type_id_list<types<T...>, Policy> {
    // If using deferred 'static_type', add an extra element in 'value',
    // default-initialized to zero, indicating the ids need to be resolved. Set
    // to 1 after this is done.
    static constexpr std::size_t values = sizeof...(T) +
        std::is_base_of_v<policies::deferred_static_rtti, Policy>;
    static type_id value[values];
    static type_id* begin;
    static type_id* end;
};

template<typename... T, class Policy>
type_id type_id_list<types<T...>, Policy>::value[values] = {
    collect_static_type_id<Policy, T>()...};

template<typename... T, class Policy>
type_id* type_id_list<types<T...>, Policy>::begin = value;

template<typename... T, class Policy>
type_id* type_id_list<types<T...>, Policy>::end = value + sizeof...(T);

template<class Policy>
struct type_id_list<types<>, Policy> {
    static constexpr type_id* const begin = nullptr;
    static constexpr auto end = begin;
};

template<class...>
struct class_declaration_aux;

template<class Policy, class Class, typename... Bases>
struct class_declaration_aux<Policy, types<Class, Bases...>> : class_info {
    class_declaration_aux() {
        this->type = collect_static_type_id<Policy, Class>();
        this->first_base = type_id_list<types<Bases...>, Policy>::begin;
        this->last_base = type_id_list<types<Bases...>, Policy>::end;
        Policy::classes.push_back(*this);
        this->is_abstract = std::is_abstract_v<Class>;
        this->static_vptr = &Policy::template static_vptr<Class>;
    }

    ~class_declaration_aux() {
        Policy::classes.remove(*this);
    }
};

// Collect the base classes of a list of classes. The result is a mp11 map that
// associates each class to a list starting with the class itself, followed by
// all its bases, as per std::is_base_of. Thus the list includes the class
// itself at least twice: at the front, and down the list, as its own improper
// base. The direct and indirect bases are all included. The runtime will
// extract the direct proper bases.
template<typename... Cs>
using inheritance_map = types<boost::mp11::mp_push_front<
    boost::mp11::mp_filter_q<
        boost::mp11::mp_bind_back<std::is_base_of, Cs>, types<Cs...>>,
    Cs>...>;

template<class Policy, class... Classes>
struct use_classes_aux;

template<class Policy, class... Classes>
struct use_classes_aux<Policy, types<Classes...>> {
    using type = boost::mp11::mp_apply<
        std::tuple,
        boost::mp11::mp_transform_q<
            boost::mp11::mp_bind_front<class_declaration_aux, Policy>,
            boost::mp11::mp_apply<inheritance_map, types<Classes...>>>>;
};

template<class Policy, class... Classes, class... MoreClassLists>
struct use_classes_aux<Policy, types<types<Classes...>, MoreClassLists...>>
    : use_classes_aux<
          Policy, boost::mp11::mp_append<types<Classes...>, MoreClassLists...>>

{};

template<typename T>
struct is_policy_fn : std::is_base_of<policies::abstract_policy, T> {};

template<typename... T>
struct is_policy_fn<types<T...>> : std::false_type {};

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

template<class Policy, class D, class B>
auto optimal_cast(B&& obj) -> decltype(auto) {
    if constexpr (requires_dynamic_cast<B, D>) {
        return Policy::template dynamic_cast_ref<D>(obj);
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

template<typename T, class Policy>
using virtual_type = typename virtual_traits<T, Policy>::virtual_type;

template<typename MethodArgList>
using virtual_types = boost::mp11::mp_transform<
    remove_virtual, boost::mp11::mp_filter<detail::is_virtual, MethodArgList>>;

template<typename T, class Policy>
struct parameter_traits {
    static auto rarg(const T& arg) {
        return nullptr;
    }

    template<typename>
    static decltype(auto) cast(T value) {
        return std::forward<T>(value);
    }
};

template<typename T, class Policy>
struct parameter_traits<virtual_<T>, Policy> : virtual_traits<T, Policy> {};

template<class Class, class Policy>
struct parameter_traits<virtual_ptr<Class, Policy>, Policy>
    : virtual_traits<virtual_ptr<Class, Policy>, Policy> {};

template<class Class, class Policy>
struct parameter_traits<const virtual_ptr<Class, Policy>&, Policy>
    : virtual_traits<const virtual_ptr<Class, Policy>&, Policy> {};

} // namespace detail

// =============================================================================
// virtual_traits

template<typename T, class Policy>
struct virtual_traits {
    using virtual_type = void;
};

template<typename T, class Policy>
struct virtual_traits<T&, Policy> {
    using virtual_type = std::remove_cv_t<T>;

    static auto rarg(const T& arg) -> const T& {
        return arg;
    }

    template<typename D>
    static auto cast(T& obj) -> D& {
        return detail::optimal_cast<Policy, D&>(obj);
    }
};

template<typename T, class Policy>
struct virtual_traits<T&&, Policy> {
    using virtual_type = T;

    static auto rarg(const T& arg) -> const T& {
        return arg;
    }

    template<typename D>
    static auto cast(T&& obj) -> D&& {
        return detail::optimal_cast<Policy, D&&>(obj);
    }
};

// For covariant return types.
template<typename T, class Policy>
struct virtual_traits<T*, Policy> {
    using virtual_type = std::remove_cv_t<T>;
};

template<class... Classes>
using use_classes = typename detail::use_classes_aux<
    boost::mp11::mp_at<
        detail::types<Classes..., BOOST_OPENMETHOD_DEFAULT_POLICY>,
        boost::mp11::mp_find_if<
            detail::types<Classes..., BOOST_OPENMETHOD_DEFAULT_POLICY>,
            detail::is_policy_fn>>,
    boost::mp11::mp_remove_if<
        detail::types<Classes...>, detail::is_policy_fn>>::type;

// =============================================================================
// virtual_ptr

template<class Class, class Policy>
struct virtual_ptr_traits {
    using element_type = Class;
    static bool constexpr is_smart_ptr = false;

    static auto dynamic_type(const Class& obj) -> type_id {
        return Policy::dynamic_type(obj);
    }

    template<typename Other>
    static auto cast(const virtual_ptr<Class, Policy>& ptr)
        -> virtual_ptr<Other, Policy> {
        return virtual_ptr<Other, Policy>(
            detail::optimal_cast<Policy, Other&>(*ptr), ptr.vptr());
    }
};

namespace detail {

template<class Class, class Policy>
struct is_virtual<virtual_ptr<Class, Policy>> : std::true_type {};

template<class Class, class Policy>
struct is_virtual<const virtual_ptr<Class, Policy>&> : std::true_type {};

template<typename>
struct is_virtual_ptr_aux : std::false_type {};

template<class Class, class Policy>
struct is_virtual_ptr_aux<virtual_ptr<Class, Policy>> : std::true_type {};

template<class Class, class Policy>
struct is_virtual_ptr_aux<const virtual_ptr<Class, Policy>&> : std::true_type {
};

template<typename T>
constexpr bool is_virtual_ptr = detail::is_virtual_ptr_aux<T>::value;

template<
    class Type, class Policy,
    bool IsSmartPtr = virtual_ptr_traits<Type, Policy>::is_smart_ptr>
class virtual_ptr_impl;

template<class Class, class Policy>
class virtual_ptr_impl<Class, Policy, false> {
    template<class, class>
    friend struct virtual_traits;

    template<class, class>
    friend struct virtual_ptr_traits;

  protected:
    static constexpr bool is_indirect = Policy::is_indirect;

    using vptr_type = std::conditional_t<
        is_indirect, std::uintptr_t const* const*, std::uintptr_t const*>;

    Class* obj;
    vptr_type vp;

  public:
    using traits = virtual_ptr_traits<Class, Policy>;
    using element_type = typename traits::element_type;

    template<class Other>
    virtual_ptr_impl(Other& other) {
        obj = &other;
        vp = Policy::dynamic_vptr(other);
    }

    template<class Other>
    virtual_ptr_impl(const virtual_ptr<Other, Policy>& other)
        : obj(other.get()), vp(other.vp) {
    }

    template<class Other>
    virtual_ptr_impl(virtual_ptr<Other, Policy>& other)
        : obj(other.get()), vp(other.vp) {
        // Why is this needed? Consider this conversion conversion from
        // smart to dumb pointer:
        //      virtual_ptr<std::shared_ptr<const Node>> p = ...;
        //      virtual_ptr<const Node> q = p;
        // Since 'p' is not const, in the absence of this ctor,
        // virtual_ptr_impl(Other&) would be preferred to
        // virtual_ptr_impl(const virtual_ptr<Other, Policy>& other), and
        // that is incorrect.
    }

    template<class Other>
    virtual_ptr_impl(Other& other, vptr_type vp) : obj(&other), vp(vp) {
    }

    Class* get() const {
        return obj;
    }

    auto operator->() const {
        return obj;
    }

    auto operator*() const -> element_type& {
        return *obj;
    }

    auto pointer() const -> const Class*& {
        return obj;
    }
};

template<class Class, class Policy>
class virtual_ptr_impl<Class, Policy, true> {
  public:
    using traits = virtual_ptr_traits<Class, Policy>;
    using element_type = typename traits::element_type;

    template<class, class>
    friend class virtual_ptr;

    template<class, typename>
    friend struct virtual_traits;

  protected:
    using vptr_type =
        decltype(Policy::dynamic_vptr(std::declval<virtual_ptr_impl>()));

    static constexpr bool is_indirect =
        std::is_same_v<vptr_type, indirect_vptr_type>;

    Class obj;
    vptr_type vp;

  public:
    template<class Other>
    virtual_ptr_impl(
        const Other& other,
        std::enable_if_t<virtual_ptr_traits<Other, Policy>::is_smart_ptr, int> =
            0) {
        vp = Policy::dynamic_vptr(*other);
        obj = other;
    }

    template<class Other>
    virtual_ptr_impl(
        Other&& other,
        std::enable_if_t<virtual_ptr_traits<Other, Policy>::is_smart_ptr, int> =
            0) {
        vp = Policy::dynamic_vptr(*other);
        obj = std::move(other);
    }

    template<class Other>
    virtual_ptr_impl(const virtual_ptr<Other, Policy>& other)
        : obj(other.obj), vp(other.vp) {
    }

    template<class Other>
    virtual_ptr_impl(virtual_ptr<Other, Policy>&& other)
        : obj(std::move(other.obj)), vp(other.vp) {
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
    virtual_ptr_impl(Arg&& obj, vptr_type vp)
        : obj(std::forward<Arg>(obj)), vp(vp) {
    }
};

} // namespace detail

template<class Policy, class Class>
inline auto final_virtual_ptr(Class&& obj);

template<class Class, class Policy = BOOST_OPENMETHOD_DEFAULT_POLICY>
class virtual_ptr : public detail::virtual_ptr_impl<Class, Policy> {
    using impl = detail::virtual_ptr_impl<Class, Policy>;

    template<class, typename>
    friend struct virtual_ptr_traits;

  public:
    using detail::virtual_ptr_impl<Class, Policy>::virtual_ptr_impl;

    template<class, class, bool>
    friend class detail::virtual_ptr_impl;

    static constexpr bool is_smart_ptr = impl::traits::is_smart_ptr;

    template<typename Other>
    decltype(auto) cast() & {
        return virtual_ptr_traits<Class, Policy>::template cast<Other>(*this);
    }

    template<typename Other>
    decltype(auto) cast() const& {
        return virtual_ptr_traits<Class, Policy>::template cast<Other>(*this);
    }

    template<typename Other>
    decltype(auto) cast() && {
        return virtual_ptr_traits<Class, Policy>::template cast<Other>(
            std::move(*this));
    }

    template<class Other>
    static auto final(Other&& obj) {
        using namespace detail;
        using namespace policies;

        using other_traits = virtual_ptr_traits<
            std::remove_cv_t<std::remove_reference_t<Other>>, Policy>;
        using other_type = typename other_traits::element_type;

        if constexpr (Policy::template has_facet<runtime_checks>) {
            // check that dynamic type == static type
            auto static_type = Policy::template static_type<other_type>();
            auto dynamic_type = other_traits::dynamic_type(obj);

            if (dynamic_type != static_type) {
                method_table_error error;
                error.type = dynamic_type;
                Policy::error(error);
                abort();
            }
        }

        typename impl::vptr_type static_vptr;

        if constexpr (impl::is_indirect) {
            static_vptr = &Policy::template static_vptr<other_type>;
        } else {
            static_vptr = Policy::template static_vptr<other_type>;
        }

        return virtual_ptr(std::forward<Other>(obj), static_vptr);
    }

    auto vptr() const {
        if constexpr (impl::is_indirect) {
            return *this->vp;
        } else {
            return this->vp;
        }
    }
};

template<class Class>
virtual_ptr(Class&) -> virtual_ptr<Class, BOOST_OPENMETHOD_DEFAULT_POLICY>;

template<class Policy, class Class>
inline auto final_virtual_ptr(Class&& obj) {
    return virtual_ptr<std::remove_reference_t<Class>, Policy>::final(
        std::forward<Class>(obj));
}

template<class Class>
inline auto final_virtual_ptr(Class&& obj) {
    return virtual_ptr<std::remove_reference_t<Class>>::final(
        std::forward<Class>(obj));
}

template<class Left, class Right, class Policy>
bool operator==(
    const virtual_ptr<Left, Policy>& left,
    const virtual_ptr<Right, Policy>& right) {
    return &*left == &*right;
}

template<class Left, class Right, class Policy>
bool operator!=(
    const virtual_ptr<Left, Policy>& left,
    const virtual_ptr<Right, Policy>& right) {
    return !(left == right);
}

template<class Class, class Policy>
struct virtual_traits<virtual_ptr<Class, Policy>, Policy> {
    using ptr_traits = virtual_ptr_traits<Class, Policy>;
    using virtual_type = typename ptr_traits::element_type;

    static auto rarg(const virtual_ptr<Class, Policy>& ptr)
        -> const virtual_ptr<Class, Policy>& {
        return ptr;
    }

    template<typename Derived>
    static decltype(auto) cast(const virtual_ptr<Class, Policy>& ptr) {
        return ptr.template cast<
            typename std::remove_reference_t<Derived>::element_type>();
    }

    template<typename Derived>
    static decltype(auto) cast(virtual_ptr<Class, Policy>&& ptr) {
        return std::move(ptr).template cast<
            typename std::remove_reference_t<Derived>::element_type>();
    }
};

template<class Class, class Policy>
struct virtual_traits<const virtual_ptr<Class, Policy>&, Policy>
    : virtual_traits<virtual_ptr<Class, Policy>, Policy> {};

// =============================================================================
// Method

namespace detail {

template<typename P, typename Q, class Policy>
struct select_overrider_virtual_type_aux {
    using type = void;
};

template<typename P, typename Q, class Policy>
struct select_overrider_virtual_type_aux<virtual_<P>, Q, Policy> {
    using type = virtual_type<Q, Policy>;
};

template<typename P, typename Q, class Policy>
struct select_overrider_virtual_type_aux<
    virtual_ptr<P, Policy>, virtual_ptr<Q, Policy>, Policy> {
    using type =
        typename virtual_traits<virtual_ptr<Q, Policy>, Policy>::virtual_type;
};

template<typename P, typename Q, class Policy>
struct select_overrider_virtual_type_aux<
    const virtual_ptr<P, Policy>&, const virtual_ptr<Q, Policy>&, Policy> {
    using type = typename virtual_traits<
        const virtual_ptr<Q, Policy>&, Policy>::virtual_type;
};

template<typename P, typename Q, class Policy>
using select_overrider_virtual_type =
    typename select_overrider_virtual_type_aux<P, Q, Policy>::type;

template<typename MethodParameters, typename OverriderParameters, class Policy>
using overrider_virtual_types = boost::mp11::mp_remove<
    boost::mp11::mp_transform_q<
        boost::mp11::mp_bind_back<select_overrider_virtual_type, Policy>,
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

template<class MethodPolicy, class Parameter>
struct is_policy_compatible : std::true_type {};

template<class Policy, typename Type, class OtherPolicy>
struct is_policy_compatible<Policy, virtual_ptr<Type, OtherPolicy>>
    : std::is_same<Policy, OtherPolicy> {};

template<class Policy, typename Type, class OtherPolicy>
struct is_policy_compatible<Policy, const virtual_ptr<Type, OtherPolicy>&>
    : std::is_same<Policy, OtherPolicy> {};

template<class Class, typename = void>
struct has_vptr : std::false_type {};

template<class Class>
struct has_vptr<Class, std::void_t<decltype(Class::boost_openmethod_vptr)>>
    : std::true_type {};

} // namespace detail

template<
    typename Method, typename ReturnType,
    class Policy = BOOST_OPENMETHOD_DEFAULT_POLICY>
class method;

template<
    typename Name, typename... Parameters, typename ReturnType, class Policy>
class method<Name(Parameters...), ReturnType, Policy>
    : public detail::method_info {
    // Aliases used in implementation only. Everything extracted from template
    // arguments is capitalized like the arguments themselves.
    using DeclaredParameters = detail::types<Parameters...>;
    using CallParameters =
        boost::mp11::mp_transform<detail::remove_virtual, DeclaredParameters>;
    using VirtualParameters =
        typename detail::virtual_types<DeclaredParameters>;
    using Signature = auto(Parameters...) -> ReturnType;
    using FunctionPointer = auto(*)(detail::remove_virtual<Parameters>...)
                                -> ReturnType;
    static constexpr auto Arity = boost::mp11::mp_count_if<
        detail::types<Parameters...>, detail::is_virtual>::value;

    // sanity checks
    static_assert(Arity > 0, "method must have at least one virtual argument");
    static_assert(
        (true && ... &&
         detail::is_policy_compatible<Policy, Parameters>::value));

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
                DeclaredParameters, detail::types<OverriderParameters...>,
                Policy>,
            Policy>;
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

template<
    typename Name, typename... Parameters, typename ReturnType, class Policy>
method<Name(Parameters...), ReturnType, Policy>
    method<Name(Parameters...), ReturnType, Policy>::fn;

template<
    typename Name, typename... Parameters, typename ReturnType, class Policy>
template<auto>
typename method<Name(Parameters...), ReturnType, Policy>::FunctionPointer
    method<Name(Parameters...), ReturnType, Policy>::next;

template<typename T>
constexpr bool is_method = std::is_base_of_v<detail::method_info, T>;

template<
    typename Name, typename... Parameters, typename ReturnType, class Policy>
method<Name(Parameters...), ReturnType, Policy>::method() {
    method_info::slots_strides_ptr = slots_strides;

    using virtual_type_ids = detail::type_id_list<
        boost::mp11::mp_transform_q<
            boost::mp11::mp_bind_back<detail::virtual_type, Policy>,
            VirtualParameters>,
        Policy>;
    method_info::vp_begin = virtual_type_ids::begin;
    method_info::vp_end = virtual_type_ids::end;
    method_info::not_implemented = (void*)not_implemented_handler;
    method_info::method_type = Policy::template static_type<method>();
    method_info::return_type = Policy::template static_type<
        typename virtual_traits<ReturnType, Policy>::virtual_type>();
    Policy::methods.push_back(*this);
}

template<
    typename Name, typename... Parameters, typename ReturnType, class Policy>
std::size_t method<
    Name(Parameters...), ReturnType, Policy>::slots_strides[2 * Arity - 1];

template<
    typename Name, typename... Parameters, typename ReturnType, class Policy>
method<Name(Parameters...), ReturnType, Policy>::~method() {
    Policy::methods.remove(*this);
}

template<
    typename Name, typename... Parameters, typename ReturnType, class Policy>
template<class Error>
auto method<Name(Parameters...), ReturnType, Policy>::check_static_offset(
    std::size_t actual, std::size_t expected) const -> void {
    using namespace detail;

    if (actual != expected) {
        if (Policy::template has_facet<policies::error_handler>) {
            Error error;
            error.method = Policy::template static_type<method>();
            error.expected = this->slots_strides[0];
            error.actual = actual;
            Policy::error(error);

            abort();
        }
    }
}

// -----------------------------------------------------------------------------
// method dispatch

template<
    typename Name, typename... Parameters, typename ReturnType, class Policy>
BOOST_FORCEINLINE auto
method<Name(Parameters...), ReturnType, Policy>::operator()(
    detail::remove_virtual<Parameters>... args) const -> ReturnType {
    using namespace detail;
    auto pf = resolve(parameter_traits<Parameters, Policy>::rarg(args)...);

    return pf(std::forward<remove_virtual<Parameters>>(args)...);
}

template<
    typename Name, typename... Parameters, typename ReturnType, class Policy>
template<typename... ArgType>
BOOST_FORCEINLINE
    typename method<Name(Parameters...), ReturnType, Policy>::FunctionPointer
    method<Name(Parameters...), ReturnType, Policy>::resolve(
        const ArgType&... args) const {
    using namespace detail;

    std::uintptr_t pf;

    if constexpr (Arity == 1) {
        pf = resolve_uni<types<Parameters...>, ArgType...>(args...);
    } else {
        pf = resolve_multi_first<types<Parameters...>, ArgType...>(args...);
    }

    return reinterpret_cast<FunctionPointer>(pf);
}

template<
    typename Name, typename... Parameters, typename ReturnType, class Policy>
template<typename ArgType>
BOOST_FORCEINLINE auto
method<Name(Parameters...), ReturnType, Policy>::vptr(const ArgType& arg) const
    -> vptr_type {
    if constexpr (detail::is_virtual_ptr<ArgType>) {
        return arg.vptr();
        // No need to check the method pointer: this was done when the
        // virtual_ptr was created.
    } else if constexpr (detail::has_vptr<ArgType>::value) {
        if constexpr (std::is_same_v<
                          decltype(arg.boost_openmethod_vptr),
                          indirect_vptr_type>) {
            return *arg.boost_openmethod_vptr;
        } else {
            return arg.boost_openmethod_vptr;
        }
    } else {
        return Policy::dynamic_vptr(arg);
    }
}

template<
    typename Name, typename... Parameters, typename ReturnType, class Policy>
template<typename MethodArgList, typename ArgType, typename... MoreArgTypes>
BOOST_FORCEINLINE auto
method<Name(Parameters...), ReturnType, Policy>::resolve_uni(
    const ArgType& arg, const MoreArgTypes&... more_args) const
    -> std::uintptr_t {

    using namespace detail;
    using namespace boost::mp11;

    if constexpr (is_virtual<mp_first<MethodArgList>>::value) {
        vptr_type vtbl;

        if constexpr (is_virtual_ptr<ArgType>) {
            vtbl = arg.vptr();
        } else {
            vtbl = vptr<ArgType>(arg);
        }

        if constexpr (has_static_offsets<method>::value) {
            if constexpr (Policy::template has_facet<
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
    typename Name, typename... Parameters, typename ReturnType, class Policy>
template<typename MethodArgList, typename ArgType, typename... MoreArgTypes>
BOOST_FORCEINLINE auto
method<Name(Parameters...), ReturnType, Policy>::resolve_multi_first(
    const ArgType& arg, const MoreArgTypes&... more_args) const
    -> std::uintptr_t {

    using namespace detail;
    using namespace boost::mp11;

    if constexpr (is_virtual<mp_first<MethodArgList>>::value) {
        vptr_type vtbl;

        if constexpr (is_virtual_ptr<ArgType>) {
            vtbl = arg.vptr();
        } else {
            vtbl = vptr<ArgType>(arg);
        }

        std::size_t slot;

        if constexpr (has_static_offsets<method>::value) {
            slot = static_offsets<method>::slots[0];
            if constexpr (Policy::template has_facet<
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
    typename Name, typename... Parameters, typename ReturnType, class Policy>
template<
    std::size_t VirtualArg, typename MethodArgList, typename ArgType,
    typename... MoreArgTypes>
BOOST_FORCEINLINE auto
method<Name(Parameters...), ReturnType, Policy>::resolve_multi_next(
    vptr_type dispatch, const ArgType& arg,
    const MoreArgTypes&... more_args) const -> std::uintptr_t {

    using namespace detail;
    using namespace boost::mp11;

    if constexpr (is_virtual<mp_first<MethodArgList>>::value) {
        vptr_type vtbl;

        if constexpr (is_virtual_ptr<ArgType>) {
            vtbl = arg.vptr();
        } else {
            vtbl = vptr<ArgType>(arg);
        }

        std::size_t slot, stride;

        if constexpr (has_static_offsets<method>::value) {
            slot = static_offsets<method>::slots[VirtualArg];
            stride = static_offsets<method>::strides[VirtualArg - 1];
            if constexpr (Policy::template has_facet<
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

template<
    typename Name, typename... Parameters, typename ReturnType, class Policy>
BOOST_NORETURN auto
method<Name(Parameters...), ReturnType, Policy>::not_implemented_handler(
    detail::remove_virtual<Parameters>... args) -> ReturnType {
    if constexpr (Policy::template has_facet<policies::error_handler>) {
        not_implemented_error error;
        error.method = Policy::template static_type<method>();
        error.arity = Arity;
        type_id types[sizeof...(args)];
        auto ti_iter = types;
        (...,
         (*ti_iter++ = Policy::dynamic_type(
              detail::parameter_traits<Parameters, Policy>::rarg(args))));
        std::copy_n(
            types,
            (std::min)(sizeof...(args), not_implemented_error::max_types),
            &error.types[0]);
        Policy::error(error);
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
    typename Name, typename... Parameters, typename ReturnType, class Policy>
template<
    auto Overrider, typename OverriderReturn, typename... OverriderParameters>
auto method<Name(Parameters...), ReturnType, Policy>::
    thunk<Overrider, OverriderReturn (*)(OverriderParameters...)>::fn(
        detail::remove_virtual<Parameters>... arg) -> ReturnType {
    using namespace detail;
    static_assert(
        (true && ... &&
         is_virtual_ptr_compatible<Parameters, OverriderParameters>),
        "virtual_ptr mismatch");
    return Overrider(
        detail::parameter_traits<Parameters, Policy>::template cast<
            OverriderParameters>(
            std::forward<detail::remove_virtual<Parameters>>(arg))...);
}

// -----------------------------------------------------------------------------
// overriders

template<
    typename Name, typename... Parameters, typename ReturnType, class Policy>
template<auto Function, typename FnReturnType>
method<Name(Parameters...), ReturnType, Policy>::override_impl<
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
    info.return_type = Policy::template static_type<
        typename virtual_traits<FnReturnType, Policy>::virtual_type>();
    info.type = Policy::template static_type<decltype(Function)>();
    info.next = reinterpret_cast<void**>(p_next);
    using Thunk = thunk<Function, decltype(Function)>;
    info.pf = (void*)Thunk::fn;
    info.vp_begin = Thunk::OverriderParameterTypeIds::begin;
    info.vp_end = Thunk::OverriderParameterTypeIds::end;
    fn.specs.push_back(info);
}

} // namespace boost::openmethod

#endif
