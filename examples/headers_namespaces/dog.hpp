#ifndef CANINES_HPP
#define CANINES_HPP

#include <iosfwd>
#include <boost/openmethod.hpp>

#include "animal.hpp"

namespace canines {

struct Dog : animals::Animal {
    using Animal::Animal;
};

} // namespace canines

#endif // CANINES_HPP
