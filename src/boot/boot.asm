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
	    
	call	get_framebuffer_info
	jc @f
	
; call uefi function to print to screen

	uefi_call_wrapper ConOut,OutputString,ConOut,_hello

; hang here

	jmp	$ 

@@:
	mov eax,EFI_SUCCESS
	retn

;
; get_framebuffer_info
;
; This routine detects graphics output protocol (GOP) and gets the
; framebuffer information.
;
get_framebuffer_info:
	call detect_gop
	jc @f
@@:
	ret

;
; detect_gop
;
; This routine detects GOP.
;
detect_gop:
	uefi_call_wrapper	BootServices,LocateProtocol,gopuuid,0,\
				gopinterface
					; invoke BS->LocateProtocol function
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

section '.reloc' fixups data discardable
