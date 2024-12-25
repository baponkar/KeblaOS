#include <stdio.h>
#include <stdlib.h>

void print_menu() {
    printf("\nHexadecimal Calculator\n");
    printf("1. Addition\n");
    printf("2. Subtraction\n");
    printf("3. Multiplication\n");
    printf("4. Division\n");
    printf("5. Exit\n");
    printf("Enter your choice: ");
}

unsigned long long get_hex_input(const char *prompt) {
    char hex_input[20];
    printf("%s", prompt);
    scanf("%s", hex_input);
    return strtoull(hex_input, NULL, 16);
}

int main() {
    int choice;
    unsigned long long num1, num2, result;

    while (1) {
        print_menu();
        scanf("%d", &choice);

        if (choice == 5) {
            printf("Exiting the calculator. Goodbye!\n");
            break;
        }

        // Get two hexadecimal numbers from the user
        num1 = get_hex_input("Enter the first hexadecimal number (e.g., 0xA): ");
        num2 = get_hex_input("Enter the second hexadecimal number (e.g., 0xB): ");

        switch (choice) {
            case 1:
                result = num1 + num2;
                printf("Result (Addition): 0x%llX\n", result);
                break;
            case 2:
                result = num1 - num2;
                printf("Result (Subtraction): 0x%llX\n", result);
                break;
            case 3:
                result = num1 * num2;
                printf("Result (Multiplication): 0x%llX\n", result);
                break;
            case 4:
                if (num2 == 0) {
                    printf("Error: Division by zero is not allowed.\n");
                } else {
                    result = num1 / num2;
                    printf("Result (Division): 0x%llX\n", result);
                }
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }

    return 0;
}
