;
; Raam Raam Ji _/\_ _/\_ _/\_
;
include 'tty_io.asm'

include 'printk.asm'

include 'gdt.asm'

include 'idt.asm'

include 'int_handler.asm'

include 'pic.asm'

include 'kbd.asm'

include 'shell.asm'


section '.text' code executable readable

; initialize our kernel here.
kernel_init:
  ; first disable interrupts
  cli

  call default_tty_init

  call gdt_init
  call idt_init

  call array_interrupt_handlers_init

  call pic_init

  ; register keyboard interrupt handler
  mov dil, IRQ1_INT_NUM
  lea rsi, [keyboard_interrupt_handler]
  call register_interrupt_handler

  ; register timer interrupt handler
  mov dil, IRQ0_INT_NUM
  lea rsi, [timer_interrupt_handler]
  call register_interrupt_handler

  ; enable interrupts now
  sti

  call run_shell

  jmp $
