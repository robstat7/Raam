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

  ; declare local variables
  sub rsp, 1  ; state
  sub rsp, 4  ; i
  sub rsp, 1  ; current_char 
  sub rsp, 1  ; specifier_char


  mov byte [rbp - 1], NORMAL
  mov dword [rbp - 5], 0

.loop_start:
  mov ebx, dword [rbp - 5]
  add rbx, rdi

  mov al, byte [rbx]
  mov byte [rbp - 6], al

  cmp byte [rbp - 6], NULL_CHARACTER   ; is string terminated?
  je .exit

  push rdi

  cmp byte [rbp - 1], FORMAT_SPECIFIER ; should we deal with a format specifier?
  jne .ahead

  cmp byte [rbp - 6], '}'
  jne .store_specifier_char
  mov dil, byte [rbp - 7]
  mov rsi, rsi
  call print_arg
  mov byte [rbp - 1], NORMAL
  jmp .end

.store_specifier_char:
  mov al, byte [rbp - 6]
  mov byte [rbp - 7], al
  jmp .end

.ahead:
  cmp byte [rbp - 6], '{'
  jne .print_char

  mov byte [rbp - 1], FORMAT_SPECIFIER
  jmp .end


.print_char:
  mov al, byte [rbp - 6]
  call tty_put_char

.end:
  pop rdi

  inc dword [rbp - 5]
  jmp .loop_start

.exit:
  ; function epilogue
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
