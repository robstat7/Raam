;
; NVMe over PCIe driver.
;
; resources used:
;   - NVM Express revision 1.3 specification
;   - https://wiki.osdev.org/NVMe
;
MAX_PCI_BUS_DEV	=	32

PCI_INVALID_VENDOR_ID = 0xffff

NVME_CLASS_CODE = 0x01    ; mass storage controller
NVME_SUBCLASS = 0x08      ; non-volatile memory controller
NVME_PROG_IF = 0x02       ; NVM Express


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
}
struct PCIE_DEV_INFO_STRUCT


section '.text' code executable readable

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

  mov eax, 0    ; found the controller
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

msg_nvme db "Found NVMe controller! Bus number = {p}, Device number = {p}, Function number = {p}", 10, 0
