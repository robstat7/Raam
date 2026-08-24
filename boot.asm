;
; Raam Raam Ji _/\_ _/\_ _/\_
;
; uefi bootloader using fasm.
;
format pe64 efi

entry start

; include the uefi library
include 'uefi.inc'

include 'kernel.asm'

include 'xsdp.asm'


struc FRAMEBUFFER {
	.framebuffer_base	        void
	.framebuffer_size	        void
	.horizontal_res		        UINT32
	.vertical_res		          UINT32
	.pixels_per_scanline	    UINT32
}
struct FRAMEBUFFER


section '.text' code executable readable

start:
  ; first initialize the uefi library
	InitializeLib

  ; note: the carry flag is set on an error during the library initialization
	jc .error_exit

  ; now set the video mode for 1280 x 1024 resolution
  call set_video_mode_1280x1024
	jc .hang

  ; and store the framebuffer information for this newly set video mode.
  call store_framebuffer_info	
	jc .hang

  ; now get the xsdp pointer.
  lea rax, [system_table]
  mov rdi, qword [rax]
  call get_xsdp_pointer
  jc .hang

  ; exit the boot services.
  call exit_boot_services

  ; go to kernel initialization code.
  mov rdi, qword [xsdp_pointer]   ; boot param
  call kernel_init

.hang:
  jmp $

.error_exit:
	; return back to firmware. The returning status code should be
	; in rax register.
	mov rax, EFIERR		; error status code
	ret 

;
; store_framebuffer_info
;
; this function stores the framebuffer information.
;
store_framebuffer_info:
	; allocate a new pool of memory to store the framebuffer information

	uefi_call_wrapper BootServices, AllocatePool, 2, \
			  qword [framebuffer_info_size], framebuffer_info
						     ; 2 indicates EfiLoaderData
	mov rbx, EFI_SUCCESS
	cmp rax, rbx
	jne .error1

	; now store the framebuffer information

	mov rax, qword [gopinterface]
	mov rcx, qword [framebuffer_info]
	mov rax, qword [rax + EFI_GRAPHICS_OUTPUT_PROTOCOL.Mode]
	mov rbx, qword [rax + EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE.FrameBufferBase]
	mov qword [rcx + FRAMEBUFFER.framebuffer_base], rbx

	mov rbx, qword [rax + EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE.FrameBufferSize]
	mov qword [rcx + FRAMEBUFFER.framebuffer_size], rbx

	mov rax, qword [rax + EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE.Info]
	mov edx, dword [rax + \
		EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.HorizontalResolution]
	mov dword [rcx + FRAMEBUFFER.horizontal_res], edx

	mov edx, dword [rax + \
		EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.VerticalResolution]
	mov dword [rcx + FRAMEBUFFER.vertical_res], edx

	mov edx, dword [rax + \
		EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.PixelsPerScanLine]
	mov dword [rcx + FRAMEBUFFER.pixels_per_scanline], edx
.exit:
	ret
.error1:
	uefi_call_wrapper ConOut, OutputString, ConOut, error_msg_8
	jmp $	; hang here on error

;
; set_video_mode_1280x1024
;
; this function gets the first graphics output protocol (GOP) instance, stores
; the total number of video modes information, queries available video modes and
; gets the video mode code for 1280 x 1024 resolution. On success, it sets this
; new video mode.
; Note that we need to set this new video mode for better console readability.
;
set_video_mode_1280x1024:
	; get the first protocol instance that matches the GOP

	; invoke EFI_BOOT_SERVICES.LocateProtocol() function
	uefi_call_wrapper BootServices, LocateProtocol, \
			  EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID, 0, gopinterface
	mov rbx, EFI_SUCCESS
	cmp rax, rbx
	jne .error1

	; store the total number of video modes information

	mov rax, qword [gopinterface]
	mov rbx, qword [rax + EFI_GRAPHICS_OUTPUT_PROTOCOL.Mode]
	mov edx, dword [rbx + EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE.MaxMode]
	mov dword [total_video_modes], edx

	; query available video modes and get the mode code for 1280 x 1024
	; resolution

.loop_start:
	mov ebx, dword [total_video_modes]
	; check loop condition
	cmp [counter_2], ebx
	jae .loop_end

	; loop body
	mov rbx, [gopinterface]
	mov edx, [counter_2]
	; invoke EFI_GRAPHICS_OUTPUT_PROTOCOL.QueryMode() function
	uefi_call_wrapper rbx, QueryMode, rbx, rdx, size_of_info, info

	mov rax, [info]
	mov ebx, dword [rax + \
		EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.HorizontalResolution]
	cmp ebx, 1280
	jne .next_iteration
	mov ebx, dword [rax + \
		EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.VerticalResolution]
	cmp ebx, 1024
	jne .next_iteration
	mov ebx, dword [rax + EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.PixelFormat]
	cmp ebx, 1
	jne .next_iteration

	mov edx, dword [counter_2]
	mov dword [mode_code], edx
	jmp .loop_end

