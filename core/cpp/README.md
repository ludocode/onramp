# Onramp Preprocessor

`cpp` is the Onramp C preprocessor. It takes C source code as input and outputs preprocessed C.

This is a description of the implementation stages of the preprocessor. The input to the preprocessor is the C programming language, the subset of which depends on the stage. The output of the preprocessor is C after preprocessing, stripped of comments and preprocessor directives.

The preprocessor is written in three stages:

- [`0-strip`](0-strip/) is written in compound assembly. It just strips comments and preprocessor directives.

- [`1-omc`](1-omc/) is written in Onramp Minimal C, except it depends on only the previous stage directive-stripping preprocessor. It preprocesses Onramp Minimal C.

- [`2-full`](2-full/) is written in C and implements all preprocessor features of C.
