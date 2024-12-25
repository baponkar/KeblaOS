#include <stdio.h>
#include <stdlib.h>

void convert_hex_to_units(unsigned long long hex_value) {
    // Conversion factors
    const unsigned long long BYTES_IN_KB = 1024;
    const unsigned long long BYTES_IN_MB = 1024 * 1024;
    const unsigned long long BYTES_IN_GB = 1024 * 1024 * 1024;
    const unsigned long long BYTES_IN_TB = 1024LL * 1024 * 1024 * 1024;

    // Perform conversions
    double kb = (double)hex_value / BYTES_IN_KB;
    double mb = (double)hex_value / BYTES_IN_MB;
    double gb = (double)hex_value / BYTES_IN_GB;
    double tb = (double)hex_value / BYTES_IN_TB;

    // Display results
    printf("Hex Value: 0x%llX\n", hex_value);
    printf("In KB: %.4f\n", kb);
    printf("In MB: %.4f\n", mb);
    printf("In GB: %.4f\n", gb);
    printf("In TB: %.6f\n", tb);
}

int main() {
    char hex_input[20];
    unsigned long long hex_value;

    // Prompt user for input
    printf("Enter a hexadecimal value (e.g., 0xBFFE0000): ");
    scanf("%s", hex_input);

    // Convert hex string to unsigned long long
    hex_value = strtoull(hex_input, NULL, 16);

    // Perform conversion
    convert_hex_to_units(hex_value);

    return 0;
}
