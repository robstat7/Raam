; Boot Loader written in FASM
struc UINT32 {
	align 4
	. dd ?
}

struc void {
	align 8
    	. dq ?
}

macro struct name {
	virtual at 0
		name name
	end virtual
}

struc EFI_TABLE_HEADER {
	.Signature		void
    	.Revision		UINT32
    	.HeaderSize             UINT32
    	.CRC32                  UINT32
    	.Reserved               UINT32
}
struct EFI_TABLE_HEADER

struc EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
	.Reset                  void
    	.OutputString           void
    	.TestString             void
    	.QueryMode              void
    	.SetMode                void
    	.SetAttribute           void
    	.ClearScreen            void
    	.SetCursorPosition      void
    	.EnableCursor           void
    	.Mode                   void
}
struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL

struc EFI_SYSTEM_TABLE {
	.Hdr                    EFI_TABLE_HEADER
    	.FirmwareVendor         void
    	.FirmwareRevision       UINT32
    	.ConsoleInHandle        void
    	.ConIn                  void
    	.ConsoleOutHandle       void
    	.ConOut                 void
    	.StandardErrorHandle    void
    	.StdErr                 void
    	.RuntimeServices        void
    	.BootServices           void
    	.NumberOfTableEntries   void
    	.ConfigurationTable     void
}
struct EFI_SYSTEM_TABLE

format PE64 EFI

entry start

section '.text' code executable readable

start:
	; store the image handle and the system table pointer passed by the firmware
	mov [ImageHandle], rcx
	mov [SystemTable], rdx

	mov rax,[SystemTable]
        
        push rbp
        sub rsp,0x20
        
	mov rdx,TestOkText
        mov rcx,[rax+EFI_SYSTEM_TABLE.ConOut]
        mov rcx,[rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString]

	call rcx
	add rsp,0x20
	; hang here
	jmp $

section '.data' data readable writeable

ImageHandle   dq ?
SystemTable   dq ?
TestOkText:   du 'Test OK',0
