## Onramp Utility Library

`libo` is a small library of common code used in the implementation of the Onramp compiler toolchain. It is written in two stages.

- The first stage is written directly in [object code](../../docs/object-code.md). It contains some common error-handling code like `fatal()`  and a few utilities like `fputd()` and `fnv1a_cstr()`.

- The second stage is written in [Onramp Practical C (opC)](../../docs/practical-c.md). It adds a source file stack, a hash table, a string container, and so on.
