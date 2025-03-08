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
