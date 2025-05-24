// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_MACROS_HPP
#define BOOST_OPENMETHOD_MACROS_HPP

#include <boost/preprocessor/cat.hpp>

#include <boost/openmethod/core.hpp>

namespace boost::openmethod::detail {

template<typename, class Method, typename ReturnType, typename... Parameters>
struct enable_forwarder;

template<class Method, typename ReturnType, typename... Parameters>
struct enable_forwarder<
    std::void_t<decltype(Method::fn(std::declval<Parameters>()...))>, Method,
    ReturnType, Parameters...> {
    using type = ReturnType;
};

} // namespace boost::openmethod::detail

#define BOOST_OPENMETHOD_GENSYM BOOST_PP_CAT(openmethod_gensym_, __COUNTER__)

#define BOOST_OPENMETHOD_REGISTER(...)                                         \
    static __VA_ARGS__ BOOST_OPENMETHOD_GENSYM

#define BOOST_OPENMETHOD_NAME(NAME) NAME##_boost_openmethod

#define BOOST_OPENMETHOD_OVERRIDERS(NAME)                                      \
    BOOST_PP_CAT(BOOST_OPENMETHOD_NAME(NAME), _overriders)

#define BOOST_OPENMETHOD_GUIDE(NAME)                                           \
    BOOST_PP_CAT(BOOST_OPENMETHOD_NAME(NAME), _guide)

#define BOOST_OPENMETHOD(NAME, ...)                                            \
    struct BOOST_OPENMETHOD_NAME(NAME);                                        \
    template<typename... ForwarderParameters>                                  \
    typename ::boost::openmethod::detail::enable_forwarder<                    \
        void,                                                                  \
        ::boost::openmethod::method<                                           \
            BOOST_OPENMETHOD_NAME(NAME), auto __VA_ARGS__>,                    \
        typename ::boost::openmethod::method<                                  \
            BOOST_OPENMETHOD_NAME(NAME), auto __VA_ARGS__>,                    \
        ForwarderParameters...>::type                                          \
        BOOST_OPENMETHOD_GUIDE(NAME)(ForwarderParameters && ... args);         \
    template<typename... ForwarderParameters>                                  \
    inline auto NAME(ForwarderParameters&&... args) ->                         \
        typename ::boost::openmethod::detail::enable_forwarder<                \
            void,                                                              \
            ::boost::openmethod::method<                                       \
                BOOST_OPENMETHOD_NAME(NAME), auto __VA_ARGS__>,                \
            typename ::boost::openmethod::method<                              \
                BOOST_OPENMETHOD_NAME(NAME), auto __VA_ARGS__>::return_type,   \
            ForwarderParameters...>::type {                                    \
        return ::boost::openmethod::                                           \
            method<BOOST_OPENMETHOD_NAME(NAME), auto __VA_ARGS__>::fn(         \
                std::forward<ForwarderParameters>(args)...);                   \
    }

#define BOOST_OPENMETHOD_DETAIL_LOCATE_METHOD(NAME)                            \
    template<typename T>                                                       \
    struct boost_openmethod_detail_locate_method_aux;                          \
    template<typename... A, typename R>                                        \
    struct boost_openmethod_detail_locate_method_aux<auto(A...)->R> {          \
        using type =                                                           \
            decltype(BOOST_OPENMETHOD_GUIDE(NAME)(std::declval<A>()...));      \
    };

#define BOOST_OPENMETHOD_DECLARE_OVERRIDER(NAME, ...)                          \
    template<typename>                                                         \
    struct BOOST_OPENMETHOD_OVERRIDERS(NAME);                                  \
    template<>                                                                 \
    struct BOOST_OPENMETHOD_OVERRIDERS(NAME)<auto __VA_ARGS__> {               \
        BOOST_OPENMETHOD_DETAIL_LOCATE_METHOD(NAME);                           \
        static auto fn __VA_ARGS__;                                            \
        static auto has_next() -> bool;                                        \
        template<typename... Args>                                             \
        static auto next(Args&&... args) -> decltype(auto);                    \
    };                                                                         \
    inline auto BOOST_OPENMETHOD_OVERRIDERS(                                   \
        NAME)<auto __VA_ARGS__>::has_next() -> bool {                          \
        return boost_openmethod_detail_locate_method_aux<                      \
            auto __VA_ARGS__>::type::has_next<fn>();                           \
    }                                                                          \
    template<typename... Args>                                                 \
    inline auto BOOST_OPENMETHOD_OVERRIDERS(NAME)<auto __VA_ARGS__>::next(     \
        Args&&... args) -> decltype(auto) {                                    \
        BOOST_ASSERT(has_next());                                              \
        return boost_openmethod_detail_locate_method_aux<                      \
            auto __VA_ARGS__>::type::next<fn>(std::forward<Args>(args)...);    \
    }                                                                          \
    inline BOOST_OPENMETHOD_REGISTER(                                          \
        BOOST_OPENMETHOD_OVERRIDERS(NAME) < auto __VA_ARGS__ >                 \
        ::boost_openmethod_detail_locate_method_aux<auto __VA_ARGS__>::type::  \
            override<                                                          \
                BOOST_OPENMETHOD_OVERRIDERS(NAME) < auto __VA_ARGS__>::fn >);

#define BOOST_OPENMETHOD_OVERRIDER(NAME, ...)                                  \
    BOOST_OPENMETHOD_OVERRIDERS(NAME)<auto __VA_ARGS__>

#define BOOST_OPENMETHOD_DEFINE_OVERRIDER(NAME, ...)                           \
    auto BOOST_OPENMETHOD_OVERRIDER(NAME, __VA_ARGS__)::fn __VA_ARGS__

#define BOOST_OPENMETHOD_INLINE_OVERRIDE(NAME, ...)                            \
    BOOST_OPENMETHOD_DECLARE_OVERRIDER(NAME, __VA_ARGS__)                      \
    inline BOOST_OPENMETHOD_DEFINE_OVERRIDER(NAME, __VA_ARGS__)

#define BOOST_OPENMETHOD_OVERRIDE(NAME, ...)                                   \
    BOOST_OPENMETHOD_DECLARE_OVERRIDER(NAME, __VA_ARGS__)                      \
    BOOST_OPENMETHOD_DEFINE_OVERRIDER(NAME, __VA_ARGS__)

#define BOOST_OPENMETHOD_CLASSES(...)                                          \
    BOOST_OPENMETHOD_REGISTER(::boost::openmethod::use_classes<__VA_ARGS__>);

#endif
