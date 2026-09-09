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

  ; fill the input buffer and print the character onto the terminal screen
  lea rbx, [input_buffer]
  xor ecx, ecx
  mov cl, byte [input_buffer_index]
  add rcx, rbx

  mov byte [rcx], al
  inc byte [input_buffer_index]

  push rax
  lea rdi, [char_string]
  xor esi, esi
  mov sil, al
  call printk

  ; if enter key was pressed, append the null character to the
  ; buffer, set buffer index to 0, and turn off the input mode.
  pop rax
  cmp al, NEWLINE_CHARACTER
  jne .end

  lea rbx, [input_buffer]
  xor ecx, ecx
  mov cl, byte [input_buffer_index]
  add rcx, rbx

  mov byte [rcx], NULL_CHARACTER
  mov byte [input_buffer_index], 0

  mov byte [input_mode], INPUT_MODE_OFF

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
.left_shift_pressed:
  cmp al, 0x2a
  jne .left_shift_released
  cmp byte [is_left_shift_pressed], 1
  je .skip
  mov byte [is_left_shift_pressed], 1   ; is_left_shift_pressed is true
.skip:
  mov al, 0x0
  jmp .end

.left_shift_released:
  cmp al, 0xaa
  jne .a
  mov byte [is_left_shift_pressed], 0
  mov al, 0x0
  jmp .end

.a:
  cmp al, 0x1e
  jne .b
  cmp byte [is_left_shift_pressed], 1
  jne .use_lower_a
  mov al, 'A'
  jmp .end
.use_lower_a:
  mov al, 'a'
  jmp .end

.b:
  cmp al, 0x30
  jne .c
  cmp byte [is_left_shift_pressed], 1
  jne .use_lower_b
  mov al, 'B'
  jmp .end
.use_lower_b:
  mov al, 'b'
  jmp .end

.c:
  cmp al, 0x2e
  jne .d
  cmp byte [is_left_shift_pressed], 1
  jne .use_lower_c
  mov al, 'C'
  jmp .end
.use_lower_c:
  mov al, 'c'
  jmp .end

.d:
  cmp al, 0x20
  jne .e
  cmp byte [is_left_shift_pressed], 1
  jne .use_lower_d
  mov al, 'D'
  jmp .end
.use_lower_d:
  mov al, 'd'
  jmp .end

.e:
  cmp al, 0x12
  jne .f
  cmp byte [is_left_shift_pressed], 1
  jne .use_lower_e
  mov al, 'E'
  jmp .end
.use_lower_e:
  mov al, 'e'
  jmp .end

.f:
  cmp al, 0x21
  jne .g
  cmp byte [is_left_shift_pressed], 1
  jne .use_lower_f
  mov al, 'F'
  jmp .end
.use_lower_f:
  mov al, 'f'
  jmp .end

.g:
  cmp al, 0x22
  jne .h
  cmp byte [is_left_shift_pressed], 1
  jne .use_lower_g
  mov al, 'G'
  jmp .end
.use_lower_g:
  mov al, 'g'
  jmp .end

.h:
  cmp al, 0x23
  jne .i
  cmp byte [is_left_shift_pressed], 1
  jne .use_lower_h
  mov al, 'H'
  jmp .end
.use_lower_h:
  mov al, 'h'
  jmp .end

.i:
  cmp al, 0x17
  jne .j
  cmp byte [is_left_shift_pressed], 1
  jne .use_lower_i
  mov al, 'I'
  jmp .end
.use_lower_i:
  mov al, 'i'
  jmp .end

.j:
  cmp al, 0x24
  jne .k
  cmp byte [is_left_shift_pressed], 1
  jne .use_lower_j
  mov al, 'J'
  jmp .end
.use_lower_j:
  mov al, 'j'
  jmp .end

.k:
  cmp al, 0x25
  jne .l
  cmp byte [is_left_shift_pressed], 1
  jne .use_lower_k
  mov al, 'K'
  jmp .end
.use_lower_k:
  mov al, 'k'
  jmp .end

.l:
  cmp al, 0x26
  jne .m
  cmp byte [is_left_shift_pressed], 1
  jne .use_lower_l
  mov al, 'L'
  jmp .end
