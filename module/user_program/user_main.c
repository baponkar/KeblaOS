

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../libc/include/syscall.h"
#include "../libc/include/stdio.h"
#include "../libc/include/string.h"
#include "../libc/include/time.h"

#include "../ext_lib/tiny-regex-c/re_test.h"
#include "../ext_lib/UGUI/ugui.h"

#include "installer.h"
#include "user_shell.h"
#include "process_thread_test.h"
#include "user_syscall_test.h"




__attribute__((section(".text")))
void _start(){

    printf("\nWelcome! This is User Program \"user_main.c\"\n");

    // instll();

    // regex_test();

    // user_syscall_test();

    // start_user_shell();

    user_syscall_test();

    while (true) {}     // Halt
}




