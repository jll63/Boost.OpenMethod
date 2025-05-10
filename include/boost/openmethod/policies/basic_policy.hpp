// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_POLICIES_CORE_HPP
#define BOOST_OPENMETHOD_POLICIES_CORE_HPP

#include <boost/openmethod/detail/types.hpp>

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/bind.hpp>

#include <functional>
#include <variant>

namespace boost::openmethod {

namespace policies {

struct facet {
    static auto finalize() -> void {
    }
};

} // namespace policies

namespace detail {

using class_catalog = detail::static_list<detail::class_info>;
using method_catalog = detail::static_list<detail::method_info>;

template<class Facet>
struct basic_facet : policies::facet {
    using facet_type = Facet;
};

template<typename Policy, class Facet>
struct fork_facet {
    using type = Facet;
};

template<
    typename NewPolicy, typename OldPolicy,
    template<typename...> class GenericFacet, typename... Args>
struct fork_facet<NewPolicy, GenericFacet<OldPolicy, Args...>> {
    using type = GenericFacet<NewPolicy, Args...>;
};

template<class Facets, class...>
struct with_aux;

template<class Facets>
struct with_aux<Facets> {
    using type = Facets;
};

template<class Facets, class Facet, class... MoreFacets>
struct with_aux<Facets, Facet, MoreFacets...> {
    using replace = mp11::mp_replace_if_q<
        Facets,
        mp11::mp_bind_front_q<
            mp11::mp_quote_trait<std::is_base_of>, typename Facet::facet_type>,
        Facet>;
    using replace_or_add = std::conditional_t<
        std::is_same_v<replace, Facets>, mp11::mp_push_back<Facets, Facet>,
        replace>;
    using type = typename with_aux<replace_or_add, MoreFacets...>::type;
};

template<class Facets, class...>
struct without_aux;

template<class Facets>
struct without_aux<Facets> {
    using type = Facets;
};

template<class Facets, class Facet, class... MoreFacets>
struct without_aux<Facets, Facet, MoreFacets...> {
    using type = typename without_aux<
        mp11::mp_remove_if_q<
            Facets,
            mp11::mp_bind_front_q<
                mp11::mp_quote_trait<std::is_base_of>,
                typename Facet::facet_type>>,
        MoreFacets...>::type;
};

} // namespace detail

namespace policies {

struct abstract_policy {};

// -----------------------------------------------------------------------------
// Facets

struct rtti : detail::basic_facet<rtti> {
    static auto type_index(type_id type) -> type_id {
        return type;
    }

    template<typename Stream>
    static void type_name(type_id type, Stream& stream) {
        stream << "type_id(" << type << ")";
    }
};

struct deferred_static_rtti : rtti {};
struct error_handler : detail::basic_facet<error_handler> {};
struct type_hash : detail::basic_facet<type_hash> {};
struct extern_vptr : detail::basic_facet<extern_vptr> {};
struct indirect_vptr : detail::basic_facet<indirect_vptr> {};
struct error_output : detail::basic_facet<error_output> {};
struct trace_output : detail::basic_facet<trace_output> {};
struct runtime_checks : detail::basic_facet<runtime_checks> {};

// -----------------------------------------------------------------------------
// domain

template<class Policy>
struct domain {
    inline static detail::class_catalog classes;
    inline static detail::method_catalog methods;
    template<class Class>
    inline static vptr_type static_vptr;
    inline static std::vector<std::uintptr_t> dispatch_data;
};

template<class Policy, class... Facets>
struct basic_policy : abstract_policy, domain<Policy>, Facets... {
    using facets = mp11::mp_list<Facets...>;

    template<class Facet>
    static constexpr bool has_facet = std::is_base_of_v<Facet, basic_policy>;

    template<class NewPolicy>
    using fork = basic_policy<
        NewPolicy, typename detail::fork_facet<NewPolicy, Facets>::type...>;

    template<class... NewFacets>
    using with = boost::mp11::mp_apply<
        basic_policy,
        boost::mp11::mp_push_front<
            typename detail::with_aux<facets, NewFacets...>::type, Policy>>;

    template<class... RemoveFacets>
    using without = boost::mp11::mp_apply<
        basic_policy,
        boost::mp11::mp_push_front<
            typename detail::without_aux<facets, RemoveFacets...>::type,
            Policy>>;
};

} // namespace policies

} // namespace boost::openmethod

#endif
