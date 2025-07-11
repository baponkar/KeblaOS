

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "libc/include/syscall.h"
#include "libc/include/stdio.h"
#include "libc/include/string.h"



#define MAX_INPUT 256
#define MAX_ARGS 10

void print_prompt() {
    printf("ksh> ");
}

void read_input(char *buf, size_t size) {
    syscall_keyboard_read((uint8_t*)buf, size);
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';
}

int tokenize(char *input, char *argv[]) {
    int argc = 0;
    char *token = strtok(input, " ");
    while (token && argc < MAX_ARGS) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }
    argv[argc] = NULL;
    return argc;
}

void handle_command(int argc, char *argv[]) {
    if (argc == 0) return;

    if (strcmp(argv[0], "exit") == 0) {
        syscall_exit();
    } else if (strcmp(argv[0], "echo") == 0) {
        for (int i = 1; i < argc; i++) {
            printf(argv[i]);
            printf(" ");
        }
        printf("\n");
    } else if (strcmp(argv[0], "ls") == 0) {
        void *dir = (void*)syscall_opendir(argc > 1 ? argv[1] : "/");
        if (!dir) {
            printf("Failed to open directory\n");
            return;
        }

        while (true) {
            uint64_t res = syscall_readdir(dir);
            if (res == 0) break;
            printf((char*)res); // Assuming syscall returns pointer to filename
            printf("\n");
        }

        syscall_closedir(dir);
    } else if (strcmp(argv[0], "cat") == 0) {
        if (argc < 2) {
            printf("Usage: cat <file>\n");
            return;
        }

        void *file = (void*)syscall_open(argv[1], FA_READ);
        if (!file) {
            printf("Cannot open file\n");
            return;
        }

        char buffer[128];
        uint64_t read_bytes;
        while ((read_bytes = syscall_read(file, buffer, sizeof(buffer) - 1)) > 0) {
            buffer[read_bytes] = '\0';
            printf(buffer);
        }

        syscall_close(file);
    } else if (strcmp(argv[0], "mkdir") == 0) {
        if (argc < 2) {
            printf("Usage: mkdir <dir>\n");
            return;
        }
        syscall_mkdir((void*)argv[1]);
    } else if (strcmp(argv[0], "run") == 0) {
        if (argc < 2) {
            printf("Usage: run <program>\n");
            return;
        }
        void *proc = syscall_create_process(argv[1]);
        if (!proc) printf("Failed to run process\n");
    } else {
        printf("Unknown command: ");
        printf(argv[0]);
        printf("\n");
    }
}

__attribute__((section(".text")))
void _start() {
    char input[MAX_INPUT];
    char *argv[MAX_ARGS];
    printf("Welcome to KeblaOS Shell!\n");

    while (true) {
        print_prompt();
        memset(input, 0, sizeof(input));
        read_input(input, sizeof(input));
        int argc = tokenize(input, argv);
        handle_command(argc, argv);
    }

    while (true) {} // Halt
}





