#define A(x, y) y B
A(
    #define B 1
    ,
    #define B 2
)
