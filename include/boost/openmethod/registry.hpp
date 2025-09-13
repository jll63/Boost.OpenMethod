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
//! A registry contains a set of policies that control how certain operations
//! are performed. For example, the "rtti" policy provides type information,
//! implements dynamic casting, etc. It can be replaced to interface with custom
//! RTII systems (like LLVM's).
//!
//! A policy must derive from one of the policy base classes, also known as
//! _policy categories_.
//!
//! - @ref rtti: type information acquisition and manipulation.
//!
//! - @ref error_handler: perform operations when errors are detected.
//!
//! - @ref vptr: v-table pointer acquisition.
//!
//! - @ref type_hash: hashing of `type_id`s.
//!
//! - @ref output: output stream for logging and debugging.
//!
//! - @ref runtime_checks: detect and report common errors.
//!
//! - @ref trace: report how dispatch tables are built.
//!
//! - @ref n2216: handle ambiguities according to the N2216 proposal.
//!
//! Policies are implemented as Boost.MP11 quoted meta-functions. A policy class
//! must contain a `template<class Registry> struct fn` that provides a set of
//! _static_ members, fulfilling the requirements specified in the policy's
//! category. Registries instantiate policies by passing themselves to the
//! nested `fn` class templates.
//!
//! There are two reason for this design.
//!
//! Some policies are "stateful": they contain static _data_ members. Since
//! several registries can co-exist in the same program, each stateful policy
//! needs its own, separate set of static data members. For example, @ref
//! vptr_vector, a "vptr" policy, contains a static vector of vptrs, which
//! cannot be shared with other registries.
//!
//! Some policies need access to other policies in the same registry. They can
//! be accessed via the `Registry` template parameter. For example, @ref
//! vptr_vector hashes type_ids before using them as an indexes, if `Registry`
//! cotains a `type_hash` policy. It performs out-of-bounds checks if `Registry`
//! contains the `runtime_checks` policy. If an error is detected, it invokes
//! the @ref error_handler policy if there is  one.
//!
//! The last three policies (runtime_checks, trace and n2216) act like flags,
//! and enabling some sections of code. They can be used as-is, without the need
//! for subclassing.

#ifdef __MRDOCS__

//! Requirements for LightweightOutputStream (exposition only)
struct LightweightOutputStream {
    LightweightOutputStream& operator<<(const char* str);
    LightweightOutputStream& operator<<(const std::string_view& view);
    LightweightOutputStream& operator<<(const void* value);
    LightweightOutputStream& operator<<(std::size_t value);
};

#endif

namespace policies {

#ifdef __MRDOCS__

//! Requirements for policy categories (exposition only)
//!
struct PolicyCategory {
    using category = PolicyCategory;
};

//! Requirements for policies (exposition only)
//!
struct Policy : PolicyCategory {
    template<class Registry>
    struct fn;
};

//! Requirements for IdsToVptr (exposition only)
//!
struct IdsToVptr {
    //! Returns an iterator to the beginning of a range of `type_id`s for a
    //! single registered class.
    auto type_id_begin() const;

    //! Returns an iterator to the end of a range of `type_id`s for a
    //! single registered class.
    auto type_id_end() const;

    //! Returns a range of `type_id`s that this assignment applies to.
    auto vptr() const -> const vptr_type&;
};

#endif

//! Policy for runtime type information.
//!
//! A @e rtti policy is responsible for acquiring and manipulating type
//! information, dynamic casting, and detecting polymorphic classes.
//!
//! @par Requirements
//!
//! A subclass of `rtti` must contain a `fn<Registry>` class template
//! that fulfills the requirements of @ref rtti::fn.
struct rtti {
    using category = rtti;

