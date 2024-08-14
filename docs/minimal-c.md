# Onramp Minimal C

Onramp Minimal C (omC) is a tiny subset of C. It is implemented by Onramp's [first stage compiler (cci/0-omc)](../core/cci/0-omc/) and [second stage preprocessor (cpp/1-omc)](../core/cpp/1-omc/).

It contains the minimum number of C features required to reasonably implement the [second stage compiler (cci/1-opc)](../core/cci/1-opc/) and other Onramp tools. It's designed to be simple enough for its compiler to be implemented in assembly, but powerful enough to be used to implement most of ANSI C (i.e. [Onramp Practical C](practical-c.md).) It is a critical step in the jump from assembly to a full implementation of C.

Its features are, in brief:

- `void`, `int` and `char` types, and pointers to them
- `if`, `while`, `continue`, `break` and `return` for control flow
- `typedef` type names, and `extern` and `static` global variable declarations
- String, character, and integer constants
- All of the arithmetic and comparison operators in C
- Functions with up to four parameters
- Local variables with initializers and simple assignments (`=`)
- Arbitrary nesting of blocks and expressions
- Mixed declarations and code
- `#include`, `#ifdef`/`#ifndef`, and object-like `#define`
- `/*C*/` and `//C++` style comments

omC supports enough of C to define libc-compatible headers. This requires `void` and `void*`, `extern` for global variables, and `typedef` for `FILE` and `size_t` (it's actually harder to bootstrap these with `#define`.) The support for `typedef` and file inclusion in particular allows for the construction of large scale projects with idiomatic code organization in omC.

If you've ever been interested in writing a compiler or interpreter, omC is a great place to start. There is a full [grammar specification](#grammar) below; a heavily documented [reference implementation](../core/cci/0-omc/); a large [battery of tests](../test/cci/0-omc/) with scripts to run them; a set of C-compatible [standard library headers](../core/libc/0-oo/include/); and some interesting programs such as an [eight queens solver](../test/cci/0-omc/programs/eight-queens.c), not to mention many of the stages of Onramp itself. (You don't even need to write the preprocessor either because, once your compiler works, you could compile and use Onramp's preprocessor!)

omC is quite powerful, yet it is simple enough to be implemented in about a thousand lines of your favorite programming language (or a few thousand lines of assembly.) It is also easy to extend with additional features from C.



## History

omC is analogous to the short-lived [NB or "New B"](https://www.bell-labs.com/usr/dmr/www/chist.html), an extension of [B](https://en.wikipedia.org/wiki/B_(programming_language)). NB essentially just added the `int` and `char` types to the typeless B. This paved the way for struct support and other features which eventually became K&R C. omC is inspired by the way C was originally bootstrapped by Dennis Ritchie more than five decades ago.

Unfortunately NB was lost. It went through several iterations, probably never having any concrete specification, and it existed so briefly that no version of it was preserved. This time, we are keeping the intermediate language around! We are implementing it in assembly and we will attempt to specify it here.

The main difference between NB and omC is, rather than making it an extension of B, omC is instead a proper subset of C. omC files can be compiled with any ordinary C99 or later compiler. This allows us to use modern tools to test and debug our code.



## Comparison with C

Compared to C, omC has:

- Only `void`, `int` and `char` types and pointers to them.
    - No `struct`/`union`/`enum`;
    - No arrays;
    - No `unsigned`, `long`, `short`;
    - No function pointers;
    - `const` is ignored;
    - Limited `typedef`.
- Only `if`, `while`, `continue`, `break` and `return` for control flow.
    - No `else`;
    - No `for` or `do` loops;
    - No labels or `goto`;
    - No `switch`/`case`;
    - Braces are required for `if` and `while`.
- Limited operators: only simple arithmetic and logic.
    - No compound assignment (`+=`, `&=`, etc.)
    - No `++`/`--`;
    - No logical `&&` or `||` (bitwise `&` and `|` must be used instead);
    - No ternary conditional `?:`, comma operator, and more;
    - `sizeof` only with parenthesized types, not expressions.
- No binary operator precedence. All binary expressions must be fully parenthesized!
- No variadic functions, and at most four function arguments.
- Only `#include`, `#ifdef`/`#ifndef`, and object-like `#define` preprocessor directives.

Despite these limitations, omC is a proper subset of C. It can be compiled (and tested!) with any C99 or later compiler.

We also support a few C99 features that were not in C89, namely mixed declarations and code and C++-style comments.



## Preprocessor

The omC preprocessor supports only a subset of the preprocessing directives in standard C:

- `#include`
- `#define` for non-recursive object-like macros
- `#ifdef`, `#ifndef` and `#endif` conditionals
- `#line`

The above features are enough to support header files with include guards and to do platform-based conditionals and substitutions (to test and debug on Linux for example.)

The following macros must be predefined by the preprocessor to identify the supported language when compiling omC:

- `__onramp_cpp_omc__`, declaring the supported preprocessor dialect
- `__onramp_cci_omc__`, declaring the supported compiler dialect

Additionally, the Onramp compiler driver defines the following macros:

- `__onramp__`, declaring the target platform
- `__onramp_cpp__`, declaring this as a stage of the Onramp preprocessor
- `__onramp_cci__`, declaring this as a stage of the Onramp compiler

Any unrecognized preprocessor directives are ignored in untaken conditional branches. You should therefore wrap any conventional preprocessor directives (like `#if`) in `#ifndef __onramp_cpp_omc__`. Most final-stage Onramp programs use this to support bootstrapping with the omC preprocessor and the opC compiler.



### Limitations

The omC preprocessor does not support function-like macros (i.e. `#define` with arguments); `#if` and friends; `#else`; `#undef`; and more. It also supports no built-in macros (like `__FILE__` or `__LINE__`), no modern features (like `__has_builtin`) and no extensions (like `__COUNTER__` or `#include_next`.)

The omC preprocessor, like the compiler, only supports the [ASCII](https://en.wikipedia.org/wiki/ASCII)-encoded [POSIX portable character set](https://en.wikipedia.org/wiki/Portable_character_set), excluding null, alert and backspace. Any characters outside of this range cause an error.

The omC preprocessor does not support recursive macros. For example:

```
#define FOO BAR
#define BAR FOO
FOO
```

Unlike standard C (which preprocesses this to `FOO`), in the omC preprocessor this is malformed. The omC preprocessor may diagnose it, or it may crash, go into an infinite loop or exhibit other undefined behaviour. This also means that you cannot define a macro to itself, which is otherwise legal C.

The `#` character can only appear at the start of a line, after optional whitespace and comments. Since `#` cannot appear anywhere else outside of a comment, the preprocessor is not required to detect and diagnose this. It may treat a `#` anywhere outside of a comment as the start of a preprocessor directive. Such code is nevertheless malformed and this behaviour should not be relied upon.



## Grammar

The below is a description of the grammar as parsed by the Onramp's omC preprocessor (cpp/1) and compiler (cci/0).

The syntax is similar to the style of parsing expression grammars (PEG):

- **bold** is used for token categories
- `monospace` is used for literal tokens or regular expressions
- _italics_ is used for the names of parsing expressions (i.e. non-terminals)
- parentheses are for grouping
- '/' is used for alternatives (the "choice" operator in PEG)
- \* means repetition
- \? means optional


### Preprocessor

omC supports C-style `/*comments*/` and C++-style `// line comments`. The preprocessor removes comments.

A preprocessor directive starts at the beginning of a line and runs until the end of the line. Whitespace and comments are allowed before and after all tokens (except in a _file-specifier_; see below.). The following directives are recognized:

_include-directive_: `#` `include` _file-specifier_

_define-directive_: `#` `define` **alphanumeric** _token_\*

_ifdef-directive_: `#` `ifdef` **alphanumeric**

_ifndef-directive_: `#` `ifndef` **alphanumeric**

_endif-directive_: `#` `endif`

_line-directive_: `#` `line` **decimal** _file-specifier_?

_empty-directive_: `#`

The include directive includes the content of the given file in place of the directive. A file specifier in `#include` and `#line` takes one of two forms:

_file-specifier_: ( `<` **filename** `>` ) / ( `"` **filename** `"` )

The form with `"` searches the current directory first; the form with `<` does not. They otherwise search the same set of include paths and behave identically. Note that whitespace and comments are not allowed in and around the filename.

**filename** is a special kind of token that is only parsed in the context of an include directive. It is essentially any non-whitespace character until the closing `"` or `>`. (This can be difficult to parse with a lexer as it is context-sensitive. Onramp's omC preprocessor is scannerless so this is not an issue.)


### Tokenization

The following tokens are recognized by the omC lexer. They are given here as regular expressions.

**alphanumeric**: `[A-Za-z_][0-9A-Za-z_]*`

**number**: **decimal** / **hexadecimal**

**decimal**: `[1-9][0-9]*`

**hexadecimal**: `0[xX][0-9A-Fa-f]*`

**string**: `"[^"]*"`

**character**: `'[^']'`

**punctuation**: `<<` / `>>` / `==` / `!=` / `<=` / `>=` / (any other non-whitespace character)

An alphanumeric token is either a keyword, an identifier or a type name. (Onramp does not use a "lexer hack"; it leaves it up to the parser to determine what an alphanumeric token is.)

Note that numbers are strictly decimal and hexadecimal integers. There is no support for numerical suffixes, floating point, or octal or binary representations.


### Globals

_file_: _global_\*

_global_: _specifier_ _type_ **alphanumeric** ( `;` / _function_)

_specifier_: `typedef` / `static` / `extern`

A file is composed of a sequence of globals. Each global is either a typedef, a variable or a function.

At most one specifier can be provided. If a specifier is provided, the global cannot be a function.

`typedef` makes the name an alias for the type.

`extern` declares a variable without emitting a definition. `static` declares a variable and emits a definition with internal linkage. If no specifier for a variable is provided, it is declared and emitted with external linkage. (There is no initializer for global variables; they are initialized to zero.)


### Functions

_function_: `(` ( `void` / _parameter-list_ ) `)` ( `;` / _compound-statement_ )

_parameter-list_: _parameter_ ( `,` _parameter_ )\*

_parameter_: _type_ **alphanumeric**?

A parameter list cannot be empty. If a function takes no parameters, it must be declared `(void)`. Functions can have at most four parameters.

Parameter names are optional. If a parameter name is omitted, the parameter's value is inaccessible.

If a function's parameter list is followed by a semicolon, it is a function declaration. Otherwise, it is a function definition, and the compound statement is the body of the function.


### Types

_type_: `const`? _base_ `const`? ( `*` `const`? )\*

_base_: `void` / `int` / `char`

A type consists of a base type followed by 0 to 15 pointer declarators. `const` is accepted and ignored anywhere before, after and in between.

Note that types are always parsed separately from identifiers. There is no support for any postfix or parenthesized declarators.

Types are simple enough to be represented in just six bits. cci/0 uses an additional bit as an l-value flag; see [cci/0](../core/cci/0-omc/) for details.


### Statements

_statement_: `;` / _compound-statement_ / _control-statement_ / _selection-statement_ / _expression-statement_

_compound-statement_: `{` ( _local-declaration_ / _statement_ )\* `}`

_control-statement_: ( `if` / `while` ) `(` expression `)` _compound-statement_

_selection-statement_: ( `break` / `continue` / ( `return` _expression_? ) ) `;`

_expression-statement_: _expression_ `;`

Statements are fairly self-explanatory. Compound statements create a scope for variables, and can have mixed declarations and statements. The value of an expression statement is ignored. `break` and `continue` statements jump to the end or to the condition check of the nearest containing `while` statement respectively.

The `return` statement exits the function returning the expression's value if provided. If the containing function returns `void`, a `return` statement must omit the expression; otherwise, it must provide an expression of the same type as the function return type.

All functions with a non-`void` return type must return via a `return` statement, except for `main()` which returns 0 if control reaches the end of the function.


### Declarations

_local-declaration_: _type_ **alphanumeric** ( `=` _expression_ )? `;`

A local declaration declares and defines a variable in the current compound statement. The variable ceases to exist at the end of the containing compound statement. While it exists, the variable shadows any previously defined variables or parameters with the same name.

An optional initializing expression can be provided. If it is, it must have the same type as the variable.


### Expressions

_expression_: _binary-expression_

_binary-expression_: _unary-expression_ ( _binary-operator_ _unary-expression_ )?

_unary-expression_: ( _unary-operator_ _unary-expression_ ) / _primary-expression_

_primary-expression_: **alphanumeric** / **number** / **string** / **character** / _sizeof-expression_ / _parenthesized-expression_

_sizeof-epxression_: `sizeof` `(` _type_ `)`

_parenthesized-expression_: `(` _expression_ `)`

A **string** expression has type `char*`. A **character** expression has type `int`. An **alphanumeric** expression has the type of and evaluates to the value of the most recently defined extant variable with a matching name.

The `sizeof` expression evaluates to the size of the given type. Note that it cannot take an expression as argument.

Note that multiple binary operators cannot be specified in one expression. They must be parenthesized.


### Unary Operators

_unary-operator_: _cast-operator_ / `+` / `-` / `!` / `~` / `*` / `&`

_cast-operator_: `(` _type_ `)`

These operators have the same meaning as they do in C.


### Binary Operators

_binary-operator_: _assignment-operator_ / _comparison-operator_ / _arithmetic-operator_

_assignment-operator_: `=`

_comparison-operator_: `==` / `!=` / `<` / `>` / `<=` / `>=`

_arithmetic-operator_: `+` / `-` / `*` / `/` / `^` / `<<` / `>>` / `&` / `|` / `^`

These operators have the same meaning as they do in C. The comparison operators evaluate to `0` or `1`.

Note that there is no binary operator precedence. All nested binary expressions must be fully parenthesized.

The `+` operator, when adding a value to a pointer, multiplies the value by the size of the pointed-to type. The `-` operator, when taking the difference of two pointers, divides the result by the size of the pointed-to type.
