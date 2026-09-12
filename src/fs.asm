;
; note: using FAT12 formatted NVMe partition for root
;
; resources used:
;   - https://wiki.osdev.org/FAT
;   - Microsoft Extensible Firmware Initiative FAT32 File System Specification,
;     FAT: General Overview of On-Disk Format,
;     Version 1.03, December 6, 2000 by Microsoft Corporation
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

  ; also store it for later use
  mov dword [FIRST_DATA_SECTOR], eax

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
; returns:
;   @ax = first cluster number of the file if the file is found else -1
;   @ebx = file size in bytes if the file is found else -1
;
get_file_on_root_directory:
  push rbp
  mov rbp, rsp

  sub rsp, 36

  temp_lba equ qword [rbp - 8]
  total_logical_blocks_to_read equ byte [rbp - 9]
  blocks_read equ byte [rbp - 10]
  entry_num equ byte [rbp - 11]
  counter equ byte [rbp - 12]
  file_name_simplified_array equ [rbp - 26]
  i equ byte [rbp - 27]
  file_found equ byte [rbp - 28]
  file_name_argument equ qword [rbp - 36]

  mov file_name_argument, rdi    ; save it

  mov file_found, 0   ; not found yet

  mov rbx, qword [ROOT_DIR_LBA]
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

  push r8
  lea rdi, [file_name_field_value]
  lea rsi, [r8 + DIR_ENTRY_STRUCT.file_name]
  mov edx, 11 ; dir entry's file name field is 11 bytes long
  call strncpy

  ; first copy file name (without extension) to the array
  mov i, 0

  lea rax, [file_name_field_value]
  lea rbx, file_name_simplified_array
.loop_start:
  cmp i, 8  ; file name (without extension) has a limit of 8 characters in FAT12
  je .loop_end

  cmp byte [rax], SPACE_CHARACTER
  je .loop_end

  mov cl, byte [rax]
  mov byte [rbx], cl
  inc rax
  inc rbx
  inc i
  jmp .loop_start

.loop_end:

  ; place a period ('.') character after the file name
  mov byte [rbx], '.'

  ; now copy the extension to the array
  mov i, 0

  lea rax, [file_name_field_value]
  add rax, 8  ; the extension starts from byte #8 (0's based)
  inc rbx
.extension_loop_start:
  cmp i, 3  ; extension has a limit of 3 characters
  je .extension_loop_end

  cmp byte [rax], SPACE_CHARACTER
  je .extension_loop_end

  mov cl, byte [rax]
  mov byte [rbx], cl
  inc rax
  inc rbx
  inc i
  jmp .extension_loop_start

.extension_loop_end:

  ; terminate file name array with newline and null characters
  mov byte [rbx], NEWLINE_CHARACTER
  mov byte [rbx + 1], 0x0

  mov rdi, file_name_argument
  call strlen
  mov edx, eax
  mov rdi, file_name_argument
  lea rsi, file_name_simplified_array
  call strncmp
  cmp eax, 0
  jne .inner_loop_next

  ; found the file!
  pop r8
  mov ax, word [r8 + DIR_ENTRY_STRUCT.first_cluster_number]
  mov ebx, dword [r8 + DIR_ENTRY_STRUCT.file_size]
  mov file_found, 1   ; file is found
  jmp .outer_loop_end


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
  cmp file_found, 0
  jne .end

  ; file is not found
  mov ax, -1
  mov ebx, -1

.end:
  restore temp_lba
  restore total_logical_blocks_to_read
  restore blocks_read
  restore entry_num
  restore counter
  restore file_name_simplified_array
  restore i
  restore file_found
  restore file_name_argument

  mov rsp, rbp
  pop rbp
  ret

;
; print_file_contents
;
; this function prints the contents of a given file.
;
; args:
;  @edi = file cluster number
;  @esi = file length in bytes
;
; returns:
;  nothing
;
; note:
;  - The FAT maps the data region of the volume by cluster number. The
;    first data cluster is cluster 2.
;  - Given any valid data cluster number N, the sector number of the
;    first sector of that cluster (again relative to sector 0 of the
;    FAT volume) is computed as follows:
;    FirstSectorofCluster = ((N – 2) * BPB_SecPerClus) + FirstDataSector;
;
print_file_contents:
  push rdi
  push rsi
  mov edi, ROOT_PARTITION_FIRST_SECTOR
  xor esi, esi
  call nvme_read
  mov r8, rax

  xor eax, eax
  mov al, byte [r8 + FAT_BS.sectors_per_cluster]

  pop rsi
  pop rdi

  ; get the first sector of the file cluster
  sub edi, 2
  imul edi, eax
  add edi, dword [FIRST_DATA_SECTOR]

  ; now read the file contents (first 512 bytes max at the moment)
  add edi, ROOT_PARTITION_FIRST_SECTOR
  xor esi, esi
  call nvme_read

  mov rdi, rax
  call printk
  ret


section '.data' data readable writeable

file_name_field_value rb 11

VOLUME_LABEL db "RAAMROOT"

; NOTE: IMPORTANT VARIABLES!!!
ROOT_DIR_LBA dq ?

FIRST_DATA_SECTOR dd ?

msg_file_name_char db "{c}", 0
msg_period db ".", 0
msg_newline_str db 10, 0
