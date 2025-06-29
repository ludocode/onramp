#!/usr/bin/env python

# The MIT License (MIT)
#
# Copyright (c) 2023-2025 Fraser Heavy Software
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.



# This is an Onramp VM implemented in Python.
#
# All values are unsigned. Registers store 32-bit unsigned values. All
# functions (including mix()) return unsigned values.
#
# Error checking is mostly omitted in order to keep it simple and to improve
# performance. Some errors (like an out-of-bounds memory access or an invalid
# file handle) will result in a Python exception. Other errors (such as invalid
# opcodes or invalid register arguments) will produce nonsense results.
#
# There are a handful of optimizations below, some of which compromise
# readability. Nevertheless, this VM is about 35x slower than a handwritten
# machine code VM, and about 100x slower than a VM in a compiled language.
# This is probably as good as it will get. Python is just very slow
# unfortunately.



from __future__ import print_function

import sys, os, struct, traceback, time

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

def fatal(message, address=None):
    raise Exception(message)

# register names
RSP = 0xC
RFP = 0xD
RPP = 0xE
RIP = 0xF

# Memory and registers
memory = bytearray(2**24)
registers = [0] * 16

# File handles.
handles = [
    hasattr(sys.stdin, "buffer") and sys.stdin.buffer or sys.stdin,
    hasattr(sys.stdout, "buffer") and sys.stdout.buffer or sys.stdout,
    hasattr(sys.stderr, "buffer") and sys.stderr.buffer or sys.stderr,
] + [None] * 13

# int view of byte array. Use memoryview.cast() if we have it (added in Python
# 3.3); fallback to a manual implementation if we don't.
if hasattr(memoryview, "cast"):
    memory_ints = memoryview(memory).cast("@I")
else:
    class IntMemoryWrapper(object):
        def __init__(self, memory):
            self.memory = memory
        def __getitem__(self, index):
            address = index << 2
            return struct.unpack("<I", self.memory[address:address+4])[0]
        def __setitem__(self, index, value):
            address = index << 2
            self.memory[address:address+4] = struct.pack("<I", value)
    memory_ints = IntMemoryWrapper(memory)

# memory layout
BASE_ADDR = 0x10000
MEMORY_SIZE = len(memory)

# error codes
VM_ERR_GENERIC     = 0xFFFFFFFF
VM_ERR_PATH        = 0xFFFFFFFE
VM_ERR_IO          = 0xFFFFFFFD
VM_ERR_UNSUPPORTED = 0xFFFFFFFC

# syscalls
SYSCALL_COUNT = 23

def loadByte(address):
    return memory[(address & 0xFFFFFFFF) - BASE_ADDR]

def storeByte(address, value):
    # Workaround for str/bytes in Python 2
    if type(value) == type(""):
        value = ord(value)
    memory[(address & 0xFFFFFFFF) - BASE_ADDR] = (value & 0xFF)

def loadWord(address):
    return memory_ints[((address & 0xFFFFFFFF) - BASE_ADDR) >> 2]

def storeWord(address, value):
    memory_ints[((address & 0xFFFFFFFF) - BASE_ADDR) >> 2] = value & 0xFFFFFFFF

def loadString(address):
    s = bytearray()
    while True:
        x = loadByte(address)
        if x == 0:
            break
        s.append(x)
        address += 1
    return s.decode("utf-8", "replace")

def mix(value):
    if value >= 0x80 and value <= 0x8F:
        return registers[value & 0xF]
    if value >= 0x90:
        # Note we're returning 32-bit unsigned. We do sign extension manually.
        return value | 0xFFFFFF00
    return value

def syscall_exit():
    sys.exit(registers[0])

def syscall_time():
    addr = registers[0]
    curtime = time.time()
    storeWord(addr, int(curtime))
    storeWord(addr + 4, int(curtime) >> 32)
    storeWord(addr + 8, int((curtime * 1000000000) % 1000000000))
    registers[0] = 0

def syscall_fopen():
    for i in range(len(handles)):
        if handles[i] is None:
            try:
                handles[i] = open(loadString(registers[0]), registers[1] and "wb" or "rb")
                registers[0] = i
            except:
                # TODO: VM_ERR_PATH is for file not found, and other errors
                # should be VM_ERR_IO. We could catch FileNotFoundError but
                # this is Python 3 only. For now we just give the generic
                # error; this is allowed in the Onramp spec.
                registers[0] = VM_ERR_GENERIC
            return
    registers[0] = VM_ERR_GENERIC

def syscall_fclose():
    # Shouldn't be able to close standard streams, we don't bother to check
    handles[registers[0]].close()
    handles[registers[0]] = None
    registers[0] = 0

