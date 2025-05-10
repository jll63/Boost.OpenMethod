// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_HPP
#define BOOST_OPENMETHOD_HPP


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


// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_POLICY_HPP
#define BOOST_OPENMETHOD_POLICY_HPP


// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_POLICIES_CORE_HPP
#define BOOST_OPENMETHOD_POLICIES_CORE_HPP


// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_DETAIL_TYPES_HPP
#define BOOST_OPENMETHOD_DETAIL_TYPES_HPP

#include <cstdint>


// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_DETAIL_STATIC_LIST_HPP
#define BOOST_OPENMETHOD_DETAIL_STATIC_LIST_HPP

#include <algorithm>
#include <boost/assert.hpp>

namespace boost::openmethod {

namespace detail {

template<typename T>
class static_list {
  public:
    static_list(static_list&) = delete;
    static_list() = default;

    class static_link {
      public:
        static_link(const static_link&) = delete;
        static_link() = default;

        auto next() -> T* {
            return next_ptr;
        }

      protected:
        friend class static_list;
        T* prev_ptr;
        T* next_ptr;
    };

    void push_back(T& node) {
        BOOST_ASSERT(node.prev_ptr == nullptr);
        BOOST_ASSERT(node.next_ptr == nullptr);

        if (!first) {
            first = &node;
            node.prev_ptr = &node;
            return;
        }

        auto last = first->prev_ptr;
        last->next_ptr = &node;
        node.prev_ptr = last;
        first->prev_ptr = &node;
    }

    void remove(T& node) {
        BOOST_ASSERT(first != nullptr);

        auto prev = node.prev_ptr;
        auto next = node.next_ptr;
        auto last = first->prev_ptr;

        node.prev_ptr = nullptr;
        node.next_ptr = nullptr;

        if (&node == last) {
            if (&node == first) {
                first = nullptr;
                return;
            }

            first->prev_ptr = prev;
            prev->next_ptr = nullptr;
            return;
        }

        if (&node == first) {
            first = next;
            first->prev_ptr = last;
            return;
        }

        prev->next_ptr = next;
        next->prev_ptr = prev;
    }

    void clear() {
        auto next = first;
        first = nullptr;

        while (next) {
            auto cur = next;
            next = cur->next_ptr;
            cur->prev_ptr = nullptr;
            cur->next_ptr = nullptr;
        }
    }

    class iterator {
      public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = value_type*;
        using reference = value_type&;

        iterator() : ptr(nullptr) {
        }
        explicit iterator(T* p) : ptr(p) {
        }

        auto operator*() -> reference {
            return *ptr;
        }
        auto operator->() -> pointer {
            return ptr;
        }

        auto operator++() -> iterator& {
            BOOST_ASSERT(ptr);
            ptr = ptr->next_ptr;
            return *this;
        }

        auto operator++(int) -> iterator {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        friend auto operator==(const iterator& a, const iterator& b) -> bool {
            return a.ptr == b.ptr;
        };
        friend auto operator!=(const iterator& a, const iterator& b) -> bool {
            return a.ptr != b.ptr;
        };

      private:
        T* ptr;
    };

    auto begin() -> iterator {
        return iterator(first);
    }

    auto end() -> iterator {
        return iterator(nullptr);
    }

    class const_iterator {
      public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = const T;
        using pointer = value_type*;
        using reference = value_type&;

        const_iterator() : ptr(nullptr) {
        }
        explicit const_iterator(T* p) : ptr(p) {
        }

        auto operator*() -> reference {
            return *ptr;
        }
        auto operator->() -> pointer {
            return ptr;
        }

        auto operator++() -> const_iterator& {
            BOOST_ASSERT(ptr);
            ptr = ptr->next_ptr;
            return *this;
        }

        auto operator++(int) -> const_iterator {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        friend auto operator==(const const_iterator& a, const const_iterator& b)
            -> bool {
            return a.ptr == b.ptr;
        };
        friend auto operator!=(const const_iterator& a, const const_iterator& b)
            -> bool {
            return a.ptr != b.ptr;
        };

      private:
        T* ptr;
    };

    auto begin() const -> const_iterator {
        return const_iterator(first);
    }

    auto end() const -> const_iterator {
        return const_iterator(nullptr);
    }

    auto size() const -> std::size_t {
        return std::distance(begin(), end());
    }

    auto empty() const -> bool {
        return !first;
    }

  protected:
    T* first;
};

} // namespace detail
} // namespace boost::openmethod

#endif


namespace boost::openmethod {

using type_id = std::uintptr_t;
using vptr_type = const std::uintptr_t*;

template<typename T>
struct virtual_;

template<class Class, class Policy>
class virtual_ptr;

template<typename T, class Policy>
struct virtual_traits;

// -----------------------------------------------------------------------------
// Error handling

struct openmethod_error {};

struct not_implemented_error : openmethod_error {
    type_id method;
    std::size_t arity;
    static constexpr std::size_t max_types = 16;
    type_id types[max_types];
};

struct unknown_class_error : openmethod_error {
    type_id type;
};

struct hash_search_error : openmethod_error {
    std::size_t attempts;
    std::size_t buckets;
};

struct type_mismatch_error : openmethod_error {
    type_id type;
};

struct static_offset_error : openmethod_error {
    type_id method;
    int actual, expected;
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

} // namespace boost::openmethod

#endif


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


// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_POLICY_STD_RTTI_HPP
#define BOOST_OPENMETHOD_POLICY_STD_RTTI_HPP




#ifndef BOOST_NO_RTTI
#include <typeindex>
#include <typeinfo>
#include <boost/core/demangle.hpp>
#endif

namespace boost::openmethod::policies {

struct std_rtti : virtual rtti {
#ifndef BOOST_NO_RTTI
    template<class Class>
    static constexpr bool is_polymorphic = std::is_polymorphic_v<Class>;

    template<class Class>
    static auto static_type() -> type_id {
        auto tip = &typeid(Class);
        return reinterpret_cast<type_id>(tip);
    }

    template<class Class>
    static auto dynamic_type(const Class& obj) -> type_id {
        auto tip = &typeid(obj);
        return reinterpret_cast<type_id>(tip);
    }

    template<typename Stream>
    static auto type_name(type_id type, Stream& stream) -> void {
        stream << boost::core::demangle(
            reinterpret_cast<const std::type_info*>(type)->name());
    }

    static auto type_index(type_id type) -> std::type_index {
        return std::type_index(*reinterpret_cast<const std::type_info*>(type));
    }

    template<typename D, typename B>
    static auto dynamic_cast_ref(B&& obj) -> D {
        return dynamic_cast<D>(obj);
    }
#endif
};

} // namespace boost::openmethod::policies

#endif


// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_POLICY_VPTR_VECTOR_HPP
#define BOOST_OPENMETHOD_POLICY_VPTR_VECTOR_HPP




#include <variant>
#include <vector>

namespace boost::openmethod {

namespace detail {

template<class Policy>
inline std::vector<vptr_type> vptr_vector_vptrs;

template<class Policy>
inline std::vector<const vptr_type*> vptr_vector_indirect_vptrs;

} // namespace detail

namespace policies {

template<class Policy>
class vptr_vector : public extern_vptr {
  public:
    template<typename ForwardIterator>
    static auto register_vptrs(ForwardIterator first, ForwardIterator last)
        -> void {
        using namespace policies;

        std::size_t size;

        if constexpr (Policy::template has_facet<type_hash>) {
            auto report = Policy::hash_initialize(first, last);
            size = report.last + 1;
        } else {
            size = 0;

            for (auto iter = first; iter != last; ++iter) {
                for (auto type_iter = iter->type_id_begin();
                     type_iter != iter->type_id_end(); ++type_iter) {
                    size = (std::max)(size, *type_iter);
                }
            }

            ++size;
        }

        if constexpr (Policy::template has_facet<indirect_vptr>) {
            detail::vptr_vector_indirect_vptrs<Policy>.resize(size);
        } else {
            detail::vptr_vector_vptrs<Policy>.resize(size);
        }

        for (auto iter = first; iter != last; ++iter) {
            for (auto type_iter = iter->type_id_begin();
                 type_iter != iter->type_id_end(); ++type_iter) {
                auto index = *type_iter;

                if constexpr (Policy::template has_facet<type_hash>) {
                    index = Policy::hash_type_id(index);
                }

                if constexpr (Policy::template has_facet<indirect_vptr>) {
                    detail::vptr_vector_indirect_vptrs<Policy>[index] =
                        &iter->vptr();
                } else {
                    detail::vptr_vector_vptrs<Policy>[index] = iter->vptr();
                }
            }
        }
    }

    template<class Class>
    static auto dynamic_vptr(const Class& arg) -> const vptr_type& {
        auto index = Policy::dynamic_type(arg);

        if constexpr (Policy::template has_facet<type_hash>) {
            index = Policy::hash_type_id(index);
        }

        if constexpr (Policy::template has_facet<indirect_vptr>) {
            return *detail::vptr_vector_indirect_vptrs<Policy>[index];
        } else {
            return detail::vptr_vector_vptrs<Policy>[index];
        }
    }

    static auto finalize() -> void {
        if constexpr (Policy::template has_facet<indirect_vptr>) {
            detail::vptr_vector_indirect_vptrs<Policy>.clear();
        } else {
            detail::vptr_vector_vptrs<Policy>.clear();
        }
    }
};

} // namespace policies
} // namespace boost::openmethod

#endif


// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_POLICY_BASIC_ERROR_OUTPUT_HPP
#define BOOST_OPENMETHOD_POLICY_BASIC_ERROR_OUTPUT_HPP




// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_DETAIL_OSTDSTREAM_HPP
#define BOOST_OPENMETHOD_DETAIL_OSTDSTREAM_HPP

#include <array>
#include <cstdio>
#include <charconv>
#include <random>

namespace boost::openmethod {

namespace detail {

// -----------------------------------------------------------------------------
// lightweight ostream

struct ostdstream {
    FILE* stream = nullptr;

    ostdstream(FILE* stream = nullptr) : stream(stream) {
    }

    void on(FILE* stream = stderr) {
        this->stream = stream;
    }

    void off() {
        this->stream = nullptr;
    }

