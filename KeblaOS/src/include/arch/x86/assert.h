#include "../driver/vga.h"

/**
 * assert(expression)
 * Checks the given expression. If it evaluates to 0 (false),
 * prints an error message with file and line information and halts.
 */
#define assert(expr)                                                \
    do {                                                            \
        if (!(expr)) {                                              \
            print("Assertion failed: ");                            \
            print(#expr);                                           \
            print(", file: ");                                      \
            print(__FILE__);                                        \
            print(", line: ");                                      \
            print_dec(__LINE__);                                    \
            print("\n");                                            \
            while (1) {} /* Halt execution */                       \
        }                                                           \
    } while (0)


