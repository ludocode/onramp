# Onramp x86-bios Freestanding VM

This is a placeholder for a freestanding Onramp VM that runs directly on x86 using BIOS boot. It is not yet implemented.

This would be written in two parts: a small-as-possible binary seed bootloader, and the actual VM. Both parts would be written in Onramp hexadecimal x86 machine code.

The boot media would be a device with an MBR partition formatted with vFAT. The bootloader would have to be converted to binary beforehand and flashed to the MBR (this is the "trusted" binary seed.) The actual VM would be kept as hexadecimal source code in a file on the root of the device, along with the uncompressed Onramp source tarball and other desired source tarballs.

The bootloader would find the VM file on the filesystem, unhex it into memory, and jump to it. The VM can then load the Onramp tarball and other tarballs into memory, unhex the Onramp OS and other tools, and launch the OS which bootstraps Onramp.

See the [Onramp OS](../../../core/os) for details on how the rest of this will work.

Since the Onramp OS will implement its own in-memory filesystem, we don't need to implement any filesystem syscalls in this VM. This VM therefore might be even simpler than a typical hosted VM.

We already have an [x86\_64 Linux VM](../x86_64-linux) so this should be fairly quick to write. We just need to reduce it to x86, get rid of all the Linux syscalls, and replace the ELF header with a BIOS bootloader. The only real BIOS calls we need are to find a large region of memory, read sectors off the disk, and print logs to the screen. The biggest problem is that we need to write the Onramp OS stages first.
