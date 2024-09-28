#!/usr/bin/env python3

# The MIT License (MIT)
#
# Copyright (c) 2023-2024 Fraser Heavy Software
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
# functions (including parseMix()) return unsigned values.
#
# Error checking is minimal in order to keep it simple and improve performance.
# Some errors (like an out-of-bounds memory access or an invalid file handle)
# will result in a Python exception. Other errors (such as unaligned
# load/store) may work for some addresses and not others.
#
# There are a handful of optimizations below, some of which compromise
# readability. Nevertheless, this VM is about 50x slower than a handwritten
# machine code VM, and about 100x slower than a VM in a compiled language.
# This is probably as good as it will get. Python is just very slow
# unfortunately.



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

# vm state
memory = bytearray(2**24)
registers = [0] * 16
handles = [sys.stdin.buffer, sys.stdout.buffer, sys.stderr.buffer] + [None] * 13

# memory layout
BASE_ADDR = 0x10000
MEMORY_SIZE = len(memory)

# error codes
VM_ERR_GENERIC     = 0xFFFFFFFF
VM_ERR_PATH        = 0xFFFFFFFE
VM_ERR_IO          = 0xFFFFFFFD
VM_ERR_UNSUPPORTED = 0xFFFFFFFC

def loadByte(address):
    return memory[(address & 0xFFFFFFFF) - BASE_ADDR]

def storeByte(address, value):
    memory[(address & 0xFFFFFFFF) - BASE_ADDR] = (value & 0xFF)

def loadWord(address):
    address = ((address & 0xFFFFFFFF) - BASE_ADDR)
    return struct.unpack("<I", memory[address:address+4])[0]

def storeWord(address, value):
    address = ((address & 0xFFFFFFFF) - BASE_ADDR)
    memory[address:address+4] = struct.pack("<I", value)

def loadString(address):
    s = bytearray()
    while True:
        x = loadByte(address)
        if x == 0:
            break
        s.append(x)
        address += 1
    return s.decode("utf-8", "replace")

def parseMix(value):
    if value >= 0x80 and value <= 0x8F:
        return registers[value & 0xF]
    if value >= 0x90:
        # Note we're returning 32-bit unsigned. We do sign extension manually.
        return value | 0xFFFFFF00
    return value

def parseRegister(value):
    if value >= 0x80 and value <= 0x8F:
        return value & 0xF
    raise Exception("Argument is not a register")

def syscall(number):
    if number == 0x00:  # halt
        sys.exit(registers[0])

    if number == 0x01:  # time
        addr = registers[0]
        curtime = time.time()
        storeWord(addr, int(curtime) & 0xFFFFFFFF)
        storeWord(addr + 1, (int(curtime) >> 32) & 0xFFFFFFFF)
        storeWord(addr, (curtime * 1000000000) % 1000000000)
        registers[0] = 0
        return

    if number == 0x02:  # spawn
        raise Exception("spawn syscall not yet implemented")

    if number == 0x03:  # fopen
        for i in range(len(handles)):
            if handles[i] is None:
                try:
                    handles[i] = open(loadString(registers[0]), registers[1] and "wb" or "rb")
                    registers[0] = i
                except FileNotFoundError:
                    registers[0] = VM_ERR_PATH
                return
        registers[0] = VM_ERR_GENERIC
        return

    if number == 0x04:  # fclose
        # Shouldn't be able to close standard streams, we don't bother to check
        handles[registers[0]].close()
        handles[registers[0]] = None
        return

    if number == 0x05:  # read
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
                storeByte(address + i + j, b[j])
            i += len(b)
        registers[0] = i
        return

    if number == 0x06:  # fwrite
        addr = registers[1] - BASE_ADDR
        handles[registers[0]].write(memory[addr:addr + registers[2]])
        # TODO try to handle errors gracefully. For now a write error takes
        # down the whole VM.
        registers[0] = registers[2]
        return

    if number == 0x07:  # fseek
        file = handles[registers[0]]
        base = registers[1]
        offset = registers[2] | (registers[3] << 32)
        if offset >= 2**63:
            offset = offset - 2**64
        file.seek(offset, base)
        registers[0] = 0
        return

    if number == 0x08:  # ftell
        position = handles[registers[0]].tell()
        addr = registers[1]
        storeWord(addr, position)
        storeWord(addr + 4, position >> 32)
        registers[0] = 0
        return

    if number == 0x09:  # ftrunc
        handles[registers[0]].truncate(registers[1] | (registers[2] << 32))
        registers[0] = 0
        return

    if number == 0x10:  # unlink
        try:
            os.remove(loadString(registers[0]))
            registers[0] = 0
        except:
            registers[0] = VM_ERR_GENERIC
        return

    if number == 0x11:  # chmod
        try:
            os.chmod(loadString(registers[0]), registers[1])
            registers[0] = 0
        except:
            registers[0] = VM_ERR_GENERIC
        return

    raise Exception("Invalid or unimplemented syscall: " + str(number))

