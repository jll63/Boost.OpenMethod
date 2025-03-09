// Copyright (c) 2018-2025 Jean-Louis Leroy
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// tag::main[]

// dl_main.cpp

#include <cstring>
#include <iostream>
#include <dlfcn.h>
#include <unistd.h>

#include <boost/openmethod.hpp>
#include <boost/openmethod/unique_ptr.hpp>
#include <boost/openmethod/compiler.hpp>

#include "dl.hpp"

BOOST_OPENMETHOD_CLASSES(Animal, Herbivore, Cow, Wolf, Carnivore, dynamic_policy);

BOOST_OPENMETHOD_OVERRIDE(
    encounter, (dyn_vptr<Animal>, dyn_vptr<Animal>), std::string) {
    return "ignore\n";
}

// end::main[]

// tag::before_dlopen[]
int main() {
    using namespace boost::openmethod;

    initialize<dynamic_policy>();

    std::cout << "Before loading library\n";

    auto gracie = make_unique_virtual<Cow, dynamic_policy>();
    // Wolf _willy;
    // auto willy = virtual_ptr<Wolf, dynamic_policy>(_willy);
    auto willy = make_unique_virtual<Wolf, dynamic_policy>();

    std::cout << "Gracie encounters Willy -> "
              << encounter(gracie, willy); // ignore
    std::cout << "Willy encounters Gracie -> "
              << encounter(willy, gracie); // ignore
    // end::before_dlopen[]

    // tag::dlopen[]
    char dl_path[4096];
    dl_path[readlink("/proc/self/exe", dl_path, sizeof(dl_path))] = 0;
    *strrchr(dl_path, '/') = 0;
    strcat(dl_path, "/libdl_shared.so");
    void* handle = dlopen(dl_path, RTLD_NOW);

    if (!handle) {
        std::cerr << "dlopen() failed: " << dlerror() << "\n";
        exit(1);
    }

    std::cout << "\nAfter loading library\n";

    boost::openmethod::initialize<dynamic_policy>();

    auto make_tiger =
        reinterpret_cast<Animal* (*)()>(dlsym(handle, "make_tiger"));

    if (!make_tiger) {
        std::cerr << "dlsym() failed: " << dlerror() << "\n";
        exit(1);
    }

    std::cout << "Willy encounters Gracie -> "
              << encounter(willy, gracie); // hunt

    {
        auto hobbes = std::unique_ptr<Animal>(make_tiger());
        std::cout << "Gracie encounters Hobbes -> "
                  << encounter(gracie, *hobbes); // run
    }
    // end::dlopen[]

    // tag::after_dlclose[]
    dlclose(handle);

    std::cout << "\nAfter unloading library\n";

    boost::openmethod::initialize<dynamic_policy>();

    std::cout << "Gracie encounters Willy -> "
              << encounter(gracie, willy); // ignore
    std::cout << "Willy encounters Gracie -> "
              << encounter(willy, gracie); // ignore
    // end::after_dlclose[]
    return 0;
}
