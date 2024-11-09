#ifndef BOOST_OPENMETHOD_DETAIL_TYPES_HPP
#define BOOST_OPENMETHOD_DETAIL_TYPES_HPP

#include <cstdint>
#include <limits>
#include <string_view>

#include <boost/openmethod/detail/static_list.hpp>

namespace boost {
namespace openmethod {

using type_id = std::uintptr_t;
constexpr type_id invalid_type = (std::numeric_limits<type_id>::max)();

template<typename T>
struct virtual_;

template<class Class, class Policy>
struct virtual_ptr;

using direct_vptr_type = const std::uintptr_t*;
using indirect_vptr_type = const direct_vptr_type*;
using vptr_type = direct_vptr_type;

namespace detail {

template<typename... Types>
struct types;

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
    std::uintptr_t** static_vptr;
    type_id *first_base, *last_base;
    bool is_abstract{false};

    auto vptr() const {
        return *static_vptr;
    }

    auto indirect_vptr() {
        return static_vptr;
    }

    auto type_id_begin() const {
        return &type;
    }

    auto type_id_end() const {
        return &type + 1;
    }
};

// -----------
// method info

struct overrider_info;

struct method_info : static_list<method_info>::static_link {
    type_id* vp_begin;
    type_id* vp_end;
    static_list<overrider_info> specs;
    void* not_implemented;
    type_id method_type;
    type_id return_type;
    std::size_t* slots_strides_ptr;

    auto arity() const {
        return std::distance(vp_begin, vp_end);
    }
};

struct overrider_info : static_list<overrider_info>::static_link {
    ~overrider_info() {
        method->specs.remove(*this);
    }

    method_info* method; // for the destructor, to remove definition
    type_id return_type; // for N2216 disambiguation
    type_id type;        // of the function, for trace
    void** next;
    type_id *vp_begin, *vp_end;
    void* pf;
};

} // namespace detail
} // namespace openmethod
} // namespace boost

#endif
