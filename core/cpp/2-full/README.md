# Onramp Preprocessor -- Final Stage

This is the final stage Onramp C preprocessor. It implements all necessary C preprocessor features and various popular extensions.

It is written in C. It requires `long long` arithmetic and function pointers (among other things) so it requires the [final stage compiler](../../cci/2-full/) to be bootstrapped. It does not depend on any advanced preprocessor features however so it can be bootstrapped with the [previous stage preprocessor](../1-omc/).

The preprocessor is a work in progress. We intend to support all of C17, most of C23, and most GNU extensions, especially those extensions not commonly implemented in other compilers. For example, preprocessor directives inside macro arguments are supported.

Like all Onramp stages, the preprocessor uses as little memory as possible. It interns all strings and reference counts everything, aggressively freeing memory as soon as it is no longer needed. It rarely buffers tokens, instead parsing directly from the lexer wherever possible. It also keeps a limited number of files in memory, closing and re-opening include files where needed.

The parsing logic is mainly implemented in these files:

- `preprocess.c` handles the main preprocessor loop and the include file stack;
- `directive.c` parses preprocessor directives;
- `expression.c` evaluates `#if` expressions;
- `macro.c` contains the macro expansion algorithm.



## Lexer

The preprocessor's lexer is somewhat different than that of the compiler.

The most notable difference is the addition of whitespace token types: (horizontal) space and (vertical) newline. This is necessary in cases where whitespace matters. For example `#define FOO()` is very different from `#define FOO ()`. Whitespace is coalesced into either a space or newline token depending on whether it contained a line ending. Comments are converted to space tokens. These tokens do not have values; the contents of whitespace and comments are discarded.

The lexer keeps track of when a token appears at the start of a line. If a `#` appears at the start of a line, it's given a token type of "directive" rather than "punctuation". This ensures that preprocessor directives are only recognized at the start of lines.

The lexer keeps a small amount of state in order to process angle-bracketed include files properly. If a `#` appears at the start of a line, and it is followed by (optional whitespace and) `include`, any `<` on the rest of the line is parsed as a string literal ending in `>`. This allows arbitrary spaces, escape sequences (with the addition of `\>`), and any other typical string syntax to occur in angle-bracketed includes.

(Note that this logic does not apply when the angle-bracketed include is expanded from macros. The preprocessor has additional machinery to work around this but there are several limitations. A warning is printed in this case.)



## Directive Parser

The preprocessor's directive parsing is mostly non-recursive. This is necessary to properly support certain extensions. For example, GNU C compilers not only allow directives inside macro arguments, they even allow them to be interleaved with arguments. Like this:

```c
#define FOO(x) x
FOO(
    #ifdef BAR
    1)
    #else
    2)
    #endif
```

GCC and Clang accept this so Onramp does as well. This precludes a recursive directive parser. Instead, when a directive branch is true, the branch is stored on a stack and the directive parser backs out to the caller. This allows macro expansion to proceed normally even with interleaved directives.



## Macro Expansion

When a macro is encountered, this preprocessor expands its expansion string from right to left. This allows our expansion buffer to consist of a simple vector that we use as a stack. Each token is simply pushed onto the stack from right to left as it is expanded. Once the expansion is complete, the top token on the stack is the left-most token, so we just pop it off the top and continue processing.

In retrospect this may have been a bad idea. It complicates certain things because we need to look backwards quite a bit. For example, for every alphanumeric token, we need to check if it's preceded not only by `defined`, but also by `defined` and `(`, both of which inhibit expansion. By contrast when scanning forward we'd just have to look for `defined` and set a flag or whatever.

The algorithm would probably be simpler to understand if we kept everything in left-to-right order. The simplest way would be to expand left to right into a temporary vector and then push the contents of the vector into our token stream from right to left. The small cost in performance would be worth it for the simplicity.

Alternatively, we may just need to pre-compute how each token should be expanded. This could be done left-to-right when the macro is defined, that way when expanding right-to-left we don't need to do any scanning forward or backwards.



## Memory Management

All tokens are reference counted, as are all strings by the [libo](../../libo/1-opc/) intern string table. We aggressively free all memory as soon as it is no longer used.

We buffer as few tokens as possible, within reason. One exception is that we macro-expand and buffer all tokens for an `#if` expression instead of streaming them. Generally speaking though, a token is only parsed by the lexer when it is immediately needed. This minimizes memory usage, allowing Onramp to run on extremely memory-constrained systems.

The lexer maintains a stack of open files. If the stack becomes too deep, it can close intermediate include files in order to limit the total number of open files. It will reopen and seek the file when needed to resume parsing from it. This allows running on a system that supports a limited number of open files. As an exception, the input file is never closed because it may be a stream.

Macros are also reference counted even though they are owned by the macro table. This is because of a quirk of GNU extensions. We support directives inside macro arguments (as mentioned earlier), which means it is possible to do this:

```c
// Don't crash!
#define FOO(x) x
FOO(
    #undef FOO
)
```

The macro expansion takes a strong reference to the macro so even though it becomes undefined, it is not freed until the end of the expansion.
