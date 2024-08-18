# Onramp Assembler -- Final Stage

This is the implementation of the final stage assembler.

It supports all the same syntax as the previous stage, except that it has better error checking and debug info.

This assembler also has optimized versions of some compound instructions. Minor optimizations can sometimes be made based on the arguments, for example when a destination register matches or differs from the sources, or when an argument is zero. Note that this means compound instructions can assemble to different numbers of primitive instructions depending on the arguments.
