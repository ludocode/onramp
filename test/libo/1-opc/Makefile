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

CFLAGS := -g
CPPFLAGS := -Wall -Wextra -Wpedantic -Wno-unused-parameter

-include ../../local.mk

ROOT=../..
BUILD=$(ROOT)/build/test

SRC=$(ROOT)/core/libo/src
OUT=$(BUILD)/libo

SRCS=\
		$(SRC)/libo-error.c \
		$(SRC)/libo-string.c \
		$(SRC)/libo-table.c \
		$(SRC)/libo-util.c \
		main.c

all: build test FORCE
FORCE:
build: $(OUT)/libo_test FORCE

clean: FORCE
	rm -rf $(OUT)

# TODO for now we just combine all test files into one executable. Later we
# will have each test compiled separately so we can have failing tests.
$(OUT)/libo_test: $(SRCS) Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRCS) -o $@

test: build FORCE
	$(OUT)/libo_test
