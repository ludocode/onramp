# Onramp Compiler -- First Stage

This is the first-stage compiler. It is implemented in compound [Onramp Assembly](../../../docs/assembly.md) and compiles [Onramp Minimal C (omC)](../../../docs/minimal-c.md).

This document describes the implementation. For a specification of the language it compiles, see [Onramp Minimal C](../../../docs/minimal-c.md).



## Types

Onramp Minimal C only supports the base types `void`, `char` and `int`, along with pointers to them: `void*`, `int**`, `char***` and so on.

Since a complete type always prefixes a name, types and names are always parsed separately. This makes for much simpler parsing.

We also require that most qualifiers (`typedef`, `extern`, `static`) come first, and they can only be used at file scope. This means we don't have to worry about qualifiers during type parsing either. The only qualifier that can appear anywhere is `const` and it is ignored.

Types are stored in the low seven bits of a `char`. This makes it possible to work with types entirely in mix-type instruction arguments in assembly. Types have the following layout:

- bits 0-4: The pointer indirection count. This is 0 for non-pointers and 1 or more for pointers.
- bits 5-6: The basic type. 1 is void, 2 is char and 3 is int.
- bit 7: The l-value flag.

For example `0x10` is `void`, `0x21` is `char*`, `0x73` is an l-value of type `int***`, and so on.



## Expression Evaluation

All expression parsing functions emit code on-the-fly. The code they emit places the result of the expression in `r0`, and they return the type of the value the code places in `r0`. The stack is used for storage of all intermediate values. We essentially treat the bytecode as a stack machine, ignoring almost all numbered registers.

For example, `parse_binary_expression()` first parses the left-hand side into r0 and pushes it; then parses the right-hand side into r0; then pops the left-hand side to r1 and calculates the result into r0. Constants and variables are always loaded into r0; mix-type arguments are not used. An expression like `(5 * 2) + (9 / 4)` therefore emits code like this:

```asm
imw r0 5       ; 5
push r0
imw r0 2       ; 2
pop r1
mul r0 r1 r0   ; *
push r0
imw r0 9       ; 9
push r0
imw r0 4       ; 4
pop r1
div r0 r1 r0   ; /
pop r1
add r0 r1 r0   ; +
```

The resulting code is essentially in reverse Polish notation.

This is 23 bytecode instructions. It is of course extremely inefficient, but it is also extremely simple, and it works. A major advantage of spilling all variables and expressions to the stack is that function calls don't need to do anything special to preserve registers.

When an expression returns an l-value, the emitted code places the *address of* the value in `r0` instead of the value itself. It is effectively an additional indirection, except that this type can also be the target of an assignment expression. The function `compile_dereference_if_lvalue()` is used to convert an l-value into an r-value; you'll see this called wherever the real value is needed.



## Local Variables

Local variable definitions are stored in a stack in `locals.os`. When a variable is defined, it is pushed onto this stack. When a block closes, all variable definitions in that block are popped.

The values of variables are stored in the current function's stack frame. Since we don't support arrays or structs, all variables take up exactly one word of space. A variable's index (plus one) is its position in words under the frame pointer.

```c
void foo(int a) {       //   ---------------
    char b;             //   | (prev. rfp) |  <- rfp
    {                   //   ---------------
        void* c;        //   |      a      |  <- rfp -4
    }                   //   ---------------
    {                   //   |      b      |  <- rfp -8
        int* d;         //   ---------------
        char** e;       //   |    c/d/f    |  <- rfp -12
    }                   //   ---------------
    int*** f;           //   |      e      |  <- rfp -16
}                       //   ---------------
```

The definition of local variables and the opening and closing of blocks does not emit any instructions. The compiler instead tracks the maximum number of variables that were defined at any time in the function and reserves space for the full stack frame at the entry point of the function (see Function Generation below.)



## Function Generation

The size of a function's stack frame can only be determined after the entire function has been parsed. Since we are a single-pass compiler, we need to emit code before we know the frame size.

We treat our output file as a stream so we can't seek back and patch the frame size in after the fact. We also don't have support in our linker to fill in values for us. We need to output the value after the function is compiled.

To do this, we place the preamble at the end of the function. For example, a function `foo()` is compiled like this:

```asm
@_F_foo
    ; ...
    ; ...
    ; ...
    leave
    ret

=foo
    enter
    imw r9 <framesize>
    sub rsp rsp r9
    jmp ^_F_foo
```

We start with a static symbol to contain the function's contents. Its name is the function name prefixed with `_F_`. It assumes the correct stack space has been reserved. We simply emit each statement one by one with labels to jump among `if` and `while` blocks.

Once the entire function has been emitted, we now know the required size of the stack frame. We can now define the actual function, which is just a "shunt" or "trampoline". It performs the preamble, reserves the appropriate stack space, and jumps to the `_F_` symbol.

The resulting function is thus technically in two separate symbols. This is of course inefficient but it is very easy to implement for bootstrapping.



## Strings

When a literal string appears in a function, we assign the string a unique id, copy the string from the lexer and store it in an array. Once the function has been emitted, all strings are output as symbols and freed. For example, a function `bar()` that uses several strings is compiled like this:

```asm
@_F_bar
    ; ...
    ; ...
    imw r0 ^_Sx0
    add r0 rpp r0
    ; ...
    ; ...
    imw r0 ^_Sx1
    add r0 rpp r0
    ; ...
    ; ...
    imw r0 ^_Sx2
    add r0 rpp r0
    ; ...
    ; ...
    leave
    ret

=bar
    enter
    imw r9 <framesize>
    sub rsp rsp r9
    jmp ^_F_bar

@_Sx0
    "Alice"'00

@_Sx1
    "Bob"'00

@_Sx2
    "Carol"'00
```

We don't support concatenation of adjacent string literals in this stage.
