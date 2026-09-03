include 'string.asm'

; states to use
NORMAL = 0
FORMAT_SPECIFIER = 1

NULL_CHARACTER = 0x0


section '.text' code executable readable

;
; printk
;
; this function prints the passed format string onto the terminal
; screen. In C, it can be called as follows:
;
;		printk("Raam Raam sa");
;
; And with a format specifier as follows:
;
;   printk("saved rip = {p}!", (void *) saved_rip);
;
;
; args:
;   @rdi = address of the format string defined as a byte string
;   @rsi, @rdx, and @rcx =
;          values that replaces the format specifiers
;          in the format string.
;
; returns:
;   nothing
;
printk:
  ; function prologue
  push rbp            ; save the old base pointer
  mov rbp, rsp

  sub rsp, 32

  state equ byte [rbp - 1]
  i equ dword [rbp - 5]
  current_char equ byte [rbp - 6]
  specifier_char equ byte [rbp - 7]
  specifier_num equ byte [rbp - 8]
  arg2 equ qword [rbp - 16]
  arg3 equ qword [rbp - 24]
  arg4 equ qword [rbp - 32]

  ; save function arguments
  mov arg2, rsi
  mov arg3, rdx
  mov arg4, rcx

  mov specifier_num, 0

  mov state, NORMAL
  mov i, 0

.loop_start:
  mov ebx, i
  add rbx, rdi

  mov al, byte [rbx]
  mov current_char, al

  cmp current_char, NULL_CHARACTER   ; is string terminated?
  je .exit

  push rdi

  cmp state, FORMAT_SPECIFIER ; should we deal with a format specifier?
  jne .ahead

  cmp current_char, '}'
  jne .store_specifier_char
  mov dil, specifier_char
  cmp specifier_num, 1
  jne .next1
  mov rsi, arg2
  jmp .print_argument

.next1:
  cmp specifier_num, 2
  jne .next2
  mov rsi, arg3
  jmp .print_argument

.next2:
  mov rsi, arg4

.print_argument:
  call print_arg
  mov state, NORMAL
  jmp .end

.store_specifier_char:
  mov al, current_char
  mov specifier_char, al
  jmp .end

.ahead:
  cmp current_char, '{'
  jne .print_char

  mov state, FORMAT_SPECIFIER
  inc specifier_num
  jmp .end


.print_char:
  mov al, current_char
  call tty_put_char

.end:
  pop rdi

  inc i
  jmp .loop_start

.exit:
  ; function epilogue
  restore state
  restore i
  restore current_char
  restore specifier_char
  restore specifier_num
  restore arg2
  restore arg3
  restore arg4

  mov rsp, rbp    ; deallocate the local variable space
  pop rbp         ; restore the caller's base pointer
  ret


;
; print_arg
;
; this function prints the argument for the given specifier character.
;
; params:
;   @dil = single specifier character
;   @rsi = the argument
;
; returns:
;   nothing
;
print_arg:
  push rbp
  mov rbp, rsp

  sub rsp, 100  ; str array - randomly choosing a larger array


  cmp dil, 'p'  ; hex number
  jne .next

  mov rdi, rsi
  lea rsi, [rbp - 100]
  call integer_to_hex_string

  lea rdi, [rbp - 100]
  call printk
  jmp .exit

.next:
  cmp dil , 'c' ; a single character
  jne .exit

  lea rdi, [rbp - 100]
  mov byte [rdi], sil
  mov byte [rdi + 1], NULL_CHARACTER
  call printk

.exit:
  mov rsp, rbp
  pop rbp
  ret
