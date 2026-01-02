

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

__attribute__((section(".data")))
int user_data_var = 12345;
char user_data_str[] = "Hello from User Data Segment!";


__attribute__((section(".text")))
void _start(){

    printf("\nWelcome! This is User Program \"user_main.c\"\n");

    printf("user_data_var = %d\n", user_data_var);
    printf("user_data_str = %s\n", user_data_str);

    // instll();

    // regex_test();

    // user_syscall_test();

    start_user_shell();

    syscall_cls_color(0x000000); // Black
    for(int i=0; i<5; i++){
        syscall_set_pixel(10 + i, 10 + i, 0xFF0000);
        sleep_seconds(1);
    }

    while (true){}     // Halt
}




