// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_OPENMETHOD_MACROS_HPP
#define BOOST_OPENMETHOD_MACROS_HPP

#include <boost/openmethod/macros/name.hpp>
#include <boost/openmethod/macros/register.hpp>

#include <boost/preprocessor/cat.hpp>

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

#define BOOST_OPENMETHOD_OVERRIDERS(NAME)                                      \
    BOOST_PP_CAT(BOOST_OPENMETHOD_NAME(NAME), _overriders)

#define BOOST_OPENMETHOD_GUIDE(NAME)                                           \
    BOOST_PP_CAT(BOOST_OPENMETHOD_NAME(NAME), _guide)

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
        BOOST_OPENMETHOD_GUIDE(NAME)(ForwarderParameters && ... args);         \
    template<typename... ForwarderParameters>                                  \
    inline auto NAME(ForwarderParameters&&... args) ->                         \
        typename ::boost::openmethod::detail::enable_forwarder<                \
            void,                                                              \
            ::boost::openmethod::method<                                       \
                BOOST_OPENMETHOD_NAME(NAME) ARGS, __VA_ARGS__>,                \
            typename ::boost::openmethod::method<                              \
                BOOST_OPENMETHOD_NAME(NAME) ARGS, __VA_ARGS__>::return_type,   \
            ForwarderParameters...>::type {                                    \
        return ::boost::openmethod::                                           \
            method<BOOST_OPENMETHOD_NAME(NAME) ARGS, __VA_ARGS__>::fn(         \
                std::forward<ForwarderParameters>(args)...);                   \
    }

#define BOOST_OPENMETHOD_DETAIL_LOCATE_METHOD(NAME, ARGS)                      \
    template<typename T>                                                       \
    struct boost_openmethod_detail_locate_method_aux;                          \
    template<typename... A>                                                    \
    struct boost_openmethod_detail_locate_method_aux<void(A...)> {             \
        using type =                                                           \
            decltype(BOOST_OPENMETHOD_GUIDE(NAME)(std::declval<A>()...));      \
    };

#define BOOST_OPENMETHOD_DETAIL_OVERRIDE(INLINE, NAME, ARGS, ...)              \
    template<typename>                                                         \
    struct BOOST_OPENMETHOD_OVERRIDERS(NAME);                                  \
    template<>                                                                 \
    struct BOOST_OPENMETHOD_OVERRIDERS(NAME)<__VA_ARGS__ ARGS> {               \
        BOOST_OPENMETHOD_DETAIL_LOCATE_METHOD(NAME, ARGS);                     \
        static auto fn ARGS->__VA_ARGS__;                                      \
        static auto has_next() -> bool;                                        \
        template<typename... Args>                                             \
        static auto next(Args&&... args) -> decltype(auto);                    \
    };                                                                         \
    inline auto BOOST_OPENMETHOD_OVERRIDERS(                                   \
        NAME)<__VA_ARGS__ ARGS>::has_next() -> bool {                          \
        using method_type =                                                    \
            boost_openmethod_detail_locate_method_aux<void ARGS>::type;        \
        return method_type::next<fn> != method_type::fn.not_implemented;       \
    }                                                                          \
    template<typename... Args>                                                 \
    inline auto BOOST_OPENMETHOD_OVERRIDERS(NAME)<__VA_ARGS__ ARGS>::next(     \
        Args&&... args) -> decltype(auto) {                                    \
        BOOST_ASSERT(has_next());                                              \
        return boost_openmethod_detail_locate_method_aux<                      \
            void ARGS>::type::next<fn>(std::forward<Args>(args)...);           \
    }                                                                          \
    INLINE BOOST_OPENMETHOD_REGISTER(                                          \
        BOOST_OPENMETHOD_OVERRIDERS(NAME) < __VA_ARGS__ ARGS >                 \
        ::boost_openmethod_detail_locate_method_aux<void ARGS>::type::         \
            override<                                                          \
                BOOST_OPENMETHOD_OVERRIDERS(NAME) < __VA_ARGS__ ARGS>::fn >);  \
    INLINE auto BOOST_OPENMETHOD_OVERRIDERS(NAME)<__VA_ARGS__ ARGS>::fn ARGS   \
        -> boost::mp11::mp_back<boost::mp11::mp_list<__VA_ARGS__>>

#define BOOST_OPENMETHOD_INLINE_OVERRIDE(NAME, ARGS, ...)                      \
    BOOST_OPENMETHOD_DETAIL_OVERRIDE(inline, NAME, ARGS, __VA_ARGS__)

#define BOOST_OPENMETHOD_OVERRIDE(NAME, ARGS, ...)                             \
    BOOST_OPENMETHOD_DETAIL_OVERRIDE(, NAME, ARGS, __VA_ARGS__)

#define BOOST_OPENMETHOD_CLASSES(...)                                          \
    BOOST_OPENMETHOD_REGISTER(::boost::openmethod::use_classes<__VA_ARGS__>);

#endif