def run():

    # These local aliases seem to improve performance
    memory = globals()["memory"]
    registers = globals()["registers"]
    parseMix = globals()["parseMix"]
    parseRegister = globals()["parseRegister"]
    loadWord = globals()["loadWord"]
    loadByte = globals()["loadByte"]
    storeWord = globals()["storeWord"]
    storeByte = globals()["storeByte"]
    syscall = globals()["syscall"]

    # We've also inlined the loadByte() function in a handful of places below,
    # basically anywhere you see BASE_ADDR. It's much quicker to access memory
    # and registers directly.

    while True:

        opcode = memory[registers[RIP] - BASE_ADDR]
        registers[RIP] += 4

        if (opcode & 0xF0) != 0x70:
            raise Exception("Invalid opcode: " + str(opcode) + " addr " + str(registers[RIP]-4))
        opcode = opcode & 0xF
        mask = 1 << opcode

        # Eleven basic instructions share the same argument types: they all take a
        # destination register and two mix-type arguments. We handle them all
        # together here.
        if mask & 0b0010010111111111:
            dest = parseRegister(memory[registers[RIP] - BASE_ADDR - 3])
            left = parseMix(memory[registers[RIP] - BASE_ADDR - 2])
            right = parseMix(memory[registers[RIP] - BASE_ADDR - 1])
            if opcode == 0: # add
                registers[dest] = (left + right) & 0xFFFFFFFF
            elif opcode == 1: # sub
                registers[dest] = (left - right) & 0xFFFFFFFF
            elif opcode == 2: # mul
                registers[dest] = (left * right) & 0xFFFFFFFF
            elif opcode == 3: # divu
                registers[dest] = int(left / right)
            elif opcode == 4: # and
                registers[dest] = left & right
            elif opcode == 5: # or
                registers[dest] = left | right
            elif opcode == 6: # xor
                registers[dest] = left ^ right
            elif opcode == 7: # ror
                right &= 0x1F
                registers[dest] = (0xFFFFFFFF &
                        ((left >> right) | (left << ((-right) & 0x1F))))
            elif opcode == 8: # ldw
                registers[dest] = loadWord(left + right)
            elif opcode == 0xA: # ldb
                registers[dest] = loadByte(left + right)
            elif opcode == 0xD: # cmpu
                if left < right:
                    registers[dest] = 0xFFFFFFFF
                elif left > right:
                    registers[dest] = 1
                else:
                    registers[dest] = 0
            else:
                raise Exception("Internal error")

        # Our two store instructions take three mix-type arguments.
        elif mask & 0b0000101000000000:
            value = parseMix(memory[registers[RIP] - BASE_ADDR - 3])
            base = parseMix(memory[registers[RIP] - BASE_ADDR - 2])
            offset = parseMix(memory[registers[RIP] - BASE_ADDR - 1])
            if opcode == 9: # stw
                storeWord(base + offset, value)
            elif opcode == 0xB: # stb
                storeByte(base + offset, value & 0xFF)
            else:
                raise Exception("Internal error")

        # Immediate short and jump if zero both take two literal bytes.
        elif mask & 0b0101000000000000:
            value = memory[registers[RIP] - BASE_ADDR - 2] | \
                    (memory[registers[RIP] - BASE_ADDR - 1] << 8)
            arg1 = memory[registers[RIP] - BASE_ADDR - 3]
            if opcode == 0xC: # ims
                dest = parseRegister(arg1)
                registers[dest] = ((registers[dest] << 16) & 0xFFFFFFFF) | value
            elif opcode == 0xE: # jz
                if 0 == parseMix(arg1):
                    if value > 0x7FFF:
                        #value |= 0xFFFF0000
                        value -= 0x10000
                    registers[RIP] += value * 4
            else:
                raise Exception("Internal error")

        # System call takes only a single argument, the syscall number. The other
        # arguments must be zero (but we don't bother to check.)
        elif opcode == 0xF: # sys
            syscall(loadByte(registers[RIP] - 3))

        else:
            raise Exception("Internal error")

def start():

    # Parse args
    args = sys.argv
    if len(args) < 2:
        raise Exception("A program filename is required.")
    filename = args[1]
    args = args[1:]

    # Make space for the process info table
    pos = BASE_ADDR
    tableAddress = pos
    pos += 4 * 10

    # Helper to copy string into VM heap
    def copyString(string):
        nonlocal pos
        stringAddress = pos
        for b in string.encode("UTF-8") + b'\0':
            storeByte(pos, b)
            pos += 1
        return stringAddress

    # Helper to copy string table to VM heap
    def copyStrings(strings):
        nonlocal pos
        tableAddress = pos
        pos += (len(strings) + 1) * 4
        for i in range(len(strings)):
            storeWord(tableAddress + i * 4, copyString(strings[i]))
        storeWord(tableAddress + len(strings) * 4, 0)
        pos = (pos + 3) & ~3  # keep memory position aligned
        return tableAddress

    # Copy args, env vars, working directory to VM heap
    argsAddress = copyStrings(args)
    envAddress = copyStrings([key + "=" + value for key, value in os.environ.items()])
    dirAddress = copyString(os.getcwd())
    pos = (pos + 3) & ~3  # keep memory position aligned

    # Write halt bytecode into VM heap
    haltAddress = pos
    pos += 4
    storeWord(haltAddress, 0x0000007F)

    # Load program into VM heap
    programAddress = pos
    with open(filename, "rb") as f:
        for b in f.read():
            storeByte(pos, b)
            pos += 1

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
    storeWord(tableAddress, 0) # version
    storeWord(tableAddress + 4, pos)  # break
    storeWord(tableAddress + 8, haltAddress)  # exit address
    storeWord(tableAddress + 12, 0)  # input stream handle
    storeWord(tableAddress + 16, 1)  # output stream handle
    storeWord(tableAddress + 20, 2)  # error stream handle
    storeWord(tableAddress + 24, argsAddress)  # command-line args
    storeWord(tableAddress + 28, envAddress)  # environment vars
    storeWord(tableAddress + 32, dirAddress)  # working directory
    storeWord(tableAddress + 36, 0)  # capabilities

    run()

if __name__ == "__main__":
    try:
        start()
    except SystemExit:
        raise
    except:
        # Any VM failure should result in an exit code of 125.
        traceback.print_exc()
        sys.exit(125)
