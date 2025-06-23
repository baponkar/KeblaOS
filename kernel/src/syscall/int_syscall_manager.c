/*
Interrupt Based System Call

References: 
    https://github.com/dreamportdev/Osdev-Notes/blob/master/06_Userspace/04_System_Calls.md
*/

#include "../lib/stdio.h"
#include "../arch/interrupt/irq_manage.h"
#include "../util/util.h"
#include "../kshell/ring_buffer.h"

#include "int_syscall_manager.h"

extern ring_buffer_t* keyboard_buffer;

registers_t *int_systemcall_handler(registers_t *regs) {
    switch (regs->int_no) {                             // syscall number
        case INT_SYSCALL_READ: {
            uint8_t *user_buf = (uint8_t *)regs->rbx;  // user buffer pointer
            size_t size = regs->rcx;                   // max bytes to read
            size_t read_count = 0;

            while (read_count < size) {
                uint8_t ch;
                if (ring_buffer_pop(keyboard_buffer, &ch) == 0) {
                    user_buf[read_count++] = ch;
                } else {
                    break; // buffer empty
                }
            }

            regs->rax = read_count; // Return number of bytes read
            break;
        }

        case INT_SYSCALL_PRINT: {
            const char *str = (const char *)regs->rbx;
            printf(str);  
            regs->rax = 0;  // success
            break;
        }

        case INT_SYSCALL_EXIT: {
            regs->rax = 0; // success
            break;
        }

        default: {
            printf("Unknown System Call!\n");
            regs->rax = -1; // unknown syscall
            break;
        }
    }

    return regs;
}



void int_syscall_init(){
    irq_install(140, (void *)&int_systemcall_handler);
    irq_install(141, (void *)&int_systemcall_handler);
    irq_install(142, (void *)&int_systemcall_handler);

    printf(" [-] Interrupt Based System Call initialized!\n");
}


void syscall_test(int syscall_no){
    switch(syscall_no){
        case(INT_SYSCALL_READ):
            asm volatile("int $172");
            break;

        case(INT_SYSCALL_PRINT):
            asm volatile("int $173");
            break;

        case(INT_SYSCALL_EXIT):
            asm volatile("int $174");
            break;

        default:
            printf("Unknown System Call Number: %d\n", syscall_no);
            break;
    }
}




