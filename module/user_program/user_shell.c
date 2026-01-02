

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../libc/include/syscall.h"
#include "../libc/include/stdio.h"
#include "../libc/include/string.h"

#include "user_shell.h"

#define MAX_INPUT 256
#define MAX_ARGS 10



static void print_prompt() {
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

static inline int is_fs_error(uint64_t res) {
    return (res == (uint64_t)-1) || (res == (uint64_t)0xFFFFFFFF);
}

static void handle_command(int argc, char *argv[]) {
    if (argc == 0) return;

    if (strcmp(argv[0], "exit") == 0) {         // ✅
        syscall_exit();

    } else if (strcmp(argv[0], "echo") == 0) {  // ✅
        for (int i = 1; i < argc; i++) {
            printf("%s ", argv[i]);
        }
        printf("\n");

    } else if (strcmp(argv[0], "ls") == 0) {    // ✅
        if(argc < 2){
            char cwd[128];
            int r = syscall_getcwd(1, cwd, sizeof(cwd));
            if(r==0){
                int res = syscall_list_dir(2, cwd);
            }
        }else{
            int res = syscall_list_dir(2, argv[1]);
        }

    } else if(strcmp(argv[0], "cd") == 0){
        if(argc < 2){

        }else{
            char *path = argv[1];

            if(path == ".."){

            }
            int res = syscall_chdir(path);
            if(res){
                printf("Failed Change directory into %s!\n", path);
            }
        }
    } else if(strcmp(argv[0], "cwd") == 0){
        char *buf = (char *)syscall_uheap_alloc(256, ALLOCATE_DATA);
        syscall_getcwd(1, buf, 256);
        buf[255] = '\0';
        printf("%s\n", buf);
    } else if (strcmp(argv[0], "cat") == 0) {
        if (argc < 2) {
            printf("Usage: cat <file>\n");
            return;
        }
        void *file = (void*)syscall_open(1, argv[1], FA_READ);
        
        if ((uint64_t)file == (uint64_t)-1 || (uint64_t)file == 0xFFFFFFFF) {
            printf("Cannot open file\n");
            return;
        }

        char buffer[128];
        uint64_t read_bytes;
        while ((read_bytes = syscall_read(1, file, 0, (void *)buffer, sizeof(buffer) - 1)) > 0) {
            buffer[read_bytes] = '\0';
            printf("%s", buffer);
        }
        syscall_close(1, file);

    } else if (strcmp(argv[0], "mkdir") == 0) { // ❌
        if (argc < 2) {
            printf("Usage: mkdir <dir>\n");
            return;
        }
        syscall_mkdir((void*)argv[1]);

    } else if (strcmp(argv[0], "rm") == 0){     // ❌
        if(argc < 2){
            printf("Usage: rm <file>\n");
            return;
        }
        uint64_t result = syscall_unlink(1, argv[1]);

        if(!is_fs_error(result)){
            printf("Remove %s failed!\n", argv[1]);
            return;
        }
        printf("Successfully Removed %s\n", argv[1]);
        
    }else if (strcmp(argv[0], "run") == 0) {    // ❌
        if (argc < 2) {
            printf("Usage: run <program>\n");
            return;
        }
        void *proc = syscall_create_process(argv[1]);
        if (!proc) printf("Failed to run process\n");

    } else if (strcmp(argv[0], "touchreadwrite") == 0) {    // ❌
        if (argc < 2) {
            printf("Usage: touchreadwrite <filename>\n");
            return;
        }

        char *filename = argv[1];
        char buffer[128];
        uint64_t read_bytes = 0;

        void *file_node = (void*) syscall_open(1, filename, FA_OPEN_EXISTING | FA_READ | FA_WRITE);

        if (!is_fs_error((uint64_t)file_node)) {
            printf("%s file already present.\n", filename);

            if (!is_fs_error(syscall_lseek(1, file_node, 0)))
                printf("Lseek is success\n");

            read_bytes = syscall_read(1, file_node, 0, buffer, sizeof(buffer) - 1);
            if ((int64_t)read_bytes <= 0) {
                printf("File is empty or read failed\n");
            } else {
                buffer[read_bytes] = '\0';
                printf("Read file content: %s\n", buffer);
            }

        } else {
            printf("Creating new file: %s\n", filename);
            file_node = (void*) syscall_open(1, filename, FA_CREATE_ALWAYS | FA_READ | FA_WRITE);
            if (is_fs_error((uint64_t)file_node)) {
                printf("Failed to create file\n");
                return;
            }

            char *str = "Hello from user_shell.c";
            if (is_fs_error(syscall_write(1, file_node, str, strlen(str)))) {
                printf("Writing failed\n");
            } else {
                syscall_lseek(1, file_node, 0);
                read_bytes = syscall_read(1, file_node, 0, buffer, sizeof(buffer) - 1);
                if ((int64_t)read_bytes > 0) {
                    buffer[read_bytes] = '\0';
                    printf("File created and read back: %s\n", buffer);
                }
            }
        }

        uint64_t close_res = syscall_close(1, file_node);
        if (close_res == 0){
            printf("File closed successfully\n");
        }else{
            printf("Failed to close file\n");
        }

        uint64_t unlink_res = syscall_unlink(1, filename);
        if (unlink_res == 0){
            printf("File deleted successfully\n");
        }else {
            printf("Failed to delete file\n");
        }
    } else if (strcmp(argv[0], "help") == 0) {  // ✅
        printf("Available commands:\n");
        printf("  exit - Exit the shell\n");
        printf("  echo <text> - Print text to the console\n");
        printf("  ls [dir] - List files in directory (default is current)\n");
        printf("  cat <file> - Display file content\n");
        printf("  mkdir <dir> - Create a new directory\n");
        printf("  rm <file> - Remove a file\n");
        printf("  run <program> - Run a user program\n");
        printf("  touchreadwrite <filename> - Create or read/write a file\n");
        printf("  cd <path> - Change Directory.\n");
        printf("  cwd - Current Working Directory.\n");

    } else {    // ✅
        printf("\nUnknown command: %s\n", argv[0]);
    }
}



void start_user_shell() {
    char input[MAX_INPUT];
    char *argv[MAX_ARGS];
    printf("Welcome to KeblaOS Shell!\n");

    while (true) {
        print_prompt();
        memset(input, 0, sizeof(input));
        read_input(input, sizeof(input));
        int argc = tokenize(input, argv);
        printf("\n");   // Creating a new line
        handle_command(argc, argv);
    }

    while (true) {}     // Halt
}





