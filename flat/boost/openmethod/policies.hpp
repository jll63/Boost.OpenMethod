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
