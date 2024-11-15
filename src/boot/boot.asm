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

	jc @f

; first we store the framebuffer info that will be needed to print onto
; the console.

	call	store_framebuffer_info
	jc @f

; call uefi function to print to screen

	uefi_call_wrapper ConOut,OutputString,ConOut,_hello

; hang here

	jmp	$

@@:
	mov eax,EFI_SUCCESS
	retn

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

_hello                                  du 'Hello World',13,10,0
gopuuid:	db			EFI_GRAPHICS_OUTPUT_PROTOCOL_UUID
gopinterface:	dq			0
framebuffer_base:	dq		0
framebuffer_size:	dq		0
horizontal_res:		dw		0
vertical_res:		dw		0
pixels_per_scanline:	dw		0

section '.reloc' fixups data discardable
