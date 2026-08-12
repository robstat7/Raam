section '.text' code executable readable

run_shell:

.cmd_line:
  lea rdi, [cmd_input]
  call printk

  mov byte [input_mode], INPUT_MODE_ON

.loop_write_command:
  cmp byte [input_mode], INPUT_MODE_OFF
  je .process_cmd
  jmp .loop_write_command

.process_cmd:
  mov al, NEWLINE_CHARACTER
  call tty_put_char

.cmd_1:
  lea rdi, [input_buffer]
  lea rsi, [cmd_1]
  mov edx, 5
  call strncmp
  cmp eax, 0
  jne .cmd_2

  lea rdi, [cmd_1_res]
  call printk
  jmp .end

.cmd_2:
  lea rdi, [input_buffer]
  lea rsi, [cmd_2]
  mov edx, 6
  call strncmp
  cmp eax, 0
  jne .cmd_3

  call clear_screen
  jmp .end

.cmd_3:

  ; lea rdi, [input_buffer]
  ; call strlen

  ; mov esi, eax
  ; lea rdi, [msg_len]
  ; call printk

  ; lea rdi, [msg1]
  ; lea rsi, [msg2]
  ; mov edx, 5
  ; call strncmp
  ; mov esi, eax
  ; lea rdi, [msg_len]
  ; call printk

.end:
  jmp .cmd_line


section '.data' data readable writeable

cmd_input db "$ ", 0

SHELL_COMMANDS:
cmd_1 db "help", 10, 0
cmd_2 db "clear", 10, 0
cmd_3 db "echo", 10, 0
cmd_4 db "reboot", 10, 0

cmd_1_res db "Available commands:", 10, "help", 10, "clear", 10, "echo", 10, "reboot", 10, 0
