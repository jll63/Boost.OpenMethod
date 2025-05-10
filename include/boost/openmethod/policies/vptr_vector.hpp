// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_POLICY_VPTR_VECTOR_HPP
#define BOOST_OPENMETHOD_POLICY_VPTR_VECTOR_HPP

#include <boost/openmethod/policies/basic_policy.hpp>

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
