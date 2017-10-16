#ifndef BOOST_OPENMETHOD_GENSYM

#include <boost/openmethod/macros/gensym.hpp>

#define BOOST_OPENMETHOD_REGISTER(...)                                         \
    static __VA_ARGS__ BOOST_OPENMETHOD_GENSYM

#endif
