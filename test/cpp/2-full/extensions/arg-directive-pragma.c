// GCC outputs the pragmas in declared order.
// Clang gives an error about #pragma in macro arguments and refuses to output anything.
// MSVC emits nonsense (the #pragma lines are on the same line as the 'a' and, with the traditional preprocessor, the 'b')
// TinyCC emits the same nonsense as MSVC traditional
// Kefir, chibicc and NVidia strip the unknown pragmas.
#define FOO(x, y) y x
a FOO(
        #pragma X X
        ,
        #pragma Y Y
   ) b
