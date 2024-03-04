#define A(x, y) x B
A(
    #define B 1
    ,
    #define B 2
)
