#include <iostream>
#include<boost/openmethod.hpp>

#include "dog.hpp"

namespace canines {

BOOST_OPENMETHOD_CLASSES(animals::Animal, Dog);

BOOST_OPENMETHOD_OVERRIDE(
    poke, (std::ostream & os, virtual_ptr<Dog> dog), void) {
    os << dog->name << " barks";
}

}
