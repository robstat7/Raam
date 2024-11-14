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
	    
	call	check_if_gop_supported
	jc @f
	
; call uefi function to print to screen

	uefi_call_wrapper ConOut,OutputString,ConOut,_hello

; hang here

	jmp	$ 

@@:
	mov eax,EFI_SUCCESS
	retn

;
; check_if_gop_supported
;
; This routine performs installation check on graphics output protocol
; (GOP). As the buffer size is 0, calling EFI_BOOT_SERVICES.LocateHandle
; function must return buffer too small. This will tell
; GOP is supported.
;
check_if_gop_supported:
	uefi_call_wrapper	BootServices,LocateHandle,2,gopuuid,0,tmp,\
				gop_handle ; invoke BS->LocateHandle function
	mov	rbx,EFI_BUFFER_TOO_SMALL
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
tmp:		dq			0
gopuuid:	db			EFI_GRAPHICS_OUTPUT_PROTOCOL_UUID
gop_handle:	dq			0
gopinterface:	dq			0

section '.reloc' fixups data discardable
