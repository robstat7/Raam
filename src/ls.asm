;
; 'ls' command's implementation
;
ROOT_PARTITION_FIRST_SECTOR = 996558848


section '.text' code executable readable

list_files_in_root_directory:
  mov edi, ROOT_PARTITION_FIRST_SECTOR
  xor esi, esi
  call nvme_read
  mov rdi, rax
  call printk
  ret


section '.data' data readable writeable
