// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_POLICY_VPTR_MAP_HPP
#define BOOST_OPENMETHOD_POLICY_VPTR_MAP_HPP

#include <boost/openmethod/registry.hpp>

#include <unordered_map>

namespace boost::openmethod {

namespace detail {

template<class Registry, class MapAdaptor, typename Key, typename Value>
inline typename MapAdaptor::template fn<Key, Value> vptr_map_vptrs;

} // namespace detail

namespace policies {

template<class MapAdaptor = mp11::mp_quote<std::unordered_map>>
class vptr_map : public extern_vptr {
  public:
    template<class Registry>
    struct fn {
        template<typename ForwardIterator>
        static void
        register_vptrs(ForwardIterator first, ForwardIterator last) {
            for (auto iter = first; iter != last; ++iter) {
                for (auto type_iter = iter->type_id_begin();
                     type_iter != iter->type_id_end(); ++type_iter) {

                    if constexpr (Registry::template has_policy<
                                      indirect_vptr>) {
                        detail::vptr_map_vptrs<Registry,
                        MapAdaptor, type_id,
                        const vptr_type*>.emplace(*type_iter, &iter->vptr());
                    } else {
                        detail::vptr_map_vptrs<
                        Registry, MapAdaptor, type_id, vptr_type
                    >.emplace(*type_iter, iter->vptr());
                    }
                }
            }
        }

        template<class Class>
        static auto dynamic_vptr(const Class& arg) -> const vptr_type& {
            auto type = Registry::Rtti::dynamic_type(arg);
            bool constexpr use_indirect_vptrs =
                Registry::template has_policy<indirect_vptr>;
            const auto& map = detail::vptr_map_vptrs<
                Registry, MapAdaptor, type_id,
                std::conditional_t<
                    use_indirect_vptrs, const vptr_type*, vptr_type>>;
            auto iter = map.find(type);

            if constexpr (Registry::RuntimeChecks) {
                if (iter == map.end()) {
                    using ErrorHandler = typename Registry::ErrorHandler;
                    if constexpr (detail::is_not_void<ErrorHandler>) {
                        unknown_class_error error;
                        error.type = type;
                        ErrorHandler::error(error);
                    }

                    abort();
                }
            }

            if constexpr (use_indirect_vptrs) {
                return *iter->second;
            } else {
                return iter->second;
            }
        }

        static auto finalize() -> void {
            detail::vptr_map_vptrs<
            Registry, MapAdaptor, type_id, std::conditional_t<
                Registry::template has_policy<indirect_vptr>,
                const vptr_type*, vptr_type>>.clear();
        }
    };
};

} // namespace policies
} // namespace boost::openmethod

#endif
