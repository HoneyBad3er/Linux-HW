#!/bin/bash

set -e

make clean && make all
cp phonebook.ko vroot/lib/modules/6.7.4/
cd vroot/ && find . | cpio -ov --format=newc | gzip -9 > ../initramfs
cd ..
qemu-system-x86_64 -kernel ./boot/vmlinuz-6.7.4 -initrd ./initramfs --enable-kvm -nographic -append "console=ttyS0"