.code

;RCX, RDX, R8, R9 are used for integer and pointer arguments in that order left to right.

;extrn ?track@calls_collector@micro_profiler@@QEAAXAEBUcall_record@2@@Z:near
;extrn ?_instance@calls_collector@micro_profiler@@0V12@A:qword

extrn lpf_penter:near
extrn lpf_pexit:near

PUSHREGS	macro		; slides stack pointer 38h bytes down
	push	rax
	push	rcx
	push	rdx
	push	r8
	push	r9
	push	r10
	push	r11
endm

POPREGS	macro		; slides stack pointer 38h bytes up
	pop r11
	pop r10
	pop	r9
	pop	r8
	pop	rdx
	pop	rcx
	pop	rax
endm

RDTSC64	macro
	rdtsc
	shl	rdx, 20h
	or		rax, rdx
endm


;profile_enter	proc
_penter	proc
	PUSHREGS
	sub		rsp, 10h		;in release lpf_penter wants an extra 48 bytes for stack allocted for it
	lea		rax, [rsp+48h]	;get the stack address of the return address
	mov		rcx, [rax]		;load the return address
	call	lpf_penter		;jump to our 'function enter' proc
	add		rsp, 10h
	POPREGS
	ret
_penter	endp
;profile_enter	endp

;profile_exit	proc
_pexit	proc
	PUSHREGS
	sub		rsp, 10h		;in release lpf_penter wants an extra 48 bytes for stack allocted for it
	lea		rax, [rsp+58h]	;get the stack address of the return address
	mov		rcx, [rax]		;load the return address
	call	lpf_pexit		;jump to our 'function leave' proc
	add		rsp, 10h
	POPREGS
	ret
_pexit endp
;profile_exit	endp

end