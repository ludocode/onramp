# Onramp Utility Library -- First Stage

This is the first stage of libo.

We don't implement much in the first stage because it's painful to implement in object code. We really only implement things that we know we'll need in the first few tools and that will be useful in all of them.

We implement:

- Error handling functions and variables
    - `current_filename` and `current_line`, the source location during parsing
    - `set_current_filename()` to store an allocated copy of it
    - `fatal()`, printing an error at the current file and line and exiting the program
- Utilities
    - `fnv1a_cstr()`, the hash function for all our hashtables
    - `itoa_d()`, similar to a common non-standard integer-to-string function
    - `fputd()` and `putd()`, functions for outputting decimal numbers
