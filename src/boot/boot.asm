;
; Raam Raam sa _/\_ _/\_ _/\_
;
; Boot Loader code.
;
format pe64 dll efi
entry start

section '.text' code executable readable

include 'uefi.inc'

start:
; initialize UEFI library.

	InitializeLib
	
; `jc` is jump if carry. Symbol @f references the nearest following anonymous
; label (@@). The carry flag is set on an error during library initialization.

	jc @f
	    
	call set_1280_by_1024_video_mode
	jc @f
	
; call uefi function to print to screen

	uefi_call_wrapper ConOut,OutputString,ConOut,_hello

; hang here

	jmp	$ 

@@:
	mov eax,EFI_SUCCESS
	retn

;
; set_1280_by_1024_video_mode
;
; This routine checks if GOP is supported, queries the available video
; modes, and get the mode number for 1280 x 1024 resolution. Set this
; mode. We will use this mode for better text visibility.
;
set_1280_by_1024_video_mode:
	call	check_if_gop_supported
	mov	rbx,EFI_BUFFER_TOO_SMALL	
	cmp	rax,rbx
	jne	.error
	ret
.error:
	xor	rax,rax
	mov	qword[gopinterface],rax
	stc
	ret

;
; check_if_gop_supported
;
; This routine performs installation check on GOP. It must return buffer
; too small if GOP is supported.
;
check_if_gop_supported:
	uefi_call_wrapper	BootServices,LocateHandle,2,gopuuid,0,tmp,gop_handle
	ret

; data
tmp:		dq			0
gopuuid:	db			EFI_GRAPHICS_OUTPUT_PROTOCOL_UUID
gop_handle:	dq			0
gopinterface:	dq			0

section '.data' data readable writeable

_hello                                  du 'Hello World',13,10,0

section '.reloc' fixups data discardable
