#include <stdint.h>

typedef struct _pml4_table {
	uint64_t p :1;
	uint64_t rw :1;
	uint64_t us :1;
	uint64_t pwt :1;
	uint64_t pcd :1;
	uint64_t a :1;
	uint64_t avl_1 :1;
	uint64_t reserved_zero_1 :1;
	uint64_t avl_2:4;
	uint64_t pdpt_phy_addr :38;
	uint64_t reserved_zero_2 :2;
	uint64_t avl_3 :11;
	uint64_t xd :1;
}__attribute__((packed)) pml4_table_t;

typedef struct _pae_page_directory_pointer_table{
	uint64_t p :1;
	uint64_t rw :1;
	uint64_t us :1;
	uint64_t pwt :1;
	uint64_t pcd :1;
	uint64_t a :1;
	uint64_t avl_1 :1;
	uint64_t ps_zero :1;
	uint64_t avl_2 :4;
	uint64_t pd_phy_addr :38;
	uint64_t reserved_zero_1 :2;
	uint64_t avl_3 :11;
	uint64_t xd :1;
}__attribute__((packed)) pae_page_directory_pointer_table_t;

typedef struct _pae_page_directory_table {
	uint64_t p :1;
	uint64_t rw :1;
	uint64_t us :1;
	uint64_t pwt :1;
	uint64_t pcd :1;
	uint64_t a :1;
	uint64_t avl_1 :1;
	uint64_t ps_zero :1;
	uint64_t avl_2 :4;
	uint64_t pt_phy_addr :38;
	uint64_t reserved_zero_1 :2;
	uint64_t avl_3 :11;
	uint64_t xd :1;
}__attribute__((packed)) pae_page_directory_table_t;

typedef struct _pae_page_table {
	uint64_t p :1;
	uint64_t rw :1;
	uint64_t us :1;
	uint64_t pwt :1;
	uint64_t pcd :1;
	uint64_t a :1;
	uint64_t d :1;
	uint64_t pat :1;
	uint64_t g :1;
	uint64_t avl_1 :3;
	uint64_t page_4k_phy_addr :38;
	uint64_t reserved_zero_1 :2;
	uint64_t avl_2 :7;
	uint64_t pk :4;
	uint64_t xd :1;
}__attribute__((packed)) pae_page_table_t;

