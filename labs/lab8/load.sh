#!/bin/sh
sudo insmod vmm_dev.ko
sudo mknod /dev/mem_dev c 111 0
sudo chmod a+w /dev/mem_dev
dmesg