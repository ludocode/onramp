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

SRC=$(ROOT)/core/libc/0-oo
OUT=$(BUILD)/libc-0-oo

VM=$(BUILD)/vm-c-debugger/vm
# TODO ar/1-unix not written yet
#AR=$(BUILD)/ar-1-unix/ar
AR=$(BUILD)/ar-0-cat/ar.oe

LIBC_FILES=\
		$(ROOT)/core/libc/0-oo/src/start.oo \
		$(ROOT)/core/libc/0-oo/src/ctype.oo \
		$(ROOT)/core/libc/0-oo/src/stdio.oo \
		$(ROOT)/core/libc/0-oo/src/stdlib_malloc.oo \
		$(ROOT)/core/libc/0-oo/src/stdlib_util.oo \
		$(ROOT)/core/libc/0-oo/src/string.oo

all: build test FORCE
FORCE:
build: $(OUT)/libc.oa FORCE

clean: FORCE
	rm -rf $(BUILD)/libc-0-oo

$(VM): FORCE
	@(make -C $(ROOT)/platform/vm/c-debugger build)

$(AR): FORCE
	# TODO ar/1-unix not written yet
	#@(make -C ../../ar/1-unix build)
	@(make -C ../../ar/0-cat build)

$(OUT)/libc.oa: $(LIBC_FILES) Makefile $(VM) $(AR)
	@rm -f $@
	@mkdir -p $(OUT)
	$(VM) $(AR) rc $@ $(LIBC_FILES)

test: build FORCE $(VM)
	../run.sh . $(OUT)/libc.oa -I$(ROOT)/core/libc/0-oo/include