def syscall_fread():
    file = handles[registers[0]]
    address = registers[1]
    count = registers[2]
    i = 0
    while i < count:
        b = file.read(count - i)
        # TODO try to handle errors gracefully. A read at EOF will return a
        # size of 0 which we handle correctly but a read error will throw
        # an exception. For now we let it take down the whole VM.
        if not b:
            break
        for j in range(len(b)):
            memory[((address + i + j) & 0xFFFFFFFF) - BASE_ADDR] = b[j]
        i += len(b)
    registers[0] = i

def syscall_fwrite():
    addr = registers[1] - BASE_ADDR
    handles[registers[0]].write(memory[addr:addr + registers[2]])
    handles[registers[0]].flush()
    # TODO try to handle errors gracefully. For now a write error takes
    # down the whole VM.
    registers[0] = registers[2]

def syscall_fseek():
    file = handles[registers[0]]
    base = registers[1]
    offset = registers[2] | (registers[3] << 32)
    if offset >= 2**63:
        offset = offset - 2**64
    file.seek(offset, base)
    registers[0] = 0

def syscall_ftell():
    position = handles[registers[0]].tell()
    addr = registers[1]
    storeWord(addr, position)
    storeWord(addr + 4, position >> 32)
    registers[0] = 0

def syscall_ftrunc():
    handles[registers[0]].truncate(registers[1] | (registers[2] << 32))
    registers[0] = 0
    return

def syscall_unlink():
    try:
        os.remove(loadString(registers[0]))
        registers[0] = 0
    except:
        registers[0] = VM_ERR_GENERIC

def syscall_chmod():
    try:
        os.chmod(loadString(registers[0]), registers[1])
        registers[0] = 0
    except:
        registers[0] = VM_ERR_GENERIC

syscalls = {
    0: syscall_exit,
    1: syscall_time,
    3: syscall_fopen,
    4: syscall_fclose,
    5: syscall_fread,
    6: syscall_fwrite,
    7: syscall_fseek,
    8: syscall_ftell,
    9: syscall_ftrunc,
    16: syscall_unlink,
    17: syscall_chmod,
}

