section '.text' code executable readable

;
; get_xsdp_pointer
;
; this function gets the xsdp structure pointer by examining the EFI
; Configuration Table within the EFI System Table. It clears the carry
; flag on success else sets it.
;
; note:
; as per the specification, the boot loader must retrieve the pointer to
; the xsdp structure before assuming platform control via the EFI
; ExitBootServices interface.
;
; args:
;   @rdi = efi system table pointer
;
; returns:
;   nothing
;
get_xsdp_pointer:
  push rbp
  mov rbp, rsp

  sub rsp, 24

  total_config_tables equ qword [rbp - 8]
  config_tables_ptr equ qword [rbp - 16]
  i equ qword [rbp - 24]

	mov rax, qword [rdi + EFI_SYSTEM_TABLE.NumberOfTableEntries]
  mov total_config_tables, rax
	mov rbx, qword [rdi + EFI_SYSTEM_TABLE.ConfigurationTable]
  mov config_tables_ptr, rbx

  mov i, 0

.loop_start:
  mov rcx, total_config_tables
  cmp i, rcx
  jae .loop_end

  mov rdx, config_tables_ptr
  mov rax, i
  imul eax, sizeof.EFI_CONFIGURATION_TABLE
  add rdx, rax      ; we get config_tables_ptr[i]
  push rdx

  push rdi
  lea rdi, [rdx + EFI_CONFIGURATION_TABLE.VendorGuid]
  lea rsi, [EFI_ACPI_20_TABLE_GUID]
  call compare_guid
  pop rdi
  cmp eax, 0
  jne .loop_next

  pop rdx
  mov rax, qword [rdx + EFI_CONFIGURATION_TABLE.VendorTable]
  mov qword [xsdp_pointer], rax
  jmp .loop_end

.loop_next:
  inc i
  jmp .loop_start

.loop_end:
  cmp qword [xsdp_pointer], 0
  je .set_carry_flag
  clc
  jmp .end

.set_carry_flag:
	uefi_call_wrapper ConOut, OutputString, ConOut, error_msg_xsdp
  stc

.end:
  restore total_config_tables
  restore config_tables_ptr
  restore i

  mov rsp, rbp
  pop rbp
	ret

;
; compare_guid
;
; this function compares the two given GUIDs.
;
; args:
;   @rdi = address of the first GUID to be compared
;   @rsi = address of the second GUID to be compared
;
; returns:
;   @eax = an integer. Returns zero if both the GUIDs are equal.
;
compare_guid:
  push rbp
  mov rbp, rsp

  sub rsp, 5

  i equ byte [rbp - 1]
  res equ dword [rbp - 5]

	; compare 32 bits at a time	
	mov i, 0
	mov res, 0

.loop_start:
	cmp i, 4
	je .loop_end

	mov rax, rdi

	mov ebx, 4	; 32-bit integer size
  xor ecx, ecx
	mov cl, i
	imul ebx, ecx
	add rax, rbx	
	mov ecx, dword [rax]

	mov rax, rsi
	add rax, rbx
	mov edx, dword [rax]

	sub ecx, edx
	or ecx, res
	mov res, ecx

	inc i
	jmp .loop_start

.loop_end:
  mov eax, res

  restore i
  restore res

  mov rsp, rbp
  pop rbp
	ret
 

section '.data' data readable writeable

xsdp_pointer dq 0

error_msg_xsdp	du "fatal error: could not find xsdp structure pointer!", 0
