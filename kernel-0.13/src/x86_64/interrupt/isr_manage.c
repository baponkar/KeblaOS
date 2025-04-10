/*
Interrupt Service Routine(ISR) Manage
*/

#include "../../util/util.h"
#include "../../lib/stdio.h"

#include "isr_manage.h"

#define TOTAL_ISR 32

char* exception_messages[] = {
    "Division By Zero", // 0
    "Debug", // 1
    "Non Maskable Interrupt", // 2
    "Breakpoint", // 3
    "Into Detected Overflow", // 4
    "Out of Bounds", // 5
    "Invalid Opcode", // 6
    "No Coprocessor", // 7
    "Double fault (pushes an error code)", // 8
    "Coprocessor Segment Overrun", // 9
    "Bad TSS (pushes an error code)", // 10
    "Segment not present (pushes an error code)", // 11
    "Stack fault (pushes an error code)", // 12
    "General protection fault (pushes an error code)", // 13
    "Page fault (pushes an error code)", // 14
    "Unknown Interrupt", // 15
    "Coprocessor Fault", // 16
    "Alignment Fault", // 17
    "Machine Check",  // 18
    "SIMD (SSE/AVX) error", // 19
    "Reserved", // 20
    "Reserved", // 21
    "Reserved", // 22
    "Reserved", // 23
    "Reserved", // 24
    "Reserved", // 25
    "Reserved", // 26
    "Reserved", // 27
    "Reserved", // 28
    "Reserved", // 29
    "Reserved", // 30
    "Reserved"  // 31
};



void cpu_exception_handler(registers_t *regs){
    printf("Received Interrupt : %d\n%s\nError Code : %d\nSystem Halted!\n", 
        regs->int_no, exception_messages[regs->int_no], regs->err_code);
    // debug_error_code(regs->err_code);
    halt_kernel();
}

void isr_handler(registers_t *regs)
{
    cpu_exception_handler(regs);
    // if (regs->int_no == 14) {
    //     page_fault_handler(regs);
    //     return;
    // }else if(regs->int_no == 13){
    //     // print("General Protection Fault\n");
    //     // debug_error_code(regs->err_code);
    //     gpf_handler(regs);
    //     return;
    // }else if(regs->int_no < 32){
    //     cpu_exception_handler(regs);
    //     return;
    // }else{
    //     printf("Received Interrupt : %d\n", regs->int_no);
    //     return;
    // }
}

