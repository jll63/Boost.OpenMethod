// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_DETAIL_TYPES_HPP
#define BOOST_OPENMETHOD_DETAIL_TYPES_HPP

#include <cstdint>

#include <boost/openmethod/detail/static_list.hpp>

#if BOOST_CXX_VERSION >= 202002L
#define BOOST_OPENMETHOD_DETAIL_CXX17(...)
#define BOOST_OPENMETHOD_DETAIL_CXX20(...) __VA_ARGS__
#else
#define BOOST_OPENMETHOD_DETAIL_CXX17(...) __VA_ARGS__
#define BOOST_OPENMETHOD_DETAIL_CXX20(...)
#endif

namespace boost::openmethod {

namespace detail {

union word {
    word() {
    } // undefined
    word(void (*pf)()) : pf(pf) {
    }
    word(word* pw) : pw(pw) {
    }
    word(std::size_t i) : i(i) {
    }

    void (*pf)();
    std::size_t i;
    word* pw;
};

#if defined(UINTPTR_MAX)
using uintptr = std::uintptr_t;
constexpr uintptr uintptr_max = UINTPTR_MAX;
#else
static_assert(
    sizeof(std::size_t) == sizeof(void*),
    "This implementation requires that size_t and void* have the same size.");
using uintptr = std::size_t;
constexpr uintptr uintptr_max = (std::numeric_limits<std::size_t>::max)();
#endif

} // namespace detail

using vptr_type = const detail::word*;

using type_id = const void*;

template<typename T>
struct virtual_;

template<typename T, class Registry>
struct virtual_traits;

// -----------------------------------------------------------------------------
// Error handling

struct openmethod_error {};

//! Registry not initialized
struct not_initialized_error : openmethod_error {
    //! Write a short description to an output stream
    //! @param os The output stream
    //! @tparam Registry The registry
    //! @tparam Stream A @ref LightweightOutputStream
    template<class Registry, class Stream>
    auto write(Stream& os) const -> void {
        os << "not initialized";
    }
};

//! Unknown class
struct unknown_class_error : openmethod_error {
    //! The type_id of the unknown class.
    type_id type;

    //! Write a short description to an output stream
    //! @param os The output stream
    //! @tparam Registry The registry
    //! @tparam Stream A @ref LightweightOutputStream
    template<class Registry, class Stream>
    auto write(Stream& os) const -> void;
};

//! Cannot find hash factors
struct fast_perfect_hash_error : openmethod_error {
    //! Number of attempts to find hash factors
    std::size_t attempts;
    //! Number of buckets used in the last attempt
    std::size_t buckets;

    //! Write a short description to an output stream
    //! @param os The output stream
    //! @tparam Registry The registry
    //! @tparam Stream A @ref LightweightOutputStream
    template<class Registry, class Stream>
    auto write(Stream& os) const -> void;
};

//! No valid overrider
struct call_error : openmethod_error {
    //! The type_id of method that was called
    type_id method;
    //! The number of @em virtual arguments in the call
    std::size_t arity;
    //! The maximum size of `types`
    static constexpr std::size_t max_types = 16;
    //! The type_ids of the arguments.
    type_id types[max_types];

  protected:
    template<class Registry, class Stream>
    auto write_aux(Stream& os, const char* subtype) const -> void;
};

//! No overrider for virtual tuple
//!
//! @see @ref call_error for data members.
struct not_implemented_error : call_error {
    //! Write a short description to an output stream
    //! @param os The output stream
    //! @tparam Registry The registry
    //! @tparam Stream A @ref LightweightOutputStream
    template<class Registry, class Stream>
    auto write(Stream& os) const -> void {
        write_aux<Registry>(os, "not implemented");
    }
};

//! Ambiguous call
//!
//! @see @ref call_error for data members.
struct ambiguous_error : call_error {
    //! Write a short description to an output stream
    //! @param os The output stream
    //! @tparam Registry The registry
    //! @tparam Stream A @ref LightweightOutputStream
    template<class Registry, class Stream>
    auto write(Stream& os) const -> void {
        write_aux<Registry>(os, "ambiguous");
    }
};

//! Static and dynamic type mismatch in "final" construct
//!
//! If runtime checks are enabled, the "final" construct checks that the static
//! and dynamic types of the object, as reported by the `rtti` policy,  are the
//! same. If they are not, and if the registry contains an @ref error_handler
//! policy, its @ref error function is called with a `final_error` object, then
//! the program is terminated with
//! @ref abort.
struct final_error : openmethod_error {
    type_id static_type, dynamic_type;

    //! Write a short description to an output stream
    //! @param os The output stream
    //! @tparam Registry The registry
    //! @tparam Stream A @ref LightweightOutputStream
    template<class Registry, class Stream>
    auto write(Stream& os) const -> void;
};

//! For future use
struct static_offset_error : openmethod_error {
    type_id method;
    int actual, expected;

    //! Write a short description to an output stream
    //! @param os The output stream
    //! @tparam Registry The registry
    //! @tparam Stream A @ref LightweightOutputStream
    template<class Registry, class Stream>
    auto write(Stream& os) const -> void;
};

//! For future use
struct static_slot_error : static_offset_error {};

//! For future use
struct static_stride_error : static_offset_error {};

namespace detail {

struct empty {};

template<typename Iterator>
struct range {
    range(Iterator first, Iterator last) : first(first), last(last) {
    }

    Iterator first, last;

    auto begin() const -> Iterator {
        return first;
    }

    auto end() const -> Iterator {
        return last;
    }
};

// -----------------------------------------------------------------------------
// class info

struct class_info : static_list<class_info>::static_link {
    type_id type;
    vptr_type* static_vptr;
    type_id *first_base, *last_base;
    bool is_abstract{false};

    auto vptr() const {
        return static_vptr;
    }

    auto type_id_begin() const {
        return &type;
    }

    auto type_id_end() const {
        return &type + 1;
    }
};

struct deferred_class_info : class_info {
    virtual void resolve_type_ids() = 0;
};

// -----------
// method info

struct overrider_info;

struct method_info : static_list<method_info>::static_link {
    type_id* vp_begin;
    type_id* vp_end;
    static_list<overrider_info> specs;
    void (*not_implemented)();
    void (*ambiguous)();
    type_id method_type_id;
    type_id return_type_id;
    std::size_t* slots_strides_ptr;

    auto arity() const {
        return std::distance(vp_begin, vp_end);
    }
};

struct deferred_method_info : method_info {
    virtual void resolve_type_ids() = 0;
};

struct overrider_info : static_list<overrider_info>::static_link {
    ~overrider_info() {
        method->specs.remove(*this);
    }

    method_info* method; // for the destructor, to remove definition
    type_id return_type; // for N2216 disambiguation
    type_id type;        // of the function, for trace
    void (**next)();
    type_id *vp_begin, *vp_end;
    void (*pf)();
};

struct deferred_overrider_info : overrider_info {
    virtual void resolve_type_ids() = 0;
};

} // namespace detail

} // namespace boost::openmethod

#endif
