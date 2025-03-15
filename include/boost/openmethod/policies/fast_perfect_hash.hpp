// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_POLICY_FAST_PERFECT_HASH_HPP
#define BOOST_OPENMETHOD_POLICY_FAST_PERFECT_HASH_HPP

#include <boost/openmethod/policies/basic_policy.hpp>

#include <random>

namespace boost {
namespace openmethod {
namespace policies {

template<class Policy>
class fast_perfect_hash : type_hash {

    static type_id hash_mult;
    static std::size_t hash_shift;
    static std::size_t hash_min;
    static std::size_t hash_max;
    static std::vector<type_id> control;

    static void check(std::size_t index, type_id type);

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
            hash_initialize(first, last, control);
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
            ++hash_max;

            if constexpr (trace_enabled) {
                if (Policy::trace_enabled) {
                    Policy::trace_stream << "  found " << hash_mult << " after "
                                         << total_attempts
                                         << " attempts; span = [" << hash_min
                                         << "," << hash_max << ")\n";
                }
            }

            return;
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
    if (index < hash_min || index >= hash_max || control[index] != type) {
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
std::size_t fast_perfect_hash<Policy>::hash_min;
template<class Policy>
std::size_t fast_perfect_hash<Policy>::hash_max;
template<class Policy>
std::vector<type_id> fast_perfect_hash<Policy>::control;

} // namespace policies
} // namespace openmethod
} // namespace boost

#endif
