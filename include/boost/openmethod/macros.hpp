// Copyright (c) 2018-2024 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_MACROS_HPP
#define BOOST_OPENMETHOD_MACROS_HPP

#include <boost/openmethod/macros/name.hpp>
#include <boost/openmethod/macros/register.hpp>

#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/tuple/size.hpp>

#define BOOST_OPENMETHOD_OVERRIDERS(NAME)                                      \
    BOOST_PP_CAT(BOOST_OPENMETHOD_NAME(NAME), _overriders)

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

#define BOOST_OPENMETHOD(NAME, ARGS, ...)                                      \
    struct BOOST_OPENMETHOD_NAME(NAME);                                        \
    template<typename... ForwarderParameters>                                  \
    typename ::boost::openmethod::detail::enable_forwarder<                    \
        void,                                                                  \
        ::boost::openmethod::method<                                           \
            BOOST_OPENMETHOD_NAME(NAME) ARGS, __VA_ARGS__>,                    \
        typename ::boost::openmethod::method<                                  \
            BOOST_OPENMETHOD_NAME(NAME) ARGS, __VA_ARGS__>,                    \
        ForwarderParameters...>::type                                          \
        BOOST_PP_CAT(BOOST_OPENMETHOD_NAME(NAME), _guide)(                     \
            ForwarderParameters && ... args);                                  \
    template<typename... ForwarderParameters>                                  \
    inline typename ::boost::openmethod::detail::enable_forwarder<             \
        void,                                                                  \
        ::boost::openmethod::method<                                           \
            BOOST_OPENMETHOD_NAME(NAME) ARGS, __VA_ARGS__>,                    \
        typename ::boost::openmethod::method<                                  \
            BOOST_OPENMETHOD_NAME(NAME) ARGS, __VA_ARGS__>::return_type,       \
        ForwarderParameters...>::type                                          \
    NAME(ForwarderParameters&&... args) {                                      \
        return ::boost::openmethod::                                           \
            method<BOOST_OPENMETHOD_NAME(NAME) ARGS, __VA_ARGS__>::fn(         \
                std::forward<ForwarderParameters>(args)...);                   \
    }

#define BOOST_OPENMETHOD_DETAIL_LOCATE_METHOD(NAME, ARGS)                      \
    template<typename T>                                                       \
    struct boost_openmethod_detail_locate_method_aux;                          \
    template<typename... A>                                                    \
    struct boost_openmethod_detail_locate_method_aux<void(A...)> {             \
        using type = decltype(NAME##_guide(std::declval<A>()...));             \
    };                                                                         \
    using method_type =                                                        \
        boost_openmethod_detail_locate_method_aux<void ARGS>::type

#define BOOST_OPENMETHOD_DETAIL_OVERRIDE(INLINE, OVERRIDERS, NAME, ARGS, ...)  \
    template<typename...>                                                      \
    struct OVERRIDERS;                                                         \
    template<>                                                                 \
    struct OVERRIDERS<__VA_ARGS__ ARGS> {                                      \
        BOOST_OPENMETHOD_DETAIL_LOCATE_METHOD(NAME, ARGS);                     \
        static auto fn ARGS->__VA_ARGS__;                                      \
        static auto has_next() {                                               \
            return method_type::next<fn> != method_type::fn.not_implemented;   \
        }                                                                      \
        template<typename... Args>                                             \
        static decltype(auto) next(Args&&... args) {                           \
            BOOST_ASSERT(has_next());                                          \
            return method_type::next<fn>(std::forward<Args>(args)...);         \
        }                                                                      \
    };                                                                         \
    INLINE BOOST_OPENMETHOD_REGISTER(                                          \
        OVERRIDERS<__VA_ARGS__ ARGS>::method_type::override<                   \
            OVERRIDERS<__VA_ARGS__ ARGS>::fn>);                                \
    INLINE auto OVERRIDERS<__VA_ARGS__ ARGS>::fn ARGS                          \
        ->boost::mp11::mp_back<boost::mp11::mp_list<__VA_ARGS__>>

#define BOOST_OPENMETHOD_INLINE_OVERRIDE(NAME, ARGS, ...)                      \
    BOOST_OPENMETHOD_DETAIL_OVERRIDE(                                          \
        inline, BOOST_OPENMETHOD_OVERRIDERS(NAME),                             \
        BOOST_OPENMETHOD_NAME(NAME), ARGS, __VA_ARGS__)

#define BOOST_OPENMETHOD_OVERRIDE(NAME, ARGS, ...)                             \
    BOOST_OPENMETHOD_DETAIL_OVERRIDE(                                          \
        , BOOST_OPENMETHOD_OVERRIDERS(NAME), BOOST_OPENMETHOD_NAME(NAME),      \
        ARGS, __VA_ARGS__)

#define BOOST_OPENMETHOD_CLASSES(...)                                          \
    BOOST_OPENMETHOD_REGISTER(::boost::openmethod::use_classes<__VA_ARGS__>);

#endif
