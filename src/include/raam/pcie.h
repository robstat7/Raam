#ifndef PCIE_H
#define PCIE_H

#include <stdint.h>

struct enhanced_config_base_struct {                                            
        uint64_t *base_addr;                                                    
        uint16_t pci_seg_grp_num;                                               
        uint8_t start_bus_num;                                                  
        uint8_t end_bus_num;                                                    
        uint32_t reserved;                                                      
};

void pcie_init(void);

#endif	// PCIE_H
