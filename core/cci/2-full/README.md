# Onramp Compiler -- Final Stage

The final stage Onramp compiler is simple and fairly traditional. It uses a handwritten recursive-descent parser, it generates a simple parse tree, and it compiles it (poorly) to basic blocks in memory. Minor optimizations are performed on both the parse tree and the blocks of assembly.

Despite its simplicity, we aim to implement most of C11 with many C23 features and many GNU and other extensions.

The compiler itself also has some optimizations in its implementation. For example it uses [string interning](https://en.wikipedia.org/wiki/String_interning) to make token comparisons fast. We also intern some base types. In combination with a faster libc, the final compiler is much faster than previous stages, especially after recompiling itself with optimizations.

The compiler implements a command-line interface similar to GCC and friends, but extension usage causes errors by default. Pass `-fgnu-extensions` or a `-std=gnu*` mode to make it behave more like GCC. (When passed to the driver, these also define `__GNUC__`.) See the [Usage Guide](../../../docs/usage-guide.md) for details.



## Components

- `block` - A basic block of assembly instructions, starting with a label and ending in a `jmp` or `ret`.
- `emit` - Low-level functions for writing the output file: bytes and numbers, opcode and register names, etc.
- `function` - The container for a function. Contains its parse tree and its list of basic blocks.
- `generate` - Code generation. Converts the parse tree into basic blocks of assembly.
- `instruction` - A single instruction in a basic block.
- `lexer` - Converts the input stream into tokens.
- `node` - A node in a parse tree, along with functions to manipulate it.
- `options` - Command-line options, such as warnings and optimization flags.
- `parse` - The parser. Converts tokens from the lexer into a parse tree.
- `record` - A struct or union.
- `scope` - Scope lookup functions. Contains hashtables for declared variables and typenames.
- `strings` - Global intern strings for all keywords and operators.
- `token` - A token, including its source location.
- `type` - One part of a type, either a declarator or a base, forming a graph that describes a complete type.



## Algorithm

The final stage compiler operates in phases on units of functions.

For each function in the input file, the following phases are performed:

- The function is parsed into a tree;
- An optional optimization pass runs on the tree;
- The tree is compiled into basic blocks containing assembly code;
- An optional optimization pass runs on each basic block;
- The complete function is emitted to the output file.

Note that, to keep the compiler simple, there currently isn't an intermediate representation. The tree is compiled directly into assembly. See the Code Generation section below.

Global variable initializers are compiled as constructor functions where necessary so they also follow the above steps. See the Relative Relocations section below.

Everything is freed once the function is emitted, minimizing the total memory usage of the compiler.



## Lexer

The final stage compiler uses the [libo/1](../../libo/1-opc) intern string table for all strings. Checking strings for equality can be done by a simple pointer comparison. This makes token comparisons trivial.

All built-in tokens are added to the string table at startup. Functions like `lexer_expect()`, `lexer_accept()` and `lexer_is()` take an intern string as argument. This is much faster than previous stages which required a call to `strcmp()` for every token comparison.

The intern string table reference counts all strings. It can free strings that are unused and remove them from the table. Since we free every function after compiling it, memory usage is kept to a minimum throughout the run of the compiler.

The lexer does not support digraphs, trigraphs, comments, or preprocessor directives other than `#line` and `#pragma`. These are the responsibility of the preprocessor. The compiler is expected to always be run on the output of the preprocessor.



## Parse Tree

The parser parses code into a tree. We sometimes call it an abstract syntax tree or a parse tree but it's really a semantic tree, as most semantic properties are resolved as it builds the tree. For example the types of all nodes are resolved during parsing, not in a separate analysis pass. It also does not include some syntax such as types.

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

In the Onramp tree, we separate labels and cases from statements for simplicity. When parsing the above, Onramp creates a synthetic `NODE_SEQUENCE` containing four children: three `NODE_CASE` and a `NODE_RETURN`. In fact the structure of the tree is essentially identical whether or not it has braces. The only difference is during parsing, wherein braces allow declarations and multiple statements and create a scope.

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

The tree is compiled directly to basic blocks of assembly. We do not (yet) have any kind of low-level intermediate representation.

The most important function is `generate_node()`. This takes a node and a numbered register (`r0`-`r9`) into which the expression should compute its value. If the expression computes a value larger than a register, the register contains a pointer to where the value should be stored (which must be provided or allocated from the stack by the parent expression.)

The register allocator is as simple as possible. Registered are allocated sequentially from r0 to r9 and freed in reverse order of allocation. If additional registers are needed, we loop back around to r0 and push the existing value to make room. (This means only the last 10 allocated registers can be used at any time. This is not a problem because most operations use at most four allocated registers.)

All local variables are spilled at all times. We don't (yet) do any kind of register allocation for variables. This has poor performance but the code generation is extremely simple.

(The code generator is by far the weakest part of the compiler, and probably the weakest part of all of the final stage Onramp tools. There isn't much focus on good code generation at this point since it's purely for performance; a more important goal is to get everything working first.)



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