    //! Default implementations of some `rtti` requirements.
    struct defaults {
        //! Default implementation for `type_index`.
        //!
        //! @param type A `type_id`.
        //!
        //! @return `type` itself.
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

#ifdef __MRDOCS__
    //! Requirements for `rtti` policies (exposition only).
    //!
    //! This class is for _exposition only_. It is the responsibility of
    //! subclasses to provide a `fn` class template that contains the members
    //! listed on this page.
    template<class Registry>
    struct fn {
        //! Tests if a class is polymorphic.
        //!
        //! @tparam Class A class.
        template<class Class>
        static constexpr bool is_polymorphic = std::is_polymorphic_v<Class>;

        //! Returns the static @ref type_id of a type.
        //!
        //! @note `Class` is not necessarily a @e registered class. This
        //! function is also called to acquire the type_id of non-virtual
        //! parameters, library types, etc, for diagnostic and trace purposes.
        //!
        //! @tparam Class A class.
        //! @return The static type_id of Class.
        template<class Class>
        static auto static_type() -> type_id;

        //! Returns the dynamic @ref type_id of an object.
        //!
        //! @tparam Class A registered class.
        //! @param obj A reference to an instance of `Class`.
        //! @return The type_id of `obj`'s class.
        template<class Class>
        static auto dynamic_type(const Class& obj) -> type_id;

        //! Writes a representation of a @ref type_id to a stream.
        //!
        //! @tparam Stream A LightweightOutputStream.
        //! @param type The `type_id` to write.
        //! @param stream The stream to write to.
        template<typename Stream>
        static auto type_name(type_id type, Stream& stream) -> void;

        //! Returns a key that uniquely identifies a class.
        //!
        //! @param type A `type_id`.
        //! @return A unique value that identifies a class with the given
        //! `type_id`.
        static auto type_index(type_id type);

        //! Casts an object to a type.
        //!
        //! @tparam D A reference to a subclass of `B`.
        //! @tparam B A registered class.
        //! @param obj A reference to an instance of `B`.
        template<typename D, typename B>
        static auto dynamic_cast_ref(B&& obj) -> D;
    };
#endif
};

#ifdef __MRDOCS__
struct std_rtti;
struct static_rtti;
#endif

//! Policy for deferred type id collection.
//!
//! Some custom RTTI systems rely on static constructors to assign type ids.
//! OpenMethod itself relies on static constructors to register classes, methods
//! and overriders. This creates order-of-initialization issues. Deriving a @e
//! rtti policy from this class - instead of just `rtti` - causes the collection
//! of type ids to be deferred until the first call to @ref update.
struct deferred_static_rtti : rtti {};

//! Policy for error handling.
//!
//! A @e error_handler policy runs code before the library terminats the program
//! due to an error. This can be useful for throwing, logging, cleanup, or other
//! actions.
//!
//! @par Requirements
//!
//! A subclass of `error_handler` must contain a `fn<Registry>` class template
//! that fulfills the requirements of @ref error_handler::fn.

struct error_handler {
    using category = error_handler;

#ifdef __MRDOCS__
    //! Requirements for `error_handler` policies (exposition only).
    //!
    //! This class is for _exposition only_. It is the responsibility of
    //! subclasses to provide a `fn` class template that contains the members
    //! listed on this page.
    template<class Registry>
    struct fn {
        //! Called when an error is detected.
        //!
        //! `error` is a function, or a set of functions, that can be called
        //! with an instance of any subclass of `openmethod_error`.
        static auto error(const auto& error) -> void;
    };
#endif
};

#ifdef __MRDOCS__
struct default_error_handler;
struct throw_error_handler;
#endif

//! Policy for v-table pointer acquisition.
//!
//! @par Requirements
//!
//! A subclass of `vptr` must contain a `fn<Registry>` class template
//! that fulfills the requirements of @ref vptr::fn.
struct vptr {
    using category = vptr;

#ifdef __MRDOCS__
    //! Requirements for `vptr` policies (exposition only)
    //!
    //! This class is for _exposition only_. It is the responsibility of
    //! subclasses to provide a `fn` class template that contains the members
    //! listed on this page.
    template<class Registry>
    struct fn {
        //! Stores the v-table pointers.
        //! @tparam ForwardIterator An iterator to a range of const
        //! @ref `IdsToVptr` objects.
        //! @param first The beginning of the range.
        //! @param last The end of the range.
        template<typename ForwardIterator>
        static auto initialize(ForwardIterator first, ForwardIterator last);

