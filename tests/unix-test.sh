#!/bin/bash
#
#
# run with 'sudo -E'

set -e

BIN=bin/carnxd
BPF=bin/carnx.bpf
LIB=lib/

# run bin
LD_LIBRARY_PATH="${LIB}" ${BIN} -i lo -l "${BPF}" &
PID=$!

# small delay
sleep 0.5

# run tests
tests/live-test.sh

# stop the process
kill ${PID}
