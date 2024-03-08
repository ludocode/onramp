/*
 * Declaration specifiers can appear in any order and redundant typedefs are
 * allowed in C as long as the types exactly match.
 *
 * Since `short` and `short int` are the same thing, and `signed` is implied as
 * long as there's no `unsigned`, all of these declarations mean exactly the
 * same thing. A proper C compiler is required to compile this without error.
 */

int short signed typedef foo;
int short typedef signed foo;
int signed short typedef foo;
int signed typedef short foo;
int typedef short signed foo;
int typedef signed short foo;
short int signed typedef foo;
short int typedef signed foo;
short typedef int signed foo;
short typedef signed int foo;
short signed typedef int foo;
short signed int typedef foo;
signed short int typedef foo;
signed short typedef int foo;
signed typedef short int foo;
signed typedef int short foo;
signed int typedef short foo;
signed int short typedef foo;
typedef signed int short foo;
typedef signed short int foo;
typedef short signed int foo;
typedef short int signed foo;
typedef int short signed foo;
typedef int signed short foo;

signed typedef short foo;
signed short typedef foo;
typedef signed short foo;
typedef short signed foo;
short signed typedef foo;
short typedef signed foo;

int typedef short foo;
int short typedef foo;
typedef int short foo;
typedef short int foo;
short int typedef foo;
short typedef int foo;

typedef short foo;
short typedef foo;

/*
 * We try to use foo just to make sure it works.
 */

typedef foo bar;
typedef foo bar;
foo typedef bar;

foo a;
foo b;
bar c;
bar d;

int main(void) {
}
