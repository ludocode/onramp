#!/usr/bin/env python

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

# This is a full-featured Python implementation of the Onramp hex tool. It
# implements all error checks and provides rich error messages with line
# numbers and expected addresses. It passes all tests.

import sys

def fatal(message):
    print("ERROR: " + message, file=sys.stderr)
    sys.exit(1)

USAGE = "Usage: " + sys.argv[0] + " <input> -o <output>"

# Parse arguments
if len(sys.argv) != 4:
    fatal(USAGE)
if sys.argv[1] == "-o":
    # ./hex -o <out> <in>
    outPath = sys.argv[2]
    inPath = sys.argv[3]
elif sys.argv[2] == "-o":
    # ./hex <in> -o <out>
    inPath = sys.argv[1]
    outPath = sys.argv[3]
else:
    fatal(USAGE)

address = 0
hexByte = ""

with open(inPath, "r") as inFile:
    with open(outPath, "wb") as outFile:
        lineNum = 0
        for line in inFile.readlines():
            lineNum += 1

            # Check for backslash at end of line
            if line.strip("\n\r")[-1:] == "\\":
                fatal("Backslash forbidden at end of line " + lineNum)

            for i, c in enumerate(line):

                # Check for ASCII
                if (c < chr(0x09) or c > chr(0x0D)) and (c < chr(0x20) or c > chr(0x7E)):
                    fatal("Invalid character in input file on line " + str(lineNum))

                # Convert hex bytes
                if c.isalnum() and (not c.isalpha() or c.lower() <= "f"):
                    hexByte += c
                    if len(hexByte) == 2:
                        outFile.write(bytes([int(hexByte, 16)]))
                        address += 1
                        hexByte = ""
                elif hexByte != "":
                    fatal("Incomplete hex byte on line " + str(lineNum))

                # Skip comment lines
                elif c in ("#", ";"):
                    break

                # Check address assertions
                elif c == "@":
                    s = line[i + 1:].split()[0]
                    if len(s) < 2 or len(s) > 10 or s[:2].lower() != "0x":
                        fatal("Invalid address assertion on line " + str(lineNum))
                    try:
                        expected = int(s[2:], 16)
                    except:
                        fatal("Invalid address assertion on line " + str(lineNum))
                    if address != expected:
                        fatal("Failed address assertion on line " + str(lineNum) + ": " +
                              "expected: 0x" + hex(expected) + ", actual: 0x" + hex(address))
                    break

            # Make sure a hex byte cannot span multiple lines...
            if hexByte != "":
                fatal("Truncated hex byte on line " + str(lineNum))

        # ...and the file cannot end in half a hex byte
        if hexByte != "":
            fatal("Truncated hex byte on line " + str(lineNum))
