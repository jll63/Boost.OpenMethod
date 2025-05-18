// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_DETAIL_TRACE_HPP
#define BOOST_OPENMETHOD_DETAIL_TRACE_HPP

#include <boost/openmethod/registry.hpp>

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

template<class Registry>
struct trace_type {
    std::size_t indentation_level{0};

    auto operator++() -> trace_type& {
        if constexpr (Registry::Trace) {
            using trace = typename Registry::template policy<policies::trace>;
            if (trace::trace_enabled) {
                for (std::size_t i = 0; i < indentation_level; ++i) {
                    Registry::template policy<policies::output>::os << "  ";
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

template<class Registry, typename T, typename F>
auto write_range(trace_type<Registry>& trace, range<T> range, F fn) -> auto& {
    if constexpr (Registry::Trace) {
        if (Registry::template policy<policies::trace>::trace_enabled) {
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

template<class Registry, typename T>
auto operator<<(trace_type<Registry>& trace, const T& value) -> auto& {
    if constexpr (Registry::Trace) {
        if (Registry::template policy<policies::trace>::trace_enabled) {
            Registry::template policy<policies::output>::os << value;
        }
    }
    return trace;
}

template<class Registry>
auto operator<<(trace_type<Registry>& trace, const rflush& rf) -> auto& {
    if constexpr (Registry::Trace) {
        if (Registry::template policy<policies::trace>::trace_enabled) {
            std::size_t digits = 1;
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

template<class Registry>
auto operator<<(
    trace_type<Registry>& trace, const boost::dynamic_bitset<>& bits) -> auto& {
    if constexpr (Registry::Trace) {
        if (Registry::template policy<policies::trace>::trace_enabled) {
            auto i = bits.size();
            while (i != 0) {
                --i;
                Registry::template policy<policies::output>::os << bits[i];
            }
        }
    }

    return trace;
}

template<class Registry>
auto operator<<(trace_type<Registry>& trace, const range<type_id*>& tips)
    -> auto& {
    return write_range(trace, tips, [](auto tip) { return type_name(tip); });
}

template<class Registry, typename T>
auto operator<<(trace_type<Registry>& trace, const range<T>& range) -> auto& {
    return write_range(trace, range, [](auto value) { return value; });
}

template<class Registry>
auto operator<<(trace_type<Registry>& trace, const type_name& manip) -> auto& {
    if constexpr (Registry::Trace) {
        Registry::template policy<policies::rtti>::type_name(manip.type, trace);
    }

    return trace;
}

} // namespace detail
} // namespace boost::openmethod

#endif // BOOST_OPENMETHOD_DETAIL_HPP
