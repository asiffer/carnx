# Makefile
# 

# shell to run commands 
SHELL      := /bin/bash

# C compilation
CC         := $(shell command -v clang)
CFLAGS     := -g -Wall -O2
LIBBPF_DIR := libbpf/src/
LDFLAGS    := -L$(LIBBPF_DIR) -lbpf

# kernel stuff
KERNEL_SRC := kernel.c
KERNEL_OBJ := carnx.bpf

# user stuff
USER_SRC   := common.c loader.c user.c
USER_OBJ   := libcarnx.so


default: clean kernel user

kernel:
	$(CC) $(CFLAGS) -c -target bpf -o $(KERNEL_OBJ) $(KERNEL_SRC)

user:
	$(CC) $(CFLAGS) -shared -fPIC -o $(USER_OBJ) $(USER_SRC) $(LDFLAGS)

export:
	cp -u $(USER_OBJ) $(KERNEL_OBJ) ..

clean:
	rm -f $(KERNEL_OBJ) $(USER_OBJ)