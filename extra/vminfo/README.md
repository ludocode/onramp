# VM Info Tool

The `vminfo` tool prints information about the program's environment, i.e. the VM and the parent program (if any.)

This tool is not yet complete. The following information will be printed:

- Initial register values
- Process info table contents
- Syscall table contents
- Command-line arguments
- Environment variables

This tool is not part of the core bootstrap but it is nevertheless written directly in hexadecimal bytecode. There are several reasons for this:

- It can be used to help debug a VM before running the Onramp bootstrap process.
- It can print information that is not accessible to C such as initial register values.
- It can be run in all versions of the Onramp VM (though it may print less information on very old versions.)
