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

struct vptr_vector : vptr {
  public:
    template<class Registry>
    struct fn {
        template<typename ForwardIterator>
        static auto initialize(ForwardIterator first, ForwardIterator last)
            -> void {
            std::size_t size;

            if constexpr (Registry::has_type_hash) {
                auto [_, max_value] =
                    Registry::type_hash::initialize(first, last);
                size = max_value + 1;
            } else {
                size = 0;

                for (auto iter = first; iter != last; ++iter) {
                    for (auto type_iter = iter->type_id_begin();
                         type_iter != iter->type_id_end(); ++type_iter) {
                        size = (std::max)(size, std::size_t(*type_iter));
                    }
                }

                ++size;
            }

            if constexpr (Registry::has_indirect_vptr) {
                detail::vptr_vector_indirect_vptrs<Registry>.resize(size);
            } else {
                detail::vptr_vector_vptrs<Registry>.resize(size);
            }

            for (auto iter = first; iter != last; ++iter) {
                for (auto type_iter = iter->type_id_begin();
                     type_iter != iter->type_id_end(); ++type_iter) {
                    std::size_t index;

                    if constexpr (Registry::has_type_hash) {
                        index = Registry::type_hash::hash(*type_iter);
                    } else {
                        index = std::size_t(*type_iter);
                    }

                    if constexpr (Registry::has_indirect_vptr) {
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
            auto dynamic_type = Registry::rtti::dynamic_type(arg);
            std::size_t index;

            if constexpr (Registry::has_type_hash) {
                index = Registry::type_hash::hash(dynamic_type);
            } else {
                index = std::size_t(dynamic_type);

                if constexpr (Registry::has_runtime_checks) {
                    std::size_t max_index = 0;

                    if constexpr (Registry::has_indirect_vptr) {
                        max_index =
                            detail::vptr_vector_indirect_vptrs<Registry>.size();
                    } else {
                        max_index = detail::vptr_vector_vptrs<Registry>.size();
                    }

                    if (index >= max_index) {
                        using error_handler = typename Registry::error_handler;

                        if constexpr (Registry::has_error_handler) {
                            unknown_class_error error;
                            error.type = dynamic_type;
                            Registry::error_handler::error(error);
                        }

                        abort();
                    }
                }
            }

            if constexpr (Registry::has_indirect_vptr) {
                return *detail::vptr_vector_indirect_vptrs<Registry>[index];
            } else {
                return detail::vptr_vector_vptrs<Registry>[index];
            }
        }

        static auto finalize() -> void {
            using namespace policies;

            if constexpr (Registry::has_indirect_vptr) {
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
