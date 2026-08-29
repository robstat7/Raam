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
;   @rsi, @rdx, @rcx, @r8, and @r9 =
;          arguments. Any additional arguments that do not fit in these
;          registers are passed on the stack in reverse order.
;
; returns:
;   nothing
;
printk:
  ; function prologue
  push rbp            ; save the old base pointer
  mov rbp, rsp

  sub rsp, 7

  state equ byte [rbp - 1]
  i equ dword [rbp - 5]
  current_char equ byte [rbp - 6]
  specifier_char equ byte [rbp - 7]

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
  mov rsi, rsi
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
