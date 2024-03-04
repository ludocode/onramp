#define A(x) # x
A(a)
#define B(x) #/*hello*/x
B(b)
#define C(x) # /*hello*/ /*world*/ x
C(c)
