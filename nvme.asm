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


section '.text' code executable readable

;
; nvme_controller_init
;
; this function initializes the NVMe controller.
;
; args:
;   @rdi = 64-bit NVMe base address
;
; returns:
;   @eax = 0 on success else -1.
;
nvme_controller_init:
  ; store the controller register map base address first
  mov qword [controller_register_map_base], rdi

  ; now reset the controller
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

  mov eax, 0
.end:
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

  ; TODO: set ASQ and ACQ registers
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

  ; poll until CSTS.RDY bit (bit #0) becomes 0, or return false if
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


section '.data' data readable writeable

nvme_controller_info rb sizeof.PCIE_DEV_INFO_STRUCT

controller_register_map_base dq 0

msg_nvme db "Found NVMe controller! Bus number = {p}, Device number = {p}, Function number = {p}", 10, 0

msg_nvme_fatal_error db "error: nvme: the controller had a fatal error!", 10, 0
msg_nvme_reset_completed db "nvme: controller reset is completed!", 10, 0
