#include <iostream>
#include <boost/openmethod.hpp>
#include <boost/openmethod/compiler.hpp>

#include "animal.hpp"
#include "cat.hpp"
#include "dog.hpp"

struct Bulldog : canines::Dog {
    using Dog::Dog;
};

BOOST_OPENMETHOD_CLASSES(canines::Dog, Bulldog);

BOOST_OPENMETHOD_OVERRIDE(
    poke, (std::ostream & os, virtual_ptr<Bulldog> dog), void) {
    next(os, dog);
    os << " and bites back";
}

auto main() -> int {
    boost::openmethod::initialize();

    std::unique_ptr<animals::Animal> felix(new felines::Cat("Felix"));
    std::unique_ptr<animals::Animal> snoopy(new canines::Dog("Snoopy"));
    std::unique_ptr<animals::Animal> hector(new Bulldog("Hector"));

    poke(std::cout, *felix); // Felix hisses
    std::cout << ".\n";

    poke(std::cout, *snoopy); // Snoopy barks
    std::cout << ".\n";

    poke(std::cout, *hector); // Hector barks and bites
    std::cout << ".\n";

    return 0;
}
