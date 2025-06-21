// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_DETAIL_TYPES_HPP
#define BOOST_OPENMETHOD_DETAIL_TYPES_HPP

#include <cstdint>

#include <boost/openmethod/detail/static_list.hpp>

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

template<class Class, class Registry>
class virtual_ptr;

template<typename T, class Registry>
struct virtual_traits;

// -----------------------------------------------------------------------------
// Error handling

struct openmethod_error {};

struct not_initialized_error : openmethod_error {
    template<class Registry, class Stream>
    auto write(Stream& os) const -> void {
        os << "not initialized";
    }
};

struct unknown_class_error : openmethod_error {
    type_id type;

    template<class Registry, class Stream>
    auto write(Stream& os) const -> void;
};

struct hash_search_error : openmethod_error {
    std::size_t attempts;
    std::size_t buckets;

    template<class Registry, class Stream>
    auto write(Stream& os) const -> void;
};

struct call_error : openmethod_error {
    type_id method;
    std::size_t arity;
    static constexpr std::size_t max_types = 16;
    type_id types[max_types];

  protected:
    template<class Registry, class Stream>
    auto write_aux(Stream& os, const char* subtype) const -> void;
};

struct not_implemented_error : call_error {
    template<class Registry, class Stream>
    auto write(Stream& os) const -> void {
        write_aux<Registry>(os, "not implemented");
    }
};

struct ambiguous_error : call_error {
    template<class Registry, class Stream>
    auto write(Stream& os) const -> void {
        write_aux<Registry>(os, "ambiguous");
    }
};

struct final_error : openmethod_error {
    type_id static_type, dynamic_type;

    template<class Registry, class Stream>
    auto write(Stream& os) const -> void;
};

struct static_offset_error : openmethod_error {
    type_id method;
    int actual, expected;

    template<class Registry, class Stream>
    auto write(Stream& os) const -> void;
};

struct static_slot_error : static_offset_error {};
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
