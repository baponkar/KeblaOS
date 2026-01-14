

#include "../libc/include/syscall.h"
#include "../libc/include/stdio.h"
#include "../libc/include/string.h"


#include "installer.h"



void instll(){

    printf("Installing KeblaOS in the available Disk...........\n");

    if(syscall_vfs_init(0) != 0){
        printf("VFS initialization is failed!\n");
    }else{
        printf("Successfully FAT32 VFS Created in disk 0.\n");
    }

    char *disk = "0:";
    if(syscall_vfs_mkfs(1, 2, 1) != 0){
        printf("Formatting first disk failed!\n");
    }else{
        printf("Successfully Format Disk 0 with FAT32 Filesystem.\n");
    }

    if(syscall_mount(1) != 0){
        printf("Mount Device 0: is failed!\n");
    }else{
        printf("Mount Disk 0\n");
    }

    char *boot_dir = "0:/boot";
    char *limine_dir = "0:/boot/limine";
    char *EFI_dir = "0:/EFI";
    if((syscall_mkdir(1, (void *) boot_dir) == -1) || (syscall_mkdir(1, (void *) limine_dir) == -1)){
        printf("Creating /boot directory failed!\n");
    }

    if(syscall_mkdir(1, (void *) limine_dir) == -1){
        printf("Creating /boot/limine directory failed!\n");
    }

    if(syscall_mkdir(1, (void *) limine_dir) == -1){
        printf("Creating /boot/limine directory failed!\n");
    }

    if(syscall_mkdir(1, (void *) EFI_dir) == -1){
        printf("Creating /EFI directory failed!\n");
    }

    printf("Successfully /boot and /boot/limine /EFI directories created.\n");

    // Now detect USB Disk which is containing Limine bootloader and Kernel.bin
    char *cdrom = "1:";
    
    if(syscall_mount(1) == 0){
        printf("Sucessfully mount cdrom\n");
    }




    printf("Successfully KeblaOS installed in the Disk 0.................\n");
}






