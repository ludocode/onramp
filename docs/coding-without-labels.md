# Coding without labels

Programs like the x86\_64-linux [hex tool](../platform/hex/x86_64-linux) and [VM](../platform/vm/x86_64-linux) are hand-written directly in hexadecimal machine code. Programs like [ld/0](../core/ld/0-global) and [hex/onramp](../platform/hex/onramp) are written in hexadecimal Onramp bytecode. When coding in machine code or bytecode, we don't have any way to resolve labels. This document describes some of the techniques used to work around this in writing these programs.



## Basic Techniques

The hardest part of hand-writing machine code is addressing functions. Jumps and calls in x86\_64 are generally rip-relative so even the smallest change can throw off addresses across the entire executable.

We use a variety of techniques for coding without labels:

All strings, functions and global variables are aligned with padding to 16 byte or greater boundaries. This makes for short and simple addresses. We add lots of extra padding which allows some editing of functions without affecting other addresses.

If a function needs to expand beyond its padding, it can be replaced with padding and moved to the end, or the function after it can be moved to the end. Only references to that function need to be fixed up.

In an ELF container, we prefer to place all read-only data together (rather than listing a bunch of sections in the ELF header.) We place it at the start of the file, just after the header, and we add a large amount of padding so we can add more later without having to move any functions. (An even simpler way would be to load the entire file into one big read-write-execute section. We avoid this for safety and to prevent false positives from virus scanners.)

In Onramp bytecode, the only restriction is that the start symbol comes first. Functions, variables and constant data (e.g. strings) are in arbitrary order in the file and can be moved around as needed.

Tables of addresses are maintained in comments at the top of the file. To call a function or access a constant or global variable, the address tables are the quickest way to find its address.

Functions use mostly relative jumps internally but use absolute addresses to call other functions or access global data. Since x86\_64 doesn't really have absolute jumps, function addresses must usually be loaded into a register and called indirectly. We do this for virtually all function calls in our x86\_64 programs.

Functions can use `call`/`ret` instructions (where available) to handle branches even within the function itself. This stores the return address on the stack, eliminating an address from the code. Such code can still access local variables through the frame pointer so they can work a bit like nested functions.

Functions tend to be very small. If a function is more than a couple hundred bytes, it should probably be broken up. Functions that must be large (such as parse functions that can recognize many different tokens) use independent blocks with early returns to minimize relative jumps.



## Address assertions

We use address assertions liberally in all hand-written programs. They are critical for maintainability and for assuring correctness in a program of any significant size.

At the very least, we place an address assertion at the start and end of each symbol. (This means every address assertion is repeated, as the end address of one symbol is usually the start address of the next. This is intentional, to make sure we don't miss anything when we move symbols around.) We also place address assertions in specific places within functions, for example around large jumps, so we can easily calculate the offset and ensure it remains correct.

It's straightforward to audit these address assertions manually if you don't want to trust software you haven't bootstrapped to do it. Just count the bytes.
