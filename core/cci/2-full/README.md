# Onramp Compiler -- Final Stage

This is the final stage of Onramp. We aim to implement most of the C2x language, excluding some features that Onramp cannot support (e.g. threads, atomics) and excluding some features that would not be terribly useful on Onramp (e.g. complex numbers.)

Unlike previous stages, the final stage compiler is not single-pass. It builds an AST, compiles to assembly in-memory, and will maybe even perform minor optimizations on both.

I have a bit of code written for this but it is not ready to publish yet.
