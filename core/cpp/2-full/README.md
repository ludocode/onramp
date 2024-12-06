# Onramp Preprocessor -- Final Stage

This is the final stage Onramp C preprocessor. It implements all necessary C preprocessor features and various popular extensions.

It is written in C. It requires `long long` arithmetic and function pointers (among other things) so it requires the [final stage compiler](../../cci/2-full/) to be bootstrapped. It does not depend on any advanced preprocessor features however so it can be bootstrapped with the [previous stage preprocessor](../1-omc/).

The preprocessor is a work in progress. We intend to support all of C17, most of C23, and most GNU extensions, especially those extensions not commonly implemented in other compilers. For example, preprocessor directives inside macro arguments are supported.

Like all Onramp stages, the preprocessor uses as little memory as possible. It interns all strings and reference counts everything, aggressively freeing memory as soon as it is no longer needed. It rarely buffers tokens, instead parsing directly from the lexer wherever possible. It also keeps a limited number of files in memory, closing and re-opening include files where needed.



## Lexer

The preprocessor's lexer is somewhat different than that of the compiler.

The most notable difference is the addition of whitespace token types: (horizontal) space and (vertical) newline. This is necessary in cases where whitespace matters. For example `#define FOO()` is very different from `#define FOO ()`. Whitespace is coalesced into either a space or newline token depending on whether it contained a line ending. Comments are converted to space tokens. These tokens do not have values; the contents of whitespace and comments are discarded.

The lexer keeps track of when a token appears at the start of a line. If a `#` appears at the start of a line, it's given a token type of "directive" rather than "punctuation". This ensures that preprocessor directives are only recognized at the start of lines.

The lexer keeps a small amount of state in order to process angle-bracketed include files properly. If a `#` appears at the start of a line, and it is followed by (optional whitespace and) `include`, any `<` on the rest of the line is parsed as a string literal ending in `>`. This allows arbitrary spaces, escape sequences (with the addition of `\>`), and any other typical string syntax to occur in angle-bracketed includes.

(Note that this logic does not apply when the angle-bracketed include is expanded from macros. The preprocessor has additional machinery to work around this but there are several limitations. A warning is printed in this case.)



## Parser

The preprocessor's parser is mostly non-recursive. This is necessary to properly support certain extensions. For example, GNU C compilers not only allow directives inside macro arguments, they even allow them to be interleaved with arguments. Like this:

```c
#define FOO(x) x
FOO(
    #ifdef BAR
    1)
    #else
    2)
    #endif
```

GCC and Clang accept this so Onramp does as well. This precludes a recursive directive parser.

The parsing logic is mainly implemented in these files:

- `preprocess.c` handles the main preprocessor loop and the include file stack;
- `directive.c` parses preprocessor directives;
- `expression.c` evaluates `#if` expressions;
- `macro.c` contains the macro expansion algorithm.
