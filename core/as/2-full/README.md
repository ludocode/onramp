# Onramp Assembler -- Final Stage

This is the implementation of the final stage assembler.

It supports all the same syntax as the previous stage, except that it has better handling of linker flags. The improvements are mainly around error checking and debug info. The assembler properly parses debug info so it reports correct file and line information on errors.


