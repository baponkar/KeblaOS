#include <stdint.h>
#include <stdbool.h>

struct pci_device_type{
    uint8_t class_code;
    uint8_t sub_class_code;
    uint8_t prog_if_reg;
};
typedef struct pci_device_type_t ;



uint32_t pci_read(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
void pci_write(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value);
void pci_scan();



