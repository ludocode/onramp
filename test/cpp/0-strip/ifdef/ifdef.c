// we're stripping preprocessor directives so it is as though
// both conditions are true.
#ifdef foo
a
#endif
#ifndef foo
b
#endif
