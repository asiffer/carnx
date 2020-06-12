#!/bin/bash
#
# Here we assume that carnxd is installed
# with 'make install'

# start the service
systemctl start carnx.service

tests/live-test.sh

# stop the service
systemctl stop carnx.service carnx.socket
