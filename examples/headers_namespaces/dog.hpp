#ifndef CANINES_HPP
#define CANINES_HPP

#include <iosfwd>
#include <boost/openmethod.hpp>

#include "animal.hpp"

namespace canines {

struct Dog : animals::Animal {
    using Animal::Animal;
};

BOOST_OPENMETHOD_INLINE_OVERRIDE(
    poke, (std::ostream & os, virtual_ptr<Dog> dog), void) {
    os << dog->name << " barks";
}

} // namespace canines

#endif // CANINES_HPP
