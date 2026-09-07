;
; NVMe over PCIe driver.
;
; resources used:
;   - NVM Express revision 1.3 specification
;   - https://wiki.osdev.org/NVMe
;   - https://wiki.osdev.org/PCI
;   - https://wiki.osdev.org/PCI_Express#Enhanced_Configuration_Mechanism
;
MAX_PCI_BUS_DEV	=	32

PCI_INVALID_VENDOR_ID = 0xffff

NVME_CLASS_CODE = 0x01    ; mass storage controller
NVME_SUBCLASS   = 0x08    ; non-volatile memory controller
NVME_PROG_IF    = 0x02    ; NVM Express


ADMIN_QUEUE_SIZE = 63     ; 0's based value


struc COMMON_CONFIG_SPACE_HEADER_STRUCT {
  .vendor_id            dw ?
  .dev_id               dw ?
  .cmd                  dw ?
  .status               dw ?
  .revision_id          db ?

  ; class code fields
  .prog_if              db ?
  .subclass             db ?
  .class_code           db ?

  .cache_line_size      db ?
  .latency_timer        db ?
  .header_type          db ?
  .bist                 db ?
}
struct COMMON_CONFIG_SPACE_HEADER_STRUCT

struc PCIE_DEV_INFO_STRUCT {
	.bus_number           dw ?
	.device_number        db ?
	.function_number      db ?
	.pci_func0_base_addr  dq ?
}
struct PCIE_DEV_INFO_STRUCT

struc HEADER_TYPE_0_TABLE_STRUCT {
  .h                    COMMON_CONFIG_SPACE_HEADER_STRUCT
  .bar0                 dd ?
  .bar1                 dd ?
  .bar2                 dd ?
  .bar3                 dd ?
  .bar4                 dd ?
  .bar5                 dd ?
  .cardbus_cis_ptr      dd ?
  .subsys_vendor_id     dw ?
  .subsys_id            dw ?
  .expansion_rom_base   dd ?
  .capabilities_ptr     db ?
  .reserved1            db ?
  .reserved2            dw ?
  .reserved3            dd ?
  .interrupt_line       db ?
  .interrupt_pin        db ?
  .min_grant            db ?
  .max_latency          db ?
}
struct HEADER_TYPE_0_TABLE_STRUCT

; kindly visit NVM Express Revision 1.3 spec's
; section 3.1 - Register Definition
struc REGISTER_MAP_STRUCT {
  .cap                  dq ?
  .vs                   dd ?
  .intms                dd ?
  .intmc                dd ?
  .cc                   dd ?
  .reserved1            dd ?
  .csts                 dd ?
  .nssr                 dd ?
  .aqa                  dd ?
  .asq                  dq ?
  .acq                  dq ?
  .cmbloc               dd ?
  .cmbsz                dd ?
  .bpinfo               dd ?
  .bprsel               dd ?
  .bpmbl                dq ?
  .reserved2            db 3760 dup (?)
  .reserved3            db 256 dup (?)
  .sq0tdbl              dd ?
  .cq0hdbl              dd ?
  .sq1tdbl              dd ?
}
struct REGISTER_MAP_STRUCT

struc SUBMISSION_QUEUE_COMMANDS_STRUCT {
  .cdw0                 dd ?
  .cdw1                 dd ?
  .cdw2                 dd ?
  .cdw3                 dd ?
  .cdw4_5               dq ?
  .cdw6_7               dq ?
  .cdw8_9               dq ?
  .cdw10                dd ?
  .cdw11                dd ?
  .cdw12                dd ?
  .cdw13                dd ?
  .cdw14                dd ?
  .cdw15                dd ?
}
struct SUBMISSION_QUEUE_COMMANDS_STRUCT


section '.text' code executable readable

