This is an implementation of the Onramp Virtual Machine directly in x86\_64 machine code.

The resulting executable is about 4 kB. This includes the ELF header, the string table and other constants, and the code. Much of the code (10%-20%) is padding between functions to give them fixed addresses and make them editable.

Not counting padding, the program code consists of about 800 x86\_64 instructions. That includes full error-checking, bounds-checking, friendly error messages, etc. This is all that is required to implement a complete Onramp VM. A VM that omits error checks could probably reduce this by 25% or more.

The full hexadecimal source code is about 2500 lines including whitespace and comments. It contains more comments than code.



### Resources

- https://www.nasm.us/doc/

- https://fasterthanli.me/series/reading-files-the-hard-way/part-2

- https://fasterthanli.me/series/making-our-own-executable-packer/part-1

- https://web.archive.org/web/20220116195013/https://www.dreamincode.net/forums/topic/285550-nasm-linux-getting-command-line-parameters/

- https://www.felixcloutier.com/x86/

- https://filippo.io/linux-syscall-table/

- https://blog.rchapman.org/posts/Linux_System_Call_Table_for_x86_64/
