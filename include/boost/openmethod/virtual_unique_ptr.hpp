#ifndef BOOST_OPENMETHOD_VIRTUAL_UNIQUE_PTR_HPP
#define BOOST_OPENMETHOD_VIRTUAL_UNIQUE_PTR_HPP

#include <boost/openmethod/core.hpp>
#include <memory>

namespace boost {
namespace openmethod {

template<class Class, class Policy>
struct virtual_ptr_traits<std::unique_ptr<Class>, Policy> {
    static bool constexpr smart_ptr = true;
    using element_type = Class;

    static auto dynamic_type(const std::unique_ptr<Class>& ptr) -> type_id {
        return Policy::dynamic_type(*ptr);
    }

    template<typename Other>
    static decltype(auto)
    cast(virtual_ptr<std::unique_ptr<Class>, Policy>&& ptr) {
        auto p = ptr.obj.release();

        if constexpr (detail::requires_dynamic_cast<Class&, Other&>) {
            return virtual_ptr<std::unique_ptr<Other>, Policy>(
                std::dynamic_pointer_cast<Other>(
                    std::unique_ptr<Other>(ptr.obj.release())),
                ptr.vp);
        } else {
            return virtual_ptr<std::unique_ptr<Other>, Policy>(
                std::static_pointer_cast<Other>(
                    std::unique_ptr<Other>(ptr.obj.release())),
                ptr.vp);
        }
    }
};

template<class Class, class Policy = BOOST_OPENMETHOD_DEFAULT_POLICY>
using virtual_unique_ptr = virtual_ptr<std::unique_ptr<Class>, Policy>;

template<
    class Class, class Policy = BOOST_OPENMETHOD_DEFAULT_POLICY, typename... T>
inline auto make_virtual_unique(T&&... args) {
    return virtual_unique_ptr<Class, Policy>::final(
        std::make_unique<Class>(std::forward<T>(args)...));
}

} // namespace openmethod
} // namespace boost

#endif
