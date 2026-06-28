;
; Interrupt Descriptor Table (IDT) initialization related functions.
;
include 'isr.asm'


NUM_IDT_ENTRIES = 256

DESCRIPTOR_SIZE = 16       ; in bytes


GDT_OFFSET_KERNEL_CODE = 0x08


struc IDT_ENTRY {
	.isr_low                  dw ?
	.kernel_cs                dw ?
  .ist                      db ?
  .attributes               db ?
	.isr_mid                  dw ?
	.isr_high                 dd ?
	.reserved                 dd ?
}
struct IDT_ENTRY


section '.text' code executable readable

idt_init:
  lea rax, [IDTR.limit]
  mov word [rax], NUM_IDT_ENTRIES * DESCRIPTOR_SIZE - 1

  lea rax, [IDTR.base]
  lea rbx, [idt]
  mov qword [rax], rbx

  mov byte [vector], 0

.loop_start:
  cmp byte [vector], 32
  jae .loop_end
  xor eax, eax
  mov al, byte [vector]

  lea rbx, [isr_stub_table]
  xor edx, edx
  mov dl, byte [vector]
  imul edx, 8       ; each address is stub table is 8 bytes long
  add rbx, rdx
  mov rbx, [rbx]    ; rbx = isr_stub_table[vector]   (the actual ISR address)

  mov ah, 0x8e

  call idt_set_descriptor

  inc byte [vector]
  jmp .loop_start

.loop_end:
  lidt [IDTR]
  ret

;
; idt_set_descriptor
;
; a helper function to define the entries.
;
; params:
;   al = vector
;   rbx = isr
;   ah = flags
;
; returns:
;   nothing
;
idt_set_descriptor:
  lea rcx, [idt]
  xor edx, edx
  mov dl, al
  imul edx, DESCRIPTOR_SIZE
  add rcx, rdx      ; &idt[vector] in rcx

  mov rdx, rbx
  and rdx, 0xffff
  mov word [rcx + IDT_ENTRY.isr_low], dx

  mov word [rcx + IDT_ENTRY.kernel_cs], GDT_OFFSET_KERNEL_CODE

  mov byte [rcx + IDT_ENTRY.ist], 0

  mov byte [rcx + IDT_ENTRY.attributes], ah

  mov rdx, rbx
  shr rdx, 16
  and rdx, 0xffff
  mov word [rcx + IDT_ENTRY.isr_mid], dx

  mov rdx, rbx
  shr rdx, 32                             
  mov edx, edx
  mov dword [rcx + IDT_ENTRY.isr_high], edx

  mov dword [rcx + IDT_ENTRY.reserved], 0
  ret


section '.data' data readable writeable

; create an array of IDT entries; aligned for performance
align 0x10
idt               rb DESCRIPTOR_SIZE * NUM_IDT_ENTRIES

IDTR:
  .limit          dw 0
  .base           dq 0

vector            db ?
