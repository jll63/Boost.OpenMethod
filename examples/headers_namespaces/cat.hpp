// felines.hpp

#ifndef FELINES_HPP
#define FELINES_HPP

#include "animal.hpp"

namespace felines {

struct Cat : animals::Animal {
    using Animal::Animal;
};

}

#endif // FELINES_HPP
