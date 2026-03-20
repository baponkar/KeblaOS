
/*
    Kernel.c
    Build Date  : 16-12-2024
    Last Update : 04-03-2026
    Description : KeblaOS is a x86 architecture based 64 bit Operating System. Currently it is using Limine Bootloader.
    Reference   : https://wiki.osdev.org/Limine
                  https://github.com/limine-bootloader/limine-c-template
                  https://wiki.osdev.org/Limine_Bare_Bones
                  https://wiki.osdev.org/SSE
                  https://allthingsembedded.com/2018/12/29/adding-gpt-support-to-fatfs/
*/



#include "kmain.h"

bool debug_on = false;

bool install = false;    // This variable is set to true when the installer is running, otherwise it is false.

extern uint64_t fb0_width;
extern uint64_t fb0_height;
extern uint64_t fb0_pitch;
extern uint16_t fb0_bpp;


extern ring_buffer_t* keyboard_buffer;          // To get the keyboard input

extern Disk *disks;
extern int disk_count; 

#define MAIN_DISK_TOTAL_SECTORS 2097152         // 1GB = 1 * 1024 * 1024 * 1024 / 512

#define ESP_START_LBA 2048
#define ESP_SECTORS 204800   // 100 MB = 100 * 1024 * 1024 / 512

#define DATA_PART_START_LBA 206848    // (ESP_START_LBA + ESP_SECTORS)
#define DATA_PART_SECTORS 1888256     // (MAIN_DISK_TOTAL_SECTORS - ESP_SECTORS - (2 * ESP_START_LBA)) // For safety deduct ESP_START_LBA

int boot_disk_no = 0;    // The Disk no which boot KeblaOS
int main_disk_no = 1;    // The Main Disk Present in the System

extern int disk_no;      // The Disk Which have FAT32 Filesystem



