#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Function to convert Hexadecimal to Decimal
unsigned long hexToDecimal(const char* hex) {
    return strtoul(hex, NULL, 16);
}

// Function to convert Decimal to Hexadecimal
void decimalToHex(unsigned long decimal, char* hexBuffer) {
    sprintf(hexBuffer, "0x%lX", decimal); // Include 0x prefix
}

// Function to remove "0x" or "0X" prefix if present
void removeHexPrefix(char* hex) {
    if (hex[0] == '0' && (hex[1] == 'x' || hex[1] == 'X')) {
        memmove(hex, hex + 2, strlen(hex) - 1);
    }
}

int main() {
    int choice;
    char hexInput[20];
    unsigned long decimalInput;
    char hexOutput[20];

    printf("Choose conversion:\n");
    printf("1. Hexadecimal to Decimal\n");
    printf("2. Decimal to Hexadecimal\n");
    printf("Enter choice (1/2): ");
    scanf("%d", &choice);

    if (choice == 1) {
        // Hex to Decimal conversion
        printf("Enter a hexadecimal number (with or without 0x prefix): ");
        scanf("%s", hexInput);
        removeHexPrefix(hexInput);  // Remove 0x if present
        printf("Decimal equivalent: %lu\n", hexToDecimal(hexInput));
    } else if (choice == 2) {
        // Decimal to Hex conversion
        printf("Enter a decimal number: ");
        scanf("%lu", &decimalInput);
        decimalToHex(decimalInput, hexOutput);
        printf("Hexadecimal equivalent: %s\n", hexOutput);
    } else {
        printf("Invalid choice! Please enter 1 or 2.\n");
    }

    return 0;
}


