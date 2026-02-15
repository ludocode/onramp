#!/usr/bin/env python

# The MIT License (MIT)
#
# Copyright (c) 2025 Fraser Heavy Software
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



# Given an Onramp bytecode program that has corresponding debug info, this
# produces a bytecode and disassembly printout of the program.

import sys

OPCODES = [
    "add", "sub", "mul", "divu",
    "and", "or",  "shl", "shru",
    "ldw", "stw", "ldb", "stb",
    "ims", "ltu", "jz",
]

REGISTERS = [
    "r0",  "r1",  "r2",  "r3",
    "r4",  "r5",  "r6",  "r7",
    "r8",  "r9",  "ra",  "rb",
    "rsp", "rfp", "rpp", "rip",
]

def print_mix(x):
    print(" ", end="")
    if 0x80 <= x <= 0x8F:
        print(REGISTERS[x - 0x80], end="")
    elif x >= 0x90:
        print(x - 256, end="")
    else:
        print(x, end="")

with open(sys.argv[1] + ".od", "r") as debuginfo:
    lines = debuginfo.readlines()
with open(sys.argv[1], "rb") as program:
    bytecode = program.read()

# skip preamble
if len(bytecode) > 128 and (bytecode[:2] == b"#!" or bytecode[:3] == b"REM"):
    bytecode = bytecode[128:]

total = 0
for line in lines:
    if line.startswith("#symbol "):
        print("\n; @" + hex(total), line.split()[1])
    elif line.startswith("#") or line.startswith(";"):
        pass
    else:
        count = int(line)

        for i in range(0, count, 4):
            start = total
            step = min(4, count - i)
            for j in range(step):
                print(" {:02x}".format(bytecode[total]), end="")
                total += 1
            if step == 4:
                opcode = bytecode[start]
                if 0x70 <= opcode <= 0x7E:
                    print("  ; " + OPCODES[opcode - 0x70], end="")
                    print_mix(bytecode[start + 1])
                    if opcode == 0x7C or opcode == 0x7E:
                        value = bytecode[start + 2] + bytecode[start + 3] * 256
                        if opcode == 0x7C: # ims
                            print("", value, end="")
                        elif opcode == 0x7E: # jz
                            if value >= 0x8000:
                                value -= 0x10000
                            print(" {:+d}".format(value), end="")
                    else:
                        print_mix(bytecode[start + 2])
                        print_mix(bytecode[start + 3])
            print()
