# Linux-HW
## Task2: Key Press Counter
* `linux-kernel-version:6.7.4`
* `busybox-1.36.1`
### How to run:
* `wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.7.4.tar.xz`
* `tar -xf linux-6.7.4.tar.xz`
* ... Все, как было на семинаре, vroot для fs, boot под установку ядра
* `make clean`
* `make all`
* `cp key_press_counter.ko vroot/lib/modules/6.7.4/`
* `cd vroot`
* `find . | cpio -ov --format=newc | gzip -9 > ../initramfs`
* `cd ../`
* `qemu-system-x86_64 -kernel ./boot/vmlinuz-6.7.4 -initrd ./initramfs --enable-kvm -nographic -append "console=ttyS0"`
### TO DO:
Написать скрипт для запуска...