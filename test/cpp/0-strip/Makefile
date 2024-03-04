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

ROOT=../../..
BUILD=$(ROOT)/build/test

SRC=$(ROOT)/core/cpp/0-strip
OUT=$(BUILD)/cpp-0-strip

VM=$(BUILD)/vm-c-debugger/vm
LD=$(BUILD)/ld-2-full/ld
AS=$(BUILD)/as-2-full/as
# TODO libc/2-full not buildable yet
#LIBC=$(BUILD)/libc-2-full/libc.oa
LIBC=$(BUILD)/libc-0-oo/libc.oa

all: build test FORCE
FORCE:
build: $(OUT)/cpp.oe FORCE

clean: FORCE
	rm -rf $(OUT)

$(VM): FORCE
	@(make -C $(ROOT)/platform/vm/c-debugger build)

$(LD): FORCE
	@(make -C ../../ld/2-full build)

$(AS): FORCE
	@(make -C ../../as/2-full build)

$(LIBC): FORCE
	# TODO libc/2-full not buildable yet
	#@(make -C ../../libc/2-full build)
	@(make -C ../../libc/0-oo build)

$(OUT): FORCE
	@mkdir -p $(OUT)

$(OUT)/cpp.oo: $(SRC)/cpp.os $(OUT) $(AS)
	$(AS) $(SRC)/cpp.os -o $@

$(OUT)/cpp.oe: $(OUT)/cpp.oo Makefile $(LD) $(LIBC)
	@rm -f $@
	$(LD) -g $(LIBC) $(OUT)/cpp.oo -o $@

test: build FORCE $(VM)
	../run.sh . $(VM) $(OUT)/cpp.oe
