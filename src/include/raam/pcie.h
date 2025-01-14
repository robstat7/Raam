#ifndef PCIE_H
#define PCIE_H

#include <stdint.h>

#define PCI_INVALID_VENDOR_ID	0xffff

#define MAX_PCI_BUS_DEV		32

struct enhanced_config_base_struct {                                            
        uint64_t base_addr;                                                    
        uint16_t pci_seg_grp_num;                                               
        uint8_t start_bus_num;                                                  
        uint8_t end_bus_num;                                                    
        uint32_t reserved;                                                      
}__attribute__((packed));

struct pcie_ecam_struct {
	uint64_t *base;
	uint8_t start_bus_num;
	uint8_t end_bus_num;
}__attribute__((packed));


extern struct pcie_ecam_struct pcie_ecam;

void pcie_init(void);

#endif	// PCIE_H
