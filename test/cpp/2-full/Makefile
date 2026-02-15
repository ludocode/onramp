# The MIT License (MIT)
#
# Copyright (c) 2023-2025 Fraser Heavy Software
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

.SUFFIXES:
CFLAGS := -g
CPPFLAGS := -Wall -Wextra -Wpedantic -DDEBUG

-include ../../local.mk

ROOT=../../..
BUILD=$(ROOT)/output/test

OUT=$(BUILD)/cpp-2-full
SRC=$(ROOT)/core/cpp/2-full/src
LIBO=$(ROOT)/core/libo/1-opc
CPPFLAGS += \
	-include $(ROOT)/test/portability.h \
	-I$(LIBO)/include

SRCS=\
	$(LIBO)/src/libo-error.c \
	$(LIBO)/src/libo-reader.c \
	$(LIBO)/src/libo-string.c \
	$(LIBO)/src/libo-table.c \
	$(LIBO)/src/libo-unicode.c \
	$(LIBO)/src/libo-util.c \
	$(LIBO)/src/libo-vector.c \
	\
	$(SRC)/common.c \
	$(SRC)/directive.c \
	$(SRC)/emit.c \
	$(SRC)/expression.c \
	$(SRC)/file.c \
	$(SRC)/hideset.c \
	$(SRC)/lexer.c \
	$(SRC)/macro.c \
	$(SRC)/main.c \
	$(SRC)/options.c \
	$(SRC)/preprocess.c \
	$(SRC)/stream.c \
	$(SRC)/strings.c \
	$(SRC)/token.c \

all: build test FORCE
FORCE:
build: $(OUT)/cpp FORCE

clean: FORCE
	rm -rf $(OUT)

$(OUT)/cpp: $(SRCS) Makefile
	@rm -f $@
	@mkdir -p $(OUT)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRCS) -o $@

test-0: build FORCE
	( cd ../0-strip ; ../run.sh . $(OUT)/cpp )

test-1: build FORCE
	( cd ../1-omc ; ../run.sh . $(OUT)/cpp )

test-2: build FORCE
	@# TODO add --strict
	../run.sh --nonstd . $(OUT)/cpp

test: test-0 test-1 test-2 FORCE

generate: build FORCE $(VM)
	../generate.sh . $(OUT)/cpp
