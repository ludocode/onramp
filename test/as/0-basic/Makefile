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

SRC=$(ROOT)/core/as/0-basic
OUT=$(BUILD)/as-0-basic

VM=$(BUILD)/vm-c-debugger/vm
LD=$(BUILD)/ld-2-full/ld
# TODO libc/3-full not buildable yet
#LIBC=$(BUILD)/libc-3-full/libc.oa
LIBC=$(BUILD)/libc-0-oo/libc.oa
# TODO libo/1-opc not buildable yet
LIBO=$(BUILD)/libo-0-oo/libo.oa

all: build test FORCE
FORCE:
build: $(OUT)/as.oe FORCE

clean: FORCE
	rm -rf $(OUT)

$(VM): FORCE
	@(make -C $(ROOT)/platform/vm/c-debugger build)

$(LD): FORCE
	@(make -C ../../ld/2-full build)

$(LIBC): FORCE
	# TODO libc/3-full not buildable yet
	#@(make -C ../../libc/3-full build)
	@(make -C ../../libc/0-oo build)

$(LIBO): FORCE
	# TODO libo/1-opc not buildable yet
	#@(make -C ../../libo/1-opc build)
	@(make -C ../../libo/0-oo build)

$(OUT)/as.oe: $(SRC)/as.oo Makefile $(LD) $(LIBC) $(LIBO)
	@rm -f $@
	@mkdir -p $(OUT)
	$(LD) -g $(LIBC) $(LIBO) $(SRC)/as.oo -o $@

test: build FORCE $(VM)
	../run.sh . $(VM) $(OUT)/as.oe

generate: build FORCE $(VM)
	../generate.sh . $(VM) $(OUT)/as.oe
