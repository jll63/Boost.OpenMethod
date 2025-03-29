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

At first sight, OpenMethod and TypeErasure exist in the same space: runtime
polymorphic systems that offer a solution to the Expression Problem.

Thanks to BOOST_TYPE_ERASURE_FREE, one can add operations to existing classes,
without the need to modify them. And of course one can add new classes to
existing operations, by providing the required functions on them. However, one
quickly hits a wall, and this is related to the fact that in B.TE polymorphism
is imbued to a "magic" handle, `any`, whereas in OpenMethod, polymorphism is
attached to the object itself.

To make this more concrete, let's look at an example. Consider a matrix library,
which contains several matrix subtypes (let's limit ourselves to "ordinary" and
"symmetric") and operations (let's just focus on transposition).

Here is what I believe to be a plausible design:

```c++
#include <boost/mpl/vector.hpp>
#include <boost/type_erasure/any.hpp>
#include <boost/type_erasure/free.hpp>
#include <iostream>

namespace mpl = boost::mpl;
using namespace boost::type_erasure;

BOOST_TYPE_ERASURE_FREE(transpose);

struct ordinary_matrix_impl {
    ordinary_matrix_impl() {
        std::cout << "ordinary_matrix_impl\n";
    }
};

struct symmetric_matrix_impl {
    symmetric_matrix_impl() {
        std::cout << "symmetric_matrix_impl\n";
    }
};

struct matrix {
    any<mpl::vector<copy_constructible<>, has_transpose<matrix(_self&)>>> impl;
};

auto transpose(matrix m) {
    return transpose(m.impl);
}

auto transpose(ordinary_matrix_impl& m) {
    std::cout << "transpose ordinary matrix\n";
    return matrix{ordinary_matrix_impl()};
}

auto transpose(symmetric_matrix_impl& m) {
    std::cout << "transpose symmetric matrix\n";
    return matrix{m};
}

auto main() -> int {
    {
        matrix m{ordinary_matrix_impl()};
        auto t = transpose(m);
    }

    {
        matrix m{symmetric_matrix_impl()};
        auto t = transpose(m);
    }

    return 0;
}
```
<iframe width="800px" height="200px" src="https://godbolt.org/e?readOnly=true#z:OYLghAFBqd5QCxAYwPYBMCmBRdBLAF1QCcAaPECAMzwBtMA7AQwFtMQByARg9KtQYEAysib0QXACx8BBAKoBnTAAUAHpwAMvAFYTStJg1DIApACYAQuYukl9ZATwDKjdAGFUtAK4sGe1wAyeAyYAHI%2BAEaYxCAArNIADqgKhE4MHt6%2BekkpjgJBIeEsUTHxtpj2eQxCBEzEBBk%2BflzllWk1dQQFYZHRcdIKtfWNWS2Dnd1FJf0AlLaoXsTI7BzmAMzByN5YANQma24RqMkEAPQsCbSnAG6YDiQAdAgJCfvYJhoAguub25h7ByOJ1OBAAnglMAB9aJMBSLTCnQygp4vN4fb5mDYMLZeXb7Q7HQYg8FQmFw4gIqgUzAo15rd5fH7Yv4AtxOQYU1hoxlfZhsBQJJjLHYXWgAgAiOyBgxAIFF%2BysXy8KSMOz5mAFQv%2B0oIsrBEOhxFh8IV6PRFgA8hahAAVSE2gCaymwkOwACVPkI5G6XQAxH3YCAEI0MAXJTAzU08z4crwOHYkfDMYigyEsJjBvCqSF4UV7ADsis%2BOxLCeISbqqfTmezucuEBmBaLpZbO0G6FlaC8BFZ%2BL2ZjMieClbTGeIWZz8tibgY5jMUeLpZM%2BfFZpXC/RsfjClBLDYmeQo5rk8uTfRLZ3e8wB6P49roobZ6%2BrdL7c7Cx7%2BL7c8v%2B/Hh%2BrO8T1oExp1nAcFxbZdV0Zdc1iLTdgzjHtAKzJ9FxLJF8VFWVbnuYh8TQBJUzQUMkIcPAInoL96VIHYEFhSFg0MMMlGwscswgSE7CocwADZI3pNF6R2OsQPgtdVwk6MmG7VAdmY0MckwCBUNUEVG2XZtSwpAhFgYBSQ1YlSWAeMTBIQ9cZLkwyWOUiAh2TKsOPvS5%2BI09CLwIDsUA/XsDn7MxFOMssKxTEUXNAmc50gnTr30iKay0xyRzU4CG2gjcrO%2BL5ZKIWylPDCBf2vf9bwnMT3JYTTC3PV9vPfbt/LcQLguUttdz/PBkESu8ovA%2BdpIwnZdIStStJYTKhsy6z8vTYJHwAWjeUTBE8pdaufF9erQybCxSlNytc2gMrg7SXzy%2BTPzWSU2qK6rYpLGacuGrS6pfNSRS0kqbzSsTTqk87W0uhSJQK4zVIs96Cxgl6W1G4gDI0LLYY4OZaE4WJeD8DgtFIVBODcaxrDbBYln%2BdYeFIXVcbRuYAGs4g0B4zAADjMWIBzMABONY%2BNiLguFifROEkbHNF4AmOF4BQQA0amJbmOBYBgRBfIuOhonISg0A1%2BgYi2QxgCY4gvAYem%2BDoAholliAIgl0gImHFNOCpp2nItCJtDuGmqd1thBAtBhaFBB2sAiLxgDcMRaFl7heCwdMjHEWnSHwCkKNuOO8cwVQ7m7FY8eCa2MdT2hKKNFMPCwB3MxYV3eFuYgjiUcVMCT4By6MRW%2BAMYAFAANTwTAAHcLQhHGqf4QQRDEdgpBkQRFBUdRU90FoDG70xLGsfRKNl2B1RAHVSCb13WfzOn8YSKo48lpvxywA%2BG1aH20hcBh3E8Jp/E/yZehiC0HIqQBAjGaKQYBVR/7FD6GMCob8BAdGGN/UYr8KKIKGF0YIPQYGANsJgsBehxj1GgdMLgcwFBk2WBIdGmNxapyljsVQrM%2BKLT4pIHYhtVRBlNubRsEBcCEBIP2NY5DeA0y0DMBmIBJDcweLEfMfEzBcH5rEbmnM1g8xFhwMWpB675jkazSQsi%2BJcDMJIDQoiNAaHzKQHGeMpYyzlgrWmStVYQCQB%2BBI3ZtYQF1gkTWxBQisBWMw1h7CdjAGQD1HhZt6YzF4JgfARBH56GnsIUQ4gF7pOXmoB269SAjyNAkButCOBYzsQ7KWFpuzeJ7KgKgTCWFsI4Vw4AOxYl8I6R4PW0QRFiJcZIuYCBMBMCwDEF%2BpddH1zMMzIWsRLEXzUXxVm3NuZmD4pUhhnAnHywkVfRmsiHj5i4PmSQ%2BYLkaD4msSQrMWHaLWPQhxOzBlX1LmYJ5ksXn7KkafG279JBAA%3D%3D"></iframe>

([Compiler Explorer](https://godbolt.org/z/hWoM6jWdd))

However, there are limitations to the B.TE way

There are some major differences though, and
