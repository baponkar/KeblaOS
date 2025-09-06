
#include "../include/stdio.h"

#include "../include/syscall.h"

// Userside system call function to manage all system call
static uint64_t system_call(uint64_t rax, uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t r10, uint64_t r8, uint64_t r9){
    
    uint64_t out;

    asm volatile (
        "mov %[_rax], %%rax\n"   // System Call Number
        "mov %[_rdi], %%rdi\n"   // Argument 1
        "mov %[_rsi], %%rsi\n"   // Argument 2
        "mov %[_rdx], %%rdx\n"   // Argument 3
        "mov %[_r10], %%r10\n"   // Argument 4
        "mov %[_r8], %%r8\n"     // Argument 5
        "mov %[_r9], %%r9\n"     // Argument 6
        "int $0x80\n"            // Trigger System Call Interrupt
        "mov %%rax, %[_out]\n"   // Storing Output
        : [_out] "=r" (out)
        : [_rax] "r" (rax), [_rdi] "r" (rdi), [_rsi] "r" (rsi), [_rdx] "r" (rdx), [_r10] "r" (r10), [_r8] "r" (r8), [_r9] "r" (r9)
        : "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9"   // Clobber registers
    );

    return out;
}

// ------------------------------- Time Manage System Call -------------------------------

time_t syscall_time(time_t *t) {
    time_t now = (time_t) system_call((uint64_t)INT_TIME, (uint64_t)t, (uint64_t)0, (uint64_t)0,  (uint64_t)0,(uint64_t)0, (uint64_t)0);
    if (t) *t = now;
    return now;
}

int syscall_clock_gettime(int clk_id, struct timespec *tp) {
    return (int) system_call((uint64_t)INT_CLOCK_GETTIME, (uint64_t)clk_id, (uint64_t)tp, (uint64_t)0, (uint64_t)0, (uint64_t)0, (uint64_t)0);
}

int syscall_gettimeofday(struct timeval *tv, struct timezone *tz) {
    return (int) system_call((uint64_t)INT_CLOCK_GETTIMEOFDAY, (uint64_t)tv, (uint64_t)tz, (uint64_t)0, (uint64_t)0, (uint64_t)0, (uint64_t)0);
}

clock_t syscall_times(struct tms *buf) {
    return (clock_t) system_call((uint64_t)INT_TIMES, (uint64_t)buf, (uint64_t)0, (uint64_t)0, (uint64_t)0, (uint64_t)0, (uint64_t)0);
}

uint64_t syscall_get_uptime(void) {
    return system_call((uint64_t)INT_SYSCALL_GET_UP_TIME, (uint64_t)0, (uint64_t)0, (uint64_t)0, (uint64_t)0, (uint64_t)0, (uint64_t)0);
}



// ------------------------------- General System Call -------------------------

int syscall_keyboard_read(uint8_t *buffer, size_t size) {
    if (!buffer || size == 0) {
        return -1; // Invalid parameters
    }

    return system_call((uint64_t) INT_SYSCALL_KEYBOARD_READ, (uint64_t) buffer, (uint64_t) size, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0);
}


int syscall_print(const char *msg) {
    if (!msg) {
        return -1; // Invalid message
    }

    return system_call((uint64_t) INT_SYSCALL_PRINT, (uint64_t) msg, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0);
}

int syscall_exit() {
    return system_call((uint64_t) INT_SYSCALL_EXIT, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0);    
}

int syscall_print_rax() {
    return system_call((uint64_t) INT_SYSCALL_PRINT_RAX, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0); 
}


uint64_t syscall_uheap_alloc(size_t size, enum allocation_type type) {
    if (size == 0) {
        return 0;
    }

    return system_call((uint64_t) INT_SYSCALL_ALLOC, (uint64_t) size, (uint64_t) type, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0); 
}

uint64_t syscall_uheap_free(void *ptr, size_t size) {
    if (!ptr || size == 0) {
        return -1; // Invalid pointer
    }

    return system_call((uint64_t) INT_SYSCALL_FREE, (uint64_t) ptr, (uint64_t) size, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0); 
}




// ------------------------------- Process Manage --------------------

void *syscall_create_process(char* process_name){
    return (void *) system_call((uint64_t) INT_CREATE_PROCESS , (uint64_t) process_name, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0); 
}

int syscall_delete_process(void *process){
    return system_call((uint64_t) INT_DELETE_PROCESS , (uint64_t) process, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0); 
}

void *syscall_get_process_from_pid(size_t pid){
    if(pid < 0){
        return NULL;
    }
    return (void *) system_call((uint64_t) INT_GET_PROCESS_FROM_PID, (uint64_t) pid, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0); 
}

void *syscall_get_current_process(){
    return (void *) system_call((uint64_t) INT_GET_CURRENT_PROCESS, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0); 
}



