// This backslash is followed by a line feed but not a subsequent line.
// TinyCC disagrees with other compilers on this test. It emits the backslash
// on its own (which to me seems incorrect.)
// GCC seems to handle this the best: it's the only compiler I've found that
// will warn about this.
// Clang, MSVC, chibicc and Kefir eliminate it without warning.
\
