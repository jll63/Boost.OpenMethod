
// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_POLICY_FAST_PERFECT_HASH_HPP
#define BOOST_OPENMETHOD_POLICY_FAST_PERFECT_HASH_HPP

#include <random>

#include <boost/openmethod/policies/core.hpp>

namespace boost {
namespace openmethod {
namespace policies {

template<class Policy>
struct fast_perfect_hash : virtual type_hash {
    struct report {
        std::size_t method_table_size, dispatch_table_size;
        std::size_t hash_search_attempts;
    };

    static type_id hash_mult;
    static std::size_t hash_shift;
    static std::size_t hash_length;
    static std::size_t hash_min;
    static std::size_t hash_max;
    static std::vector<type_id> control;

#ifdef _MSC_VER
    __forceinline
#endif
        static auto
        hash_type_id(type_id type) -> type_id {

        auto index = (hash_mult * type) >> hash_shift;

        if constexpr (Policy::template has_facet<runtime_checks>) {
            check(index, type);
        }

        return index;
    }

    template<typename ForwardIterator>
    static void hash_initialize(ForwardIterator first, ForwardIterator last) {
        if constexpr (Policy::template has_facet<runtime_checks>) {
            hash_initialize(first, last, control);
        } else {
            std::vector<type_id> buckets;
            hash_initialize(first, last, buckets);
        }
    }

    template<typename ForwardIterator>
    static void hash_initialize(
        ForwardIterator first, ForwardIterator last,
        std::vector<type_id>& buckets);

    static void check(std::size_t index, type_id type);
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

        if constexpr (trace_enabled) {
            if (Policy::trace_enabled) {
                Policy::trace_stream << "  trying with M = " << M << ", "
                                     << hash_size << " buckets\n";
            }
        }

        bool found = false;
        std::size_t attempts = 0;
        buckets.resize(hash_size);
        hash_length = 0;

        while (!found && attempts < 100000) {
            std::fill(buckets.begin(), buckets.end(), static_cast<type_id>(-1));
            ++attempts;
            ++total_attempts;
            found = true;
            hash_mult = uniform_dist(rnd) | 1;

            for (auto iter = first; iter != last; ++iter) {
                for (auto type_iter = iter->type_id_begin();
                     type_iter != iter->type_id_end(); ++type_iter) {
                    auto type = *type_iter;
                    auto index = (type * hash_mult) >> hash_shift;
                    hash_min = (std::min)(hash_min, index);
                    hash_max = (std::max)(hash_max, index);

                    if (buckets[index] != static_cast<type_id>(-1)) {
                        found = false;
                        break;
                    }

                    buckets[index] = type;
                }
            }
        }

        if (found) {
            hash_length = hash_max + 1;

            if constexpr (trace_enabled) {
                if (Policy::trace_enabled) {
                    Policy::trace_stream << "  found " << hash_mult << " after "
                                         << total_attempts
                                         << " attempts; min = " << hash_min
                                         << ", max = " << hash_max << "\n";
                }
            }

            return;
        }
    }

    hash_search_error error;
    error.attempts = total_attempts;
    error.buckets = 1 << M;

    if constexpr (Policy::template has_facet<error_handler>) {
        Policy::error(error);
    }

    abort();
}

template<class Policy>
void fast_perfect_hash<Policy>::check(std::size_t index, type_id type) {
    if (index >= hash_length || control[index] != type) {
        if constexpr (Policy::template has_facet<error_handler>) {
            unknown_class_error error;
            error.type = type;
            Policy::error(error);
        }

        abort();
    }
}

template<class Policy>
type_id fast_perfect_hash<Policy>::hash_mult;
template<class Policy>
std::size_t fast_perfect_hash<Policy>::hash_shift;
template<class Policy>
std::size_t fast_perfect_hash<Policy>::hash_length;
template<class Policy>
std::size_t fast_perfect_hash<Policy>::hash_min;
template<class Policy>
std::size_t fast_perfect_hash<Policy>::hash_max;
template<class Policy>
std::vector<type_id> fast_perfect_hash<Policy>::control;

} // namespace policies
} // namespace openmethod
} // namespace boost

#endif
