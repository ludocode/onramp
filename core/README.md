# Onramp Core

This folder contains source code for all of the Onramp core components.

All core components are written in Onramp languages and are fully platform-independent. The build scripts are written in [Onramp Shell](../docs/shell.md) so they can run inside an Onramp VM.



## Table of Components

| Program          | Description            | Operation                                         |
|------------------|------------------------|---------------------------------------------------|
| [`cc`](cc/)      | Driver                 | Performs any or all phases of translation         |
| [`cpp`](cpp/)    | Preprocessor           | Preprocesses `.c` to `.i`                         |
| [`cci`](cci/)    | Compiler               | Compiles `.i` to `.os`                            |
| [`as`](as/)      | Assembler              | Assembles `.os` to `.oo` object file              |
| [`ar`](ar/)      | Archiver               | Combines `.oo` object files into `.oa` library    |
| [`ld`](ld/)      | Linker                 | Links `.oo` and `.oa` into `.oe` executable       |
| [`libc`](libc/)  | Standard library       | Provides C and POSIX library functions            |
| [`sh`](sh/)      | Shell                  | Runs scripts                                      |
| [`os`](os/)      | Operating System       | Implements a filesystem and syscalls              |



## Folder Structure

The core sources are organized with the following folder structure:

```
core/<name>/<stage>/
```

- `<name>` is the name of the component. It is also the base name of the resulting binary (executable or archive) when compiled.

- `<stage>` is the stage of the component. This is typically of the form `#-<lang>`, where `#` is the stage number and `<lang>` is the subset of the input language it supports. Some components have only a single stage, in which case the `<stage>/` folder is omitted.