    auto is_on() const -> bool {
        return stream != nullptr;
    }
};

struct ostderr : ostdstream {
    ostderr() : ostdstream(stderr) {
    }
};

inline ostdstream cerr;

inline auto operator<<(ostdstream& os, const char* str) -> ostdstream& {
    if (os.stream) {
        fputs(str, os.stream);
    }

    return os;
}

inline auto operator<<(ostdstream& os, const std::string_view& view)
    -> ostdstream& {
    if (os.stream) {
        fwrite(view.data(), sizeof(*view.data()), view.length(), os.stream);
    }

    return os;
}

inline auto operator<<(ostdstream& os, const void* value) -> ostdstream& {
    if (os.stream) {
        std::array<char, 20> str;
        auto end = std::to_chars(
                       str.data(), str.data() + str.size(),
                       reinterpret_cast<uintptr_t>(value), 16)
                       .ptr;
        os << std::string_view(str.data(), end - str.data());
    }

    return os;
}

inline auto operator<<(ostdstream& os, std::size_t value) -> ostdstream& {
    if (os.stream) {
        std::array<char, 20> str;
        auto end =
            std::to_chars(str.data(), str.data() + str.size(), value).ptr;
        os << std::string_view(str.data(), end - str.data());
    }

    return os;
}

} // namespace detail

} // namespace boost::openmethod

#endif


namespace boost::openmethod::policies {

template<class Policy, typename Stream = detail::ostderr>
struct basic_error_output : error_output {
    inline static Stream error_stream;
};

} // namespace boost::openmethod::policies

#endif



// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_POLICY_BASIC_TRACE_OUTPUT_HPP
#define BOOST_OPENMETHOD_POLICY_BASIC_TRACE_OUTPUT_HPP






#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

namespace boost::openmethod::policies {

template<class Policy, typename Stream = detail::ostderr>
struct basic_trace_output : virtual trace_output {
    inline static bool trace_enabled = []() {
        auto env = getenv("BOOST_OPENMETHOD_TRACE");
        return env && *env++ == '1' && *env++ == 0;
    }();
    inline static Stream trace_stream;
};

} // namespace boost::openmethod::policies

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif


// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_POLICY_FAST_PERFECT_HASH_HPP
#define BOOST_OPENMETHOD_POLICY_FAST_PERFECT_HASH_HPP




#include <limits>
#include <random>

namespace boost::openmethod {

namespace detail {

template<class Policy>
std::vector<type_id> fast_perfect_hash_control;

}

namespace policies {

template<class Policy>
class fast_perfect_hash : public type_hash {

    inline static type_id hash_mult;
    inline static std::size_t hash_shift;
    inline static std::size_t hash_min;
    inline static std::size_t hash_max;
    inline static void check(std::size_t index, type_id type);

  public:
    struct report {
        std::size_t first, last;
    };

    BOOST_FORCEINLINE
    static auto hash_type_id(type_id type) -> type_id {
        auto index = (hash_mult * type) >> hash_shift;

        if constexpr (Policy::template has_facet<runtime_checks>) {
            check(index, type);
        }

        return index;
    }

    template<typename ForwardIterator>
    static auto hash_initialize(ForwardIterator first, ForwardIterator last) {
        if constexpr (Policy::template has_facet<runtime_checks>) {
            hash_initialize(
                first, last, detail::fast_perfect_hash_control<Policy>);
        } else {
            std::vector<type_id> buckets;
            hash_initialize(first, last, buckets);
        }

        return report{hash_min, hash_max};
    }

    template<typename ForwardIterator>
    static void hash_initialize(
        ForwardIterator first, ForwardIterator last,
        std::vector<type_id>& buckets);

    static auto finalize() -> void {
        detail::fast_perfect_hash_control<Policy>.clear();
    }
};

template<class Policy>
template<typename ForwardIterator>
void fast_perfect_hash<Policy>::hash_initialize(
    ForwardIterator first, ForwardIterator last,
    std::vector<type_id>& buckets) {
    using namespace policies;

    constexpr bool trace_enabled = Policy::template has_facet<trace_output>;
    const auto N = std::distance(first, last);

    if constexpr (trace_enabled) {
        if (Policy::trace_enabled) {
            Policy::trace_stream << "Finding hash factor for " << N
                                 << " types\n";
        }
    }

    std::default_random_engine rnd(13081963);
    std::size_t total_attempts = 0;
    std::size_t M = 1;

    for (auto size = N * 5 / 4; size >>= 1;) {
        ++M;
    }

    std::uniform_int_distribution<type_id> uniform_dist;

    for (std::size_t pass = 0; pass < 4; ++pass, ++M) {
        hash_shift = 8 * sizeof(type_id) - M;
        auto hash_size = 1 << M;
        hash_min = (std::numeric_limits<std::size_t>::max)();
        hash_max = (std::numeric_limits<std::size_t>::min)();

        if constexpr (trace_enabled) {
            if (Policy::trace_enabled) {
                Policy::trace_stream << "  trying with M = " << M << ", "
                                     << hash_size << " buckets\n";
            }
        }

        std::size_t attempts = 0;
        buckets.resize(hash_size);

        while (attempts < 100000) {
            std::fill(buckets.begin(), buckets.end(), static_cast<type_id>(-1));
            ++attempts;
            ++total_attempts;
            hash_mult = uniform_dist(rnd) | 1;

            for (auto iter = first; iter != last; ++iter) {
                for (auto type_iter = iter->type_id_begin();
                     type_iter != iter->type_id_end(); ++type_iter) {
                    auto type = *type_iter;
                    auto index = (type * hash_mult) >> hash_shift;
                    hash_min = (std::min)(hash_min, index);
                    hash_max = (std::max)(hash_max, index);

                    if (buckets[index] != static_cast<type_id>(-1)) {
                        goto collision;
                    }

                    buckets[index] = type;
                }
            }

            if constexpr (trace_enabled) {
                if (Policy::trace_enabled) {
                    Policy::trace_stream << "  found " << hash_mult << " after "
                                         << total_attempts
                                         << " attempts; span = [" << hash_min
                                         << ", " << hash_max << "]\n";
                }
            }

            return;

        collision: {}
        }
    }

    hash_search_error error;
    error.attempts = total_attempts;
    error.buckets = std::size_t(1) << M;

    if constexpr (Policy::template has_facet<error_handler>) {
        Policy::error(error);
    }

    abort();
}

template<class Policy>
void fast_perfect_hash<Policy>::check(std::size_t index, type_id type) {
    if (index < hash_min || index > hash_max ||
        detail::fast_perfect_hash_control<Policy>[index] != type) {
        if constexpr (Policy::template has_facet<error_handler>) {
            unknown_class_error error;
            error.type = type;
            Policy::error(error);
        }

        abort();
    }
}

} // namespace policies
} // namespace boost::openmethod

#endif


// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_POLICY_VECTORED_ERROR_HPP
#define BOOST_OPENMETHOD_POLICY_VECTORED_ERROR_HPP




#include <functional>
#include <variant>

namespace boost::openmethod::policies {

template<class Policy>
class vectored_error_handler : public error_handler {
  public:
    using error_variant = std::variant<
        openmethod_error, not_implemented_error, unknown_class_error,
        hash_search_error, type_mismatch_error, static_slot_error,
        static_stride_error>;

    using function_type = std::function<void(const error_variant& error)>;

    template<class Error>
    static auto error(const Error& error) -> void {
        fn(error_variant(error));
    }

    static auto set_error_handler(function_type handler) -> function_type {
        auto prev = fn;
        fn = handler;

        return prev;
    }

  private:
    static auto default_handler(const error_variant& error_v) {
        using namespace detail;
        using namespace policies;

        if constexpr (Policy::template has_facet<error_output>) {
            if (auto error = std::get_if<not_implemented_error>(&error_v)) {
                Policy::error_stream << "no applicable overrider for ";
                Policy::type_name(error->method, Policy::error_stream);
                Policy::error_stream << "(";
                auto comma = "";

                for (auto ti :
                     range{error->types, error->types + error->arity}) {
                    Policy::error_stream << comma;
                    Policy::type_name(ti, Policy::error_stream);
                    comma = ", ";
                }

                Policy::error_stream << ")\n";
            } else if (
                auto error = std::get_if<unknown_class_error>(&error_v)) {
                Policy::error_stream << "unknown class ";
                Policy::type_name(error->type, Policy::error_stream);
                Policy::error_stream << "\n";
            } else if (
                auto error = std::get_if<type_mismatch_error>(&error_v)) {
                Policy::error_stream << "invalid method table for ";
                Policy::type_name(error->type, Policy::error_stream);
                Policy::error_stream << "\n";
            } else if (auto error = std::get_if<hash_search_error>(&error_v)) {
                Policy::error_stream << "could not find hash factors after "
                                     << error->attempts << "s using "
                                     << error->buckets << " buckets\n";
            }
        }
    }

