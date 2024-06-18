;
; load gdt and idt
;
format elf64

public load_gdt
public load_idt

section '.text' executable

KCODE_SEG		equ 0x0008
KDATA_SEG		equ 0x0010

load_gdt:

	lgdt [rdi]
	
	mov rax, KDATA_SEG
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	
	mov rsi, rsp
	mov rcx, .flush
	
	push rax		; set DATA_SEG for iretq
	push rsi		; set rsp for iretq
	pushfq			; set eflag for iretq
	push QWORD KCODE_SEG	; set CODE_SEG for iretq
	push rcx		; far jump address for iretq
	iretq

.flush:
	ret

load_idt:
	lidt [rdi]
	ret
