#ifndef BOOST_OPENMETHOD_VIRTUAL_SHARED_PTR_HPP
#define BOOST_OPENMETHOD_VIRTUAL_SHARED_PTR_HPP

#include <boost/openmethod/core.hpp>
#include <boost/openmethod/shared_ptr.hpp>
#include <memory>

namespace boost {
namespace openmethod {

template<class Class, class Policy>
struct virtual_ptr_traits<std::shared_ptr<Class>, Policy> {
    static bool constexpr smart_ptr = true;
    using element_type = Class;

    static auto dynamic_type(const std::shared_ptr<Class>& obj) -> type_id {
        return Policy::dynamic_type(*obj);
    }

    template<typename Other>
    static decltype(auto)
    cast(const virtual_ptr<std::shared_ptr<Class>, Policy>& ptr) {
        if constexpr (detail::requires_dynamic_cast<Class&, Other&>) {
            return virtual_ptr<std::shared_ptr<Other>, Policy>(
                std::dynamic_pointer_cast<Other>(ptr.pointer()), ptr.vp);
        } else {
            return virtual_ptr<std::shared_ptr<Other>, Policy>(
                std::static_pointer_cast<Other>(ptr.pointer()), ptr.vp);
        }
    }
};

template<class Class, class Policy>
struct virtual_ptr_traits<const std::shared_ptr<Class>&, Policy>
    : virtual_ptr_traits<std::shared_ptr<Class>, Policy> {};

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
