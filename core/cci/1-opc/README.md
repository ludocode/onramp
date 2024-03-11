# Onramp Compiler -- Second Stage

This is the implementation of the Onramp second stage compiler. It is written in [Onramp Minimal C](../../../docs/minimal-c.md) and compiles [Onramp Practical C](../../../docs/practical-c.md).

This adds `else`, `for`, `struct`, `enum`, `switch`, `long long`, arrays, variadic functions and more, with which we can implement our final stage C compiler.

This document describes the implementation. For a description of the language it compiles, see [Onramp Practical C](../../../docs/practical-c.md).



## Status

All of omC is supported and some parts of opC are done. We need to fix or add:

- [x] make self-hosting
- [x] new type system (see below)
- [x] globals hashtable
- [ ] `struct` and `union`
- [ ] `unsigned`
- [ ] `short`
- [ ] `long long`
- [x] `enum`
- [ ] arrays
- [x] `else`
- [ ] `for` (with variable declaration)
- [ ] `switch` and `case`
- [ ] `goto` and labels (maybe?)
- [ ] full expression parser
- [x] constant expression parser



## Types

opC has all C99 signed and unsigned integer types, `void`, `struct`, `union`, `enum`, pointers and arrays. However, it has no function pointers, no multi-dimensional arrays, and no pointers to arrays. Additionally, the `const` and `volatile` qualifiers are ignored.

These simplifications mean we do not have to store a real declarator list. The only items allowed in a declarator list before the final array size are pointers, so we can flatten the entire declarator list down to a pointer count and an array size.

Types are therefore not represented as a recursive data structure (e.g. a linked list) as they are in other compilers (although types can recurse indirectly through structs and unions.) Types are instead represented by a very simple structure. A type, called `type_t`, is composed of:

- a qualified base type;
- an l-value flag;
- an optional record pointer;
- an indirection (pointer) count; and
- an optional array size.

A qualified base type means either a basic type or a record type. A basic type is a fundamental type with an optional signedness qualifier: `int`, `signed char`, `unsigned long long`, `void`, etc. A record is a struct or union type: it stores the names, types and offsets of the contained fields. (An enum is just an alias of `int`; see below.)

These fields are wrapped in a type called `type_t` which is allocated and passed by pointer (see "emulated structs" below.) Expression parsing functions that generate a type (functions whose name starts with `parse` or `try_parse`) typically return ownership of one so the caller must free it or pass it along. Most other functions either return a type while retaining ownership of it (such as variable lookup) or modify a given type (such as `compile_dereference_if_lvalue()`.)

Generally speaking, if a function takes a non-const type as an argument, it takes ownership of it, and if a function returns a non-const type, it is returning ownership of it. Functions that modify a given type return it again in order to stick to this rule.

The record pointer points to an immutable type called `record_t`. This contains a linked list of immutable `field_t`, each of which describes the name, type and offset of the fields in the struct. Records and fields are owned by the record table and exist for the lifetime of the compiler.



## Expression Evaluation

Expression parsing in cci/1 is similar to cci/0. All expression functions emit code on the fly. When an expression parsing function returns, code to compute that expression has already been emitted.

The code they emit places the result of the expression either in `r0` or on the stack. If the result type of the expression is an address or an integer that fits in a register (e.g. `char`, `short`, `int`, all pointers, all l-values), it is placed in `r0`. Otherwise (e.g. `long long`, all `struct` and `union` types), it is at the top of the stack.

When an expression returns an l-value, the emitted code places the *address of* the value in `r0` instead of the value itself. The function `compile_lvalue_to_rvalue()` converts an l-value into an r-value.



## Function Calls

The calling convention for function calls is described in the [Onramp bytecode spec](../../../docs/virtual-machine.md). Any values that are not addresses and not integers that fit in a word are passed indirectly.

The function call parser first reserves stack space for the return value if it is passed indirectly; the address will be passed as the first argument. It then parses the arguments. Each argument is copied (in full) to the stack, and argument types and offsets are stored in a list. Then, the function parser puts the first four arguments, or addresses to them if passed indirectly, in registers `r0`-`r3`. Finally each remaining argument, or address to it if passed indirectly, is pushed to the stack right to left.

This mechanism supports passing and returning structs and `long long` integers by-value.

There is no special behaviour needed for variadic functions. They use the same calling convention as regular functions.



## Emulated Structs

This implementation does not use structs because they are not supported by omC. Instead we do a "poor man's struct": we emulate them using accessor functions. For example, suppose we wanted to write:

```c
typedef struct {
    char* name;
    int age;
} person_t;
```

In omC, instead we would declare `person_t` as an array of pointers and write accessor functions to access the contents. For example:

```c
typedef char person_t;
person_t* person_new(const char* name, int age);
void person_delete(person_t* person);
char* person_name(person_t* person);
int person_age(person_t* person);
```

The implementation just does the pointer arithmetic and casting manually.

```c
person_t* person_new(const char* name, int age) {
    person_t* person = malloc(2 * sizeof(void*));
    *(int*)person = strdup(name);
    *(int*)(person + sizeof(char*)) = age;
}

void person_delete(person_t* person) {
    free(person_name(person));
    free(person);
}

char* person_name(person_t* person) {
    return *(int*)person;
}

int person_age(person_t* person) {
    return *(int*)(person + sizeof(char*));
}
```

This way we can create a `person_t` and access its fields using functions. We can write somewhat natural-looking, though a bit object-oriented, code:

```c
person_t* person = person_new("John Doe", 25);

fputs("The current user is: ", stdout);
fputs(person_name(person), stdout);

person_delete(person);
```

This is all valid omC. Of course this has many limitations: it's very slow, it's verbose, and we cannot pass them by value or place them on the stack. It is however extremely simple and legible and more than sufficient to bootstrap our full C compiler.



## Enums

Enum types are treated as equivalent to `int`, and enum values are treated as global variables of type `int`. In fact, the compiler does not even keep track of enum declarations. The name of an enum type is ignored entirely.

Whenever the compiler encounters `enum`, it consumes and ignores the name that follows it. Then, if at global scope, it checks for `{`; if found, each enum value it contains is defined as a global integer variable. After that, the fact that it was an `enum` is discarded; it is treated as though it was the keyword `int`.

This means enums can be forward declared and can be used without ever being defined, and enum values are interchangeable and can even be modified. For example:

```
enum foo; // invalid, forward declaration of enum

enum foo {
    foo_1 = 1;
};
enum foo {  // invalid, duplicate enum
    foo_2 = 2;
};

enum while x; // invalid, enum name is a keyword

unsigned short enum foo foo = 2; // invalid, enum with integer type specifiers

enum bar bar = foo_1; // invalid, undeclared enum

foo_2 = 4; // invalid, assigning an enum value
```

All of the above nonsense is accepted by this compiler. Nevertheless, these constructs are not legal opC, and should be avoided to remain compatible with C.
