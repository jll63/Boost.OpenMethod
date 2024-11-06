#ifndef BOOST_OPENMETHOD_INTRUSIVE_VPTR_HPP
#define BOOST_OPENMETHOD_INTRUSIVE_VPTR_HPP

#include <boost/openmethod/core.hpp>

namespace boost {
namespace openmethod {

template<class Class, class Policy = BOOST_OPENMETHOD_DEFAULT_POLICY>
struct set_vptr {
    set_vptr() {
        if constexpr (Policy::template has_facet<policies::indirect_vptr>) {
            reinterpret_cast<Class*>(this)->boost_openmethod_vptr =
                &Policy::template static_vptr<Class>;
        } else {
            reinterpret_cast<Class*>(this)->boost_openmethod_vptr =
                Policy::template static_vptr<Class>;
        }
    }
};

} // namespace openmethod
} // namespace boost

#endif // BOOST_OPENMETHOD_INTRUSIVE_VPTR_HPP
