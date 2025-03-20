#ifndef BOOST_OPENMETHOD_WITH_VPTR_HPP
#define BOOST_OPENMETHOD_WITH_VPTR_HPP

#include <boost/openmethod/core.hpp>

// =============================================================================
// with_vptr

namespace boost::openmethod {

namespace detail {

void boost_openmethod_policy(...);
void boost_openmethod_bases(...);

template<class Class>
using with_vptr_policy =
    decltype(boost_openmethod_policy(std::declval<Class*>()));

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
    using policy = with_vptr_policy<Class>;
    using bases = decltype(boost_openmethod_bases(obj));

    if constexpr (mp11::mp_size<bases>::value == 0) {
        if constexpr (policy::template has_facet<policies::indirect_vptr>) {
            obj->boost_openmethod_vptr = &policy::template static_vptr<To>;
        } else {
            obj->boost_openmethod_vptr = policy::template static_vptr<To>;
        }
    } else {
        update_vptr_bases<bases>::template fn<To, Class>(obj);
    }
}

struct with_vptr_derived {};

template<class, class, bool>
class with_vptr_aux;

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic ignored "-Wnon-template-friend"
#endif

template<class Class, class Policy>
class with_vptr_aux<Class, Policy, true> {
  protected:
    template<class To, class Other>
    friend void update_vptr(Other*);
    friend auto boost_openmethod_policy(Class*) -> Policy;
    friend auto boost_openmethod_bases(Class*) -> mp11::mp_list<>;

    with_vptr_aux() {
        (void)&use_classes<Class, Policy>::instance;
        detail::update_vptr<Class>(static_cast<Class*>(this));
    }

    ~with_vptr_aux() {
        boost_openmethod_vptr = nullptr;
    }

    friend auto boost_openmethod_vptr(const Class& obj) -> vptr_type {
        if constexpr (Policy::template has_facet<policies::indirect_vptr>) {
            return *obj.boost_openmethod_vptr;
        } else {
            return obj.boost_openmethod_vptr;
        }
    }

    friend auto boost_openmethod_policy(Class*) -> Policy;

    std::conditional_t<
        Policy::template has_facet<policies::indirect_vptr>, const vptr_type*,
        vptr_type>
        boost_openmethod_vptr = nullptr;
};

template<class Class, class Base>
class with_vptr_aux<Class, Base, false> : with_vptr_derived {
  protected:
    friend void update_vptr(Class*);

    with_vptr_aux() {
        (void)&use_classes<Class, Base, with_vptr_policy<Class>>::instance;
        detail::update_vptr<Class>(static_cast<Class*>(this));
    }

    ~with_vptr_aux() {
        detail::update_vptr<Base>(
            static_cast<Base*>(static_cast<Class*>(this)));
    }

    friend auto boost_openmethod_bases(Class*) -> mp11::mp_list<Base>;
};

} // namespace detail

template<typename...>
struct with_vptr;

template<class Class>
struct with_vptr<Class>
    : detail::with_vptr_aux<Class, BOOST_OPENMETHOD_DEFAULT_POLICY, true> {};

template<class Class, class Other>
struct with_vptr<Class, Other>
    : detail::with_vptr_aux<Class, Other, detail::is_policy<Other>> {};

template<class Class, class Base1, class Base2, class... MoreBases>
class with_vptr<Class, Base1, Base2, MoreBases...> : detail::with_vptr_derived {

    static_assert(
        !detail::is_policy<Base1> && !detail::is_policy<Base2> &&
            (!detail::is_policy<MoreBases> && ...),
        "policy can be specified only for root classes");

  protected:
    with_vptr() {
        (void)&use_classes<
            Class, Base1, Base2, MoreBases...,
            detail::with_vptr_policy<Base1>>::instance;
        detail::update_vptr<Class>(static_cast<Class*>(this));
    }

    ~with_vptr() {
        auto obj = static_cast<Class*>(this);
        detail::update_vptr<Base1>(static_cast<Base1*>(obj));
        detail::update_vptr<Base2>(static_cast<Base2*>(obj));
        (detail::update_vptr<MoreBases>(static_cast<Base2*>(obj)), ...);
    }

    friend auto boost_openmethod_policy(Class*)
        -> detail::with_vptr_policy<Base1>;
    friend auto boost_openmethod_bases(Class*)
        -> mp11::mp_list<Base1, Base2, MoreBases...>;
    friend auto boost_openmethod_vptr(const Class& obj) -> vptr_type {
        return boost_openmethod_vptr(static_cast<const Base1&>(obj));
    }
};

} // namespace boost::openmethod

#endif // BOOST_OPENMETHOD_WITH_VPTR_HPP
