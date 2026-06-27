;
; Raam Raam Ji _/\_ _/\_ _/\_
;
include 'tty_io.asm'

include 'printk.asm'

include 'gdt.asm'


section '.text' code executable readable

; initialize our kernel here.
kernel_init:
  ; first disable interrupts
  cli
  call gdt_init
  call default_tty_init

  lea rax, [msg1]
  call printk

  lea rax, [msg2]
  call printk

  jmp $


section '.data' data readable writeable

msg1   db "Raam Raam sa", 10, 0
msg2   db "Dileep Sankhla", 0
