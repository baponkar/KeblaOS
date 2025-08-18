
#include "libc.h"


void libc_test(){

    printf("My Custom C Library Functions Test Start...\n");

    char *str1 = "Hello, ";
    char *str2 = "World!";

    // strcat(str1, str2);
    // printf("%s\n", str1);         // Hello, World!

    // strncat(str1, str2, 3);
    // printf("%s\n", str1);     // Hello, Wor

    char *path = "/my_dir/test.txt";

    printf("strlen(%s): %d\n", path, strlen(path));         // 16
    printf("strlen(%s): %d\n", path, strnlen(path, 256));   // 16

    char c = 'l';
    printf("first occurance of %c in %s is %dth position\n", c, str1, strchr(str1, c) - str1); // first occurance of l in Hello,  is 2th position

    char str[] = "hello,world,this,is,C";
    char *token = strtok(str, ",");

    printf("strlen(token):%d\n", strlen(token)); // Output: strlen(token):5

    while (token != NULL) {
        printf("%s\n", token);
        token = strtok(NULL, ",");      // hello world this is C
    }


    char *last_slash = strrchr(path, '/');

    if (last_slash) {
        printf("Parent: %s\n", last_slash - 1); // Output: test.txt
        printf("Filename: %s\n", last_slash + 1);   // Output: /test.txt
    } else {
        printf("No slash found.\n");
    }

    printf("My Custom C Library Functions Test End...\n");
}




