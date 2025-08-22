
/*
    POSIX Functions which will be use to enable standard c library.
*/

#include "../include/syscall.h"

#include "../include/posix.h"

/*
// --------------File Operations-----------------------//
int open(const char *pathname, int flags, mode_t mode) {

}

int read(int fd, void *buf, size_t count) {

}

int write(int fd, const void *buf, size_t count){

}

int close(int fd){

}

int lseek(int fd, off_t offset, int whence){

}

int ftruncate(int fd, off_t length){

}

int unlink(const char *pathname){

}

int stat(const char *pathname, struct stat *buf){

}


// --------------------Directory Operations------------------------------//
int opendir(const char *name){

}

int readdir(DIR *dirp){

}

int closedir(DIR *dirp){

}

int mkdir(const char *pathname, mode_t mode){

}

int rmdir(const char *pathname){

}


// ----------------------------Process Management------------------------//

// Creates a new process by duplicating the current process.
int fork(){

}

// Replaces the current process image with a new one.
int execve(const char *path, char *const argv[], char *const envp[]) {

}

// Terminates the process with given status.
int exit(int status) {

}   

int wait(int *status){

}

int kill(pid_t pid, int sig){

}

int getpid(){

}

int getppid(){

}


// ---------------------------------------Memory Management-------------------------//
// Increases or decreases the data segment (heap).
int sbrk(intptr_t increment){

}

// Maps file or anonymous memory to process space.
int mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset){

}

// Unmaps memory allocated via mmap.
int munmap(void *addr, size_t length){

}


// Console I/O
int read(){

}

int and write(){

}


// -----------------------Time Management----------------------------//
int sleep(unsigned int seconds){

}

int usleep(useconds_t usec) {    // Sleeps in microseconds.

}

int gettimeofday(struct timeval *tv, struct timezone *tz){

}


int chdir() { // Change Directory.

}  

int getcwd(){   // Get Current Directory

}

int dup() {

}

int dup2(){

}

int pipe(){     // Inter Process Communication

}


int selecct(){

}


int ioctl(){    // Device control and configuration

}	


*/








