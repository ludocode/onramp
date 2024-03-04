# Onramp Preprocessor -- First Stage

This is a minimal "preprocessor" that just strips comments and preprocessor directives. It doesn't really do much preprocessing because there is no handling of directives at all. They are simply removed, as if `#` is a line comment.

The sole purpose of this is to bootstrap the omC preprocessor.

This allows us to use standard `#include` directives so that the omC preprocessor can be compiled as ordinary C, while providing fallback libc declarations under `#ifdef __onramp_cpp_strip__`. This macro is never actually defined, but because this preprocessor strips the directives, it's equivalent to the condition being true.

This also allows us to use comments in the implementation of the omC preprocessor without having to support comments in the omC compiler. No version of the Onramp `cci` compiler supports comments; they are always stripped by the preprocessor.
