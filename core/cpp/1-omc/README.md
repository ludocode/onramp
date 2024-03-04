# Onramp Preprocessor -- Second Stage

This is the implementation of the second stage preprocessor. It preprocesses [Onramp Minimal C (omC)](../docs/minimal-c.md).

It is implemented in omC, except that it can be preprocessed with the [first stage directive-stripping preprocessor (cpp/0-strip)](../0-strip).



## Implementation

The preprocessor doesn't have a tokenizer. It has a character scanner that normalizes line endings and consumes comments. It otherwise provides characters directly to the parser.

The parser reads characters and forms them into tokens as needed by the grammar. In retrospect this was probably a bad idea but it works.
