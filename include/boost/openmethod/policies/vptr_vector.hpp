// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_POLICY_VPTR_VECTOR_HPP
#define BOOST_OPENMETHOD_POLICY_VPTR_VECTOR_HPP

#include <boost/openmethod/registry.hpp>

#include <variant>
#include <vector>

namespace boost::openmethod {

namespace detail {

template<class Registry>
inline std::vector<vptr_type> vptr_vector_vptrs;

template<class Registry>
inline std::vector<const vptr_type*> vptr_vector_indirect_vptrs;

} // namespace detail

namespace policies {

struct vptr_vector : extern_vptr {
  public:
    template<class Registry>
    struct fn {
        template<typename ForwardIterator>
        static auto register_vptrs(ForwardIterator first, ForwardIterator last)
            -> void {
            std::size_t size;

            if constexpr (Registry::template has_policy<type_hash>) {
                auto report =
                    Registry::template policy<type_hash>::initialize(
                        first, last);
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

            if constexpr (Registry::template has_policy<indirect_vptr>) {
                detail::vptr_vector_indirect_vptrs<Registry>.resize(size);
            } else {
                detail::vptr_vector_vptrs<Registry>.resize(size);
            }

            for (auto iter = first; iter != last; ++iter) {
                for (auto type_iter = iter->type_id_begin();
                     type_iter != iter->type_id_end(); ++type_iter) {
                    auto index = *type_iter;

                    if constexpr (Registry::template has_policy<type_hash>) {
                        index =
                            Registry::template policy<type_hash>::hash(index);
                    }

                    if constexpr (Registry::template has_policy<
                                      indirect_vptr>) {
                        detail::vptr_vector_indirect_vptrs<Registry>[index] =
                            &iter->vptr();
                    } else {
                        detail::vptr_vector_vptrs<Registry>[index] =
                            iter->vptr();
                    }
                }
            }
        }

        template<class Class>
        static auto dynamic_vptr(const Class& arg) -> const vptr_type& {
            auto index = Registry::template policy<rtti>::dynamic_type(arg);

            if constexpr (Registry::template has_policy<type_hash>) {
                index = Registry::template policy<type_hash>::hash(index);
            }

            if constexpr (Registry::template has_policy<indirect_vptr>) {
                return *detail::vptr_vector_indirect_vptrs<Registry>[index];
            } else {
                return detail::vptr_vector_vptrs<Registry>[index];
            }
        }

        static auto finalize() -> void {
            if constexpr (Registry::template has_policy<indirect_vptr>) {
                detail::vptr_vector_indirect_vptrs<Registry>.clear();
            } else {
                detail::vptr_vector_vptrs<Registry>.clear();
            }
        };
    };
};

} // namespace policies
} // namespace boost::openmethod

#endif
