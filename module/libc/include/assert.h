#include "stdio.h"

/**
 * assert(expression)
 * Checks the given expression. If it evaluates to 0 (false),
 * prints an error message with file and line information and halts.
 */
#define assert(expr)                                                 \
    do {                                                             \
        if (!(expr)) {                                               \
            printf("Assertion failed: ");                            \
            printf(#expr);                                           \
            printf(", file: ");                                      \
            printf(__FILE__);                                        \
            printf(", line: ");                                      \
            printf("%d", __LINE__);                                  \
            printf("\n");                                            \
            while (1) {} /* Halt execution */                        \
        }                                                            \
    } while (0)


