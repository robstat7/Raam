;
; note: using FAT12 formatted NVMe partition for root
;
ROOT_PARTITION_FIRST_SECTOR = 996558848   ; NOTE: CAUTION!!!


struc FAT_EXTBS_16 {
  ; extended fat12 and fat16 stuff
  .bios_drive_num             db ?
  .reserved1                  db ?
  .boot_signature             db ?
  .volume_id                  dd ?
  .volume_label               db 11 dup (?)
  .fat_type_label             db 8 dup (?)
}
struct FAT_EXTBS_16

struc FAT_BS {
  .bootjmp                    db 3 dup (?)
  .oem_name                   db 8 dup (?)
  .bytes_per_sector           dw ?
  .sectors_per_cluster        db ?
  .reserved_sector_count      dw ?
  .table_count                db ?
  .root_entry_count           dw ?
  .total_sectors_16           dw ?
  .media_type                 db ?
  .table_size_16              dw ?
  .sectors_per_track          dw ?
  .head_side_count            dw ?
  .hidden_sector_count        dd ?
  .total_sectors_32           dd ?
  .extended_section           FAT_EXTBS_16
}
struct FAT_BS

struc DIR_ENTRY_STRUCT {
  .file_name                  db 11 dup (?)
  .file_attributes            db ?
  .reserved1                  db ?
  .creation_time_100th_sec    db ?
  .creation_time              dw ?
  .creation_date              dw ?
  .last_accessed_date         dw ?
  .reserved2                  dw ?
  .last_modified_time         dw ?
  .last_modified_date         dw ?
  .first_cluster_number       dw ?
  .file_size                  dd ?      ; in bytes
}
struct DIR_ENTRY_STRUCT


section '.text' code executable readable

; this function finds the LBA for the root directory.
; args:
;   nothing
; returns:
;   @rax = root directory's LBA
find_root_directory_lba:
  push rbp
  mov rbp, rsp

  sub rsp, 12

  root_dir_sectors equ dword [rbp - 4]
  first_data_sector equ dword [rbp - 8]
  first_root_dir_sector equ dword [rbp - 12]

  mov edi, ROOT_PARTITION_FIRST_SECTOR
  xor esi, esi
  call nvme_read
  mov r8, rax

  ; first calculate the size of the root directory.
  ; This calculation will round up. 32 is the size of a FAT directory in bytes.
  xor eax, eax
  mov ax, word [r8 + FAT_BS.root_entry_count]
  imul ax, 32
  xor ecx, ecx
  mov cx, word [r8 + FAT_BS.bytes_per_sector]
  xor ebx, ebx
  mov bx, cx
  dec cx
  add ax, cx
  xor edx, edx
  div ebx
  mov root_dir_sectors, eax

  ; now find the first data sector (that is, the first sector in which
  ; directories and files may be stored)
  xor eax, eax
  mov ax, word [r8 + FAT_BS.table_size_16]
  xor ebx, ebx
  mov bl, byte [r8 + FAT_BS.table_count]
  imul ebx, eax
  mov ax, word [r8 + FAT_BS.reserved_sector_count]
  add eax, ebx
  add eax, root_dir_sectors
  mov first_data_sector, eax

  ; find the first root directory sector.
  mov eax, root_dir_sectors
  mov ebx, first_data_sector
  sub ebx, eax 
  mov first_root_dir_sector, ebx

  mov ebx, first_root_dir_sector
  add rbx, ROOT_PARTITION_FIRST_SECTOR
  mov qword [ROOT_DIR_LBA], rbx
  mov rax, rbx

  restore root_dir_sectors
  restore first_data_sector
  restore first_root_dir_sector

  mov rsp, rbp
  pop rbp
  ret

; function for 'ls' command.
; args:
;   nothing
; returns:
;   nothing
list_files_in_root_directory:
  push rbp
  mov rbp, rsp

  sub rsp, 12

  temp_lba equ qword [rbp - 8]
  total_logical_blocks_to_read equ byte [rbp - 9]
  blocks_read equ byte [rbp - 10]
  entry_num equ byte [rbp - 11]
  counter equ byte [rbp - 12]

  mov rbx, qword [ROOT_DIR_LBA]
  mov temp_lba, rbx

  ; list the directory entries

  ; 8 blocks for 4 times = 512 dir entries data
  mov total_logical_blocks_to_read, 32
  mov blocks_read, 8
.outer_loop_start:
  mov al, total_logical_blocks_to_read
  cmp blocks_read, al
  ja .outer_loop_end

  mov rdi, temp_lba
  mov esi, 7    ; read 8 logical blocks = 4KiB of data
  call nvme_read
  mov r8, rax
  mov entry_num, 0

.inner_loop_start:
  cmp entry_num, 128    ; 128 entries could be read in single NVMe read
  je .inner_loop_end

  mov al, byte [r8]
  cmp al, 0x0       ; no more files/directories in this directory
  je .outer_loop_end

  cmp al, 0xe5      ; the entry is unused
  je .inner_loop_next

  lea rdi, [file_name_field_value]
  lea rsi, [r8 + DIR_ENTRY_STRUCT.file_name]
  mov edx, 11 ; dir entry's file name field is 11 bytes long
  call strncpy

  ; skip printing volume label entry name
  mov rdi, rax
  lea rsi, [VOLUME_LABEL]
  mov edx, 8  ; length of VOLUME_LABEL
  call strncmp
  cmp eax, 0
  je .inner_loop_next

  ; print file name
  push r8
  lea rax, [file_name_field_value]
  mov counter, 0
