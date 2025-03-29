# Boost.OpenMethod

THIS IS NOT A BOOST LIBRARY (yet).

The content of this repository is derived from YOMM2. It has been adapted to
Boost naming conventions for the purpose of being reviewed for inclusion in the
Boost C++ libraries.

The documentation is [here](https://jll63.github.io/Boost.OpenMethod/).

You can experiment with the library on Compiler Explorer by including
`<https://jll63.github.io/Boost.OpenMethod/boost/openmethod.hpp>`. It also
includes the headers for the compiler, `shared_ptr` and `unique_ptr`. For
example, here is the last iteration of the [AST
example](https://godbolt.org/z/cPjzfanc8) from the tutorial. Don't forget to
turn on optimizations (`-O2 -DNDEBUG`) and to select the Boost library.

## OpenMethod vs Boost.TypeErasure

OpenMethod and TypeErasure exist in the same space: runtime polymorphic systems
that offer a solution to the Expression Problem.

Thanks to BOOST_TYPE_ERASURE_FREE, we can add operations to existing classes,
without the need to modify them. And of course we can add new classes to
existing operations, by providing the required functions on them.

However, we quickly hits a wall, and this is related to the fact that in B.TE
polymorphism is imbued to a "magic" handle, `any`, whereas in OpenMethod,
polymorphism is attached to the object itself.

Let's look at an example. Consider a matrix library, which contains several
matrix subtypes (let's limit ourselves to "ordinary" and "symmetric") and
operations (let's just focus on transposition).

Here is (what I believe is) a plausible design:

```c++
#include <boost/mpl/vector.hpp>
#include <boost/type_erasure/any.hpp>
#include <boost/type_erasure/free.hpp>
#include <boost/type_erasure/typeid_of.hpp>
#include <boost/core/demangle.hpp>
#include <iostream>

namespace mpl = boost::mpl;
using namespace boost::type_erasure;

BOOST_TYPE_ERASURE_FREE(transpose);

struct ordinary_matrix_impl {
    virtual ~ordinary_matrix_impl() { /* ... */ }
    std::size_t rows, cols;
    std::vector<double> elements; // rows x cols doubles
};

struct symmetric_matrix_impl {
    virtual ~symmetric_matrix_impl() { /* ... */ }
    std::size_t rows; // cols == rows
    std::vector<double> elements; // rows doubles
};

struct matrix {
    any<mpl::vector<copy_constructible<>, has_transpose<matrix(_self&)>, typeid_<>>> impl;
};

auto transpose(matrix m) {
    return transpose(m.impl);
}

auto transpose(ordinary_matrix_impl& m) {
    return matrix{ordinary_matrix_impl()};
}

auto transpose(symmetric_matrix_impl& m) {
    return matrix{m};
}

auto main() -> int {
    {
        matrix m{ordinary_matrix_impl()};
        auto t = transpose(m);
        std::cout << boost::core::demangle(typeid_of(t.impl).name()) << "\n";
            // ordinary_matrix_impl
    }

    {
        matrix m{symmetric_matrix_impl()};
        auto t = transpose(m);
        std::cout << boost::core::demangle(typeid_of(t.impl).name()) << "\n";
            // symmetric_matrix_impl
    }

    return 0;
}
```

([Compiler Explorer](https://godbolt.org/z/613Eo5zGP)

We were able to add new behavior to existing classes (the "impl" classes)
without needing to modify them. This is promising.

Here is the equivalent using open-methods:

```c++
```



We can also add serialization to JSON:
