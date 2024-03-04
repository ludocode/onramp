# Onramp Linker -- Final Stage

This is the Onramp final stage linker. It is written in [Onramp Practical C (opC)](../../../docs/practical-c.md).

It supports all Onramp linker features, performs garbage collection of unused symbols, and gives proper error messages with line numbers.



## Under Construction

This linker is good enough to link virtually all C programs but there are several features and extensions we still need to implement:

- [ ] weak symbols
- [ ] constructor/destructor
- [ ] garbage collection (only partially implemented)
- [ ] zero symbols (bss)
- [ ] print stats

In addition, this linker was adapted from the first stage omC linker and was written before we had the libo string and hashtable. The code is therefore quite messy. It could use a good cleanup.



## Algorithm

The linker performs four passes over all input.

The first pass collects all symbol names and measures their sizes. This also includes static symbols so that we can perform garbage collection on them.

The second pass collects usage info. For each symbol, we gather a list of symbols it references. (This is a separate pass because we only want to gather symbol usage, not label usage, to avoid storing all labels for all files in memory.)

We then walk the usage graph from `__start` (and from all constructors and destructors) marking any reached symbols as used. When finding a weak symbol, we continue searching to try to find a strong one. Any unreached symbols are unused and will be skipped in future passes.

We then walk through the full list of symbols in order. Any kept symbols are assigned an address.

We then perform the third and fourth passes for each input file (or each file part of a static library.) The third pass collects label addresses (in relation to their containing symbol) and the fourth pass outputs all used symbols.

Finally, we output metadata: the constructor list, the destructor list and the zero size symbol (bss).



## Data Structures

Each symbol is stored in a struct with two linked list pointers, a static file index, and a list of used symbols.

The static file index is used to differentiate local scope. Global symbols have file index -1. There can be multiple symbols with the same name as long as they have different file indexes.

There are two hashtables. One stores symbols and one stores labels. They use closed hashing with linked lists for collision resolution. The hashtables use FNV-1a on the name only.

The symbol table is filled out on the first pass and kept for the entire link in order to perform garbage collection and assign addresses. The label table is filled in the third pass and cleared after the fourth for each file.



