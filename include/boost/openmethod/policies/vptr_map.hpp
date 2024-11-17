
// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_POLICY_VPTR_MAP_HPP
#define BOOST_OPENMETHOD_POLICY_VPTR_MAP_HPP

#include <boost/openmethod/policies/core.hpp>

#include <unordered_map>

namespace boost {
namespace openmethod {
namespace policies {

template<class Policy, class Map = std::unordered_map<type_id, vptr_type>>
struct vptr_map : virtual extern_vptr {
    static_assert(
        std::is_same_v<typename Map::mapped_type, direct_vptr_type> ||
        std::is_same_v<typename Map::mapped_type, indirect_vptr_type>);

    static constexpr bool is_indirect =
        std::is_same_v<typename Map::mapped_type, indirect_vptr_type>;

    static Map vptrs;

    template<typename ForwardIterator>
    static void register_vptrs(ForwardIterator first, ForwardIterator last) {
        for (auto iter = first; iter != last; ++iter) {
            for (auto type_iter = iter->type_id_begin();
                 type_iter != iter->type_id_end(); ++type_iter) {
                vptrs[*type_iter] = iter->vptr();
            }
        }
    }

    template<class Class>
    static auto dynamic_vptr(const Class& arg) -> vptr_type {
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

        return iter->second;
    }
};

template<class Policy, class Map>
Map vptr_map<Policy, Map>::vptrs;

} // namespace policies
} // namespace openmethod
} // namespace boost

#endif
