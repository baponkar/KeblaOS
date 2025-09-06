

#pragma once

#include <stdint.h>

char *get_serial_log();

void serial_init();
void serial_putchar(char c);
void serial_print(const char* str);
void serial_clearchar();
void serial_print_hex(uint64_t num);
void serial_print_dec(int64_t num);
void serial_print_bin(uint64_t num);




