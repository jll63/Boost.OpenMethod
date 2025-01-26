#ifndef BOOST_OPENMETHOD_VIRTUAL_SHARED_PTR_HPP
#define BOOST_OPENMETHOD_VIRTUAL_SHARED_PTR_HPP

#include <boost/openmethod/core.hpp>
#include <boost/openmethod/shared_ptr.hpp>
#include <memory>

namespace boost {
namespace openmethod {

template<class Class, class Policy = BOOST_OPENMETHOD_DEFAULT_POLICY>
using virtual_shared_ptr = virtual_ptr<std::shared_ptr<Class>, Policy>;

template<
    class Class, class Policy = BOOST_OPENMETHOD_DEFAULT_POLICY, typename... T>
inline auto make_virtual_shared(T&&... args) {
    return virtual_shared_ptr<Class, Policy>::final(
        std::make_shared<Class>(std::forward<T>(args)...));
}

} // namespace openmethod
} // namespace boost

#endif
