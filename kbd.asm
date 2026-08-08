PS2_DATA_PORT = 0x60


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

  lea rdi, [msg_scan_code]
  xor esi, esi
  mov sil, al
  call printk

  mov edi, 1    ; set IRQ no. 1 for keyboard interrupt in dil register
  call pic_send_eoi
  pop rdi
  ret


section '.data' data readable writeable

msg_scan_code db "{p}", 0
