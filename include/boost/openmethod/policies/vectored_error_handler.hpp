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
        openmethod_error, not_implemented_error, unknown_class_error,
        hash_search_error, final_error, static_slot_error, static_stride_error>;

    using function_type = std::function<void(const error_variant& error)>;

    template<class Registry>
    class fn {
      public:
        template<class Error>
        static auto error(const Error& error) -> void {
            handler(error_variant(error));
        }

        static auto set(function_type handler) -> function_type {
            auto prev = handler;
            fn::handler = handler;

            return prev;
        }

        static auto default_handler(const error_variant& error_v) {
            using namespace detail;
            using namespace policies;

            if constexpr (Registry::has_output) {
                auto& os = Registry::template policy<policies::output>::os;

                if (auto error = std::get_if<not_implemented_error>(&error_v)) {
                    os << "no applicable overrider for ";
                    Registry::template policy<policies::rtti>::type_name(
                        error->method, os);
                    os << "(";
                    auto comma = "";

                    for (auto ti :
                         range{error->types, error->types + error->arity}) {
                        os << comma;
                        Registry::template policy<policies::rtti>::type_name(
                            ti, os);
                        comma = ", ";
                    }

                    os << ")\n";
                } else if (
                    auto error = std::get_if<unknown_class_error>(&error_v)) {
                    os << "unknown class ";
                    Registry::template policy<policies::rtti>::type_name(
                        error->type, os);
                    os << "\n";
                } else if (auto error = std::get_if<final_error>(&error_v)) {
                    os << "invalid method table for ";
                    Registry::template policy<policies::rtti>::type_name(
                        error->type, os);
                    os << "\n";
                } else if (
                    auto error = std::get_if<hash_search_error>(&error_v)) {
                    os << "could not find hash factors after "
                       << error->attempts << "s using " << error->buckets
                       << " buckets\n";
                }
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
