HEXADECIMAL_BASE = 16


section '.text' code executable readable
;
; integer_to_hex_string
;
; this function converts integer number to hexadecimal string.
; Argument`rsi` contains the result.
; Implemented simple paper-pen method to convert decimal to hex number.
;
; params:
;   @rdi = 8-byte decimal number
;   @rsi = str array
;
; returns:
;   nothing
;
integer_to_hex_string:
  push rbp
  mov rbp, rsp

  sub rsp, 4    ; i

  mov dword [rbp - 4], 0

  cmp rdi, 0
  jne .else
  mov byte [rsi], '0'
  inc dword [rbp - 4]
  jmp .next

.else:

.loop_start:
  cmp rdi, 0
  je .next

  sub rsp, 4    ; r

  mov rax, rdi
  xor rdx, rdx
  push rsi
  mov rsi, HEXADECIMAL_BASE
  div rsi   ; divide rdx:rax by divisor in rsi. Remainder (modulo) is now in rdx
  pop rsi

  mov dword [rbp - 8], edx

  cmp dword [rbp - 8], 10
  jae .else_2

  ; '0' to '9'
  mov eax, 48
  add eax, dword [rbp - 8] 

  mov ebx, dword [rbp - 4]
  add rbx, rsi

  mov byte [rbx], al
  inc dword [rbp - 4]
  jmp .update_number

.else_2:

  ; 'a' to 'f'
  mov eax, 87
  add eax, dword [rbp - 8] 

  mov ebx, dword [rbp - 4]
  add rbx, rsi

  mov byte [rbx], al
  inc dword [rbp - 4]

.update_number:
  mov rax, rdi
  xor rdx, rdx
  mov rcx, HEXADECIMAL_BASE
  div rcx   ; divide rdx:rax by rcx. Quotient in rax
  mov rdi, rax

  jmp .loop_start

.next:
  ; append hex notation "0x" in reverse order
  mov rax, rsi
  mov ebx, dword [rbp - 4]
  add rax, rbx
  mov byte [rax], 'x'
  inc dword [rbp - 4]

  mov rax, rsi
  mov ebx, dword [rbp - 4]
  add rax, rbx
  mov byte [rax], '0'
  inc dword [rbp - 4]

  ; append null character

  mov rax, rsi
  mov ebx, dword [rbp - 4]
  add rax, rbx
  mov byte [rax], NULL_CHARACTER

  ; reverse the string
  mov rdi, rsi
  mov esi, dword [rbp - 4]
  call reverse_string

  mov rsp, rbp
  pop rbp
  ret 


; a utility function to reverse a string
; args:
;   @rdi = str array
;   @rsi = string length
;
; returns:
;   nothing
reverse_string:
  push rbp
  mov rbp, rsp

  sub rsp, 4    ; start
  sub rsp, 4    ; end


  mov dword [rbp - 4], 0
  mov rax, rsi
  dec rax
  mov dword [rbp - 8], eax

.loop_start:
  mov eax, dword [rbp - 4]
  mov ebx, dword [rbp - 8]
  cmp eax, ebx    ; start < end
  jae .end

  sub rsp, 1    ; temp 

  mov eax, dword [rbp - 4]
  add rax, rdi
  mov bl, byte [rax]
  mov byte [rbp - 9], bl

  mov ebx, dword [rbp - 8]
  add rbx, rdi
  mov cl, byte [rbx]
  mov byte [rax], cl

  mov cl, byte [rbp - 9]
  mov byte [rbx], cl

  dec dword [rbp - 8]
  inc dword [rbp - 4]

  jmp .loop_start

.end:
  mov rsp, rbp
  pop rbp
  ret
