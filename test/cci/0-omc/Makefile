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

SRC=$(ROOT)/core/cci/0-omc/src
OUT=$(BUILD)/cci-0-omc

VM=$(BUILD)/vm-c-debugger/vm
LD=$(BUILD)/ld-2-full/ld
AS=$(BUILD)/as-2-full/as
# TODO libc/2-full not buildable yet
#LIBC=$(BUILD)/libc-2-full/libc.oa
LIBC=$(BUILD)/libc-0-oo/libc.oa
# TODO libo/1-opc not buildable yet
LIBO=$(BUILD)/libo-0-oo/libo.oa

all: build test FORCE
FORCE:
build: $(OUT)/cci.oe FORCE

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

$(OUT):
	@mkdir -p $(OUT)

$(OUT)/common.oo: $(SRC)/common.os $(OUT) $(AS)
	$(AS) $(SRC)/common.os -o $@

$(OUT)/compile.oo: $(SRC)/compile.os $(OUT) $(AS)
	$(AS) $(SRC)/compile.os -o $@

$(OUT)/emit.oo: $(SRC)/emit.os $(OUT) $(AS)
	$(AS) $(SRC)/emit.os -o $@

$(OUT)/globals.oo: $(SRC)/globals.os $(OUT) $(AS)
	$(AS) $(SRC)/globals.os -o $@

$(OUT)/lexer.oo: $(SRC)/lexer.os $(OUT) $(AS)
	$(AS) $(SRC)/lexer.os -o $@

$(OUT)/locals.oo: $(SRC)/locals.os $(OUT) $(AS)
	$(AS) $(SRC)/locals.os -o $@

$(OUT)/main.oo: $(SRC)/main.os $(OUT) $(AS)
	$(AS) $(SRC)/main.os -o $@

$(OUT)/parse.oo: $(SRC)/parse.os $(OUT) $(AS)
	$(AS) $(SRC)/parse.os -o $@

$(OUT)/type.oo: $(SRC)/type.os $(OUT) $(AS)
	$(AS) $(SRC)/type.os -o $@

OBJS=\
	 $(OUT)/common.oo \
	 $(OUT)/compile.oo \
	 $(OUT)/emit.oo \
	 $(OUT)/globals.oo \
	 $(OUT)/lexer.oo \
	 $(OUT)/locals.oo \
	 $(OUT)/main.oo \
	 $(OUT)/parse.oo \
	 $(OUT)/type.oo \

$(OUT)/cci.oe: $(OBJS) Makefile $(LD) $(LIBC) $(LIBO)
	$(LD) -g $(LIBC) $(LIBO) $(OBJS) -o $@

test: build FORCE $(VM)
	../run.sh --nonstd . omc $(VM) $(OUT)/cci.oe

generate: build FORCE $(VM)
	../generate.sh . omc $(VM) $(OUT)/cci.oe

debug: build FORCE
	onrampvm $(OUT)/cci.oe /tmp/a.i -o /dev/stdout

# TODO why does this cause rebuilding of assembly files?
#.DELETE_ON_ERROR:
