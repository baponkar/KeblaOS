qemu-system-x86_64 -cdrom build/image.iso  -m 4096 -serial file:serial_output.log \
    -d guest_errors,int,cpu_reset -vga std -monitor stdio -no-shutdown -no-reboot
