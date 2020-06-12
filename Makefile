# Makefile
# 

# shell to run commands 
SHELL := /bin/bash

# Go compilation
GO      := $(shell command -v go)
GOFLAGS := 

# iface (unload only)
IFACE := 

# Server (Go)
SRC     := main.go server.go
BIN     := carnxd
BPF     := carnx.bpf
LIB     := libbpf.so.0 libcarnx.so
SERVICE := carnx.socket carnx.service

# build dir
BUILD_BIN_DIR      := bin
BUILD_LIB_DIR      := lib

# install dir
INSTALL_BIN_DIR     := /usr/bin
INSTALL_LIB_DIR     := /usr/lib
INSTALL_BPF_DIR     := /var/lib/carnx
INSTALL_SERVICE_DIR := /lib/systemd/system

default: clean prepare libcarnx grpc build

prepare:
	mkdir -p $(BUILD_LIB_DIR) 
	mkdir -p $(BUILD_BIN_DIR)

build:
	$(GO) build -o $(BUILD_BIN_DIR)/$(BIN) $(SRC)

go-deps:
	$(GO) get ./...

libcarnx:
	cd c/; make; cd ..
	cp -u c/libcarnx.so $(BUILD_LIB_DIR)
	cp -u c/carnx.bpf $(BUILD_BIN_DIR)

libbpf:
	cd c/libbpf/src; make clean; make
	cp -u c/libbpf/src/libbpf.so.0 $(BUILD_LIB_DIR)

unload:
	ip link set dev $(IFACE) xdpgeneric off

grpc:
	protoc --doc_out=api --doc_opt=markdown,README.md --go_out=plugins=grpc:. --go_opt=paths=source_relative api/carnx.proto

install:
	install $(BUILD_BIN_DIR)/$(BIN) $(INSTALL_BIN_DIR)
	install $(addprefix $(BUILD_LIB_DIR)/,$(LIB)) $(INSTALL_LIB_DIR)
	mkdir -p $(INSTALL_BPF_DIR)
	install $(BUILD_BIN_DIR)/$(BPF) $(INSTALL_BPF_DIR)
	install $(addprefix systemd/,$(SERVICE)) $(INSTALL_SERVICE_DIR)
	systemctl daemon-reload

uninstall:
	rm -f $(INSTALL_BIN_DIR)/$(BIN)
	rm -f $(addprefix $(INSTALL_SERVICE_DIR)/,$(SERVICE))
	rm -rf $(INSTALL_BPF_DIR)
	rm -f $(addprefix $(INSTALL_LIB_DIR)/,$(LIB))
	systemctl daemon-reload

concourse-build:
	fly -t tutorial execute --config .build.yml -i code="." -o binary="."

clean:
	rm -rf $(BUILD_BIN_DIR)