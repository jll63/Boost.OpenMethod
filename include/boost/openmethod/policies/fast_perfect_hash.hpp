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

//! Hash a @ref type_id using a fast, perfect hash function
//!
//! `fast_perfect_hash` implements the @ref type_hash policy using a hash
//! function in the form `H(x)=(M*x)>>S`. It attempts to determine values for
//! `M` and `S` that do not result in collisions for the set of registered
//! type_ids. This may fail for certain sets of inputs, although it is very
//! likely to succeed for addresses of `std::type_info` objects.
//!
//! There is no guarantee that every value in the codomain of the function
//! corresponds to a value in the domain, or even that the codomain is a dense
//! range of integers. In other words, a lot of space may be wasted in presence
//! of large sets of type_ids.
struct fast_perfect_hash : type_hash {
    //! A model of @ref type_hash::fn.
    //!
    //! @tparam Registry The registry containing this policy
    template<class Registry>
    class fn {
        inline static std::size_t mult;
        inline static std::size_t shift;
        inline static std::size_t min_value;
        inline static std::size_t max_value;
        inline static void check(std::size_t index, type_id type);

        template<typename ForwardIterator>
        static void initialize(
            ForwardIterator first, ForwardIterator last,
            std::vector<type_id>& buckets);

      public:
        //! Find the hash factors
        //!
        //! Attempts to find suitable values for the multiplication factor `M`
        //! and the shift amount `S` to that do not result in collisions for the
        //! specified input values.
        //!
        //! If no suitable values are found, calls the error handler with
        //! a @ref hash_error object then calls `abort`.
        //!
        //! @tparam ForwardIterator A forward iterator yielding
        //! @ref IdsToVptr objects
        //! @param first Beginning of the range
        //! @param last End of the range
        template<typename ForwardIterator>
        static auto initialize(ForwardIterator first, ForwardIterator last) {
            if constexpr (Registry::has_runtime_checks) {
                initialize(
                    first, last, detail::fast_perfect_hash_control<Registry>);
            } else {
                std::vector<type_id> buckets;
                initialize(first, last, buckets);
            }

            return std::pair{min_value, max_value};
        }

        //! Hash a type id
        //!
        //! Hash a type id.
        //!
        //! If `Registry` contains the @ref runtime_checks policy, checks that
        //! the type id is valid, i.e. if it was present in the set passed to
        //! @ref initialize. Its absence indicates that a class involved in a
        //! method definition, method overrider, or method call was not
        //! registered. In this case, signal a @ref unknown_class_error using
        //! the registry's @ref error_handler if present; then calls `abort`.
        //!
        //! @param type The type_id to hash
        //! @return The hash value
        BOOST_FORCEINLINE
        static auto hash(type_id type) -> std::size_t {
            auto index =
                (mult * reinterpret_cast<detail::uintptr>(type)) >> shift;

            if constexpr (Registry::has_runtime_checks) {
                check(index, type);
            }

            return index;
        }

        //! Releases the memory allocated by `initialize`.
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

    const auto N = std::distance(first, last);

    if constexpr (Registry::has_trace && Registry::has_output) {
        if (Registry::trace::on) {
            Registry::output::os << "Finding hash factor for " << N
                                 << " types\n";
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
        shift = 8 * sizeof(type_id) - M;
        auto hash_size = 1 << M;
        min_value = (std::numeric_limits<std::size_t>::max)();
        max_value = (std::numeric_limits<std::size_t>::min)();

        if constexpr (Registry::has_trace && Registry::has_output) {
            if (Registry::trace::on) {
                Registry::output::os << "  trying with M = " << M << ", "
                                     << hash_size << " buckets\n";
            }
        }

        std::size_t attempts = 0;
        buckets.resize(hash_size);

        while (attempts < 100000) {
            std::fill(
                buckets.begin(), buckets.end(), type_id(detail::uintptr_max));
            ++attempts;
            ++total_attempts;
            mult = uniform_dist(rnd) | 1;

            for (auto iter = first; iter != last; ++iter) {
                for (auto type_iter = iter->type_id_begin();
                     type_iter != iter->type_id_end(); ++type_iter) {
                    auto type = *type_iter;
                    auto index = (detail::uintptr(type) * mult) >> shift;
                    min_value = (std::min)(min_value, index);
                    max_value = (std::max)(max_value, index);

                    if (detail::uintptr(buckets[index]) !=
                        detail::uintptr_max) {
                        goto collision;
                    }

                    buckets[index] = type;
                }
            }

            if constexpr (Registry::has_trace && Registry::has_output) {
                if (Registry::trace::on) {
                    Registry::output::os
                        << "  found " << mult << " after " << total_attempts
                        << " attempts; span = [" << min_value << ", "
                        << max_value << "]\n";
                }
            }

            return;

        collision: {}
        }
    }

    fast_perfect_hash_error error;
    error.attempts = total_attempts;
    error.buckets = std::size_t(1) << M;

    if constexpr (Registry::has_error_handler) {
        Registry::error_handler::error(error);
    }

    abort();
}

template<class Registry>
void fast_perfect_hash::fn<Registry>::check(std::size_t index, type_id type) {
    if (index < min_value || index > max_value ||
        detail::fast_perfect_hash_control<Registry>[index] != type) {

        if constexpr (Registry::has_error_handler) {
            unknown_class_error error;
            error.type = type;
            Registry::error_handler::error(error);
        }

        abort();
    }
}

} // namespace policies
} // namespace boost::openmethod

#endif
