PIC1 = 0x20     ; IO base address for master PIC
PIC2 = 0xa0     ; IO base address for slave PIC
PIC1_COMMAND = PIC1
PIC1_DATA = PIC1 + 1
PIC2_COMMAND = PIC2
PIC2_DATA = PIC2 + 1

ICW1_ICW4 = 0x01    ; indicates that ICW4 will be present
ICW1_INIT = 0x10    ; initialization - required!
ICW2_MASTER = 0x20  ; master PIC vector offset
ICW2_SLAVE = 0x28   ; slave PIC vector offset
ICW4_8086 = 0x01    ; 8086/88 (MCS-80/85) mode

CASCADE_IRQ = 2


section '.text' code executable readable

; a simple but imprecise wait for older machines.
; Wait a very small amount of time (1 to 4 microseconds, generally).
; Port 0x80 is an unused port.
io_wait:
  xor eax, eax
  out 0x80, al
  ret

; initialize the PICs
pic_init:
  xor edx, edx
  mov dx, PIC1_COMMAND
  xor eax, eax
  mov al, ICW1_INIT
  or al, ICW1_ICW4
  out dx, al            ; starts the initialization sequence (in cascade mode)
  mov dx, PIC2_COMMAND
  mov al, ICW1_INIT
  or al, ICW1_ICW4
  out dx, al

  mov dx, PIC1_DATA
  mov al, ICW2_MASTER
  out dx, al

  mov dx, PIC2_DATA
  mov al, ICW2_SLAVE
  out dx, al

  mov dx, PIC1_DATA
  mov al, 1
  shl al, CASCADE_IRQ
  out dx, al          ; ICW3: tell master PIC that there is a slave PIC at IRQ2

  mov dx, PIC2_DATA
  mov al, CASCADE_IRQ
  out dx, al          ; ICW3: tell slave PIC its cascade identity

  ; ICW4: have the PICs use 8086 mode (and not 8080 mode)

  mov dx, PIC1_DATA
  mov al, ICW4_8086
  out dx, al

  mov dx, PIC2_DATA
  mov al, ICW4_8086
  out dx, al

  ; unmask both PICs

  xor eax, eax
  mov dx, PIC1_DATA
  out dx, al

  mov dx, PIC2_DATA
  out dx, al

  ret
