#!/bin/sh

BASE_URL="https://github.com/fullstorydev/grpcurl/releases/download/v1.6.0"
GRPCURL="grpcurl_1.6.0_linux_x86_64.tar.gz"

apt update
apt install -y tar wget libelf1
# get grpcurl
wget "${BASE_URL}/${GRPCURL}"
tar -xvf "${GRPCURL}"
cp grpcurl /usr/bin
