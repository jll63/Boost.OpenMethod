// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_POLICY_VPTR_MAP_HPP
#define BOOST_OPENMETHOD_POLICY_VPTR_MAP_HPP

#include <boost/openmethod/registry.hpp>

#include <unordered_map>

namespace boost::openmethod {

namespace policies {

template<class MapAdaptor = mp11::mp_quote<std::unordered_map>>
class vptr_map : public extern_vptr {
  public:
    template<class Registry>
    struct fn {
        static constexpr bool IndirectVptr =
            Registry::template has_policy<indirect_vptr>;
        using Value =
            std::conditional_t<IndirectVptr, const vptr_type*, vptr_type>;
        static inline typename MapAdaptor::template fn<type_id, Value> vptrs;

        template<typename ForwardIterator>
        static void
        register_vptrs(ForwardIterator first, ForwardIterator last) {
            for (auto iter = first; iter != last; ++iter) {
                for (auto type_iter = iter->type_id_begin();
                     type_iter != iter->type_id_end(); ++type_iter) {

                    if constexpr (IndirectVptr) {
                        vptrs.emplace(*type_iter, &iter->vptr());
                    } else {
                        vptrs.emplace(*type_iter, iter->vptr());
                    }
                }
            }
        }

        template<class Class>
        static auto dynamic_vptr(const Class& arg) -> const vptr_type& {
            auto type = Registry::Rtti::dynamic_type(arg);
            auto iter = vptrs.find(type);

            if constexpr (Registry::RuntimeChecks) {
                if (iter == vptrs.end()) {
                    using ErrorHandler = typename Registry::ErrorHandler;

                    if constexpr (detail::is_not_void<ErrorHandler>) {
                        unknown_class_error error;
                        error.type = type;
                        ErrorHandler::error(error);
                    }

                    abort();
                }
            }

            if constexpr (IndirectVptr) {
                return *iter->second;
            } else {
                return iter->second;
            }
        }

        static auto finalize() -> void {
            vptrs.clear();
        }
    };
};

} // namespace policies
} // namespace boost::openmethod

#endif
