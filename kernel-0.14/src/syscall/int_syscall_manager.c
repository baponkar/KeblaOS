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
    registers_t *temp = regs;
    switch(regs->int_no){
        case 172:
            printf("System Call 172\n");
            break;
        case 173:
            printf("System Call 173\n");
            break;
        case 174:
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


