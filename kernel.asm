;
; Raam Raam Ji _/\_ _/\_ _/\_
;
include 'tty_io.asm'

include 'printk.asm'

include 'gdt.asm'

include 'idt.asm'

include 'pic.asm'

include 'kbd.asm'


section '.text' code executable readable

; initialize our kernel here.
kernel_init:
  ; first disable interrupts
  cli

  call gdt_init
  call idt_init

  call pic_init
  call kbd_init

  call default_tty_init

  ; enable interrupts now
  sti

  lea rax, [msg_debug]
  call printk

  jmp $


section '.data' data readable writeable

msg_debug db "Test message", 10, 0
