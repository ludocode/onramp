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
# Memory is stored as an array of 4K pages starting at the base address. Each
# page is None until accessed.
#
# All values are unsigned. Registers store 32-bit unsigned values. All
# functions (including parseMix()) return unsigned values.
#
# Error checking is minimal in order to keep it simple and improve performance.
# Some errors (like an out-of-bounds memory access or an invalid file handle)
# will result in a Python exception. Other errors (such as unaligned
# load/store) may work for some addresses and not others.
#
# Despite this, the Python VM is around 100x slower than the C89 VM. I have
# some ideas for how to improve this. The obvious one is to eliminate paging.
#
# We could try simply creating a 16MB bytearray on startup. We could also try
# memoryview to read words and bytes rather than using struct and slices.
#
# Another option is to try a plain list of words instead of an array, e.g.
# `memory = [0]*2**22`. This doubles the memory usage but avoids the
# boxing/unboxing penalty of array values. Since most stores and loads are
# words it's probably best to avoid struct/memoryview to access them.



import sys, os, struct, traceback, time
#import cProfile

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

def fatal(message, address=None):
    raise Exception(message)

# debug tracing
traceEnabled = False
traceDepth = 0
def traceInc():
    if traceEnabled:
        global traceDepth
        traceDepth += 1
def traceDec():
    if traceEnabled:
        global traceDepth
        traceDepth -= 1
def trace(*args, **kwargs):
    if traceEnabled:
        eprint("  " * traceDepth, end='')
        eprint(*args, **kwargs)

# register names
RSP = 0xC
RFP = 0xD
RPP = 0xE
RIP = 0xF

# memory layout
BASE_ADDR = 0x10000
PAGE_SIZE = 4096
TOTAL_PAGES = 8192 # 32 MB

# vm state
pages = [None] * TOTAL_PAGES
registers = [0] * 16
handles = [sys.stdin.buffer, sys.stdout.buffer, sys.stderr.buffer] + [None] * 13

# Sets a register to a given value, clamping it to 32-bit unsigned.
def setRegister(register, value):
    #if value < 0:
    #    value += 2**32
    #value &= 0xFFFFFFFF
    #trace("setting register",register,"to",value,hex(value))
    #if (register == 0xF):
        #trace(f"\nCALL/RET {value - BASE_ADDR} hex:{hex(value - BASE_ADDR)}\n")
    registers[register] = value & 0xFFFFFFFF

def getPage(address):
    address &= 0xFFFFFFFF
    #if address < BASE_ADDR or address >= BASE_ADDR + len(pages) * PAGE_SIZE:
    #    raise Exception("Memory address out of range: " + str(address) + " " + hex(address))
    pageNum = (address - BASE_ADDR) // PAGE_SIZE
    if pages[pageNum] is None:
        pages[pageNum] = bytearray(PAGE_SIZE)
    return pages[pageNum]

def loadByte(address):
    return getPage(address)[(address - BASE_ADDR) % PAGE_SIZE]

def storeByte(address, byte):
    getPage(address)[(address - BASE_ADDR) % PAGE_SIZE] = byte

def loadWord(address):
    #if (address % 4) != 0:
    #    raise Exception("Unaligned load at " + str(address))
    offset = (address - BASE_ADDR) % PAGE_SIZE
    return struct.unpack("<I", getPage(address)[offset:offset+4])[0]
#
#    value = page[offset] | (page[offset + 1] << 8) | \
#            (page[offset + 2] << 16) | (page[offset + 3] << 24)
#    #trace("loaded",value,"(hex)",hex(value),"at",address)
#    return value

def storeWord(address, value):
    #if (address % 4) != 0:
    #    raise Exception("Unaligned store at " + str(address))
    #trace("storing word",value,"(hex)",hex(value),"at",address)
    offset = (address - BASE_ADDR) % PAGE_SIZE
    getPage(address)[offset:offset+4] = struct.pack("<I", value)
#    page[offset] = value & 0xFF
#    page[offset + 1] = (value >> 8) & 0xFF
#    page[offset + 2] = (value >> 16) & 0xFF
#    page[offset + 3] = (value >> 24) & 0xFF

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

# Parses the arguments to an instruction where the first argument is the
# destination register and the next two arguments are mix-type source bytes.
# This is the format used by most opcodes.
def parseBinaryOp():
    dest = parseRegister(loadByte(registers[RIP] - 3))
    left = parseMix(loadByte(registers[RIP] - 2))
    right = parseMix(loadByte(registers[RIP] - 1))
    return dest, left, right

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
                    setRegister(0, -1)
                return
        raise Exception("No free file handles")

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
            b = file.read(1)
            if not b:
                break
            storeByte(address + i, b[0])
            i += 1
        registers[0] = i
        return

    if number == 0x06:  # fwrite
        for i in range(registers[2]):
            handles[registers[0]].write(bytes([loadByte(registers[1] + i)]))
        registers[0] = registers[2]
        return

    if number == 0x07:  # fseek
        file = handles[registers[0]]
        offset = registers[1] | registers[2] << 32
        if offset == 2**64-1:
            file.seek(0, 2)
        else:
            file.seek(offset, 0)
        pos = file.tell()
        setRegister(0, pos)
        setRegister(1, pos >> 32)
        return

    if number == 0x10:  # unlink
        try:
            os.remove(loadString(registers[0]))
        except:
            pass #TODO
        return

    if number == 0x11:  # chmod
        try:
            os.chmod(loadString(registers[0], registers[1]))
        except:
            pass #TODO
        return

    raise Exception("Invalid or unimplemented syscall: " + str(number))

