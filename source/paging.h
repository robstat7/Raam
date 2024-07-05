typedef struct _pml4_table {
	unsigned long long p :1;
	unsigned long long rw :1;
	unsigned long long us :1;
	unsigned long long pwt :1;
	unsigned long long pcd :1;
	unsigned long long a :1;
	unsigned long long ignored_1 :1;
	unsigned long long ps :1;
	unsigned long long ignored_2 :3;
	unsigned long long r :1;
	unsigned long long pdpt_phy_addr :38;
	unsigned long long reserved :2;
	unsigned long long ignored_3 :11;
	unsigned long long xd :1;
}__attribute__((packed)) pml4_table_t;

typedef struct _pae_page_directory_pointer_table{
	unsigned long long p :1;
	unsigned long long rw :1;
	unsigned long long us :1;
	unsigned long long pwt :1;
	unsigned long long pcd :1;
	unsigned long long a :1;
	unsigned long long d :1;
	unsigned long long ps :1;
	unsigned long long g :1;
	unsigned long long ignored_1 :2;
	unsigned long long r :1;
	unsigned long long pat :1;
	unsigned long long reserved_1 :17;
	unsigned long long pd_phy_addr :20;
	unsigned long long reserved_2 :2;
	unsigned long long ignored_2 :7;
	unsigned long long pk :4;
	unsigned long long xd :1;
}__attribute__((packed)) pae_page_directory_pointer_table_t;

