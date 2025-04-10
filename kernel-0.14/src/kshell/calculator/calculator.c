/*

*/

#include "../../lib/stdio.h"
#include "../../lib/string.h"
#include "../../lib/stdlib.h" // atof
#include "../../process/process.h"
#include "../../process/thread.h"
#include "../kshell.h"

#include "calculator.h"

extern void restore_cpu_state(registers_t* registers);

process_t* calculator_process;


void calculator_main(void* arg) {
    (void)arg; // Unused argument

    char operator;
    double num1, num2, result;
    char buffer[32];

    while (1) {
        printf("Calculator>> Enter an operator (+, -, *, /) or 'q' to quit: ");
        
        // Read operator
        read_command(buffer, sizeof(buffer));
        operator = buffer[0];

        if (operator == 'q' || operator == 'Q') {
            printf("\nExiting calculator...\n");
            break;
        }

        printf("\nEnter first number: ");
        read_command(buffer, sizeof(buffer));
        num1 = atof(buffer); // You must have an atof() implementation

        printf("\nEnter second number: ");
        read_command(buffer, sizeof(buffer));
        num2 = atof(buffer);

        switch (operator) {
            case '+':
                result = num1 + num2;
                printf("\nResult: %f\n", result);
                break;
            case '-':
                result = num1 - num2;
                printf("\nResult: %f\n", result);
                break;
            case '*':
                result = num1 * num2;
                printf("\nResult: %f\n", result);
                break;
            case '/':
                if (num2 != 0) {
                    result = num1 / num2;
                    printf("\nResult: %f\n", result);
                } else {
                    printf("\nError! Division by zero.\n");
                }
                break;
            default:
                printf("\nInvalid operator.\n");
                break;
        }
    }

    // After calculator exits, go back to shell
    run_kshell();
}


void start_calculator() {
    // Create a new process for the calculator.
    calculator_process = create_process("Calculator Process");
    if (!calculator_process) {
        printf("Failed to create calculator process!\n");
        return;
    }

    // Create a new thread within the calculator process, with calculator_main as its entry.
    thread_t* calculator_thread = create_thread(calculator_process, "Calculator Thread", &calculator_main, NULL);
    if (!calculator_thread) {
        printf("Failed to create calculator thread!\n");
        return;
    }

    restore_cpu_state((registers_t *)(uintptr_t)&calculator_thread->registers);

    printf("Calculator started successfully.\n");
}

void destroy_calculator() {
    // Cleanup code for calculator process and thread if needed.
    // This is a placeholder as the actual cleanup would depend on your process/thread management implementation.
    delete_process(calculator_process);
    printf("Calculator process and thread destroyed.\n");
}