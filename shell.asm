section '.text' code executable readable

run_shell:
  lea rdi, [first_msg]
  call printk

.loop_start:
  lea rdi, [cmd_input]
  call printk

  mov byte [input_mode], INPUT_MODE_ON

.loop_write_command:
  cmp byte [input_mode], INPUT_MODE_OFF
  je .process_cmd
  jmp .loop_write_command

.process_cmd:
  lea rdi, [newline_string]
  call printk

  lea rdi, [input_buffer]
  call strlen
  cmp eax, 1  ; user just pressed enter in command line
  je .end

.cmd_1:
  lea rdi, [input_buffer]
  lea rsi, [cmd_1]
  mov edx, 5
  call strncmp
  cmp eax, 0
  jne .cmd_2

  lea rdi, [cmd_1_response]
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
  lea rdi, [input_buffer]
  lea rsi, [cmd_3]
  mov edx, 4
  call strncmp
  cmp eax, 0
  jne .cmd_4

  lea rdi, [input_buffer]
  call strlen
  cmp eax, 5  ; cmd 3 length itself
  jne .next

  lea rdi, [newline_string]
  call printk
  jmp .end


.next:
  lea rdi, [input_buffer]
  add rdi, 5  ; length of "echo "
  call printk
  jmp .end

.cmd_4:
  lea rdi, [input_buffer]
  lea rsi, [cmd_4]
  mov edx, 7
  call strncmp
  cmp eax, 0
  jne .default

  ; using intel's motherboard chipset register for reboot
  ; specification link:
  ; Intel 400 Series Chipset Register Database (https://tinyurl.com/mrxudsnd)
  mov dx, 0xcf9       ; reset control register
  mov al, 0x06        ; bit 1 = system reset, bit 2 = reset cpu
  out dx, al
  jmp .end

.default:
  lea rdi, [cmd_not_found_msg]
  call printk

.end:
  jmp .loop_start


section '.data' data readable writeable

first_msg db 'Type "help" for the list of available commands', 10, 10, 0
cmd_input db "$ ", 0
newline_string db 10, 0

SHELL_COMMANDS:
cmd_1 db "help", 10, 0
cmd_2 db "clear", 10, 0
cmd_3 db "echo", 10, 0
cmd_4 db "reboot", 10, 0

cmd_1_response db "Available commands:", 10, "help", 10, "clear", 10, "echo", \
10, "reboot", 10, 0

cmd_not_found_msg db "Command not found", 10, 0
