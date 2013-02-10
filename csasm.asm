; .asm

INCLUDE ksamd64.inc

; Note: MASM sucks

; branch hints
nottaken	textequ <byte 2Eh>
taken		textequ <byte 3Eh>

_text SEGMENT PARA 'code'

EXTERN ScheduleTask:PROC

get_TEB PROC
	mov rax,gs:[30h]
	ret
get_TEB ENDP

SwitchToNextTask_Frame STRUCT
  c_r15 qword ?
  c_xmm6 qword ?
  c_xmm6hi qword ?
  c_xmm7 qword ?
  c_xmm7hi qword ?
  c_xmm8 qword ?
  c_xmm8hi qword ?
  c_xmm9 qword ?
  c_xmm9hi qword ?
  c_xmm10 qword ?
  c_xmm10hi qword ?
  c_xmm11 qword ?
  c_xmm11hi qword ?
  c_xmm12 qword ?
  c_xmm12hi qword ?
  c_xmm13 qword ?
  c_xmm13hi qword ?
  c_xmm14 qword ?
  c_xmm14hi qword ?
  c_xmm15 qword ?
  c_xmm15hi qword ?
  c_r14 qword ?
  c_r13 qword ?
  c_r12 qword ?
  c_rdi qword ?
  c_rsi qword ?
  c_rbp qword ?
  c_rbx qword ?
SwitchToNextTask_Frame ENDS

; Note: on amd64 ABI, we're allowed to clobber
; rax, rcx, rdx, r8-r11, xmm0-xmm5

SwitchToNextTask PROC
	lea rcx,[rsp - (sizeof SwitchToNextTask_Frame)]
	sub rsp,(4*8) + (sizeof SwitchToNextTask_Frame) + 8

	; Save context
	movdqa xmmword ptr [rcx + SwitchToNextTask_Frame.c_xmm6 ],xmm6
	movdqa xmmword ptr [rcx + SwitchToNextTask_Frame.c_xmm7 ],xmm7
	movdqa xmmword ptr [rcx + SwitchToNextTask_Frame.c_xmm8 ],xmm8
	movdqa xmmword ptr [rcx + SwitchToNextTask_Frame.c_xmm9 ],xmm9
	movdqa xmmword ptr [rcx + SwitchToNextTask_Frame.c_xmm10],xmm10
	movdqa xmmword ptr [rcx + SwitchToNextTask_Frame.c_xmm11],xmm11
	movdqa xmmword ptr [rcx + SwitchToNextTask_Frame.c_xmm12],xmm12
	movdqa xmmword ptr [rcx + SwitchToNextTask_Frame.c_xmm13],xmm13
	movdqa xmmword ptr [rcx + SwitchToNextTask_Frame.c_xmm14],xmm14
	movdqa xmmword ptr [rcx + SwitchToNextTask_Frame.c_xmm15],xmm15

	mov [rcx + SwitchToNextTask_Frame.c_r15],r15
	mov [rcx + SwitchToNextTask_Frame.c_r14],r14
	mov [rcx + SwitchToNextTask_Frame.c_r13],r13
	mov [rcx + SwitchToNextTask_Frame.c_r12],r12
	mov [rcx + SwitchToNextTask_Frame.c_rdi],rdi
	mov [rcx + SwitchToNextTask_Frame.c_rsi],rsi
	mov [rcx + SwitchToNextTask_Frame.c_rbp],rbp
	mov [rcx + SwitchToNextTask_Frame.c_rbx],rbx

	; We already provided 4 quadwords of spill space above

	; rcx has register argument already
	call ScheduleTask
	;mov rax,rcx

	; Get jump target loaded ASAP, in case it speculatively 
	; make it to the jmp early, it might help
	mov rcx,[rax + (sizeof SwitchToNextTask_Frame)]

	; Compute new stack pointer value ASAP
	lea rdx,[rax + (4*8) + (sizeof SwitchToNextTask_Frame) + 8]

	; Restore context
	movdqa xmm6 ,xmmword ptr [rax + SwitchToNextTask_Frame.c_xmm6]
	movdqa xmm7 ,xmmword ptr [rax + SwitchToNextTask_Frame.c_xmm7]
	movdqa xmm8 ,xmmword ptr [rax + SwitchToNextTask_Frame.c_xmm8]
	movdqa xmm9 ,xmmword ptr [rax + SwitchToNextTask_Frame.c_xmm9]
	movdqa xmm10,xmmword ptr [rax + SwitchToNextTask_Frame.c_xmm10]
	movdqa xmm11,xmmword ptr [rax + SwitchToNextTask_Frame.c_xmm11]
	movdqa xmm12,xmmword ptr [rax + SwitchToNextTask_Frame.c_xmm12]
	movdqa xmm13,xmmword ptr [rax + SwitchToNextTask_Frame.c_xmm13]
	movdqa xmm14,xmmword ptr [rax + SwitchToNextTask_Frame.c_xmm14]
	movdqa xmm15,xmmword ptr [rax + SwitchToNextTask_Frame.c_xmm15]
	mov r15,[rax + SwitchToNextTask_Frame.c_r15]
	mov r14,[rax + SwitchToNextTask_Frame.c_r14]
	mov r13,[rax + SwitchToNextTask_Frame.c_r13]
	mov r12,[rax + SwitchToNextTask_Frame.c_r12]
	mov rdi,[rax + SwitchToNextTask_Frame.c_rdi]
	mov rsi,[rax + SwitchToNextTask_Frame.c_rsi]
	mov rbp,[rax + SwitchToNextTask_Frame.c_rbp]
	mov rbx,[rax + SwitchToNextTask_Frame.c_rbx]

	mov rsp,rdx
	jmp rcx
SwitchToNextTask ENDP

PUBLIC SwitchToTask
SwitchToTask PROC
	push rbx
	push rbp
	push rsi
	push rdi
	push r12
	push r13
	push r14
	push r15

	; Save stack pointer of outgoing task
	mov [rdx],rsp

	; Switch to new stack
	mov rsp,rcx

	pop r15
	pop r14
	pop r13
	pop r12
	pop rdi
	pop rsi
	pop rbp
	pop rbx
	pop rax
	jmp rax
SwitchToTask ENDP

; Stub loads register arguments from initial saved registers
PUBLIC StartNewTask
StartNewTask PROC
	mov rcx,r12
	mov rdx,r13
	mov r8,r14
	mov r9,r15
	movd xmm0,rcx
	movd xmm1,rdx
	movd xmm2,r8
	movd xmm3,r9
	jmp rbp
StartNewTask ENDP

_text ENDS

END
