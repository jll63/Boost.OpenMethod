#ifndef BOOST_OPENMETHOD_REGISTRY_HPP
#define BOOST_OPENMETHOD_REGISTRY_HPP

#include <boost/openmethod/detail/types.hpp>

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/bind.hpp>

#include <stdlib.h>
#include <vector>

#ifdef __MRDOCS__
#include <unordered_map>
#endif

namespace boost::openmethod {

//! Namespace containing the policy framework.
//!
//! A registry contains a list of policies that control how some operations are
//! performed. Policies fall into categories, depending on which subset of
//! operations they control. Each category has a corresponding class:
//!
//! - @ref rtti: type information acquisition and manipulation.
//!
//! - @ref vptr: v-table pointer acquisition.
//!
//! - @ref type_hash: hashing of `type_id`s.
//!
//! - @ref error_handler: perform operations when errors are detected.
//!
//! - @ref output: output stream for logging and debugging.
//!
//! - @ref runtime_checks: detect and report common errors.
//!
//! - @ref trace: report how dispatch tables are built.
//!
//! - @ref n2216: handle ambiguities according to the N2216 proposal.
//!
//! The last three policy categories act like flags. They can appear in the
//! registry's policy list, and enable existing blocks of code.
//!
//! The other categories cannot be used directly. They must be subclassed, and
//! the subclass must contain a `template<class Registry> struct fn` that
//! provides a set of static members that depends on the policy's category.
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

//! Requirements for LightweightOutputStream (exposition only)
struct LightweightOutputStream {
    LightweightOutputStream& operator<<(const char* str);
    LightweightOutputStream& operator<<(const std::string_view& view);
    LightweightOutputStream& operator<<(const void* value);
    LightweightOutputStream& operator<<(std::size_t value);
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
//! information, dynamic casting, and identifying polymorphic classes.
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
struct throw_handler;
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

template<class... Policies>
class registry : detail::registry_base {
    struct compiler;

    inline static detail::class_catalog classes;

    inline static detail::method_catalog methods;

    template<class...>
    friend struct detail::use_class_aux;
    template<typename Name, typename ReturnType, class Registry>
    friend class method;

  public:
    static auto initialize();
    static void check_initialized();
    static void finalize();

    //! `true` if `initialize` was called.
    inline static bool initialized;

    template<class Class>
    inline static vptr_type static_vptr;
    inline static std::vector<detail::word> dispatch_data;

    using policy_list = mp11::mp_list<Policies...>;

    template<class PolicyCategory>
    using policy = typename detail::get_policy_aux<
        registry,
        mp11::mp_find_if_q<
            policy_list,
            mp11::mp_bind_front_q<
                mp11::mp_quote_trait<std::is_base_of>, PolicyCategory>>>::type;

    template<class... NewPolicies>
    using with = boost::mp11::mp_apply<
        registry, typename detail::with_aux<policy_list, NewPolicies...>::type>;

    template<class... RemovePolicies>
    using without = boost::mp11::mp_apply<
        registry,
        typename detail::without_aux<policy_list, RemovePolicies...>::type>;

    using rtti = policy<policies::rtti>;

    using vptr = policy<policies::vptr>;
    static constexpr auto has_vptr = !std::is_same_v<vptr, void>;

    using error_handler = policy<policies::error_handler>;
    static constexpr auto has_error_handler =
        !std::is_same_v<error_handler, void>;

    using output = policy<policies::output>;
    static constexpr auto has_output = !std::is_same_v<output, void>;

    using trace = policy<policies::trace>;
    static constexpr auto has_trace = !std::is_same_v<trace, void>;

    static constexpr auto has_deferred_static_rtti =
        !std::is_same_v<policy<policies::deferred_static_rtti>, void>;
    static constexpr auto has_runtime_checks =
        !std::is_same_v<policy<policies::runtime_checks>, void>;
    static constexpr auto has_indirect_vptr =
        !std::is_same_v<policy<policies::indirect_vptr>, void>;
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
