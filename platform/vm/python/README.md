# Onramp Python VM

This is an Onramp VM in Python.

The Python VM is dozens of times slower than a handwritten machine code VM. The Onramp bootstrap with the Python VM takes several hours on a modern machine. I'm not sure if there is a way to fix this. Unfortunately Python is just extremely slow.

Both Python 2.7 and Python 3 are supported, although it is even slower on versions before 3.3 due to the lack of `memoryview.cast()`.

Only the minimum necessary syscalls are implemented. In particular, it lacks any kind of terminal configuration so it is not capable of running Doom.

This is mostly only useful as a demonstration of an extremely simple VM that performs virtually no error checking. If you want an example of an Onramp VM to port to another language, this is a great place to start.
