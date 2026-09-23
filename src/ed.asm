;
; ed - line-oriented text editor
;
section '.text' code executable readable

ed_start:

.init:
  mov word [buffer_index], 0

  lea rdi, [text_buffer]
  mov byte [rdi], NULL_CHARACTER

.write_ed_command_start:
  mov byte [input_mode], INPUT_MODE_ON

.loop_write_ed_command:
  cmp byte [input_mode], INPUT_MODE_OFF
  je .process_ed_cmd
  jmp .loop_write_ed_command

.process_ed_cmd:
  lea rdi, [input_buffer]
  lea rsi, [cmd_insert]
  mov edx, 2  
  call strncmp
  cmp eax, 0
  jne .cmd_print_buffer
  jmp .insert_text_start

.cmd_print_buffer:
  lea rdi, [input_buffer]
  lea rsi, [cmd_print_buffer]
  mov edx, 3
  call strncmp
  cmp eax, 0
  jne .cmd_exit_ed
  jmp .print_buffer

.cmd_exit_ed:
  lea rdi, [input_buffer]
  lea rsi, [cmd_quit]
  mov edx, 2
  call strncmp
  cmp eax, 0
  jne .cmd_write_to_file
  jmp .exit_ed

.cmd_write_to_file:
  lea rdi, [input_buffer]
  lea rsi, [cmd_write_to_file]
  mov edx, 2
  call strncmp
  cmp eax, 0
  jne .unrecognized_cmd

  lea rdi, [input_buffer + 2] ; file name arg
  lea rsi, [text_buffer]
  call write_buffer_to_file

  lea rdi, [text_buffer]
  call strlen
  lea rdi, [msg_bytes_written]
  mov esi, eax
  call printk
  jmp .write_ed_command_start

.print_buffer:
  lea rdi, [text_buffer]
  call printk
  jmp .write_ed_command_start

.insert_text_start:
  mov byte [input_mode], INPUT_MODE_ON

.loop_insert_text:
  cmp byte [input_mode], INPUT_MODE_OFF
  je .insert_text_in_buffer
  jmp .loop_insert_text

.insert_text_in_buffer:
  ; first check if end of insert command was issued
  lea rdi, [input_buffer]
  lea rsi, [cmd_insert_end]
  mov edx, 2
  call strncmp
  cmp eax, 0
  je .end_buffer_with_null_char

  lea rdi, [input_buffer]
  call strlen
  push rax
  mov edx, eax
  lea rdi, [text_buffer]
  xor eax, eax
  mov ax, word [buffer_index]
  add rdi, rax
  lea rsi, [input_buffer]
  call strncpy

  ; increase buffer_index
  pop rax
  add word [buffer_index], ax

  jmp .insert_text_start

.end_buffer_with_null_char:
  lea rdi, [text_buffer]
  xor eax, eax
  mov ax, word [buffer_index]
  add rdi, rax
  mov byte [rdi], NULL_CHARACTER
  jmp .write_ed_command_start

.unrecognized_cmd:
  lea rdi, [cmd_unknown]
  call printk
  jmp .write_ed_command_start

.exit_ed:
  ret


section '.data' data readable writeable

; ed commands
ED_COMMANDS:
cmd_insert db "i", 10, 0
cmd_print_buffer db ",p", 10, 0
cmd_write_to_file db "w ", 0
cmd_quit db "q", 10, 0

cmd_unknown db "?", 10, 0

cmd_insert_end db ".", 10, 0


text_buffer rb 512
buffer_index dw 0


msg_bytes_written db "{p}", 10, 0
