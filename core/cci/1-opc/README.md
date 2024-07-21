# Onramp Compiler -- Second Stage

This is the implementation of the Onramp second stage compiler. It is written in [Onramp Minimal C](../../../docs/minimal-c.md) and compiles [Onramp Practical C](../../../docs/practical-c.md).

This adds `short`, `else`, `for`, `struct`, `enum`, `switch`, arrays, variadic functions and more, with which we can implement our final stage C compiler.

It implements most of C89 with some C99 and C11 features. We exclude global and compound initializers, function pointers, multi-dimensional arrays, `long long`, floating point, and a few other minor features.

This document describes the implementation. For a description of the language it compiles, see [Onramp Practical C](../../../docs/practical-c.md).



## Types

The opC type system has `char`, `short`, `int`, `long` (as an alias of `int`), `void`, `struct`, `union`, `enum`, pointers and arrays. However, it has no function pointers, no multi-dimensional arrays, and no pointers to arrays. Additionally, the `const` and `volatile` qualifiers are ignored.

These simplifications mean we do not have to store a real declarator list. The only items allowed in a declarator list before the final array size are pointers, so we can flatten the entire declarator list down to a pointer count and an array size.

Types are therefore not represented as a recursive data structure (e.g. a linked list) as they are in other compilers (although types can recurse indirectly through structs and unions.) Types are instead represented by a very simple structure. A type, called `type_t`, is composed of:

- a qualified base type;
- an l-value flag;
- an optional record pointer;
- an indirection (pointer) count; and
- an optional array size.

A qualified base type means either a basic type or a record type. A basic type is a fundamental type with an optional signedness qualifier: `int`, `unsigned short`, `void`, etc. A record is a struct or union type: it stores the names, types and offsets of the contained fields. (An enum is just an alias of `int`; see below.)

These fields are wrapped in a type called `type_t` which is allocated and passed by pointer (see "emulated structs" below.) Expression parsing functions that generate a type (functions whose name starts with `parse` or `try_parse`) typically return ownership of one so the caller must free it or pass it along. Most other functions either return a type while retaining ownership of it (such as variable lookup) or modify a given type (such as `compile_dereference_if_lvalue()`.)

Generally speaking, if a function takes a non-const type as an argument, it takes ownership of it, and if a function returns a non-const type, it is returning ownership of it. Functions that modify a given type return it again in order to stick to this rule.

The record pointer points to an immutable type called `record_t`. This contains a linked list of immutable `field_t`, each of which describes the name, type and offset of the fields in the struct. Records and fields are owned by the record table and exist for the lifetime of the compiler.



## Expression Evaluation

Expression parsing in cci/1 is similar to cci/0. All expression functions emit code on the fly. When an expression parsing function returns, code to compute that expression has already been emitted.

The code they emit places the result of the expression in `r0`. This stage only supports register-sized r-values in expressions. This means structs cannot be passed by value and `long long` is not supported.

When an expression returns an l-value, the emitted code places the *address of* the value in `r0` instead of the value itself. The function `compile_lvalue_to_rvalue()` converts an l-value into an r-value.



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



## Switch

The switch statement in this stage is inefficient but very simple in implementation. We use switch statements extensively in our final stage compiler so we need a switch. It doesn't have to be fast; it just has to work.

(As usual the final stage compiler has a much better implementation of switch. This section describes the switch implemented by this bootstrapping stage.)

The entire implementation is about 120 lines of code. The switch parsing code is in these functions in `parse-stmt.c`:

- `parse_switch()`
- `parse_case()`
- `parse_default()`
- `parse_break()`

As everything else, the switch statement emits all its contents on-the-fly, including `case` and `default` labels. We do not collect case labels, and we do not emit a jump table or binary search or anything else. Each `case` test is emitted as parsed, interleaved with the other code contents of the switch. All emitted assembly is in the same order as the original source.

It's best to follow the details with a diagram. Here's an example:

```c
switch (foo) {
    case X: puts("X"); break;
    default:
    case Y: puts("Y"); // fallthrough
    case Z: puts("Z"); break;
}
```

The above compiles to the following logical blocks:

```
             ------------------------------
             | compute and store `foo`    |            switch (foo)
         .-- | jump to "test case X"      |            {
         |   ------------------------------
         |   | jump over test case X      | --.            case X:
         '-> | test case X. if no match,  |   |
         .-- |     jump to "test case Y"  |   |
         |   ------------------------------   |
         |   | call puts("X")             | <-'                puts("X");
      .--+-- | break                      |                    break;
      |  |   ------------------------------
      |  |   | jump over test case Y      | --. <--.       default: case Y:
      |  '-> | test case Y. if no match,  |   |    |
      |  .-- |     jump to "test case Z"  |   |    |
      |  |   ------------------------------   |    |
      |  |   | call puts("Y")             | <-'    |           puts("Y");
      |  |   ------------------------------        |
      |  |   | jump over test case Z      | --.    |       case Z:
      |  '-> | test case Z. if no match,  |   |    |
      |  .-- |     jump to end/default    |   |    |
      |  |   ------------------------------   |    |
      |  |   | call puts("Z")             | <-'    |           puts("Z");
      |.-+-- | break                      |        |           break;
      |  |   -----------------------------|        |
      |  '-> | jump to default label      | -------'   }
      '----> | end of switch              |
             ------------------------------
```

The `switch` statement creates an anonymous local variable in which its expression is stored. It then generates a label for the first case label test and emits a jump to it, and then parses a block as normal.

A `case` label emits the previously generated label, then tests the local variable against the value of the `case` expression. It emits a conditional jump to the next case test when the values don't match. The `case` labels form a sort of linked list.

When a match is found, the case tests don't jump, so the control flow continues to the next statement after the case. In order to ignore the `case` label tests when executing the contents of the switch, each `case` label also emits an unconditional jump around it (the "jump over" lines in the above graphic.) This makes fallthrough work. Once we've matched any case test, subsequent case tests are skipped as we flow through the block.

A `default` label just emits a label. At the closing brace of the switch, if none of the labels matched, we emit a jump to the `default` label if one exists.

The result is of course a mess of unnecessary jumps, but it means the compiler does not need to store anything at all besides a couple of label numbers. Everything is truly emitted on-the-fly, and the compiler memory usage is essentially zero even for gigantic nested switches.

An interesting quirk of this switch statement is that `case` label expressions don't have to be constant. In fact it is necessary because our enum values are not actually constants; they're just global variables. If we required cases to take constant expressions, we'd have to create a new kind of symbol to store enum values as constants. It's simpler to just make cases non-constant.

This means you can put arbitrary expressions in `case` labels and even call functions with side effects. Obviously it's not a good idea to do this; this is not officially supported by opC and must be avoided to remain compatible with C.
