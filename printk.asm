section '.text' code executable readable

;
; printk
;
; this function prints the passed format string onto the terminal
; screen. It can be called as follows:
;
;		printk("Raam Raam sa");
;
; args:
;   @rax = address of the format string defined as a byte string
;
; returns:
;   nothing
;
printk:
  mov dword [i], 0

.loop_start:
  xor ebx, ebx
  mov ebx, dword [i]
  add rbx, rax

  cmp byte [rbx], 0x00      ; is string terminated?
  je .exit

  push rax

  mov al, byte [rbx]
  call tty_put_char

  pop rax

  inc dword [i]
  jmp .loop_start

.exit:
  ret


section '.data' data readable writeable

i               dd ?