// ---------------------------- Thread Manage --------------------------------
void *syscall_create_thread(void* parent, const char* thread_name, void (*function)(void*), void* arg){
    return (void *) system_call((uint64_t) INT_CREATE_THREAD, (uint64_t) parent, (uint64_t) thread_name, (uint64_t) function, (uint64_t) arg, (uint64_t) 0, (uint64_t) 0);
}

void *syscall_delete_thread(void *thread){
    return (void *) system_call((uint64_t) INT_DELETE_THREAD, (uint64_t) thread, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0);
}



// ------------------------------- VFS Manage ------------------------
int64_t syscall_vfs_mkfs(int fs_type, char *disk){
    if(fs_type < 0x1){
        return -1;
    }
    
    if(!disk){
        return -1;
    }

    return system_call((uint64_t)INT_VFS_MKFS, (uint64_t) fs_type, (uint64_t) disk, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0);
}


int64_t syscall_vfs_init(char *fs_name) {
    if (!fs_name) {
        return -1;                  // Invalid path
    }
    return system_call((uint64_t)INT_VFS_INIT, (uint64_t) fs_name, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0);
}


uint64_t syscall_mount(char *disk_path) {

    return system_call((uint64_t) INT_SYSCALL_MOUNT, (uint64_t) disk_path, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0); 
}

// Opening a file by path name
uint64_t syscall_open(const char *path, uint64_t flags) {

    if (!path) {
        return -1;                  // Invalid path
    }

    return system_call((uint64_t) INT_SYSCALL_OPEN, (uint64_t) path, (uint64_t) flags, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0);
    
}


uint64_t syscall_close(void *file) {
    if (!file) {
        return -1; // Invalid file pointer
    }

    return system_call((uint64_t) INT_SYSCALL_CLOSE, (uint64_t) file, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0);
}

uint64_t syscall_read(void *file, uint64_t offset, void *buf, uint32_t size) {
    
    if (!file || !buf || size == 0) {
        return -1; // Invalid parameters
    }

    return system_call((uint64_t) INT_SYSCALL_READ, (uint64_t) file, (uint64_t) offset, (uint64_t) buf, (uint64_t) size, (uint64_t) 0, (uint64_t) 0);
}

uint64_t syscall_write(void *file, uint64_t offset, void *buf, uint32_t btw) {
    
    if (!file || !buf || btw == 0) {
        return (uint64_t)-1;    // Invalid parameters
    }

    return system_call((uint64_t) INT_SYSCALL_WRITE, (uint64_t) file, (uint64_t) offset, (uint64_t) buf, (uint64_t) btw, (uint64_t) 0, (uint64_t) 0);
}

uint64_t syscall_lseek(void *file, uint32_t offs) {
    if (!file) {
        return -1;                  // Invalid parameters
    }

    return system_call((uint64_t) INT_SYSCALL_LSEEK, (uint64_t) file, (uint64_t) offs, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0);
}

uint64_t syscall_truncate(char *path, uint32_t offset) {
    if (!path) {
        return -1;                  // Invalid parameters
    }

    return system_call((uint64_t) INT_SYSCALL_TRUNCATE, (uint64_t) path, (uint64_t) offset, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0);
}

uint64_t syscall_unlink(char *path){
    if(!path){
        return (uint64_t)-1;
    }

    return system_call((uint64_t)INT_SYSCALL_UNLINK, (uint64_t)path, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0);
}

uint64_t syscall_opendir(const char *path){
    if(!path) {
        return -1;
    }

    return system_call((uint64_t) INT_SYSCALL_OPENDIR, (uint64_t) path, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0);
}

uint64_t syscall_closedir(void * dir_ptr){
    if(!dir_ptr){
        return -1;
    }
    return system_call((uint64_t) INT_SYSCALL_CLOSEDIR, (uint64_t) dir_ptr, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0);
}


uint64_t syscall_readdir(void * dir_ptr){
    if(!dir_ptr){
        return -1;
    }
    return system_call((uint64_t) INT_SYSCALL_READDIR, (uint64_t) dir_ptr, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0);
}

uint64_t syscall_mkdir(void * dir_ptr){
    if(!dir_ptr){
        return -1;
    }
    return system_call((uint64_t) INT_SYSCALL_MKDIR, (uint64_t) dir_ptr, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0);
}


int syscall_list_dir(const char* path){
    if(!path){
        return -1;
    }
    return system_call((uint64_t)INT_SYSCALL_LIST, (uint64_t)path, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0);
}



int syscall_getcwd(void *buf, size_t size){
    if(!buf || !size){
        return -1;
    }
    return system_call((uint64_t)INT_SYSCALL_GETCWD, (uint64_t)buf, (uint64_t) size, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0);
}

int syscall_chdir(const char *path){
    if(!path){
        return -1;
    }
    return system_call((uint64_t)INT_SYSCALL_CHDIR, (uint64_t)path, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0);
}


int syscall_chdrive(const char *path){
    if(!path){
        return -1;
    }
    return system_call((uint64_t)INT_SYSCALL_CHDRIVE, (uint64_t)path, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0, (uint64_t) 0);
}






