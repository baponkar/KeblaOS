
#include "../../lib/stdio.h"
#include "ports.h"

#include "serial.h"

void serial_init() {
    outb(0x3F8 + 1, 0x00); // Disable interrupts
    outb(0x3F8 + 3, 0x80); // Enable DLAB (set baud rate divisor)
    outb(0x3F8 + 0, 0x03); // Set divisor to 3 (38400 baud)
    outb(0x3F8 + 1, 0x00);
    outb(0x3F8 + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(0x3F8 + 2, 0xC7); // Enable FIFO, clear them, 14-byte threshold
    outb(0x3F8 + 4, 0x0B); // IRQs enabled, RTS/DSR set

    printf("Serial read write enabled.\n");
}

void serial_putchar(char c) {
    while ((inb(0x3F8 + 5) & 0x20) == 0); // Wait until buffer is empty
    outb(0x3F8, c);
}

void serial_print(const char* str) {
    while (*str) serial_putchar(*str++);
}

void serial_clearchar() {
    serial_putchar('\b'); // Move cursor back
    serial_putchar(' ');  // Overwrite with space
    serial_putchar('\b'); // Move cursor back again
}



void serial_print_hex(uint64_t num) {
    char hex_chars[] = "0123456789ABCDEF";
    char buffer[17];  // 16 hex digits + null terminator
    buffer[16] = '\0';

    for (int i = 15; i >= 0; i--) {
        buffer[i] = hex_chars[num & 0xF]; // Get last 4 bits
        num >>= 4; // Shift right by 4 bits
    }

    serial_print("0x"); // Prefix for hex numbers
    serial_print(buffer);
}


void serial_print_dec(int64_t num) {
    char buffer[21]; // Max for int64_t (-9223372036854775808) + null
    int i = 20;
    buffer[i--] = '\0';

    if (num == 0) {
        serial_print("0");
        return;
    }

    // Handle negative numbers
    int negative = 0;
    if (num < 0) {
        negative = 1;
        num = -num;
    }

    // Convert to string
    while (num > 0) {
        buffer[i--] = '0' + (num % 10);
        num /= 10;
    }

    if (negative) {
        buffer[i--] = '-';
    }

    serial_print(&buffer[i + 1]);
}


void serial_print_bin(uint64_t num) {
    char buffer[65]; // 64-bit binary + null
    buffer[64] = '\0';

    for (int i = 63; i >= 0; i--) {
        buffer[i] = (num & 1) ? '1' : '0';
        num >>= 1;
    }

    serial_print("0b"); // Binary prefix
    serial_print(buffer);
}

