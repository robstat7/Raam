section '.text' code executable readable

run_shell:

.cmd_line:
  lea rdi, [cmd_input]
  call printk

  mov byte [input_mode], INPUT_MODE_ON

.loop_write_command:
  cmp byte [input_mode], INPUT_MODE_OFF
  je .end
  jmp .loop_write_command

.end:
  mov al, NEWLINE_CHARACTER
  call tty_put_char

  lea rdi, [input_buffer]
  call printk

  jmp .cmd_line


section '.data' data readable writeable

cmd_input db "$ ", 0
