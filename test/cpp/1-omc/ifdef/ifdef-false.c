#ifdef foo
a
#endif
#ifdef bar


// test of line numbers in untaken conditional branches


b



// test of token pasting in untaken branches
foo ## bar
#define x foo ## bar



// test of invalid preprocessor directives in untaken conditional branches
#foo(#{}%${<
#


#endif
c
