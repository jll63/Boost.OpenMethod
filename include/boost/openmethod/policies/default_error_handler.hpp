// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_POLICY_VECTORED_ERROR_HPP
#define BOOST_OPENMETHOD_POLICY_VECTORED_ERROR_HPP

#include <boost/openmethod/registry.hpp>

#include <functional>
#include <variant>

namespace boost::openmethod::policies {

struct default_error_handler : error_handler {
    using error_variant = std::variant<
        not_initialized_error, not_implemented_error, ambiguous_error,
        unknown_class_error, hash_search_error, final_error, static_slot_error,
        static_stride_error>;

    using function_type = std::function<void(const error_variant& error)>;

    template<class Registry>
    class fn {
      public:
        template<class Error>
        static auto error(const Error& error) -> void {
            handler(error_variant(error));
        }

        static auto set(function_type new_handler) -> function_type {
            auto prev = handler;
            handler = new_handler;

            return prev;
        }

        static auto default_handler(const error_variant& error) -> void {
            if constexpr (Registry::template has_policy<output>) {
                std::visit(
                    [](auto&& error) {
                        error.template write<Registry>(Registry::output::os);
                    },
                    error);
                Registry::output::os << "\n";
            }
        }

      private:
        static function_type
            handler; // Cannot be inline static because it confuses MSVC
    };
};

template<class Registry>
typename default_error_handler::function_type
    default_error_handler::fn<Registry>::handler = default_handler;

} // namespace boost::openmethod::policies

#endif
