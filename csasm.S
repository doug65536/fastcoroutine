// .asm

// branch hints
#define nottaken  .byte 0x2E
#define taken     .byte 0x3E

.text

// EXTERN ScheduleTask:PROC

// Returns a pointer to the thread environment block for this thread
.globl get_TEB
get_TEB:
  // Actual linear address of TEB is stored at gs:[0x30]
  gs movq 0x30,%rax
  ret

// SwitchToNextTask_Frame STRUCT
// This is the structure of a saved context
.struct 0
c_rbp    : .quad 0
c_xmm6   : .quad 0
c_xmm6hi : .quad 0
c_xmm7   : .quad 0
c_xmm7hi : .quad 0
c_xmm8   : .quad 0
c_xmm8hi : .quad 0
c_xmm9   : .quad 0
c_xmm9hi : .quad 0
c_xmm10  : .quad 0
c_xmm10hi: .quad 0
c_xmm11  : .quad 0
c_xmm11hi: .quad 0
c_xmm12  : .quad 0
c_xmm12hi: .quad 0
c_xmm13  : .quad 0
c_xmm13hi: .quad 0
c_xmm14  : .quad 0
c_xmm14hi: .quad 0
c_xmm15  : .quad 0
c_xmm15hi: .quad 0
c_r15    : .quad 0
c_r14    : .quad 0
c_r13    : .quad 0
c_r12    : .quad 0
c_rdi    : .quad 0
c_rsi    : .quad 0
c_rbx    : .quad 0
SwitchToNextTask_Frame_size:
.text

// Note: on amd64 ABI, we're allowed to clobber
// rax, rcx, rdx, r8-r11, xmm0-xmm5

//.globl SwitchToNextTask
//SwitchToNextTask:
//  // Calculate a pointer to the beginning of the context save area
//  lea -SwitchToNextTask_Frame_size(%rsp),%rcx
//
//  // Adjust the stack pointer to allocate room for the context and
//  // also reserve 4 quadwords of spill space for the called function
//  // below, plus 8 more bytes to realign the stack pointer
//  sub $(4*8 + 8 + SwitchToNextTask_Frame_size),%rsp
//
//  // Save context
//  movdqa %xmm6 ,c_xmm6 (%rcx)
//  movdqa %xmm7 ,c_xmm7 (%rcx)
//  movdqa %xmm8 ,c_xmm8 (%rcx)
//  movdqa %xmm9 ,c_xmm9 (%rcx)
//  movdqa %xmm10,c_xmm10(%rcx)
//  movdqa %xmm11,c_xmm11(%rcx)
//  movdqa %xmm12,c_xmm12(%rcx)
//  movdqa %xmm13,c_xmm13(%rcx)
//  movdqa %xmm14,c_xmm14(%rcx)
//  movdqa %xmm15,c_xmm15(%rcx)
//
//  mov %r15, c_r15(%rcx)
//  mov %r14, c_r14(%rcx)
//  mov %r13, c_r13(%rcx)
//  mov %r12, c_r12(%rcx)
//  mov %rdi, c_rdi(%rcx)
//  mov %rsi, c_rsi(%rcx)
//  mov %rbp, c_rbp(%rcx)
//  mov %rbx, c_rbx(%rcx)
//
//  // We already provided 4 quadwords of spill space above
//
//  // rcx has register argument already
//  call ScheduleTask
//  // Pointer to new context returned in %rax
//
//  // Schedule fetch of return address and new stack
//  // pointer value ahead of time.
//  mov SwitchToNextTask_Frame_size(%rax),%rcx
//  lea 8+SwitchToNextTask_Frame_size(%rax),%rdx
//
//  // Restore context
//  movdqa c_xmm6 (%rax),%xmm6
//  movdqa c_xmm7 (%rax),%xmm7
//  movdqa c_xmm8 (%rax),%xmm8
//  movdqa c_xmm9 (%rax),%xmm9
//  movdqa c_xmm10(%rax),%xmm10
//  movdqa c_xmm11(%rax),%xmm11
//  movdqa c_xmm12(%rax),%xmm12
//  movdqa c_xmm13(%rax),%xmm13
//  movdqa c_xmm14(%rax),%xmm14
//  movdqa c_xmm15(%rax),%xmm15
//
//  mov c_r15(%rax),%r15
//  mov c_r14(%rax),%r14
//  mov c_r13(%rax),%r13
//  mov c_r12(%rax),%r12
//  mov c_rdi(%rax),%rdi
//  mov c_rsi(%rax),%rsi
//  mov c_rbp(%rax),%rbp
//  mov c_rbx(%rax),%rbx
//
//  mov %rdx,%rsp
//  jmp *%rcx