    static function_type fn; // Cannot be inline static because it confuses MSVC
};

template<class Policy>
typename vectored_error_handler<Policy>::function_type
vectored_error_handler<Policy>::fn = default_handler;

} // namespace boost::openmethod::policies

#endif


namespace boost::openmethod {

namespace policies {

struct release : basic_policy<
                     release, std_rtti, fast_perfect_hash<release>,
                     vptr_vector<release>, vectored_error_handler<release>> {};

struct debug : release::fork<debug>::with<
                   runtime_checks, basic_error_output<debug>,
                   basic_trace_output<debug>> {};

} // namespace policies

#ifdef NDEBUG
using default_policy = policies::release;
#else
using default_policy = policies::debug;
#endif

} // namespace boost::openmethod

#endif


#ifndef BOOST_OPENMETHOD_DEFAULT_POLICY
#define BOOST_OPENMETHOD_DEFAULT_POLICY ::boost::openmethod::default_policy
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4646)
#endif

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
struct type_id_list<mp11::mp_list<T...>, Policy> {
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
type_id type_id_list<mp11::mp_list<T...>, Policy>::value[values] = {
    collect_static_type_id<Policy, T>()...};

template<typename... T, class Policy>
type_id* type_id_list<mp11::mp_list<T...>, Policy>::begin = value;

template<typename... T, class Policy>
type_id* type_id_list<mp11::mp_list<T...>, Policy>::end = value + sizeof...(T);

template<class Policy>
struct type_id_list<mp11::mp_list<>, Policy> {
    static constexpr type_id* const begin = nullptr;
    static constexpr auto end = begin;
};

template<class...>
struct class_declaration_aux;

template<class Policy, class Class, typename... Bases>
struct class_declaration_aux<Policy, mp11::mp_list<Class, Bases...>>
    : class_info {
    class_declaration_aux() {
        this->type = collect_static_type_id<Policy, Class>();
        this->first_base = type_id_list<mp11::mp_list<Bases...>, Policy>::begin;
        this->last_base = type_id_list<mp11::mp_list<Bases...>, Policy>::end;
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
using inheritance_map = mp11::mp_list<boost::mp11::mp_push_front<
    boost::mp11::mp_filter_q<
        boost::mp11::mp_bind_back<std::is_base_of, Cs>, mp11::mp_list<Cs...>>,
    Cs>...>;

// =============================================================================
// Policy helpers

template<typename T>
constexpr bool is_policy = std::is_base_of_v<policies::abstract_policy, T>;

template<typename...>
struct extract_policy;

template<>
struct extract_policy<> {
    using policy = BOOST_OPENMETHOD_DEFAULT_POLICY;
    using others = mp11::mp_list<>;
};

template<typename Type>
struct extract_policy<Type> {
    using policy = std::conditional_t<
        is_policy<Type>, Type, BOOST_OPENMETHOD_DEFAULT_POLICY>;
    using others = std::conditional_t<
        is_policy<Type>, mp11::mp_list<>, mp11::mp_list<Type>>;
};

template<typename Type1, typename Type2, typename... MoreTypes>
struct extract_policy<Type1, Type2, MoreTypes...> {
    static_assert(!is_policy<Type1>, "policy must be the last in the list");
    using policy = typename extract_policy<Type2, MoreTypes...>::policy;
    using others = mp11::mp_push_front<
        typename extract_policy<Type2, MoreTypes...>::others, Type1>;
};

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
        return Policy::template dynamic_cast_ref<D>(std::forward<B>(obj));
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
    static auto peek(const T&) {
        return nullptr;
    }

    template<typename>
    static auto cast(T value) -> T {
        return value;
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

    static auto peek(const T& arg) -> const T& {
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

    static auto peek(const T& arg) -> const T& {
        return arg;
    }

    template<typename D>
    static auto cast(T&& obj) -> D&& {
        return detail::optimal_cast<Policy, D&&>(obj);
    }
};

template<typename T, class Policy>
struct virtual_traits<T*, Policy> {
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
                typename detail::extract_policy<Classes...>::policy>,
            boost::mp11::mp_apply<
                detail::inheritance_map,
                typename detail::extract_policy<Classes...>::others>>>;
    tuple_type tuple;
};

// =============================================================================
// virtual_ptr

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

template<class Class, class Policy, typename = void>
class virtual_ptr_impl {
  public:
    using traits = virtual_traits<Class&, Policy>;
    using element_type = Class;
    static constexpr bool is_smart_ptr = false;

    static constexpr bool use_indirect_vptrs =
        Policy::template has_facet<policies::indirect_vptr>;

    virtual_ptr_impl() = default;

    explicit virtual_ptr_impl(std::nullptr_t)
        : vp(box_vptr<use_indirect_vptrs>(null_vptr)), obj(nullptr) {
    }

    template<
        class Other,
        typename = std::enable_if_t<
            std::is_constructible_v<Class*, Other*> &&
            Policy::template is_polymorphic<Class>>>
    virtual_ptr_impl(Other& other)
        : vp(box_vptr<use_indirect_vptrs>(Policy::dynamic_vptr(other))),
          obj(&other) {
    }

    template<
        class Other,
        typename = std::enable_if_t<
            std::is_constructible_v<
                Class*,
                decltype(std::declval<virtual_ptr<Other, Policy>>().get())> &&
            Policy::template is_polymorphic<Class>>>
    virtual_ptr_impl(Other* other)
        : vp(box_vptr<use_indirect_vptrs>(Policy::dynamic_vptr(*other))),
          obj(other) {
    }

    template<
        class Other,
        typename = std::enable_if_t<std::is_constructible_v<
            Class*,
            decltype(std::declval<virtual_ptr<Other, Policy>>().get())>>>
    virtual_ptr_impl(const virtual_ptr<Other, Policy>& other)
        : vp(other.vp), obj(other.get()) {
    }

    template<
        class Other,
        typename = std::enable_if_t<std::is_constructible_v<
            Class*,
            decltype(std::declval<virtual_ptr<Other, Policy>>().get())>>>
    virtual_ptr_impl(virtual_ptr_impl<Other, Policy>& other)
        : vp(other.vp), obj(other.get()) {
        // Why is this needed? Consider this conversion conversion from
        // smart to dumb pointer:
        //      virtual_ptr<std::shared_ptr<const Node>> p = ...;
        //      virtual_ptr<const Node> q = p;
        // Since 'p' is not const, in the absence of this ctor,
        // virtual_ptr_impl(Other&) would be preferred to
        // virtual_ptr_impl(const virtual_ptr<Other, Policy>& other), and
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
            Policy::template is_polymorphic<Class>>>
    virtual_ptr_impl& operator=(Other& other) {
        obj = &other;
        vp = box_vptr<use_indirect_vptrs>(Policy::dynamic_vptr(other));
        return *this;
    }

    template<
        class Other,
        typename = std::enable_if_t<
            std::is_assignable_v<Class*, Other*> &&
            Policy::template is_polymorphic<Class>>>
    virtual_ptr_impl& operator=(Other* other) {
        obj = other;
        vp = box_vptr<use_indirect_vptrs>(Policy::dynamic_vptr(*other));
        return *this;
    }

    template<
        class Other,
        typename = std::enable_if_t<std::is_assignable_v<
            Class*,
            decltype(std::declval<virtual_ptr<Other, Policy>>().get())>>>
    virtual_ptr_impl& operator=(const virtual_ptr_impl<Other, Policy>& other) {
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

        return virtual_ptr<Other, Policy>(
            traits::template cast<Other&>(*obj), unbox_vptr(vp));
    }

    template<class, class>
    friend struct virtual_traits;

  protected:
    std::conditional_t<use_indirect_vptrs, const vptr_type*, vptr_type> vp;
    Class* obj;
};

template<class Class, class Other, class Policy, typename = void>
struct same_smart_ptr_aux : std::false_type {};

template<class Class, class Other, class Policy>
struct same_smart_ptr_aux<
    Class, Other, Policy,
    std::void_t<typename virtual_traits<Class, Policy>::template rebind<
        typename Other::element_type>>>
    : std::is_same<
          Other,
          typename virtual_traits<Class, Policy>::template rebind<
              typename Other::element_type>> {};

template<class Class, class Other, class Policy>
constexpr bool same_smart_ptr = same_smart_ptr_aux<Class, Other, Policy>::value;

template<class Class, class Policy>
class virtual_ptr_impl<
    Class, Policy,
    std::void_t<
        typename virtual_traits<Class, Policy>::template rebind<Class>>> {

  public:
    using traits = virtual_traits<Class, Policy>;
    using element_type = typename Class::element_type;

    template<class, class>
    friend class virtual_ptr;
    template<class, class, typename>
    friend class virtual_ptr_impl;
    template<class, class>
    friend struct virtual_traits;

  protected:
    static constexpr bool use_indirect_vptrs =
        Policy::template has_facet<policies::indirect_vptr>;

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
            same_smart_ptr<Class, Other, Policy> &&
            std::is_constructible_v<Class, const Other&> &&
            Policy::template is_polymorphic<element_type>>>
    virtual_ptr_impl(const Other& other)
        : vp(box_vptr<use_indirect_vptrs>(
              other ? Policy::dynamic_vptr(*other) : null_vptr)),
          obj(other) {
    }

    template<
        class Other,
        typename = std::enable_if_t<
            same_smart_ptr<Class, Other, Policy> &&
            std::is_constructible_v<Class, Other&> &&
            Policy::template is_polymorphic<element_type>>>
    virtual_ptr_impl(Other& other)
        : vp(box_vptr<use_indirect_vptrs>(
              other ? Policy::dynamic_vptr(*other) : null_vptr)),
          obj(other) {
    }

    template<
        class Other,
        typename = std::enable_if_t<
            same_smart_ptr<Class, Other, Policy> &&
            std::is_constructible_v<Class, Other&&> &&
            Policy::template is_polymorphic<element_type>>>
    virtual_ptr_impl(Other&& other)
        : vp(box_vptr<use_indirect_vptrs>(
              other ? Policy::dynamic_vptr(*other) : null_vptr)),
          obj(std::move(other)) {
    }

    template<
        class Other,
        typename = std::enable_if_t<
            same_smart_ptr<Class, Other, Policy> &&
            std::is_constructible_v<Class, const Other&>>>
    virtual_ptr_impl(const virtual_ptr_impl<Other, Policy>& other)
        : vp(other.vp), obj(other.obj) {
    }

    template<
        class Other,
        typename = std::enable_if_t<
            same_smart_ptr<Class, Other, Policy> &&
            std::is_constructible_v<Class, Other&>>>
    virtual_ptr_impl(virtual_ptr_impl<Other, Policy>& other)
        : vp(other.vp), obj(other.obj) {
    }

    virtual_ptr_impl(virtual_ptr_impl&& other)
        : vp(other.vp), obj(std::move(other.obj)) {
        other.vp = box_vptr<use_indirect_vptrs>(null_vptr);
    }

    template<
        class Other,
        typename = std::enable_if_t<
            same_smart_ptr<Class, Other, Policy> &&
            std::is_constructible_v<Class, Other&&>>>
    virtual_ptr_impl(virtual_ptr_impl<Other, Policy>&& other)
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
            same_smart_ptr<Class, Other, Policy> &&
            std::is_assignable_v<Class, const Other&> &&
            Policy::template is_polymorphic<element_type>>>
    virtual_ptr_impl& operator=(const Other& other) {
        obj = other;
        vp = box_vptr<use_indirect_vptrs>(Policy::dynamic_vptr(*other));
        return *this;
    }

    template<
        class Other,
        typename = std::enable_if_t<
            same_smart_ptr<Class, Other, Policy> &&
            std::is_assignable_v<Class, Other&&> &&
            Policy::template is_polymorphic<element_type>>>
    virtual_ptr_impl& operator=(Other&& other) {
        vp = box_vptr<use_indirect_vptrs>(
            other ? Policy::dynamic_vptr(*other) : null_vptr);
        obj = std::move(other);
        return *this;
    }

    template<
        class Other,
        typename = std::enable_if_t<
            same_smart_ptr<Class, Other, Policy> &&
            std::is_assignable_v<Class, Other&>>>
    virtual_ptr_impl& operator=(virtual_ptr_impl<Other, Policy>& other) {
        obj = other.obj;
        vp = other.vp;
        return *this;
    }

    virtual_ptr_impl& operator=(const virtual_ptr_impl& other) = default;

    template<
        class Other,
        typename = std::enable_if_t<
            same_smart_ptr<Class, Other, Policy> &&
            std::is_assignable_v<Class, const Other&>>>
    virtual_ptr_impl& operator=(const virtual_ptr_impl<Other, Policy>& other) {
        obj = other.obj;
        vp = other.vp;
        return *this;
    }

    template<
        class Other,
        typename = std::enable_if_t<
            same_smart_ptr<Class, Other, Policy> &&
            std::is_assignable_v<Class, Other&&>>>
    virtual_ptr_impl& operator=(virtual_ptr_impl<Other, Policy>&& other) {
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

        return virtual_ptr<other_smart_ptr, Policy>(
            traits::template cast<other_smart_ptr>(obj), vp);
    }

    template<class Other>
    auto cast() const& -> decltype(auto) {
        static_assert(
            std::is_base_of_v<element_type, Other> ||
            std::is_base_of_v<Other, element_type>);

        using other_smart_ptr = typename traits::template rebind<Other>;

        return virtual_ptr<other_smart_ptr, Policy>(
            traits::template cast<other_smart_ptr>(obj), vp);
    }

    template<class Other>
    auto cast() && -> decltype(auto) {
        static_assert(
            std::is_base_of_v<element_type, Other> ||
            std::is_base_of_v<Other, element_type>);

        using other_smart_ptr = typename traits::template rebind<Other>;

        return virtual_ptr<other_smart_ptr, Policy>(
            traits::template cast<other_smart_ptr>(std::move(obj)), vp);
    }
};

} // namespace detail

template<class Class, class Policy = BOOST_OPENMETHOD_DEFAULT_POLICY>
class virtual_ptr : public detail::virtual_ptr_impl<Class, Policy> {
    using impl = detail::virtual_ptr_impl<Class, Policy>;

  public:
    using detail::virtual_ptr_impl<Class, Policy>::virtual_ptr_impl;
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
        using other_traits = virtual_traits<Other, Policy>;
        using other_class = typename other_traits::virtual_type;

        static_assert(
            std::is_base_of_v<element_type, other_class> ||
            std::is_base_of_v<other_class, element_type>);

        if constexpr (
            Policy::template has_facet<policies::runtime_checks> &&
            Policy::template is_polymorphic<
                typename impl::traits::virtual_type> &&
            Policy::template is_polymorphic<other_class>) {
            // check that dynamic type == static type
            auto static_type = Policy::template static_type<other_class>();

            type_id dynamic_type;

            dynamic_type = Policy::dynamic_type(other_traits::peek(obj));

            if (dynamic_type != static_type) {
                type_mismatch_error error;
                error.type = dynamic_type;
                Policy::error(error);
                abort();
            }
        }

        return virtual_ptr(
            std::forward<Other>(obj),
            detail::box_vptr<impl::use_indirect_vptrs>(
                Policy::template static_vptr<other_class>));
    }

    auto vptr() const {
        return detail::unbox_vptr(this->vp);
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
auto operator==(
    const virtual_ptr<Left, Policy>& left,
    const virtual_ptr<Right, Policy>& right) -> bool {
    return &*left == &*right;
}

template<class Left, class Right, class Policy>
auto operator!=(
    const virtual_ptr<Left, Policy>& left,
    const virtual_ptr<Right, Policy>& right) -> bool {
    return !(left == right);
}

template<class Class, class Policy>
struct virtual_traits<virtual_ptr<Class, Policy>, Policy> {
    using virtual_type = typename virtual_ptr<Class, Policy>::element_type;

    static auto peek(const virtual_ptr<Class, Policy>& ptr)
        -> const virtual_ptr<Class, Policy>& {
        return ptr;
    }

    template<typename Derived>
    static auto cast(const virtual_ptr<Class, Policy>& ptr) -> decltype(auto) {
        return ptr.template cast<typename Derived::element_type>();
    }

    template<typename Derived>
    static auto cast(virtual_ptr<Class, Policy>&& ptr) -> decltype(auto) {
        return std::move(ptr).template cast<typename Derived::element_type>();
    }
};

template<class Class, class Policy>
struct virtual_traits<const virtual_ptr<Class, Policy>&, Policy> {
    using virtual_type = typename virtual_ptr<Class, Policy>::element_type;

    static auto peek(const virtual_ptr<Class, Policy>& ptr)
        -> const virtual_ptr<Class, Policy>& {
        return ptr;
    }

    template<typename Derived>
    static auto cast(const virtual_ptr<Class, Policy>& ptr) -> decltype(auto) {
        return ptr.template cast<
            typename std::remove_reference_t<Derived>::element_type>();
    }

    template<typename Derived>
    static auto cast(virtual_ptr<Class, Policy>&& ptr) -> decltype(auto) {
        return std::move(ptr).template cast<typename Derived::element_type>();
    }
};

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

void boost_openmethod_vptr(...);

template<class Class>
constexpr bool has_vptr_fn = std::is_same_v<
    decltype(boost_openmethod_vptr(std::declval<const Class&>())), vptr_type>;

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
                DeclaredParameters, mp11::mp_list<OverriderParameters...>,
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
    auto pf = resolve(parameter_traits<Parameters, Policy>::peek(args)...);

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
        pf = resolve_uni<mp11::mp_list<Parameters...>, ArgType...>(args...);
    } else {
        pf = resolve_multi_first<mp11::mp_list<Parameters...>, ArgType...>(
            args...);
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
    } else if constexpr (detail::has_vptr_fn<ArgType>) {
        return boost_openmethod_vptr(arg);
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
        vptr_type vtbl = vptr<ArgType>(arg);

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
        vptr_type vtbl = vptr<ArgType>(arg);
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
        vptr_type vtbl = vptr<ArgType>(arg);
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

namespace detail {

template<class Policy, class Class>
auto error_type_id(const Class& obj) {
    if constexpr (Policy::template is_polymorphic<Class>) {
        return Policy::template dynamic_type<Class>(obj);
    } else {
        return Policy::template static_type<void>();
    }
}

} // namespace detail

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
         (*ti_iter++ = detail::error_type_id<Policy>(
              detail::parameter_traits<Parameters, Policy>::peek(args))));
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

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif


// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_MACROS_HPP
#define BOOST_OPENMETHOD_MACROS_HPP

#include <boost/preprocessor/cat.hpp>




namespace boost::openmethod::detail {

template<typename, class Method, typename ReturnType, typename... Parameters>
struct enable_forwarder;

template<class Method, typename ReturnType, typename... Parameters>
struct enable_forwarder<
    std::void_t<decltype(Method::fn(std::declval<Parameters>()...))>, Method,
    ReturnType, Parameters...> {
    using type = ReturnType;
};

} // namespace boost::openmethod::detail

#define BOOST_OPENMETHOD_GENSYM BOOST_PP_CAT(openmethod_gensym_, __COUNTER__)

#define BOOST_OPENMETHOD_REGISTER(...)                                         \
    static __VA_ARGS__ BOOST_OPENMETHOD_GENSYM

#define BOOST_OPENMETHOD_NAME(NAME) NAME##_boost_openmethod

#define BOOST_OPENMETHOD_OVERRIDERS(NAME)                                      \
    BOOST_PP_CAT(BOOST_OPENMETHOD_NAME(NAME), _overriders)

#define BOOST_OPENMETHOD_GUIDE(NAME)                                           \
    BOOST_PP_CAT(BOOST_OPENMETHOD_NAME(NAME), _guide)

#define BOOST_OPENMETHOD(NAME, ARGS, ...)                                      \
    struct BOOST_OPENMETHOD_NAME(NAME);                                        \
    template<typename... ForwarderParameters>                                  \
    typename ::boost::openmethod::detail::enable_forwarder<                    \
        void,                                                                  \
        ::boost::openmethod::method<                                           \
            BOOST_OPENMETHOD_NAME(NAME) ARGS, __VA_ARGS__>,                    \
        typename ::boost::openmethod::method<                                  \
            BOOST_OPENMETHOD_NAME(NAME) ARGS, __VA_ARGS__>,                    \
        ForwarderParameters...>::type                                          \
        BOOST_OPENMETHOD_GUIDE(NAME)(ForwarderParameters && ... args);         \
    template<typename... ForwarderParameters>                                  \
    inline auto NAME(ForwarderParameters&&... args) ->                         \
        typename ::boost::openmethod::detail::enable_forwarder<                \
            void,                                                              \
            ::boost::openmethod::method<                                       \
                BOOST_OPENMETHOD_NAME(NAME) ARGS, __VA_ARGS__>,                \
            typename ::boost::openmethod::method<                              \
                BOOST_OPENMETHOD_NAME(NAME) ARGS, __VA_ARGS__>::return_type,   \
            ForwarderParameters...>::type {                                    \
        return ::boost::openmethod::                                           \
            method<BOOST_OPENMETHOD_NAME(NAME) ARGS, __VA_ARGS__>::fn(         \
                std::forward<ForwarderParameters>(args)...);                   \
    }

#define BOOST_OPENMETHOD_DETAIL_LOCATE_METHOD(NAME, ARGS)                      \
    template<typename T>                                                       \
    struct boost_openmethod_detail_locate_method_aux;                          \
    template<typename... A>                                                    \
    struct boost_openmethod_detail_locate_method_aux<void(A...)> {             \
        using type =                                                           \
            decltype(BOOST_OPENMETHOD_GUIDE(NAME)(std::declval<A>()...));      \
    };

#define BOOST_OPENMETHOD_DECLARE_OVERRIDER(NAME, ARGS, ...)                    \
    template<typename>                                                         \
    struct BOOST_OPENMETHOD_OVERRIDERS(NAME);                                  \
    template<>                                                                 \
    struct BOOST_OPENMETHOD_OVERRIDERS(NAME)<__VA_ARGS__ ARGS> {               \
        BOOST_OPENMETHOD_DETAIL_LOCATE_METHOD(NAME, ARGS);                     \
        static auto fn ARGS->__VA_ARGS__;                                      \
        static auto has_next() -> bool;                                        \
        template<typename... Args>                                             \
        static auto next(Args&&... args) -> decltype(auto);                    \
    };                                                                         \
    inline auto BOOST_OPENMETHOD_OVERRIDERS(                                   \
        NAME)<__VA_ARGS__ ARGS>::has_next() -> bool {                          \
        using method_type =                                                    \
            boost_openmethod_detail_locate_method_aux<void ARGS>::type;        \
        return method_type::next<fn> != method_type::fn.not_implemented;       \
    }                                                                          \
    template<typename... Args>                                                 \
    inline auto BOOST_OPENMETHOD_OVERRIDERS(NAME)<__VA_ARGS__ ARGS>::next(     \
        Args&&... args) -> decltype(auto) {                                    \
        BOOST_ASSERT(has_next());                                              \
        return boost_openmethod_detail_locate_method_aux<                      \
            void ARGS>::type::next<fn>(std::forward<Args>(args)...);           \
    }                                                                          \
    inline BOOST_OPENMETHOD_REGISTER(                                          \
        BOOST_OPENMETHOD_OVERRIDERS(NAME) < __VA_ARGS__ ARGS >                 \
        ::boost_openmethod_detail_locate_method_aux<void ARGS>::type::         \
            override<                                                          \
                BOOST_OPENMETHOD_OVERRIDERS(NAME) < __VA_ARGS__ ARGS>::fn >);

#define BOOST_OPENMETHOD_OVERRIDER(NAME, ARGS, ...)                            \
    BOOST_OPENMETHOD_OVERRIDERS(NAME)<__VA_ARGS__ ARGS>

#define BOOST_OPENMETHOD_DEFINE_OVERRIDER(NAME, ARGS, ...)                     \
    auto BOOST_OPENMETHOD_OVERRIDER(NAME, ARGS, __VA_ARGS__)::fn ARGS          \
        -> boost::mp11::mp_back<boost::mp11::mp_list<__VA_ARGS__>>

#define BOOST_OPENMETHOD_INLINE_OVERRIDE(NAME, ARGS, ...)                      \
    BOOST_OPENMETHOD_DECLARE_OVERRIDER(NAME, ARGS, __VA_ARGS__)                \
    inline BOOST_OPENMETHOD_DEFINE_OVERRIDER(NAME, ARGS, __VA_ARGS__)

#define BOOST_OPENMETHOD_OVERRIDE(NAME, ARGS, ...)                             \
    BOOST_OPENMETHOD_DECLARE_OVERRIDER(NAME, ARGS, __VA_ARGS__)                \
    BOOST_OPENMETHOD_DEFINE_OVERRIDER(NAME, ARGS, __VA_ARGS__)

#define BOOST_OPENMETHOD_CLASSES(...)                                          \
    BOOST_OPENMETHOD_REGISTER(::boost::openmethod::use_classes<__VA_ARGS__>);

#endif


#ifndef BOOST_OPENMETHOD_DISABLE_GLOBAL_VIRTUAL_PTR
using boost::openmethod::virtual_ptr;
#endif

#endif // BOOST_OPENMETHOD_HPP
// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_UNIQUE_PTR_HPP
#define BOOST_OPENMETHOD_UNIQUE_PTR_HPP




#include <memory>

namespace boost::openmethod {

template<class Class, class Policy>
struct virtual_traits<std::unique_ptr<Class>, Policy> {
    using virtual_type = std::remove_cv_t<Class>;

    static auto peek(const std::unique_ptr<Class>& arg) -> const Class& {
        return *arg;
    }

    template<class Other>
    using rebind = std::unique_ptr<Other>;

    template<typename Other>
    static auto cast(std::unique_ptr<Class>&& ptr) {
        if constexpr (detail::requires_dynamic_cast<Class&, Other&>) {
            return Other(
                &dynamic_cast<typename Other::element_type&>(*ptr.release()));
        } else {
            return Other(
                &static_cast<typename Other::element_type&>(*ptr.release()));
        }
    }
};

template<class Class, class Policy = BOOST_OPENMETHOD_DEFAULT_POLICY>
using unique_virtual_ptr = virtual_ptr<std::unique_ptr<Class>, Policy>;

template<
    class Class, class Policy = BOOST_OPENMETHOD_DEFAULT_POLICY, typename... T>
inline auto make_unique_virtual(T&&... args)
    -> unique_virtual_ptr<Class, Policy> {
    return unique_virtual_ptr<Class, Policy>::final(
        std::make_unique<Class>(std::forward<T>(args)...));
}

} // namespace boost::openmethod

#endif
// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_SHARED_PTR_HPP
#define BOOST_OPENMETHOD_SHARED_PTR_HPP



#include <memory>

namespace boost::openmethod {
namespace detail {

template<typename Class>
struct shared_ptr_traits {
    static const bool is_shared_ptr = false;
};

template<typename Class>
struct shared_ptr_traits<std::shared_ptr<Class>> {
    static const bool is_shared_ptr = true;
    static const bool is_const_ref = false;
    using virtual_type = Class;
};

template<typename Class>
struct shared_ptr_traits<const std::shared_ptr<Class>&> {
    static const bool is_shared_ptr = true;
    static const bool is_const_ref = true;
    using virtual_type = Class;
};

} // namespace detail

template<class Class, class Policy>
struct virtual_traits<const std::shared_ptr<Class>&, Policy> {
    using virtual_type = std::remove_cv_t<Class>;

    static auto peek(const std::shared_ptr<Class>& arg) -> const Class& {
        return *arg;
    }

    template<class Other>
    using rebind = std::shared_ptr<Other>;

    template<class Other>
    static void check_cast() {
        using namespace boost::openmethod::detail;

        static_assert(shared_ptr_traits<Other>::is_shared_ptr);
        static_assert(
            shared_ptr_traits<Other>::is_const_ref,
            "cannot cast from 'const shared_ptr<base>&' to "
            "'shared_ptr<derived>'");
        static_assert(
            std::is_class_v<typename shared_ptr_traits<Other>::virtual_type>);
    }

    template<class Other>
    static auto cast(const std::shared_ptr<Class>& obj) {
        using namespace boost::openmethod::detail;

        check_cast<Other>();

        if constexpr (detail::requires_dynamic_cast<Class*, Other>) {
            return std::dynamic_pointer_cast<
                typename shared_ptr_traits<Other>::virtual_type>(obj);
        } else {
            return std::static_pointer_cast<
                typename shared_ptr_traits<Other>::virtual_type>(obj);
        }
    }
};

template<typename Class, class Policy>
struct virtual_traits<std::shared_ptr<Class>, Policy> {
    using virtual_type = std::remove_cv_t<Class>;

    static auto peek(const std::shared_ptr<Class>& arg) -> const Class& {
        return *arg;
    }

    template<class Other>
    using rebind = std::shared_ptr<Other>;

    template<class Other>
    static void check_cast() {
        using namespace boost::openmethod::detail;

        static_assert(shared_ptr_traits<Other>::is_shared_ptr);
        static_assert(
            !shared_ptr_traits<Other>::is_const_ref,
            "cannot cast from 'const shared_ptr<base>&' to "
            "'shared_ptr<derived>'");
        static_assert(
            std::is_class_v<typename shared_ptr_traits<Other>::virtual_type>);
    }
    template<class Other>
    static auto cast(const std::shared_ptr<Class>& obj) {
        using namespace boost::openmethod::detail;

        check_cast<Other>();

        if constexpr (detail::requires_dynamic_cast<
                          Class*, typename Other::element_type*>) {
            return std::dynamic_pointer_cast<
                typename shared_ptr_traits<Other>::virtual_type>(obj);
        } else {
            return std::static_pointer_cast<
                typename shared_ptr_traits<Other>::virtual_type>(obj);
        }
    }
};

template<class Class, class Policy = BOOST_OPENMETHOD_DEFAULT_POLICY>
using shared_virtual_ptr = virtual_ptr<std::shared_ptr<Class>, Policy>;

template<
    class Class, class Policy = BOOST_OPENMETHOD_DEFAULT_POLICY, typename... T>
inline auto make_shared_virtual(T&&... args)
    -> shared_virtual_ptr<Class, Policy> {
    return shared_virtual_ptr<Class, Policy>::final(
        std::make_shared<Class>(std::forward<T>(args)...));
}

} // namespace boost::openmethod

#endif
// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_COMPILER_HPP
#define BOOST_OPENMETHOD_COMPILER_HPP






// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_DETAIL_TRACE_HPP
#define BOOST_OPENMETHOD_DETAIL_TRACE_HPP




#include <boost/dynamic_bitset.hpp>

namespace boost::openmethod {

namespace detail {

template<typename Iterator>
struct range;

struct rflush {
    std::size_t width;
    std::size_t value;
    explicit rflush(std::size_t width, std::size_t value)
        : width(width), value(value) {
    }
};

struct type_name {
    type_name(type_id type) : type(type) {
    }
    type_id type;
};

template<class Policy>
struct trace_type {
    static constexpr bool trace_enabled =
        Policy::template has_facet<policies::trace_output>;

    std::size_t indentation_level{0};

    auto operator++() -> trace_type& {
        if constexpr (trace_enabled) {
            if (Policy::trace_enabled) {
                for (std::size_t i = 0; i < indentation_level; ++i) {
                    Policy::trace_stream << "  ";
                }
            }
        }

        return *this;
    }

    struct indent {
        trace_type& trace;
        int by;

        explicit indent(trace_type& trace, int by = 2) : trace(trace), by(by) {
            trace.indentation_level += by;
        }

        ~indent() {
            trace.indentation_level -= by;
        }
    };
};

template<class Policy, typename T, typename F>
auto write_range(trace_type<Policy>& trace, range<T> range, F fn) -> auto& {
    if constexpr (trace_type<Policy>::trace_enabled) {
        if (Policy::trace_enabled) {
            trace << "(";
            const char* sep = "";
            for (auto value : range) {
                trace << sep << fn(value);
                sep = ", ";
            }

            trace << ")";
        }
    }

    return trace;
}

template<class Policy, typename T>
auto operator<<(trace_type<Policy>& trace, const T& value) -> auto& {
    if constexpr (trace_type<Policy>::trace_enabled) {
        if (Policy::trace_enabled) {
            Policy::trace_stream << value;
        }
    }
    return trace;
}

template<class Policy>
auto operator<<(trace_type<Policy>& trace, const rflush& rf) -> auto& {
    if constexpr (trace_type<Policy>::trace_enabled) {
        if (Policy::trace_enabled) {
            std::size_t digits = 1;
            auto tmp = rf.value / 10;

            while (tmp) {
                ++digits;
                tmp /= 10;
            }

            while (digits < rf.width) {
                trace << " ";
                ++digits;
            }

            trace << rf.value;
        }
    }

    return trace;
}

template<class Policy>
auto operator<<(trace_type<Policy>& trace, const boost::dynamic_bitset<>& bits)
    -> auto& {
    if constexpr (trace_type<Policy>::trace_enabled) {
        if (Policy::trace_enabled) {
            if (Policy::trace_enabled) {
                auto i = bits.size();
                while (i != 0) {
                    --i;
                    Policy::trace_stream << bits[i];
                }
            }
        }
    }

    return trace;
}

template<class Policy>
auto operator<<(trace_type<Policy>& trace, const range<type_id*>& tips)
    -> auto& {
    return write_range(trace, tips, [](auto tip) { return type_name(tip); });
}

template<class Policy, typename T>
auto operator<<(trace_type<Policy>& trace, const range<T>& range) -> auto& {
    return write_range(trace, range, [](auto value) { return value; });
}

template<class Policy>
auto operator<<(trace_type<Policy>& trace, const type_name& manip) -> auto& {
    if constexpr (Policy::template has_facet<policies::trace_output>) {
        Policy::type_name(manip.type, trace);
    }

    return trace;
}

} // namespace detail
} // namespace boost::openmethod

#endif // BOOST_OPENMETHOD_DETAIL_HPP


#include <algorithm>
#include <cstdint>
#include <deque>
#include <map>
#include <memory>
#include <numeric>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <boost/assert.hpp>
#include <boost/dynamic_bitset.hpp>

namespace boost::openmethod {
namespace detail {

template<class Reports, class Facets, typename = void>
struct aggregate_reports;

template<class... Reports, class Facet, class... MoreFacets>
struct aggregate_reports<
    mp11::mp_list<Reports...>, mp11::mp_list<Facet, MoreFacets...>,
    std::void_t<typename Facet::report>> {
    using type = typename aggregate_reports<
        mp11::mp_list<Reports..., typename Facet::report>,
        mp11::mp_list<MoreFacets...>>::type;
};

template<class... Reports, class Facet, class... MoreFacets, typename Void>
struct aggregate_reports<
    mp11::mp_list<Reports...>, mp11::mp_list<Facet, MoreFacets...>, Void> {
    using type = typename aggregate_reports<
        mp11::mp_list<Reports...>, mp11::mp_list<MoreFacets...>>::type;
};

template<class... Reports, typename Void>
struct aggregate_reports<mp11::mp_list<Reports...>, mp11::mp_list<>, Void> {
    struct type : Reports... {};
};

inline void merge_into(boost::dynamic_bitset<>& a, boost::dynamic_bitset<>& b) {
    if (b.size() < a.size()) {
        b.resize(a.size());
    }

    for (std::size_t i = 0; i < a.size(); ++i) {
        if (a[i]) {
            b[i] = true;
        }
    }
}

inline void set_bit(boost::dynamic_bitset<>& mask, std::size_t bit) {
    if (bit >= mask.size()) {
        mask.resize(bit + 1);
    }

    mask[bit] = true;
}

struct generic_compiler {

    struct method;

    struct parameter {
        struct method* method;
        std::size_t param;
    };

    struct vtbl_entry {
        std::size_t method_index, vp_index, group_index;
    };

    struct class_ {
        bool is_abstract = false;
        std::vector<type_id> type_ids;
        std::vector<class_*> transitive_bases;
        std::vector<class_*> direct_bases;
        std::vector<class_*> direct_derived;
        std::unordered_set<class_*> transitive_derived;
        std::vector<parameter> used_by_vp;
        boost::dynamic_bitset<> used_slots;
        boost::dynamic_bitset<> reserved_slots;
        int first_slot = 0;
        std::size_t mark = 0;   // temporary mark to detect cycles
        std::size_t weight = 0; // number of proper direct or indirect bases
        std::vector<vtbl_entry> vtbl;
        vptr_type* static_vptr;

        auto is_base_of(class_* other) const -> bool {
            return std::find(
                       transitive_derived.begin(), transitive_derived.end(),
                       other) != transitive_derived.end();
        }

        auto vptr() const -> const vptr_type& {
            return *static_vptr;
        }

        auto type_id_begin() const {
            return type_ids.begin();
        }

        auto type_id_end() const {
            return type_ids.end();
        }
    };

    struct overrider {
        detail::overrider_info* info = nullptr;
        overrider* next = nullptr;
        std::vector<class_*> vp;
        class_* covariant_return_type = nullptr;
        std::uintptr_t pf;
        std::size_t method_index, spec_index;
    };

    using bitvec = boost::dynamic_bitset<>;

    struct group {
        std::vector<class_*> classes;
        bool has_concrete_classes{false};
    };

    using group_map = std::map<bitvec, group>;

    struct method_report {
        std::size_t cells = 0;
        std::size_t not_implemented = 0;
        std::size_t ambiguous = 0;
    };

    struct report : method_report {};

    static void accumulate(const method_report& partial, report& total);

    struct method {
        detail::method_info* info;
        std::vector<class_*> vp;
        class_* covariant_return_type = nullptr;
        std::vector<overrider> specs;
        std::vector<std::size_t> slots;
        std::vector<std::size_t> strides;
        std::vector<const overrider*> dispatch_table;
        // following two are dummies, when converting to a function pointer, we will
        // get the corresponding pointer from method_info
        overrider not_implemented;
        vptr_type gv_dispatch_table = nullptr;
        auto arity() const {
            return vp.size();
        }
        method_report report;
    };

    std::deque<class_> classes;
    std::vector<method> methods;
    std::size_t class_mark = 0;
    bool compilation_done = false;
};

template<class Policy>
auto operator<<(trace_type<Policy>& trace, const generic_compiler::class_& cls)
    -> trace_type<Policy>& {
    if constexpr (Policy::template has_facet<policies::trace_output>) {
        trace << type_name(cls.type_ids[0]);
    }

    return trace;
}

template<class Policy, template<typename...> class Container, typename... T>
auto operator<<(
    trace_type<Policy>& trace,
    Container<generic_compiler::class_*, T...>& classes)
    -> trace_type<Policy>& {
    if constexpr (Policy::template has_facet<policies::trace_output>) {
        trace << "(";
        const char* sep = "";
        for (auto cls : classes) {
            trace << sep << *cls;
            sep = ", ";
        }

        trace << ")";
    }

    return trace;
}

struct spec_name {
    spec_name(
        const detail::generic_compiler::method& method,
        const detail::generic_compiler::overrider* def)
        : method(method), def(def) {
    }
    const detail::generic_compiler::method& method;
    const detail::generic_compiler::overrider* def;
};

template<class Policy>
auto operator<<(trace_type<Policy>& trace, const spec_name& sn)
    -> trace_type<Policy>& {
    if (sn.def == &sn.method.not_implemented) {
        trace << "not implemented";
    } else {
        trace << type_name(sn.def->info->type);
    }

    return trace;
}

} // namespace detail

template<class Policy>
struct compiler : detail::generic_compiler {
    using policy_type = Policy;
    using type_index_type = decltype(Policy::type_index(0));

    typename detail::aggregate_reports<
        mp11::mp_list<report>, typename Policy::facets>::type report;

    std::unordered_map<type_index_type, class_*> class_map;

    compiler();

    auto compile();
    auto initialize();
    void install_global_tables();

    void resolve_static_type_ids();
    void augment_classes();
    void calculate_transitive_derived(class_& cls);
    void augment_methods();
    void assign_slots();
    void assign_tree_slots(class_& cls, std::size_t base_slot);
    void assign_lattice_slots(class_& cls);
    void build_dispatch_tables();
    void build_dispatch_table(
        method& m, std::size_t dim,
        std::vector<group_map>::const_iterator group, const bitvec& candidates,
        bool concrete);
    void write_global_data();
    void print(const method_report& report) const;
    static void select_dominant_overriders(
        std::vector<overrider*>& dominants, std::size_t& pick,
        std::size_t& remaining);
    static auto is_more_specific(const overrider* a, const overrider* b)
        -> bool;
    static auto is_base(const overrider* a, const overrider* b) -> bool;

    static auto static_type(type_id type) -> type_id {
        if constexpr (std::is_base_of_v<
                          policies::deferred_static_rtti, policies::rtti>) {
            return reinterpret_cast<type_id (*)()>(type)();
        } else {
            return type;
        }
    }

    mutable detail::trace_type<Policy> trace;
    static constexpr bool trace_enabled =
        Policy::template has_facet<policies::trace_output>;
    using indent = typename detail::trace_type<Policy>::indent;
};

compiler() -> compiler<default_policy>;

template<class Policy>
void compiler<Policy>::install_global_tables() {
    if (!compilation_done) {
        abort();
    }

    write_global_data();

    print(report);
    ++trace << "Finished\n";
}

template<class Policy>
auto compiler<Policy>::compile() {
    resolve_static_type_ids();
    augment_classes();
    augment_methods();
    assign_slots();
    build_dispatch_tables();

    compilation_done = true;

    return report;
}

template<class Policy>
auto compiler<Policy>::initialize() {
    compile();
    install_global_tables();

    return *this;
}

template<class Policy>
compiler<Policy>::compiler() {
}

template<class Policy>
void compiler<Policy>::resolve_static_type_ids() {
    using namespace detail;

    auto resolve = [](type_id* p) {
        auto pf = reinterpret_cast<type_id (*)()>(*p);
        *p = pf();
    };

    if constexpr (std::is_base_of_v<policies::deferred_static_rtti, Policy>) {
        for (auto& ci : Policy::classes) {
            resolve(&ci.type);

            if (*ci.last_base == 0) {
                for (auto& ti : range{ci.first_base, ci.last_base}) {
                    resolve(&ti);
                }

                *ci.last_base = 1;
            }
        }

        for (auto& method : Policy::methods) {
            for (auto& ti : range{method.vp_begin, method.vp_end}) {
                if (*method.vp_end == 0) {
                    resolve(&ti);
                    *method.vp_end = 1;
                }

                for (auto& overrider : method.specs) {
                    if (*overrider.vp_end == 0) {
                        for (auto& ti :
                             range{overrider.vp_begin, overrider.vp_end}) {
                            resolve(&ti);
                        }

                        *overrider.vp_end = 1;
                    }
                }
            }
        }
    }
}

template<class Policy>
void compiler<Policy>::augment_classes() {
    using namespace detail;

    // scope
    {
        ++trace << "Static class info:\n";

        // The standard does not guarantee that there is exactly one
        // type_info object per class. However, it guarantees that the
        // type_index for a class has a unique value.
        for (auto& cr : Policy::classes) {
            {
                indent _(trace);
                ++trace << type_name(cr.type) << ": "
                        << range{cr.first_base, cr.last_base} << "\n";
            }

            auto& rtc = class_map[Policy::type_index(cr.type)];

            if (rtc == nullptr) {
                rtc = &classes.emplace_back();
                rtc->is_abstract = cr.is_abstract;
                rtc->static_vptr = cr.static_vptr;
            }

            // In the unlikely case that a class does have more than one
            // associated  ti*, collect them in a vector. We don't use an
            // unordered_set because, again, this situation is highly
            // unlikely, and, were it to occur, the number of distinct ti*s
            // would probably be small.
            if (std::find(
                    rtc->type_ids.begin(), rtc->type_ids.end(), cr.type) ==
                rtc->type_ids.end()) {
                rtc->type_ids.push_back(cr.type);
            }
        }
    }

    // All known classes now have exactly one associated class_* in the
    // map. Collect the bases.

    for (auto& cr : Policy::classes) {
        auto& rtc = class_map[Policy::type_index(cr.type)];

        for (auto base_iter = cr.first_base; base_iter != cr.last_base;
             ++base_iter) {
            auto rtb = class_map[Policy::type_index(*base_iter)];

            if (!rtb) {
                unknown_class_error error;
                error.type = *base_iter;

                if constexpr (Policy::template has_facet<
                                  policies::error_handler>) {
                    Policy::error(error);
                }

                abort();
            }

            if (rtc != rtb) {
                // At compile time we collected the class as its own
                // improper base, as per std::is_base_of. Eliminate that.
                rtc->transitive_bases.push_back(rtb);
            }
        }
    }

    // At this point bases may contain duplicates, and also indirect
    // bases. Clean that up.

    std::size_t mark = ++class_mark;

    for (auto& rtc : classes) {
        decltype(rtc.transitive_bases) bases;
        mark = ++class_mark;

        for (auto rtb : rtc.transitive_bases) {
            if (rtb->mark != mark) {
                bases.push_back(rtb);
                rtb->mark = mark;
            }
        }

        // Record the "weight" of the class, i.e. the total number of direct
        // and indirect proper bases it has.
        rtc.weight = bases.size();
        rtc.transitive_bases.swap(bases);
    }

    for (auto& rtc : classes) {
        // Sort base classes by weight. This ensures that a base class is
        // never preceded by one if its own bases classes.
        std::sort(
            rtc.transitive_bases.begin(), rtc.transitive_bases.end(),
            [](auto a, auto b) { return a->weight > b->weight; });
        mark = ++class_mark;

        // Collect the direct base classes. The first base is certainly a
        // direct one. Remove *its* bases from the candidates, by marking
        // them. Continue with the next base that is not marked. It is the
        // next direct base. And so on...

        for (auto rtb : rtc.transitive_bases) {
            if (rtb->mark == mark) {
                continue;
            }

            rtc.direct_bases.push_back(rtb);

            for (auto rtbb : rtb->transitive_bases) {
                rtbb->mark = mark;
            }
        }
    }

    for (auto& rtc : classes) {
        for (auto rtb : rtc.direct_bases) {
            rtb->direct_derived.push_back(&rtc);
        }
    }

    for (auto& rtc : classes) {
        calculate_transitive_derived(rtc);
    }

    if constexpr (trace_enabled) {
        ++trace << "Inheritance lattice:\n";

        for (auto& rtc : classes) {
            indent _(trace);
            ++trace << rtc << "\n";

            {
                indent _(trace);
                ++trace << "bases:      " << rtc.direct_bases << "\n";
                ++trace << "derived:    " << rtc.direct_derived << "\n";
                ++trace << "covariant: " << rtc.transitive_derived << "\n";
            }
        }
    }
}

template<class Policy>
void compiler<Policy>::calculate_transitive_derived(class_& cls) {
    if (!cls.transitive_derived.empty()) {
        return;
    }

    cls.transitive_derived.insert(&cls);

    for (auto derived : cls.direct_derived) {
        if (derived->transitive_derived.empty()) {
            calculate_transitive_derived(*derived);
        }

        std::copy(
            derived->transitive_derived.begin(),
            derived->transitive_derived.end(),
            std::inserter(
                cls.transitive_derived, cls.transitive_derived.end()));
    }
}

template<class Policy>
void compiler<Policy>::augment_methods() {
    using namespace policies;
    using namespace detail;

    methods.resize(Policy::methods.size());

    ++trace << "Methods:\n";
    indent _(trace);

    auto meth_iter = methods.begin();

    for (auto& meth_info : Policy::methods) {
        ++trace << type_name(meth_info.method_type) << " "
                << range{meth_info.vp_begin, meth_info.vp_end} << "\n";

        indent _(trace);

        meth_iter->info = &meth_info;
        meth_iter->vp.reserve(meth_info.arity());
        meth_iter->slots.resize(meth_info.arity());
        std::size_t param_index = 0;

        for (auto ti : range{meth_info.vp_begin, meth_info.vp_end}) {
            auto class_ = class_map[Policy::type_index(ti)];
            if (!class_) {
                ++trace << "unkown class " << ti << "(" << type_name(ti)
                        << ") for parameter #" << (param_index + 1) << "\n";
                unknown_class_error error;
                error.type = ti;

                if constexpr (Policy::template has_facet<error_handler>) {
                    Policy::error(error);
                }

                abort();
            }

            meth_iter->vp.push_back(class_);
        }

        if (Policy::type_index(meth_info.return_type) !=
            Policy::type_index(Policy::template static_type<void>())) {
            auto covariant_return_iter =
                class_map.find(Policy::type_index(meth_info.return_type));

            if (covariant_return_iter != class_map.end()) {
                meth_iter->covariant_return_type =
                    covariant_return_iter->second;
            }
        }

        // initialize the function pointer in the synthetic not_implemented
        // overrider
        const auto method_index = meth_iter - methods.begin();
        auto spec_size = meth_info.specs.size();
        meth_iter->not_implemented.pf =
            reinterpret_cast<uintptr_t>(meth_iter->info->not_implemented);
        meth_iter->not_implemented.method_index = method_index;
        meth_iter->not_implemented.spec_index = spec_size;

        meth_iter->specs.resize(spec_size);
        auto spec_iter = meth_iter->specs.begin();

        for (auto& overrider_info : meth_info.specs) {
            spec_iter->method_index = meth_iter - methods.begin();
            spec_iter->spec_index = spec_iter - meth_iter->specs.begin();

            ++trace << type_name(overrider_info.type) << " ("
                    << overrider_info.pf << ")\n";
            spec_iter->info = &overrider_info;
            spec_iter->vp.reserve(meth_info.arity());
            std::size_t param_index = 0;

            for (auto type :
                 range{overrider_info.vp_begin, overrider_info.vp_end}) {
                indent _(trace);
                auto class_ = class_map[Policy::type_index(type)];

                if (!class_) {
                    ++trace << "unknown class error for *virtual* parameter #"
                            << (param_index + 1) << "\n";
                    unknown_class_error error;
                    error.type = type;

                    if constexpr (Policy::template has_facet<error_handler>) {
                        Policy::error(error);
                    }

                    abort();
                }
                spec_iter->pf =
                    reinterpret_cast<uintptr_t>(spec_iter->info->pf);
                spec_iter->vp.push_back(class_);
            }

            if (meth_iter->covariant_return_type) {
                auto covariant_return_iter = class_map.find(
                    Policy::type_index(overrider_info.return_type));

                if (covariant_return_iter != class_map.end()) {
                    spec_iter->covariant_return_type =
                        covariant_return_iter->second;
                } else {
                    unknown_class_error error;
                    error.type = overrider_info.return_type;

                    if constexpr (Policy::template has_facet<error_handler>) {
                        Policy::error(error);
                    }

                    abort();
                }
            }

            ++spec_iter;
        }

        ++meth_iter;
    }

    for (auto& method : methods) {
        std::size_t param_index = 0;

        for (auto vp : method.vp) {
            vp->used_by_vp.push_back({&method, param_index++});
        }
    }
}

template<class Policy>
void compiler<Policy>::assign_slots() {
    ++trace << "Allocating slots...\n";

    {
        indent _(trace);

        ++class_mark;

        for (auto& cls : classes) {
            if (cls.direct_bases.size() == 0) {
                if (std::find_if(
                        cls.transitive_derived.begin(),
                        cls.transitive_derived.end(), [](auto cls) {
                            return cls->direct_bases.size() > 1;
                        }) == cls.transitive_derived.end()) {
                    indent _(trace);
                    assign_tree_slots(cls, 0);
                } else {
                    assign_lattice_slots(cls);
                }
            }
        }
    }

    ++trace << "Allocating MI v-tables...\n";

    {
        indent _(trace);

        for (auto& cls : classes) {
            if (cls.used_slots.empty()) {
                // not involved in multiple inheritance
                continue;
            }

            auto first_slot = cls.used_slots.find_first();
            cls.first_slot =
                first_slot == boost::dynamic_bitset<>::npos ? 0u : first_slot;
            cls.vtbl.resize(cls.used_slots.size() - cls.first_slot);
            ++trace << cls << " vtbl: " << cls.first_slot << "-"
                    << cls.used_slots.size() << " slots " << cls.used_slots
                    << "\n";
        }
    }
}

template<class Policy>
void compiler<Policy>::assign_tree_slots(class_& cls, std::size_t base_slot) {
    auto next_slot = base_slot;
    using namespace detail;

    for (const auto& mp : cls.used_by_vp) {
        ++trace << " in " << cls << " for "
                << type_name(mp.method->info->method_type) << " parameter "
                << mp.param << ": " << next_slot << "\n";
        mp.method->slots[mp.param] = next_slot++;
    }

    cls.first_slot = 0;
    cls.vtbl.resize(next_slot);

    for (auto pd : cls.direct_derived) {
        assign_tree_slots(*pd, next_slot);
    }
}

template<class Policy>
void compiler<Policy>::assign_lattice_slots(class_& cls) {
    using namespace detail;

    if (cls.mark == class_mark) {
        return;
    }

    cls.mark = class_mark;

    if (!cls.used_by_vp.empty()) {
        for (const auto& mp : cls.used_by_vp) {
            ++trace << " in " << cls << " for "
                    << type_name(mp.method->info->method_type) << " parameter "
                    << mp.param << "\n";

            indent _(trace);

            ++trace << "reserved slots: " << cls.reserved_slots
                    << " used slots: " << cls.used_slots << "\n";

            auto unavailable_slots = cls.used_slots;
            detail::merge_into(cls.reserved_slots, unavailable_slots);

            ++trace << "unavailable slots: " << unavailable_slots << "\n";

            std::size_t slot = 0;

            for (; slot < unavailable_slots.size(); ++slot) {
                if (!unavailable_slots[slot]) {
                    break;
                }
            }

            ++trace << "first available slot: " << slot << "\n";

            mp.method->slots[mp.param] = slot;
            detail::set_bit(cls.used_slots, slot);
            detail::set_bit(cls.reserved_slots, slot);

            {
                ++trace << "reserve slots " << cls.used_slots << " in:\n";
                indent _(trace);

                for (auto base : cls.transitive_bases) {
                    ++trace << *base << "\n";
                    detail::merge_into(cls.used_slots, base->reserved_slots);
                }
            }

            {
                ++trace << "assign slots " << cls.used_slots << " in:\n";
                indent _(trace);

                for (auto covariant : cls.transitive_derived) {
                    if (&cls != covariant) {
                        ++trace << *covariant << "\n";
                        detail::merge_into(
                            cls.used_slots, covariant->used_slots);

                        for (auto base : covariant->transitive_bases) {
                            ++trace << *base << "\n";
                            detail::merge_into(
                                cls.used_slots, base->reserved_slots);
                        }
                    }
                }
            }
        }
    }

    for (auto pd : cls.direct_derived) {
        assign_lattice_slots(*pd);
    }
}

template<class Policy>
void compiler<Policy>::build_dispatch_tables() {
    using namespace detail;

    for (auto& m : methods) {
        ++trace << "Building dispatch table for "
                << type_name(m.info->method_type) << "\n";
        indent _(trace);

        auto dims = m.arity();

        std::vector<group_map> groups;
        groups.resize(dims);

        {
            std::size_t dim = 0;

            for (auto vp : m.vp) {
                auto& dim_group = groups[dim];
                ++trace << "make groups for param #" << dim << ", class " << *vp
                        << "\n";
                indent _(trace);

                for (auto covariant_class : vp->transitive_derived) {
                    ++trace << "specs applicable to " << *covariant_class
                            << "\n";
                    bitvec mask;
                    mask.resize(m.specs.size());

                    std::size_t group_index = 0;
                    indent _(trace);

                    for (auto& spec : m.specs) {
                        if (spec.vp[dim]->transitive_derived.find(
                                covariant_class) !=
                            spec.vp[dim]->transitive_derived.end()) {
                            ++trace << type_name(spec.info->type) << "\n";
                            mask[group_index] = 1;
                        }
                        ++group_index;
                    }

                    auto& group = dim_group[mask];
                    group.classes.push_back(covariant_class);
                    group.has_concrete_classes = group.has_concrete_classes ||
                        !covariant_class->is_abstract;

                    ++trace << "-> mask: " << mask << "\n";
                }

                ++dim;
            }
        }

        {
            std::size_t stride = 1;
            m.strides.reserve(dims - 1);

            for (std::size_t dim = 1; dim < m.arity(); ++dim) {
                stride *= groups[dim - 1].size();
                ++trace << "    stride for dim " << dim << " = " << stride
                        << "\n";
                m.strides.push_back(stride);
            }
        }

        for (std::size_t dim = 0; dim < m.arity(); ++dim) {
            indent _(trace);
            std::size_t group_num = 0;

            for (auto& [mask, group] : groups[dim]) {
                ++trace << "groups for dim " << dim << ":\n";
                indent _(trace);
                ++trace << group_num << " mask " << mask << ":\n";

                for (auto cls : group.classes) {
                    indent _(trace);
                    ++trace << type_name(cls->type_ids[0]) << "\n";
                    auto& entry = cls->vtbl[m.slots[dim] - cls->first_slot];
                    entry.method_index = &m - &methods[0];
                    entry.vp_index = dim;
                    entry.group_index = group_num;
                }

                ++group_num;
            }
        }

        {
            ++trace << "building dispatch table\n";
            bitvec all(m.specs.size());
            all = ~all;
            build_dispatch_table(m, dims - 1, groups.end() - 1, all, true);

            if (m.arity() > 1) {
                indent _(trace);
                m.report.cells = 1;
                ++trace << "dispatch table rank: ";
                const char* prefix = "";

                for (const auto& dim_groups : groups) {
                    m.report.cells *= dim_groups.size();
                    trace << prefix << dim_groups.size();
                    prefix = " x ";
                }

                prefix = ", concrete only: ";

                for (const auto& dim_groups : groups) {
                    auto cells = std::count_if(
                        dim_groups.begin(), dim_groups.end(),
                        [](const auto& group) {
                            return group.second.has_concrete_classes;
                        });
                    trace << prefix << cells;
                    prefix = " x ";
                }

                trace << "\n";
            }

            print(m.report);
            accumulate(m.report, report);
        }
    }
}

template<class Policy>
void compiler<Policy>::build_dispatch_table(
    method& m, std::size_t dim,
    std::vector<group_map>::const_iterator group_iter, const bitvec& candidates,
    bool concrete) {
    using namespace detail;

    indent _(trace);
    std::size_t group_index = 0;

    for (const auto& [group_mask, group] : *group_iter) {
        auto mask = candidates & group_mask;

        if constexpr (trace_enabled) {
            ++trace << "group " << dim << "/" << group_index << " mask " << mask
                    << "\n";
            indent _(trace);
            for (auto cls : range{group.classes.begin(), group.classes.end()}) {
                ++trace << type_name(cls->type_ids[0]) << "\n";
            }
        }

        if (dim == 0) {
            std::vector<overrider*> candidates;
            std::size_t i = 0;

            for (auto& spec : m.specs) {
                if (mask[i]) {
                    candidates.push_back(&spec);
                }
                ++i;
            }

            if constexpr (trace_enabled) {
                ++trace << "select best of:\n";
                indent _(trace);

                for (auto& app : candidates) {
                    ++trace << "#" << app->spec_index << " "
                            << type_name(app->info->type) << "\n";
                }
            }

            std::vector<overrider*> dominants = candidates;
            std::size_t pick, remaining;

            select_dominant_overriders(dominants, pick, remaining);

            if (remaining == 0) {
                indent _(trace);
                ++trace << "not implemented\n";
                m.dispatch_table.push_back(&m.not_implemented);
                ++m.report.not_implemented;
            } else {
                auto overrider = dominants[pick];
                m.dispatch_table.push_back(overrider);
                ++trace;

                trace << "-> #" << overrider->spec_index << " "
                      << type_name(overrider->info->type)
                      << " pf = " << overrider->info->pf;

                if (remaining > 1) {
                    trace << " (ambiguous)";
                    ++m.report.ambiguous;
                }

                trace << "\n";

                // -------------------------------------------------------------
                // next

                // First remove the dominant overriders from the candidates.
                // Note that the dominants appear in the candidates in the same
                // relative order.
                auto candidate = candidates.begin();
                remaining = 0;

                for (auto dominant : dominants) {
                    if (*candidate == dominant) {
                        *candidate = nullptr;
                    } else {
                        ++remaining;
                    }

                    ++candidate;
                }

                if (remaining == 0) {
                    ++trace << "no 'next'\n";
                    overrider->next = &m.not_implemented;
                } else {
                    if constexpr (trace_enabled) {
                        ++trace << "for 'next', select best of:\n";
                        indent _(trace);

                        for (auto& app : candidates) {
                            if (app) {
                                ++trace << "#" << app->spec_index << " "
                                        << type_name(app->info->type) << "\n";
                            }
                        }
                    }

                    select_dominant_overriders(candidates, pick, remaining);

                    auto next_overrider = candidates[pick];
                    overrider->next = next_overrider;

                    ++trace << "-> #" << next_overrider->spec_index << " "
                            << type_name(next_overrider->info->type)
                            << " pf = " << next_overrider->info->pf;

                    if (remaining > 1) {
                        trace << " (ambiguous)";
                        // do not increment m.report.ambiguous, for same reason
                    }

                    trace << "\n";
                }
            }
        } else {
            build_dispatch_table(
                m, dim - 1, group_iter - 1, mask,
                concrete && group.has_concrete_classes);
        }

        ++group_index;
    }
}

inline void detail::generic_compiler::accumulate(
    const method_report& partial, report& total) {
    total.cells += partial.cells;
    total.not_implemented += partial.not_implemented != 0;
    total.ambiguous += partial.ambiguous != 0;
}

template<class Policy>
void compiler<Policy>::write_global_data() {
    using namespace policies;
    using namespace detail;

    auto dispatch_data_size = std::accumulate(
        methods.begin(), methods.end(), std::size_t(0),
        [](auto sum, auto& m) { return sum + m.dispatch_table.size(); });
    dispatch_data_size = std::accumulate(
        classes.begin(), classes.end(), dispatch_data_size,
        [](auto sum, auto& cls) { return sum + cls.vtbl.size(); });

    Policy::dispatch_data.resize(dispatch_data_size);
    auto gv_first = Policy::dispatch_data.data();
    [[maybe_unused]] auto gv_last = gv_first + Policy::dispatch_data.size();
    auto gv_iter = gv_first;

    ++trace << "Initializing multi-method dispatch tables at " << gv_iter
            << "\n";

    for (auto& m : methods) {
        if (m.info->arity() == 1) {
            // Uni-methods just need an index in the method table.
            m.info->slots_strides_ptr[0] = m.slots[0];
        } else {
            auto strides_iter = std::copy(
                m.slots.begin(), m.slots.end(), m.info->slots_strides_ptr);
            std::copy(m.strides.begin(), m.strides.end(), strides_iter);

            if constexpr (trace_enabled) {
                ++trace << rflush(4, Policy::dispatch_data.size()) << " "
                        << " method #" << m.dispatch_table[0]->method_index
                        << " " << type_name(m.info->method_type) << "\n";
                indent _(trace);

                for (auto& entry : m.dispatch_table) {
                    ++trace << "spec #" << entry->spec_index << " "
                            << spec_name(m, entry) << "\n";
                }
            }

            m.gv_dispatch_table = gv_iter;
            BOOST_ASSERT(gv_iter + m.dispatch_table.size() <= gv_last);
            gv_iter = std::transform(
                m.dispatch_table.begin(), m.dispatch_table.end(), gv_iter,
                [](auto spec) { return spec->pf; });
        }
    }

    ++trace << "Setting 'next' pointers\n";

    for (auto& m : methods) {
        indent _(trace);
        ++trace << "method #"
                << " " << type_name(m.info->method_type) << "\n";

        for (auto& overrider : m.specs) {
            if (overrider.next) {
                ++trace << "#" << overrider.spec_index << " "
                        << spec_name(m, &overrider) << " -> ";

                trace << "#" << overrider.next->spec_index << " "
                      << spec_name(m, overrider.next);
                *overrider.info->next = (void*)overrider.next->pf;
            } else {
                trace << "none";
            }

            trace << "\n";
        }
    }

    ++trace << "Initializing v-tables at " << gv_iter << "\n";

    for (auto& cls : classes) {
        if (cls.first_slot == -1) {
            // corner case: no methods for this class
            *cls.static_vptr = gv_iter;
            continue;
        }

        *cls.static_vptr = gv_iter - cls.first_slot;

        ++trace << rflush(4, gv_iter - gv_first) << " " << gv_iter
                << " vtbl for " << cls << " slots " << cls.first_slot << "-"
                << (cls.first_slot + cls.vtbl.size() - 1) << "\n";
        indent _(trace);

        for (auto& entry : cls.vtbl) {
            ++trace << "method #" << entry.method_index << " ";
            auto& method = methods[entry.method_index];

            if (method.arity() == 1) {
                auto spec = method.dispatch_table[entry.group_index];
                trace << "spec #" << spec->spec_index << "\n";
                indent _(trace);
                ++trace << type_name(method.info->method_type) << "\n";
                ++trace << spec_name(method, spec);
                BOOST_ASSERT(gv_iter + 1 <= gv_last);
                *gv_iter++ = spec->pf;
            } else {
                trace << "vp #" << entry.vp_index << " group #"
                      << entry.group_index << "\n";
                indent _(trace);
                ++trace << type_name(method.info->method_type);
                BOOST_ASSERT(gv_iter + 1 <= gv_last);

                if (entry.vp_index == 0) {
                    *gv_iter++ = std::uintptr_t(
                        method.gv_dispatch_table + entry.group_index);
                } else {
                    *gv_iter++ = entry.group_index;
                }
            }

            trace << "\n";
        }
    }

    ++trace << rflush(4, Policy::dispatch_data.size()) << " " << gv_iter
            << " end\n";

    if constexpr (Policy::template has_facet<extern_vptr>) {
        Policy::register_vptrs(classes.begin(), classes.end());
    }
}

template<class Policy>
void compiler<Policy>::select_dominant_overriders(
    std::vector<overrider*>& candidates, std::size_t& pick,
    std::size_t& remaining) {

    pick = 0;
    remaining = 0;

    for (size_t i = 0; i < candidates.size(); ++i) {
        if (candidates[i]) {
            for (size_t j = i + 1; j < candidates.size(); ++j) {
                if (candidates[j]) {
                    if (is_more_specific(candidates[i], candidates[j])) {
                        candidates[j] = nullptr;
                    } else if (is_more_specific(candidates[j], candidates[i])) {
                        candidates[i] = nullptr;
                        break; // this one is dead
                    }
                }
            }
        }

        if (candidates[i]) {
            pick = i;
            ++remaining;
        }
    }

    if (remaining <= 1 || !candidates[pick]->covariant_return_type) {
        return;
    }

    remaining = 0;

    for (size_t i = 0; i < candidates.size(); ++i) {
        if (candidates[i]) {
            for (size_t j = i + 1; j < candidates.size(); ++j) {
                if (candidates[j]) {
                    BOOST_ASSERT(candidates[i] != candidates[j]);

                    if (candidates[i]->covariant_return_type->is_base_of(
                            candidates[j]->covariant_return_type)) {
                        candidates[i] = nullptr;
                    } else if (candidates[j]->covariant_return_type->is_base_of(
                                   candidates[i]->covariant_return_type)) {
                        candidates[j] = nullptr;
                    }
                }
            }
        }

        if (candidates[i]) {
            pick = i;
            ++remaining;
        }
    }
}

template<class Policy>
auto compiler<Policy>::is_more_specific(const overrider* a, const overrider* b)
    -> bool {
    bool result = false;

    auto a_iter = a->vp.begin(), a_last = a->vp.end(), b_iter = b->vp.begin();

    for (; a_iter != a_last; ++a_iter, ++b_iter) {
        if (*a_iter != *b_iter) {
            if ((*b_iter)->transitive_derived.find(*a_iter) !=
                (*b_iter)->transitive_derived.end()) {
                result = true;
            } else if (
                (*a_iter)->transitive_derived.find(*b_iter) !=
                (*a_iter)->transitive_derived.end()) {
                return false;
            }
        }
    }

    return result;
}

template<class Policy>
auto compiler<Policy>::is_base(const overrider* a, const overrider* b) -> bool {
    bool result = false;

    auto a_iter = a->vp.begin(), a_last = a->vp.end(), b_iter = b->vp.begin();

    for (; a_iter != a_last; ++a_iter, ++b_iter) {
        if (*a_iter != *b_iter) {
            if ((*a_iter)->transitive_derived.find(*b_iter) ==
                (*a_iter)->transitive_derived.end()) {
                return false;
            } else {
                result = true;
            }
        }
    }

    return result;
}

template<class Policy>
void compiler<Policy>::print(const method_report& report) const {
    ++trace;

    if (report.cells) {
        // only for multi-methods, uni-methods don't have dispatch tables
        ++trace << report.cells << " dispatch table cells, ";
    }

    trace << report.not_implemented << " not implemented, " << report.ambiguous
          << " ambiguous\n";
}

template<class Policy = BOOST_OPENMETHOD_DEFAULT_POLICY>
auto initialize() -> compiler<Policy> {
    compiler<Policy> compiler;
    compiler.initialize();

    return compiler;
}

template<class Policy = BOOST_OPENMETHOD_DEFAULT_POLICY>
auto finalize() -> void {
    mp11::mp_for_each<typename Policy::facets>(
        [](auto facet) { decltype(facet)::finalize(); });
    Policy::dispatch_data.clear();
}

} // namespace boost::openmethod

#endif
