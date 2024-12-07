/* This function will works on regular host system*/

#include <stdio.h>
#include <stdlib.h>

// Function to display a menu and get user choice
char display_menu() {
    char choice;
    printf("\nCalculator\n");
    printf("Choose an operation:\n");
    printf("  + : Addition\n");
    printf("  - : Subtraction\n");
    printf("  * : Multiplication\n");
    printf("  / : Division\n");
    printf("  %% : Modulus\n");
    printf("  h : Hex to Decimal Conversion\n");
    printf("  d : Decimal to Hex Conversion\n");
    printf("  q : Quit\n");
    printf("Enter your choice: ");
    scanf(" %c", &choice);
    return choice;
}

int main() {
    char choice;
    unsigned int num1, num2, result;
    int is_hex;

    do {
        // Ask user if they want to use hexadecimal or decimal mode
        printf("\nSelect mode (1 for Hexadecimal, 0 for Decimal): ");
        scanf("%d", &is_hex);

        // Get user input based on mode
        if (is_hex) {
            printf("\nEnter first hexadecimal number (e.g., 0x1A): ");
            scanf("%x", &num1);
            printf("Enter second hexadecimal number (e.g., 0x1A): ");
            scanf("%x", &num2);
        } else {
            printf("\nEnter first decimal number: ");
            scanf("%u", &num1);
            printf("Enter second decimal number: ");
            scanf("%u", &num2);
        }

        // Display menu and get operation choice
        choice = display_menu();

        switch (choice) {
            case '+':
                result = num1 + num2;
                if (is_hex)
                    printf("Result: 0x%X\n", result);
                else
                    printf("Result: %u\n", result);
                break;
            case '-':
                result = num1 - num2;
                if (is_hex)
                    printf("Result: 0x%X\n", result);
                else
                    printf("Result: %u\n", result);
                break;
            case '*':
                result = num1 * num2;
                if (is_hex)
                    printf("Result: 0x%X\n", result);
                else
                    printf("Result: %u\n", result);
                break;
            case '/':
                if (num2 == 0) {
                    printf("Error: Division by zero is not allowed.\n");
                } else {
                    result = num1 / num2;
                    if (is_hex)
                        printf("Result: 0x%X\n", result);
                    else
                        printf("Result: %u\n", result);
                }
                break;
            case '%':
                if (num2 == 0) {
                    printf("Error: Modulus by zero is not allowed.\n");
                } else {
                    result = num1 % num2;
                    if (is_hex)
                        printf("Result: 0x%X\n", result);
                    else
                        printf("Result: %u\n", result);
                }
                break;
            case 'h': {
                unsigned int hex_value;
                printf("\nEnter a hexadecimal number (e.g., 0x1A): ");
                scanf("%x", &hex_value);
                printf("Decimal value: %u\n", hex_value);
                break;
            }
            case 'd': {
                unsigned int decimal_value;
                printf("\nEnter a decimal number: ");
                scanf("%u", &decimal_value);
                printf("Hexadecimal value: 0x%X\n", decimal_value);
                break;
            }
            case 'q':
                printf("Exiting the calculator. Goodbye!\n");
                break;
            default:
                printf("Invalid choice. Please select a valid operation.\n");
        }

    } while (choice != 'q');

    return 0;
}
