// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// tag::dl_shared[]
// dl_shared.cpp

#include <string>
#include <boost/openmethod.hpp>

#include "dl.hpp"

BOOST_OPENMETHOD_OVERRIDE(
    encounter, (dyn_vptr<Herbivore>, dyn_vptr<Carnivore>), std::string) {
    return "run\n";
}

struct Tiger : Carnivore {};

BOOST_OPENMETHOD_CLASSES(Tiger, Carnivore, dynamic_policy);

extern "C" auto make_tiger() -> Tiger* {
    return new Tiger;
}

BOOST_OPENMETHOD_OVERRIDE(
    encounter, (dyn_vptr<Carnivore>, dyn_vptr<Herbivore>), std::string) {
    return "hunt\n";
}
// end::dl_shared[]
