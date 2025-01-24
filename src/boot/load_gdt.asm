;                                                                               
; load global descriptor table (GDT).
;                                                                               
[bits 64]

[GLOBAL load_gdt]
                                                                                
; the System V ABI passes the first argument via the RDI register.  
load_gdt:                                                                       
        lgdt [rdi] 
	call reload_segments
	ret

reload_segments:
   ; Reload CS register:
   push 0x08                 ; Push code segment to stack, 0x08 is a stand-in for your code segment
   lea rax, [rel .reload_CS] ; Load address of .reload_CS into RAX
   push rax                  ; Push this value to the stack
   retfq                     ; Perform a far return, RETFQ or LRETQ depending on syntax
.reload_CS:
   ; Reload data segment registers
   mov   ax, 0x10 ; 0x10 is a stand-in for your data segment
   mov   ds, ax
   mov   es, ax
   mov   fs, ax
   mov   gs, ax
   mov   ss, ax
   ret