.next_iteration:
	inc [counter_2]
	jmp .loop_start

.loop_end:
	mov edx, dword [mode_code]
	cmp edx, 100
	je .error2

	; set this new video mode

	mov rbx, [gopinterface]
	mov edx, dword [mode_code]
	; invoke EFI_GRAPHICS_OUTPUT_PROTOCOL.SetMode() function
	uefi_call_wrapper rbx, SetMode, rbx, rdx
	mov rbx, EFI_SUCCESS
	cmp rax, rbx
	jne .error3
	jmp .exit

.error1:
	stc
	uefi_call_wrapper ConOut, OutputString, ConOut, error_msg_1
	jmp .exit
.error2:
	stc
	uefi_call_wrapper ConOut, OutputString, ConOut, error_msg_2
	jmp .exit
.error3:
	stc
	uefi_call_wrapper ConOut, OutputString, ConOut, error_msg_3
	jmp .exit
.exit:
	ret

;
; exit_boot_services
;
; this function exits the boot services.
;
exit_boot_services:
	; get memory map size

	; invoke EFI_BOOT_SERVICES.GetMemoryMap() function
	uefi_call_wrapper BootServices, GetMemoryMap, memory_map_size, 0, \
			  map_key, desc_size, 0
	mov rbx, EFI_BUFFER_TOO_SMALL
	cmp rax, rbx
	jne .error1

	; update the memory map size
	;
	; We basically perform memory_map_size += 2 * descriptor_size.
	; Allocating the pool creates at least one new descriptor for the chunk
	; of memory changed to EfiLoaderData. Not sure that UEFI firmware must
	; allocate on a memory type boundary! If not, then two descriptors might
	; be created.
	;
	; We will use this updated memory map size to allocate pool for the
	; memory map and to get memory map later.

	mov rax, qword [desc_size]
	mov bl, 2
	mul bl
	mov rcx, qword [memory_map_size]
	add rcx, rax
	clc
	mov qword [memory_map_size], rcx

	; allocate a new pool of memory for the memory map

	uefi_call_wrapper BootServices, AllocatePool, 2, qword [memory_map_size], \
			  memory_map	; 2 indicates EfiLoaderData
	mov rbx, EFI_SUCCESS
	cmp rax, rbx
	jne .error2

	; get memory map

	uefi_call_wrapper BootServices, GetMemoryMap, memory_map_size, \
			  qword [memory_map], map_key, desc_size, 0
	mov rbx, EFI_SUCCESS
	cmp rax, rbx
	jne .error3

	; now try to exit boot services 3 times

	xor cx, cx
.exit_bs_loop:
	uefi_call_wrapper BootServices, ExitBootServices, \
			  qword [image_handle], qword [map_key]
	mov rbx, EFI_SUCCESS
	cmp rax, rbx
	je .exit
	inc cx
	cmp cx, 3
	jl .exit_bs_loop
.error0:
	uefi_call_wrapper ConOut, OutputString, ConOut, error_msg_7
	jmp .hang
.error1:
	uefi_call_wrapper ConOut, OutputString, ConOut, error_msg_4
	jmp .hang
.error2:
	uefi_call_wrapper ConOut, OutputString, ConOut, error_msg_5
	jmp .hang
.error3:
	uefi_call_wrapper ConOut, OutputString, ConOut, error_msg_6
	jmp .hang
.hang:
	; hang here on error
	jmp $
.exit:
	ret


section '.data' data readable writeable

; the console needs "\r\n" to properly move to the start of the next line
greeting_text	du "_/\_ Raam Raam Ji _/\_", 13, 10, 0
gopinterface	dq 0
size_of_info	dq 0
info		dq 0

error_msg_1	du "error: unable to get the GOP instance!", 0
error_msg_2    du "error: unable to get video mode for 1280 x 1024 resolution!"\
		   , 0
error_msg_3    du "error: unable to set video mode for 1280 x 1024 resolution!"\
		   , 0
error_msg_4	du "fatal error: error getting memory map size!", 0
error_msg_5	du "fatal error: error allocating memory map buffer!", 0
error_msg_6	du "fatal error: error getting memory map!", 0
error_msg_7	du "fatal error: exit boot services: map key is incorrect!", 0
error_msg_8	du "fatal error: error allocating fb info buffer!", 0

total_video_modes dd 0
mode_code	dd 100	; taking an arbitrary high mode code
counter_2		dd 0
; framebuffer info variables
framebuffer_info_size	dq 28	; 28 bytes to hold framebuffer information
framebuffer_info	dq 0
; memory map variables
memory_map_size	dq 0
map_key		dq ?
desc_size	dq ?
memory_map	dq 0
