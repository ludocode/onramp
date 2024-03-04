# Onramp Archiver

`ar` is an archiving tool originating from Unix. It is used (almost exclusively) to combine multiple `.oo` object files into a `.oa` static library.

An Onramp static library is essentially the same as an Onramp object file, except that it contains metadata to delimit the contained object files. The metadata allows the archive to be modified and allows the linker to correctly treat each contained object file as individually scoped.

This tool helps us assemble our libc into a static library. This simplifies compilation, especially since libc stages are overlays over each other. This tool is also necessary to provide a POSIX-style compilation process. Libraries are used in the build process for popular compilers, and the `-l` option to `cc` must be able to find static libraries.

The archiver is written in two stages:

- `ar/0-cat` is essentially a glorified `cat`. It just concatenates the source object files, outputting `%<filename>` before each file. It only supports "rc" mode.

- `ar/1-unix` adds various commands for manipulating static archives. It also supports arbitrary file contents.

TODO % should be replaced with %% when adding files, and we shouldn't allow filenames that start with %. put this in the linker spec
