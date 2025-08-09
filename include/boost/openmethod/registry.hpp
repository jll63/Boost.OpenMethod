#ifndef BOOST_OPENMETHOD_REGISTRY_HPP
#define BOOST_OPENMETHOD_REGISTRY_HPP

#include <boost/openmethod/detail/types.hpp>

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/bind.hpp>

#include <stdlib.h>
#include <vector>

namespace boost::openmethod {

//! Namespace containing the policy framework.
//!
//! A @e policy is a class that controls how some fundamental operations are
//! performed by the library: type information acquisition (`rtti`), v-table
//! pointer acquisition (`vptr`), error handling (`error_handler`), etc.
//!
//! Some policies can be used directly in a registry's policy list. They
//! function like flags, enabling blocks of code in the dispatch mechanism. For
//! example, `runtime_checks` enables code that detects missing class
//! registrations.
//!
//! Others policy classes need to be derived from. The subclass is required to
//! contain a `template<class Registry> struct fn`, which contains a set static
//! members that follow the requirements of the policy. For example, subclasses
//! of `vptr` must provide a `fn` that contains a `static vptr_type
//! dynamic_vptr(const Class& arg)` that returns a v-table pointer for an object
//! of type `Class`.

namespace policies {

//! RTTI policy base class.
//!
//! A @e rtti policy is responsible for acquiring and manipulating type
//! information, dynamic casting, and identifying polymorphic classes.
//!
//! Class `rtti` provides a default implementation for some of these operations.
//!
//! @par Requirements
//!
//! A @e rtti policy must derive from class `rtti`, and contain a `fn<Registry>`
//! class template with the following public static members:
//!
//! - `template<class Class> constexpr bool is_polymorphic`: evaluates to `true`
//!   if `Class` is polymorphic.
//!
//! - `template<class Class> type_id static_type()`: returns a `type_id`
//!   for `Class`.
//!
//! - `template<class Class> type_id dynamic_type(const Class& obj)`:
//!   returns the type_id of an object's dynamic type.
//!
//! - `template<typename Stream> void type_name(type_id type, Stream&
//!   stream)`: writes a description of `type` to `stream`.
//!
//! - `template<typename Stream> void type_name(type_id type, Stream&
//!   stream)`: writes a description of `type` to `stream`.
//!
//! - `/* unspecified */ type_index(type_id type)`: returns a @e unique
//!   identifier for `type`.

struct rtti {
    using category = rtti;

    template<class Register>
    struct fn {
        //! Default implementation of `type_index`.
        //!
        //! Returns `type`.
        //!
        //! @param type A `type_id`.
        static auto type_index(type_id type) -> type_id {
            return type;
        }

        //! Default implementation of `type_name`.
        //!
        //! Executes `stream << "type_id(" << type << ")"`.
        //!
        //! @param type A `type_id`.
        //! @param stream A stream to write to.
        template<typename Stream>
        static void type_name(type_id type, Stream& stream) {
            stream << "type_id(" << type << ")";
        }
    };
};

//! RTTI policy base class with defered type id collection.
//!
//! Some custom RTTI systems rely on static constructors to assign type ids.
//! OpenMethod itself relies on static constructors to register classes, methods
//! and overriders. This creates order-of-initialization issues. Deriving a @e
//! rtti policy from this class - instead of just `rtti` - causes the collection
//! of type ids to be deferred until the first call to @ref update.
struct deferred_static_rtti : rtti {};

//! Error handling policy base class.
//!
//! A @e error_handler policy runs code before terminating the program due to an
//! error. This can be useful for throwing, logging, cleanup, or other actions.
//!
//! @par Requirements
//!
//! A subclass of `error_handler` contains a `fn<Registry>` struct that provides
//! one or more `error` static functions that take an error object. `error` must
//! be callable with an instance of any subclass of `openmethod_error`.

struct error_handler {
    using category = error_handler;
};

//! Hashing policy base class.
//!
//! A @e type_hash policy calculates an integer hash for a `type_id`.
//!
//! @par Requirements
//!
//! A subclass of `error_handler` contains a `fn<Registry>` struct that provides
//! the following static member functions:
//! - auto initialize(ForwardIterator first, ForwardIterator last) -> [min,
//!   max]: initialize the hash table with the values in the [iter, last) range.
//!   `*iter` is an object of an unspecified type that has two members,
//!   `type_id_begin()` and `type_id_end()`, which return iterators to a range
//!   of `type_id`s. `initialize` returns a pair with the minimum and maximum
//!   hash values.
//! - auto hash(type_id type) -> std::size_t: returns a hash value for `type`.
//!   The value must be in the range returned by `initialize`.
//! - void finalize(): releases any resources allocated by `initialize`. This
//!   member is optional.

struct type_hash {
    using category = type_hash;
};

/**
    Acquires a v-table pointer for an object.

    @par Requirements

    Any implementation of this policy must provide a meta-function `fn` that
    in the form of:

    @code
    template<class Registry>
    struct fn {
        template<class Class> static auto dynamic_vptr(const Class& arg) -> const vptr_type&;
    };
    @endcode

    @see policies::vptr_vector::fn::dynamic_vptr for an example.

*/

struct vptr {
    using category = vptr;
};

struct extern_vptr : vptr {};

struct indirect_vptr {
    using category = indirect_vptr;
    template<class Registry>
    struct fn {};
};

struct output {
    using category = output;
};

struct trace {
    using category = trace;
    template<class Registry>
    struct fn {
        inline static bool trace_enabled = []() {
#ifdef _MSC_VER
            char* env;
            std::size_t len;
            auto result =
                _dupenv_s(&env, &len, "BOOST_OPENMETHOD_TRACE") == 0 && env &&
                len == 2 && *env == '1';
            free(env);
            return result;
#else
            auto env = getenv("BOOST_OPENMETHOD_TRACE");
            return env && *env++ == '1' && *env++ == 0;
#endif
        }();
    };
};

struct runtime_checks {
    using category = runtime_checks;
    template<class Registry>
    struct fn {};
};

struct n2216 {
    using category = n2216;
    template<class Registry>
    struct fn {};
};

template<typename Key>
struct unique {
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
    using registry_type = registry;

    inline static bool initialized;
    inline static detail::class_catalog classes;
    inline static detail::method_catalog methods;
    template<class Class>
    inline static vptr_type static_vptr;
    inline static std::vector<detail::word> dispatch_data;

    using policy_list = mp11::mp_list<Policies...>;

    template<class PolicyCategory>
    using policy = typename detail::get_policy_aux<
        registry_type,
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

    static constexpr auto has_error_handler =
        has_policy<policies::error_handler>;
    static constexpr auto has_output = has_policy<policies::output>;
    static constexpr auto has_trace = has_policy<policies::trace>;
    static constexpr auto deferred_static_rtti =
        has_policy<policies::deferred_static_rtti>;
    static constexpr auto runtime_checks = has_policy<policies::runtime_checks>;
    static constexpr auto indirect_vptr = has_policy<policies::indirect_vptr>;

    using rtti = policy<policies::rtti>;
    using error_handler = policy<policies::error_handler>;
    using output = policy<policies::output>;
    using trace = policy<policies::trace>;

    static void check_initialized();
};

template<class... Policies>
inline void registry<Policies...>::check_initialized() {
    if constexpr (has_policy<policies::runtime_checks>) {
        if (!initialized) {
            using error_handler = policy<policies::error_handler>;

            if constexpr (detail::is_not_void<error_handler>) {
                error_handler::error(not_initialized_error());
            }

            abort();
        }
    }
}

template<class Registry, class Stream>
auto call_error::write_aux(Stream& os, const char* subtype) const -> void {
    using namespace detail;
    using namespace policies;

    os << "invalid call to ";
    Registry::template policy<rtti>::type_name(method, os);
    os << "(";
    auto comma = "";

    for (auto ti : range{types, types + arity}) {
        os << comma;
        Registry::template policy<rtti>::type_name(ti, os);
        comma = ", ";
    }

    os << "): " << subtype;
}

template<class Registry, class Stream>
auto unknown_class_error::write(Stream& os) const -> void {
    os << "unknown class ";
    Registry::rtti::type_name(type, os);
}

template<class Registry, class Stream>
auto final_error::write(Stream& os) const -> void {
    os << "invalid call to final construct: static type = ";
    Registry::rtti::type_name(static_type, os);
    os << ", dynamic type = ";
    Registry::rtti::type_name(dynamic_type, os);
}

template<class Registry, class Stream>
auto hash_search_error::write(Stream& os) const -> void {
    os << "could not find hash factors after " << attempts << "s using "
       << buckets << " buckets\n";
}

template<class Registry, class Stream>
auto static_offset_error::write(Stream& os) const -> void {
    os << "static offset error in ";
    Registry::rtti::type_name(method, os);
    os << ": expected " << expected << ", got " << actual;
}

} // namespace boost::openmethod

#endif // BOOST_OPENMETHOD_REGISTRY_HPP
