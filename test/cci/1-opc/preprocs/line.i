#line 25 "stdio.h"
int puts(const char* s);
#line 8 "foo.c"
int main(void) {
#pragma onramp file push
#line 12 "bar.c"
puts("Hello world!");
#pragma onramp file pop
#line 15 "foo.c"
}
