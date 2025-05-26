// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_POLICY_FAST_PERFECT_HASH_HPP
#define BOOST_OPENMETHOD_POLICY_FAST_PERFECT_HASH_HPP

#include <boost/openmethod/registry.hpp>

#include <limits>
#include <random>

namespace boost::openmethod {

namespace detail {

template<class Registry>
std::vector<type_id> fast_perfect_hash_control;

}

namespace policies {

struct fast_perfect_hash : type_hash {
    template<class Registry>
    struct fn {

        inline static std::size_t hash_mult;
        inline static std::size_t hash_shift;
        inline static std::size_t hash_min;
        inline static std::size_t hash_max;
        inline static void check(std::size_t index, type_id type);

      public:
        struct report {
            std::size_t first, last;
        };

        BOOST_FORCEINLINE
        static auto hash(type_id type) -> std::size_t {
            auto index =
                (hash_mult * reinterpret_cast<detail::uintptr>(type)) >>
                hash_shift;

            if constexpr (Registry::template has_policy<runtime_checks>) {
                check(index, type);
            }

            return index;
        }

        template<typename ForwardIterator>
        static auto initialize(ForwardIterator first, ForwardIterator last) {
            if constexpr (Registry::template has_policy<runtime_checks>) {
                initialize(
                    first, last, detail::fast_perfect_hash_control<Registry>);
            } else {
                std::vector<type_id> buckets;
                initialize(first, last, buckets);
            }

            return report{hash_min, hash_max};
        }

        template<typename ForwardIterator>
        static void initialize(
            ForwardIterator first, ForwardIterator last,
            std::vector<type_id>& buckets);

        static auto finalize() -> void {
            detail::fast_perfect_hash_control<Registry>.clear();
        }
    };
};

template<class Registry>
template<typename ForwardIterator>
void fast_perfect_hash::fn<Registry>::initialize(
    ForwardIterator first, ForwardIterator last,
    std::vector<type_id>& buckets) {
    using namespace policies;

    constexpr bool trace_enabled = Registry::template has_policy<trace>;
    const auto N = std::distance(first, last);

    if constexpr (trace_enabled) {
        if (Registry::template policy<policies::trace>::trace_enabled) {
            Registry::template policy<policies::output>::os
                << "Finding hash factor for " << N << " types\n";
        }
    }

    std::default_random_engine rnd(13081963);
    std::size_t total_attempts = 0;
    std::size_t M = 1;

    for (auto size = N * 5 / 4; size >>= 1;) {
        ++M;
    }

    std::uniform_int_distribution<std::size_t> uniform_dist;

    for (std::size_t pass = 0; pass < 4; ++pass, ++M) {
        hash_shift = 8 * sizeof(type_id) - M;
        auto hash_size = 1 << M;
        hash_min = (std::numeric_limits<std::size_t>::max)();
        hash_max = (std::numeric_limits<std::size_t>::min)();

        if constexpr (trace_enabled) {
            if (Registry::template policy<policies::trace>::trace_enabled) {
                Registry::template policy<policies::output>::os
                    << "  trying with M = " << M << ", " << hash_size
                    << " buckets\n";
            }
        }

        std::size_t attempts = 0;
        buckets.resize(hash_size);

        while (attempts < 100000) {
            std::fill(
                buckets.begin(), buckets.end(), type_id(detail::uintptr_max));
            ++attempts;
            ++total_attempts;
            hash_mult = uniform_dist(rnd) | 1;

            for (auto iter = first; iter != last; ++iter) {
                for (auto type_iter = iter->type_id_begin();
                     type_iter != iter->type_id_end(); ++type_iter) {
                    auto type = *type_iter;
                    auto index =
                        (detail::uintptr(type) * hash_mult) >> hash_shift;
                    hash_min = (std::min)(hash_min, index);
                    hash_max = (std::max)(hash_max, index);

                    if (detail::uintptr(buckets[index]) !=
                        detail::uintptr_max) {
                        goto collision;
                    }

                    buckets[index] = type;
                }
            }

            if constexpr (trace_enabled) {
                if (Registry::template policy<policies::trace>::trace_enabled) {
                    Registry::template policy<policies::output>::os
                        << "  found " << hash_mult << " after "
                        << total_attempts << " attempts; span = [" << hash_min
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

    if constexpr (Registry::template has_policy<policies::error_handler>) {
        Registry::template policy<policies::error_handler>::error(error);
    }

    abort();
}

template<class Registry>
void fast_perfect_hash::fn<Registry>::check(std::size_t index, type_id type) {
    if (index < hash_min || index > hash_max ||
        detail::fast_perfect_hash_control<Registry>[index] != type) {
        if constexpr (Registry::template has_policy<policies::error_handler>) {
            unknown_class_error error;
            error.type = type;
            Registry::template policy<policies::error_handler>::error(error);
        }

        abort();
    }
}

} // namespace policies
} // namespace boost::openmethod

#endif
