# The MIT License (MIT)
#
# Copyright (c) 2024 Fraser Heavy Software
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

SRC=$(ROOT)/core/libc/1-omc/src
OUT=$(BUILD)/libc-1-omc

TOOL_VM=$(BUILD)/vm-c-debugger/vm
TOOL_AR=$(BUILD)/ar-0-cat/ar.oe    # TODO ar/1
TOOL_CC=$(BUILD)/cc/cc
TOOL_CPP=$(BUILD)/cpp-1-omc/cpp    # TODO cpp/2
TOOL_CCI=$(BUILD)/cci-1-opc/cci    # TODO cci/2
TOOL_AS=$(BUILD)/as-2-full/as
TOOL_LD=$(BUILD)/ld-2-full/ld
TOOLS=$(TOOL_VM) $(TOOL_AR) $(TOOL_CC) $(TOOL_CCI) $(TOOL_CPP) $(TOOL_AS) $(TOOL_LD)

OBJS=\
		$(ROOT)/core/libc/0-oo/src/start.oo \
		\
		$(ROOT)/core/libc/0-oo/src/ctype.oo \
		$(ROOT)/core/libc/0-oo/src/environ.oo \
		$(ROOT)/core/libc/0-oo/src/errno.oo \
		$(ROOT)/core/libc/0-oo/src/malloc_util.oo \
		$(ROOT)/core/libc/0-oo/src/spawn.oo \
		$(ROOT)/core/libc/0-oo/src/stdio.oo \
		$(ROOT)/core/libc/0-oo/src/string.oo \
		\
		$(OUT)/malloc.oo \
		$(OUT)/strrchr.oo \
		$(OUT)/strtol.oo \

# TODO remove -D__onramp_cci_opc__ etc. once we have full cci, full cpp
CCARGS=\
	    -with-cpp=$(TOOL_CPP) \
	    -with-cci=$(TOOL_CCI) \
	    -with-as=$(TOOL_AS) \
	    -with-ld=$(TOOL_LD) \
		-nostdinc \
		-nostdlib \
		-D__onramp_cci_opc__=1 \
		-D__onramp_libc_omc__=1 \
		-I$(ROOT)/core/libc/common/include \
		-include __onramp/__predef.h

all: build test FORCE
FORCE:
build: $(OUT)/libc.oa FORCE

clean: FORCE
	rm -rf $(BUILD)/libc-1-omc

# tools
$(TOOL_VM): FORCE
	@(make -C $(ROOT)/platform/vm/c-debugger build)
$(TOOL_AR): FORCE
	@(make -C ../../ar/0-cat build)    # TODO ar/1
$(TOOL_CC): FORCE
	@(make -C ../../cc build)
$(TOOL_CCI): FORCE
	@(make -C ../../cci/1-opc build)   # TODO cci/2 build
$(TOOL_CPP): FORCE
	@(make -C ../../cpp/1-omc build)   # TODO cpp/2 build
$(TOOL_AS): FORCE
	@(make -C ../../as/2-full build)   # TODO as/1 build
$(TOOL_LD): FORCE
	@(make -C ../../ld/2-full build)   # TODO ld/1 build


$(OUT)/malloc.oo: $(SRC)/malloc.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(TOOL_CC) $(CCARGS) -c $(SRC)/malloc.c -o $@

$(OUT)/strrchr.oo: $(SRC)/strrchr.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(TOOL_CC) $(CCARGS) -c $(SRC)/strrchr.c -o $@

$(OUT)/strtol.oo: $(SRC)/strtol.c Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(TOOL_CC) $(CCARGS) -c $(SRC)/strtol.c -o $@


$(OUT)/libc.oa: Makefile $(TOOLS) $(OBJS)
	@rm -f $@
	@mkdir -p $(OUT)
	$(TOOL_VM) $(TOOL_AR) rc $@ $(OBJS)

test: build FORCE $(VM)
	../run.sh ../0-oo omc $(OUT)/libc.oa
	../run.sh .       omc $(OUT)/libc.oa