        //! Returns a *reference* to a v-table pointer for an object.
        //!
        //! @tparam Class A registered class.
        //! @param arg A reference to a const object of type `Class`.
        //! @return A reference to a the v-table pointer for `Class`.
        template<class Class>
        static auto dynamic_vptr(const Class& arg) -> const vptr_type&;

        //! Releases the resources allocated by `initialize`. This function is
        //! optional.
        static auto finalize() -> void;
    };
#endif
};

//! Policy to add an indirection to pointers to v-tables.
//!
//! If this policy is present, constructs like @ref virtual_ptr, @ref
//! inplace_vptr, @ref vptr_vector, etc store pointers to pointers to v-tables.
//! These indirect pointers remain valid after a call to @ref initialize, even
//! though the v-tables move to different locations. This is useful in presence
//! of dynamic loading.
//!
//! @par Requirements
//!
//! None. `indirect_vptr` can be added to a registry's policy list as-is.

struct indirect_vptr final {
    using category = indirect_vptr;
    template<class Registry>
    struct fn {};
};

#ifdef __MRDOCS__
class vptr_vector;
template<class MapFn>
class vptr_map;
#endif

//! Policy for type_id hashing.
//!
//! A @e type_hash policy calculates an integer hash for a @ref type_id.
//!
//! @par Requirements
//!
//! A subclass of `type_hash` must contain a `fn<Registry>` class template
//! that fulfills the requirements of @ref type_hash::fn.
struct type_hash {
    using category = type_hash;

#ifdef __MRDOCS__
    //! Requirements for `type_hash` policies (exposition only)
    //! @tparam Registry The registry containing this policy.
    //!
    //! This class is for _exposition only_. It is the responsibility of
    //! subclasses to provide a `fn` class template that contains the members
    //! listed on this page.
    template<class Registry>
    struct fn {
        //! Initializes the hash table.
        //! @tparam ForwardIterator An iterator to a range of const
        //! @ref IdsToVptr objects.
        //! @param first The beginning of the range.
        //! @param last The end of the range.
        //! @return A pair containing the minimum and maximum hash values.
        template<typename ForwardIterator>
        static auto initialize(ForwardIterator first, ForwardIterator last)
            -> std::pair<std::size_t, std::size_t>;

        //! Hashes a `type_id`.
        //! @param type A @ref type_id.
        //! @return A hash value for the given `type_id`.
        static auto hash(type_id type) -> std::size_t;

        //! Releases the resources allocated by `initialize`. This function is
        //! optional.
        static auto finalize() -> void;
    };
#endif
};

#ifdef __MRDOCS__
class fast_perfect_hash;
#endif

//! Policy for writing diagnostics and trace.
//!
//! If an `output` policy is present, the default error handler uses it to write
//! error messages to its output stream. `initialize` can also use it to write
//! trace messages.
//!
//! @par Requirements
//!
//! A subclass of `output` must contain a `fn<Registry>` class template
//! that fulfills the requirements of @ref output::fn.
struct output {
    using category = output;

#ifdef __MRDOCS__
    //! Requirements for `output` policies (exposition only)
    //!
    //! This class is for _exposition only_. It is the responsibility of
    //! subclasses to provide a `fn` class template that contains the members
    //! listed on this page.
    template<class Registry>
    struct fn {
        //! A @ref LightweightOutputStream.
        inline static LightweightOutputStream os;
    };
#endif
};

#ifdef __MRDOCS__
struct stderr_output;
#endif

//! Policy for tracing.
//!
//! If `trace` is present, trace instructions are added to various parts of the
//! initialization process (dispatch table construction, hash factors search,
//! etc). These instructions are executed only if `trace::fn<Registry>::on` is
//! set to `true`. The default value of `on` is `true` if environment variable
//! `BOOST_OPENMETHOD_TRACE` is set to the string "1". At the moment, any other
//! value disables tracing.
//!
//! `trace` requires an `output` policy to be present. Trace is written to its
//! output stream.
//!
//! The exact format of the trace output is not specified, and may change at any
//! time. The only guarantee is that it is detailed and comprehensive, and makes
//! it possible to troubleshoot problems like missing class registrations,
//! missing or ambiguous overriders, etc.
struct trace final {
    using category = trace;