def step():
    opcode = loadByte(registers[RIP])
    registers[RIP] += 4
    #trace("")
    #trace(hex(256+opcode)[3:].upper(),
    #      hex(256+loadByte(registers[RIP] - 3))[3:].upper(),
    #      hex(256+loadByte(registers[RIP] - 2))[3:].upper(),
    #      hex(256+loadByte(registers[RIP] - 1))[3:].upper())
    #trace("step", "opcode",hex(opcode)[2:],"reg",registers)

    if (opcode & 0xF0) != 0x70:
        raise Exception("Invalid opcode: " + str(opcode) + " addr " + str(registers[RIP]-4))
    opcode = opcode & 0xF
    mask = 1 << opcode

    # Eleven basic instructions share the same argument types: they all take a
    # destination register and two mix-type arguments. We handle them all
    # together here.
    if mask & 0b0010010111111111:
        dest, left, right = parseBinaryOp()
        #trace(f"{hex(dest)} = {left} <op> {right},  opcode: {hex(0x70+opcode)[2:]}")
        if opcode == 0: # add
            setRegister(dest, left + right)
        elif opcode == 1: # sub
            setRegister(dest, left - right)
        elif opcode == 2: # mul
            setRegister(dest, left * right)
        elif opcode == 3: # div
            setRegister(dest, int(left / right))
        elif opcode == 4: # and
            setRegister(dest, left & right)
        elif opcode == 5: # or
            setRegister(dest, left | right)
        elif opcode == 6: # xor
            setRegister(dest, left ^ right)
        elif opcode == 7: # ror
            right &= 0x1F
            setRegister(dest, 0xFFFFFFFF &
                    (left >> right) | (left << ((-right) & 0x1F)))
        elif opcode == 8: # ldw
            setRegister(dest, loadWord(left + right))
        elif opcode == 0xA: # ldb
            setRegister(dest, loadByte(left + right))
        elif opcode == 0xD: # cmpu
            if left < right:
                setRegister(dest, -1)
            elif left > right:
                setRegister(dest, 1)
            else:
                setRegister(dest, 0)

    # Our two store instructions take three mix-type arguments.
    elif mask & 0b0000101000000000:
        value = parseMix(loadByte(registers[RIP] - 3))
        base = parseMix(loadByte(registers[RIP] - 2))
        offset = parseMix(loadByte(registers[RIP] - 1))
        if opcode == 9: # stw
            storeWord(base + offset, value)
        elif opcode == 0xB: # stb
            storeByte(base + offset, value & 0xFF)

    # Immediate short and jump if zero both take two literal bytes.
    elif mask & 0b0101000000000000:
        value = loadByte(registers[RIP] - 2) | (loadByte(registers[RIP] - 1) << 8)
        if opcode == 0xC: # ims
            dest = parseRegister(loadByte(registers[RIP] - 3))
            setRegister(dest, (registers[dest] << 16) & 0xFFFFFFFF | value)
        elif opcode == 0xE: # jz
            pred = parseMix(loadByte(registers[RIP] - 3))
            if pred == 0:
                if value > 0x7FFF:
                    #value |= 0xFFFF0000
                    value -= 0x10000
                #trace("jumping by " + str(value) + " * 4")
                #setRegister(RIP, registers[RIP] + value * 4)
                registers[RIP] += value * 4
            else:
                pass##trace("pred is not zero, not jumping")

    # System call takes only a single argument, the syscall number. The other
    # arguments must be zero.
    elif opcode == 0xF: # sys
        arg2 = loadByte(registers[RIP] - 2)
        arg3 = loadByte(registers[RIP] - 1)
        if arg2 != 0 or arg3 != 0:
            raise Exception("Extra arguments to syscall instruction must be 0.")
        syscall(loadByte(registers[RIP] - 3))

    else:
        raise Exception()

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

    # Initialize registers
    registers[RPP] = programAddress
    registers[RIP] = programAddress
    registers[0] = tableAddress
    registers[RSP] = BASE_ADDR + PAGE_SIZE * TOTAL_PAGES

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

    # Run
    while True:
        step()

if __name__ == "__main__":
    try:
        #cProfile.run("start()")
        start()
    except SystemExit:
        raise
    except:
        # Any VM failure should result in an exit code of 125.
        traceback.print_exc()
        sys.exit(125)
