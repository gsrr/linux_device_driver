#!/bin/bash
make M=drivers/mymod
insmod drivers/mymod/mymod.ko
mkfs.ext3 /dev/simp_blkdev
lsblk
df -h
rmmod drivers/mymod/mymod.ko
