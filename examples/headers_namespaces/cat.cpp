// cat.cpp

#include <iostream>
#include <boost/openmethod.hpp>

#include "cat.hpp"

namespace felines {

BOOST_OPENMETHOD_CLASSES(animals::Animal, Cat);

BOOST_OPENMETHOD_OVERRIDE(
    poke, (std::ostream & os, virtual_ptr<Cat> cat), void) {
    os << cat->name << " hisses";
}

}
