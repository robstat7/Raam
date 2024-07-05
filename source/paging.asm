;
; Enable paging.
;
format elf64

public enable_paging

section '.text' executable

; the first parameter pml4e is in r8 register
enable_paging:
	; firstly, deactivate paging while in long mode
	mov rbx, cr0
	and ebx, 01111111111111111111111111111111b	; clear the PG bit (bit #31)
	mov cr0, rbx

	; flush the TLB
	mov rcx, 0x0
	mov cr3, rcx
	
	; enable PAE
	mov rdx, cr4
	or  edx, 100000b
	mov cr4, rdx

	; Set LME (long mode enable)
	mov ecx, 0xC0000080
	rdmsr
	or  eax, 100000000b
	wrmsr


	; point the CR3 register to the base address of pml4 table
	mov cr3, r8
	
	; enable paging
	or ebx, 10000000000000000000000000000000b
	mov cr0, rbx
	
	; ; now reload the segment registers (CS, DS, SS, etc.) with the appropriate segment selectors
	; mov ax, DATA_SEL
	; mov ds, ax
	; mov es, ax
	; mov fs, ax
	; mov gs, ax
	; 
	; ; Reload CS with a 64-bit code selector by performing a long jmp
	; 
	; jmp CODE_SEL:reloadCS
 
; [BITS 64]
; reloadCS:
; hlt   ; Done. Replace these lines with your own code
; jmp reloadCS

	ret
