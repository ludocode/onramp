# Onramp Compiler -- Final Stage

The final stage Onramp compiler is simple and fairly traditional. It uses a handwritten recursive-descent parser, it generates a simple parse tree, and it compiles to basic blocks in memory. Minor optimizations are performed on the parse tree.

The compiler produces [Onramp Intermediate Representation](../../../docs/intermediate-representation.md), a simple assembly-like language. This is optimized and converted to assembly by the [code generator cg/1](../../cg/1-full/).

Despite its simplicity, we aim to implement most of C11 with many C23 features and many GNU and other extensions.

The compiler implements a command-line interface similar to GCC and friends, but extension usage causes errors by default. Pass `-fgnu-extensions` or a `-std=gnu*` mode to make it behave more like GCC. (When passed to the driver, these also define `__GNUC__`.) See the [Usage Guide](../../../docs/usage-guide.md) for details.

NOTE: The compiler has recently been converted in-place to generate [intermediate representation](../../../docs/intermediate-representation.md) instead of assembly. You may see lots of `#ifdef CCI2_IR` remaining in the code. The assembly generation will be removed once the IR is complete.



## Components

- `arithmetic` - Wrappers for `long long`, `float` and `double` math.
- `block` - A basic block of instructions, starting with a label and ending in a `br`, `jmp` or `ret`.
- `common` - Common utility code such as error handling functions.
- `emit` - Low-level functions for writing the output file: bytes and numbers, opcodes and temporaries, etc.
- `enum` - The container for an enum and its values.
- `function` - The container for a function. Contains its parse tree and its list of basic blocks.
- `generate` - Code generation. Converts the parse tree into basic blocks of instructions.
- `instruction` - A single instruction in a basic block.
- `lexer` - Converts the input stream into tokens.
- `node` - A node in a parse tree, along with functions to manipulate it.
- `options` - Command-line options, such as warnings and optimization flags.
- `parse` - The parser. Converts tokens from the lexer into a parse tree.
- `record` - The container for a struct or union and its members.
- `scope` - Scope handling functions. Owns records and contains hashtables for declared symbols and type names.
- `strings` - Global intern strings for all keywords and operators.
- `symbol` - A container for any kind of symbol: variables, functions, constants and builtins.
- `token` - A token, including its source location.
- `type` - One part of a type, either a declarator or a base, forming a graph that describes a complete type.



## Implementation Language

The final stage compiler is written in [opC](../../../docs/practical-c.md) with the [omC](../../../docs/minimal-c.md) preprocessor. The code mostly looks like C11 but there are some major limitations that affect its implementation.

opC only supports 32-bit integer values. It does not have `long long`, `float` or `double` so we can't use them in this implementation. Instead, `float` values are stored in a `uint32_t`, and the compiler has a type `u64_t` to store 64-bit `long long` and `double` values.

We define functions `llong_*()`, `double_*()` and `float_*()` for performing arithmetic. When bootstrapping, these are wrappers for the corresponding arithmetic functions in the Onramp libc. When the compiler rebuilds itself, or when compiling with a native compiler (e.g. when unit testing), these instead wrap the normal C operators.

opC does not have function pointers, and the omC preprocessor does not have function-like macros. In cases where these would have been useful, we have to do other workarounds instead. The code can end up being quite a bit more verbose than you might expect.

NOTE: The compiler (and its dependencies) are currently being transitioned to be built with cci/0 rather than cci/1 so that cci/1 can be deleted. This is introducing even more limitations; in particular, functions are limited to four parameters, and variadic arguments are not supported.



## Algorithm

The final stage compiler operates in phases on units of functions.

For each function in the input file, the following phases are performed:

- The function is parsed into a tree;
- An optional optimization pass runs on the tree;
- The tree is compiled into basic blocks of instructions;
- The complete function is emitted to the output file.

Global variable initializers are compiled as constructor functions where necessary so they also follow the above steps. See the Relative Relocations section below.

Everything is freed once the function is emitted, minimizing the total memory usage of the compiler.



## Lexer

The final stage compiler uses the [libo/1](../../libo/1-opc) intern string table for all strings. Checking strings for equality can be done by a simple pointer comparison. This makes token comparisons trivial.

All built-in tokens are added to the string table at startup. Functions like `lexer_expect()`, `lexer_accept()` and `lexer_is()` take an intern string as argument. This is much faster than previous stages which required a call to `strcmp()` for every token comparison.

The intern string table reference counts all strings. It can free strings that are unused and remove them from the table. Since we free every function after compiling it, memory usage is kept to a minimum throughout the run of the compiler.

The lexer does not support digraphs, trigraphs, comments, or preprocessor directives other than `#line` and `#pragma`. These are the responsibility of the preprocessor. The compiler is expected to always be run on the output of the preprocessor.



## Parse Tree

The parser parses code into a tree. We sometimes call it an abstract syntax tree or a parse tree but it's really a semantic tree, as most semantic properties are resolved as it builds the tree. For example the types of all nodes are resolved during parsing, not in a separate analysis pass. It also does not include some syntax such as types, enum declarations, etc.

