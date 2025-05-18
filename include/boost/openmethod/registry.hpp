#ifndef BOOST_OPENMETHOD_REGISTRY_HPP
#define BOOST_OPENMETHOD_REGISTRY_HPP

#include <boost/openmethod/detail/types.hpp>

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/bind.hpp>

#include <vector>

namespace boost::openmethod {

namespace policies {

struct policy {};

struct rtti : policy {
    using category = rtti;

    template<class Register>
    struct fn {
        static auto type_index(type_id type) -> type_id {
            return type;
        }

        template<typename Stream>
        static void type_name(type_id type, Stream& stream) {
            stream << "type_id(" << type << ")";
        }
    };
};

struct deferred_static_rtti : rtti {};

struct error_handler : policy {
    using category = error_handler;
};

struct type_hash : policy {
    using category = type_hash;
};

struct vptr : policy {
    using category = vptr;
};

struct extern_vptr : vptr {};

struct indirect_vptr : policy {
    using category = indirect_vptr;
    template<class Registry>
    struct fn {};
};

struct output : policy {
    using category = output;
};

struct trace : policy {
    using category = trace;
    template<class Registry>
    struct fn {
        inline static bool trace_enabled = []() {
            auto env = getenv("BOOST_OPENMETHOD_TRACE");
            return env && *env++ == '1' && *env++ == 0;
        }();
    };
};

struct runtime_checks : policy {
    using category = runtime_checks;
    template<class Registry>
    struct fn {};
};

template<typename Key>
struct unique : policy {
    using category = unique;
    template<class Registry>
    struct fn {};
};

} // namespace policies

namespace detail {

struct registry_base {};

template<typename T>
constexpr bool is_registry = std::is_base_of_v<registry_base, T>;

template<typename T>
constexpr bool is_not_void = !std::is_same_v<T, void>;

template<
    class Registry, class Index,
    class Size = mp11::mp_size<typename Registry::policy_list>>
struct get_policy_aux {
    using type = typename mp11::mp_at<
        typename Registry::policy_list, Index>::template fn<Registry>;
};

template<class Registry, class Size>
struct get_policy_aux<Registry, Size, Size> {
    using type = void;
};

using class_catalog = detail::static_list<detail::class_info>;
using method_catalog = detail::static_list<detail::method_info>;

template<class Policies, class...>
struct with_aux;

template<class Policies>
struct with_aux<Policies> {
    using type = Policies;
};

template<class Policies, class Policy, class... MorePolicies>
struct with_aux<Policies, Policy, MorePolicies...> {
    using replace = mp11::mp_replace_if_q<
        Policies,
        mp11::mp_bind_front_q<
            mp11::mp_quote_trait<std::is_base_of>, typename Policy::category>,
        Policy>;
    using replace_or_add = std::conditional_t<
        std::is_same_v<replace, Policies>, mp11::mp_push_back<Policies, Policy>,
        replace>;
    using type = typename with_aux<replace_or_add, MorePolicies...>::type;
};

template<class Policies, class...>
struct without_aux;

template<class Policies>
struct without_aux<Policies> {
    using type = Policies;
};

template<class Policies, class Policy, class... MorePolicies>
struct without_aux<Policies, Policy, MorePolicies...> {
    using type = typename without_aux<
        mp11::mp_remove_if_q<
            Policies,
            mp11::mp_bind_front_q<
                mp11::mp_quote_trait<std::is_base_of>,
                typename Policy::category>>,
        MorePolicies...>::type;
};

} // namespace detail

template<class... Policies>
struct registry : detail::registry_base {
    inline static detail::class_catalog classes;
    inline static detail::method_catalog methods;
    template<class Class>
    inline static vptr_type static_vptr;
    inline static std::vector<std::uintptr_t> dispatch_data;

    using policy_list = mp11::mp_list<Policies...>;

    template<class PolicyCategory>
    using policy = typename detail::get_policy_aux<
        registry,
        mp11::mp_find_if_q<
            policy_list,
            mp11::mp_bind_front_q<
                mp11::mp_quote_trait<std::is_base_of>, PolicyCategory>>>::type;

    template<class PolicyCategory>
    static constexpr bool has_policy =
        detail::is_not_void<policy<PolicyCategory>>;

    template<class... NewPolicies>
    using with = boost::mp11::mp_apply<
        registry, typename detail::with_aux<policy_list, NewPolicies...>::type>;

    template<class... RemovePolicies>
    using without = boost::mp11::mp_apply<
        registry,
        typename detail::without_aux<policy_list, RemovePolicies...>::type>;

    using registry_type = registry;
    using rtti = policy<policies::rtti>;
    using error_handler = policy<policies::error_handler>;
    static constexpr auto runtime_checks = has_policy<policies::runtime_checks>;
    static constexpr auto trace = has_policy<policies::trace>;
};

} // namespace boost::openmethod

#endif // BOOST_OPENMETHOD_REGISTRY_HPP
