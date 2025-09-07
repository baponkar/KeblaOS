

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../libc/include/syscall.h"
#include "../libc/include/stdio.h"
#include "../libc/include/string.h"

#include "installer.h"
#include "user_shell.h"




__attribute__((section(".text")))
void _start(){

    printf("\nWelcome! This is User Program \"user_main.c\"\n");

    instll();

    start_user_shell();

    while (true) {}     // Halt
}



