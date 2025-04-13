/*
Interrupt Based System Call

References: 
    https://github.com/dreamportdev/Osdev-Notes/blob/master/06_Userspace/04_System_Calls.md
*/

#include "../lib/stdio.h"
#include "../x86_64/interrupt/irq_manage.h"
#include "../util/util.h"

#include "int_syscall_manager.h"


registers_t *int_systemcall_handler(registers_t *regs){
    switch(regs->int_no){
        case INT_SYSCALL_PRINT:
            printf("System Call 172\n");
            break;
        case INT_SYSCALL_READ:
            printf("System Call 173\n");
            break;
        case INT_SYSCALL_EXIT:
            printf("System Call 174\n");
            break;
        default:
            printf("Unknown Syscall!\n");
            break;
    }

    return regs;
}


void int_syscall_init(){
    irq_install(140, (void *)&int_systemcall_handler);
    irq_install(141, (void *)&int_systemcall_handler);
    irq_install(142, (void *)&int_systemcall_handler);

    printf("[Info] Interrupt Based System Call initialized!\n");
}

char *syscall_test(int syscall_no){
    switch(syscall_no){
        case(INT_SYSCALL_PRINT):
            asm volatile("int $173");
            return NULL;
            break;
        case(INT_SYSCALL_READ):
            asm volatile("int $173");
            return NULL;
            break;
        case(INT_SYSCALL_EXIT):
            asm volatile("int $173");
            return NULL;
            break;
        default:
            return NULL;
            break;
    }
    return NULL;
}
