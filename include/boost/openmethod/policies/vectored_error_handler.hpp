// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_POLICY_VECTORED_ERROR_HPP
#define BOOST_OPENMETHOD_POLICY_VECTORED_ERROR_HPP

#include <boost/openmethod/policies/basic_policy.hpp>

#include <functional>
#include <variant>

namespace boost::openmethod::policies {

template<class Policy>
class vectored_error_handler : public error_handler {
  public:
    using error_variant = std::variant<
        openmethod_error, not_implemented_error, unknown_class_error,
        hash_search_error, type_mismatch_error, static_slot_error,
        static_stride_error>;

    using function_type = std::function<void(const error_variant& error)>;

    template<class Error>
    static auto error(const Error& error) -> void {
        fn(error_variant(error));
    }

    static auto set_error_handler(function_type handler) -> function_type {
        auto prev = fn;
        fn = handler;

        return prev;
    }

  private:
    static function_type fn;

    static auto default_handler(const error_variant& error_v) {
        using namespace detail;
        using namespace policies;

        if constexpr (Policy::template has_facet<error_output>) {
            if (auto error = std::get_if<not_implemented_error>(&error_v)) {
                Policy::error_stream << "no applicable overrider for ";
                Policy::type_name(error->method, Policy::error_stream);
                Policy::error_stream << "(";
                auto comma = "";

                for (auto ti :
                     range{error->types, error->types + error->arity}) {
                    Policy::error_stream << comma;
                    Policy::type_name(ti, Policy::error_stream);
                    comma = ", ";
                }

                Policy::error_stream << ")\n";
            } else if (
                auto error = std::get_if<unknown_class_error>(&error_v)) {
                Policy::error_stream << "unknown class ";
                Policy::type_name(error->type, Policy::error_stream);
                Policy::error_stream << "\n";
            } else if (
                auto error = std::get_if<type_mismatch_error>(&error_v)) {
                Policy::error_stream << "invalid method table for ";
                Policy::type_name(error->type, Policy::error_stream);
                Policy::error_stream << "\n";
            } else if (auto error = std::get_if<hash_search_error>(&error_v)) {
                Policy::error_stream << "could not find hash factors after "
                                     << error->attempts << "s using "
                                     << error->buckets << " buckets\n";
            }
        }
    }
};

template<class Policy>
typename vectored_error_handler<Policy>::function_type
    vectored_error_handler<Policy>::fn =
        vectored_error_handler<Policy>::default_handler;

} // namespace boost::openmethod::policies

#endif
