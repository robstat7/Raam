;
; Raam Raam Ji _/\_ _/\_ _/\_
;
; uefi library for fasm by dileep sankhla
;
struc UINT32 {
	align 4	; enusre 4-byte alignment
	. dd ?	; anon member - reserve 4 bytes of uninitialized space
}

struc void {
	align 8
	. dq ?	; reserve 8 bytes of uninitialized space
}

; helper macro for definition of relative structure member offsets

macro struct name {
	virtual at 0	  ; starts a virtual block at memory address 0
		name name ; creates an instance of struc name with label name
	end virtual	  ; ends the virtual block
}

struc EFI_TABLE_HEADER {
	.Signature				void
	.Revision				UINT32
	.HeaderSize				UINT32
	.CRC32    				UINT32
	.Reserved				UINT32
}
struct EFI_TABLE_HEADER

struc EFI_SYSTEM_TABLE {
	.Hdr                    		EFI_TABLE_HEADER
	.FirmwareVendor         		void
	.FirmwareRevision       		UINT32
	.ConsoleInHandle        		void
	.ConIn                  		void
	.ConsoleOutHandle       		void
	.ConOut                 		void
	.StandardErrorHandle    		void
	.StdErr                 		void
	.RuntimeServices        		void
	.BootServices           		void
	.NumberOfTableEntries   		void
	.ConfigurationTable     		void
}
struct EFI_SYSTEM_TABLE

struc EFI_BOOT_SERVICES {
	.Hdr					EFI_TABLE_HEADER
	.RaiseTPL				void
	.RestoreTPL				void
	.AllocatePages				void
	.FreePages				void
	.GetMemoryMap				void
	.AllocatePool				void
	.FreePool				void
	.CreateEvent				void
	.SetTimer				void
	.WaitForEvent				void
	.SignalEvent				void
	.CloseEvent				void
	.CheckEvent				void
	.InstallProtocolInterface		void
	.ReinstallProtocolInterface		void
	.UninstallProtocolInterface		void
	.HandleProtocol				void
	.Reserved				void
	.RegisterProtocolNotify			void
	.LocateHandle				void
	.LocateDevicePath			void
	.InstallConfigurationTable		void
	.LoadImage				void
	.StartImage				void
	.Exit					void
	.UnloadImage				void
	.ExitBootServices			void
	.GetNextMonotonicCount			void
	.Stall					void
	.SetWatchdogTimer			void
	.ConnectController			void
	.DisconnectController			void
	.OpenProtocol				void
	.CloseProtocol				void
	.OpenProtocolInformation		void
	.ProtocolsPerHandle			void
	.LocateHandleBuffer			void
	.LocateProtocol				void
	.InstallMultipleProtocolInterfaces	void
	.UninstallMultipleProtocolInterfaces	void
	.CalculateCrc32				void
	.CopyMem				void
	.SetMem					void
	.CreateEventEx				void
}
struct EFI_BOOT_SERVICES

struc EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
	.Reset            			void
	.OutputString   			void
	.TestString      			void
	.QueryMode          			void
	.SetMode            			void
	.SetAttribute       			void
	.ClearScreen        			void
	.SetCursorPosition  			void
	.EnableCursor       			void
	.Mode             			void
}
struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL

struc EFI_GRAPHICS_OUTPUT_PROTOCOL {
	.QueryMode				void
	.SetMode				void
	.Blt					void
	.Mode					void
}
struct EFI_GRAPHICS_OUTPUT_PROTOCOL

struc EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE {
	.MaxMode				UINT32
	.Mode					UINT32
	.Info					void
	.SizeOfInfo				void
	.FrameBufferBase			void
	.FrameBufferSize			void
}
struct EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE

struc EFI_PIXEL_BITMASK {
	.RedMask				UINT32
	.GreenMask				UINT32
	.BlueMask				UINT32
	.ReservedMask				UINT32
}
struct EFI_PIXEL_BITMASK

struc EFI_GRAPHICS_OUTPUT_MODE_INFORMATION {
	.Version				UINT32
	.HorizontalResolution			UINT32
	.VerticalResolution			UINT32
	.PixelFormat				UINT32
	.PixelInformation			EFI_PIXEL_BITMASK
	.PixelsPerScanLine			UINT32
}
struct EFI_GRAPHICS_OUTPUT_MODE_INFORMATION


; EFI status constants

EFI_SUCCESS = 0
EFIERR = 0x8000000000000000
EFI_INVALID_PARAMETER = EFIERR or 0x2
EFI_BUFFER_TOO_SMALL = EFIERR or 0x5

; --- macros to make life easier ---

