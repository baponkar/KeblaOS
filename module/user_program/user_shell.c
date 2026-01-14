

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "../libc/include/syscall.h"
#include "../libc/include/stdio.h"
#include "../libc/include/string.h"

#include "user_shell.h"

#define MAX_INPUT 256
#define MAX_ARGS 10

extern int boot_disk_no;
extern int user_disk_no;

static void print_prompt() {
    char cwd[256];

    int res = syscall_getcwd(user_disk_no, cwd, sizeof(cwd));
    if (res == 0) {
        printf("ksh:%s>> ", cwd);
    } else {
        printf("ksh>> ");
    }
}


// This function reads input from the keyboard into the buffer
void read_input(char *buf, size_t size) {
    size_t len =syscall_keyboard_read((uint8_t*)buf, size); // Leave space for null terminator

    if(len < 0) {
        printf("\nError reading input!\n");
        buf[0] = '\0'; // Clear buffer on error
        return;
    }

    buf[len] = '\0'; // Null-terminate the string

    if (len > 0 && buf[len - 1] == '\n') {  // Remove trailing newline
        buf[len - 1] = '\0';
    }
}

// This function tokenizes the input string into arguments and returns the argument count
int tokenize(char *input, char *argv[]) {
    int argc = 0;                       // Store the number of arguments
    char *token = strtok(input, " ");   // Get the first token
    while (token && argc < MAX_ARGS) {
        argv[argc++] = token;           // Store the token in argv
        token = strtok(NULL, " ");      // Get the next token
    }
    argv[argc] = NULL;                  // Null-terminate the argv array

    return argc;                        // total number of arguments
}

static inline int is_fs_error(uint64_t res) {
    return (res == (uint64_t)-1) || (res == (uint64_t)0xFFFFFFFF);
}