.use_lower_l:
  mov al, 'l'
  jmp .end

.m:
  cmp al, 0x32
  jne .n
  cmp byte [is_left_shift_pressed], 1
  jne .use_lower_m
  mov al, 'M'
  jmp .end
.use_lower_m:
  mov al, 'm'
  jmp .end

.n:
  cmp al, 0x31
  jne .o
  cmp byte [is_left_shift_pressed], 1
  jne .use_lower_n
  mov al, 'N'
  jmp .end
.use_lower_n:
  mov al, 'n'
  jmp .end

.o:
  cmp al, 0x18
  jne .p
  cmp byte [is_left_shift_pressed], 1
  jne .use_lower_o
  mov al, 'O'
  jmp .end
.use_lower_o:
  mov al, 'o'
  jmp .end

.p:
  cmp al, 0x19
  jne .q
  cmp byte [is_left_shift_pressed], 1
  jne .use_lower_p
  mov al, 'P'
  jmp .end
.use_lower_p:
  mov al, 'p'
  jmp .end

.q:
  cmp al, 0x10
  jne .r
  cmp byte [is_left_shift_pressed], 1
  jne .use_lower_q
  mov al, 'Q'
  jmp .end
.use_lower_q:
  mov al, 'q'
  jmp .end

.r:
  cmp al, 0x13
  jne .s
  cmp byte [is_left_shift_pressed], 1
  jne .use_lower_r
  mov al, 'R'
  jmp .end
.use_lower_r:
  mov al, 'r'
  jmp .end

.s:
  cmp al, 0x1f
  jne .t
  cmp byte [is_left_shift_pressed], 1
  jne .use_lower_s
  mov al, 'S'
  jmp .end
.use_lower_s:
  mov al, 's'
  jmp .end

.t:
  cmp al, 0x14
  jne .u
  cmp byte [is_left_shift_pressed], 1
  jne .use_lower_t
  mov al, 'T'
  jmp .end
.use_lower_t:
  mov al, 't'
  jmp .end

.u:
  cmp al, 0x16
  jne .v
  cmp byte [is_left_shift_pressed], 1
  jne .use_lower_u
  mov al, 'U'
  jmp .end
.use_lower_u:
  mov al, 'u'
  jmp .end

.v:
  cmp al, 0x2f
  jne .w
  cmp byte [is_left_shift_pressed], 1
  jne .use_lower_v
  mov al, 'V'
  jmp .end
.use_lower_v:
  mov al, 'v'
  jmp .end

.w:
  cmp al, 0x11
  jne .x
  cmp byte [is_left_shift_pressed], 1
  jne .use_lower_w
  mov al, 'W'
  jmp .end
.use_lower_w:
  mov al, 'w'
  jmp .end

.x:
  cmp al, 0x2d
  jne .y
  cmp byte [is_left_shift_pressed], 1
  jne .use_lower_x
  mov al, 'X'
  jmp .end
.use_lower_x:
  mov al, 'x'
  jmp .end

.y:
  cmp al, 0x15
  jne .z
  cmp byte [is_left_shift_pressed], 1
  jne .use_lower_y
  mov al, 'Y'
  jmp .end
.use_lower_y:
  mov al, 'y'
  jmp .end

.z:
  cmp al, 0x2c
  jne .enter
  cmp byte [is_left_shift_pressed], 1
  jne .use_lower_z
  mov al, 'Z'
  jmp .end
.use_lower_z:
  mov al, 'z'
  jmp .end

.enter:
  cmp al, 0x1c
  jne .space
  mov al, NEWLINE_CHARACTER
  jmp .end

.space:
  cmp al, 0x39
  jne .period
  mov al, SPACE_CHARACTER
  jmp .end

.period:
  cmp al, 0x34
  jne .default
  mov al, '.'
  jmp .end


.default:     ; key released event
  mov al, 0x0

.end:
  ret


section '.data' data readable writeable

input_buffer  rb  256
input_buffer_index db 0

input_mode  db ?

char_string db "{c}", 0


is_left_shift_pressed db 0