;
; nvme_controller_init
;
; this function initializes the NVMe controller.
;
; args:
;   @rdi = 64-bit NVMe base address
;   @rsi = NVMe queues buffer pointer (buffer is zeroed)
;
; returns:
;   @eax = 0 on success else -1.
;
nvme_controller_init:
  ; store the controller register map base address first.
  mov qword [controller_register_map_base], rdi

  ; now set 4 KiB aligned address for NVMe queues buffer.
  mov rdi, rsi
  call align_address_to_4kib_boundary
  mov qword [nvme_queues_free_region], rax

  call reset_controller
  cmp eax, 0
  je .next

  lea rdi, [msg_nvme_fatal_error]
  call printk
  mov eax, -1
  jmp .end

.next:
  lea rdi, [msg_nvme_reset_completed]
  call printk

  call configure_admin_queues

  call enable_controller
  cmp eax, 0
  je .next2

  lea rdi, [msg_nvme_fatal_error]
  call printk
  mov eax, -1
  jmp .end

.next2:
  lea rdi, [msg_nvme_enable_completed]
  call printk

  call create_first_io_completion_queue
  call create_first_io_submission_queue

  mov eax, 0

.end:
  ret

create_first_io_completion_queue:
  push rbp
  mov rbp, rsp

  sub rsp, 16

  cdw0 equ dword [rbp - 4]
  cdw1 equ dword [rbp - 8]
  cdw10 equ dword [rbp - 12]
  cdw11 equ dword [rbp - 16]

  ; first we need to set the CC.IOCQES field before creating the I/O
  ; completion queue. As our controller supports only one I/O Command
  ; Set i.e. the NVM Command Set, both the required and the maximum
  ; values for CQES are 4 (2^4 = 16 bytes). We will use this value.
  mov rax, qword [controller_register_map_base]
  mov ebx, dword [rax + REGISTER_MAP_STRUCT.cc]
  or ebx, 0x400000
  mov dword [rax + REGISTER_MAP_STRUCT.cc], ebx

  ; CDW0 CID 0, PRP used (bits 15:14 clear), FUSE normal (bits 9:8 clear),
  ; command create io completion queue (0x5)
  mov cdw0, 0x5

  mov cdw1, 0x0         ; CDW1 ignored
  mov cdw10, 0xf0001    ; queue size = 15 commands (0's based), qid = 1
  mov cdw11, 0x1        ; interrupts disabled, physically contiguous (1<<0)

  mov rbx, qword [nvme_queues_free_region]
  mov qword [nvme_iocqb], rbx
  add rbx, 4096
  mov qword [nvme_queues_free_region], rbx

  mov edi, cdw0
  mov esi, cdw1
  mov rdx, qword [nvme_iocqb]
  mov ecx, cdw10
  mov r8d, cdw11
  call send_admin_command

  restore cdw0
  restore cdw1
  restore cdw10
  restore cdw11

  mov rsp, rbp
  pop rbp
  ret

send_admin_command:
  push rbp
  mov rbp, rsp

  sub rsp, 14

  admin_sq_tail_dbl_val equ byte [rbp - 1]
  old_admin_sq_tail_dbl_val equ byte [rbp - 2]
  offset equ dword [rbp - 6]
  asqb_ptr equ qword [rbp - 14]

  mov al, byte [admin_sq_tail_doorbell]
  mov admin_sq_tail_dbl_val, al
  mov old_admin_sq_tail_dbl_val, al

  inc admin_sq_tail_dbl_val

  cmp admin_sq_tail_dbl_val, 64
  je .wrap
  jmp .next

.wrap:
  mov admin_sq_tail_dbl_val, 0        ; wrap after 64 commands

.next:
  ; calculate the offset into the submission ring
  xor eax, eax
  mov al, old_admin_sq_tail_dbl_val
  imul eax, 64
  mov offset, eax

  ; find the address in the submission ring to build the command
  mov rax, qword [nvme_asqb]
  xor ebx, ebx
  mov ebx, offset
  add rax, rbx
  mov asqb_ptr, rax

  mov r9d, r8d
  mov r8d, ecx
  mov rcx, rdx
  mov edx, esi
  mov esi, edi
  mov rdi, asqb_ptr
  call build_command_structure

  ; now send the admin command by updating the admin submission queue tail
  ; doorbell register.
  ; store the new tail doorbell value first.
  xor eax, eax
  mov al, admin_sq_tail_dbl_val
  mov byte [admin_sq_tail_doorbell], al

  ; ring the doorbell by writing the newly incremented value to it.
  mov rbx, qword [controller_register_map_base]
  mov dword [rbx + REGISTER_MAP_STRUCT.sq0tdbl], eax

  xor eax, eax
  mov al, old_admin_sq_tail_dbl_val
  mov edi, eax
  call check_admin_completion_queue

  restore admin_sq_tail_dbl_val
  restore old_admin_sq_tail_dbl_val
  restore offset
  restore asqb_ptr

  mov rsp, rbp
  pop rbp
  ret

check_admin_completion_queue:
  imul edi, 16    ; each entry is 16 bytes
  add edi, 12     ; add 12 for double word 3

  mov rax, qword [nvme_acqb]
  add rax, rdi

.loop_start:
  mov ebx, dword [rax]
  cmp ebx, 0
  je .loop_start

  xor ebx, ebx
  mov bx, word [rax + 2]

  push rax
  lea rdi, [nvme_debug_msg]
  mov esi, ebx
  call printk

  pop rax
  mov dword [rax], 0  ; overwrite the old entry
  ret

build_command_structure:
  mov dword [rdi + SUBMISSION_QUEUE_COMMANDS_STRUCT.cdw0], esi
  mov dword [rdi + SUBMISSION_QUEUE_COMMANDS_STRUCT.cdw1], edx
  mov dword [rdi + SUBMISSION_QUEUE_COMMANDS_STRUCT.cdw2], 0
  mov dword [rdi + SUBMISSION_QUEUE_COMMANDS_STRUCT.cdw3], 0
  mov qword [rdi + SUBMISSION_QUEUE_COMMANDS_STRUCT.cdw4_5], 0
  mov qword [rdi + SUBMISSION_QUEUE_COMMANDS_STRUCT.cdw6_7], rcx
  mov qword [rdi + SUBMISSION_QUEUE_COMMANDS_STRUCT.cdw8_9], 0
  mov dword [rdi + SUBMISSION_QUEUE_COMMANDS_STRUCT.cdw10], r8d
  mov dword [rdi + SUBMISSION_QUEUE_COMMANDS_STRUCT.cdw11], r9d
  mov dword [rdi + SUBMISSION_QUEUE_COMMANDS_STRUCT.cdw12], 0
  mov dword [rdi + SUBMISSION_QUEUE_COMMANDS_STRUCT.cdw13], 0
  mov dword [rdi + SUBMISSION_QUEUE_COMMANDS_STRUCT.cdw14], 0
  mov dword [rdi + SUBMISSION_QUEUE_COMMANDS_STRUCT.cdw15], 0
  ret

create_first_io_submission_queue:
  push rbp
  mov rbp, rsp

  sub rsp, 16

  cdw0 equ dword [rbp - 4]
  cdw1 equ dword [rbp - 8]
  cdw10 equ dword [rbp - 12]
  cdw11 equ dword [rbp - 16]

  ; first we need to set the CC.IOSQES field before creating the I/O
  ; submission queue. As our controller supports only one I/O Command
  ; Set i.e. the NVM Command Set, both the required and the maximum
  ; values for SQES are 6 (2^6 = 64 bytes). We will use this value.
  mov rax, qword [controller_register_map_base]
  mov ebx, dword [rax + REGISTER_MAP_STRUCT.cc]
  or ebx, 0x60000
  mov dword [rax + REGISTER_MAP_STRUCT.cc], ebx

  ; CDW0 CID 0, PRP used (bits 15:14 clear), FUSE normal (bits 9:8 clear),
  ; command create io submission queue (0x1)
  mov cdw0, 0x1

  mov cdw1, 0x0         ; CDW1 ignored
  mov cdw10, 0x3f0001   ; queue size = 63 commands (0's based), qid = 1
  mov cdw11, 0x10001    ; cqid = 1, physically contiguous (1<<0)

  mov rbx, qword [nvme_queues_free_region]
  mov qword [nvme_iosqb], rbx
  add rbx, 4096
  mov qword [nvme_queues_free_region], rbx

  mov edi, cdw0
  mov esi, cdw1
  mov rdx, qword [nvme_iosqb]
  mov ecx, cdw10
  mov r8d, cdw11
  call send_admin_command

  restore cdw0
  restore cdw1
  restore cdw10
  restore cdw11

  mov rsp, rbp
  pop rbp
  ret

;
; configure_admin_queues
;
; this function configures the admin submission and the admin completion
; queues by setting up the admin queue attributes (AQA) first. The
; attributes include the admin completion queue size (ACQS) and the
; admin submission queue size (ASQS). Both are set to 63 commands or
; entries, which is a 0's based value (i.e. 64 commands in total). Then
; it sets up the admin submission queue's and the admin completion
; queue's base addresses by setting up the admin submission queue (ASQ)
; and the admin completion queue (ACQ) registers of the controller.
;
; args:
;   nothing
;
; returns:
;   nothing
;
configure_admin_queues:
  mov rax, qword [controller_register_map_base]

  ; combine ACQS (bits 27:16) and ASQS (bits 11:0) into the
  ; AQA register value.
  mov ebx, ADMIN_QUEUE_SIZE
  shl ebx, 16
  mov ecx, ADMIN_QUEUE_SIZE
  or ebx, ecx
  mov dword [rax + REGISTER_MAP_STRUCT.aqa], ebx

  ; set ASQ and ACQ registers in the controller (addresses must be 4KiB aligned)
  mov rbx, qword [nvme_queues_free_region]
  mov qword [rax + REGISTER_MAP_STRUCT.asq], rbx
  mov qword [nvme_asqb], rbx
  add rbx, 4096
  mov qword [rax + REGISTER_MAP_STRUCT.acq], rbx
  mov qword [nvme_acqb], rbx
  add rbx, 4096
  mov qword [nvme_queues_free_region], rbx
  ret

;
; enable_controller
;
; this function enables the controller by setting the
; CC.EN bit (bit #0). It then waits for the controller to indicate that
; the previous enable is complete by waiting for the CSTS.RDY bit
; (bit #0) to be set.
;
; args:
;   nothing
;
; returns:
;   @eax = 0 on success else -1.
;

enable_controller:
  mov rax, qword [controller_register_map_base]
  mov ebx, dword [rax + REGISTER_MAP_STRUCT.cc]
  mov ecx, 0x1
  or ebx, ecx
  mov dword [rax + REGISTER_MAP_STRUCT.cc], ebx

  ; poll until CSTS.RDY bit (bit #0) is set, or return -1 if
  ; CSTS.CFS bit (bit #1) is set.
.loop_start:
  mov ebx, 0x1
  mov ecx, dword [rax + REGISTER_MAP_STRUCT.csts]
  and ecx, ebx
  jnz .loop_end

  ; check if CSTS.CFS bit (bit #1) is set (fatal error)
  mov ebx, 0x2
  mov ecx, dword [rax + REGISTER_MAP_STRUCT.csts]
  and ecx, ebx
  jz .loop_next

  mov eax, -1
  jmp .end

.loop_next:
  jmp .loop_start

.loop_end:
  mov eax, 0

.end:
  ret

;
; reset_controller
;
; this function resets the controller by clearing the
; CC.EN bit (bit #0). It then waits for the controller to indicate that
; the previous reset is completed by waiting for the CSTS.RDY bit
; (bit #0) to become 0.
;
; args:
;   nothing
;
; returns:
;   @eax = 0 on success else -1.
;
reset_controller:
  mov rax, qword [controller_register_map_base]

  mov ebx, dword [rax + REGISTER_MAP_STRUCT.cc]
  mov ecx, 0x1
  not ecx
  and ebx, ecx
  mov dword [rax + REGISTER_MAP_STRUCT.cc], ebx

  ; poll until CSTS.RDY bit (bit #0) becomes 0, or return -1 if
  ; CSTS.CFS bit (bit #1) is set.
.loop_start:
  mov ebx, 0x1
  mov ecx, dword [rax + REGISTER_MAP_STRUCT.csts]
  and ecx, ebx
  jz .loop_end

  ; check if CSTS.CFS bit (bit #1) is set (fatal error)
  mov ebx, 0x2
  mov ecx, dword [rax + REGISTER_MAP_STRUCT.csts]
  and ecx, ebx
  jz .loop_next

  mov eax, -1
  jmp .end

.loop_next:
  jmp .loop_start

.loop_end:
  mov eax, 0

.end:
  ret

;
; get_nvme_base_address
;
; this function gets the NVMe base address that we will use to
; initialize the controller. We clear the lowest 4 bits of the
; base address as they are not part of the address, instead, they serve
; other purposes.
;
; args:
;   @rdi = struct PCIE_DEV_INFO_STRUCT's variable
;          nvme_controller_info's address
;
; returns:
;   @rax = the 64-bit NVMe base address
;
;
; note - resources used:
;    - https://wiki.osdev.org/PCI
;
get_nvme_base_address:
  mov rax, qword [rdi + PCIE_DEV_INFO_STRUCT.pci_func0_base_addr]

  ; our header type from the "Common Header Fields" is 0x0 that means it
  ; is a general device. We will thus use the "header type 0x0 table"
  ; for reading the Base Address #0 register.
  mov ebx, dword [rax + HEADER_TYPE_0_TABLE_STRUCT.bar0]

  ; the "type" (bits 2-1) from the memory space BAR layout above is 0x0.
  ; It suggests that the base register is 32-bit wide.
  ; Now we will clear the lowest 4 bits.
  mov rcx, 0xf
  not rcx
  and rbx, rcx

  mov rax, rbx
  ret

;
; check_function_number_0
;
; this function checks the device's function number 0 on the given PCIe
; bus to find the NVMe controller.
;
; args:
;   @edi = the bus number (0-255)
;   @esi = the device number on the specified bus (0-31)
;   @rdx = PCIe ECAM base address
;
; returns:
;   @eax = integer 0 if the controller is found else -1.
;
; notes:
;   - this function assumes the use of a PCIe ECAM (Enhanced
;     Configuration Access Mechanism) to access PCI configuration
;     space.
;   - the PCI configuration space for each function is 4096 bytes
;     in size.
;   - The ECAM layout formula can be found at:
;     https://wiki.osdev.org/PCI_Express#Enhanced_Configuration_Mechanism
;   - the Class Code, Subclass, and Prog IF register values are used
;     to identify the device type, the device function, and the
;     device register-level programming interface respectively.
;
check_function_number_0:
  push rbp
  mov rbp, rsp

  sub rsp, 4

  func equ dword [rbp - 4]

  mov func, 0     ; our function number is 0

  ; first get the physical MMIO address of the PCI configuration space
  ; for the function number 0. We will compute this address using the
  ; ECAM layout formula.
  shl edi, 20
  shl esi, 15
  or edi, esi
  shl func, 12
  or edi, func
  add rdx, rdi

  ; check if a device is present
  cmp word [rdx + COMMON_CONFIG_SPACE_HEADER_STRUCT.vendor_id], PCI_INVALID_VENDOR_ID
  je .not_found

  ; check for nvme class, subclass, and programming interface
  cmp byte [rdx + COMMON_CONFIG_SPACE_HEADER_STRUCT.class_code], NVME_CLASS_CODE
  jne .not_found
  cmp byte [rdx + COMMON_CONFIG_SPACE_HEADER_STRUCT.subclass], NVME_SUBCLASS
  jne .not_found
  cmp byte [rdx + COMMON_CONFIG_SPACE_HEADER_STRUCT.prog_if], NVME_PROG_IF
  jne .not_found

  ; found the controller.
  ; save the physical address for this function's PCI configuration space
  lea rax, [nvme_controller_info]
  mov qword [rax + PCIE_DEV_INFO_STRUCT.pci_func0_base_addr], rdx

  mov eax, 0
  jmp .end

.not_found:
  mov eax, -1

.end:
  restore func
  mov rsp, rbp
  pop rbp
  ret

;
; find_nvme_controller
;
; this function finds the NVMe controller on all the PCIe buses.
;
; args:
;   @rdi = struct pcie_ecam variable pointer
;
; returns:
;   @eax = an integer 0 if the controller is found else -1.
;
; note:
;   - only the function number 0 is probed because
;     most of the NVMe controllers are the single-function device.
;
find_nvme_controller:
  push rbp
  mov rbp, rsp

  sub rsp, 4

  bus equ word [rbp - 2]
  dev equ byte [rbp - 3]
  found equ byte [rbp - 4]

  mov found, -1    ; controller is not found yet

  xor eax, eax
  mov al, byte [rdi + PCIE_ECAM_STRUCT.start_bus_num]
  mov bus, ax

.loop_start_pcie_bus:
  xor eax, eax
  mov al, byte [rdi + PCIE_ECAM_STRUCT.end_bus_num]
  cmp bus, ax
  ja .loop_end_pcie_bus

  mov dev, 0

.loop_start_pcie_dev:
  cmp dev, MAX_PCI_BUS_DEV
  jae .loop_end_pcie_dev

  mov rdx, qword [rdi + PCIE_ECAM_STRUCT.base]
  push rdi
  xor edi, edi
  mov di, bus
  xor esi, esi
  mov sil, dev
  call check_function_number_0
  pop rdi
  cmp eax, 0
  jne .loop_next_pcie_dev

  ; controller is found!
  ; save controller info
  lea rax, [nvme_controller_info]
  mov bx, bus
  mov word [rax + PCIE_DEV_INFO_STRUCT.bus_number], bx
  mov bl, dev
  mov byte [rax + PCIE_DEV_INFO_STRUCT.device_number], bl
  mov byte [rax + PCIE_DEV_INFO_STRUCT.function_number], 0

  lea rdi, [msg_nvme]
  xor esi, esi
  mov si, bus
  xor edx, edx
  mov dl, dev
  xor ecx, ecx
  call printk

  mov found, 0
  jmp .loop_end_pcie_bus

.loop_next_pcie_dev:
  inc dev
  jmp .loop_start_pcie_dev

.loop_end_pcie_dev:
.loop_next_pcie_bus:
  inc bus
  jmp .loop_start_pcie_bus

.loop_end_pcie_bus:
  xor eax, eax
  mov al, found

  restore bus
  restore dev
  restore found

  mov rsp, rbp
  pop rbp
  ret

;
; align_address_to_4kib_boundary
;
; this function aligns a given address to the next 4 KiB boundary.
;
; args:
;   @rdi = address
;
; returns:
;   @rax = 4 KiB aligned address
;
align_address_to_4kib_boundary:
  ; if the address is not aligned, the remainder tells us how far
  ; the address is from the previous 4KiB boundary.
  xor edx, edx
  mov rax, rdi
  mov ecx, 4096
  div rcx
  cmp rdx, 0
  je .end

  sub rcx, rdx
  add rdi, rcx

.end:
  mov rax, rdi
  ret


section '.data' data readable writeable

nvme_controller_info rb sizeof.PCIE_DEV_INFO_STRUCT


controller_register_map_base dq 0


msg_nvme db "Found NVMe controller! Bus number = {p}, Device number = {p}, Function number = {p}", 10, 0

msg_nvme_fatal_error db "nvme: error: the controller had a fatal error!", 10, 0
msg_nvme_reset_completed db "nvme: controller reset is completed!", 10, 0
msg_nvme_enable_completed db "nvme: controller is enabled!", 10, 0


; contains the next 4 KiB aligned address to be used for NVMe queues.
; the memory region is already zeroed.
nvme_queues_free_region dq 0


; admin queues' base addresses
nvme_asqb dq 0
nvme_acqb dq 0

; io queues' base addresses
nvme_iocqb dq 0
nvme_iosqb dq 0


admin_sq_tail_doorbell  db 0

nvme_debug_msg db "@acq: status field plus phase tag value = {p}", 10, 0
