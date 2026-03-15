# Documentation

This is a reference of all Onramp documentation.

Onramp documentation (including this `README.md`) is written in [GitHub-Flavored Markdown][gfm]. Onramp is heavily documented; specifications are written for all of Onramp's tools and languages.



## Project Organization

- [core/](../core/) - The platform-independent parts of Onramp. Contains the compiler, linker, driver, libc, etc.
- [platform/](../platform/) - Implementations of the platform-specific components of Onramp for various platforms.
- [extra/](../extra/) - Additional tools beyond the core bootstrap.
- [scripts/](../scripts/) - Scripts for building and installing Onramp on various platforms.
- [docs/](../docs/) - Specifications of Onramp's languages and other documentation. Defines the Onramp subsets of C, Onramp Assembly, etc.
- [test/](../test/) - Test cases and scripts for testing the various Onramp components.



## Index

The following documentation pages are available:

- [Main Page](../README.md)
- Guides
    - [Setup Guide](setup-guide.md)
    - [Usage Guide](usage-guide.md)
- Specifications
    - [Onramp Hexadecimal](hexadecimal.md)
    - [Onramp Virtual Machine](virtual-machine.md)
    - [Onramp Object Code](object-code.md)
    - [Onramp Assembly](assembly.md)
    - [Onramp Intermediate Representation](intermediate-representation.md)
    - [Onramp Minimal C](minimal-c.md)
    - [Onramp Practical C](practical-c.md)
    - [Onramp Shell](shell.md)
    - [Onramp Debug Info](debug-info.md)
- Miscellaneous
    - [Implementation Status](status.md)
    - [Bootstrap Path](bootstrap-path.md)
    - [Testing Onramp](../test/README.md)
    - [Coding Without Labels](coding-without-labels.md)
    - [Inspiration](inspiration.md)
- Tools
    - [`core`: Core Tools](../core/README.md)
        - [`ar`: Archiver](../core/ar/README.md)
            - [`ar/0-cat`: Primitive Archiver](../core/ar/0-cat/)
            - [`ar/1-unix`: Final Archiver](../core/ar/1-unix/)
        - [`as`: Assembler](../core/as/README.md)
            - [`as/0-basic`: Primitive Assembler](../core/as/0-basic/)
            - [`as/1-compound`: Compound Assembler](../core/as/1-compound/)
            - [`as/2-full`: Final Assembler](../core/as/2-full/)
        - [`cc`: Driver](../core/cc/README.md)
        - [`cci`: Compiler](../core/cci/README.md)
            - [`cci/0-omc`: omC Compiler](../core/cci/0-omc/)
            - [`cci/1-opc`: opC Compiler](../core/cci/1-opc/)
            - [`cci/2-full`: Final Compiler](../core/cci/2-full/)
        - [`cg`: Code Generator](../core/cg/README.md)
            - [`cg/0-asm`: Assembly Code Generator](../core/cg/0-asm/)
            - [`cg/1-full`: Final (IR) Code Generator](../core/cg/1-full/)
        - [`cpp`: Preprocessor](../core/cpp/README.md)
            - [`cpp/0-strip`: Directive Stripper](../core/cpp/0-strip/)
            - [`cpp/1-omc`: omC Preprocessor](../core/cpp/1-omc/)
            - [`cpp/2-full`: Final Preprocessor](../core/cpp/2-full/)
        - [`hex`: Hex Tool](../platform/hex/)
            - [`hex/0-onramp`: Bytecode Hex Tool](../core/hex/0-onramp/)
            - [`hex/1-c89`: Final (C89) Hex Tool](../core/hex/1-c89/)
        - [`ld`: Linker](../core/ld/README.md)
            - [`ld/0-global`: Primitive Linker](../core/ld/0-global/)
            - [`ld/1-omc`: omC Linker](../core/ld/1-omc/)
            - [`ld/2-full`: Final Linker](../core/ld/2-full/)
        - [`libc`: Standard Library](../core/libc/README.md)
            - [`libc/0-oo`: Object Code libc](../core/libc/0-oo/)
            - [`libc/1-omc`: omC libc](../core/libc/1-omc/)
            - [`libc/2-opc`: opC libc](../core/libc/2-opc/)
            - [`libc/3-full`: Final libc](../core/libc/3-full/)
        - [`os`: Operating System](../core/os/README.md)
            - [`os/0-minfs`: Minimal Filesystem OS](../core/os/0-mins/)
            - [`os/1-full`: Final OS](../core/os/1-full/)
        - [`sh`: Shell](../core/sh/README.md)
    - [`extra`: Extra Tools](../extra/)
        - [`disas`: Disassembler](../extra/disas/)
        - [`strace`: Syscall Tracer](../extra/strace/)
        - [`vminfo`: VMInfo tool](../extra/vminfo/)
    - [`platform`: Platform-specific Tools](../platform/)
        - [`cc`: Platform-specific Driver Extensions](../platform/cc/)
        - [`hex`: Platform-specific Hex Tools](../platform/hex/)
        - [`vm`: Virtual Machines](../platform/vm/)



## File Types

| Extension   | Description                                                                  |
|-------------|------------------------------------------------------------------------------|
| `.ohx`      | [Onramp Hexadecimal][dochex], plain-text hexadecimal with comments           |
| `.oe`       | [Onramp Executable][docvm], an Onramp bytecode program in binary             |
| `.oo`       | [Onramp Object File][docobj], plain-text bytecode with labels                |
| `.oa`       | [Onramp Archive][docobj], a static library of `.oo` files                    |
| `.os`       | [Onramp Assembly][docasm], our custom assembly language                      |
| `.oir`      | [Onramp Intermediate Representation][docir], our compiler IR                 |
| `.i`        | Preprocessed C source code (no comments, no preprocessor directives)         |
| `.c`        | C source code, an Onramp Subset ([omC] or [opC]) or a standard version       |
| `.sh`       | [Onramp Shell][docsh], our subset of POSIX shell                             |
| `.od`       | [Onramp Debug Info][docdebug], the debug symbols for an Onramp executable    |