You can pass `-ast-dump` to the compiler to view the tree. (By default the output uses UTF-8-encoded box-drawing characters. Pass `-ast-dump=ascii` to restrict the output to ASCII.) For example:

```c
int square(int x) {
    return x * x;
}
```

The above is parsed into:

```
FUNCTION `square` int
├─PARAMETER `x` int
└─SEQUENCE `{` void
  └─RETURN `return` int
    └─MUL `*` int
      ├─ACCESS `x` int
      └─ACCESS `x` int
```

The tree is based on expressions. Every node in the tree has a type, which is the result type of the expression it represents. If the node is a statement or a declaration, we still consider it an expression of type void.

This allows us to share node types for statements and expressions. For example, `NODE_IF` is used for both the `if` statement and the ternary conditional `?` operator. In the case of `if`, the node's type is `void`. In the case of `?`, the node's type is the type of its contained expressions. These are compiled in exactly the same way; the code generator does not know or care whether it was an `if` or a `?`.

Similarly, `NODE_SEQUENCE` is used for any sequence of statements or expressions. This includes a compound statement (i.e. a block), a comma operator expression, an expression statement (a GNU extension), and for some internal purposes. A compound statement always has type `void`, whereas a comma expression and an expression statement will have the type of their last child node. These are also all compiled identically.

Implicit casts are frequently inserted by the parser, especially around arithmetic operators and statement expression. Type promotions, usual arithmetic conversions and so forth are performed by the parser and the casts are reflected in the parse tree. Whenever an expression's value is ignored, for example in an ordinary assignment expression, an implicit cast to void is inserted. These are necessary to make code generation work properly; see the Code Generation section below.



## Labels and Cases

In the C grammar, labels and cases are always attached to the statement that follows. For example, this is a switch with one unbraced statement:

```c
// skip the rest of this function if foo is A, B or C
switch (foo)
    case A:
    case B:
    case C:
        return;
```

In the Onramp tree, we separate labels and cases from statements for simplicity. When parsing the above, Onramp creates a synthetic `NODE_SEQUENCE` containing four children: three `NODE_CASE` and a `NODE_RETURN`. In fact the structure of the tree is essentially identical whether or not it has braces. The only difference is during parsing, wherein braces create a scope and allow multiple statements and declarations.

The above switch statement is parsed as:

```
SWITCH `switch` void
├─ACCESS `foo` int
└─SEQUENCE void
  ├─CASE `case` void
  │ └─ACCESS `A` int
  ├─CASE `case` void
  │ └─ACCESS `B` int
  ├─CASE `case` void
  │ └─ACCESS `C` int
  └─RETURN `return` void
```



## Code Generation

The tree is compiled to basic blocks of IR in memory.

The most important function is `generate_node()`. This takes a node and a temporary into which the expression should compute its value. If the expression computes a value that is passed indirectly (such as a struct or a 64-bit value), the temporary contains the address of where the value should be stored; otherwise, the expression must assign a value to the temporary.

Whenever a value is passed indirectly, the parent of the node is responsible for providing storage for it. This often happens in `generate_cast()`, which is the main reason the parser casts all expression statements to void. Storage is typically generated by a `var` instruction; the code generator turns this into space in the stack frame.



## Relative Relocations

C allows constant initializers at file scope to use the address of other variables in an additive constant expression. For example:

```c
// at file scope
char x[4];
char* y = &x[2];
```

The above is legal C. On a platform like Linux, the compiler would emit a relocation of `x` with offset 2 to replace the value of `y`. The dynamic linker would apply this relocation at program load.

Onramp has no dynamic linker. Its static linker has no way to apply offsets to invocations, and it cannot pre-compute the address because all Onramp programs are position-independent. The value of y is really `rpp + ^x + 2`, and we don't know `rpp` until runtime so it must be computed at program start.

To do this, the compiler emits an initializer function to initialize `y`. The function is marked `__attribute__((constructor))` with a higher priority than any that is user-specifiable. This ensures that it will be run by the libc before any user code.

For simplicity, such initializer functions are used to initialize most globals. The initializer for all variables is simply parsed into a tree and then compiled, either into the parent function (for locals) or into a dedicated constructor function (for globals). This allows us to use the same code generation path to initialize both global and local variables.



## Error Reporting

The lexer reports some errors through the same mechanism as previous stages: the `fatal()` function in libo which tracks the current file and line of the lexer.

Unlike previous stages however, file and line information is attached to each token. The parser therefore reports most errors against a particular token using the `fatal_token()` function. This gives more accurate file and line information especially when errors are reported after a whole expression or function is parsed.

In both cases, the stack of included files is stored, and the compiler can report the path of `#include` directives that led to the incorrect source line (although this is not entirely implemented yet.) Tokens contain a reference to a parent token from the file that included them, so the stack of include files for a token is essentially a linked list of tokens.

This compiler still only reports the first error and immediately exits. All errors are considered fatal; there is no attempt at all to recover from errors or to report multiple errors.
