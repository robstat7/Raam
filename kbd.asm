PS2_DATA_PORT = 0x60

; keyboard input modes
INPUT_MODE_ON = 1
INPUT_MODE_OFF = 0


section '.text' code executable readable

; keyboard interrupt handler
; args:
;   @rdi = interrupt stack frame pointer
;
; returns:
;   nothing
keyboard_interrupt_handler:
  push rdi

  ; read the scan code byte from data port
  in al, PS2_DATA_PORT

  call convert_scan_code_byte

  cmp al, 0x0 ; key release event
  je .end

  ; if enter key is pressed, append the newline character to the
  ; buffer followed by a null character. Set buffer index to 0,
  ; turn off input mode and end interrupt.
  cmp al, NEWLINE_CHARACTER
  jne .continue
  lea rbx, [input_buffer]
  xor ecx, ecx
  mov cl, byte [input_buffer_index]
  add rcx, rbx

  mov byte [rcx], al
  mov byte [rcx + 1], NULL_CHARACTER
  mov byte [input_buffer_index], 0

  mov byte [input_mode], INPUT_MODE_OFF
  jmp .end


.continue:
  ; fill input buffer and print character on terminal screen
  lea rbx, [input_buffer]
  xor ecx, ecx
  mov cl, byte [input_buffer_index]
  add rcx, rbx

  mov byte [rcx], al
  inc byte [input_buffer_index]

  call tty_put_char

.end:
  mov edi, 1    ; set IRQ no. 1 for keyboard interrupt in dil register
  call pic_send_eoi
  pop rdi
  ret

; convert the scan code byte to ASCII character for a key pressed event.
; Note that we are using scan code set 1.
; (https://wiki.osdev.org/PS/2_Keyboard)
;
; args:
;   @al = scan code byte
;
; returns:
;   @al = ASCII character
convert_scan_code_byte:
.a:
  cmp al, 0x1e
  jne .b
  mov al, 'a'
  jmp .end

.b:
  cmp al, 0x30
  jne .c
  mov al, 'b'
  jmp .end

.c:
  cmp al, 0x2e
  jne .d
  mov al, 'c'
  jmp .end

.d:
  cmp al, 0x20
  jne .e
  mov al, 'd'
  jmp .end

.e:
  cmp al, 0x12
  jne .f
  mov al, 'e'
  jmp .end

.f:
  cmp al, 0x21
  jne .g
  mov al, 'f'
  jmp .end

.g:
  cmp al, 0x22
  jne .h
  mov al, 'g'
  jmp .end

.h:
  cmp al, 0x23
  jne .i
  mov al, 'h'
  jmp .end

.i:
  cmp al, 0x17
  jne .j
  mov al, 'i'
  jmp .end

.j:
  cmp al, 0x24
  jne .k
  mov al, 'j'
  jmp .end

.k:
  cmp al, 0x25
  jne .l
  mov al, 'k'
  jmp .end

.l:
  cmp al, 0x26
  jne .m
  mov al, 'l'
  jmp .end

.m:
  cmp al, 0x32
  jne .n
  mov al, 'm'
  jmp .end

.n:
  cmp al, 0x31
  jne .o
  mov al, 'n'
  jmp .end

.o:
  cmp al, 0x18
  jne .p
  mov al, 'o'
  jmp .end

.p:
  cmp al, 0x19
  jne .q
  mov al, 'p'
  jmp .end

.q:
  cmp al, 0x10
  jne .r
  mov al, 'q'
  jmp .end

.r:
  cmp al, 0x13
  jne .s
  mov al, 'r'
  jmp .end

.s:
  cmp al, 0x1f
  jne .t
  mov al, 's'
  jmp .end

.t:
  cmp al, 0x14
  jne .u
  mov al, 't'
  jmp .end

.u:
  cmp al, 0x16
  jne .v
  mov al, 'u'
  jmp .end

.v:
  cmp al, 0x2f
  jne .w
  mov al, 'v'
  jmp .end

.w:
  cmp al, 0x11
  jne .x
  mov al, 'w'
  jmp .end

.x:
  cmp al, 0x2d
  jne .y
  mov al, 'x'
  jmp .end

.y:
  cmp al, 0x15
  jne .z
  mov al, 'y'
  jmp .end

.z:
  cmp al, 0x2c
  jne .enter
  mov al, 'z'
  jmp .end

.enter:
  cmp al, 0x1c
  jne .default
  mov al, NEWLINE_CHARACTER
  jmp .end


.default:     ; key released event
  mov al, 0x0

.end:
  ret


section '.data' data readable writeable

input_buffer  rb  256
input_buffer_index db 0

input_mode  db ?
