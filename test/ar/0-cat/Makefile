# The MIT License (MIT)

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

SRC=$(ROOT)/core/ar/0-cat
OUT=$(BUILD)/ar-0-cat

VM=$(BUILD)/vm-c-debugger/vm
LD=$(BUILD)/ld-2-full/ld

# TODO this should just depend on libc/3 which will depend on ar/1

LIBC_FILES=\
		$(ROOT)/core/libc/0-oo/src/start.oo \
		\
		$(ROOT)/core/libc/0-oo/src/ctype.oo \
		$(ROOT)/core/libc/0-oo/src/environ.oo \
		$(ROOT)/core/libc/0-oo/src/errno.oo \
		$(ROOT)/core/libc/0-oo/src/malloc.oo \
		$(ROOT)/core/libc/0-oo/src/malloc_util.oo \
		$(ROOT)/core/libc/0-oo/src/spawn.oo \
		$(ROOT)/core/libc/0-oo/src/stdio.oo \
		$(ROOT)/core/libc/0-oo/src/string.oo

LIBO_FILES=\
		$(ROOT)/core/libo/0-oo/src/libo-error.oo \
		$(ROOT)/core/libo/0-oo/src/libo-util.oo

all: build test FORCE
FORCE:
build: $(OUT)/ar.oe FORCE

clean: FORCE
	rm -rf $(BUILD)/ar-0-cat

$(VM): FORCE
	@(make -C $(ROOT)/platform/vm/c-debugger build)

$(LD): FORCE
	@(make -C ../../ld/2-full build)

$(OUT)/ar.oe: $(SRC)/ar.oo Makefile $(VM) $(LD) $(LIBC_FILES) $(LIBO_FILES)
	@rm -f $@
	@mkdir -p $(OUT)
	@# Since we don't have an archive tool yet we can't use the libc and libo
	@# archives. We pass all the files manually to the linker.
	$(LD) -g $(LIBC_FILES) $(LIBO_FILES) $(SRC)/ar.oo -o $@

test: build FORCE $(VM)
	../run.sh $(VM) $(AR)
	@# TODO there aren't any tests yet so instead we test just by
	@# archiving the stage0 libc
	$(VM) $(OUT)/ar.oe rc $(OUT)/libc.oa $(LIBC_FILES)
