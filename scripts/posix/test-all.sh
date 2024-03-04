#!/bin/sh

# Test everything that runs on this platform.









# TODO none of this works
exit 1








set -e

for 

for component in core/*; do
    if [ -e $component/Makefile ]; then
        echo
        echo "Testing $component"
        make -C $component test
    else
        for stage in $component/*; do
            if [ -e $stage/Makefile ]; then
                echo
                echo "Testing $stage"
                make -C $stage test
            fi
        done
    fi
done

echo "All tests done."








# the rest of the below is junk

exit 0









TODO need to clean up all the Makefiles.
- all makefiles should build into top-level build/ folder
- all makefiles should have "clean" and "test" options. this script should run `make clean && make test`
    - tools that are lax can add --lax in their own makefiles
- all makefiles should use "onrampvm" and "onramphex", not specific vm or hex tools
- have separate script to bootstrap vm
- write a doc on how onramp is tested
- maybe make an equivalent script to this to test on windows



#######################
# setup
#######################

# For the purposes of this test suite we use the C89 implementations of the VM
# and hex tool. This requires at least a C89 compiler.

# make hex and vm, install, source env.sh

# TODO or maybe just require that the hex and onramp setup is done first. make setup.sh script that does it



#######################
# hex
#######################

echo -e "\n\nTesting hex/c89"
( cd core/hex/c89 && make hex && ../test/test.sh ./hex )



#######################
# vm
#######################

echo -e "\n\nTesting vm/c-debugger"
( cd core/vm/c-debugger && make vm && ../test/test.sh ./vm )

echo -e "\n\nTesting vm/python"
( cd core/vm/python && ../test/test.sh ./vm.py )

if [ "$(uname -s)" = "Linux" ] && [ "$(uname -m)" = "x86_64" ]; then
    echo -e "\n\nTesting vm/x86_64-linux"
    ( cd core/vm/x86_64-linux && make vm && ../test/test.sh ./vm )
fi



#######################
# hex/onramp
#######################

# We need to test this after getting our VMs working.

echo -e "\n\nTesting hex/c89"
( cd core/hex/onramp && make hex.oe && ../test/test.sh ../../vm/c-debugger/vm hex.oe )
