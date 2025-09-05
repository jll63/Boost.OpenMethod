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

//! Stores v-table pointers in a map keyed by `type_id`s.
//!
//! `vptr_map` stores v-table pointers in a map keyed by `type_id`s.
//!
//! If the registry contains the @ref indirect_vptr policy, `vptr_map` stores
//! pointers to pointers to v-tables.
//!
//! @tparam MapFn A mp11 quoted meta-function that takes a key type and a
//! value type, and returns an @ref AssociativeContainer.
template<class MapFn = mp11::mp_quote<std::unordered_map>>
class vptr_map : public vptr {
  public:
    //! A model of @ref vptr::fn.
    //!
    //! @tparam Registry The registry containing this policy.
    template<class Registry>
    class fn {
        using Value = std::conditional_t<
            Registry::has_indirect_vptr, const vptr_type*, vptr_type>;
        static inline typename MapFn::template fn<type_id, Value> vptrs;

      public:
        //! Stores the v-table pointers.
        //!
        //! @tparam ForwardIterator A forward iterator yielding
        //! @ref IdsToVptr objects.
        //! @param first The beginning of the range.
        //! @param last The end of the range.
        template<typename ForwardIterator>
        static void initialize(ForwardIterator first, ForwardIterator last) {
            for (auto iter = first; iter != last; ++iter) {
                for (auto type_iter = iter->type_id_begin();
                     type_iter != iter->type_id_end(); ++type_iter) {

                    if constexpr (Registry::has_indirect_vptr) {
                        vptrs.emplace(*type_iter, &iter->vptr());
                    } else {
                        vptrs.emplace(*type_iter, iter->vptr());
                    }
                }
            }
        }

        //! Returns a reference to a v-table pointer for an object.
        //!
        //! Acquires the dynamic @ref type_id of `arg`, using the registry's
        //! @ref rtti policy.
        //!
        //! If the registry contains the @ref runtime_checks policy, checks that
        //! the map contains the type id. If it does not, and if the registry
        //! contains a @ref error_handler policy, calls its
        //! @ref error function with a @ref unknown_class_error value, then
        //! terminates the program with @ref abort.
        //!
        //! @tparam Class A registered class.
        //! @param arg A reference to a const object of type `Class`.
        //! @return A reference to a the v-table pointer for `Class`.
        template<class Class>
        static auto dynamic_vptr(const Class& arg) -> const vptr_type& {
            auto type = Registry::rtti::dynamic_type(arg);
            auto iter = vptrs.find(type);

            if constexpr (Registry::has_runtime_checks) {
                if (iter == vptrs.end()) {
                    if constexpr (Registry::has_error_handler) {
                        unknown_class_error error;
                        error.type = type;
                        Registry::error_handler::error(error);
                    }

                    abort();
                }
            }

            if constexpr (Registry::has_indirect_vptr) {
                return *iter->second;
            } else {
                return iter->second;
            }
        }

        //! Clears the map.
        static auto finalize() -> void {
            vptrs.clear();
        }
    };
};

} // namespace policies
} // namespace boost::openmethod

#endif