.print_loop_start:
  cmp counter, 8
  jae .print_loop_end

  cmp byte [rax], SPACE_CHARACTER
  je .print_loop_end

  lea rdi, [msg_file_name_char]
  xor esi, esi
  mov sil, byte [rax]
  push rax
  call printk
  pop rax

.print_loop_next:
  inc counter
  inc rax
  jmp .print_loop_start

.print_loop_end:
  ; now print the file extension
  lea rdi, [msg_period]
  call printk

  lea rax, [file_name_field_value]
  add rax, 8    ; file extension starts from byte #8 (0's based)
  mov counter, 0

.print_extension_loop_start:
  cmp counter, 3
  jae .print_extension_loop_end

  cmp byte [rax], SPACE_CHARACTER
  je .print_extension_loop_end

  lea rdi, [msg_file_name_char]
  xor esi, esi
  mov sil, byte [rax]
  push rax
  call printk
  pop rax

.print_extension_loop_next:
  inc counter
  inc rax
  jmp .print_extension_loop_start

.print_extension_loop_end:
  lea rdi, [msg_newline_str]
  call printk
  pop r8

.inner_loop_next:
  inc entry_num
  add r8, sizeof.DIR_ENTRY_STRUCT
  jmp .inner_loop_start

.inner_loop_end:

.outer_loop_next:
  add temp_lba, 8 
  add blocks_read, 8
  jmp .outer_loop_start

.outer_loop_end:

  restore temp_lba
  restore total_logical_blocks_to_read
  restore blocks_read
  restore entry_num
  restore counter

  mov rsp, rbp
  pop rbp
  ret

; this function gets a file on the root directory. The file is
; specified in the @rdi register.
; args:
;   @rdi = pointer to file name stored in the keyboard input buffer
;          in the form of "README.MD" (without quotes) for example.
get_file_on_root_directory:
  push rbp
  mov rbp, rsp

  sub rsp, 12

  temp_lba equ qword [rbp - 8]
  total_logical_blocks_to_read equ byte [rbp - 9]
  blocks_read equ byte [rbp - 10]
  entry_num equ byte [rbp - 11]
  counter equ byte [rbp - 12]

  mov rax, qword [ROOT_DIR_LBA]
  mov temp_lba, rbx


  ; get the directory entries

  ; 8 blocks for 4 times = 512 dir entries data
  mov total_logical_blocks_to_read, 32
  mov blocks_read, 8
.outer_loop_start:
  mov al, total_logical_blocks_to_read
  cmp blocks_read, al
  ja .outer_loop_end

  mov rdi, temp_lba
  mov esi, 7    ; read 8 logical blocks = 4KiB of data
  call nvme_read
  mov r8, rax
  mov entry_num, 0

.inner_loop_start:
  cmp entry_num, 128    ; 128 entries could be read in single NVMe read
  je .inner_loop_end

  mov al, byte [r8]
  cmp al, 0x0       ; no more files/directories in this directory
  je .outer_loop_end

  cmp al, 0xe5      ; the entry is unused
  je .inner_loop_next

  lea rdi, [file_name_field_value]
  lea rsi, [r8 + DIR_ENTRY_STRUCT.file_name]
  mov edx, 11 ; dir entry's file name field is 11 bytes long
  call strncpy

  ; TODO
  ; compare file name
  push r8
  lea rax, [file_name_field_value]
  mov counter, 0
.print_loop_start:
  cmp counter, 8
  jae .print_loop_end

  cmp byte [rax], SPACE_CHARACTER
  je .print_loop_end

  lea rdi, [msg_file_name_char]
  xor esi, esi
  mov sil, byte [rax]
  push rax
  call printk
  pop rax

.print_loop_next:
  inc counter
  inc rax
  jmp .print_loop_start

.print_loop_end:
  ; now print the file extension
  lea rdi, [msg_period]
  call printk

  lea rax, [file_name_field_value]
  add rax, 8    ; file extension starts from byte #8 (0's based)
  mov counter, 0

.print_extension_loop_start:
  cmp counter, 3
  jae .print_extension_loop_end

  cmp byte [rax], SPACE_CHARACTER
  je .print_extension_loop_end

  lea rdi, [msg_file_name_char]
  xor esi, esi
  mov sil, byte [rax]
  push rax
  call printk
  pop rax

.print_extension_loop_next:
  inc counter
  inc rax
  jmp .print_extension_loop_start

.print_extension_loop_end:
  lea rdi, [msg_newline_str]
  call printk
  pop r8

.inner_loop_next:
  inc entry_num
  add r8, sizeof.DIR_ENTRY_STRUCT
  jmp .inner_loop_start

.inner_loop_end:

.outer_loop_next:
  add temp_lba, 8 
  add blocks_read, 8
  jmp .outer_loop_start

.outer_loop_end:

  restore temp_lba
  restore total_logical_blocks_to_read
  restore blocks_read
  restore entry_num
  restore counter

  mov rsp, rbp
  pop rbp
  ret


section '.data' data readable writeable

file_name_field_value rb 11

VOLUME_LABEL db "RAAMROOT"

; NOTE: IMPORTANT VARIABLE!!!
ROOT_DIR_LBA dq ?

msg_file_name_char db "{c}", 0
msg_period db ".", 0
msg_newline_str db 10, 0
