;
; Raam Raam Ji _/\_ _/\_ _/\_
;
include 'tty_io.asm'

include 'printk.asm'

include 'gdt.asm'

include 'idt.asm'

include 'int_handler.asm'

include 'pic.asm'

include 'pcie.asm'

include 'kbd.asm'

include 'shell.asm'


struc XSDP_STRUCT {
  .signature          db 8 dup (?)
  .checksum           db ?
  .oem_id             db 6 dup (?)
  .revision           db ?
  .rsdt_address       dd ?        ;  deprecated since version 2.0

  .length             dd ?
  .xsdt_address       dq ?
  .extended_checksum  db ?
  .reserved           db 3 dup (?)
}
struct XSDP_STRUCT

struc ACPI_SDT_HEADER {
  .signature          db 4 dup (?)
  .length             dd ?
  .revision           db ?
  .checksum           db ?
  .oem_id             db 6 dup (?)
  .oem_table_id       db 8 dup (?)
  .oem_revision       dd ?
  .creator_id         dd ?
  .creator_revision   dd ?
}
struct ACPI_SDT_HEADER

; note that the 'pointer_to_other_sdts' field as defined below should
; be aligned to a 4-byte boundary and not the default 8-byte alignment
; for a uint64_t. Source: XSDT - osdev wiki.
struc XSDT_STRUCT {
  .h                  ACPI_SDT_HEADER

  ; an array of 64-bit physical addresses that point to other
  ; system description tables
  align 4
  .pointer_to_other_sdts  dq ?
}
struct XSDT_STRUCT

struc MCFG_STRUCT {
  .h                  ACPI_SDT_HEADER

  .reserved           dq ?
  ; below is actually an array of type struct enhanced_config_base_struct
  .e                  ENHANCED_CONFIG_BASE_STRUCT
}
struct MCFG_STRUCT


section '.text' code executable readable

;
; kernel_init
;
; this function initializes our kernel.
;
; args (boot params):
;   @rdi = xsdp table pointer
;
; returns:
;   nothing
;
kernel_init:
  ; first disable interrupts
  cli

  push rdi

  call default_tty_init

  call gdt_init
  call idt_init

  call array_interrupt_handlers_init

  call pic_init

  ; register keyboard interrupt handler
  mov dil, IRQ1_INT_NUM
  lea rsi, [keyboard_interrupt_handler]
  call register_interrupt_handler

  ; register timer interrupt handler
  mov dil, IRQ0_INT_NUM
  lea rsi, [timer_interrupt_handler]
  call register_interrupt_handler

  ; get xsdt table pointer
  pop rdi
  mov rax, qword [rdi + XSDP_STRUCT.xsdt_address]
  mov qword [xsdt_pointer], rax

  ; get mcfg table pointer
  mov rdi, qword [xsdt_pointer]
  call get_mcfg_pointer
  cmp eax, 0
  jne .end

  ; get and store PCIe ECAM base address and the starting and the ending
  ; bus numbers
  mov rdi, qword [mcfg_pointer]
  call pcie_init

  ; enable interrupts now
  sti

  ; print welcome message :)
  lea rdi, [welcome_msg]
  call printk

  ; and finally run the shell
  call run_shell

.end:
  jmp $

;
; get_mcfg_pointer
;
; this function finds the MCFG table pointer using the XSDT table.
; The variable `total_entries` contains the total number of pointers to
; other System Description Tables (SDTs) within the XSDT table.
;
; note: the XSDT is the main SDT. However,
; there are many kinds of SDT. All the SDTs may be split into two parts.
; One (the header) which is common to all the SDTs and another (data)
; which is different for each table.
;
; args:
;   @rdi = xsdt table pointer
;
; returns:
;   @eax = 0 on success, -1 on failure
;
get_mcfg_pointer:
  push rbp
  mov rbp, rsp

  sub rsp, 20

  total_entries equ dword [rbp - 4]
  i equ dword [rbp - 8]
  desc_header equ qword [rbp - 16]
  str_mcfg equ dword [rbp - 20]

  mov eax, dword [rdi + XSDT_STRUCT.h.length]
  sub eax, sizeof.ACPI_SDT_HEADER
  xor edx, edx
  mov ecx, 8    ; 8-byte pointer size
  div rcx
  mov total_entries, eax

  mov i, 0

.loop_start:
  mov eax, total_entries
  cmp i, eax
  jae .loop_end

  mov rbx, rdi
  add rbx, XSDT_STRUCT.pointer_to_other_sdts
  mov ecx, i
  imul ecx, 8 ; 8-byte pointer size
  add rbx, rcx
  mov rdx, qword [rbx]
  mov desc_header, rdx    ; save xsdt->pointer_to_other_sdts[i]

  ; check MCFG table signature

  ; copy "MCFG" string to the variable in its little-endian hex value
  mov str_mcfg, 0x4746434d
  push rdi
  mov rbx, desc_header
  add rbx, ACPI_SDT_HEADER.signature
  mov rdi, rbx
  mov rsi, rbp
  sub rsi, 20   ; variable `str_mcfg` address
  mov edx, 4
  call strncmp
  pop rdi
  cmp eax, 0
  jne .loop_next

  mov rax, desc_header
  mov qword [mcfg_pointer], rax
  jmp .loop_end

.loop_next:
  inc i
  jmp .loop_start

.loop_end:
  mov rax, qword [mcfg_pointer]
  cmp rax, 0
  jne .success

  lea rdi, [error_msg_mcfg]
  call printk
  mov eax, -1
  jmp .end

.success:
  mov eax, 0

.end:
  restore total_entries
  restore i
  restore desc_header
  restore str_mcfg

  mov rsp, rbp
  pop rbp
  ret


section '.data' data readable writeable

welcome_msg db "_/\_ Raam Raam Ji _/\_", 10, 10, \
"Welcome to Raam x86-64 version 0.01!", 10, 10, 0

xsdt_pointer  dq 0
mcfg_pointer  dq 0

error_msg_mcfg db "error: could not find MCFG table!", 10, 0
