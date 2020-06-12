# Carnx

XDP-based network statistics digger.

## What is XDP ?

**XDP** (for eXpress Data Path) is a Linux Kernel feature (`>=4.8`) providing an early hook in the incoming packets queue (RX).

The hook is placed in the network interface controller (NIC) driver just after the interrupt processing, and before any memory allocation needed by the network stack itself [Wikipedia][1].

The main asset of XDP is it does not require memory allocation. Due to this design, XDP can process roughly 5x more packets than classical userspace tools (like `iptables`).

![kernel](assets/kernel-diagram.svg)

## Architecture

![architecture](assets/archi.svg)


[1]: https://en.wikipedia.org/wiki/Express_Data_Path
[2]: https://github.com/xdp-project/xdp-paper/blob/master/xdp-presentation.pdf