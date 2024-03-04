# Onramp Utility Library -- Second Stage

This is the second stage of libo. It is written in [Onramp Practical C (opC)](../../docs/practical-c.md).

This adds extensions to the error handling functions.

The current filename and line now forms a stack. It can be pushed when an `#include` directive is reached and popped when the include file ends. `fatal()` prints the file stack like modern compilers do.

`fatal()` now takes `printf()`-style arguments.

This also adds an intern string container and a hashtable.
