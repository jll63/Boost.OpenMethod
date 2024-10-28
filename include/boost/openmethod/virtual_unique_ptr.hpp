#ifndef BOOST_OPENMETHOD_VIRTUAL_UNIQUE_PTR_HPP
#define BOOST_OPENMETHOD_VIRTUAL_UNIQUE_PTR_HPP

#include <boost/openmethod/core.hpp>
#include <memory>

namespace boost {
namespace openmethod {

namespace detail {

template<class Class, class Policy>
struct virtual_ptr_traits<std::unique_ptr<Class>, Policy> {
    static bool constexpr is_smart_ptr = true;
    using element_type = Class;

    static auto dynamic_type(const std::unique_ptr<Class>& ptr) {
        return Policy::dynamic_type(*ptr);
    }

    template<typename Other>
    static auto cast(virtual_ptr<std::unique_ptr<Class>, Policy> ptr)
        -> decltype(auto) {
        static_assert(
            std::is_base_of_v<Class, Other>,
            "std::unique_ptr only supports upcast");

        return Other(ptr);
    }
};

}

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
