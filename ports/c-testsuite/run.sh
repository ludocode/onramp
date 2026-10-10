#!/bin/bash

cd "$(dirname "$0")"/../../..

if ! [ -e c-testsuite ]; then
    echo "ERROR: Couldn't find c-testsuite/ folder."
    echo "It must be checked out alongside onramp/ to use this script."
    exit 1
fi
cd c-testsuite

CC=onrampcc CFLAGS=-O ./single-exec posix | tee c-testsuite.log | grep -v '^#'
