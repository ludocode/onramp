# Onramp Archiver -- Final Stage

This stage is written in modern C. It implements most of the POSIX `ar` functionality, in particular adding, extracting, removing and replacing files. It also "fakes" some functionality such as indexing.

TODO implement this.

We hopefully don't need a totally complete implementation of `ar` here, just enough for whatever GCC's dependencies do when building static libraries.

Most commands, e.g. `d`, `m`, `r` are probably easy to implement. Just walk the archive, rewriting the whole thing to a temporary file. Probably we always write into a temporary file and then rename afterwards. In the case of `m` we need to do a full pass over the file to find the offsets of the given files then seek to each one to rewrite them. Simplest implementation is to just build an in-memory index of the entire source archive before doing anything.

We can probably ignore stored indexes. Ignore index commands (`s`, `S`), don't provide any support for them but pretend we do.

For most modifiers we can probably just error out. Wait until someone complains. Some like `l` (dependencies) will require additional metadata and changes to `cc` or `ld` to handle them, see if we can get away with never implementing them.

We should also provide `ranlib` as a no-op somehow. (Since we'll need a POSIX shell anyway to build anything that needs ranlib, we can probably just make ranlib a no-op shell script. This would only be used on a POSIX platform so it wouldn't belong here.)

