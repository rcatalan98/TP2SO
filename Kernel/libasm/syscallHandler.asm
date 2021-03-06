;
; syscallHandler.asm: Nexo entre dispatcher de excepciones (int80h) y syscalls (pasadas por rax).
;
GLOBAL _syscallHandler
EXTERN syscallDispatcher

SECTION .text

%macro pushState 0
	; ovbiamos rax.
	push rbx
	push rcx
	push rdx
	push rbp
	push rdi
	push rsi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
%endmacro

%macro popState 0
	; obviamos rax.
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rbp
	pop rdx
	pop rcx
	pop rbx
%endmacro

_syscallHandler:
    pushState
	call syscallDispatcher
	popState
	iretq