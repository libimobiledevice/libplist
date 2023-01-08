#!/bin/sh

CURDIR=`pwd`
FUZZDIR=`dirname $0`

cd ${FUZZDIR}

if ! test -x xplist_fuzzer || ! test -x bplist_fuzzer || ! test -x jplist_fuzzer; then
	echo "ERROR: you need to build the fuzzers first."
	cd ${CURDIR}
	exit 1
fi

mkdir -p xplist-input
cp ../test/data/*.plist xplist-input/
./xplist_fuzzer -merge=1 xplist-input xplist-crashes xplist-leaks -dict=xplist.dict

mkdir -p bplist-input
cp ../test/data/*.bplist bplist-input/
./bplist_fuzzer -merge=1 bplist-input bplist-crashes bplist-leaks -dict=bplist.dict

mkdir -p jplist-input
mkdir -p jplist-crashes
mkdir -p jplist-leaks
cp ../test/data/j1.plist jplist-input/
cp ../test/data/j2.plist jplist-input/
./jplist_fuzzer -merge=1 jplist-input jplist-crashes jplist-leaks -dict=jplist.dict

mkdir -p oplist-input
mkdir -p oplist-crashes
mkdir -p oplist-leaks
cp ../test/data/*.ostep oplist-input/
cp ../test/data/test.strings oplist-input/
./oplist_fuzzer -merge=1 oplist-input oplist-crashes oplist-leaks -dict=oplist.dict

cd ${CURDIR}
exit 0
