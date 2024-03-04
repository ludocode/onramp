#define A(x, y) B x y
A(
    #define B 1
    ,
    #define B 2
)