void kmain(){
    
    serial_init("Successfully Serial Printing initialized!\n");

    get_bootloader_info();
    init_vga();
    
    print_bootloader_info();

    get_set_memory();
    
    init_bs_cpu_core();
    
    // Initialize APIC and IOAPIC
    if(!has_apic()) printf("[Error] This System does not have APIC.\n");
        
    init_all_cpu_cores();    // Starts all CPU cores

    if(!pci_exists()){
        printf("[Error] This system do not have PCI!\n");
    }

    init_controllers();     // This function has PCI Scan

    // Detect and Initialize all Disks
    if(kebla_get_disks() <= 0){
        printf("No Disk Found!\n");
    }
    printf("[KMAIN] Total %d Disks Found.\n", disk_count);

    // Print detected disk type
    for(int disk=0; disk < disk_count; disk++){
        printf("[KMAIN] Disk No %d: Disk Type: %d\n", disk, find_disk_type(disk));
    }

    // print keblaos boot from where
    switch(find_disk_type(0)){
        case DISK_TYPE_SATAPI:
            printf("[KMAIN] KeblaOS is Booting from ISO Disk type.\n\n");
            break;
        case DISK_TYPE_AHCI_SATA:
            printf("[KMAIN] KeblaOS is Booting from AHCI SATA Disk type.\n\n");
            break;
        case DISK_TYPE_IDE_PATA:
            printf("[KMAIN] KeblaOS is Booting from IDE PATA Disk type.\n\n");
            break;
        default:
            printf("[KMAIN] KeblaOS is Booting from an Unknown Disk type.\n\n");
    }

    // Update boot_disk_no/main_disk_no on based boot disk
    if(disk_count == 1 && disks[0].type != DISK_TYPE_SATAPI){
        boot_disk_no = 0;
        main_disk_no = 0;
        disk_no = 0;
    }else if(disk_count > 1){
        boot_disk_no = 0;
        main_disk_no = 1;
        disk_no = 1;
    }

    // Checking Installation in main_disk
    if(!verify_installation(main_disk_no, ESP_START_LBA)){
        printf("[KMAIN] KeblaOS is not installed in Disk %d\n", main_disk_no);
    }else{
        printf("[KMAIN] KeblaOS is already installed in the Disk %d.\n", main_disk_no);
    }
    
    if(install){
        if(!uefi_install(boot_disk_no, main_disk_no, ESP_START_LBA, ESP_SECTORS, MAIN_DISK_TOTAL_SECTORS)){
            printf("[KMAIN] Failed to Install KeblaOS in Disk %d.\n", boot_disk_no);
        }
        printf("[KMAIN] Successfully Install KeblaOS in Disk %d.\n", boot_disk_no);
    }
    
    // fat32_fs_test(main_disk_no,  DATA_PART_START_LBA, DATA_PART_SECTORS);

    // vfs_test(main_disk_no, DATA_PART_START_LBA, VFS_FAT32);

    // if(!create_fat32_volume( DATA_PART_START_LBA, DATA_PART_SECTORS)){
    //     printf("Failed to crate FAT32 Volume at Sector %d\n", DATA_PART_START_LBA);
    // }else{
    //     printf("Successfully crated FAT32 Volume at Sector %d\n", DATA_PART_START_LBA);
    // }

    PartitionEntry *partitions = get_partitions(main_disk_no);
    if(!partitions){
        printf("Failed to get partitions array\n");
    }

    for(int i=0; i < MAX_PARTITIONS; i++){
        PartitionEntry part = partitions[i];

        char *guid_string = (char *)malloc(17);
        guid_to_string(part.partition_guid, guid_string);
        guid_string[16] = '\0';

        char *guid_type_string = (char *)malloc(17);
        guid_to_string(part.partition_type_guid, guid_type_string);
        guid_type_string[16] = '\0';

        printf("Entry No: %d \
            \n\tpartition_no: %d \
            \n\tstart_lba: %d   \
            \n\tsectors: %d      \
            \n\tpartition_guid: %s \
            \n\tPartition Type: %s\n",
            i, part.partition_no, part.start_lba, part.sectors, guid_string, guid_type_string);
    }

    if(vfs_mount(main_disk_no, DATA_PART_START_LBA, VFS_FAT32) != 0){
        printf("Failed to Mount 2nd Partition\n");
    }else{
        printf("Successfully Mount 2nd Partition\n");
    }

    FAT32_FILE *fp = malloc(sizeof(FAT32_FILE));
    if(!fp){
        printf("Memory allocation failed for fp!\n");
    }
    memset(fp, 0, sizeof(FAT32_FILE));

    void *opened_file = vfs_open(main_disk_no, "/testfile.txt", VFS_CREATE_ALWAYS | VFS_WRITE | VFS_READ);

    if(!opened_file){
        printf("Failed to create testfile.txt\n");
    }else{
        printf("Successfully opened testfile.txt\n");
    }

    char *text_data = "This is a text data.";

    if(vfs_write(main_disk_no, opened_file, text_data, strlen(text_data)) != 0){
        printf("Failed to write data in /testfile.txt\n");
    }else{
        printf("Successfully written data in /testfrile.txt\n");
    }

    char *buffer = malloc(strlen(text_data));

    if(vfs_read(main_disk_no, opened_file, buffer, strlen(text_data)) != 0){
        printf("Failed to  read /testfile.txt in buffer\n");
    }else{
        printf("Successfully read /testfile.txt in buffer\n");
    }

    if(vfs_close(main_disk_no, opened_file) != 0){
        printf("Failed to close /testfile.txt\n");
    }else{
        printf("Successfully Closed the /estfile.txt\n");
    }
   
    // test_time_functions();

    // mouse_init();

    // ugui_test_1();
    // desktop_init();

    // test_e1000_driver();
    // start();
    // test_e1000();


    // switch_to_core(3);

    // init_user_mode();

    // Load and parse kernel modules by using limine bootloader
    // get_kernel_modules_info();
    // print_kernel_modules_info();
    // load_user_elf_and_jump();

    // acpi_poweroff();
    // acpi_reboot();

    // while(true){
    //     printf("Time: %d, Random Number: %d\n", get_apic_ticks(), rand());
    //     sleep_seconds(0, 10);
    // }

    // switch_to_core(2);
    

    halt_kernel();
}












