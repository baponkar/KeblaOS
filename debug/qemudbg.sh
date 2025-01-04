#!/usr/bin/bash

qemu-system-x86_64 -cdrom build/image.iso  -m 4096 -serial file:serial_output.log \
    -d guest_errors,int,cpu_reset -vga std -no-shutdown -no-reboot -S -s &
QEMU_PID=$!

#        -ex 'layout src' \
#        -ex 'layout regs' \

gdb build/kernel.bin \
        -ex 'target remote localhost:1234' \
        -ex 'break kmain' \
        -ex 'continue'

ps --pid $QEMU_PID > /dev/null
if [ "$?" -eq 0 ]; then
    kill -9 $QEMU_PID
fi

stty sane

