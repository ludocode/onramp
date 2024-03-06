# Onramp Operating System

This is a placeholder for building a trivial OS on top of a freestanding Onramp VM. The OS would only have some basic filesystem support (plus an output/error stream), which should be all we need to bootstrap a native compiler, build a native OS kernel and chainboot into it.

The goal is to be able to bootstrap a real native OS from pure source code, using as small a binary seed as possible, without having to trust an existing binary OS.

With a hosted Onramp VM, the VM runs atop an existing operating system and bridges its filesystem. With a freestanding Onramp VM, we want to run the OS *inside* the VM. The VM simply redirects all filesystem syscalls to the OS running inside it, this way our OS can be completely portable.

I intend to start writing this once I get the Onramp compiler able to build some real C programs. We just need to be able to build a native compiler (e.g. TinyCC) and port some basic coreutils (a small number of utilities possibly from BusyBox/toybox) to Onramp. This should be enough to run the buildsystems for a native kernel, libc and coreutils.



## Stages

The OS will be written in two stages:

- [`0-minfs`](0-minfs/) - The first stage will be written directly in hexadecimal Onramp bytecode. It will implement a trivial in-memory filesystem. Files can only be created (not modified), and only one file can be open for writing at a time; the file data is simply appended to the end of the filesystem memory. Directory syscalls are not supported. This is enough to bootstrap the full Onramp toolchain.

- [`1-full`](1-full/) - The final stage will be written in C. Once the Onramp compiler is bootstrapped, we build this OS and jump into it. It will convert the trivial filesystem to a "real" one with directories, segmented files, writing multiple files at a time, opening files for modification, etc.

When the first stage OS starts up, it will run an init script in the Onramp shell. The init script will bootstrap Onramp, build the final stage OS, and launch it. The final stage OS will then run another init script which will do whatever the user desires (e.g. building a native OS.)



### Boot Stage

The first stage OS will be written directly in Onramp bytecode. It will implement a simple-as-possible in-memory filesystem in a dedicated region of memory.

Files are stored contiguously one after another starting at the start of the filesystem memory. Each file is aligned to a 512-byte boundary and is preceded by a 512-byte header that indicates its full filename and its size. To open a file for reading, the OS simply walks the headers until it finds the matching file. Since files are contiguous, most syscalls (e.g. read, seek) are trivial.

The filesystem only supports writing one file at a time. When a file is opened for writing, the OS writes a new header for the file at the end of the existing filesystem data, and all file writes are simply appended to it. Seeking is only supported for reading files, not for writing files (this is one reason the early stage compilers use trampolines to set up stack frames.)

Files cannot be modified. To delete a file we simply write a flag to its header to mark it as a tombstone. Directories are not really supported; a full filename contains slashes to simulate directories. The filesystem otherwise ignores all directories, ignores `mkdir`, and does not implement any syscalls for reading directories.

The trick here is that the in-memory filesystem format is mostly compatible with tarball archives. In order to seed the initial filesystem, the VM/bootloader just needs to pull the (uncompressed) Onramp tarball and any other needed tarballs off the boot media, copy them to an arbitrary address in memory and hand the address to the OS. This forms the initial filesystem. This makes everything as simple and as auditable as possible.



### Final Stage

The intention with the final stage OS is to implement just enough to get a real POSIX shell and a few other tools working, which will be enough to build a native compiler and OS.

We need to convert the "fake" filesystem into a "real" one with directories and such. It can still be as simple as possible; in particular, we don't need to implement any data integrity checks because the file system will not be persisted to disk. The final filesystem could potentially use a real disk and page sectors to it, but this would only be to reduce the memory required. The VM would just need to implement read/write sector syscalls to make this work.

The final stage will hopefully not need to support any kind of multitasking. Spawning programs will work the same way it does normally in Onramp: a program spawns another simply by loading it into memory, setting up the registers and a process info table, and jumping to it. The child program runs to completion before returning control to its parent. This means we can't implement `fork()`, pipes, etc.

We should still be able to implement enough of `posix_spawn()` to get a POSIX-like shell ported, though it will have obvious limitations. A pipeline can be simulated just by running each program one after the other using temporary files to buffer all data. A "background" task can just run to completion before returning control to the parent.

Remember, the goal here is not to make a real interactive OS. It's just to implement the bare minimum necessary to compile one.
