
// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_POLICY_VPTR_VECTOR_HPP
#define BOOST_OPENMETHOD_POLICY_VPTR_VECTOR_HPP

#include <boost/openmethod/policies/core.hpp>

namespace boost {
namespace openmethod {
namespace policies {

template<class Policy, class Base = extern_vptr>
class vptr_vector : Base {
    static_assert(std::is_base_of_v<extern_vptr, Base>);
    static constexpr bool is_indirect =
        std::is_base_of_v<indirect_extern_vptr, Base>;
    using element_type =
        std::conditional_t<is_indirect, const vptr_type*, vptr_type>;
    static std::vector<element_type> vptrs;

  public:
    template<typename ForwardIterator>
    static auto register_vptrs(ForwardIterator first, ForwardIterator last)
        -> void {
        using namespace policies;

        std::size_t size;

        if constexpr (Policy::template has_facet<type_hash>) {
            auto report = Policy::hash_initialize(first, last);
            size = report.last;
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

        vptrs.resize(size);

        for (auto iter = first; iter != last; ++iter) {
            for (auto type_iter = iter->type_id_begin();
                 type_iter != iter->type_id_end(); ++type_iter) {
                auto index = *type_iter;

                if constexpr (Policy::template has_facet<type_hash>) {
                    index = Policy::hash_type_id(index);
                }

                if constexpr (is_indirect) {
                    vptrs[index] = &iter->vptr();
                } else {
                    vptrs[index] = iter->vptr();
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

        if constexpr (is_indirect) {
            return *vptrs[index];
        } else {
            return vptrs[index];
        }
    }
};

template<class Policy, class Base>
std::vector<typename vptr_vector<Policy, Base>::element_type>
    vptr_vector<Policy, Base>::vptrs;

} // namespace policies
} // namespace openmethod
} // namespace boost

#endif
