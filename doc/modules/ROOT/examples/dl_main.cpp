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
#include <boost/openmethod/initialize.hpp>

#include "dl.hpp"

BOOST_OPENMETHOD_CLASSES(
    Animal, Herbivore, Cow, Wolf, Carnivore, dynamic);

BOOST_OPENMETHOD_OVERRIDE(
    encounter, (dyn_vptr<Animal>, dyn_vptr<Animal>), std::string) {
    return "ignore\n";
}

// end::main[]

// tag::before_dlopen[]
auto main() -> int {
    using namespace boost::openmethod;

    dynamic::initialize();

    std::cout << "Before loading library\n";

    auto gracie = make_unique_virtual<Cow, dynamic>();
    // Wolf _willy;
    // auto willy = virtual_ptr<Wolf, dynamic>(_willy);
    auto willy = make_unique_virtual<Wolf, dynamic>();

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

    dynamic::initialize();

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

    dynamic::initialize();

    std::cout << "Gracie encounters Willy -> "
              << encounter(gracie, willy); // ignore
    std::cout << "Willy encounters Gracie -> "
              << encounter(willy, gracie); // ignore
    // end::after_dlclose[]
    return 0;
}
