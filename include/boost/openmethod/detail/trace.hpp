// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_DETAIL_TRACE_HPP
#define BOOST_OPENMETHOD_DETAIL_TRACE_HPP

#include <boost/openmethod/policies/basic_policy.hpp>

#include <boost/dynamic_bitset.hpp>

namespace boost::openmethod {

namespace detail {

template<typename Iterator>
struct range;

struct rflush {
    std::size_t width;
    std::size_t value;
    explicit rflush(std::size_t width, std::size_t value)
        : width(width), value(value) {
    }
};

struct type_name {
    type_name(type_id type) : type(type) {
    }
    type_id type;
};

template<class Policy>
struct trace_type {
    static constexpr bool trace_enabled =
        Policy::template has_facet<policies::trace_output>;

    std::size_t indentation_level{0};

    auto operator++() -> trace_type& {
        if constexpr (trace_enabled) {
            if (Policy::trace_enabled) {
                for (int i = 0; i < indentation_level; ++i) {
                    Policy::trace_stream << "  ";
                }
            }
        }

        return *this;
    }

    struct indent {
        trace_type& trace;
        int by;

        explicit indent(trace_type& trace, int by = 2) : trace(trace), by(by) {
            trace.indentation_level += by;
        }

        ~indent() {
            trace.indentation_level -= by;
        }
    };
};

template<class Policy, typename T, typename F>
auto write_range(trace_type<Policy>& trace, range<T> range, F fn) -> auto& {
    if constexpr (trace_type<Policy>::trace_enabled) {
        if (Policy::trace_enabled) {
            trace << "(";
            const char* sep = "";
            for (auto value : range) {
                trace << sep << fn(value);
                sep = ", ";
            }

            trace << ")";
        }
    }

    return trace;
}

template<class Policy, typename T>
auto operator<<(trace_type<Policy>& trace, const T& value) -> auto& {
    if constexpr (trace_type<Policy>::trace_enabled) {
        if (Policy::trace_enabled) {
            Policy::trace_stream << value;
        }
    }
    return trace;
}

template<class Policy>
auto operator<<(trace_type<Policy>& trace, const rflush& rf) -> auto& {
    if constexpr (trace_type<Policy>::trace_enabled) {
        if (Policy::trace_enabled) {
            auto pad = rf.width;
            auto remain = rf.value;

            int digits = 1;
            auto tmp = rf.value / 10;

            while (tmp) {
                ++digits;
                tmp /= 10;
            }

            while (digits < rf.width) {
                trace << " ";
                ++digits;
            }

            trace << rf.value;
        }
    }

    return trace;
}

template<class Policy>
auto operator<<(trace_type<Policy>& trace, const boost::dynamic_bitset<>& bits)
    -> auto& {
    if constexpr (trace_type<Policy>::trace_enabled) {
        if (Policy::trace_enabled) {
            if (Policy::trace_enabled) {
                auto i = bits.size();
                while (i != 0) {
                    --i;
                    Policy::trace_stream << bits[i];
                }
            }
        }
    }

    return trace;
}

template<class Policy>
auto operator<<(trace_type<Policy>& trace, const range<type_id*>& tips)
    -> auto& {
    return write_range(trace, tips, [](auto tip) { return type_name(tip); });
}

template<class Policy, typename T>
auto operator<<(trace_type<Policy>& trace, const range<T>& range) -> auto& {
    return write_range(trace, range, [](auto value) { return value; });
}

template<class Policy>
auto operator<<(trace_type<Policy>& trace, const type_name& manip) -> auto& {
    if constexpr (Policy::template has_facet<policies::trace_output>) {
        Policy::type_name(manip.type, trace);
    }

    return trace;
}

} // namespace detail
} // namespace boost::openmethod

#endif // BOOST_OPENMETHOD_DETAIL_HPP
