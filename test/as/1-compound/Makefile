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

SRC=$(ROOT)/core/as/1-compound/src
OUT=$(BUILD)/as-1-compound

VM=$(BUILD)/vm-c-debugger/vm
LD=$(BUILD)/ld-2-full/ld
AS=$(BUILD)/as-2-full/as
# TODO libc/2-full not buildable yet
#LIBC=$(BUILD)/libc-2-full/libc.oa
LIBC=$(BUILD)/libc-0-oo/libc.oa
# TODO libo/1-opc not buildable yet
LIBO=$(BUILD)/libo-0-oo/libo.oa

OBJS=\
	$(OUT)/emit.oo \
	$(OUT)/main.oo \
	$(OUT)/op_arithmetic.oo \
	$(OUT)/op_control.oo \
	$(OUT)/op_logic.oo \
	$(OUT)/op_memory.oo \
	$(OUT)/opcodes.oo \
	$(OUT)/parse.oo \

all: build test FORCE
FORCE:
build: $(OUT)/as.oe FORCE

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

$(LIBO): FORCE
	# TODO libo/1-opc not buildable yet
	#@(make -C ../../libo/1-opc build)
	@(make -C ../../libo/0-oo build)

$(OUT): FORCE
	@mkdir -p $(OUT)

$(OUT)/emit.oo: $(SRC)/emit.os $(OUT) $(AS)
	$(AS) $(SRC)/emit.os -o $@
$(OUT)/main.oo: $(SRC)/main.os $(OUT) $(AS)
	$(AS) $(SRC)/main.os -o $@
$(OUT)/op_arithmetic.oo: $(SRC)/op_arithmetic.os $(OUT) $(AS)
	$(AS) $(SRC)/op_arithmetic.os -o $@
$(OUT)/op_control.oo: $(SRC)/op_control.os $(OUT) $(AS)
	$(AS) $(SRC)/op_control.os -o $@
$(OUT)/op_logic.oo: $(SRC)/op_logic.os $(OUT) $(AS)
	$(AS) $(SRC)/op_logic.os -o $@
$(OUT)/op_memory.oo: $(SRC)/op_memory.os $(OUT) $(AS)
	$(AS) $(SRC)/op_memory.os -o $@
$(OUT)/opcodes.oo: $(SRC)/opcodes.os $(OUT) $(AS)
	$(AS) $(SRC)/opcodes.os -o $@
$(OUT)/parse.oo: $(SRC)/parse.os $(OUT) $(AS)
	$(AS) $(SRC)/parse.os -o $@

$(OUT)/as.oe: $(OBJS) Makefile $(LD) $(LIBC) $(LIBO)
	@rm -f $@
	$(LD) -g $(LIBC) $(LIBO) $(OBJS) -o $@

test-0: build FORCE $(VM)
	../run.sh --other-stage ../0-basic $(VM) $(OUT)/as.oe

test-1: build FORCE $(VM)
	../run.sh . $(VM) $(OUT)/as.oe

test: test-0 test-1 FORCE

generate: build FORCE $(VM)
	../generate.sh . $(VM) $(OUT)/as.oe
