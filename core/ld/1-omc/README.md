# Onramp Linker -- Second Stage

This is the implementation of the Onramp second stage linker. It is written in [Onramp Minimal C (omC)](../../../docs/minimal-c.md).

This linker adds file scope. Since our previous stage linker doesn't have file scope, we have to implement this in a single C source file (otherwise generated label names would collide.) Thankfully we can still link it with assembly files (since we've manually ensured they have unique label names) so we're linking against our libc.

We don't support any advanced linker features yet so this linker is still relatively simple. We just need to differentiate between globals and locals, clear locals after each input file, and correctly seek archives to handle file scope. Local (static) symbols and labels are treated in exactly the same way except for padding of symbols to word boundaries.
