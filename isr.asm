QWORD_SIZE = 8    ; in bytes


macro isr_err_stub vector
{
  isr_stub_#vector:
    cli
    push qword vector
    jmp isr_common
}

macro isr_no_err_stub vector
{
  isr_stub_#vector:
    cli
    push qword 0
    push qword vector
    jmp isr_common
}


section '.text' code executable readable

isr_common:
  push rax
  push rbx
  push rcx
  push rdx
  push rsi
  push rdi
  push rbp
  push rsp
  push r8
  push r9
  push r10
  push r11
  push r12
  push r13
  push r14
  push r15

  mov rdi, rsp
  call isr_handler

  pop r15
  pop r14
  pop r13
  pop r12
  pop r11
  pop r10
  pop r9
  pop r8
  pop rsp
  pop rbp
  pop rdi
  pop rsi
  pop rdx
  pop rcx
  pop rbx
  pop rax

  ; remove the vector number + error code
  add rsp, 16

  sti
  iretq

;
; isr_handler
;
; this function is a generic interrupt service routine handler. It
; handles the interrupts which have handlers defined else it prints the
; interrupt number, saved rip, and error code and halts the computer.
;
; args:
;   @rdi = interrupt stack frame's pointer
;
; returns:
;   nothing
;
isr_handler:
  ; get interrupt number first
  mov rax, rdi
  add rax, QWORD_SIZE * 16 ; interrupt number qword was pushed 16 times before
  mov rsi, qword [rax]

  push rsi  ; push it

  lea rax, [interrupt_handlers]  
  mov rbx, rsi
  imul ebx, QWORD_SIZE
  add rbx, rax

  mov rcx, qword [rbx]
  cmp rcx, 0
  je .no_handler

  call rcx  ; call the interrupt handler with argument in rdi

  pop rsi
  jmp .exit

.no_handler:
  ; print interrupt received message with interrupt number
  pop rsi
  push rdi
  lea rdi, [msg_interrupt]
  call printk
  pop rdi

  ; print saved rip
  mov rax, rdi
  add rax, QWORD_SIZE * 18
  mov rsi, qword [rax]
  push rdi
  lea rdi, [msg_saved_rip]
  call printk
  pop rdi

  ; also print error code
  mov rax, rdi
  add rax, QWORD_SIZE * 17
  mov rsi, qword [rax]
  push rdi
  lea rdi, [msg_error_code]
  call printk
  pop rdi
  jmp .halt

.exit:
  ret

.halt:
  hlt       ; computer halts here


isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31
; IRQs
isr_no_err_stub 32  ; IRQ0 timer
isr_no_err_stub 33  ; IRQ1 keyboard


section '.data' data readable writeable

isr_stub_table:
rept 34 counter:0
{
    dq isr_stub_#counter
}

interrupt_handlers  rb  QWORD_SIZE * NUM_IDT_ENTRIES

msg_interrupt db "Interrupt no. {p} received!", 10, 0
msg_saved_rip db "Saved rip = {p}!", 10, 0
msg_error_code db "Error code = {p}!", 10, 0