static void handle_command(int argc, char *argv[]) {
    if(argc == 0) return;

    if(strcmp(argv[0], "exit") == 0){
        syscall_exit();

    }else if (strcmp(argv[0], "echo") == 0){
        for (int i = 1; i < argc; i++) {
            printf("%s ", argv[i]);
        }
        printf("\n");

    }else if (strcmp(argv[0], "ls") == 0) {
        if(argc < 2){
            char cwd[128];
            int r = syscall_getcwd(user_disk_no, cwd, sizeof(cwd));
            if(r==0){
                int res = syscall_list_dir(user_disk_no, cwd);
            }
        }else{
            int res = syscall_list_dir(user_disk_no, argv[1]);
        }

    }else if(strcmp(argv[0], "cd") == 0){
        if(argc < 2){
            printf("Usage: cd <directory>\n");
        }else{
            char *path = argv[1];
            int res = syscall_chdir(user_disk_no, path);
            if(res != 0){
                printf("Failed Change directory into %s! with error %d\n", path, res);
            }
        }
    } else if(strcmp(argv[0], "cwd") == 0){
        char buf[256];
        int getcwd_res = syscall_getcwd(user_disk_no, buf, sizeof(buf));
        if(getcwd_res != 0){
            printf("Get Current Working Directory failed! with error %d\n", getcwd_res);
        }
        printf("Current Working Directory: %s\n", buf);

    } else if (strcmp(argv[0], "cat") == 0) {
        
        if (argc < 2) {
            printf("Usage: cat <file>\n");
            return;
        }

        char *path = argv[1];
        if(!path){
            printf("Invalid file path!\n");
            return;
        }
        int flags = FA_OPEN_EXISTING | FA_READ;

        void *file = (void*)syscall_open(user_disk_no, path, flags);
        
        if (!file) {
            printf("Cannot open file %s\n", path);
            return;
        }

        char buffer[256];
        uint64_t read_bytes = syscall_read(user_disk_no, file, buffer, sizeof(buffer) - 1);

        if (read_bytes > 0) {
            buffer[read_bytes] = '\0'; // Null-terminate the string
            printf("%s", buffer);
            read_bytes = syscall_read(user_disk_no, file, buffer, sizeof(buffer) - 1);
        }
        syscall_close(user_disk_no, file);
    }else if (strcmp(argv[0], "mkdir") == 0) { 
        if (argc < 2) {
            printf("Usage: mkdir <dir>\n");
            return;
        }
        syscall_mkdir(user_disk_no, (void*)argv[1]);

    } else if (strcmp(argv[0], "rm") == 0){
        if(argc < 2){
            printf("Usage: rm <file>\n");
            return;
        }
        uint64_t result = syscall_unlink(user_disk_no, argv[1]);

        if(result != 0){
            printf("Remove %s failed!\n", argv[1]);
            return;
        }
        printf("Successfully Removed %s\n", argv[1]);
        
    }else if (strcmp(argv[0], "run") == 0) { 
        if (argc < 2) {
            printf("Usage: run <program>\n");
            return;
        }
        void *proc = syscall_create_process(argv[1]);
        if (!proc) printf("Failed to run process\n");

    }else if (strcmp(argv[0], "touch") == 0) { 
        if (argc < 2) {
            printf("Usage: touch <filename>\n");
            return;
        }

        char *filename = argv[1];

        void *file_node = (void*) syscall_open(user_disk_no, filename, FA_CREATE_ALWAYS);
        if(!file_node){
            printf("Failed to create file: %s\n", filename);
            return;
        }

        syscall_close(user_disk_no, file_node);

    } else if (strcmp(argv[0], "write") == 0) {
        if (argc < 2) {
            printf("Usage: write <file> <text>\n");
            printf("Example: write hello.txt Hello World\n");
            return;
        }
        
        // Open file for writing (create or truncate)
        void *file = (void*)syscall_open(user_disk_no, argv[1], FA_CREATE_ALWAYS | FA_WRITE);
        
        if (!file) {
            printf("Cannot open/create file %s for writing\n", argv[1]);
            return;
        }

        // Combine remaining arguments into a single string
        char content[256];
        content[0] = '\0';
        
        for (int i = 2; i < argc; i++) {
            strcat(content, argv[i]);
            if (i < argc - 1) {
                strcat(content, " ");
            }
        }
        
        // Add newline if no content was provided
        if (argc == 2) {
            strcpy(content, "\n");
        } else {
            strcat(content, "\n");
        }
        
        // Write to file
        uint64_t write_bytes = syscall_write(user_disk_no, file, (void *)content, strlen(content));
        
        if (write_bytes == strlen(content)) {
            printf("Written %lu bytes to %s\n", write_bytes, argv[1]);
        } else {
            printf("Failed to write to %s\n", argv[1]);
        }
        
        syscall_close(user_disk_no, file);

    } else if (strcmp(argv[0], "edit") == 0) {
        if (argc < 2) {
            printf("Usage: edit <file>\n");
            printf("This will append to the file. Type 'END' on a new line to finish.\n");
            return;
        }
        
        // Open file for appending
        void *file = (void*)syscall_open(user_disk_no, argv[1], FA_OPEN_ALWAYS | FA_WRITE);
        
        if (!file) {
            printf("Cannot open file %s for editing\n", argv[1]);
            return;
        }

        // Move to end of file
        syscall_lseek(user_disk_no, file, 0xFFFFFFFF);  // Seek to end
        
        printf("Editing %s. Type your text line by line.\n", argv[1]);
        printf("Type 'END' on a new line to finish editing.\n");
        
        char line[256];
        bool editing = true;
        
        while (editing) {
            printf("> ");
            memset(line, 0, sizeof(line));
            read_input(line, sizeof(line));
            
            // Check for END command
            if (strcmp(line, "END") == 0) {
                editing = false;
                break;
            }
            
            // Add newline to the line
            strcat(line, "\n");
            
            // Write the line to file
            uint64_t write_bytes = syscall_write(user_disk_no, file, (void *)line, strlen(line));
            
            if (write_bytes != strlen(line)) {
                printf("Warning: Failed to write line to file\n");
            }
        }
        
        printf("Finished editing %s\n", argv[1]);
        syscall_close(user_disk_no, file);

    } else if (strcmp(argv[0], "append") == 0) {
        // Simple append command (like write but doesn't truncate)
        if (argc < 3) {
            printf("Usage: append <file> <text>\n");
            printf("Example: append log.txt New log entry\n");
            return;
        }
        
        void *file = (void*)syscall_open(user_disk_no, argv[1], FA_OPEN_APPEND | FA_WRITE);
        
        if (!file) {
            printf("Cannot open file %s for appending\n", argv[1]);
            return;
        }

        // Combine remaining arguments
        char content[256];
        content[0] = '\0';
        
        for (int i = 2; i < argc; i++) {
            strcat(content, argv[i]);
            if (i < argc - 1) {
                strcat(content, " ");
            }
        }
        
        strcat(content, "\n");
        
        uint64_t write_bytes = syscall_write(user_disk_no, file, (void *)content, strlen(content));
        
        if (write_bytes == strlen(content)) {
            printf("Appended %lu bytes to %s\n", write_bytes, argv[1]);
        } else {
            printf("Failed to append to %s\n", argv[1]);
        }
        
        syscall_close(user_disk_no, file);

    } else if (strcmp(argv[0], "clear") == 0 || strcmp(argv[0], "cls") == 0) {

        // Clear screen with black background
        if(syscall_cls_color(0xFF000000) != 0){   // RGB: black
            printf("Failed to clear screen!\n");
        }  

    }else if (strcmp(argv[0], "help") == 0) {
        printf("Available commands:\n");
        printf("  help - Show this help message\n");
        printf("  clear/cls - Clear the screen\n");
        printf("  exit - Exit the shell\n");
        printf("  echo <text> - Print text to the console\n");
        printf("  ls [dir] - List files in directory (default is current)\n");
        printf("  cat <file> - Display file content\n");
        printf("  mkdir <dir> - Create a new directory\n");
        printf("  rm <file> - Remove a file\n");
        printf("  run <program> - Run a user program\n");
        printf("  touch <filename> - Create or read/write a file\n");
        printf("  append <file> <text> - Append text to a file\n");
        printf("  write <file> <text> - Write text to a file\n");
        printf("  edit <file> - Edit a file (append mode)\n");
        printf("  cd <path> - Change Directory.\n");
        printf("  cwd - Current Working Directory.\n");
    } else {
        printf("\nUnknown command: %s\n", argv[0]);
        printf("Type 'help' for a list of commands.\n");
    }
}



char welcome_message[] = "Welcome to KeblaOS Shell!\n";

void start_user_shell() {

    char input[MAX_INPUT];
    char *argv[MAX_ARGS];

    printf("%s", welcome_message);

    while (true) {
        print_prompt();
        memset(input, 0, sizeof(input));    // Clear input buffer
        read_input(input, sizeof(input));   // Read user input

        int argc = tokenize(input, argv);   // Tokenize input into argv array and get argc

        printf("\n");                       // Creating a new line for better readability

        // Debug
        // for(int i=0; i<argc; i++){
        //     printf("argv[%d]: %s\n", i, argv[i]);
        // }

        handle_command(argc, argv);         // Handle the command based on argc and argv
    }

    while (true) {}                         // Halt
}





