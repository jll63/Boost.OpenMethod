#ifndef BOOST_OPENMETHOD_DETAIL_MRDOCS_HPP
#define BOOST_OPENMETHOD_DETAIL_MRDOCS_HPP

#include <memory>
#include <utility>

namespace boost::openmethod {

template<typename Name, typename ReturnType, class Registry>
class method;

template<
    typename Name, typename... Parameters, typename ReturnType, class Registry>
class method<Name, ReturnType(Parameters...), Registry>;

template<class... Classes>
struct use_classes;

template<typename T>
struct virtual_;

template<class Class, class Registry, typename>
class virtual_ptr;

namespace detail {
template<typename, class, typename>
struct is_smart_ptr_aux;
}

template<typename T, class Registry>
constexpr bool is_smart_ptr =
    detail::is_smart_ptr_aux<T, Registry, void>::value;

template<class Class, class Registry>
class virtual_ptr<
    Class, Registry, std::enable_if_t<is_smart_ptr<Class, Registry>>>;

template<class Class, class Registry>
using shared_virtual_ptr = virtual_ptr<std::shared_ptr<Class>, Registry, void>;

struct openmethod_error;
struct not_initialized_error;
struct unknown_class_error;
struct hash_search_error;
struct call_error;
struct not_implemented_error;
struct ambiguous_error;
struct final_error;
struct static_offset_error;
struct static_slot_error;
struct static_stride_error;

template<typename T, class Registry>
struct virtual_traits;

template<typename T, class Registry>
struct virtual_traits<T&, Registry>;

template<typename T, class Registry>
struct virtual_traits<T&&, Registry>;

template<typename T, class Registry>
struct virtual_traits<T*, Registry>;

template<class Class, class Registry>
struct virtual_traits<virtual_ptr<Class, Registry, void>, Registry>;

template<class Class, class Registry>
struct virtual_traits<const virtual_ptr<Class, Registry, void>&, Registry>;

template<class Class, class Registry>
struct virtual_traits<const std::shared_ptr<Class>&, Registry>;

template<class Class, class Registry>
struct virtual_traits<std::shared_ptr<Class>, Registry>;

template<class Class, class Registry>
struct virtual_traits<std::unique_ptr<Class>, Registry>;

} // namespace boost::openmethod

#endif // BOOST_OPENMETHOD_DETAIL_MRDOCS_HPP
