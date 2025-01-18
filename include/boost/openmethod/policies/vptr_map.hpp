
// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_POLICY_VPTR_MAP_HPP
#define BOOST_OPENMETHOD_POLICY_VPTR_MAP_HPP

#include <boost/openmethod/policies/basic_policy.hpp>

#include <unordered_map>

namespace boost {
namespace openmethod {
namespace policies {

template<
    class Policy, class Base = extern_vptr,
    class Map = std::unordered_map<
        type_id,
        std::conditional_t<
            std::is_base_of_v<indirect_extern_vptr, Base>, const vptr_type*,
            vptr_type>>>
class vptr_map : virtual Base {
    static_assert(std::is_base_of_v<extern_vptr, Base>);
    static constexpr bool is_indirect =
        std::is_base_of_v<indirect_extern_vptr, Base>;
    static_assert(
        std::is_same_v<typename Map::mapped_type, vptr_type> ||
        std::is_same_v<typename Map::mapped_type, const vptr_type*>);
    static_assert(
        std::is_same_v<typename Map::mapped_type, const vptr_type*> ==
        is_indirect);

    static Map vptrs;

  public:
    template<typename ForwardIterator>
    static void register_vptrs(ForwardIterator first, ForwardIterator last) {
        for (auto iter = first; iter != last; ++iter) {
            for (auto type_iter = iter->type_id_begin();
                 type_iter != iter->type_id_end(); ++type_iter) {

                if constexpr (is_indirect) {
                    vptrs[*type_iter] = &iter->vptr();
                } else {
                    vptrs[*type_iter] = iter->vptr();
                }
            }
        }
    }

    template<class Class>
    static auto dynamic_vptr(const Class& arg) -> const vptr_type& {
        auto type = Policy::dynamic_type(arg);
        auto iter = vptrs.find(type);

        if constexpr (Policy::template has_facet<runtime_checks>) {
            if (iter == vptrs.end()) {
                if constexpr (Policy::template has_facet<error_handler>) {
                    unknown_class_error error;
                    error.type = type;
                    Policy::error(error);
                }

                abort();
            }
        }

        if constexpr (is_indirect) {
            return *iter->second;
        } else {
            return iter->second;
        }
    }
};

template<class Policy, class Base, class Map>
Map vptr_map<Policy, Base, Map>::vptrs;

} // namespace policies
} // namespace openmethod
} // namespace boost

#endif
