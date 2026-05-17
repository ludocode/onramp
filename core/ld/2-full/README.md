# Onramp Linker -- Final Stage

This is the Onramp final stage linker. It is written in [Onramp Practical C (opC)](../../../docs/practical-c.md).

It supports all Onramp linker features, performs garbage collection of unused symbols, and gives proper error messages with line numbers.



## Under Construction

This linker is good enough to link virtually all C programs but there are several features and extensions we still need to implement:

- [ ] weak symbols
- [ ] constructor/destructor
- [ ] garbage collection (only partially implemented)
- [ ] print stats



## Algorithm

The linker performs two passes over all input.

The first pass collects all symbol names and measures their sizes. If optimization is enabled, it also collects all symbol invocations, forming a dependency graph of symbols.

(The first pass also collects labels that are not resolved by the final stage assembler. During bootstrapping, this means all labels since the bootstrap assemblers don't resolve them. After the final assembler is bootstrapped, there should be no labels left, but the functionality is still available to allow linking handwritten object files.)

Once all symbols and labels are collected, if optimization is enabled, the linker walks the graph from `__start` (and from all constructors and destructors) marking any reached symbols as used. The linker then performs the layout of the object file. It goes through the symbol list (in parse order) and calculates the address of each symbol (skipping unused symbols under optimization.) When finding a weak symbol, we continue searching to try to find a strong one.

The linker then performs the second pass over the input, emitting all symbols (or all used symbols) with computed values substituted for all invocations. Finally, the linker outputs metadata symbols, such as the constructor and destructor lists.



## Data Structures

A global hashtable stores all symbols. Each symbol has a hashtable to store its labels and a vector to store the symbols it references (for garbage collection.)

Each symbol has a file index used to differentiate static symbols. Global symbols have file index -1. There can be multiple symbols with the same name as long as they have different file indexes.