    template<class Registry>
    struct fn {
        inline static bool on = []() {
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

//! Policy for runtime sanity checks.
//!
//! If this policy is present, various checks are performed at runtime.
//! Currently they all attempt to detect missing class registrations.
struct runtime_checks final {
    using category = runtime_checks;
    template<class Registry>
    struct fn {};
};

//! Policy for N2216 ambiguity resolution.
//!
//! If this policy is present, additional steps are taken to select a single
//! overrider in presence of ambiguous overriders sets, according to the rules
//! defined in the N2216 paper. If the normal resolution procedure fails to
//! select a single overrider, the following steps are applied, in order:
//!
//! - If the return types of the remaining overriders are all polymorphic and
//!   covariant, and one of the return types is more specialized thjat all the
//!   others, use it.
//!
//! - Otherwise, pick one of the overriders. Which one is used is unspecified,
//!   but remains the same throughtout the program, and across different runs of
//!   the same program.
struct n2216 final {
    using category = n2216;
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

template<class...>
struct use_class_aux;

} // namespace detail

//! A collection of methods and their associated dispatch data.
//!
//! Methods exist in a registry, which also contains descriptions for all the
//! classes that can appear in the methods, their overriders, and method calls.
//!
//! Before calling a method, the @ref initialize function must be called for its
//! registry to set up the dispatch tables. This is typically done at the
//! beginning of `main`.
//!
//! Multiple registries can co-exist in the same program. They must be
//! initialized independently. Classes referenced by methods in different
//! registries must be registered with each registry individually.
//!
//!
//!
//! @tparam Policies The policies used in the registry.
//!
//! @par Requirements
//!
//! `Policies` must be models of @ref policies::Policy. There may be at most one
//! policy per category, i.e. `Policies::category...` must all be different.
//!
//! @see @ref policies
template<class... Policies>
class registry : detail::registry_base {
    inline static detail::class_catalog classes;
    inline static detail::method_catalog methods;

    template<class...>
    friend struct detail::use_class_aux;
    template<typename Name, typename ReturnType, class Registry>
    friend class method;

    struct compiler;

    inline static std::vector<detail::word> dispatch_data;
    inline static bool initialized;

  public:
    //! Initializes the registry.
    //!
    //! `initialize` must be called, typically at the beginning of `main`,
    //! before using any of the methods in the registry. It sets up the
    //! v-tables, multi-method dispatch tables, and any other data required by
    //! the policies.
    //!
    //! @note
    //! A translation unit that contains a call to `initialize` must include the
    //! `<boost/openmethod/initialize.hpp>` header.
    //!
    //! @par Errors
    //!
    //! @li @ref unknown_class_error: A class used in a virtual parameter was
    //! not registered.
    //!
    //! In addition, policies may encounter and report errors.
    static auto initialize();

    //! Checks if the registry is initialized.
    //!
    //! Checks if `initialize` has been called for this registry, and report an
    //! error if not.
    //!
    //! @par Errors
    //!
    //! @li @ref not_initialized_error: The registry is not initialized.
    static void check_initialized();

    //! Releases the resources held by the registry.
    //!
    //! `finalize` may be called to release any resources allocated by
    //! `initialize`.
    //!
    //! @note
    //! A translation unit that contains a call to `finalize` must include the
    //! `<boost/openmethod/initialize.hpp>` header.
    static void finalize();

    //! A pointer to the virtual table for a registered class.
    //!
    //! `static_vptr` is set by @ref initialize to the address of the class's
    //! virtual table. It remains valid until the next call to `initialize` or
    //! `finalize`.
    //!
    //! @tparam Class A registered class.
    template<class Class>
    inline static vptr_type static_vptr;

    //! The list of policies selected in a registry.
    //!
    //! `policy_list` is a Boost.Mp11 list containing the policies passed to the
    //! @ref registry clas template.
    //!
    //! @tparam Class A registered class.
    using policy_list = mp11::mp_list<Policies...>;

    //! Returns the policy for a policy category.
    //!
    //! `policy` searches for a policy that derives from the specified @ref
    //! PolicyCategory. If none is found, it aliases to `void`. Otherwise, it
    //! aliases to the policy's `fn` class template, instantiated for this
    //! registry.
    //!
    //! @tparam Category A model of @ref policies::PolicyCategory.
    template<class Category>
    using policy = typename detail::get_policy_aux<
        registry,
        mp11::mp_find_if_q<
            policy_list,
            mp11::mp_bind_front_q<
                mp11::mp_quote_trait<std::is_base_of>, Category>>>::type;

    //! Returns a copy of this registry, with additional policies.
    //!
    //! `with` aliases to a registry containing `NewPolicies`, in addition to
    //! this registry's policies that are not in the same category as any of the
    //! `NewPolicies`.
    //!
    //! @tparam NewPolicies Models of @ref policies::Policy.
    template<class... NewPolicies>
    using with = boost::mp11::mp_apply<
        registry, typename detail::with_aux<policy_list, NewPolicies...>::type>;

    //! Returns a copy of this registry, with some policies removed.
    //!
    //! `without` returns a copy of this registry, without the policies that
    //! derive from `Categories`.
    //!
    //! @tparam Categories Models of @ref policies::PolicyCategory.
    template<class... Categories>
    using without = boost::mp11::mp_apply<
        registry,
        typename detail::without_aux<policy_list, Categories...>::type>;

    //! The registry's rtti policy.
    using rtti = policy<policies::rtti>;

    //! The registry's vptr policy if it contains one, or `void`.
    using vptr = policy<policies::vptr>;

    //! `true` if the registry has a vptr policy.
    static constexpr auto has_vptr = !std::is_same_v<vptr, void>;

    //! The registry's error_handler policy if it contains one, or `void`.
    using error_handler = policy<policies::error_handler>;

    //! `true` if the registry has an error_handler policy.
    static constexpr auto has_error_handler =
        !std::is_same_v<error_handler, void>;

    //! The registry's output policy if it contains one, or `void`.
    using output = policy<policies::output>;

    //! `true` if the registry has an output policy.
    static constexpr auto has_output = !std::is_same_v<output, void>;

    //! The registry's trace policy if it contains one, or `void`.
    using trace = policy<policies::trace>;

    //! `true` if the registry has a trace policy.
    static constexpr auto has_trace = !std::is_same_v<trace, void>;

    //! `true` if the registry has a deferred_static_rtti policy.
    static constexpr auto has_deferred_static_rtti =
        !std::is_same_v<policy<policies::deferred_static_rtti>, void>;

    //! `true` if the registry has a runtime_checks policy.
    static constexpr auto has_runtime_checks =
        !std::is_same_v<policy<policies::runtime_checks>, void>;

    //! `true` if the registry has an indirect_vptr policy.
    static constexpr auto has_indirect_vptr =
        !std::is_same_v<policy<policies::indirect_vptr>, void>;

    //! `true` if the registry has a n2216 policy.
    static constexpr auto has_n2216 =
        !std::is_same_v<policy<policies::n2216>, void>;
};

template<class... Policies>
inline void registry<Policies...>::check_initialized() {
    if constexpr (registry::has_runtime_checks) {
        if (!initialized) {
            if constexpr (registry::has_error_handler) {
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
auto fast_perfect_hash_error::write(Stream& os) const -> void {
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
