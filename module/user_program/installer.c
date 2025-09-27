

#include "../libc/include/syscall.h"
#include "../libc/include/stdio.h"
#include "../libc/include/string.h"


#include "installer.h"



void instll(){

    printf("Installing KeblaOS in the available Disk...........\n");

    if(syscall_vfs_init("fat") != 0){
        printf("VFS initialization is failed!\n");
    }else{
        printf("Successfully FAT32 VFS Created in disk 0.\n");
    }

    char *disk = "0:";
    if(syscall_vfs_mkfs(2, disk)){
        printf("Formatting first disk failed!\n");
    }else{
        printf("Successfully Format Disk 0 with FAT32 Filesystem.\n");
    }

    if(syscall_mount(disk) != 0){
        printf("Mount Device 0: is failed!\n");
    }else{
        printf("Mount Disk 0\n");
    }

    char *boot_dir = "0:/boot";
    char *limine_dir = "0:/boot/limine";
    char *EFI_dir = "0:/EFI";
    if((syscall_mkdir((void *) boot_dir) == -1) || (syscall_mkdir((void *) limine_dir) == -1)){
        printf("Creating /boot directory failed!\n");
    }

    if(syscall_mkdir((void *) limine_dir) == -1){
        printf("Creating /boot/limine directory failed!\n");
    }

    if(syscall_mkdir((void *) limine_dir) == -1){
        printf("Creating /boot/limine directory failed!\n");
    }

    if(syscall_mkdir((void *) EFI_dir) == -1){
        printf("Creating /EFI directory failed!\n");
    }

    printf("Successfully /boot and /boot/limine /EFI directories created.\n");

    // Now detect USB Disk which is containing Limine bootloader and Kernel.bin
    char *cdrom = "1:";
    
    if(syscall_mount(cdrom) == 0){
        printf("Sucessfully mount cdrom\n");
    }




    printf("Successfully KeblaOS installed in the Disk 0.................\n");
}






