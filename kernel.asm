;
; Raam Raam Ji _/\_ _/\_ _/\_
;
include 'font.asm'

struc TTY {
	.framebuffer_base	        void
	.horizontal_res		        UINT32
	.vertical_res		          UINT32
	.pixels_per_scanline	    UINT32
  .cursor_x                 UINT32
  .cursor_y                 UINT32
  .fg_color                 UINT32
  .bg_color                 UINT32
}
struct TTY


; constants

; tty colors
COLOR_WHITE = 0xffffff
COLOR_BLACK = 0x000000

; origin coordinate
COORDINATE_ORIGIN = 0


; font height and width
FONT_HEIGHT = 16
FONT_WIDTH = 8


section '.text' code executable readable

; initialize our kernel here.
kernel_init:
  call default_tty_init

  mov al, 'R'
  call tty_put_char

  mov al, 'a'
  call tty_put_char

  jmp $

;
; tty_put_char
;
; this function puts a single character onto the terminal screen. It
; uses the 8x16 font array to get the font's data and writes it to the
; framebuffer pixel by pixel. It only writes the "on" pixels with the
; foreground color. It then updates the terminal cursors' positions to
; put the next character at the right place.
;
; args:
;   @al register = the ascii code of the character to print
;
; returns:
;   nothing
;
tty_put_char:
  ; reset row and col
  mov dword [row], 0
  mov dword [col], 0

  xor ebx, ebx
  mov bl, al
  imul ebx, FONT_HEIGHT
  mov dword [offset], ebx

.row_loop:  
  cmp [row], 16     ; 16 rows in a 8x16 font
  jae .after

  mov eax, dword [offset]
  add eax, dword [row]

  xor ebx, ebx
  add rax, font_8x16
  mov bl, byte [rax]
  mov byte [row_data], bl

  mov dword [col], 0      ; reset col at the start of each row
.col_loop:
  cmp [col], 8
  jae .row_loop_inc

  ; check if we need to put a pixel from the fonts row
  mov rax, mask
  mov ebx, dword [col]
  add rax, rbx
  xor ebx, ebx
  mov bl, byte [row_data] 
  mov dl, byte [rax]          ; mask
  test bl, dl
  jz .col_loop_inc

  mov r8, default_tty
  mov eax, dword [r8 + TTY.fg_color]
  mov ebx, dword [r8 + TTY.cursor_x]
  add ebx, [col]
  mov ecx, dword [r8 + TTY.cursor_y]
  add ecx, [row]
  call tty_put_pixel

.col_loop_inc:
  inc dword [col]
  jmp .col_loop

.row_loop_inc:
  inc dword [row]
  jmp .row_loop

.after:
  ; update tty cursors' positions
  call update_tty_cursors
  
.exit:
  ret

;
; update_tty_cursors
;
; this function updates the terminal screen's cursors' positions to put
; the next character at the right place.
;
; args:
;   nothing
;
; retuns:
;   nothing
;
update_tty_cursors:
  mov r8, default_tty

  mov eax, dword [r8 + TTY.cursor_x]
  add eax, FONT_WIDTH
  mov dword [r8 + TTY.cursor_x], eax

  mov ebx, dword [r8 + TTY.horizontal_res]
  cmp dword [r8 + TTY.cursor_x], ebx
  jl .exit

  mov dword [r8 + TTY.cursor_x], 0    ; wrap around to the next line

  ; move down by the font height
  mov eax, dword [r8 + TTY.cursor_y]
  add eax, FONT_HEIGHT
  mov dword [r8 + TTY.cursor_y], eax

.exit:
  ret

;
; tty_put_pixel
;
; this function puts a single pixel on the terminal. It should only be
; called by `tty_put_char`.
;
; params:
;   eax: pixel_color
;   ebx: x
;   ecx: y
;
tty_put_pixel:
  xor edx, edx
  mov r8, default_tty
  mov edx, dword [r8 + TTY.pixels_per_scanline]
  imul edx, ecx
  add edx, ebx
  shl edx, 2           ; * 4 (bytes per pixel)
  add rdx, qword [r8 + TTY.framebuffer_base]
  mov qword [pixel_addr], rdx

  mov r9, qword [pixel_addr]
  mov dword[r9], eax
  ret

; 
; default_tty_init
;
; this function initializes the default terminal (default_tty) attributes.
;
; args:
;   nothing
;
; returns:
;   nothing
;
default_tty_init:
  mov rax, qword [framebuffer_info] 
  mov rbx, default_tty

  mov rcx, qword [rax + FRAMEBUFFER.framebuffer_base]
  mov qword [rbx + TTY.framebuffer_base], rcx

  mov ecx, dword [rax + FRAMEBUFFER.horizontal_res]
  mov dword [rbx + TTY.horizontal_res], ecx

  mov ecx, dword [rax + FRAMEBUFFER.vertical_res]
  mov dword [rbx + TTY.vertical_res], ecx

  mov ecx, dword [rax + FRAMEBUFFER.pixels_per_scanline]
  mov dword [rbx + TTY.pixels_per_scanline], ecx

  mov dword [rbx + TTY.cursor_x], COORDINATE_ORIGIN
  mov dword [rbx + TTY.cursor_y], COORDINATE_ORIGIN

  mov dword [rbx + TTY.fg_color], COLOR_WHITE
  mov dword [rbx + TTY.bg_color], COLOR_BLACK

  ret


section '.data' data readable writeable

; reserve 36 bytes to hold the default terminal attributes (see struc TTY above)
align 8
default_tty     rb 36

offset          dd 0
row             dd 0
col             dd 0
row_data        db 0

pixel_addr      dq 0

; mask for each pixel in a row
mask:
  db 128
  db 64
  db 32
  db 16
  db 8
  db 4
  db 2
  db 1