def run():

    # These local aliases seem to improve performance
    memory = globals()["memory"]
    memory_ints = globals()["memory_ints"]
    registers = globals()["registers"]
    mix = globals()["mix"]

    while True:

        # Load the instruction.
        # Note that we ignore the upper 4 bits of the opcode. We assume it
        # starts with 0x7. Skipping this check gives a ~5% performance
        # improvement.
        offset = registers[RIP] - BASE_ADDR
        opcode = memory[offset] & 0xF
        a = memory[offset + 1]
        b = memory[offset + 2]
        c = memory[offset + 3]

        # Debug helpers
        #print(f"rip {hex(registers[RIP])} {hex(memory[offset])[2:]}" +
        #        f" {hex(a)[2:]} {hex(b)[2:]} {hex(c)[2:]}", file=sys.stderr)
        #if memory[offset] & 0xF0 != 0x70:
        #    print(f"Invalid instruction at {hex(offset)}")
        #    sys.exit(125)

        registers[RIP] += 4

        # Note also that we ignore the upper 4 bits of any destination
        # register below. We assume it starts with 0x8.

        # Python doesn't optimize a flat sequence of if statements (and the
        # match statement is compiled to an if sequence) so we manually search
        # for the opcode in blocks of 4. (This also gives a ~5% performance
        # improvement. A full binary search is not significantly faster and
        # makes this much harder to read.)

        if opcode < 4:
            if opcode == 0:
                registers[a & 0xF] = (mix(b) + mix(c)) & 0xFFFFFFFF    # add
            elif opcode == 1:
                registers[a & 0xF] = (mix(b) - mix(c)) & 0xFFFFFFFF    # sub
            elif opcode == 2:
                registers[a & 0xF] = (mix(b) * mix(c)) & 0xFFFFFFFF    # mul
            else:
                registers[a & 0xF] = (mix(b) // mix(c)) & 0xFFFFFFFF   # divu
        elif opcode < 8:
            if opcode == 4:
                registers[a & 0xF] = (mix(b) & mix(c))                 # and
            elif opcode == 5:
                registers[a & 0xF] = (mix(b) | mix(c))                 # or
            elif opcode == 6:
                registers[a & 0xF] = (mix(b) << mix(c)) & 0xFFFFFFFF   # shl
            else:
                registers[a & 0xF] = (mix(b) >> mix(c))                # shru
        elif opcode < 12:
            if opcode == 8:
                registers[a & 0xF] = memory_ints[(((mix(b) + mix(c)) & 0xFFFFFFFF) - BASE_ADDR) >> 2]    # ldw
            elif opcode == 9:
                memory_ints[(((mix(b) + mix(c)) & 0xFFFFFFFF) - BASE_ADDR) >> 2] = mix(a)  # stw
            elif opcode == 10:
                registers[a & 0xF] = memory[((mix(b) + mix(c)) & 0xFFFFFFFF) - BASE_ADDR]  # ldb
            else:
                memory[((mix(b) + mix(c)) & 0xFFFFFFFF) - BASE_ADDR] = mix(a) & 0xFF  # stb
        else:
            if opcode == 12:
                registers[a & 0xF] = (registers[a & 0xF] & 0xFFFF) << 16 | b | c << 8  # ims
            elif opcode == 13:
                registers[a & 0xF] = (mix(b) < mix(c)) and 1 or 0  # ltu
            elif opcode == 14:
                # A bit of magic here to do sign extension without branching.
                # This is the same algorithm the assembler does to implement
                # the sxs instruction. It might be faster to just branch.
                if 0 == mix(a): registers[RIP] = (registers[RIP] +
                        ((0x7FFF - ((0x7FFF - (b | c << 8)) & 0xFFFF)) << 2)) & 0xFFFFFFFF  # jz
            else:
                syscall = registers[RPP]
                if syscall not in syscalls:
                    raise Exception("Invalid opcode or unsupported syscall.")
                syscalls[syscall]()
                registers[RIP] = loadWord(registers[RSP])

def initialize():
    # We use breakAddress as a cursor into the heap where we append data.
    # Python 2 doesn't support nonlocal so we just make this global.
    global breakAddress

    # Parse args
    args = sys.argv
    if len(args) < 2:
        raise Exception("A program filename is required.")
    filename = args[1]
    args = args[1:]

    # Make space for the process info table
    breakAddress = BASE_ADDR
    tableAddress = breakAddress
    breakAddress += 4 * 10

    # Helper to copy string into VM heap
    def copyString(string):
        global breakAddress
        stringAddress = breakAddress
        for b in string.encode("UTF-8") + b'\0':
            storeByte(breakAddress, b)
            breakAddress += 1
        return stringAddress

    # Helper to copy string table to VM heap
    def copyStrings(strings):
        global breakAddress
        tableAddress = breakAddress
        breakAddress += (len(strings) + 1) * 4
        for i in range(len(strings)):
            storeWord(tableAddress + i * 4, copyString(strings[i]))
        storeWord(tableAddress + len(strings) * 4, 0)
        breakAddress = (breakAddress + 3) & ~3  # keep memory position aligned
        return tableAddress

    # Copy args, env vars, working directory to VM heap
    argsAddress = copyStrings(args)
    envAddress = copyStrings([key + "=" + value for key, value in os.environ.items()])
    dirAddress = copyString(os.getcwd())
    breakAddress = (breakAddress + 3) & ~3  # keep memory position aligned

    # Put a 0x7F opcode into mapped memory. We'll use this to detect syscalls.
    # (This is faster than checking for a fixed address on every instruction.)
    syscallAddress = breakAddress
    breakAddress += 4
    storeByte(syscallAddress, 0x7F)

    # Write syscall table
    syscallTableAddress = breakAddress
    breakAddress += SYSCALL_COUNT * 8
    for i in syscalls:
        storeWord(syscallTableAddress + i * 8, syscallAddress)  # rip
        storeWord(syscallTableAddress + i * 8 + 4, i)           # rpp

    # Write halt bytecode into VM heap
    haltAddress = breakAddress
    breakAddress += 4
    storeWord(haltAddress, 0x0000007F)

    # Load program into VM heap
    programAddress = breakAddress
    with open(filename, "rb") as f:
        for b in f.read():
            storeByte(breakAddress, b)
            breakAddress += 1

    # Skip any #! or REM wrap header
    programIndex = programAddress - BASE_ADDR
    if memory[programIndex:programIndex + 2] == b"#!" or \
            memory[programIndex:programIndex + 3] == b"REM":
        programAddress += 128

    # Initialize registers
    registers[RPP] = programAddress
    registers[RIP] = programAddress
    registers[0] = tableAddress
    registers[RSP] = BASE_ADDR + MEMORY_SIZE

    # Fill process info table
    storeWord(tableAddress, 2) # version
    storeWord(tableAddress + 4, breakAddress)  # program break
    storeWord(tableAddress + 8, syscallTableAddress)  # syscall table
    storeWord(tableAddress + 12, 0)  # input stream handle
    storeWord(tableAddress + 16, 1)  # output stream handle
    storeWord(tableAddress + 20, 2)  # error stream handle
    storeWord(tableAddress + 24, argsAddress)  # command-line args
    storeWord(tableAddress + 28, envAddress)   # environment vars
    storeWord(tableAddress + 32, dirAddress)   # working directory
    storeWord(tableAddress + 36, 7)  # capabilities = echo | blocking | line-oriented

if __name__ == "__main__":
    try:
        initialize()
        run()
    except SystemExit:
        raise
    except:
        # Any VM failure should result in an exit code of 125.
        traceback.print_exc()
        sys.exit(125)