; InitializeLib:
; the image handle and the system table pointer are passed by the firmware to
; the bootloader in rcx and rdx registers respectively. First check if the
; system table pointer is not a null pointer and the first 4 bytes of the EFI
; table header signature is equal to 20494249h. After that, store the image
; handle and the system table pointer for future use.
; note: call this macro early after the entry point.
macro InitializeLib {
	clc		; clear carry flag
	or rdx, rdx
	jz .badout	; oops, system table pointer is a null pointer
	cmp dword [rdx], 20494249h
	je @f
.badout:
	xor rcx, rcx
	xor rdx, rdx
	stc		; set carry flag
@@:
	; store the image handle and the system table pointer
	mov qword [image_handle], rcx
	mov qword [system_table], rdx
}

; invoke a uefi function
macro uefi_call_wrapper	interface, function, arg1, arg2, arg3, arg4, arg5 {
	total_arg = 0
	if ~ arg5 eq
		total_arg = total_arg + 1
		if ~ arg5 eq r10
			mov r10, arg5
		end if
	end if
	if ~ arg4 eq
		total_arg = total_arg + 1
		if ~ arg4 eq r9
			mov r9, arg4
		end if
	end if
	if ~ arg3 eq
		total_arg = total_arg + 1
		if ~ arg3 eq r8
			mov r8, arg3
		end if
	end if
	if ~ arg2 eq
		total_arg = total_arg + 1
		if ~ arg2 eq rdx
			mov rdx, arg2
		end if
	end if
	if ~ arg1 eq
		total_arg = total_arg + 1
		if ~ arg1 eq rcx
			if ~ arg1 in <ConsoleInHandle, ConIn, \
				      ConsoleOutHandle, ConOut, \
				      StandardErrorHandle, StdErr, \
				      RuntimeServices, BootServices>
				mov rcx, arg1
			end if
		end if
	end if

	xor rax, rax
	mov al, total_arg
	if interface in <ConsoleInHandle, ConIn, ConsoleOutHandle, ConOut, \
			 StandardErrorHandle, StdErr, RuntimeServices, \
			 BootServices>
		mov rbx, [system_table]
		mov rbx, [rbx + EFI_SYSTEM_TABLE.#interface]
	else
		if ~ interface eq rbx
			mov rbx, interface
		end if
	end if
	if arg1 in <ConsoleInHandle, ConIn, ConsoleOutHandle, ConOut, \
		    StandardErrorHandle, StdErr, RuntimeServices, BootServices>
		mov rcx, rbx
	end if
	if defined EFI_GRAPHICS_OUTPUT_PROTOCOL.#function
		mov rbx, [rbx + EFI_GRAPHICS_OUTPUT_PROTOCOL.#function]
	else if defined EFI_BOOT_SERVICES.#function
		mov rbx, [rbx + EFI_BOOT_SERVICES.#function]
	else if defined EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.#function
		mov rbx, [rbx + EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.#function]
	else if defined EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE.#function
		mov rbx, [rbx + EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE.#function]
	else
		mov rbx, [rbx + function]
	end if

	call uefi_func
}

;
; library functions
;

section '.text' code executable readable

; uefi_func function
; note: see the microsoft x64 calling convention for shadow space details.
uefi_func:
	; save stack pointer
	mov qword [uefi_rsptmp], rsp
	; set up new aligned stack:
	; as per the microsoft x64 calling convention, the stack pointer (rsp)
	; must be 16-byte aligned at the point of any function call.
	and esp, 0FFFFFFF0h
	; alignment check on arguments:
	; if total_arg is less than 5, we don't need any stack pointer
	; alignment. Reason: we only pass 5th and further arguments via the
	; stack and it might or might not misalign it.
	cmp eax, 5	; al contains the total_arg from uefi_call_wrapper macro
	jb .before_call
	bt eax, 0
	jnc @f
	push rax	; push dummy rax reg value for stack alignment purpose
	; arguments
@@:	cmp al, 5
	jb @f
	push r10
@@:
.before_call:
	; 32 bytes (shadow) space for
	; r9
	; r8
	; rdx
	; rcx
	sub rsp, 4*8
	; call function
	call rbx
	; restore old stack
	mov rsp, qword [uefi_rsptmp]
	ret


section '.data' data readable writeable

image_handle   	dq 0
system_table   	dq 0
uefi_rsptmp	dq 0
; GUIDs
EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID:
		dd 0x9042a9de
		dw 0x23dc
		dw 0x4a38
		db 0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a
EFI_BLOCK_IO_PROTOCOL_GUID:
		dd 0x964e5b21
		dw 0x6459
		dw 0x11d2
		db 0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b
