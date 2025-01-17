#ifndef NVME_H
#define NVME_H

#include <stdint.h>
#include <stdbool.h>

#define NVME_CLASS_CODE		0x01	/* Mass Storage Controller */
#define NVME_SUBCLASS		0x08	/* Non-Volatile Memory Controller */
#define NVME_PROG_IF		0x02	/* NVM Express */

/* header type */
#define STANDARD_HEADER		0x0

/* nvme commands */
#define CMD_IDENTIFY		0x6
#define CMD_IGNORED		0x0
#define CNS_CONTROLLER		0x1

#define PAGE_SIZE		4096

struct common_config_space_header_struct {
	uint16_t vendor_id;
	uint16_t dev_id;
	uint16_t cmd;
	uint16_t status;
	uint8_t revision_id;

	// class code fields
	uint8_t prog_if;
	uint8_t subclass;
	uint8_t class_code;

	uint8_t cache_line_size;
	uint8_t latency_timer;
	uint8_t header_type;
	uint8_t bist;
}__attribute__((packed));

struct nvme_pcie_dev_info_struct {
	int found;
	uint16_t bus;
	uint8_t dev;
	uint8_t func;
};	

struct header_type_0_table_struct {
	struct common_config_space_header_struct h;
	uint32_t bar0;
	uint32_t bar1;
	uint32_t bar2;
	uint32_t bar3;
	uint32_t bar4;
	uint32_t bar5;
	uint32_t cardbus_cis_ptr;
	uint16_t subsys_vendor_id;
	uint16_t subsys_id;
	uint32_t expansion_rom_base;
	uint8_t capabilities_ptr;
	uint8_t reserved1;
	uint16_t reserved2;
	uint32_t reserved3;
	uint8_t interrupt_line;
	uint8_t interrupt_pin;
	uint8_t min_grant;
	uint8_t max_latency;
}__attribute__((packed));

/* please see NVM Express Revision 1.3 - 3.1 Register Definition */
struct register_map_struct {
	uint64_t cap;
	uint32_t vs;
	uint32_t intms;
	uint32_t intmc;
	uint32_t cc;
	uint32_t reserved1;
	uint32_t csts;
	uint32_t nssr;
	uint32_t aqa;
	uint64_t asq;
	uint64_t acq;
	uint32_t cmbloc;
	uint32_t cmbsz;
	uint32_t bpinfo;
	uint32_t bprsel;
	uint64_t bpmbl;
	char reserved2[3760];
	char reserved3[256];
	uint32_t sq0tdbl;
	uint32_t cq0hdbl;
	uint32_t sq1tdbl;
}__attribute__((packed));

struct submission_queue_commands_struct {
	uint32_t cdw0;
	uint32_t cdw1;
	uint32_t cdw2;
	uint32_t cdw3;
	uint64_t cdw4_5;
	uint64_t cdw6_7;
	uint64_t cdw8_9;
	uint32_t cdw10;
	uint32_t cdw11;
	uint32_t cdw12;
	uint32_t cdw13;
	uint32_t cdw14;
	uint32_t cdw15;
}__attribute__((packed));
	
int nvme_init(const uint8_t *system_variables);
static void find_controller(struct nvme_pcie_dev_info_struct *controller);
static int check_function(uint16_t bus, uint8_t dev, uint8_t func);
static uint64_t get_config_space_phy_mmio_addr(uint32_t bus, uint32_t dev,
					       uint32_t func);
static uint64_t *get_nvme_base(struct nvme_pcie_dev_info_struct
			       *controller_info);
static bool reset_controller(void);
static bool wait_for_reset_complete(void);
static void configure_admin_queues(void);
char *align_to_4096(char *addr);
static bool nvme_init_enable_wait(void);
static void get_identify_controller_data_structure(void);
static void send_admin_command(const uint32_t cdw0, const uint32_t cdw1,
                               const uint64_t cdw6_7, const uint32_t cdw10,
                               const uint32_t cdw11);
static void nvme_admin_savetail(const uint32_t admin_tail_val,
				char* nvme_admin_tail,
				uint32_t old_admin_tail_val);
static void nvme_admin_wait(uint32_t *acqb_ptr);
static char *get_next_4096_alligned_address(char *addr);
static void build_command_structure(
                        struct submission_queue_commands_struct *sq_cmds,
                        const uint32_t cdw0, const uint32_t cdw1,
                        const uint64_t cdw6_7, const uint32_t cdw10,
                        const uint32_t cdw11);
static void create_io_queues(void);

#endif	// NVME_H