// Explicitly switch from a specified task to a specified task
// Takes two register arguments, the incoming context pointer,
// and a pointer to the outgoing task stack pointer
// incoming task context pointer passed in %rcx
// pointer to outgoing task context pointer passed in %rdx
.globl SwitchToTask
SwitchToTask:
  // Calculate a pointer to the beginning of the context save area
  lea -SwitchToNextTask_Frame_size(%rsp),%rax

  // Adjust the stack pointer to allocate room for the context and
  // also reserve 4 quadwords of spill space for the called function
  // below, plus 8 more bytes to realign the stack pointer
  sub $(4*8 + 8 + SwitchToNextTask_Frame_size),%rsp

  // Save context
  movdqa %xmm6 ,c_xmm6 (%rax)
  movdqa %xmm7 ,c_xmm7 (%rax)
  movdqa %xmm8 ,c_xmm8 (%rax)
  movdqa %xmm9 ,c_xmm9 (%rax)
  movdqa %xmm10,c_xmm10(%rax)
  movdqa %xmm11,c_xmm11(%rax)
  movdqa %xmm12,c_xmm12(%rax)
  movdqa %xmm13,c_xmm13(%rax)
  movdqa %xmm14,c_xmm14(%rax)
  movdqa %xmm15,c_xmm15(%rax)

  mov %r15, c_r15(%rax)
  mov %r14, c_r14(%rax)
  mov %r13, c_r13(%rax)
  mov %r12, c_r12(%rax)
  mov %rdi, c_rdi(%rax)
  mov %rsi, c_rsi(%rax)
  mov %rbp, c_rbp(%rax)
  mov %rbx, c_rbx(%rax)

  // Store the outgoing task context pointer
  mov %rax,(%rdx)

  // Load the incoming task stack pointer
  mov %rcx,%rax

  // Schedule fetch of return address and new stack
  // pointer value ahead of time.
  mov SwitchToNextTask_Frame_size(%rax),%rcx
  lea 8+SwitchToNextTask_Frame_size(%rax),%rdx

  // Restore context
  movdqa c_xmm6 (%rax),%xmm6
  movdqa c_xmm7 (%rax),%xmm7
  movdqa c_xmm8 (%rax),%xmm8
  movdqa c_xmm9 (%rax),%xmm9
  movdqa c_xmm10(%rax),%xmm10
  movdqa c_xmm11(%rax),%xmm11
  movdqa c_xmm12(%rax),%xmm12
  movdqa c_xmm13(%rax),%xmm13
  movdqa c_xmm14(%rax),%xmm14
  movdqa c_xmm15(%rax),%xmm15

  mov c_r15(%rax),%r15
  mov c_r14(%rax),%r14
  mov c_r13(%rax),%r13
  mov c_r12(%rax),%r12
  mov c_rdi(%rax),%rdi
  mov c_rsi(%rax),%rsi
  mov c_rbp(%rax),%rbp
  mov c_rbx(%rax),%rbx

  mov %rdx,%rsp
  jmp *%rcx

// Stub loads register arguments from initial saved registers
.globl StartNewTask
StartNewTask:
  // Reserve spill space
  sub $(8*4),%rsp
	mov %r12,%rcx
	mov %r13,%rdx
	mov %r14,%r8
	mov %r15,%r9
	movd %rcx,%xmm0
	movd %rdx,%xmm1
	movd %r8, %xmm2
	movd %r9, %xmm3
	call *%rbp
	jmp task_terminate
