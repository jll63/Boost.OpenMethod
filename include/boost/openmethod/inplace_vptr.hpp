#ifndef BOOST_OPENMETHOD_inplace_vptr_HPP
#define BOOST_OPENMETHOD_inplace_vptr_HPP

#include <boost/openmethod/core.hpp>

// =============================================================================
// inplace_vptr

namespace boost::openmethod {

namespace detail {

void boost_openmethod_registry(...);
void boost_openmethod_bases(...);

template<class Class>
using inplace_vptr_registry =
    decltype(boost_openmethod_registry(std::declval<Class*>()));

template<class>
struct update_vptr_bases;

template<class To, class Class>
void update_vptr(Class* obj);

template<class... Bases>
struct update_vptr_bases<mp11::mp_list<Bases...>> {
    template<class To, class Class>
    static void fn(Class* obj) {
        (update_vptr<To>(static_cast<Bases*>(obj)), ...);
    }
};

template<class To, class Class>
void update_vptr(Class* obj) {
    using registry = inplace_vptr_registry<Class>;
    using bases = decltype(boost_openmethod_bases(obj));

    if constexpr (mp11::mp_size<bases>::value == 0) {
        if constexpr (registry::indirect_vptr) {
            obj->boost_openmethod_vptr = &registry::template static_vptr<To>;
        } else {
            obj->boost_openmethod_vptr = registry::template static_vptr<To>;
        }
    } else {
        update_vptr_bases<bases>::template fn<To, Class>(obj);
    }
}

struct inplace_vptr_derived {};

template<class, class, bool>
class inplace_vptr_aux;

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic ignored "-Wnon-template-friend"
#endif

template<class... Classes>
inline use_classes<Classes...> inplace_vptr_use_classes;

template<class Class, class Registry>
class inplace_vptr_aux<Class, Registry, true> {
  protected:
    template<class To, class Other>
    friend void update_vptr(Other*);
    friend auto boost_openmethod_registry(Class*) -> Registry;
    friend auto boost_openmethod_bases(Class*) -> mp11::mp_list<>;

    inplace_vptr_aux() {
        (void)&inplace_vptr_use_classes<Class, Registry>;
        detail::update_vptr<Class>(static_cast<Class*>(this));
    }

    ~inplace_vptr_aux() {
        boost_openmethod_vptr = nullptr;
    }

    friend auto boost_openmethod_vptr(const Class& obj, Registry*)
        -> vptr_type {
        if constexpr (Registry::indirect_vptr) {
            return *obj.boost_openmethod_vptr;
        } else {
            return obj.boost_openmethod_vptr;
        }
    }

    friend auto boost_openmethod_registry(Class*) -> Registry;

    std::conditional_t<Registry::indirect_vptr, const vptr_type*, vptr_type>
        boost_openmethod_vptr = nullptr;
};

template<class Class, class Base>
class inplace_vptr_aux<Class, Base, false> : inplace_vptr_derived {
  protected:
    friend void update_vptr(Class*);

    inplace_vptr_aux() {
        (void)&inplace_vptr_use_classes<
            Class, Base, inplace_vptr_registry<Class>>;
        detail::update_vptr<Class>(static_cast<Class*>(this));
    }

    ~inplace_vptr_aux() {
        detail::update_vptr<Base>(
            static_cast<Base*>(static_cast<Class*>(this)));
    }

    friend auto boost_openmethod_bases(Class*) -> mp11::mp_list<Base>;
};

} // namespace detail

template<typename...>
class inplace_vptr;

template<class Class>
class inplace_vptr<Class>
    : public detail::inplace_vptr_aux<
          Class, BOOST_OPENMETHOD_DEFAULT_REGISTRY, true> {};

template<class Class, class Other>
class inplace_vptr<Class, Other>
    : public detail::inplace_vptr_aux<
          Class, Other, detail::is_registry<Other>> {};

template<class Class, class Base1, class Base2, class... MoreBases>
class inplace_vptr<Class, Base1, Base2, MoreBases...>
    : detail::inplace_vptr_derived {

    static_assert(
        !detail::is_registry<Base1> && !detail::is_registry<Base2> &&
            (!detail::is_registry<MoreBases> && ...),
        "registry can be specified only for root classes");

  protected:
    inplace_vptr() {
        (void)&detail::inplace_vptr_use_classes<
            Class, Base1, Base2, MoreBases...,
            detail::inplace_vptr_registry<Base1>>;
        detail::update_vptr<Class>(static_cast<Class*>(this));
    }

    ~inplace_vptr() {
        auto obj = static_cast<Class*>(this);
        detail::update_vptr<Base1>(static_cast<Base1*>(obj));
        detail::update_vptr<Base2>(static_cast<Base2*>(obj));
        (detail::update_vptr<MoreBases>(static_cast<Base2*>(obj)), ...);
    }

    friend auto boost_openmethod_registry(Class*)
        -> detail::inplace_vptr_registry<Base1>;
    friend auto boost_openmethod_bases(Class*)
        -> mp11::mp_list<Base1, Base2, MoreBases...>;
    friend auto boost_openmethod_vptr(
        const Class& obj, detail::inplace_vptr_registry<Base1>* registry)
        -> vptr_type {
        return boost_openmethod_vptr(static_cast<const Base1&>(obj), registry);
    }
};

} // namespace boost::openmethod

#endif // BOOST_OPENMETHOD_inplace_vptr_HPP
