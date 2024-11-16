;
; Raam Raam sa _/\_ _/\_ _/\_
;
; Boot Loader code.
;
format pe64 dll efi
entry start

section '.text' code executable readable

; include UEFI library.

include 'uefi.inc'

start:
; initialize UEFI library. Following is the name of a macroinstruction.

	InitializeLib
	
; `jc` is jump if carry. Symbol @f references the nearest following
; anonymous label i.e. @@. The carry flag is set on an error during
; library initialization.

	jc	@f

; call boot loader's main routine.

	call bootloader_main
@@:
	mov eax,EFI_SUCCESS
	retn

;
; bootloader_main
;
; This routine stores the framebuffer information, exits boot services,
; and jumps to the kernel.
;
bootloader_main:
; first we store the framebuffer info that will be needed by the kernel
; to initialize the console output.

	call	store_framebuffer_info
	jc	@f
	call	exit_boot_services

; hang here

	jmp	$
@@:
	ret

;
; exit_boot_services
;
; This routine gets the memory map and tries to exit boot services 3
; times.
;
exit_boot_services:
	call	get_memory_map
	jc	@f
@@:
	ret

;
; get_memory_map
;
; This routine gets the memory map size, updates it, and allocates a new
; pool of memory for the memory map. Then it gets the memory map into
; this newly allocated pool.
;
get_memory_map:
	call	get_memory_map_size
	jc	@f
	call	update_memory_map_size
@@:
	ret

;
; update_memory_map_size
;
; This routine updates the memory map size. It basically performs
; memory_map_size += 2 * descriptor_size. Allocating the pool creates at
; least one new descriptor for the chunk of memory changed to
; EfiLoaderData. Not sure that UEFI firmware must allocate on a memory
; type boundary! If not, then two descriptors might be created.
;
; We will use this updated memory map size to allocate pool for the
; memory map and to get memory map later.
;
update_memory_map_size:
	mov	rax,qword[desc_size]
	mov	bl,2
	mul	bl
	mov	rcx,qword[memory_map_size]
	add	rcx,rax
	clc
	mov	qword[memory_map_size],rcx
	ret

;
; get_memory_map_size
;
; This routine gets the memory map size.
;
get_memory_map_size:
	uefi_call_wrapper	BootServices,GetMemoryMap,memory_map_size,0,\
				map_key,desc_size,0
	mov	rbx,EFI_BUFFER_TOO_SMALL
	cmp	rax,rbx
	jne	.error
	ret
.error:
	uefi_call_wrapper	ConOut,OutputString,ConOut,error_msg1
	stc
	ret

;
; store_framebuffer_info
;
; This routine detects graphics output protocol (GOP) and stores the
; framebuffer information.
;
store_framebuffer_info:
	call detect_gop
	jc @f
	mov	rax,qword[gopinterface]
	mov	rax,qword[rax+EFI_GRAPHICS_OUTPUT_PROTOCOL.Mode]
	mov	rbx,qword[rax+\
			  EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE.FrameBufferBase]
	mov	qword[framebuffer_base],rbx
	mov	rcx,qword[rax+\
			  EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE.FrameBufferSize]
	mov	qword[framebuffer_size],rcx
	mov	rax,qword[rax+EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE.ModeInfo]
	xor	rdx,rdx
	mov	edx,dword[rax+\
		      EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.HorizontalResolution]
	mov	dword[horizontal_res],edx
	xor	rdx,rdx
	mov	edx,dword[rax+\
			EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.VerticalResolution]
	mov	dword[vertical_res],edx
	xor	rdx,rdx
	mov	edx,dword[rax+\
			EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.PixelsPerScanline]
	mov	dword[pixels_per_scanline],edx
@@:
	ret

;
; detect_gop
;
; This routine detects GOP.
;
detect_gop:
	uefi_call_wrapper	BootServices,LocateProtocol,gopuuid,0,\
				gopinterface ;invoke BS->LocateProtocol function
	mov	rbx,EFI_SUCCESS
	cmp	rax,rbx
	jne	.error
	ret
.error:
	xor	rax,rax
	mov	qword[gopinterface],rax
	stc
	ret

section '.data' data readable writeable

error_msg1:	du	'fatal error: error getting memory map size',13,10,0
gopuuid:		db		EFI_GRAPHICS_OUTPUT_PROTOCOL_UUID
gopinterface:		dq		0
framebuffer_base:	dq		0
framebuffer_size:	dq		0
horizontal_res:		dw		0
vertical_res:		dw		0
pixels_per_scanline:	dw		0
memory_map_size:	dq		0
map_key:		dq		0
desc_size:		dq		0

section '.reloc' fixups data discardable
