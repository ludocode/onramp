# Onramp Utility Library -- First Stage

This is the first stage of libo.

We don't implement much in the first stage because it's painful to implement in object code. We really only implement things that we know we'll need in the first few tools and that will be useful in all of them.

We implement:

- Error handling functions and variables
    - `current_filename`
    - `current_line`
    - `set_current_filename()` to store an allocated copy of it
    - `fatal()`, printing an error at the current file+line and exiting
- Utilities
    - `fnv1a_cstr()`, the hash function for all our hashtables
    - `itoa()`, a common but non-standard integer-to-string function
    - `fputn()`, a function to write a number to a file
