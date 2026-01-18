

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
char welcome_msg[38] = "Hello from User Program user_main.c!\n";
int res;
int boot_disk_no = 0;  // Boot Disk is 0
int user_disk_no = 0;  // User Disk is 1

int boot_pd = 0;

int boot_ld = 0;
int user_ld = 1;


__attribute__((section(".text")))
void _start(){

    printf("%s\n", welcome_msg);


    // instll();

    // regex_test();

    // user_syscall_test();

    start_user_shell();

    while (true){}     // Halt
}




