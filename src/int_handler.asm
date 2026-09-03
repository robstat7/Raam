IRQ0_INT_NUM = 32   ; timer interrupt number
IRQ1_INT_NUM = 33   ; keyboard interrupt number

section '.text' code executable readable

; this function zeros the interrupt_handlers array declared in isr.asm.
array_interrupt_handlers_init:
  push rbp
  mov rbp, rsp

  ; zero the interrupt_handlers array

  sub rsp, 4  ; i variable

  lea rdi, [interrupt_handlers]
  mov dword [rbp - 4], 0

.loop_start:
  cmp dword [rbp - 4], NUM_IDT_ENTRIES
  je .exit

  mov eax, dword [rbp - 4]
  add rax, rdi

  mov qword [rax], 0

  inc dword [rbp - 4]
  jmp .loop_start

.exit:
  mov rsp, rbp
  pop rbp
  ret

; register an interrupt handler
; args:
;   @dil = interrupt number
;   @rsi = handler function
;
; returns:
;   nothing
register_interrupt_handler:
  lea rax, [interrupt_handlers]

  xor ebx, ebx
  mov bl, dil

  imul ebx, QWORD_SIZE
  add rbx, rax

  mov qword [rbx], rsi
  ret

; timer interrupt handler
; args:
;   @rdi = interrupt stack frame pointer
;
; returns:
;   nothing
timer_interrupt_handler:
  push rdi
  xor edi, edi    ; set IRQ no. 0 for timer interrupt in dil register
  call pic_send_eoi
  pop rdi
  ret
