;
; Global Descriptor Table (GDT) initialization related functions.
;
NUM_GDT_ENTRIES = 3


section '.text' code executable readable

;
; gdt_init
;
; this function initializes the global descriptor table (GDT).
;
; notes:
;
;  1. In 64-bit mode, the Base and Limit values are ignored, each
; descriptor covers the entire linear address space regardless of what
; they are set to.
;
;  2. gdt descriptor table limit: we subtract 1 because the maximum
; value of limit (i.e. size) is 65535, while the GDT can be up to 65536
; bytes in length (8192 entries). Further, no GDT can have a limit of 0
; bytes.
;
gdt_init:
  call create_descriptors

  lea rax, [GDTR.limit]
  mov word [rax], NUM_GDT_ENTRIES * 8 - 1

  lea rax, [GDTR.address]
  lea rbx, [gdt_entries]
  mov qword [rax], rbx

  lgdt [GDTR]

  ; now reload the current selectors, since they are using cached information
  ; from the previous GDT.
  call reload_segments
  ret

reload_segments:
  ; reload cs register:
  ; 1. push the target 64-bit code segment selector onto the stack.
  push 0x08     ; kernel code segment descriptor offset

  ; 2. push the address of the label directly below onto the stack.
  mov rax, $$ + .reload_cs
  push rax

  ; 3. execute a 64-bit far return. 
  ; this pops rax into rip and 0x08 into CS simultaneously.
  retfq

; note: at the hardware level, a "data segment" is a region of memory
; tracked by the CPU's specific segment registers (ds, es, fs, gs, ss).
.reload_cs:
   ; CS is now successfully reloaded! Proceed to data segments.
  mov   ax, 0x10    ; kernel data segment descriptor offset
  mov   ds, ax
  mov   es, ax
  mov   fs, ax
  mov   gs, ax
  mov   ss, ax
  ret

create_descriptors:
  ; null descriptor, required to be here.
  lea rax, [gdt_entries.null_selector]
  mov qword [rax], 0

  ; kernel mode code segment descriptor.
  lea rax, [kernel_code]
  mov qword [rax], 0

  ; type of selector
  mov ebx, 1011b
  shl ebx, 8
  or qword [rax], rbx

  ; code segment descriptor
  mov ebx, 1
  shl ebx, 12
  or qword [rax], rbx

  ; DPL field = 0
  xor ebx, ebx
  shl ebx, 13
  or qword [rax], rbx

  ; present
  mov ebx, 1
  shl ebx, 15
  or qword [rax], rbx

  ; long-mode segment
  mov ebx, 1
  shl ebx, 21
  or qword [rax], rbx

  lea rcx, [gdt_entries.kernel_cs]
  shl qword [rax], 32
  mov rbx, qword [rax]

  mov qword [rcx], rbx

  ; kernel mode data segment descriptor.
  lea rax, [kernel_data]
  mov qword [rax], 0

  ; type of selector
  mov ebx, 11b
  shl ebx, 8
  or qword [rax], rbx

  ; data segment descriptor
  mov ebx, 1
  shl ebx, 12
  or qword [rax], rbx

  ; DPL field = 0
  xor ebx, ebx
  shl ebx, 13
  or qword [rax], rbx

  ; present
  mov ebx, 1
  shl ebx, 15
  or qword [rax], rbx

  ; long-mode code flag
  xor ebx, ebx
  shl ebx, 21
  or qword [rax], rbx

  lea rcx, [gdt_entries.kernel_ds]
  shl qword [rax], 32
  mov rbx, qword [rax]

  mov qword [rcx], rbx

  ret


section '.data' data readable writeable

gdt_entries:
  .null_selector  dq 0
  .kernel_cs      dq 0
  .kernel_ds      dq 0

kernel_code       dq 0
kernel_data       dq 0

GDTR:
  .limit          dw 0
  .address        dq 0
