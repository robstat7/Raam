PS2_CMD_PORT = 0x64
PS2_DATA_PORT = 0x60


; PS/2 controller configuration byte commands
CMD_READ_CONFIG_BYTE = 0x20
CMD_WRITE_CONFIG_BYTE = 0x60


section '.text' code executable readable

kbd_init:
  ; read PS/2 controller configuration byte
  mov al, CMD_READ_CONFIG_BYTE
  out PS2_CMD_PORT, al

  in al, PS2_DATA_PORT

  ; enable IRQ1 keyboard interrupt
  or al, 0x1
  mov bl, al

  ; write PS/2 controller configuration byte
  mov al, CMD_WRITE_CONFIG_BYTE
  out PS2_CMD_PORT, al

  mov al, bl
  out PS2_DATA_PORT, al

  ret
