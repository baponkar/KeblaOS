

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
int user_disk_no = 1;  // User Disk is 1


__attribute__((section(".text")))
void _start(){

    printf("%s\n", welcome_msg);

    uint64_t res = syscall_vfs_init(user_disk_no);
    if(res != 0){
        printf("VFS Initialization on Disk %d failed!\n", user_disk_no);    
        return;
    }
    printf("VFS Initialization on Disk %d successful!\n", user_disk_no);

    res = syscall_mount(user_disk_no);
    if(res != 0){
        printf("Mounting Disk %d failed!\n", user_disk_no);    
        return;
    }
    printf("Mounting Disk %d successful!\n", user_disk_no);


    res = syscall_chdrive(user_disk_no, "1:/");
    if(res != 0){
        printf("Changing Drive to 1:/ failed!\n");
        return;
    }
    printf("Changing Drive to 1:/ successful!\n");

    // instll();

    // regex_test();

    // user_syscall_test();

    start_user_shell();

    while (true){}     // Halt
}




