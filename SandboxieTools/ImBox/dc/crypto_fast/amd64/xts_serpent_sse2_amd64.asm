; this code compiled with Intel® C++ Compiler Version 11.1.835.200
;
; Disassembly of file: xts_serpent_sse2.obj
; Mon May 10 08:16:59 2010
; Mode: 64 bits
; Syntax: YASM/NASM
; Instruction set: SSE2, x64

default rel

global xts_serpent_sse2_encrypt
global xts_serpent_sse2_decrypt
global xts_serpent_sse2_available

extern serpent256_encrypt				; near


SECTION .text	align=16 execute			; section number 2, code

xts_serpent_sse2_encrypt:; Function begin
	push	r15					; 0000 _ 41: 57
	push	r14					; 0002 _ 41: 56
	push	r13					; 0004 _ 41: 55
	push	r12					; 0006 _ 41: 54
	push	rbp					; 0008 _ 55
	sub	rsp, 160				; 0009 _ 48: 81. EC, 000000A0
	mov	rax, qword [rsp+0F0H]			; 0010 _ 48: 8B. 84 24, 000000F0
	movaps	oword [rsp+70H], xmm6			; 0018 _ 0F 29. 74 24, 70
	movaps	oword [rsp+60H], xmm7			; 001D _ 0F 29. 7C 24, 60
	movaps	oword [rsp+50H], xmm8			; 0022 _ 44: 0F 29. 44 24, 50
	movaps	oword [rsp+40H], xmm9			; 0028 _ 44: 0F 29. 4C 24, 40
	movaps	oword [rsp+30H], xmm10			; 002E _ 44: 0F 29. 54 24, 30
	movaps	oword [rsp+20H], xmm11			; 0034 _ 44: 0F 29. 5C 24, 20
	shr	r9, 9					; 003A _ 49: C1. E9, 09
	mov	qword [rsp+80H], r9			; 003E _ 4C: 89. 8C 24, 00000080
	lea	r9, [rax+2710H] 			; 0046 _ 4C: 8D. 88, 00002710
	mov	qword [rsp+88H], 0			; 004D _ 48: C7. 84 24, 00000088, 00000000
	mov	r10d, 135				; 0059 _ 41: BA, 00000087
	mov	r12, rax				; 005F _ 49: 89. C4
	movd	xmm1, r10d				; 0062 _ 66 41: 0F 6E. CA
	movdqa	xmm7, xmm1				; 0067 _ 66: 0F 6F. F9
	mov	rbp, rdx				; 006B _ 48: 89. D5
	mov	r13, rcx				; 006E _ 49: 89. CD
	mov	r14, r8 				; 0071 _ 4D: 89. C6
	mov	r15, r9 				; 0074 _ 4D: 89. CF
	pcmpeqd xmm0, xmm0				; 0077 _ 66: 0F 76. C0
	movdqa	xmm6, xmm0				; 007B _ 66: 0F 6F. F0
	jmp	?_002					; 007F _ EB, 0A

?_001:	movdqa	oword [rsp+90H], xmm9			; 0081 _ 66 44: 0F 7F. 8C 24, 00000090
?_002:	inc	qword [rsp+80H] 			; 008B _ 48: FF. 84 24, 00000080
	lea	rcx, [rsp+80H]				; 0093 _ 48: 8D. 8C 24, 00000080
	mov	r8, r15 				; 009B _ 4D: 89. F8
	lea	rdx, [rsp+90H]				; 009E _ 48: 8D. 94 24, 00000090
	call	serpent256_encrypt			; 00A6 _ E8, 00000000(rel)
	movdqa	xmm9, oword [rsp+90H]			; 00AB _ 66 44: 0F 6F. 8C 24, 00000090
	xor	r10d, r10d				; 00B5 _ 45: 33. D2
?_003:	movdqa	xmm5, xmm9				; 00B8 _ 66 41: 0F 6F. E9
	movdqa	xmm4, xmm9				; 00BD _ 66 41: 0F 6F. E1
	movdqa	xmm3, xmm9				; 00C2 _ 66 41: 0F 6F. D9
	movdqu	xmm11, oword [r13]			; 00C7 _ F3 45: 0F 6F. 5D, 00
	movdqu	xmm1, oword [r13+20H]			; 00CD _ F3 41: 0F 6F. 4D, 20
	psllq	xmm5, 1 				; 00D3 _ 66: 0F 73. F5, 01
	pslldq	xmm4, 8 				; 00D8 _ 66: 0F 73. FC, 08
	psrldq	xmm4, 7 				; 00DD _ 66: 0F 73. DC, 07
	psrlq	xmm4, 7 				; 00E2 _ 66: 0F 73. D4, 07
	por	xmm5, xmm4				; 00E7 _ 66: 0F EB. EC
	psraw	xmm3, 8 				; 00EB _ 66: 0F 71. E3, 08
	psrldq	xmm3, 15				; 00F0 _ 66: 0F 73. DB, 0F
	pand	xmm3, xmm7				; 00F5 _ 66: 0F DB. DF
	pxor	xmm5, xmm3				; 00F9 _ 66: 0F EF. EB
	movdqa	xmm4, xmm5				; 00FD _ 66: 0F 6F. E5
	movdqa	xmm8, xmm5				; 0101 _ 66 44: 0F 6F. C5
	movdqa	xmm10, xmm5				; 0106 _ 66 44: 0F 6F. D5
	psllq	xmm4, 1 				; 010B _ 66: 0F 73. F4, 01
	pslldq	xmm8, 8 				; 0110 _ 66 41: 0F 73. F8, 08
	psrldq	xmm8, 7 				; 0116 _ 66 41: 0F 73. D8, 07
	psrlq	xmm8, 7 				; 011C _ 66 41: 0F 73. D0, 07
	por	xmm4, xmm8				; 0122 _ 66 41: 0F EB. E0
	psraw	xmm10, 8				; 0127 _ 66 41: 0F 71. E2, 08
	psrldq	xmm10, 15				; 012D _ 66 41: 0F 73. DA, 0F
	pand	xmm10, xmm7				; 0133 _ 66 44: 0F DB. D7
	pxor	xmm4, xmm10				; 0138 _ 66 41: 0F EF. E2
	movdqa	xmm3, xmm4				; 013D _ 66: 0F 6F. DC
	movdqa	xmm8, xmm4				; 0141 _ 66 44: 0F 6F. C4
	movdqa	xmm2, xmm4				; 0146 _ 66: 0F 6F. D4
	psllq	xmm3, 1 				; 014A _ 66: 0F 73. F3, 01
	pslldq	xmm8, 8 				; 014F _ 66 41: 0F 73. F8, 08
	psrldq	xmm8, 7 				; 0155 _ 66 41: 0F 73. D8, 07
	psrlq	xmm8, 7 				; 015B _ 66 41: 0F 73. D0, 07
	por	xmm3, xmm8				; 0161 _ 66 41: 0F EB. D8
	movdqu	xmm8, oword [r13+10H]			; 0166 _ F3 45: 0F 6F. 45, 10
	psraw	xmm2, 8 				; 016C _ 66: 0F 71. E2, 08
	psrldq	xmm2, 15				; 0171 _ 66: 0F 73. DA, 0F
	pand	xmm2, xmm7				; 0176 _ 66: 0F DB. D7
	pxor	xmm3, xmm2				; 017A _ 66: 0F EF. DA
	movdqu	xmm2, oword [r13+30H]			; 017E _ F3 41: 0F 6F. 55, 30
	pxor	xmm11, xmm9				; 0184 _ 66 45: 0F EF. D9
	movdqa	xmm10, xmm11				; 0189 _ 66 45: 0F 6F. D3
	pxor	xmm8, xmm5				; 018E _ 66 44: 0F EF. C5
	pxor	xmm1, xmm4				; 0193 _ 66: 0F EF. CC
	movdqa	xmm0, xmm1				; 0197 _ 66: 0F 6F. C1
	pxor	xmm2, xmm3				; 019B _ 66: 0F EF. D3
	punpckldq xmm10, xmm8				; 019F _ 66 45: 0F 62. D0
	punpckldq xmm0, xmm2				; 01A4 _ 66: 0F 62. C2
	punpckhdq xmm11, xmm8				; 01A8 _ 66 45: 0F 6A. D8
	movdqa	xmm8, xmm11				; 01AD _ 66 45: 0F 6F. C3
	punpckhdq xmm1, xmm2				; 01B2 _ 66: 0F 6A. CA
	movdqa	xmm2, xmm10				; 01B6 _ 66 41: 0F 6F. D2
	punpckhqdq xmm10, xmm0				; 01BB _ 66 44: 0F 6D. D0
	punpcklqdq xmm8, xmm1				; 01C0 _ 66 44: 0F 6C. C1
	punpcklqdq xmm2, xmm0				; 01C5 _ 66: 0F 6C. D0
	punpckhqdq xmm11, xmm1				; 01C9 _ 66 44: 0F 6D. D9
	movd	xmm0, dword [r12+1280H] 		; 01CE _ 66 41: 0F 6E. 84 24, 00001280
	pshufd	xmm0, xmm0, 0				; 01D8 _ 66: 0F 70. C0, 00
	pxor	xmm2, xmm0				; 01DD _ 66: 0F EF. D0
	movd	xmm0, dword [r12+1284H] 		; 01E1 _ 66 41: 0F 6E. 84 24, 00001284
	pshufd	xmm0, xmm0, 0				; 01EB _ 66: 0F 70. C0, 00
	pxor	xmm10, xmm0				; 01F0 _ 66 44: 0F EF. D0
	movdqa	xmm1, xmm10				; 01F5 _ 66 41: 0F 6F. CA
	movd	xmm0, dword [r12+1288H] 		; 01FA _ 66 41: 0F 6E. 84 24, 00001288
	pshufd	xmm0, xmm0, 0				; 0204 _ 66: 0F 70. C0, 00
	pxor	xmm8, xmm0				; 0209 _ 66 44: 0F EF. C0
	movd	xmm0, dword [r12+128CH] 		; 020E _ 66 41: 0F 6E. 84 24, 0000128C
	pshufd	xmm0, xmm0, 0				; 0218 _ 66: 0F 70. C0, 00
	pxor	xmm11, xmm0				; 021D _ 66 44: 0F EF. D8
	pxor	xmm11, xmm2				; 0222 _ 66 44: 0F EF. DA
	pand	xmm1, xmm11				; 0227 _ 66 41: 0F DB. CB
	pxor	xmm10, xmm8				; 022C _ 66 45: 0F EF. D0
	pxor	xmm1, xmm2				; 0231 _ 66: 0F EF. CA
	por	xmm2, xmm11				; 0235 _ 66 41: 0F EB. D3
	pxor	xmm2, xmm10				; 023A _ 66 41: 0F EF. D2
	pxor	xmm10, xmm11				; 023F _ 66 45: 0F EF. D3
	pxor	xmm11, xmm8				; 0244 _ 66 45: 0F EF. D8
	por	xmm8, xmm1				; 0249 _ 66 44: 0F EB. C1
	pxor	xmm8, xmm10				; 024E _ 66 45: 0F EF. C2
	pxor	xmm10, xmm6				; 0253 _ 66 44: 0F EF. D6
	por	xmm10, xmm1				; 0258 _ 66 44: 0F EB. D1
	pxor	xmm1, xmm11				; 025D _ 66 41: 0F EF. CB
	pxor	xmm1, xmm10				; 0262 _ 66 41: 0F EF. CA
	por	xmm11, xmm2				; 0267 _ 66 44: 0F EB. DA
	pxor	xmm1, xmm11				; 026C _ 66 41: 0F EF. CB
	movdqa	xmm0, xmm1				; 0271 _ 66: 0F 6F. C1
	pxor	xmm10, xmm11				; 0275 _ 66 45: 0F EF. D3
	movdqa	xmm11, xmm8				; 027A _ 66 45: 0F 6F. D8
	pslld	xmm0, 13				; 027F _ 66: 0F 72. F0, 0D
	psrld	xmm1, 19				; 0284 _ 66: 0F 72. D1, 13
	por	xmm0, xmm1				; 0289 _ 66: 0F EB. C1
	pslld	xmm11, 3				; 028D _ 66 41: 0F 72. F3, 03
	psrld	xmm8, 29				; 0293 _ 66 41: 0F 72. D0, 1D
	por	xmm11, xmm8				; 0299 _ 66 45: 0F EB. D8
	movdqa	xmm8, xmm0				; 029E _ 66 44: 0F 6F. C0
	pxor	xmm10, xmm0				; 02A3 _ 66 44: 0F EF. D0
	pxor	xmm10, xmm11				; 02A8 _ 66 45: 0F EF. D3
	movdqa	xmm1, xmm10				; 02AD _ 66 41: 0F 6F. CA
	pxor	xmm2, xmm11				; 02B2 _ 66 41: 0F EF. D3
	pslld	xmm8, 3 				; 02B7 _ 66 41: 0F 72. F0, 03
	pxor	xmm2, xmm8				; 02BD _ 66 41: 0F EF. D0
	movdqa	xmm8, xmm2				; 02C2 _ 66 44: 0F 6F. C2
	pslld	xmm1, 1 				; 02C7 _ 66: 0F 72. F1, 01
	psrld	xmm10, 31				; 02CC _ 66 41: 0F 72. D2, 1F
	por	xmm1, xmm10				; 02D2 _ 66 41: 0F EB. CA
	movdqa	xmm10, xmm1				; 02D7 _ 66 44: 0F 6F. D1
	pslld	xmm8, 7 				; 02DC _ 66 41: 0F 72. F0, 07
	psrld	xmm2, 25				; 02E2 _ 66: 0F 72. D2, 19
	por	xmm8, xmm2				; 02E7 _ 66 44: 0F EB. C2
	pxor	xmm0, xmm1				; 02EC _ 66: 0F EF. C1
	pxor	xmm0, xmm8				; 02F0 _ 66 41: 0F EF. C0
	pxor	xmm11, xmm8				; 02F5 _ 66 45: 0F EF. D8
	pslld	xmm10, 7				; 02FA _ 66 41: 0F 72. F2, 07
	pxor	xmm11, xmm10				; 0300 _ 66 45: 0F EF. DA
	movdqa	xmm10, xmm0				; 0305 _ 66 44: 0F 6F. D0
	movdqa	xmm2, xmm11				; 030A _ 66 41: 0F 6F. D3
	psrld	xmm0, 27				; 030F _ 66: 0F 72. D0, 1B
	pslld	xmm10, 5				; 0314 _ 66 41: 0F 72. F2, 05
	por	xmm10, xmm0				; 031A _ 66 44: 0F EB. D0
	pslld	xmm2, 22				; 031F _ 66: 0F 72. F2, 16
	psrld	xmm11, 10				; 0324 _ 66 41: 0F 72. D3, 0A
	por	xmm2, xmm11				; 032A _ 66 41: 0F EB. D3
	movd	xmm11, dword [r12+1290H]		; 032F _ 66 45: 0F 6E. 9C 24, 00001290
	pshufd	xmm11, xmm11, 0 			; 0339 _ 66 45: 0F 70. DB, 00
	pxor	xmm10, xmm11				; 033F _ 66 45: 0F EF. D3
	pxor	xmm10, xmm6				; 0344 _ 66 44: 0F EF. D6
	movd	xmm11, dword [r12+1294H]		; 0349 _ 66 45: 0F 6E. 9C 24, 00001294
	pshufd	xmm11, xmm11, 0 			; 0353 _ 66 45: 0F 70. DB, 00
	pxor	xmm1, xmm11				; 0359 _ 66 41: 0F EF. CB
	movd	xmm11, dword [r12+1298H]		; 035E _ 66 45: 0F 6E. 9C 24, 00001298
	pshufd	xmm11, xmm11, 0 			; 0368 _ 66 45: 0F 70. DB, 00
	pxor	xmm2, xmm11				; 036E _ 66 41: 0F EF. D3
	pxor	xmm2, xmm6				; 0373 _ 66: 0F EF. D6
	movd	xmm11, dword [r12+129CH]		; 0377 _ 66 45: 0F 6E. 9C 24, 0000129C
	pshufd	xmm11, xmm11, 0 			; 0381 _ 66 45: 0F 70. DB, 00
	pxor	xmm8, xmm11				; 0387 _ 66 45: 0F EF. C3
	movdqa	xmm11, xmm10				; 038C _ 66 45: 0F 6F. DA
	pand	xmm11, xmm1				; 0391 _ 66 44: 0F DB. D9
	pxor	xmm2, xmm11				; 0396 _ 66 41: 0F EF. D3
	por	xmm11, xmm8				; 039B _ 66 45: 0F EB. D8
	pxor	xmm8, xmm2				; 03A0 _ 66 44: 0F EF. C2
	pxor	xmm1, xmm11				; 03A5 _ 66 41: 0F EF. CB
	pxor	xmm11, xmm10				; 03AA _ 66 45: 0F EF. DA
	por	xmm10, xmm1				; 03AF _ 66 44: 0F EB. D1
	pxor	xmm1, xmm8				; 03B4 _ 66 41: 0F EF. C8
	por	xmm2, xmm11				; 03B9 _ 66 41: 0F EB. D3
	pand	xmm2, xmm10				; 03BE _ 66 41: 0F DB. D2
	movdqa	xmm0, xmm2				; 03C3 _ 66: 0F 6F. C2
	pxor	xmm11, xmm1				; 03C7 _ 66 44: 0F EF. D9
	pand	xmm1, xmm2				; 03CC _ 66: 0F DB. CA
	pxor	xmm1, xmm11				; 03D0 _ 66 41: 0F EF. CB
	pand	xmm11, xmm2				; 03D5 _ 66 44: 0F DB. DA
	pxor	xmm10, xmm11				; 03DA _ 66 45: 0F EF. D3
	pslld	xmm0, 13				; 03DF _ 66: 0F 72. F0, 0D
	psrld	xmm2, 19				; 03E4 _ 66: 0F 72. D2, 13
	por	xmm0, xmm2				; 03E9 _ 66: 0F EB. C2
	movdqa	xmm2, xmm8				; 03ED _ 66 41: 0F 6F. D0
	psrld	xmm8, 29				; 03F2 _ 66 41: 0F 72. D0, 1D
	pxor	xmm10, xmm0				; 03F8 _ 66 44: 0F EF. D0
	pslld	xmm2, 3 				; 03FD _ 66: 0F 72. F2, 03
	por	xmm2, xmm8				; 0402 _ 66 41: 0F EB. D0
	movdqa	xmm8, xmm0				; 0407 _ 66 44: 0F 6F. C0
	pxor	xmm10, xmm2				; 040C _ 66 44: 0F EF. D2
	pxor	xmm1, xmm2				; 0411 _ 66: 0F EF. CA
	pslld	xmm8, 3 				; 0415 _ 66 41: 0F 72. F0, 03
	pxor	xmm1, xmm8				; 041B _ 66 41: 0F EF. C8
	movdqa	xmm8, xmm10				; 0420 _ 66 45: 0F 6F. C2
	psrld	xmm10, 31				; 0425 _ 66 41: 0F 72. D2, 1F
	pslld	xmm8, 1 				; 042B _ 66 41: 0F 72. F0, 01
	por	xmm8, xmm10				; 0431 _ 66 45: 0F EB. C2
	movdqa	xmm10, xmm1				; 0436 _ 66 44: 0F 6F. D1
	movdqa	xmm11, xmm8				; 043B _ 66 45: 0F 6F. D8
	psrld	xmm1, 25				; 0440 _ 66: 0F 72. D1, 19
	pslld	xmm10, 7				; 0445 _ 66 41: 0F 72. F2, 07
	por	xmm10, xmm1				; 044B _ 66 44: 0F EB. D1
	pxor	xmm0, xmm8				; 0450 _ 66 41: 0F EF. C0
	pxor	xmm0, xmm10				; 0455 _ 66 41: 0F EF. C2
	pxor	xmm2, xmm10				; 045A _ 66 41: 0F EF. D2
	pslld	xmm11, 7				; 045F _ 66 41: 0F 72. F3, 07
	pxor	xmm2, xmm11				; 0465 _ 66 41: 0F EF. D3
	movdqa	xmm11, xmm0				; 046A _ 66 44: 0F 6F. D8
	psrld	xmm0, 27				; 046F _ 66: 0F 72. D0, 1B
	pslld	xmm11, 5				; 0474 _ 66 41: 0F 72. F3, 05
	por	xmm11, xmm0				; 047A _ 66 44: 0F EB. D8
	movdqa	xmm0, xmm2				; 047F _ 66: 0F 6F. C2
	psrld	xmm2, 10				; 0483 _ 66: 0F 72. D2, 0A
	pslld	xmm0, 22				; 0488 _ 66: 0F 72. F0, 16
	por	xmm0, xmm2				; 048D _ 66: 0F EB. C2
	movd	xmm2, dword [r12+12A0H] 		; 0491 _ 66 41: 0F 6E. 94 24, 000012A0
	pshufd	xmm2, xmm2, 0				; 049B _ 66: 0F 70. D2, 00
	pxor	xmm11, xmm2				; 04A0 _ 66 44: 0F EF. DA
	movdqa	xmm1, xmm11				; 04A5 _ 66 41: 0F 6F. CB
	movd	xmm2, dword [r12+12A4H] 		; 04AA _ 66 41: 0F 6E. 94 24, 000012A4
	pshufd	xmm2, xmm2, 0				; 04B4 _ 66: 0F 70. D2, 00
	pxor	xmm8, xmm2				; 04B9 _ 66 44: 0F EF. C2
	movd	xmm2, dword [r12+12A8H] 		; 04BE _ 66 41: 0F 6E. 94 24, 000012A8
	pshufd	xmm2, xmm2, 0				; 04C8 _ 66: 0F 70. D2, 00
	pxor	xmm0, xmm2				; 04CD _ 66: 0F EF. C2
	pand	xmm1, xmm0				; 04D1 _ 66: 0F DB. C8
	pxor	xmm0, xmm8				; 04D5 _ 66 41: 0F EF. C0
	movd	xmm2, dword [r12+12ACH] 		; 04DA _ 66 41: 0F 6E. 94 24, 000012AC
	pshufd	xmm2, xmm2, 0				; 04E4 _ 66: 0F 70. D2, 00
	pxor	xmm10, xmm2				; 04E9 _ 66 44: 0F EF. D2
	pxor	xmm1, xmm10				; 04EE _ 66 41: 0F EF. CA
	pxor	xmm0, xmm1				; 04F3 _ 66: 0F EF. C1
	por	xmm10, xmm11				; 04F7 _ 66 45: 0F EB. D3
	pxor	xmm10, xmm8				; 04FC _ 66 45: 0F EF. D0
	movdqa	xmm2, xmm10				; 0501 _ 66 41: 0F 6F. D2
	movdqa	xmm8, xmm0				; 0506 _ 66 44: 0F 6F. C0
	pxor	xmm11, xmm0				; 050B _ 66 44: 0F EF. D8
	por	xmm2, xmm11				; 0510 _ 66 41: 0F EB. D3
	pxor	xmm2, xmm1				; 0515 _ 66: 0F EF. D1
	pand	xmm1, xmm10				; 0519 _ 66 41: 0F DB. CA
	pxor	xmm11, xmm1				; 051E _ 66 44: 0F EF. D9
	pxor	xmm10, xmm2				; 0523 _ 66 44: 0F EF. D2
	pxor	xmm10, xmm11				; 0528 _ 66 45: 0F EF. D3
	movdqa	xmm1, xmm10				; 052D _ 66 41: 0F 6F. CA
	pxor	xmm11, xmm6				; 0532 _ 66 44: 0F EF. DE
	pslld	xmm8, 13				; 0537 _ 66 41: 0F 72. F0, 0D
	psrld	xmm0, 19				; 053D _ 66: 0F 72. D0, 13
	por	xmm8, xmm0				; 0542 _ 66 44: 0F EB. C0
	pslld	xmm1, 3 				; 0547 _ 66: 0F 72. F1, 03
	psrld	xmm10, 29				; 054C _ 66 41: 0F 72. D2, 1D
	por	xmm1, xmm10				; 0552 _ 66 41: 0F EB. CA
	movdqa	xmm10, xmm8				; 0557 _ 66 45: 0F 6F. D0
	pxor	xmm2, xmm8				; 055C _ 66 41: 0F EF. D0
	pxor	xmm2, xmm1				; 0561 _ 66: 0F EF. D1
	movdqa	xmm0, xmm2				; 0565 _ 66: 0F 6F. C2
	pxor	xmm11, xmm1				; 0569 _ 66 44: 0F EF. D9
	pslld	xmm10, 3				; 056E _ 66 41: 0F 72. F2, 03
	pxor	xmm11, xmm10				; 0574 _ 66 45: 0F EF. DA
	movdqa	xmm10, xmm11				; 0579 _ 66 45: 0F 6F. D3
	pslld	xmm0, 1 				; 057E _ 66: 0F 72. F0, 01
	psrld	xmm2, 31				; 0583 _ 66: 0F 72. D2, 1F
	por	xmm0, xmm2				; 0588 _ 66: 0F EB. C2
	pslld	xmm10, 7				; 058C _ 66 41: 0F 72. F2, 07
	psrld	xmm11, 25				; 0592 _ 66 41: 0F 72. D3, 19
	por	xmm10, xmm11				; 0598 _ 66 45: 0F EB. D3
	movdqa	xmm11, xmm0				; 059D _ 66 44: 0F 6F. D8
	pxor	xmm8, xmm0				; 05A2 _ 66 44: 0F EF. C0
	pxor	xmm8, xmm10				; 05A7 _ 66 45: 0F EF. C2
	movdqa	xmm2, xmm8				; 05AC _ 66 41: 0F 6F. D0
	pxor	xmm1, xmm10				; 05B1 _ 66 41: 0F EF. CA
	pslld	xmm11, 7				; 05B6 _ 66 41: 0F 72. F3, 07
	pxor	xmm1, xmm11				; 05BC _ 66 41: 0F EF. CB
	pslld	xmm2, 5 				; 05C1 _ 66: 0F 72. F2, 05
	psrld	xmm8, 27				; 05C6 _ 66 41: 0F 72. D0, 1B
	por	xmm2, xmm8				; 05CC _ 66 41: 0F EB. D0
	movdqa	xmm8, xmm1				; 05D1 _ 66 44: 0F 6F. C1
	movd	xmm11, dword [r12+12B0H]		; 05D6 _ 66 45: 0F 6E. 9C 24, 000012B0
	psrld	xmm1, 10				; 05E0 _ 66: 0F 72. D1, 0A
	pshufd	xmm11, xmm11, 0 			; 05E5 _ 66 45: 0F 70. DB, 00
	pslld	xmm8, 22				; 05EB _ 66 41: 0F 72. F0, 16
	por	xmm8, xmm1				; 05F1 _ 66 44: 0F EB. C1
	pxor	xmm2, xmm11				; 05F6 _ 66 41: 0F EF. D3
	movdqa	xmm1, xmm2				; 05FB _ 66: 0F 6F. CA
	movd	xmm11, dword [r12+12B4H]		; 05FF _ 66 45: 0F 6E. 9C 24, 000012B4
	pshufd	xmm11, xmm11, 0 			; 0609 _ 66 45: 0F 70. DB, 00
	pxor	xmm0, xmm11				; 060F _ 66 41: 0F EF. C3
	movd	xmm11, dword [r12+12B8H]		; 0614 _ 66 45: 0F 6E. 9C 24, 000012B8
	pshufd	xmm11, xmm11, 0 			; 061E _ 66 45: 0F 70. DB, 00
	pxor	xmm8, xmm11				; 0624 _ 66 45: 0F EF. C3
	movd	xmm11, dword [r12+12BCH]		; 0629 _ 66 45: 0F 6E. 9C 24, 000012BC
	pshufd	xmm11, xmm11, 0 			; 0633 _ 66 45: 0F 70. DB, 00
	pxor	xmm10, xmm11				; 0639 _ 66 45: 0F EF. D3
	por	xmm1, xmm10				; 063E _ 66 41: 0F EB. CA
	pxor	xmm10, xmm0				; 0643 _ 66 44: 0F EF. D0
	pand	xmm0, xmm2				; 0648 _ 66: 0F DB. C2
	pxor	xmm2, xmm8				; 064C _ 66 41: 0F EF. D0
	pxor	xmm8, xmm10				; 0651 _ 66 45: 0F EF. C2
	pand	xmm10, xmm1				; 0656 _ 66 44: 0F DB. D1
	por	xmm2, xmm0				; 065B _ 66: 0F EB. D0
	pxor	xmm10, xmm2				; 065F _ 66 44: 0F EF. D2
	pxor	xmm1, xmm0				; 0664 _ 66: 0F EF. C8
	pand	xmm2, xmm1				; 0668 _ 66: 0F DB. D1
	pxor	xmm0, xmm10				; 066C _ 66 41: 0F EF. C2
	pxor	xmm2, xmm8				; 0671 _ 66 41: 0F EF. D0
	por	xmm0, xmm1				; 0676 _ 66: 0F EB. C1
	pxor	xmm0, xmm8				; 067A _ 66 41: 0F EF. C0
	movdqa	xmm8, xmm0				; 067F _ 66 44: 0F 6F. C0
	pxor	xmm1, xmm10				; 0684 _ 66 41: 0F EF. CA
	por	xmm8, xmm10				; 0689 _ 66 45: 0F EB. C2
	pxor	xmm1, xmm8				; 068E _ 66 41: 0F EF. C8
	movdqa	xmm11, xmm1				; 0693 _ 66 44: 0F 6F. D9
	psrld	xmm1, 19				; 0698 _ 66: 0F 72. D1, 13
	pslld	xmm11, 13				; 069D _ 66 41: 0F 72. F3, 0D
	por	xmm11, xmm1				; 06A3 _ 66 44: 0F EB. D9
	movdqa	xmm1, xmm10				; 06A8 _ 66 41: 0F 6F. CA
	psrld	xmm10, 29				; 06AD _ 66 41: 0F 72. D2, 1D
	pxor	xmm0, xmm11				; 06B3 _ 66 41: 0F EF. C3
	pslld	xmm1, 3 				; 06B8 _ 66: 0F 72. F1, 03
	por	xmm1, xmm10				; 06BD _ 66 41: 0F EB. CA
	movdqa	xmm10, xmm11				; 06C2 _ 66 45: 0F 6F. D3
	pxor	xmm0, xmm1				; 06C7 _ 66: 0F EF. C1
	movdqa	xmm8, xmm0				; 06CB _ 66 44: 0F 6F. C0
	pxor	xmm2, xmm1				; 06D0 _ 66: 0F EF. D1
	pslld	xmm10, 3				; 06D4 _ 66 41: 0F 72. F2, 03
	pxor	xmm2, xmm10				; 06DA _ 66 41: 0F EF. D2
	movdqa	xmm10, xmm2				; 06DF _ 66 44: 0F 6F. D2
	pslld	xmm8, 1 				; 06E4 _ 66 41: 0F 72. F0, 01
	psrld	xmm0, 31				; 06EA _ 66: 0F 72. D0, 1F
	por	xmm8, xmm0				; 06EF _ 66 44: 0F EB. C0
	pslld	xmm10, 7				; 06F4 _ 66 41: 0F 72. F2, 07
	psrld	xmm2, 25				; 06FA _ 66: 0F 72. D2, 19
	por	xmm10, xmm2				; 06FF _ 66 44: 0F EB. D2
	movdqa	xmm2, xmm8				; 0704 _ 66 41: 0F 6F. D0
	pxor	xmm11, xmm8				; 0709 _ 66 45: 0F EF. D8
	pxor	xmm11, xmm10				; 070E _ 66 45: 0F EF. DA
	movdqa	xmm0, xmm11				; 0713 _ 66 41: 0F 6F. C3
	pxor	xmm1, xmm10				; 0718 _ 66 41: 0F EF. CA
	pslld	xmm2, 7 				; 071D _ 66: 0F 72. F2, 07
	pxor	xmm1, xmm2				; 0722 _ 66: 0F EF. CA
	movdqa	xmm2, xmm1				; 0726 _ 66: 0F 6F. D1
	pslld	xmm0, 5 				; 072A _ 66: 0F 72. F0, 05
	psrld	xmm11, 27				; 072F _ 66 41: 0F 72. D3, 1B
	por	xmm0, xmm11				; 0735 _ 66 41: 0F EB. C3
	pslld	xmm2, 22				; 073A _ 66: 0F 72. F2, 16
	psrld	xmm1, 10				; 073F _ 66: 0F 72. D1, 0A
	por	xmm2, xmm1				; 0744 _ 66: 0F EB. D1
	movd	xmm11, dword [r12+12C0H]		; 0748 _ 66 45: 0F 6E. 9C 24, 000012C0
	pshufd	xmm11, xmm11, 0 			; 0752 _ 66 45: 0F 70. DB, 00
	pxor	xmm0, xmm11				; 0758 _ 66 41: 0F EF. C3
	movd	xmm11, dword [r12+12C4H]		; 075D _ 66 45: 0F 6E. 9C 24, 000012C4
	pshufd	xmm11, xmm11, 0 			; 0767 _ 66 45: 0F 70. DB, 00
	pxor	xmm8, xmm11				; 076D _ 66 45: 0F EF. C3
	movd	xmm11, dword [r12+12C8H]		; 0772 _ 66 45: 0F 6E. 9C 24, 000012C8
	pshufd	xmm11, xmm11, 0 			; 077C _ 66 45: 0F 70. DB, 00
	pxor	xmm2, xmm11				; 0782 _ 66 41: 0F EF. D3
	movd	xmm11, dword [r12+12CCH]		; 0787 _ 66 45: 0F 6E. 9C 24, 000012CC
	pshufd	xmm11, xmm11, 0 			; 0791 _ 66 45: 0F 70. DB, 00
	pxor	xmm10, xmm11				; 0797 _ 66 45: 0F EF. D3
	pxor	xmm8, xmm10				; 079C _ 66 45: 0F EF. C2
	movdqa	xmm1, xmm8				; 07A1 _ 66 41: 0F 6F. C8
	pxor	xmm10, xmm6				; 07A6 _ 66 44: 0F EF. D6
	pxor	xmm2, xmm10				; 07AB _ 66 41: 0F EF. D2
	pxor	xmm10, xmm0				; 07B0 _ 66 44: 0F EF. D0
	pand	xmm1, xmm10				; 07B5 _ 66 41: 0F DB. CA
	pxor	xmm1, xmm2				; 07BA _ 66: 0F EF. CA
	movdqa	xmm11, xmm1				; 07BE _ 66 44: 0F 6F. D9
	pxor	xmm8, xmm10				; 07C3 _ 66 45: 0F EF. C2
	pxor	xmm0, xmm8				; 07C8 _ 66 41: 0F EF. C0
	pand	xmm2, xmm8				; 07CD _ 66 41: 0F DB. D0
	pxor	xmm2, xmm0				; 07D2 _ 66: 0F EF. D0
	pand	xmm0, xmm1				; 07D6 _ 66: 0F DB. C1
	pxor	xmm10, xmm0				; 07DA _ 66 44: 0F EF. D0
	por	xmm8, xmm1				; 07DF _ 66 44: 0F EB. C1
	pxor	xmm8, xmm0				; 07E4 _ 66 44: 0F EF. C0
	por	xmm0, xmm10				; 07E9 _ 66 41: 0F EB. C2
	pxor	xmm0, xmm2				; 07EE _ 66: 0F EF. C2
	pand	xmm2, xmm10				; 07F2 _ 66 41: 0F DB. D2
	pxor	xmm0, xmm6				; 07F7 _ 66: 0F EF. C6
	pxor	xmm8, xmm2				; 07FB _ 66 44: 0F EF. C2
	movdqa	xmm2, xmm0				; 0800 _ 66: 0F 6F. D0
	pslld	xmm11, 13				; 0804 _ 66 41: 0F 72. F3, 0D
	psrld	xmm1, 19				; 080A _ 66: 0F 72. D1, 13
	por	xmm11, xmm1				; 080F _ 66 44: 0F EB. D9
	pslld	xmm2, 3 				; 0814 _ 66: 0F 72. F2, 03
	psrld	xmm0, 29				; 0819 _ 66: 0F 72. D0, 1D
	por	xmm2, xmm0				; 081E _ 66: 0F EB. D0
	movdqa	xmm0, xmm11				; 0822 _ 66 41: 0F 6F. C3
	pxor	xmm8, xmm11				; 0827 _ 66 45: 0F EF. C3
	pxor	xmm8, xmm2				; 082C _ 66 44: 0F EF. C2
	pxor	xmm10, xmm2				; 0831 _ 66 44: 0F EF. D2
	pslld	xmm0, 3 				; 0836 _ 66: 0F 72. F0, 03
	pxor	xmm10, xmm0				; 083B _ 66 44: 0F EF. D0
	movdqa	xmm0, xmm8				; 0840 _ 66 41: 0F 6F. C0
	psrld	xmm8, 31				; 0845 _ 66 41: 0F 72. D0, 1F
	pslld	xmm0, 1 				; 084B _ 66: 0F 72. F0, 01
	por	xmm0, xmm8				; 0850 _ 66 41: 0F EB. C0
	movdqa	xmm8, xmm10				; 0855 _ 66 45: 0F 6F. C2
	psrld	xmm10, 25				; 085A _ 66 41: 0F 72. D2, 19
	pxor	xmm11, xmm0				; 0860 _ 66 44: 0F EF. D8
	pslld	xmm8, 7 				; 0865 _ 66 41: 0F 72. F0, 07
	por	xmm8, xmm10				; 086B _ 66 45: 0F EB. C2
	movdqa	xmm10, xmm0				; 0870 _ 66 44: 0F 6F. D0
	pxor	xmm11, xmm8				; 0875 _ 66 45: 0F EF. D8
	movdqa	xmm1, xmm11				; 087A _ 66 41: 0F 6F. CB
	pxor	xmm2, xmm8				; 087F _ 66 41: 0F EF. D0
	pslld	xmm10, 7				; 0884 _ 66 41: 0F 72. F2, 07
	pxor	xmm2, xmm10				; 088A _ 66 41: 0F EF. D2
	pslld	xmm1, 5 				; 088F _ 66: 0F 72. F1, 05
	psrld	xmm11, 27				; 0894 _ 66 41: 0F 72. D3, 1B
	por	xmm1, xmm11				; 089A _ 66 41: 0F EB. CB
	movdqa	xmm11, xmm2				; 089F _ 66 44: 0F 6F. DA
	psrld	xmm2, 10				; 08A4 _ 66: 0F 72. D2, 0A
	pslld	xmm11, 22				; 08A9 _ 66 41: 0F 72. F3, 16
	por	xmm11, xmm2				; 08AF _ 66 44: 0F EB. DA
	movd	xmm2, dword [r12+12D0H] 		; 08B4 _ 66 41: 0F 6E. 94 24, 000012D0
	pshufd	xmm10, xmm2, 0				; 08BE _ 66 44: 0F 70. D2, 00
	pxor	xmm1, xmm10				; 08C4 _ 66 41: 0F EF. CA
	movd	xmm2, dword [r12+12D4H] 		; 08C9 _ 66 41: 0F 6E. 94 24, 000012D4
	pshufd	xmm10, xmm2, 0				; 08D3 _ 66 44: 0F 70. D2, 00
	pxor	xmm0, xmm10				; 08D9 _ 66 41: 0F EF. C2
	pxor	xmm1, xmm0				; 08DE _ 66: 0F EF. C8
	movd	xmm2, dword [r12+12D8H] 		; 08E2 _ 66 41: 0F 6E. 94 24, 000012D8
	pshufd	xmm10, xmm2, 0				; 08EC _ 66 44: 0F 70. D2, 00
	pxor	xmm11, xmm10				; 08F2 _ 66 45: 0F EF. DA
	movd	xmm2, dword [r12+12DCH] 		; 08F7 _ 66 41: 0F 6E. 94 24, 000012DC
	pshufd	xmm10, xmm2, 0				; 0901 _ 66 44: 0F 70. D2, 00
	pxor	xmm8, xmm10				; 0907 _ 66 45: 0F EF. C2
	pxor	xmm0, xmm8				; 090C _ 66 41: 0F EF. C0
	movdqa	xmm2, xmm0				; 0911 _ 66: 0F 6F. D0
	pxor	xmm8, xmm6				; 0915 _ 66 44: 0F EF. C6
	pxor	xmm11, xmm8				; 091A _ 66 45: 0F EF. D8
	pand	xmm2, xmm1				; 091F _ 66: 0F DB. D1
	pxor	xmm2, xmm11				; 0923 _ 66 41: 0F EF. D3
	movdqa	xmm10, xmm2				; 0928 _ 66 44: 0F 6F. D2
	por	xmm11, xmm0				; 092D _ 66 44: 0F EB. D8
	pxor	xmm0, xmm8				; 0932 _ 66 41: 0F EF. C0
	pand	xmm8, xmm2				; 0937 _ 66 44: 0F DB. C2
	pxor	xmm8, xmm1				; 093C _ 66 44: 0F EF. C1
	pxor	xmm0, xmm2				; 0941 _ 66: 0F EF. C2
	pxor	xmm0, xmm11				; 0945 _ 66 41: 0F EF. C3
	pxor	xmm11, xmm1				; 094A _ 66 44: 0F EF. D9
	pand	xmm1, xmm8				; 094F _ 66 41: 0F DB. C8
	pxor	xmm11, xmm6				; 0954 _ 66 44: 0F EF. DE
	pxor	xmm1, xmm0				; 0959 _ 66: 0F EF. C8
	por	xmm0, xmm8				; 095D _ 66 41: 0F EB. C0
	pxor	xmm0, xmm11				; 0962 _ 66 41: 0F EF. C3
	pslld	xmm10, 13				; 0967 _ 66 41: 0F 72. F2, 0D
	psrld	xmm2, 19				; 096D _ 66: 0F 72. D2, 13
	por	xmm10, xmm2				; 0972 _ 66 44: 0F EB. D2
	movdqa	xmm2, xmm1				; 0977 _ 66: 0F 6F. D1
	movdqa	xmm11, xmm10				; 097B _ 66 45: 0F 6F. DA
	psrld	xmm1, 29				; 0980 _ 66: 0F 72. D1, 1D
	pslld	xmm2, 3 				; 0985 _ 66: 0F 72. F2, 03
	por	xmm2, xmm1				; 098A _ 66: 0F EB. D1
	pxor	xmm8, xmm10				; 098E _ 66 45: 0F EF. C2
	pxor	xmm8, xmm2				; 0993 _ 66 44: 0F EF. C2
	movdqa	xmm1, xmm8				; 0998 _ 66 41: 0F 6F. C8
	pxor	xmm0, xmm2				; 099D _ 66: 0F EF. C2
	pslld	xmm11, 3				; 09A1 _ 66 41: 0F 72. F3, 03
	pxor	xmm0, xmm11				; 09A7 _ 66 41: 0F EF. C3
	movdqa	xmm11, xmm0				; 09AC _ 66 44: 0F 6F. D8
	pslld	xmm1, 1 				; 09B1 _ 66: 0F 72. F1, 01
	psrld	xmm8, 31				; 09B6 _ 66 41: 0F 72. D0, 1F
	por	xmm1, xmm8				; 09BC _ 66 41: 0F EB. C8
	pslld	xmm11, 7				; 09C1 _ 66 41: 0F 72. F3, 07
	psrld	xmm0, 25				; 09C7 _ 66: 0F 72. D0, 19
	por	xmm11, xmm0				; 09CC _ 66 44: 0F EB. D8
	pxor	xmm10, xmm1				; 09D1 _ 66 44: 0F EF. D1
	movdqa	xmm8, xmm1				; 09D6 _ 66 44: 0F 6F. C1
	pxor	xmm10, xmm11				; 09DB _ 66 45: 0F EF. D3
	pxor	xmm2, xmm11				; 09E0 _ 66 41: 0F EF. D3
	pslld	xmm8, 7 				; 09E5 _ 66 41: 0F 72. F0, 07
	pxor	xmm2, xmm8				; 09EB _ 66 41: 0F EF. D0
	movdqa	xmm8, xmm10				; 09F0 _ 66 45: 0F 6F. C2
	psrld	xmm10, 27				; 09F5 _ 66 41: 0F 72. D2, 1B
	pslld	xmm8, 5 				; 09FB _ 66 41: 0F 72. F0, 05
	por	xmm8, xmm10				; 0A01 _ 66 45: 0F EB. C2
	movdqa	xmm10, xmm2				; 0A06 _ 66 44: 0F 6F. D2
	psrld	xmm2, 10				; 0A0B _ 66: 0F 72. D2, 0A
	pslld	xmm10, 22				; 0A10 _ 66 41: 0F 72. F2, 16
	por	xmm10, xmm2				; 0A16 _ 66 44: 0F EB. D2
	movd	xmm2, dword [r12+12E0H] 		; 0A1B _ 66 41: 0F 6E. 94 24, 000012E0
	pshufd	xmm2, xmm2, 0				; 0A25 _ 66: 0F 70. D2, 00
	pxor	xmm8, xmm2				; 0A2A _ 66 44: 0F EF. C2
	movd	xmm2, dword [r12+12E4H] 		; 0A2F _ 66 41: 0F 6E. 94 24, 000012E4
	pshufd	xmm2, xmm2, 0				; 0A39 _ 66: 0F 70. D2, 00
	pxor	xmm1, xmm2				; 0A3E _ 66: 0F EF. CA
	movd	xmm2, dword [r12+12E8H] 		; 0A42 _ 66 41: 0F 6E. 94 24, 000012E8
	pshufd	xmm2, xmm2, 0				; 0A4C _ 66: 0F 70. D2, 00
	pxor	xmm10, xmm2				; 0A51 _ 66 44: 0F EF. D2
	pxor	xmm10, xmm6				; 0A56 _ 66 44: 0F EF. D6
	movd	xmm2, dword [r12+12ECH] 		; 0A5B _ 66 41: 0F 6E. 94 24, 000012EC
	pshufd	xmm2, xmm2, 0				; 0A65 _ 66: 0F 70. D2, 00
	pxor	xmm11, xmm2				; 0A6A _ 66 44: 0F EF. DA
	movdqa	xmm2, xmm11				; 0A6F _ 66 41: 0F 6F. D3
	pand	xmm2, xmm8				; 0A74 _ 66 41: 0F DB. D0
	pxor	xmm8, xmm11				; 0A79 _ 66 45: 0F EF. C3
	pxor	xmm2, xmm10				; 0A7E _ 66 41: 0F EF. D2
	por	xmm10, xmm11				; 0A83 _ 66 45: 0F EB. D3
	pxor	xmm1, xmm2				; 0A88 _ 66: 0F EF. CA
	pxor	xmm10, xmm8				; 0A8C _ 66 45: 0F EF. D0
	por	xmm8, xmm1				; 0A91 _ 66 44: 0F EB. C1
	pxor	xmm10, xmm1				; 0A96 _ 66 44: 0F EF. D1
	pxor	xmm11, xmm8				; 0A9B _ 66 45: 0F EF. D8
	por	xmm8, xmm2				; 0AA0 _ 66 44: 0F EB. C2
	pxor	xmm8, xmm10				; 0AA5 _ 66 45: 0F EF. C2
	movdqa	xmm0, xmm8				; 0AAA _ 66 41: 0F 6F. C0
	pxor	xmm11, xmm2				; 0AAF _ 66 44: 0F EF. DA
	pxor	xmm11, xmm8				; 0AB4 _ 66 45: 0F EF. D8
	pxor	xmm2, xmm6				; 0AB9 _ 66: 0F EF. D6
	pand	xmm10, xmm11				; 0ABD _ 66 45: 0F DB. D3
	pxor	xmm2, xmm10				; 0AC2 _ 66 41: 0F EF. D2
	movdqa	xmm10, xmm11				; 0AC7 _ 66 45: 0F 6F. D3
	pslld	xmm0, 13				; 0ACC _ 66: 0F 72. F0, 0D
	psrld	xmm8, 19				; 0AD1 _ 66 41: 0F 72. D0, 13
	por	xmm0, xmm8				; 0AD7 _ 66 41: 0F EB. C0
	pslld	xmm10, 3				; 0ADC _ 66 41: 0F 72. F2, 03
	psrld	xmm11, 29				; 0AE2 _ 66 41: 0F 72. D3, 1D
	por	xmm10, xmm11				; 0AE8 _ 66 45: 0F EB. D3
	movdqa	xmm11, xmm0				; 0AED _ 66 44: 0F 6F. D8
	pxor	xmm1, xmm0				; 0AF2 _ 66: 0F EF. C8
	pxor	xmm1, xmm10				; 0AF6 _ 66 41: 0F EF. CA
	pxor	xmm2, xmm10				; 0AFB _ 66 41: 0F EF. D2
	pslld	xmm11, 3				; 0B00 _ 66 41: 0F 72. F3, 03
	pxor	xmm2, xmm11				; 0B06 _ 66 41: 0F EF. D3
	movdqa	xmm11, xmm1				; 0B0B _ 66 44: 0F 6F. D9
	movdqa	xmm8, xmm2				; 0B10 _ 66 44: 0F 6F. C2
	psrld	xmm1, 31				; 0B15 _ 66: 0F 72. D1, 1F
	pslld	xmm11, 1				; 0B1A _ 66 41: 0F 72. F3, 01
	por	xmm11, xmm1				; 0B20 _ 66 44: 0F EB. D9
	pslld	xmm8, 7 				; 0B25 _ 66 41: 0F 72. F0, 07
	psrld	xmm2, 25				; 0B2B _ 66: 0F 72. D2, 19
	por	xmm8, xmm2				; 0B30 _ 66 44: 0F EB. C2
	movdqa	xmm2, xmm11				; 0B35 _ 66 41: 0F 6F. D3
	pxor	xmm0, xmm11				; 0B3A _ 66 41: 0F EF. C3
	pxor	xmm0, xmm8				; 0B3F _ 66 41: 0F EF. C0
	movdqa	xmm1, xmm0				; 0B44 _ 66: 0F 6F. C8
	pxor	xmm10, xmm8				; 0B48 _ 66 45: 0F EF. D0
	pslld	xmm2, 7 				; 0B4D _ 66: 0F 72. F2, 07
	pxor	xmm10, xmm2				; 0B52 _ 66 44: 0F EF. D2
	movdqa	xmm2, xmm10				; 0B57 _ 66 41: 0F 6F. D2
	pslld	xmm1, 5 				; 0B5C _ 66: 0F 72. F1, 05
	psrld	xmm0, 27				; 0B61 _ 66: 0F 72. D0, 1B
	por	xmm1, xmm0				; 0B66 _ 66: 0F EB. C8
	pslld	xmm2, 22				; 0B6A _ 66: 0F 72. F2, 16
	psrld	xmm10, 10				; 0B6F _ 66 41: 0F 72. D2, 0A
	por	xmm2, xmm10				; 0B75 _ 66 41: 0F EB. D2
	movd	xmm10, dword [r12+12F0H]		; 0B7A _ 66 45: 0F 6E. 94 24, 000012F0
	pshufd	xmm10, xmm10, 0 			; 0B84 _ 66 45: 0F 70. D2, 00
	pxor	xmm1, xmm10				; 0B8A _ 66 41: 0F EF. CA
	movd	xmm10, dword [r12+12F4H]		; 0B8F _ 66 45: 0F 6E. 94 24, 000012F4
	pshufd	xmm10, xmm10, 0 			; 0B99 _ 66 45: 0F 70. D2, 00
	pxor	xmm11, xmm10				; 0B9F _ 66 45: 0F EF. DA
	movdqa	xmm0, xmm11				; 0BA4 _ 66 41: 0F 6F. C3
	movd	xmm10, dword [r12+12F8H]		; 0BA9 _ 66 45: 0F 6E. 94 24, 000012F8
	pshufd	xmm10, xmm10, 0 			; 0BB3 _ 66 45: 0F 70. D2, 00
	pxor	xmm2, xmm10				; 0BB9 _ 66 41: 0F EF. D2
	por	xmm0, xmm2				; 0BBE _ 66: 0F EB. C2
	movd	xmm10, dword [r12+12FCH]		; 0BC2 _ 66 45: 0F 6E. 94 24, 000012FC
	pshufd	xmm10, xmm10, 0 			; 0BCC _ 66 45: 0F 70. D2, 00
	pxor	xmm8, xmm10				; 0BD2 _ 66 45: 0F EF. C2
	pxor	xmm0, xmm8				; 0BD7 _ 66 41: 0F EF. C0
	pxor	xmm11, xmm2				; 0BDC _ 66 44: 0F EF. DA
	pxor	xmm2, xmm0				; 0BE1 _ 66: 0F EF. D0
	por	xmm8, xmm11				; 0BE5 _ 66 45: 0F EB. C3
	pand	xmm8, xmm1				; 0BEA _ 66 44: 0F DB. C1
	pxor	xmm11, xmm2				; 0BEF _ 66 44: 0F EF. DA
	pxor	xmm8, xmm0				; 0BF4 _ 66 44: 0F EF. C0
	por	xmm0, xmm11				; 0BF9 _ 66 41: 0F EB. C3
	pxor	xmm0, xmm1				; 0BFE _ 66: 0F EF. C1
	por	xmm1, xmm11				; 0C02 _ 66 41: 0F EB. CB
	pxor	xmm1, xmm2				; 0C07 _ 66: 0F EF. CA
	pxor	xmm0, xmm11				; 0C0B _ 66 41: 0F EF. C3
	pxor	xmm2, xmm0				; 0C10 _ 66: 0F EF. D0
	pand	xmm0, xmm1				; 0C14 _ 66: 0F DB. C1
	pxor	xmm0, xmm11				; 0C18 _ 66 41: 0F EF. C3
	pxor	xmm2, xmm6				; 0C1D _ 66: 0F EF. D6
	por	xmm2, xmm1				; 0C21 _ 66: 0F EB. D1
	pxor	xmm11, xmm2				; 0C25 _ 66 44: 0F EF. DA
	movdqa	xmm2, xmm11				; 0C2A _ 66 41: 0F 6F. D3
	psrld	xmm11, 19				; 0C2F _ 66 41: 0F 72. D3, 13
	pslld	xmm2, 13				; 0C35 _ 66: 0F 72. F2, 0D
	por	xmm2, xmm11				; 0C3A _ 66 41: 0F EB. D3
	movdqa	xmm11, xmm0				; 0C3F _ 66 44: 0F 6F. D8
	movdqa	xmm10, xmm2				; 0C44 _ 66 44: 0F 6F. D2
	psrld	xmm0, 29				; 0C49 _ 66: 0F 72. D0, 1D
	pslld	xmm11, 3				; 0C4E _ 66 41: 0F 72. F3, 03
	por	xmm11, xmm0				; 0C54 _ 66 44: 0F EB. D8
	pxor	xmm8, xmm2				; 0C59 _ 66 44: 0F EF. C2
	pxor	xmm8, xmm11				; 0C5E _ 66 45: 0F EF. C3
	movdqa	xmm0, xmm8				; 0C63 _ 66 41: 0F 6F. C0
	pxor	xmm1, xmm11				; 0C68 _ 66 41: 0F EF. CB
	pslld	xmm10, 3				; 0C6D _ 66 41: 0F 72. F2, 03
	pxor	xmm1, xmm10				; 0C73 _ 66 41: 0F EF. CA
	movdqa	xmm10, xmm1				; 0C78 _ 66 44: 0F 6F. D1
	pslld	xmm0, 1 				; 0C7D _ 66: 0F 72. F0, 01
	psrld	xmm8, 31				; 0C82 _ 66 41: 0F 72. D0, 1F
	por	xmm0, xmm8				; 0C88 _ 66 41: 0F EB. C0
	movdqa	xmm8, xmm0				; 0C8D _ 66 44: 0F 6F. C0
	pslld	xmm10, 7				; 0C92 _ 66 41: 0F 72. F2, 07
	psrld	xmm1, 25				; 0C98 _ 66: 0F 72. D1, 19
	por	xmm10, xmm1				; 0C9D _ 66 44: 0F EB. D1
	pxor	xmm2, xmm0				; 0CA2 _ 66: 0F EF. D0
	pxor	xmm2, xmm10				; 0CA6 _ 66 41: 0F EF. D2
	pxor	xmm11, xmm10				; 0CAB _ 66 45: 0F EF. DA
	pslld	xmm8, 7 				; 0CB0 _ 66 41: 0F 72. F0, 07
	pxor	xmm11, xmm8				; 0CB6 _ 66 45: 0F EF. D8
	movdqa	xmm8, xmm2				; 0CBB _ 66 44: 0F 6F. C2
	movdqa	xmm1, xmm11				; 0CC0 _ 66 41: 0F 6F. CB
	psrld	xmm2, 27				; 0CC5 _ 66: 0F 72. D2, 1B
	pslld	xmm8, 5 				; 0CCA _ 66 41: 0F 72. F0, 05
	por	xmm8, xmm2				; 0CD0 _ 66 44: 0F EB. C2
	pslld	xmm1, 22				; 0CD5 _ 66: 0F 72. F1, 16
	psrld	xmm11, 10				; 0CDA _ 66 41: 0F 72. D3, 0A
	por	xmm1, xmm11				; 0CE0 _ 66 41: 0F EB. CB
	movd	xmm11, dword [r12+1300H]		; 0CE5 _ 66 45: 0F 6E. 9C 24, 00001300
	pshufd	xmm2, xmm11, 0				; 0CEF _ 66 41: 0F 70. D3, 00
	pxor	xmm8, xmm2				; 0CF5 _ 66 44: 0F EF. C2
	movd	xmm11, dword [r12+1304H]		; 0CFA _ 66 45: 0F 6E. 9C 24, 00001304
	pshufd	xmm2, xmm11, 0				; 0D04 _ 66 41: 0F 70. D3, 00
	pxor	xmm0, xmm2				; 0D0A _ 66: 0F EF. C2
	movd	xmm11, dword [r12+1308H]		; 0D0E _ 66 45: 0F 6E. 9C 24, 00001308
	pshufd	xmm2, xmm11, 0				; 0D18 _ 66 41: 0F 70. D3, 00
	pxor	xmm1, xmm2				; 0D1E _ 66: 0F EF. CA
	movd	xmm11, dword [r12+130CH]		; 0D22 _ 66 45: 0F 6E. 9C 24, 0000130C
	pshufd	xmm2, xmm11, 0				; 0D2C _ 66 41: 0F 70. D3, 00
	pxor	xmm10, xmm2				; 0D32 _ 66 44: 0F EF. D2
	movdqa	xmm2, xmm0				; 0D37 _ 66: 0F 6F. D0
	pxor	xmm10, xmm8				; 0D3B _ 66 45: 0F EF. D0
	pxor	xmm0, xmm1				; 0D40 _ 66: 0F EF. C1
	pand	xmm2, xmm10				; 0D44 _ 66 41: 0F DB. D2
	pxor	xmm2, xmm8				; 0D49 _ 66 41: 0F EF. D0
	por	xmm8, xmm10				; 0D4E _ 66 45: 0F EB. C2
	pxor	xmm8, xmm0				; 0D53 _ 66 44: 0F EF. C0
	pxor	xmm0, xmm10				; 0D58 _ 66 41: 0F EF. C2
	pxor	xmm10, xmm1				; 0D5D _ 66 44: 0F EF. D1
	por	xmm1, xmm2				; 0D62 _ 66: 0F EB. CA
	pxor	xmm1, xmm0				; 0D66 _ 66: 0F EF. C8
	pxor	xmm0, xmm6				; 0D6A _ 66: 0F EF. C6
	por	xmm0, xmm2				; 0D6E _ 66: 0F EB. C2
	pxor	xmm2, xmm10				; 0D72 _ 66 41: 0F EF. D2
	pxor	xmm2, xmm0				; 0D77 _ 66: 0F EF. D0
	por	xmm10, xmm8				; 0D7B _ 66 45: 0F EB. D0
	pxor	xmm2, xmm10				; 0D80 _ 66 41: 0F EF. D2
	movdqa	xmm11, xmm2				; 0D85 _ 66 44: 0F 6F. DA
	pxor	xmm0, xmm10				; 0D8A _ 66 41: 0F EF. C2
	psrld	xmm2, 19				; 0D8F _ 66: 0F 72. D2, 13
	pslld	xmm11, 13				; 0D94 _ 66 41: 0F 72. F3, 0D
	por	xmm11, xmm2				; 0D9A _ 66 44: 0F EB. DA
	movdqa	xmm2, xmm1				; 0D9F _ 66: 0F 6F. D1
	movdqa	xmm10, xmm11				; 0DA3 _ 66 45: 0F 6F. D3
	psrld	xmm1, 29				; 0DA8 _ 66: 0F 72. D1, 1D
	pslld	xmm2, 3 				; 0DAD _ 66: 0F 72. F2, 03
	por	xmm2, xmm1				; 0DB2 _ 66: 0F EB. D1
	pxor	xmm0, xmm11				; 0DB6 _ 66 41: 0F EF. C3
	pxor	xmm0, xmm2				; 0DBB _ 66: 0F EF. C2
	movdqa	xmm1, xmm0				; 0DBF _ 66: 0F 6F. C8
	pxor	xmm8, xmm2				; 0DC3 _ 66 44: 0F EF. C2
	pslld	xmm10, 3				; 0DC8 _ 66 41: 0F 72. F2, 03
	pxor	xmm8, xmm10				; 0DCE _ 66 45: 0F EF. C2
	movdqa	xmm10, xmm8				; 0DD3 _ 66 45: 0F 6F. D0
	pslld	xmm1, 1 				; 0DD8 _ 66: 0F 72. F1, 01
	psrld	xmm0, 31				; 0DDD _ 66: 0F 72. D0, 1F
	por	xmm1, xmm0				; 0DE2 _ 66: 0F EB. C8
	pslld	xmm10, 7				; 0DE6 _ 66 41: 0F 72. F2, 07
	psrld	xmm8, 25				; 0DEC _ 66 41: 0F 72. D0, 19
	por	xmm10, xmm8				; 0DF2 _ 66 45: 0F EB. D0
	movdqa	xmm8, xmm1				; 0DF7 _ 66 44: 0F 6F. C1
	pxor	xmm11, xmm1				; 0DFC _ 66 44: 0F EF. D9
	pxor	xmm11, xmm10				; 0E01 _ 66 45: 0F EF. DA
	movdqa	xmm0, xmm11				; 0E06 _ 66 41: 0F 6F. C3
	pxor	xmm2, xmm10				; 0E0B _ 66 41: 0F EF. D2
	pslld	xmm8, 7 				; 0E10 _ 66 41: 0F 72. F0, 07
	pxor	xmm2, xmm8				; 0E16 _ 66 41: 0F EF. D0
	movdqa	xmm8, xmm2				; 0E1B _ 66 44: 0F 6F. C2
	pslld	xmm0, 5 				; 0E20 _ 66: 0F 72. F0, 05
	psrld	xmm11, 27				; 0E25 _ 66 41: 0F 72. D3, 1B
	por	xmm0, xmm11				; 0E2B _ 66 41: 0F EB. C3
	pslld	xmm8, 22				; 0E30 _ 66 41: 0F 72. F0, 16
	psrld	xmm2, 10				; 0E36 _ 66: 0F 72. D2, 0A
	por	xmm8, xmm2				; 0E3B _ 66 44: 0F EB. C2
	movd	xmm11, dword [r12+1310H]		; 0E40 _ 66 45: 0F 6E. 9C 24, 00001310
	pshufd	xmm2, xmm11, 0				; 0E4A _ 66 41: 0F 70. D3, 00
	pxor	xmm0, xmm2				; 0E50 _ 66: 0F EF. C2
	movd	xmm11, dword [r12+1314H]		; 0E54 _ 66 45: 0F 6E. 9C 24, 00001314
	pshufd	xmm2, xmm11, 0				; 0E5E _ 66 41: 0F 70. D3, 00
	pxor	xmm1, xmm2				; 0E64 _ 66: 0F EF. CA
	pxor	xmm0, xmm6				; 0E68 _ 66: 0F EF. C6
	movd	xmm11, dword [r12+1318H]		; 0E6C _ 66 45: 0F 6E. 9C 24, 00001318
	pshufd	xmm2, xmm11, 0				; 0E76 _ 66 41: 0F 70. D3, 00
	pxor	xmm8, xmm2				; 0E7C _ 66 44: 0F EF. C2
	pxor	xmm8, xmm6				; 0E81 _ 66 44: 0F EF. C6
	movd	xmm11, dword [r12+131CH]		; 0E86 _ 66 45: 0F 6E. 9C 24, 0000131C
	pshufd	xmm2, xmm11, 0				; 0E90 _ 66 41: 0F 70. D3, 00
	movdqa	xmm11, xmm0				; 0E96 _ 66 44: 0F 6F. D8
	pxor	xmm10, xmm2				; 0E9B _ 66 44: 0F EF. D2
	pand	xmm11, xmm1				; 0EA0 _ 66 44: 0F DB. D9
	pxor	xmm8, xmm11				; 0EA5 _ 66 45: 0F EF. C3
	por	xmm11, xmm10				; 0EAA _ 66 45: 0F EB. DA
	pxor	xmm10, xmm8				; 0EAF _ 66 45: 0F EF. D0
	pxor	xmm1, xmm11				; 0EB4 _ 66 41: 0F EF. CB
	pxor	xmm11, xmm0				; 0EB9 _ 66 44: 0F EF. D8
	por	xmm0, xmm1				; 0EBE _ 66: 0F EB. C1
	pxor	xmm1, xmm10				; 0EC2 _ 66 41: 0F EF. CA
	por	xmm8, xmm11				; 0EC7 _ 66 45: 0F EB. C3
	pand	xmm8, xmm0				; 0ECC _ 66 44: 0F DB. C0
	pxor	xmm11, xmm1				; 0ED1 _ 66 44: 0F EF. D9
	pand	xmm1, xmm8				; 0ED6 _ 66 41: 0F DB. C8
	pxor	xmm1, xmm11				; 0EDB _ 66 41: 0F EF. CB
	pand	xmm11, xmm8				; 0EE0 _ 66 45: 0F DB. D8
	pxor	xmm0, xmm11				; 0EE5 _ 66 41: 0F EF. C3
	movdqa	xmm11, xmm8				; 0EEA _ 66 45: 0F 6F. D8
	psrld	xmm8, 19				; 0EEF _ 66 41: 0F 72. D0, 13
	pslld	xmm11, 13				; 0EF5 _ 66 41: 0F 72. F3, 0D
	por	xmm11, xmm8				; 0EFB _ 66 45: 0F EB. D8
	movdqa	xmm8, xmm10				; 0F00 _ 66 45: 0F 6F. C2
	psrld	xmm10, 29				; 0F05 _ 66 41: 0F 72. D2, 1D
	pxor	xmm0, xmm11				; 0F0B _ 66 41: 0F EF. C3
	pslld	xmm8, 3 				; 0F10 _ 66 41: 0F 72. F0, 03
	por	xmm8, xmm10				; 0F16 _ 66 45: 0F EB. C2
	movdqa	xmm10, xmm11				; 0F1B _ 66 45: 0F 6F. D3
	pxor	xmm0, xmm8				; 0F20 _ 66 41: 0F EF. C0
	pxor	xmm1, xmm8				; 0F25 _ 66 41: 0F EF. C8
	pslld	xmm10, 3				; 0F2A _ 66 41: 0F 72. F2, 03
	pxor	xmm1, xmm10				; 0F30 _ 66 41: 0F EF. CA
	movdqa	xmm10, xmm0				; 0F35 _ 66 44: 0F 6F. D0
	movdqa	xmm2, xmm1				; 0F3A _ 66: 0F 6F. D1
	psrld	xmm0, 31				; 0F3E _ 66: 0F 72. D0, 1F
	pslld	xmm10, 1				; 0F43 _ 66 41: 0F 72. F2, 01
	por	xmm10, xmm0				; 0F49 _ 66 44: 0F EB. D0
	movdqa	xmm0, xmm10				; 0F4E _ 66 41: 0F 6F. C2
	pslld	xmm2, 7 				; 0F53 _ 66: 0F 72. F2, 07
	psrld	xmm1, 25				; 0F58 _ 66: 0F 72. D1, 19
	por	xmm2, xmm1				; 0F5D _ 66: 0F EB. D1
	pxor	xmm11, xmm10				; 0F61 _ 66 45: 0F EF. DA
	pxor	xmm11, xmm2				; 0F66 _ 66 44: 0F EF. DA
	pxor	xmm8, xmm2				; 0F6B _ 66 44: 0F EF. C2
	pslld	xmm0, 7 				; 0F70 _ 66: 0F 72. F0, 07
	pxor	xmm8, xmm0				; 0F75 _ 66 44: 0F EF. C0
	movdqa	xmm0, xmm11				; 0F7A _ 66 41: 0F 6F. C3
	movdqa	xmm1, xmm8				; 0F7F _ 66 41: 0F 6F. C8
	psrld	xmm11, 27				; 0F84 _ 66 41: 0F 72. D3, 1B
	pslld	xmm0, 5 				; 0F8A _ 66: 0F 72. F0, 05
	por	xmm0, xmm11				; 0F8F _ 66 41: 0F EB. C3
	pslld	xmm1, 22				; 0F94 _ 66: 0F 72. F1, 16
	psrld	xmm8, 10				; 0F99 _ 66 41: 0F 72. D0, 0A
	por	xmm1, xmm8				; 0F9F _ 66 41: 0F EB. C8
	movd	xmm8, dword [r12+1320H] 		; 0FA4 _ 66 45: 0F 6E. 84 24, 00001320
	movd	xmm11, dword [r12+1324H]		; 0FAE _ 66 45: 0F 6E. 9C 24, 00001324
	pshufd	xmm8, xmm8, 0				; 0FB8 _ 66 45: 0F 70. C0, 00
	pxor	xmm0, xmm8				; 0FBE _ 66 41: 0F EF. C0
	pshufd	xmm8, xmm11, 0				; 0FC3 _ 66 45: 0F 70. C3, 00
	pxor	xmm10, xmm8				; 0FC9 _ 66 45: 0F EF. D0
	movd	xmm11, dword [r12+1328H]		; 0FCE _ 66 45: 0F 6E. 9C 24, 00001328
	pshufd	xmm8, xmm11, 0				; 0FD8 _ 66 45: 0F 70. C3, 00
	pxor	xmm1, xmm8				; 0FDE _ 66 41: 0F EF. C8
	movd	xmm11, dword [r12+132CH]		; 0FE3 _ 66 45: 0F 6E. 9C 24, 0000132C
	pshufd	xmm8, xmm11, 0				; 0FED _ 66 45: 0F 70. C3, 00
	movdqa	xmm11, xmm0				; 0FF3 _ 66 44: 0F 6F. D8
	pxor	xmm2, xmm8				; 0FF8 _ 66 41: 0F EF. D0
	pand	xmm11, xmm1				; 0FFD _ 66 44: 0F DB. D9
	pxor	xmm11, xmm2				; 1002 _ 66 44: 0F EF. DA
	pxor	xmm1, xmm10				; 1007 _ 66 41: 0F EF. CA
	pxor	xmm1, xmm11				; 100C _ 66 41: 0F EF. CB
	por	xmm2, xmm0				; 1011 _ 66: 0F EB. D0
	pxor	xmm2, xmm10				; 1015 _ 66 41: 0F EF. D2
	movdqa	xmm8, xmm2				; 101A _ 66 44: 0F 6F. C2
	pxor	xmm0, xmm1				; 101F _ 66: 0F EF. C1
	por	xmm8, xmm0				; 1023 _ 66 44: 0F EB. C0
	pxor	xmm8, xmm11				; 1028 _ 66 45: 0F EF. C3
	pand	xmm11, xmm2				; 102D _ 66 44: 0F DB. DA
	pxor	xmm0, xmm11				; 1032 _ 66 41: 0F EF. C3
	movdqa	xmm11, xmm1				; 1037 _ 66 44: 0F 6F. D9
	pxor	xmm2, xmm8				; 103C _ 66 41: 0F EF. D0
	pxor	xmm2, xmm0				; 1041 _ 66: 0F EF. D0
	movdqa	xmm10, xmm2				; 1045 _ 66 44: 0F 6F. D2
	pxor	xmm0, xmm6				; 104A _ 66: 0F EF. C6
	pslld	xmm11, 13				; 104E _ 66 41: 0F 72. F3, 0D
	psrld	xmm1, 19				; 1054 _ 66: 0F 72. D1, 13
	por	xmm11, xmm1				; 1059 _ 66 44: 0F EB. D9
	pslld	xmm10, 3				; 105E _ 66 41: 0F 72. F2, 03
	psrld	xmm2, 29				; 1064 _ 66: 0F 72. D2, 1D
	por	xmm10, xmm2				; 1069 _ 66 44: 0F EB. D2
	movdqa	xmm2, xmm11				; 106E _ 66 41: 0F 6F. D3
	pxor	xmm8, xmm11				; 1073 _ 66 45: 0F EF. C3
	pxor	xmm8, xmm10				; 1078 _ 66 45: 0F EF. C2
	movdqa	xmm1, xmm8				; 107D _ 66 41: 0F 6F. C8
	pxor	xmm0, xmm10				; 1082 _ 66 41: 0F EF. C2
	pslld	xmm2, 3 				; 1087 _ 66: 0F 72. F2, 03
	pxor	xmm0, xmm2				; 108C _ 66: 0F EF. C2
	pslld	xmm1, 1 				; 1090 _ 66: 0F 72. F1, 01
	psrld	xmm8, 31				; 1095 _ 66 41: 0F 72. D0, 1F
	por	xmm1, xmm8				; 109B _ 66 41: 0F EB. C8
	movdqa	xmm8, xmm0				; 10A0 _ 66 44: 0F 6F. C0
	movdqa	xmm2, xmm1				; 10A5 _ 66: 0F 6F. D1
	psrld	xmm0, 25				; 10A9 _ 66: 0F 72. D0, 19
	pslld	xmm8, 7 				; 10AE _ 66 41: 0F 72. F0, 07
	por	xmm8, xmm0				; 10B4 _ 66 44: 0F EB. C0
	pxor	xmm11, xmm1				; 10B9 _ 66 44: 0F EF. D9
	pxor	xmm11, xmm8				; 10BE _ 66 45: 0F EF. D8
	movdqa	xmm0, xmm11				; 10C3 _ 66 41: 0F 6F. C3
	pxor	xmm10, xmm8				; 10C8 _ 66 45: 0F EF. D0
	pslld	xmm2, 7 				; 10CD _ 66: 0F 72. F2, 07
	pxor	xmm10, xmm2				; 10D2 _ 66 44: 0F EF. D2
	movdqa	xmm2, xmm10				; 10D7 _ 66 41: 0F 6F. D2
	pslld	xmm0, 5 				; 10DC _ 66: 0F 72. F0, 05
	psrld	xmm11, 27				; 10E1 _ 66 41: 0F 72. D3, 1B
	por	xmm0, xmm11				; 10E7 _ 66 41: 0F EB. C3
	pslld	xmm2, 22				; 10EC _ 66: 0F 72. F2, 16
	psrld	xmm10, 10				; 10F1 _ 66 41: 0F 72. D2, 0A
	por	xmm2, xmm10				; 10F7 _ 66 41: 0F EB. D2
	movd	xmm10, dword [r12+1330H]		; 10FC _ 66 45: 0F 6E. 94 24, 00001330
	movd	xmm11, dword [r12+1334H]		; 1106 _ 66 45: 0F 6E. 9C 24, 00001334
	pshufd	xmm10, xmm10, 0 			; 1110 _ 66 45: 0F 70. D2, 00
	pxor	xmm0, xmm10				; 1116 _ 66 41: 0F EF. C2
	pshufd	xmm10, xmm11, 0 			; 111B _ 66 45: 0F 70. D3, 00
	pxor	xmm1, xmm10				; 1121 _ 66 41: 0F EF. CA
	movd	xmm11, dword [r12+1338H]		; 1126 _ 66 45: 0F 6E. 9C 24, 00001338
	pshufd	xmm10, xmm11, 0 			; 1130 _ 66 45: 0F 70. D3, 00
	pxor	xmm2, xmm10				; 1136 _ 66 41: 0F EF. D2
	movd	xmm11, dword [r12+133CH]		; 113B _ 66 45: 0F 6E. 9C 24, 0000133C
	pshufd	xmm10, xmm11, 0 			; 1145 _ 66 45: 0F 70. D3, 00
	movdqa	xmm11, xmm0				; 114B _ 66 44: 0F 6F. D8
	pxor	xmm8, xmm10				; 1150 _ 66 45: 0F EF. C2
	por	xmm11, xmm8				; 1155 _ 66 45: 0F EB. D8
	pxor	xmm8, xmm1				; 115A _ 66 44: 0F EF. C1
	pand	xmm1, xmm0				; 115F _ 66: 0F DB. C8
	pxor	xmm0, xmm2				; 1163 _ 66: 0F EF. C2
	pxor	xmm2, xmm8				; 1167 _ 66 41: 0F EF. D0
	pand	xmm8, xmm11				; 116C _ 66 45: 0F DB. C3
	por	xmm0, xmm1				; 1171 _ 66: 0F EB. C1
	pxor	xmm8, xmm0				; 1175 _ 66 44: 0F EF. C0
	pxor	xmm11, xmm1				; 117A _ 66 44: 0F EF. D9
	pand	xmm0, xmm11				; 117F _ 66 41: 0F DB. C3
	pxor	xmm1, xmm8				; 1184 _ 66 41: 0F EF. C8
	pxor	xmm0, xmm2				; 1189 _ 66: 0F EF. C2
	por	xmm1, xmm11				; 118D _ 66 41: 0F EB. CB
	pxor	xmm1, xmm2				; 1192 _ 66: 0F EF. CA
	movdqa	xmm2, xmm1				; 1196 _ 66: 0F 6F. D1
	pxor	xmm11, xmm8				; 119A _ 66 45: 0F EF. D8
	por	xmm2, xmm8				; 119F _ 66 41: 0F EB. D0
	pxor	xmm11, xmm2				; 11A4 _ 66 44: 0F EF. DA
	movdqa	xmm10, xmm11				; 11A9 _ 66 45: 0F 6F. D3
	movdqa	xmm2, xmm8				; 11AE _ 66 41: 0F 6F. D0
	psrld	xmm11, 19				; 11B3 _ 66 41: 0F 72. D3, 13
	pslld	xmm10, 13				; 11B9 _ 66 41: 0F 72. F2, 0D
	por	xmm10, xmm11				; 11BF _ 66 45: 0F EB. D3
	pslld	xmm2, 3 				; 11C4 _ 66: 0F 72. F2, 03
	psrld	xmm8, 29				; 11C9 _ 66 41: 0F 72. D0, 1D
	por	xmm2, xmm8				; 11CF _ 66 41: 0F EB. D0
	movdqa	xmm8, xmm10				; 11D4 _ 66 45: 0F 6F. C2
	pxor	xmm1, xmm10				; 11D9 _ 66 41: 0F EF. CA
	pxor	xmm1, xmm2				; 11DE _ 66: 0F EF. CA
	pxor	xmm0, xmm2				; 11E2 _ 66: 0F EF. C2
	pslld	xmm8, 3 				; 11E6 _ 66 41: 0F 72. F0, 03
	pxor	xmm0, xmm8				; 11EC _ 66 41: 0F EF. C0
	movdqa	xmm8, xmm1				; 11F1 _ 66 44: 0F 6F. C1
	psrld	xmm1, 31				; 11F6 _ 66: 0F 72. D1, 1F
	pslld	xmm8, 1 				; 11FB _ 66 41: 0F 72. F0, 01
	por	xmm8, xmm1				; 1201 _ 66 44: 0F EB. C1
	movdqa	xmm1, xmm0				; 1206 _ 66: 0F 6F. C8
	movdqa	xmm11, xmm8				; 120A _ 66 45: 0F 6F. D8
	psrld	xmm0, 25				; 120F _ 66: 0F 72. D0, 19
	pslld	xmm1, 7 				; 1214 _ 66: 0F 72. F1, 07
	por	xmm1, xmm0				; 1219 _ 66: 0F EB. C8
	pxor	xmm10, xmm8				; 121D _ 66 45: 0F EF. D0
	pxor	xmm10, xmm1				; 1222 _ 66 44: 0F EF. D1
	movdqa	xmm0, xmm10				; 1227 _ 66 41: 0F 6F. C2
	pxor	xmm2, xmm1				; 122C _ 66: 0F EF. D1
	pslld	xmm11, 7				; 1230 _ 66 41: 0F 72. F3, 07
	pxor	xmm2, xmm11				; 1236 _ 66 41: 0F EF. D3
	pslld	xmm0, 5 				; 123B _ 66: 0F 72. F0, 05
	psrld	xmm10, 27				; 1240 _ 66 41: 0F 72. D2, 1B
	por	xmm0, xmm10				; 1246 _ 66 41: 0F EB. C2
	movdqa	xmm10, xmm2				; 124B _ 66 44: 0F 6F. D2
	movd	xmm11, dword [r12+1344H]		; 1250 _ 66 45: 0F 6E. 9C 24, 00001344
	psrld	xmm2, 10				; 125A _ 66: 0F 72. D2, 0A
	pslld	xmm10, 22				; 125F _ 66 41: 0F 72. F2, 16
	por	xmm10, xmm2				; 1265 _ 66 44: 0F EB. D2
	movd	xmm2, dword [r12+1340H] 		; 126A _ 66 41: 0F 6E. 94 24, 00001340
	pshufd	xmm2, xmm2, 0				; 1274 _ 66: 0F 70. D2, 00
	pxor	xmm0, xmm2				; 1279 _ 66: 0F EF. C2
	pshufd	xmm2, xmm11, 0				; 127D _ 66 41: 0F 70. D3, 00
	movd	xmm11, dword [r12+1348H]		; 1283 _ 66 45: 0F 6E. 9C 24, 00001348
	pxor	xmm8, xmm2				; 128D _ 66 44: 0F EF. C2
	pshufd	xmm2, xmm11, 0				; 1292 _ 66 41: 0F 70. D3, 00
	pxor	xmm10, xmm2				; 1298 _ 66 44: 0F EF. D2
	movd	xmm11, dword [r12+134CH]		; 129D _ 66 45: 0F 6E. 9C 24, 0000134C
	pshufd	xmm2, xmm11, 0				; 12A7 _ 66 41: 0F 70. D3, 00
	pxor	xmm1, xmm2				; 12AD _ 66: 0F EF. CA
	pxor	xmm8, xmm1				; 12B1 _ 66 44: 0F EF. C1
	movdqa	xmm11, xmm8				; 12B6 _ 66 45: 0F 6F. D8
	pxor	xmm1, xmm6				; 12BB _ 66: 0F EF. CE
	pxor	xmm10, xmm1				; 12BF _ 66 44: 0F EF. D1
	pxor	xmm1, xmm0				; 12C4 _ 66: 0F EF. C8
	pand	xmm11, xmm1				; 12C8 _ 66 44: 0F DB. D9
	pxor	xmm11, xmm10				; 12CD _ 66 45: 0F EF. DA
	pxor	xmm8, xmm1				; 12D2 _ 66 44: 0F EF. C1
	pxor	xmm0, xmm8				; 12D7 _ 66 41: 0F EF. C0
	movdqa	xmm2, xmm11				; 12DC _ 66 41: 0F 6F. D3
	pand	xmm10, xmm8				; 12E1 _ 66 45: 0F DB. D0
	pxor	xmm10, xmm0				; 12E6 _ 66 44: 0F EF. D0
	pand	xmm0, xmm11				; 12EB _ 66 41: 0F DB. C3
	pxor	xmm1, xmm0				; 12F0 _ 66: 0F EF. C8
	por	xmm8, xmm11				; 12F4 _ 66 45: 0F EB. C3
	pxor	xmm8, xmm0				; 12F9 _ 66 44: 0F EF. C0
	por	xmm0, xmm1				; 12FE _ 66: 0F EB. C1
	pxor	xmm0, xmm10				; 1302 _ 66 41: 0F EF. C2
	pand	xmm10, xmm1				; 1307 _ 66 44: 0F DB. D1
	pxor	xmm0, xmm6				; 130C _ 66: 0F EF. C6
	pxor	xmm8, xmm10				; 1310 _ 66 45: 0F EF. C2
	pslld	xmm2, 13				; 1315 _ 66: 0F 72. F2, 0D
	psrld	xmm11, 19				; 131A _ 66 41: 0F 72. D3, 13
	por	xmm2, xmm11				; 1320 _ 66 41: 0F EB. D3
	movdqa	xmm11, xmm0				; 1325 _ 66 44: 0F 6F. D8
	movdqa	xmm10, xmm2				; 132A _ 66 44: 0F 6F. D2
	psrld	xmm0, 29				; 132F _ 66: 0F 72. D0, 1D
	pslld	xmm11, 3				; 1334 _ 66 41: 0F 72. F3, 03
	por	xmm11, xmm0				; 133A _ 66 44: 0F EB. D8
	pxor	xmm8, xmm2				; 133F _ 66 44: 0F EF. C2
	pxor	xmm8, xmm11				; 1344 _ 66 45: 0F EF. C3
	movdqa	xmm0, xmm8				; 1349 _ 66 41: 0F 6F. C0
	pxor	xmm1, xmm11				; 134E _ 66 41: 0F EF. CB
	pslld	xmm10, 3				; 1353 _ 66 41: 0F 72. F2, 03
	pxor	xmm1, xmm10				; 1359 _ 66 41: 0F EF. CA
	movdqa	xmm10, xmm1				; 135E _ 66 44: 0F 6F. D1
	pslld	xmm0, 1 				; 1363 _ 66: 0F 72. F0, 01
	psrld	xmm8, 31				; 1368 _ 66 41: 0F 72. D0, 1F
	por	xmm0, xmm8				; 136E _ 66 41: 0F EB. C0
	movdqa	xmm8, xmm0				; 1373 _ 66 44: 0F 6F. C0
	pslld	xmm10, 7				; 1378 _ 66 41: 0F 72. F2, 07
	psrld	xmm1, 25				; 137E _ 66: 0F 72. D1, 19
	por	xmm10, xmm1				; 1383 _ 66 44: 0F EB. D1
	pxor	xmm2, xmm0				; 1388 _ 66: 0F EF. D0
	pxor	xmm2, xmm10				; 138C _ 66 41: 0F EF. D2
	pxor	xmm11, xmm10				; 1391 _ 66 45: 0F EF. DA
	pslld	xmm8, 7 				; 1396 _ 66 41: 0F 72. F0, 07
	pxor	xmm11, xmm8				; 139C _ 66 45: 0F EF. D8
	movdqa	xmm8, xmm2				; 13A1 _ 66 44: 0F 6F. C2
	movdqa	xmm1, xmm11				; 13A6 _ 66 41: 0F 6F. CB
	psrld	xmm2, 27				; 13AB _ 66: 0F 72. D2, 1B
	pslld	xmm8, 5 				; 13B0 _ 66 41: 0F 72. F0, 05
	por	xmm8, xmm2				; 13B6 _ 66 44: 0F EB. C2
	pslld	xmm1, 22				; 13BB _ 66: 0F 72. F1, 16
	psrld	xmm11, 10				; 13C0 _ 66 41: 0F 72. D3, 0A
	por	xmm1, xmm11				; 13C6 _ 66 41: 0F EB. CB
	movd	xmm11, dword [r12+1350H]		; 13CB _ 66 45: 0F 6E. 9C 24, 00001350
	movd	xmm2, dword [r12+1354H] 		; 13D5 _ 66 41: 0F 6E. 94 24, 00001354
	pshufd	xmm11, xmm11, 0 			; 13DF _ 66 45: 0F 70. DB, 00
	pxor	xmm8, xmm11				; 13E5 _ 66 45: 0F EF. C3
	pshufd	xmm11, xmm2, 0				; 13EA _ 66 44: 0F 70. DA, 00
	pxor	xmm0, xmm11				; 13F0 _ 66 41: 0F EF. C3
	pxor	xmm8, xmm0				; 13F5 _ 66 44: 0F EF. C0
	movd	xmm2, dword [r12+1358H] 		; 13FA _ 66 41: 0F 6E. 94 24, 00001358
	pshufd	xmm11, xmm2, 0				; 1404 _ 66 44: 0F 70. DA, 00
	movd	xmm2, dword [r12+135CH] 		; 140A _ 66 41: 0F 6E. 94 24, 0000135C
	pxor	xmm1, xmm11				; 1414 _ 66 41: 0F EF. CB
	pshufd	xmm11, xmm2, 0				; 1419 _ 66 44: 0F 70. DA, 00
	pxor	xmm10, xmm11				; 141F _ 66 45: 0F EF. D3
	pxor	xmm0, xmm10				; 1424 _ 66 41: 0F EF. C2
	movdqa	xmm11, xmm0				; 1429 _ 66 44: 0F 6F. D8
	pxor	xmm10, xmm6				; 142E _ 66 44: 0F EF. D6
	pxor	xmm1, xmm10				; 1433 _ 66 41: 0F EF. CA
	pand	xmm11, xmm8				; 1438 _ 66 45: 0F DB. D8
	pxor	xmm11, xmm1				; 143D _ 66 44: 0F EF. D9
	movdqa	xmm2, xmm11				; 1442 _ 66 41: 0F 6F. D3
	por	xmm1, xmm0				; 1447 _ 66: 0F EB. C8
	pxor	xmm0, xmm10				; 144B _ 66 41: 0F EF. C2
	pand	xmm10, xmm11				; 1450 _ 66 45: 0F DB. D3
	pxor	xmm10, xmm8				; 1455 _ 66 45: 0F EF. D0
	pxor	xmm0, xmm11				; 145A _ 66 41: 0F EF. C3
	pxor	xmm0, xmm1				; 145F _ 66: 0F EF. C1
	pxor	xmm1, xmm8				; 1463 _ 66 41: 0F EF. C8
	pand	xmm8, xmm10				; 1468 _ 66 45: 0F DB. C2
	pxor	xmm1, xmm6				; 146D _ 66: 0F EF. CE
	pxor	xmm8, xmm0				; 1471 _ 66 44: 0F EF. C0
	por	xmm0, xmm10				; 1476 _ 66 41: 0F EB. C2
	pxor	xmm0, xmm1				; 147B _ 66: 0F EF. C1
	movdqa	xmm1, xmm8				; 147F _ 66 41: 0F 6F. C8
	pslld	xmm2, 13				; 1484 _ 66: 0F 72. F2, 0D
	psrld	xmm11, 19				; 1489 _ 66 41: 0F 72. D3, 13
	por	xmm2, xmm11				; 148F _ 66 41: 0F EB. D3
	pslld	xmm1, 3 				; 1494 _ 66: 0F 72. F1, 03
	psrld	xmm8, 29				; 1499 _ 66 41: 0F 72. D0, 1D
	por	xmm1, xmm8				; 149F _ 66 41: 0F EB. C8
	movdqa	xmm8, xmm2				; 14A4 _ 66 44: 0F 6F. C2
	pxor	xmm10, xmm2				; 14A9 _ 66 44: 0F EF. D2
	pxor	xmm10, xmm1				; 14AE _ 66 44: 0F EF. D1
	movdqa	xmm11, xmm10				; 14B3 _ 66 45: 0F 6F. DA
	pxor	xmm0, xmm1				; 14B8 _ 66: 0F EF. C1
	pslld	xmm8, 3 				; 14BC _ 66 41: 0F 72. F0, 03
	pxor	xmm0, xmm8				; 14C2 _ 66 41: 0F EF. C0
	movdqa	xmm8, xmm0				; 14C7 _ 66 44: 0F 6F. C0
	pslld	xmm11, 1				; 14CC _ 66 41: 0F 72. F3, 01
	psrld	xmm10, 31				; 14D2 _ 66 41: 0F 72. D2, 1F
	por	xmm11, xmm10				; 14D8 _ 66 45: 0F EB. DA
	movdqa	xmm10, xmm11				; 14DD _ 66 45: 0F 6F. D3
	pslld	xmm8, 7 				; 14E2 _ 66 41: 0F 72. F0, 07
	psrld	xmm0, 25				; 14E8 _ 66: 0F 72. D0, 19
	por	xmm8, xmm0				; 14ED _ 66 44: 0F EB. C0
	pxor	xmm2, xmm11				; 14F2 _ 66 41: 0F EF. D3
	pxor	xmm2, xmm8				; 14F7 _ 66 41: 0F EF. D0
	pxor	xmm1, xmm8				; 14FC _ 66 41: 0F EF. C8
	pslld	xmm10, 7				; 1501 _ 66 41: 0F 72. F2, 07
	movd	xmm0, dword [r12+1360H] 		; 1507 _ 66 41: 0F 6E. 84 24, 00001360
	pxor	xmm1, xmm10				; 1511 _ 66 41: 0F EF. CA
	movdqa	xmm10, xmm2				; 1516 _ 66 44: 0F 6F. D2
	psrld	xmm2, 27				; 151B _ 66: 0F 72. D2, 1B
	pshufd	xmm0, xmm0, 0				; 1520 _ 66: 0F 70. C0, 00
	pslld	xmm10, 5				; 1525 _ 66 41: 0F 72. F2, 05
	por	xmm10, xmm2				; 152B _ 66 44: 0F EB. D2
	movdqa	xmm2, xmm1				; 1530 _ 66: 0F 6F. D1
	psrld	xmm1, 10				; 1534 _ 66: 0F 72. D1, 0A
	pxor	xmm10, xmm0				; 1539 _ 66 44: 0F EF. D0
	pslld	xmm2, 22				; 153E _ 66: 0F 72. F2, 16
	por	xmm2, xmm1				; 1543 _ 66: 0F EB. D1
	movd	xmm0, dword [r12+1364H] 		; 1547 _ 66 41: 0F 6E. 84 24, 00001364
	pshufd	xmm0, xmm0, 0				; 1551 _ 66: 0F 70. C0, 00
	pxor	xmm11, xmm0				; 1556 _ 66 44: 0F EF. D8
	movd	xmm0, dword [r12+1368H] 		; 155B _ 66 41: 0F 6E. 84 24, 00001368
	pshufd	xmm0, xmm0, 0				; 1565 _ 66: 0F 70. C0, 00
	pxor	xmm2, xmm0				; 156A _ 66: 0F EF. D0
	pxor	xmm2, xmm6				; 156E _ 66: 0F EF. D6
	movd	xmm0, dword [r12+136CH] 		; 1572 _ 66 41: 0F 6E. 84 24, 0000136C
	pshufd	xmm0, xmm0, 0				; 157C _ 66: 0F 70. C0, 00
	pxor	xmm8, xmm0				; 1581 _ 66 44: 0F EF. C0
	movdqa	xmm1, xmm8				; 1586 _ 66 41: 0F 6F. C8
	pand	xmm1, xmm10				; 158B _ 66 41: 0F DB. CA
	pxor	xmm10, xmm8				; 1590 _ 66 45: 0F EF. D0
	pxor	xmm1, xmm2				; 1595 _ 66: 0F EF. CA
	por	xmm2, xmm8				; 1599 _ 66 41: 0F EB. D0
	pxor	xmm11, xmm1				; 159E _ 66 44: 0F EF. D9
	pxor	xmm2, xmm10				; 15A3 _ 66 41: 0F EF. D2
	por	xmm10, xmm11				; 15A8 _ 66 45: 0F EB. D3
	pxor	xmm2, xmm11				; 15AD _ 66 41: 0F EF. D3
	pxor	xmm8, xmm10				; 15B2 _ 66 45: 0F EF. C2
	por	xmm10, xmm1				; 15B7 _ 66 44: 0F EB. D1
	pxor	xmm10, xmm2				; 15BC _ 66 44: 0F EF. D2
	movdqa	xmm0, xmm10				; 15C1 _ 66 41: 0F 6F. C2
	pxor	xmm8, xmm1				; 15C6 _ 66 44: 0F EF. C1
	pxor	xmm8, xmm10				; 15CB _ 66 45: 0F EF. C2
	pxor	xmm1, xmm6				; 15D0 _ 66: 0F EF. CE
	pand	xmm2, xmm8				; 15D4 _ 66 41: 0F DB. D0
	pxor	xmm1, xmm2				; 15D9 _ 66: 0F EF. CA
	movdqa	xmm2, xmm8				; 15DD _ 66 41: 0F 6F. D0
	pslld	xmm0, 13				; 15E2 _ 66: 0F 72. F0, 0D
	psrld	xmm10, 19				; 15E7 _ 66 41: 0F 72. D2, 13
	por	xmm0, xmm10				; 15ED _ 66 41: 0F EB. C2
	pslld	xmm2, 3 				; 15F2 _ 66: 0F 72. F2, 03
	psrld	xmm8, 29				; 15F7 _ 66 41: 0F 72. D0, 1D
	por	xmm2, xmm8				; 15FD _ 66 41: 0F EB. D0
	movdqa	xmm8, xmm0				; 1602 _ 66 44: 0F 6F. C0
	pxor	xmm11, xmm0				; 1607 _ 66 44: 0F EF. D8
	pxor	xmm11, xmm2				; 160C _ 66 44: 0F EF. DA
	pxor	xmm1, xmm2				; 1611 _ 66: 0F EF. CA
	pslld	xmm8, 3 				; 1615 _ 66 41: 0F 72. F0, 03
	pxor	xmm1, xmm8				; 161B _ 66 41: 0F EF. C8
	movdqa	xmm8, xmm11				; 1620 _ 66 45: 0F 6F. C3
	movdqa	xmm10, xmm1				; 1625 _ 66 44: 0F 6F. D1
	psrld	xmm11, 31				; 162A _ 66 41: 0F 72. D3, 1F
	pslld	xmm8, 1 				; 1630 _ 66 41: 0F 72. F0, 01
	por	xmm8, xmm11				; 1636 _ 66 45: 0F EB. C3
	movdqa	xmm11, xmm8				; 163B _ 66 45: 0F 6F. D8
	pslld	xmm10, 7				; 1640 _ 66 41: 0F 72. F2, 07
	psrld	xmm1, 25				; 1646 _ 66: 0F 72. D1, 19
	por	xmm10, xmm1				; 164B _ 66 44: 0F EB. D1
	pxor	xmm0, xmm8				; 1650 _ 66 41: 0F EF. C0
	pxor	xmm0, xmm10				; 1655 _ 66 41: 0F EF. C2
	pxor	xmm2, xmm10				; 165A _ 66 41: 0F EF. D2
	pslld	xmm11, 7				; 165F _ 66 41: 0F 72. F3, 07
	pxor	xmm2, xmm11				; 1665 _ 66 41: 0F EF. D3
	movdqa	xmm11, xmm0				; 166A _ 66 44: 0F 6F. D8
	movdqa	xmm1, xmm2				; 166F _ 66: 0F 6F. CA
	psrld	xmm0, 27				; 1673 _ 66: 0F 72. D0, 1B
	pslld	xmm11, 5				; 1678 _ 66 41: 0F 72. F3, 05
	por	xmm11, xmm0				; 167E _ 66 44: 0F EB. D8
	pslld	xmm1, 22				; 1683 _ 66: 0F 72. F1, 16
	psrld	xmm2, 10				; 1688 _ 66: 0F 72. D2, 0A
	por	xmm1, xmm2				; 168D _ 66: 0F EB. CA
	movd	xmm2, dword [r12+1370H] 		; 1691 _ 66 41: 0F 6E. 94 24, 00001370
	pshufd	xmm2, xmm2, 0				; 169B _ 66: 0F 70. D2, 00
	pxor	xmm11, xmm2				; 16A0 _ 66 44: 0F EF. DA
	movd	xmm2, dword [r12+1374H] 		; 16A5 _ 66 41: 0F 6E. 94 24, 00001374
	pshufd	xmm2, xmm2, 0				; 16AF _ 66: 0F 70. D2, 00
	pxor	xmm8, xmm2				; 16B4 _ 66 44: 0F EF. C2
	movdqa	xmm0, xmm8				; 16B9 _ 66 41: 0F 6F. C0
	movd	xmm2, dword [r12+1378H] 		; 16BE _ 66 41: 0F 6E. 94 24, 00001378
	pshufd	xmm2, xmm2, 0				; 16C8 _ 66: 0F 70. D2, 00
	pxor	xmm1, xmm2				; 16CD _ 66: 0F EF. CA
	movd	xmm2, dword [r12+137CH] 		; 16D1 _ 66 41: 0F 6E. 94 24, 0000137C
	pshufd	xmm2, xmm2, 0				; 16DB _ 66: 0F 70. D2, 00
	pxor	xmm10, xmm2				; 16E0 _ 66 44: 0F EF. D2
	por	xmm0, xmm1				; 16E5 _ 66: 0F EB. C1
	pxor	xmm0, xmm10				; 16E9 _ 66 41: 0F EF. C2
	pxor	xmm8, xmm1				; 16EE _ 66 44: 0F EF. C1
	pxor	xmm1, xmm0				; 16F3 _ 66: 0F EF. C8
	por	xmm10, xmm8				; 16F7 _ 66 45: 0F EB. D0
	pand	xmm10, xmm11				; 16FC _ 66 45: 0F DB. D3
	pxor	xmm8, xmm1				; 1701 _ 66 44: 0F EF. C1
	pxor	xmm10, xmm0				; 1706 _ 66 44: 0F EF. D0
	por	xmm0, xmm8				; 170B _ 66 41: 0F EB. C0
	pxor	xmm0, xmm11				; 1710 _ 66 41: 0F EF. C3
	por	xmm11, xmm8				; 1715 _ 66 45: 0F EB. D8
	pxor	xmm11, xmm1				; 171A _ 66 44: 0F EF. D9
	pxor	xmm0, xmm8				; 171F _ 66 41: 0F EF. C0
	pxor	xmm1, xmm0				; 1724 _ 66: 0F EF. C8
	pand	xmm0, xmm11				; 1728 _ 66 41: 0F DB. C3
	pxor	xmm0, xmm8				; 172D _ 66 41: 0F EF. C0
	pxor	xmm1, xmm6				; 1732 _ 66: 0F EF. CE
	por	xmm1, xmm11				; 1736 _ 66 41: 0F EB. CB
	pxor	xmm8, xmm1				; 173B _ 66 44: 0F EF. C1
	movdqa	xmm1, xmm8				; 1740 _ 66 41: 0F 6F. C8
	psrld	xmm8, 19				; 1745 _ 66 41: 0F 72. D0, 13
	pslld	xmm1, 13				; 174B _ 66: 0F 72. F1, 0D
	por	xmm1, xmm8				; 1750 _ 66 41: 0F EB. C8
	movdqa	xmm8, xmm0				; 1755 _ 66 44: 0F 6F. C0
	movdqa	xmm2, xmm1				; 175A _ 66: 0F 6F. D1
	psrld	xmm0, 29				; 175E _ 66: 0F 72. D0, 1D
	pslld	xmm8, 3 				; 1763 _ 66 41: 0F 72. F0, 03
	por	xmm8, xmm0				; 1769 _ 66 44: 0F EB. C0
	pxor	xmm10, xmm1				; 176E _ 66 44: 0F EF. D1
	pxor	xmm10, xmm8				; 1773 _ 66 45: 0F EF. D0
	pxor	xmm11, xmm8				; 1778 _ 66 45: 0F EF. D8
	pslld	xmm2, 3 				; 177D _ 66: 0F 72. F2, 03
	pxor	xmm11, xmm2				; 1782 _ 66 44: 0F EF. DA
	movdqa	xmm2, xmm10				; 1787 _ 66 41: 0F 6F. D2
	movdqa	xmm0, xmm11				; 178C _ 66 41: 0F 6F. C3
	psrld	xmm10, 31				; 1791 _ 66 41: 0F 72. D2, 1F
	pslld	xmm2, 1 				; 1797 _ 66: 0F 72. F2, 01
	por	xmm2, xmm10				; 179C _ 66 41: 0F EB. D2
	movdqa	xmm10, xmm2				; 17A1 _ 66 44: 0F 6F. D2
	pslld	xmm0, 7 				; 17A6 _ 66: 0F 72. F0, 07
	psrld	xmm11, 25				; 17AB _ 66 41: 0F 72. D3, 19
	por	xmm0, xmm11				; 17B1 _ 66 41: 0F EB. C3
	pxor	xmm1, xmm2				; 17B6 _ 66: 0F EF. CA
	pxor	xmm1, xmm0				; 17BA _ 66: 0F EF. C8
	pxor	xmm8, xmm0				; 17BE _ 66 44: 0F EF. C0
	pslld	xmm10, 7				; 17C3 _ 66 41: 0F 72. F2, 07
	movd	xmm11, dword [r12+1380H]		; 17C9 _ 66 45: 0F 6E. 9C 24, 00001380
	pxor	xmm8, xmm10				; 17D3 _ 66 45: 0F EF. C2
	movdqa	xmm10, xmm1				; 17D8 _ 66 44: 0F 6F. D1
	psrld	xmm1, 27				; 17DD _ 66: 0F 72. D1, 1B
	pslld	xmm10, 5				; 17E2 _ 66 41: 0F 72. F2, 05
	por	xmm10, xmm1				; 17E8 _ 66 44: 0F EB. D1
	movdqa	xmm1, xmm8				; 17ED _ 66 41: 0F 6F. C8
	psrld	xmm8, 10				; 17F2 _ 66 41: 0F 72. D0, 0A
	pslld	xmm1, 22				; 17F8 _ 66: 0F 72. F1, 16
	por	xmm1, xmm8				; 17FD _ 66 41: 0F EB. C8
	pshufd	xmm8, xmm11, 0				; 1802 _ 66 45: 0F 70. C3, 00
	pxor	xmm10, xmm8				; 1808 _ 66 45: 0F EF. D0
	movd	xmm11, dword [r12+1384H]		; 180D _ 66 45: 0F 6E. 9C 24, 00001384
	pshufd	xmm8, xmm11, 0				; 1817 _ 66 45: 0F 70. C3, 00
	movd	xmm11, dword [r12+1388H]		; 181D _ 66 45: 0F 6E. 9C 24, 00001388
	pxor	xmm2, xmm8				; 1827 _ 66 41: 0F EF. D0
	pshufd	xmm8, xmm11, 0				; 182C _ 66 45: 0F 70. C3, 00
	pxor	xmm1, xmm8				; 1832 _ 66 41: 0F EF. C8
	movd	xmm11, dword [r12+138CH]		; 1837 _ 66 45: 0F 6E. 9C 24, 0000138C
	pshufd	xmm8, xmm11, 0				; 1841 _ 66 45: 0F 70. C3, 00
	movdqa	xmm11, xmm2				; 1847 _ 66 44: 0F 6F. DA
	pxor	xmm0, xmm8				; 184C _ 66 41: 0F EF. C0
	pxor	xmm0, xmm10				; 1851 _ 66 41: 0F EF. C2
	pand	xmm11, xmm0				; 1856 _ 66 44: 0F DB. D8
	pxor	xmm2, xmm1				; 185B _ 66: 0F EF. D1
	pxor	xmm11, xmm10				; 185F _ 66 45: 0F EF. DA
	por	xmm10, xmm0				; 1864 _ 66 44: 0F EB. D0
	pxor	xmm10, xmm2				; 1869 _ 66 44: 0F EF. D2
	pxor	xmm2, xmm0				; 186E _ 66: 0F EF. D0
	pxor	xmm0, xmm1				; 1872 _ 66: 0F EF. C1
	por	xmm1, xmm11				; 1876 _ 66 41: 0F EB. CB
	pxor	xmm1, xmm2				; 187B _ 66: 0F EF. CA
	pxor	xmm2, xmm6				; 187F _ 66: 0F EF. D6
	por	xmm2, xmm11				; 1883 _ 66 41: 0F EB. D3
	pxor	xmm11, xmm0				; 1888 _ 66 44: 0F EF. D8
	pxor	xmm11, xmm2				; 188D _ 66 44: 0F EF. DA
	por	xmm0, xmm10				; 1892 _ 66 41: 0F EB. C2
	pxor	xmm11, xmm0				; 1897 _ 66 44: 0F EF. D8
	movdqa	xmm8, xmm11				; 189C _ 66 45: 0F 6F. C3
	pxor	xmm2, xmm0				; 18A1 _ 66: 0F EF. D0
	psrld	xmm11, 19				; 18A5 _ 66 41: 0F 72. D3, 13
	pslld	xmm8, 13				; 18AB _ 66 41: 0F 72. F0, 0D
	por	xmm8, xmm11				; 18B1 _ 66 45: 0F EB. C3
	movdqa	xmm11, xmm1				; 18B6 _ 66 44: 0F 6F. D9
	movdqa	xmm0, xmm8				; 18BB _ 66 41: 0F 6F. C0
	psrld	xmm1, 29				; 18C0 _ 66: 0F 72. D1, 1D
	pslld	xmm11, 3				; 18C5 _ 66 41: 0F 72. F3, 03
	por	xmm11, xmm1				; 18CB _ 66 44: 0F EB. D9
	pxor	xmm2, xmm8				; 18D0 _ 66 41: 0F EF. D0
	pxor	xmm2, xmm11				; 18D5 _ 66 41: 0F EF. D3
	pxor	xmm10, xmm11				; 18DA _ 66 45: 0F EF. D3
	pslld	xmm0, 3 				; 18DF _ 66: 0F 72. F0, 03
	pxor	xmm10, xmm0				; 18E4 _ 66 44: 0F EF. D0
	movdqa	xmm0, xmm2				; 18E9 _ 66: 0F 6F. C2
	psrld	xmm2, 31				; 18ED _ 66: 0F 72. D2, 1F
	pslld	xmm0, 1 				; 18F2 _ 66: 0F 72. F0, 01
	por	xmm0, xmm2				; 18F7 _ 66: 0F EB. C2
	movdqa	xmm2, xmm10				; 18FB _ 66 41: 0F 6F. D2
	psrld	xmm10, 25				; 1900 _ 66 41: 0F 72. D2, 19
	pxor	xmm8, xmm0				; 1906 _ 66 44: 0F EF. C0
	pslld	xmm2, 7 				; 190B _ 66: 0F 72. F2, 07
	por	xmm2, xmm10				; 1910 _ 66 41: 0F EB. D2
	movdqa	xmm10, xmm0				; 1915 _ 66 44: 0F 6F. D0
	pxor	xmm8, xmm2				; 191A _ 66 44: 0F EF. C2
	pxor	xmm11, xmm2				; 191F _ 66 44: 0F EF. DA
	pslld	xmm10, 7				; 1924 _ 66 41: 0F 72. F2, 07
	pxor	xmm11, xmm10				; 192A _ 66 45: 0F EF. DA
	movdqa	xmm10, xmm8				; 192F _ 66 45: 0F 6F. D0
	psrld	xmm8, 27				; 1934 _ 66 41: 0F 72. D0, 1B
	pslld	xmm10, 5				; 193A _ 66 41: 0F 72. F2, 05
	por	xmm10, xmm8				; 1940 _ 66 45: 0F EB. D0
	movdqa	xmm8, xmm11				; 1945 _ 66 45: 0F 6F. C3
	psrld	xmm11, 10				; 194A _ 66 41: 0F 72. D3, 0A
	pslld	xmm8, 22				; 1950 _ 66 41: 0F 72. F0, 16
	por	xmm8, xmm11				; 1956 _ 66 45: 0F EB. C3
	movd	xmm11, dword [r12+1390H]		; 195B _ 66 45: 0F 6E. 9C 24, 00001390
	pshufd	xmm11, xmm11, 0 			; 1965 _ 66 45: 0F 70. DB, 00
	pxor	xmm10, xmm11				; 196B _ 66 45: 0F EF. D3
	pxor	xmm10, xmm6				; 1970 _ 66 44: 0F EF. D6
	movd	xmm11, dword [r12+1394H]		; 1975 _ 66 45: 0F 6E. 9C 24, 00001394
	pshufd	xmm11, xmm11, 0 			; 197F _ 66 45: 0F 70. DB, 00
	pxor	xmm0, xmm11				; 1985 _ 66 41: 0F EF. C3
	movd	xmm11, dword [r12+1398H]		; 198A _ 66 45: 0F 6E. 9C 24, 00001398
	pshufd	xmm11, xmm11, 0 			; 1994 _ 66 45: 0F 70. DB, 00
	pxor	xmm8, xmm11				; 199A _ 66 45: 0F EF. C3
	movd	xmm11, dword [r12+139CH]		; 199F _ 66 45: 0F 6E. 9C 24, 0000139C
	pshufd	xmm11, xmm11, 0 			; 19A9 _ 66 45: 0F 70. DB, 00
	pxor	xmm2, xmm11				; 19AF _ 66 41: 0F EF. D3
	movdqa	xmm11, xmm10				; 19B4 _ 66 45: 0F 6F. DA
	pxor	xmm8, xmm6				; 19B9 _ 66 44: 0F EF. C6
	pand	xmm11, xmm0				; 19BE _ 66 44: 0F DB. D8
	pxor	xmm8, xmm11				; 19C3 _ 66 45: 0F EF. C3
	por	xmm11, xmm2				; 19C8 _ 66 44: 0F EB. DA
	pxor	xmm2, xmm8				; 19CD _ 66 41: 0F EF. D0
	pxor	xmm0, xmm11				; 19D2 _ 66 41: 0F EF. C3
	pxor	xmm11, xmm10				; 19D7 _ 66 45: 0F EF. DA
	por	xmm10, xmm0				; 19DC _ 66 44: 0F EB. D0
	pxor	xmm0, xmm2				; 19E1 _ 66: 0F EF. C2
	por	xmm8, xmm11				; 19E5 _ 66 45: 0F EB. C3
	pand	xmm8, xmm10				; 19EA _ 66 45: 0F DB. C2
	pxor	xmm11, xmm0				; 19EF _ 66 44: 0F EF. D8
	pand	xmm0, xmm8				; 19F4 _ 66 41: 0F DB. C0
	pxor	xmm0, xmm11				; 19F9 _ 66 41: 0F EF. C3
	pand	xmm11, xmm8				; 19FE _ 66 45: 0F DB. D8
	pxor	xmm10, xmm11				; 1A03 _ 66 45: 0F EF. D3
	movdqa	xmm11, xmm8				; 1A08 _ 66 45: 0F 6F. D8
	psrld	xmm8, 19				; 1A0D _ 66 41: 0F 72. D0, 13
	pslld	xmm11, 13				; 1A13 _ 66 41: 0F 72. F3, 0D
	por	xmm11, xmm8				; 1A19 _ 66 45: 0F EB. D8
	movdqa	xmm8, xmm2				; 1A1E _ 66 44: 0F 6F. C2
	psrld	xmm2, 29				; 1A23 _ 66: 0F 72. D2, 1D
	pxor	xmm10, xmm11				; 1A28 _ 66 45: 0F EF. D3
	pslld	xmm8, 3 				; 1A2D _ 66 41: 0F 72. F0, 03
	por	xmm8, xmm2				; 1A33 _ 66 44: 0F EB. C2
	movdqa	xmm2, xmm11				; 1A38 _ 66 41: 0F 6F. D3
	pxor	xmm10, xmm8				; 1A3D _ 66 45: 0F EF. D0
	movdqa	xmm1, xmm10				; 1A42 _ 66 41: 0F 6F. CA
	pxor	xmm0, xmm8				; 1A47 _ 66 41: 0F EF. C0
	pslld	xmm2, 3 				; 1A4C _ 66: 0F 72. F2, 03
	pxor	xmm0, xmm2				; 1A51 _ 66: 0F EF. C2
	pslld	xmm1, 1 				; 1A55 _ 66: 0F 72. F1, 01
	psrld	xmm10, 31				; 1A5A _ 66 41: 0F 72. D2, 1F
	por	xmm1, xmm10				; 1A60 _ 66 41: 0F EB. CA
	movdqa	xmm10, xmm0				; 1A65 _ 66 44: 0F 6F. D0
	movdqa	xmm2, xmm1				; 1A6A _ 66: 0F 6F. D1
	psrld	xmm0, 25				; 1A6E _ 66: 0F 72. D0, 19
	pslld	xmm10, 7				; 1A73 _ 66 41: 0F 72. F2, 07
	por	xmm10, xmm0				; 1A79 _ 66 44: 0F EB. D0
	pxor	xmm11, xmm1				; 1A7E _ 66 44: 0F EF. D9
	pxor	xmm11, xmm10				; 1A83 _ 66 45: 0F EF. DA
	movdqa	xmm0, xmm11				; 1A88 _ 66 41: 0F 6F. C3
	pxor	xmm8, xmm10				; 1A8D _ 66 45: 0F EF. C2
	pslld	xmm2, 7 				; 1A92 _ 66: 0F 72. F2, 07
	pxor	xmm8, xmm2				; 1A97 _ 66 44: 0F EF. C2
	pslld	xmm0, 5 				; 1A9C _ 66: 0F 72. F0, 05
	psrld	xmm11, 27				; 1AA1 _ 66 41: 0F 72. D3, 1B
	por	xmm0, xmm11				; 1AA7 _ 66 41: 0F EB. C3
	movdqa	xmm11, xmm8				; 1AAC _ 66 45: 0F 6F. D8
	psrld	xmm8, 10				; 1AB1 _ 66 41: 0F 72. D0, 0A
	pslld	xmm11, 22				; 1AB7 _ 66 41: 0F 72. F3, 16
	por	xmm11, xmm8				; 1ABD _ 66 45: 0F EB. D8
	movd	xmm8, dword [r12+13A0H] 		; 1AC2 _ 66 45: 0F 6E. 84 24, 000013A0
	pshufd	xmm2, xmm8, 0				; 1ACC _ 66 41: 0F 70. D0, 00
	pxor	xmm0, xmm2				; 1AD2 _ 66: 0F EF. C2
	movd	xmm8, dword [r12+13A4H] 		; 1AD6 _ 66 45: 0F 6E. 84 24, 000013A4
	pshufd	xmm2, xmm8, 0				; 1AE0 _ 66 41: 0F 70. D0, 00
	pxor	xmm1, xmm2				; 1AE6 _ 66: 0F EF. CA
	movd	xmm8, dword [r12+13A8H] 		; 1AEA _ 66 45: 0F 6E. 84 24, 000013A8
	pshufd	xmm2, xmm8, 0				; 1AF4 _ 66 41: 0F 70. D0, 00
	movd	xmm8, dword [r12+13ACH] 		; 1AFA _ 66 45: 0F 6E. 84 24, 000013AC
	pxor	xmm11, xmm2				; 1B04 _ 66 44: 0F EF. DA
	pshufd	xmm2, xmm8, 0				; 1B09 _ 66 41: 0F 70. D0, 00
	movdqa	xmm8, xmm0				; 1B0F _ 66 44: 0F 6F. C0
	pxor	xmm10, xmm2				; 1B14 _ 66 44: 0F EF. D2
	pand	xmm8, xmm11				; 1B19 _ 66 45: 0F DB. C3
	pxor	xmm8, xmm10				; 1B1E _ 66 45: 0F EF. C2
	pxor	xmm11, xmm1				; 1B23 _ 66 44: 0F EF. D9
	pxor	xmm11, xmm8				; 1B28 _ 66 45: 0F EF. D8
	movdqa	xmm2, xmm11				; 1B2D _ 66 41: 0F 6F. D3
	por	xmm10, xmm0				; 1B32 _ 66 44: 0F EB. D0
	pxor	xmm10, xmm1				; 1B37 _ 66 44: 0F EF. D1
	movdqa	xmm1, xmm10				; 1B3C _ 66 41: 0F 6F. CA
	pxor	xmm0, xmm11				; 1B41 _ 66 41: 0F EF. C3
	pslld	xmm2, 13				; 1B46 _ 66: 0F 72. F2, 0D
	por	xmm1, xmm0				; 1B4B _ 66: 0F EB. C8
	pxor	xmm1, xmm8				; 1B4F _ 66 41: 0F EF. C8
	pand	xmm8, xmm10				; 1B54 _ 66 45: 0F DB. C2
	pxor	xmm0, xmm8				; 1B59 _ 66 41: 0F EF. C0
	pxor	xmm10, xmm1				; 1B5E _ 66 44: 0F EF. D1
	pxor	xmm10, xmm0				; 1B63 _ 66 44: 0F EF. D0
	pxor	xmm0, xmm6				; 1B68 _ 66: 0F EF. C6
	psrld	xmm11, 19				; 1B6C _ 66 41: 0F 72. D3, 13
	por	xmm2, xmm11				; 1B72 _ 66 41: 0F EB. D3
	movdqa	xmm11, xmm10				; 1B77 _ 66 45: 0F 6F. DA
	psrld	xmm10, 29				; 1B7C _ 66 41: 0F 72. D2, 1D
	pxor	xmm1, xmm2				; 1B82 _ 66: 0F EF. CA
	pslld	xmm11, 3				; 1B86 _ 66 41: 0F 72. F3, 03
	por	xmm11, xmm10				; 1B8C _ 66 45: 0F EB. DA
	movdqa	xmm10, xmm2				; 1B91 _ 66 44: 0F 6F. D2
	pxor	xmm1, xmm11				; 1B96 _ 66 41: 0F EF. CB
	movdqa	xmm8, xmm1				; 1B9B _ 66 44: 0F 6F. C1
	pxor	xmm0, xmm11				; 1BA0 _ 66 41: 0F EF. C3
	pslld	xmm10, 3				; 1BA5 _ 66 41: 0F 72. F2, 03
	pxor	xmm0, xmm10				; 1BAB _ 66 41: 0F EF. C2
	pslld	xmm8, 1 				; 1BB0 _ 66 41: 0F 72. F0, 01
	psrld	xmm1, 31				; 1BB6 _ 66: 0F 72. D1, 1F
	por	xmm8, xmm1				; 1BBB _ 66 44: 0F EB. C1
	movdqa	xmm1, xmm0				; 1BC0 _ 66: 0F 6F. C8
	movdqa	xmm10, xmm8				; 1BC4 _ 66 45: 0F 6F. D0
	psrld	xmm0, 25				; 1BC9 _ 66: 0F 72. D0, 19
	pslld	xmm1, 7 				; 1BCE _ 66: 0F 72. F1, 07
	por	xmm1, xmm0				; 1BD3 _ 66: 0F EB. C8
	pxor	xmm2, xmm8				; 1BD7 _ 66 41: 0F EF. D0
	pxor	xmm2, xmm1				; 1BDC _ 66: 0F EF. D1
	pxor	xmm11, xmm1				; 1BE0 _ 66 44: 0F EF. D9
	pslld	xmm10, 7				; 1BE5 _ 66 41: 0F 72. F2, 07
	pxor	xmm11, xmm10				; 1BEB _ 66 45: 0F EF. DA
	movdqa	xmm0, xmm2				; 1BF0 _ 66: 0F 6F. C2
	psrld	xmm2, 27				; 1BF4 _ 66: 0F 72. D2, 1B
	pslld	xmm0, 5 				; 1BF9 _ 66: 0F 72. F0, 05
	por	xmm0, xmm2				; 1BFE _ 66: 0F EB. C2
	movdqa	xmm2, xmm11				; 1C02 _ 66 41: 0F 6F. D3
	movd	xmm10, dword [r12+13B4H]		; 1C07 _ 66 45: 0F 6E. 94 24, 000013B4
	psrld	xmm11, 10				; 1C11 _ 66 41: 0F 72. D3, 0A
	pslld	xmm2, 22				; 1C17 _ 66: 0F 72. F2, 16
	por	xmm2, xmm11				; 1C1C _ 66 41: 0F EB. D3
	movd	xmm11, dword [r12+13B0H]		; 1C21 _ 66 45: 0F 6E. 9C 24, 000013B0
	pshufd	xmm11, xmm11, 0 			; 1C2B _ 66 45: 0F 70. DB, 00
	pxor	xmm0, xmm11				; 1C31 _ 66 41: 0F EF. C3
	pshufd	xmm11, xmm10, 0 			; 1C36 _ 66 45: 0F 70. DA, 00
	movd	xmm10, dword [r12+13B8H]		; 1C3C _ 66 45: 0F 6E. 94 24, 000013B8
	pxor	xmm8, xmm11				; 1C46 _ 66 45: 0F EF. C3
	pshufd	xmm11, xmm10, 0 			; 1C4B _ 66 45: 0F 70. DA, 00
	pxor	xmm2, xmm11				; 1C51 _ 66 41: 0F EF. D3
	movd	xmm10, dword [r12+13BCH]		; 1C56 _ 66 45: 0F 6E. 94 24, 000013BC
	pshufd	xmm11, xmm10, 0 			; 1C60 _ 66 45: 0F 70. DA, 00
	movdqa	xmm10, xmm0				; 1C66 _ 66 44: 0F 6F. D0
	pxor	xmm1, xmm11				; 1C6B _ 66 41: 0F EF. CB
	por	xmm10, xmm1				; 1C70 _ 66 44: 0F EB. D1
	pxor	xmm1, xmm8				; 1C75 _ 66 41: 0F EF. C8
	pand	xmm8, xmm0				; 1C7A _ 66 44: 0F DB. C0
	pxor	xmm0, xmm2				; 1C7F _ 66: 0F EF. C2
	pxor	xmm2, xmm1				; 1C83 _ 66: 0F EF. D1
	pand	xmm1, xmm10				; 1C87 _ 66 41: 0F DB. CA
	por	xmm0, xmm8				; 1C8C _ 66 41: 0F EB. C0
	pxor	xmm1, xmm0				; 1C91 _ 66: 0F EF. C8
	pxor	xmm10, xmm8				; 1C95 _ 66 45: 0F EF. D0
	pand	xmm0, xmm10				; 1C9A _ 66 41: 0F DB. C2
	pxor	xmm8, xmm1				; 1C9F _ 66 44: 0F EF. C1
	pxor	xmm0, xmm2				; 1CA4 _ 66: 0F EF. C2
	por	xmm8, xmm10				; 1CA8 _ 66 45: 0F EB. C2
	pxor	xmm8, xmm2				; 1CAD _ 66 44: 0F EF. C2
	movdqa	xmm2, xmm8				; 1CB2 _ 66 41: 0F 6F. D0
	pxor	xmm10, xmm1				; 1CB7 _ 66 44: 0F EF. D1
	por	xmm2, xmm1				; 1CBC _ 66: 0F EB. D1
	pxor	xmm10, xmm2				; 1CC0 _ 66 44: 0F EF. D2
	movdqa	xmm11, xmm10				; 1CC5 _ 66 45: 0F 6F. DA
	movdqa	xmm2, xmm1				; 1CCA _ 66: 0F 6F. D1
	psrld	xmm10, 19				; 1CCE _ 66 41: 0F 72. D2, 13
	pslld	xmm11, 13				; 1CD4 _ 66 41: 0F 72. F3, 0D
	por	xmm11, xmm10				; 1CDA _ 66 45: 0F EB. DA
	movdqa	xmm10, xmm11				; 1CDF _ 66 45: 0F 6F. D3
	pslld	xmm2, 3 				; 1CE4 _ 66: 0F 72. F2, 03
	psrld	xmm1, 29				; 1CE9 _ 66: 0F 72. D1, 1D
	por	xmm2, xmm1				; 1CEE _ 66: 0F EB. D1
	pxor	xmm8, xmm11				; 1CF2 _ 66 45: 0F EF. C3
	pxor	xmm8, xmm2				; 1CF7 _ 66 44: 0F EF. C2
	movdqa	xmm1, xmm8				; 1CFC _ 66 41: 0F 6F. C8
	pxor	xmm0, xmm2				; 1D01 _ 66: 0F EF. C2
	pslld	xmm10, 3				; 1D05 _ 66 41: 0F 72. F2, 03
	pxor	xmm0, xmm10				; 1D0B _ 66 41: 0F EF. C2
	movdqa	xmm10, xmm0				; 1D10 _ 66 44: 0F 6F. D0
	pslld	xmm1, 1 				; 1D15 _ 66: 0F 72. F1, 01
	psrld	xmm8, 31				; 1D1A _ 66 41: 0F 72. D0, 1F
	por	xmm1, xmm8				; 1D20 _ 66 41: 0F EB. C8
	movdqa	xmm8, xmm1				; 1D25 _ 66 44: 0F 6F. C1
	pslld	xmm10, 7				; 1D2A _ 66 41: 0F 72. F2, 07
	psrld	xmm0, 25				; 1D30 _ 66: 0F 72. D0, 19
	por	xmm10, xmm0				; 1D35 _ 66 44: 0F EB. D0
	pxor	xmm11, xmm1				; 1D3A _ 66 44: 0F EF. D9
	pxor	xmm11, xmm10				; 1D3F _ 66 45: 0F EF. DA
	pxor	xmm2, xmm10				; 1D44 _ 66 41: 0F EF. D2
	pslld	xmm8, 7 				; 1D49 _ 66 41: 0F 72. F0, 07
	pxor	xmm2, xmm8				; 1D4F _ 66 41: 0F EF. D0
	movdqa	xmm8, xmm11				; 1D54 _ 66 45: 0F 6F. C3
	psrld	xmm11, 27				; 1D59 _ 66 41: 0F 72. D3, 1B
	pslld	xmm8, 5 				; 1D5F _ 66 41: 0F 72. F0, 05
	por	xmm8, xmm11				; 1D65 _ 66 45: 0F EB. C3
	movdqa	xmm11, xmm2				; 1D6A _ 66 44: 0F 6F. DA
	psrld	xmm2, 10				; 1D6F _ 66: 0F 72. D2, 0A
	pslld	xmm11, 22				; 1D74 _ 66 41: 0F 72. F3, 16
	por	xmm11, xmm2				; 1D7A _ 66 44: 0F EB. DA
	movd	xmm2, dword [r12+13C0H] 		; 1D7F _ 66 41: 0F 6E. 94 24, 000013C0
	pshufd	xmm2, xmm2, 0				; 1D89 _ 66: 0F 70. D2, 00
	pxor	xmm8, xmm2				; 1D8E _ 66 44: 0F EF. C2
	movd	xmm2, dword [r12+13C4H] 		; 1D93 _ 66 41: 0F 6E. 94 24, 000013C4
	pshufd	xmm2, xmm2, 0				; 1D9D _ 66: 0F 70. D2, 00
	pxor	xmm1, xmm2				; 1DA2 _ 66: 0F EF. CA
	movd	xmm2, dword [r12+13C8H] 		; 1DA6 _ 66 41: 0F 6E. 94 24, 000013C8
	pshufd	xmm2, xmm2, 0				; 1DB0 _ 66: 0F 70. D2, 00
	pxor	xmm11, xmm2				; 1DB5 _ 66 44: 0F EF. DA
	movd	xmm2, dword [r12+13CCH] 		; 1DBA _ 66 41: 0F 6E. 94 24, 000013CC
	pshufd	xmm2, xmm2, 0				; 1DC4 _ 66: 0F 70. D2, 00
	pxor	xmm10, xmm2				; 1DC9 _ 66 44: 0F EF. D2
	pxor	xmm1, xmm10				; 1DCE _ 66 41: 0F EF. CA
	movdqa	xmm0, xmm1				; 1DD3 _ 66: 0F 6F. C1
	pxor	xmm10, xmm6				; 1DD7 _ 66 44: 0F EF. D6
	pxor	xmm11, xmm10				; 1DDC _ 66 45: 0F EF. DA
	pxor	xmm10, xmm8				; 1DE1 _ 66 45: 0F EF. D0
	pand	xmm0, xmm10				; 1DE6 _ 66 41: 0F DB. C2
	pxor	xmm0, xmm11				; 1DEB _ 66 41: 0F EF. C3
	movdqa	xmm2, xmm0				; 1DF0 _ 66: 0F 6F. D0
	pxor	xmm1, xmm10				; 1DF4 _ 66 41: 0F EF. CA
	pxor	xmm8, xmm1				; 1DF9 _ 66 44: 0F EF. C1
	pand	xmm11, xmm1				; 1DFE _ 66 44: 0F DB. D9
	pxor	xmm11, xmm8				; 1E03 _ 66 45: 0F EF. D8
	pand	xmm8, xmm0				; 1E08 _ 66 44: 0F DB. C0
	pxor	xmm10, xmm8				; 1E0D _ 66 45: 0F EF. D0
	por	xmm1, xmm0				; 1E12 _ 66: 0F EB. C8
	pxor	xmm1, xmm8				; 1E16 _ 66 41: 0F EF. C8
	por	xmm8, xmm10				; 1E1B _ 66 45: 0F EB. C2
	pxor	xmm8, xmm11				; 1E20 _ 66 45: 0F EF. C3
	pand	xmm11, xmm10				; 1E25 _ 66 45: 0F DB. DA
	pxor	xmm8, xmm6				; 1E2A _ 66 44: 0F EF. C6
	pxor	xmm1, xmm11				; 1E2F _ 66 41: 0F EF. CB
	movdqa	xmm11, xmm8				; 1E34 _ 66 45: 0F 6F. D8
	pslld	xmm2, 13				; 1E39 _ 66: 0F 72. F2, 0D
	psrld	xmm0, 19				; 1E3E _ 66: 0F 72. D0, 13
	por	xmm2, xmm0				; 1E43 _ 66: 0F EB. D0
	pslld	xmm11, 3				; 1E47 _ 66 41: 0F 72. F3, 03
	psrld	xmm8, 29				; 1E4D _ 66 41: 0F 72. D0, 1D
	por	xmm11, xmm8				; 1E53 _ 66 45: 0F EB. D8
	movdqa	xmm8, xmm2				; 1E58 _ 66 44: 0F 6F. C2
	pxor	xmm1, xmm2				; 1E5D _ 66: 0F EF. CA
	pxor	xmm1, xmm11				; 1E61 _ 66 41: 0F EF. CB
	pxor	xmm10, xmm11				; 1E66 _ 66 45: 0F EF. D3
	pslld	xmm8, 3 				; 1E6B _ 66 41: 0F 72. F0, 03
	pxor	xmm10, xmm8				; 1E71 _ 66 45: 0F EF. D0
	movdqa	xmm8, xmm1				; 1E76 _ 66 44: 0F 6F. C1
	movdqa	xmm0, xmm10				; 1E7B _ 66 41: 0F 6F. C2
	psrld	xmm1, 31				; 1E80 _ 66: 0F 72. D1, 1F
	pslld	xmm8, 1 				; 1E85 _ 66 41: 0F 72. F0, 01
	por	xmm8, xmm1				; 1E8B _ 66 44: 0F EB. C1
	pslld	xmm0, 7 				; 1E90 _ 66: 0F 72. F0, 07
	psrld	xmm10, 25				; 1E95 _ 66 41: 0F 72. D2, 19
	por	xmm0, xmm10				; 1E9B _ 66 41: 0F EB. C2
	movdqa	xmm10, xmm8				; 1EA0 _ 66 45: 0F 6F. D0
	pxor	xmm2, xmm8				; 1EA5 _ 66 41: 0F EF. D0
	pxor	xmm2, xmm0				; 1EAA _ 66: 0F EF. D0
	movdqa	xmm1, xmm2				; 1EAE _ 66: 0F 6F. CA
	pxor	xmm11, xmm0				; 1EB2 _ 66 44: 0F EF. D8
	pslld	xmm10, 7				; 1EB7 _ 66 41: 0F 72. F2, 07
	pxor	xmm11, xmm10				; 1EBD _ 66 45: 0F EF. DA
	pslld	xmm1, 5 				; 1EC2 _ 66: 0F 72. F1, 05
	psrld	xmm2, 27				; 1EC7 _ 66: 0F 72. D2, 1B
	por	xmm1, xmm2				; 1ECC _ 66: 0F EB. CA
	movdqa	xmm2, xmm11				; 1ED0 _ 66 41: 0F 6F. D3
	psrld	xmm11, 10				; 1ED5 _ 66 41: 0F 72. D3, 0A
	pslld	xmm2, 22				; 1EDB _ 66: 0F 72. F2, 16
	por	xmm2, xmm11				; 1EE0 _ 66 41: 0F EB. D3
	movd	xmm11, dword [r12+13D0H]		; 1EE5 _ 66 45: 0F 6E. 9C 24, 000013D0
	pshufd	xmm11, xmm11, 0 			; 1EEF _ 66 45: 0F 70. DB, 00
	pxor	xmm1, xmm11				; 1EF5 _ 66 41: 0F EF. CB
	movd	xmm10, dword [r12+13D4H]		; 1EFA _ 66 45: 0F 6E. 94 24, 000013D4
	pshufd	xmm11, xmm10, 0 			; 1F04 _ 66 45: 0F 70. DA, 00
	pxor	xmm8, xmm11				; 1F0A _ 66 45: 0F EF. C3
	pxor	xmm1, xmm8				; 1F0F _ 66 41: 0F EF. C8
	movd	xmm10, dword [r12+13D8H]		; 1F14 _ 66 45: 0F 6E. 94 24, 000013D8
	pshufd	xmm11, xmm10, 0 			; 1F1E _ 66 45: 0F 70. DA, 00
	pxor	xmm2, xmm11				; 1F24 _ 66 41: 0F EF. D3
	movd	xmm10, dword [r12+13DCH]		; 1F29 _ 66 45: 0F 6E. 94 24, 000013DC
	pshufd	xmm11, xmm10, 0 			; 1F33 _ 66 45: 0F 70. DA, 00
	pxor	xmm0, xmm11				; 1F39 _ 66 41: 0F EF. C3
	pxor	xmm8, xmm0				; 1F3E _ 66 44: 0F EF. C0
	movdqa	xmm10, xmm8				; 1F43 _ 66 45: 0F 6F. D0
	pxor	xmm0, xmm6				; 1F48 _ 66: 0F EF. C6
	pxor	xmm2, xmm0				; 1F4C _ 66: 0F EF. D0
	pand	xmm10, xmm1				; 1F50 _ 66 44: 0F DB. D1
	pxor	xmm10, xmm2				; 1F55 _ 66 44: 0F EF. D2
	movdqa	xmm11, xmm10				; 1F5A _ 66 45: 0F 6F. DA
	por	xmm2, xmm8				; 1F5F _ 66 41: 0F EB. D0
	pxor	xmm8, xmm0				; 1F64 _ 66 44: 0F EF. C0
	pand	xmm0, xmm10				; 1F69 _ 66 41: 0F DB. C2
	pxor	xmm0, xmm1				; 1F6E _ 66: 0F EF. C1
	pxor	xmm8, xmm10				; 1F72 _ 66 45: 0F EF. C2
	pxor	xmm8, xmm2				; 1F77 _ 66 44: 0F EF. C2
	pxor	xmm2, xmm1				; 1F7C _ 66: 0F EF. D1
	pand	xmm1, xmm0				; 1F80 _ 66: 0F DB. C8
	pxor	xmm2, xmm6				; 1F84 _ 66: 0F EF. D6
	pxor	xmm1, xmm8				; 1F88 _ 66 41: 0F EF. C8
	por	xmm8, xmm0				; 1F8D _ 66 44: 0F EB. C0
	pxor	xmm8, xmm2				; 1F92 _ 66 44: 0F EF. C2
	movdqa	xmm2, xmm1				; 1F97 _ 66: 0F 6F. D1
	pslld	xmm11, 13				; 1F9B _ 66 41: 0F 72. F3, 0D
	psrld	xmm10, 19				; 1FA1 _ 66 41: 0F 72. D2, 13
	por	xmm11, xmm10				; 1FA7 _ 66 45: 0F EB. DA
	movdqa	xmm10, xmm11				; 1FAC _ 66 45: 0F 6F. D3
	pslld	xmm2, 3 				; 1FB1 _ 66: 0F 72. F2, 03
	psrld	xmm1, 29				; 1FB6 _ 66: 0F 72. D1, 1D
	por	xmm2, xmm1				; 1FBB _ 66: 0F EB. D1
	pxor	xmm0, xmm11				; 1FBF _ 66 41: 0F EF. C3
	pxor	xmm0, xmm2				; 1FC4 _ 66: 0F EF. C2
	movdqa	xmm1, xmm0				; 1FC8 _ 66: 0F 6F. C8
	pxor	xmm8, xmm2				; 1FCC _ 66 44: 0F EF. C2
	pslld	xmm10, 3				; 1FD1 _ 66 41: 0F 72. F2, 03
	pxor	xmm8, xmm10				; 1FD7 _ 66 45: 0F EF. C2
	movdqa	xmm10, xmm8				; 1FDC _ 66 45: 0F 6F. D0
	pslld	xmm1, 1 				; 1FE1 _ 66: 0F 72. F1, 01
	psrld	xmm0, 31				; 1FE6 _ 66: 0F 72. D0, 1F
	por	xmm1, xmm0				; 1FEB _ 66: 0F EB. C8
	pslld	xmm10, 7				; 1FEF _ 66 41: 0F 72. F2, 07
	psrld	xmm8, 25				; 1FF5 _ 66 41: 0F 72. D0, 19
	por	xmm10, xmm8				; 1FFB _ 66 45: 0F EB. D0
	movdqa	xmm8, xmm1				; 2000 _ 66 44: 0F 6F. C1
	pxor	xmm11, xmm1				; 2005 _ 66 44: 0F EF. D9
	pxor	xmm11, xmm10				; 200A _ 66 45: 0F EF. DA
	pxor	xmm2, xmm10				; 200F _ 66 41: 0F EF. D2
	pslld	xmm8, 7 				; 2014 _ 66 41: 0F 72. F0, 07
	pxor	xmm2, xmm8				; 201A _ 66 41: 0F EF. D0
	movdqa	xmm8, xmm11				; 201F _ 66 45: 0F 6F. C3
	movdqa	xmm0, xmm2				; 2024 _ 66: 0F 6F. C2
	psrld	xmm11, 27				; 2028 _ 66 41: 0F 72. D3, 1B
	pslld	xmm8, 5 				; 202E _ 66 41: 0F 72. F0, 05
	por	xmm8, xmm11				; 2034 _ 66 45: 0F EB. C3
	pslld	xmm0, 22				; 2039 _ 66: 0F 72. F0, 16
	psrld	xmm2, 10				; 203E _ 66: 0F 72. D2, 0A
	por	xmm0, xmm2				; 2043 _ 66: 0F EB. C2
	movd	xmm2, dword [r12+13E0H] 		; 2047 _ 66 41: 0F 6E. 94 24, 000013E0
	pshufd	xmm11, xmm2, 0				; 2051 _ 66 44: 0F 70. DA, 00
	pxor	xmm8, xmm11				; 2057 _ 66 45: 0F EF. C3
	movd	xmm2, dword [r12+13E4H] 		; 205C _ 66 41: 0F 6E. 94 24, 000013E4
	pshufd	xmm11, xmm2, 0				; 2066 _ 66 44: 0F 70. DA, 00
	pxor	xmm1, xmm11				; 206C _ 66 41: 0F EF. CB
	movd	xmm2, dword [r12+13E8H] 		; 2071 _ 66 41: 0F 6E. 94 24, 000013E8
	pshufd	xmm11, xmm2, 0				; 207B _ 66 44: 0F 70. DA, 00
	movd	xmm2, dword [r12+13ECH] 		; 2081 _ 66 41: 0F 6E. 94 24, 000013EC
	pxor	xmm0, xmm11				; 208B _ 66 41: 0F EF. C3
	pshufd	xmm11, xmm2, 0				; 2090 _ 66 44: 0F 70. DA, 00
	pxor	xmm10, xmm11				; 2096 _ 66 45: 0F EF. D3
	movdqa	xmm2, xmm10				; 209B _ 66 41: 0F 6F. D2
	pxor	xmm0, xmm6				; 20A0 _ 66: 0F EF. C6
	pand	xmm2, xmm8				; 20A4 _ 66 41: 0F DB. D0
	pxor	xmm8, xmm10				; 20A9 _ 66 45: 0F EF. C2
	pxor	xmm2, xmm0				; 20AE _ 66: 0F EF. D0
	por	xmm0, xmm10				; 20B2 _ 66 41: 0F EB. C2
	pxor	xmm1, xmm2				; 20B7 _ 66: 0F EF. CA
	pxor	xmm0, xmm8				; 20BB _ 66 41: 0F EF. C0
	por	xmm8, xmm1				; 20C0 _ 66 44: 0F EB. C1
	pxor	xmm0, xmm1				; 20C5 _ 66: 0F EF. C1
	pxor	xmm10, xmm8				; 20C9 _ 66 45: 0F EF. D0
	por	xmm8, xmm2				; 20CE _ 66 44: 0F EB. C2
	pxor	xmm8, xmm0				; 20D3 _ 66 44: 0F EF. C0
	movdqa	xmm11, xmm8				; 20D8 _ 66 45: 0F 6F. D8
	pxor	xmm10, xmm2				; 20DD _ 66 44: 0F EF. D2
	pxor	xmm10, xmm8				; 20E2 _ 66 45: 0F EF. D0
	pxor	xmm2, xmm6				; 20E7 _ 66: 0F EF. D6
	pand	xmm0, xmm10				; 20EB _ 66 41: 0F DB. C2
	pxor	xmm2, xmm0				; 20F0 _ 66: 0F EF. D0
	pslld	xmm11, 13				; 20F4 _ 66 41: 0F 72. F3, 0D
	psrld	xmm8, 19				; 20FA _ 66 41: 0F 72. D0, 13
	por	xmm11, xmm8				; 2100 _ 66 45: 0F EB. D8
	movdqa	xmm8, xmm10				; 2105 _ 66 45: 0F 6F. C2
	psrld	xmm10, 29				; 210A _ 66 41: 0F 72. D2, 1D
	pxor	xmm1, xmm11				; 2110 _ 66 41: 0F EF. CB
	pslld	xmm8, 3 				; 2115 _ 66 41: 0F 72. F0, 03
	por	xmm8, xmm10				; 211B _ 66 45: 0F EB. C2
	movdqa	xmm10, xmm11				; 2120 _ 66 45: 0F 6F. D3
	pxor	xmm1, xmm8				; 2125 _ 66 41: 0F EF. C8
	movdqa	xmm0, xmm1				; 212A _ 66: 0F 6F. C1
	pxor	xmm2, xmm8				; 212E _ 66 41: 0F EF. D0
	pslld	xmm10, 3				; 2133 _ 66 41: 0F 72. F2, 03
	pxor	xmm2, xmm10				; 2139 _ 66 41: 0F EF. D2
	pslld	xmm0, 1 				; 213E _ 66: 0F 72. F0, 01
	psrld	xmm1, 31				; 2143 _ 66: 0F 72. D1, 1F
	por	xmm0, xmm1				; 2148 _ 66: 0F EB. C1
	movdqa	xmm1, xmm2				; 214C _ 66: 0F 6F. CA
	psrld	xmm2, 25				; 2150 _ 66: 0F 72. D2, 19
	pxor	xmm11, xmm0				; 2155 _ 66 44: 0F EF. D8
	pslld	xmm1, 7 				; 215A _ 66: 0F 72. F1, 07
	por	xmm1, xmm2				; 215F _ 66: 0F EB. CA
	movdqa	xmm2, xmm0				; 2163 _ 66: 0F 6F. D0
	pxor	xmm11, xmm1				; 2167 _ 66 44: 0F EF. D9
	pxor	xmm8, xmm1				; 216C _ 66 44: 0F EF. C1
	pslld	xmm2, 7 				; 2171 _ 66: 0F 72. F2, 07
	pxor	xmm8, xmm2				; 2176 _ 66 44: 0F EF. C2
	movdqa	xmm2, xmm11				; 217B _ 66 41: 0F 6F. D3
	movdqa	xmm10, xmm8				; 2180 _ 66 45: 0F 6F. D0
	psrld	xmm11, 27				; 2185 _ 66 41: 0F 72. D3, 1B
	pslld	xmm2, 5 				; 218B _ 66: 0F 72. F2, 05
	por	xmm2, xmm11				; 2190 _ 66 41: 0F EB. D3
	movd	xmm11, dword [r12+13F4H]		; 2195 _ 66 45: 0F 6E. 9C 24, 000013F4
	pslld	xmm10, 22				; 219F _ 66 41: 0F 72. F2, 16
	psrld	xmm8, 10				; 21A5 _ 66 41: 0F 72. D0, 0A
	por	xmm10, xmm8				; 21AB _ 66 45: 0F EB. D0
	movd	xmm8, dword [r12+13F0H] 		; 21B0 _ 66 45: 0F 6E. 84 24, 000013F0
	pshufd	xmm8, xmm8, 0				; 21BA _ 66 45: 0F 70. C0, 00
	pxor	xmm2, xmm8				; 21C0 _ 66 41: 0F EF. D0
	pshufd	xmm8, xmm11, 0				; 21C5 _ 66 45: 0F 70. C3, 00
	movd	xmm11, dword [r12+13F8H]		; 21CB _ 66 45: 0F 6E. 9C 24, 000013F8
	pxor	xmm0, xmm8				; 21D5 _ 66 41: 0F EF. C0
	pshufd	xmm8, xmm11, 0				; 21DA _ 66 45: 0F 70. C3, 00
	pxor	xmm10, xmm8				; 21E0 _ 66 45: 0F EF. D0
	movd	xmm11, dword [r12+13FCH]		; 21E5 _ 66 45: 0F 6E. 9C 24, 000013FC
	pshufd	xmm8, xmm11, 0				; 21EF _ 66 45: 0F 70. C3, 00
	movdqa	xmm11, xmm0				; 21F5 _ 66 44: 0F 6F. D8
	pxor	xmm1, xmm8				; 21FA _ 66 41: 0F EF. C8
	pxor	xmm0, xmm10				; 21FF _ 66 41: 0F EF. C2
	por	xmm11, xmm10				; 2204 _ 66 45: 0F EB. DA
	pxor	xmm11, xmm1				; 2209 _ 66 44: 0F EF. D9
	pxor	xmm10, xmm11				; 220E _ 66 45: 0F EF. D3
	por	xmm1, xmm0				; 2213 _ 66: 0F EB. C8
	pand	xmm1, xmm2				; 2217 _ 66: 0F DB. CA
	pxor	xmm0, xmm10				; 221B _ 66 41: 0F EF. C2
	pxor	xmm1, xmm11				; 2220 _ 66 41: 0F EF. CB
	por	xmm11, xmm0				; 2225 _ 66 44: 0F EB. D8
	pxor	xmm11, xmm2				; 222A _ 66 44: 0F EF. DA
	por	xmm2, xmm0				; 222F _ 66: 0F EB. D0
	pxor	xmm2, xmm10				; 2233 _ 66 41: 0F EF. D2
	pxor	xmm11, xmm0				; 2238 _ 66 44: 0F EF. D8
	pxor	xmm10, xmm11				; 223D _ 66 45: 0F EF. D3
	pand	xmm11, xmm2				; 2242 _ 66 44: 0F DB. DA
	pxor	xmm11, xmm0				; 2247 _ 66 44: 0F EF. D8
	pxor	xmm10, xmm6				; 224C _ 66 44: 0F EF. D6
	por	xmm10, xmm2				; 2251 _ 66 44: 0F EB. D2
	pxor	xmm0, xmm10				; 2256 _ 66 41: 0F EF. C2
	movdqa	xmm8, xmm0				; 225B _ 66 44: 0F 6F. C0
	movdqa	xmm10, xmm11				; 2260 _ 66 45: 0F 6F. D3
	psrld	xmm0, 19				; 2265 _ 66: 0F 72. D0, 13
	pslld	xmm8, 13				; 226A _ 66 41: 0F 72. F0, 0D
	por	xmm8, xmm0				; 2270 _ 66 44: 0F EB. C0
	pslld	xmm10, 3				; 2275 _ 66 41: 0F 72. F2, 03
	psrld	xmm11, 29				; 227B _ 66 41: 0F 72. D3, 1D
	por	xmm10, xmm11				; 2281 _ 66 45: 0F EB. D3
	movdqa	xmm11, xmm8				; 2286 _ 66 45: 0F 6F. D8
	pxor	xmm1, xmm8				; 228B _ 66 41: 0F EF. C8
	pxor	xmm1, xmm10				; 2290 _ 66 41: 0F EF. CA
	pxor	xmm2, xmm10				; 2295 _ 66 41: 0F EF. D2
	pslld	xmm11, 3				; 229A _ 66 41: 0F 72. F3, 03
	pxor	xmm2, xmm11				; 22A0 _ 66 41: 0F EF. D3
	movdqa	xmm11, xmm1				; 22A5 _ 66 44: 0F 6F. D9
	psrld	xmm1, 31				; 22AA _ 66: 0F 72. D1, 1F
	pslld	xmm11, 1				; 22AF _ 66 41: 0F 72. F3, 01
	por	xmm11, xmm1				; 22B5 _ 66 44: 0F EB. D9
	movdqa	xmm1, xmm2				; 22BA _ 66: 0F 6F. CA
	psrld	xmm2, 25				; 22BE _ 66: 0F 72. D2, 19
	pxor	xmm8, xmm11				; 22C3 _ 66 45: 0F EF. C3
	pslld	xmm1, 7 				; 22C8 _ 66: 0F 72. F1, 07
	por	xmm1, xmm2				; 22CD _ 66: 0F EB. CA
	movdqa	xmm2, xmm11				; 22D1 _ 66 41: 0F 6F. D3
	pxor	xmm8, xmm1				; 22D6 _ 66 44: 0F EF. C1
	pxor	xmm10, xmm1				; 22DB _ 66 44: 0F EF. D1
	pslld	xmm2, 7 				; 22E0 _ 66: 0F 72. F2, 07
	pxor	xmm10, xmm2				; 22E5 _ 66 44: 0F EF. D2
	movdqa	xmm2, xmm8				; 22EA _ 66 41: 0F 6F. D0
	psrld	xmm8, 27				; 22EF _ 66 41: 0F 72. D0, 1B
	pslld	xmm2, 5 				; 22F5 _ 66: 0F 72. F2, 05
	por	xmm2, xmm8				; 22FA _ 66 41: 0F EB. D0
	movdqa	xmm8, xmm10				; 22FF _ 66 45: 0F 6F. C2
	psrld	xmm10, 10				; 2304 _ 66 41: 0F 72. D2, 0A
	pslld	xmm8, 22				; 230A _ 66 41: 0F 72. F0, 16
	por	xmm8, xmm10				; 2310 _ 66 45: 0F EB. C2
	movd	xmm10, dword [r12+1400H]		; 2315 _ 66 45: 0F 6E. 94 24, 00001400
	pshufd	xmm10, xmm10, 0 			; 231F _ 66 45: 0F 70. D2, 00
	pxor	xmm2, xmm10				; 2325 _ 66 41: 0F EF. D2
	movd	xmm10, dword [r12+1404H]		; 232A _ 66 45: 0F 6E. 94 24, 00001404
	pshufd	xmm10, xmm10, 0 			; 2334 _ 66 45: 0F 70. D2, 00
	pxor	xmm11, xmm10				; 233A _ 66 45: 0F EF. DA
	movd	xmm10, dword [r12+1408H]		; 233F _ 66 45: 0F 6E. 94 24, 00001408
	pshufd	xmm10, xmm10, 0 			; 2349 _ 66 45: 0F 70. D2, 00
	pxor	xmm8, xmm10				; 234F _ 66 45: 0F EF. C2
	movd	xmm10, dword [r12+140CH]		; 2354 _ 66 45: 0F 6E. 94 24, 0000140C
	pshufd	xmm10, xmm10, 0 			; 235E _ 66 45: 0F 70. D2, 00
	pxor	xmm1, xmm10				; 2364 _ 66 41: 0F EF. CA
	movdqa	xmm10, xmm11				; 2369 _ 66 45: 0F 6F. D3
	pxor	xmm1, xmm2				; 236E _ 66: 0F EF. CA
	pxor	xmm11, xmm8				; 2372 _ 66 45: 0F EF. D8
	pand	xmm10, xmm1				; 2377 _ 66 44: 0F DB. D1
	pxor	xmm10, xmm2				; 237C _ 66 44: 0F EF. D2
	por	xmm2, xmm1				; 2381 _ 66: 0F EB. D1
	pxor	xmm2, xmm11				; 2385 _ 66 41: 0F EF. D3
	pxor	xmm11, xmm1				; 238A _ 66 44: 0F EF. D9
	pxor	xmm1, xmm8				; 238F _ 66 41: 0F EF. C8
	por	xmm8, xmm10				; 2394 _ 66 45: 0F EB. C2
	pxor	xmm8, xmm11				; 2399 _ 66 45: 0F EF. C3
	pxor	xmm11, xmm6				; 239E _ 66 44: 0F EF. DE
	por	xmm11, xmm10				; 23A3 _ 66 45: 0F EB. DA
	pxor	xmm10, xmm1				; 23A8 _ 66 44: 0F EF. D1
	pxor	xmm10, xmm11				; 23AD _ 66 45: 0F EF. D3
	por	xmm1, xmm2				; 23B2 _ 66: 0F EB. CA
	pxor	xmm10, xmm1				; 23B6 _ 66 44: 0F EF. D1
	pxor	xmm11, xmm1				; 23BB _ 66 44: 0F EF. D9
	movdqa	xmm1, xmm10				; 23C0 _ 66 41: 0F 6F. CA
	psrld	xmm10, 19				; 23C5 _ 66 41: 0F 72. D2, 13
	pslld	xmm1, 13				; 23CB _ 66: 0F 72. F1, 0D
	por	xmm1, xmm10				; 23D0 _ 66 41: 0F EB. CA
	movdqa	xmm10, xmm8				; 23D5 _ 66 45: 0F 6F. D0
	psrld	xmm8, 29				; 23DA _ 66 41: 0F 72. D0, 1D
	pxor	xmm11, xmm1				; 23E0 _ 66 44: 0F EF. D9
	pslld	xmm10, 3				; 23E5 _ 66 41: 0F 72. F2, 03
	por	xmm10, xmm8				; 23EB _ 66 45: 0F EB. D0
	movdqa	xmm8, xmm1				; 23F0 _ 66 44: 0F 6F. C1
	pxor	xmm11, xmm10				; 23F5 _ 66 45: 0F EF. DA
	movdqa	xmm0, xmm11				; 23FA _ 66 41: 0F 6F. C3
	pxor	xmm2, xmm10				; 23FF _ 66 41: 0F EF. D2
	pslld	xmm8, 3 				; 2404 _ 66 41: 0F 72. F0, 03
	pxor	xmm2, xmm8				; 240A _ 66 41: 0F EF. D0
	movdqa	xmm8, xmm2				; 240F _ 66 44: 0F 6F. C2
	pslld	xmm0, 1 				; 2414 _ 66: 0F 72. F0, 01
	psrld	xmm11, 31				; 2419 _ 66 41: 0F 72. D3, 1F
	por	xmm0, xmm11				; 241F _ 66 41: 0F EB. C3
	movdqa	xmm11, xmm0				; 2424 _ 66 44: 0F 6F. D8
	pslld	xmm8, 7 				; 2429 _ 66 41: 0F 72. F0, 07
	psrld	xmm2, 25				; 242F _ 66: 0F 72. D2, 19
	por	xmm8, xmm2				; 2434 _ 66 44: 0F EB. C2
	pxor	xmm1, xmm0				; 2439 _ 66: 0F EF. C8
	pxor	xmm1, xmm8				; 243D _ 66 41: 0F EF. C8
	movdqa	xmm2, xmm1				; 2442 _ 66: 0F 6F. D1
	pxor	xmm10, xmm8				; 2446 _ 66 45: 0F EF. D0
	pslld	xmm11, 7				; 244B _ 66 41: 0F 72. F3, 07
	pxor	xmm10, xmm11				; 2451 _ 66 45: 0F EF. D3
	movdqa	xmm11, xmm10				; 2456 _ 66 45: 0F 6F. DA
	pslld	xmm2, 5 				; 245B _ 66: 0F 72. F2, 05
	psrld	xmm1, 27				; 2460 _ 66: 0F 72. D1, 1B
	por	xmm2, xmm1				; 2465 _ 66: 0F EB. D1
	pslld	xmm11, 22				; 2469 _ 66 41: 0F 72. F3, 16
	psrld	xmm10, 10				; 246F _ 66 41: 0F 72. D2, 0A
	por	xmm11, xmm10				; 2475 _ 66 45: 0F EB. DA
	movd	xmm10, dword [r12+1410H]		; 247A _ 66 45: 0F 6E. 94 24, 00001410
	pshufd	xmm10, xmm10, 0 			; 2484 _ 66 45: 0F 70. D2, 00
	pxor	xmm2, xmm10				; 248A _ 66 41: 0F EF. D2
	pxor	xmm2, xmm6				; 248F _ 66: 0F EF. D6
	movd	xmm10, dword [r12+1414H]		; 2493 _ 66 45: 0F 6E. 94 24, 00001414
	pshufd	xmm10, xmm10, 0 			; 249D _ 66 45: 0F 70. D2, 00
	pxor	xmm0, xmm10				; 24A3 _ 66 41: 0F EF. C2
	movd	xmm10, dword [r12+1418H]		; 24A8 _ 66 45: 0F 6E. 94 24, 00001418
	pshufd	xmm10, xmm10, 0 			; 24B2 _ 66 45: 0F 70. D2, 00
	pxor	xmm11, xmm10				; 24B8 _ 66 45: 0F EF. DA
	pxor	xmm11, xmm6				; 24BD _ 66 44: 0F EF. DE
	movd	xmm10, dword [r12+141CH]		; 24C2 _ 66 45: 0F 6E. 94 24, 0000141C
	pshufd	xmm10, xmm10, 0 			; 24CC _ 66 45: 0F 70. D2, 00
	pxor	xmm8, xmm10				; 24D2 _ 66 45: 0F EF. C2
	movdqa	xmm10, xmm2				; 24D7 _ 66 44: 0F 6F. D2
	pand	xmm10, xmm0				; 24DC _ 66 44: 0F DB. D0
	pxor	xmm11, xmm10				; 24E1 _ 66 45: 0F EF. DA
	por	xmm10, xmm8				; 24E6 _ 66 45: 0F EB. D0
	pxor	xmm8, xmm11				; 24EB _ 66 45: 0F EF. C3
	pxor	xmm0, xmm10				; 24F0 _ 66 41: 0F EF. C2
	pxor	xmm10, xmm2				; 24F5 _ 66 44: 0F EF. D2
	por	xmm2, xmm0				; 24FA _ 66: 0F EB. D0
	pxor	xmm0, xmm8				; 24FE _ 66 41: 0F EF. C0
	por	xmm11, xmm10				; 2503 _ 66 45: 0F EB. DA
	pand	xmm11, xmm2				; 2508 _ 66 44: 0F DB. DA
	movdqa	xmm1, xmm11				; 250D _ 66 41: 0F 6F. CB
	pxor	xmm10, xmm0				; 2512 _ 66 44: 0F EF. D0
	pand	xmm0, xmm11				; 2517 _ 66 41: 0F DB. C3
	pxor	xmm0, xmm10				; 251C _ 66 41: 0F EF. C2
	pand	xmm10, xmm11				; 2521 _ 66 45: 0F DB. D3
	pxor	xmm2, xmm10				; 2526 _ 66 41: 0F EF. D2
	movdqa	xmm10, xmm8				; 252B _ 66 45: 0F 6F. D0
	pslld	xmm1, 13				; 2530 _ 66: 0F 72. F1, 0D
	psrld	xmm11, 19				; 2535 _ 66 41: 0F 72. D3, 13
	por	xmm1, xmm11				; 253B _ 66 41: 0F EB. CB
	pslld	xmm10, 3				; 2540 _ 66 41: 0F 72. F2, 03
	psrld	xmm8, 29				; 2546 _ 66 41: 0F 72. D0, 1D
	por	xmm10, xmm8				; 254C _ 66 45: 0F EB. D0
	movdqa	xmm8, xmm1				; 2551 _ 66 44: 0F 6F. C1
	pxor	xmm2, xmm1				; 2556 _ 66: 0F EF. D1
	pxor	xmm2, xmm10				; 255A _ 66 41: 0F EF. D2
	pxor	xmm0, xmm10				; 255F _ 66 41: 0F EF. C2
	pslld	xmm8, 3 				; 2564 _ 66 41: 0F 72. F0, 03
	pxor	xmm0, xmm8				; 256A _ 66 41: 0F EF. C0
	movdqa	xmm8, xmm2				; 256F _ 66 44: 0F 6F. C2
	psrld	xmm2, 31				; 2574 _ 66: 0F 72. D2, 1F
	pslld	xmm8, 1 				; 2579 _ 66 41: 0F 72. F0, 01
	por	xmm8, xmm2				; 257F _ 66 44: 0F EB. C2
	movdqa	xmm2, xmm0				; 2584 _ 66: 0F 6F. D0
	movdqa	xmm11, xmm8				; 2588 _ 66 45: 0F 6F. D8
	psrld	xmm0, 25				; 258D _ 66: 0F 72. D0, 19
	pslld	xmm2, 7 				; 2592 _ 66: 0F 72. F2, 07
	por	xmm2, xmm0				; 2597 _ 66: 0F EB. D0
	pxor	xmm1, xmm8				; 259B _ 66 41: 0F EF. C8
	pxor	xmm1, xmm2				; 25A0 _ 66: 0F EF. CA
	pxor	xmm10, xmm2				; 25A4 _ 66 44: 0F EF. D2
	pslld	xmm11, 7				; 25A9 _ 66 41: 0F 72. F3, 07
	pxor	xmm10, xmm11				; 25AF _ 66 45: 0F EF. D3
	movdqa	xmm11, xmm1				; 25B4 _ 66 44: 0F 6F. D9
	movdqa	xmm0, xmm10				; 25B9 _ 66 41: 0F 6F. C2
	psrld	xmm1, 27				; 25BE _ 66: 0F 72. D1, 1B
	pslld	xmm11, 5				; 25C3 _ 66 41: 0F 72. F3, 05
	por	xmm11, xmm1				; 25C9 _ 66 44: 0F EB. D9
	pslld	xmm0, 22				; 25CE _ 66: 0F 72. F0, 16
	psrld	xmm10, 10				; 25D3 _ 66 41: 0F 72. D2, 0A
	por	xmm0, xmm10				; 25D9 _ 66 41: 0F EB. C2
	movd	xmm10, dword [r12+1420H]		; 25DE _ 66 45: 0F 6E. 94 24, 00001420
	pshufd	xmm10, xmm10, 0 			; 25E8 _ 66 45: 0F 70. D2, 00
	pxor	xmm11, xmm10				; 25EE _ 66 45: 0F EF. DA
	movd	xmm10, dword [r12+1424H]		; 25F3 _ 66 45: 0F 6E. 94 24, 00001424
	pshufd	xmm10, xmm10, 0 			; 25FD _ 66 45: 0F 70. D2, 00
	pxor	xmm8, xmm10				; 2603 _ 66 45: 0F EF. C2
	movd	xmm10, dword [r12+1428H]		; 2608 _ 66 45: 0F 6E. 94 24, 00001428
	pshufd	xmm10, xmm10, 0 			; 2612 _ 66 45: 0F 70. D2, 00
	pxor	xmm0, xmm10				; 2618 _ 66 41: 0F EF. C2
	movd	xmm10, dword [r12+142CH]		; 261D _ 66 45: 0F 6E. 94 24, 0000142C
	pshufd	xmm10, xmm10, 0 			; 2627 _ 66 45: 0F 70. D2, 00
	pxor	xmm2, xmm10				; 262D _ 66 41: 0F EF. D2
	movdqa	xmm10, xmm11				; 2632 _ 66 45: 0F 6F. D3
	pand	xmm10, xmm0				; 2637 _ 66 44: 0F DB. D0
	pxor	xmm10, xmm2				; 263C _ 66 44: 0F EF. D2
	pxor	xmm0, xmm8				; 2641 _ 66 41: 0F EF. C0
	pxor	xmm0, xmm10				; 2646 _ 66 41: 0F EF. C2
	movdqa	xmm1, xmm0				; 264B _ 66: 0F 6F. C8
	por	xmm2, xmm11				; 264F _ 66 41: 0F EB. D3
	pxor	xmm2, xmm8				; 2654 _ 66 41: 0F EF. D0
	movdqa	xmm8, xmm2				; 2659 _ 66 44: 0F 6F. C2
	pxor	xmm11, xmm0				; 265E _ 66 44: 0F EF. D8
	pslld	xmm1, 13				; 2663 _ 66: 0F 72. F1, 0D
	por	xmm8, xmm11				; 2668 _ 66 45: 0F EB. C3
	pxor	xmm8, xmm10				; 266D _ 66 45: 0F EF. C2
	pand	xmm10, xmm2				; 2672 _ 66 44: 0F DB. D2
	pxor	xmm11, xmm10				; 2677 _ 66 45: 0F EF. DA
	pxor	xmm2, xmm8				; 267C _ 66 41: 0F EF. D0
	pxor	xmm2, xmm11				; 2681 _ 66 41: 0F EF. D3
	pxor	xmm11, xmm6				; 2686 _ 66 44: 0F EF. DE
	psrld	xmm0, 19				; 268B _ 66: 0F 72. D0, 13
	por	xmm1, xmm0				; 2690 _ 66: 0F EB. C8
	movdqa	xmm0, xmm2				; 2694 _ 66: 0F 6F. C2
	psrld	xmm2, 29				; 2698 _ 66: 0F 72. D2, 1D
	pxor	xmm8, xmm1				; 269D _ 66 44: 0F EF. C1
	pslld	xmm0, 3 				; 26A2 _ 66: 0F 72. F0, 03
	por	xmm0, xmm2				; 26A7 _ 66: 0F EB. C2
	movdqa	xmm2, xmm1				; 26AB _ 66: 0F 6F. D1
	pxor	xmm8, xmm0				; 26AF _ 66 44: 0F EF. C0
	pxor	xmm11, xmm0				; 26B4 _ 66 44: 0F EF. D8
	pslld	xmm2, 3 				; 26B9 _ 66: 0F 72. F2, 03
	pxor	xmm11, xmm2				; 26BE _ 66 44: 0F EF. DA
	movdqa	xmm2, xmm8				; 26C3 _ 66 41: 0F 6F. D0
	movdqa	xmm10, xmm11				; 26C8 _ 66 45: 0F 6F. D3
	psrld	xmm8, 31				; 26CD _ 66 41: 0F 72. D0, 1F
	pslld	xmm2, 1 				; 26D3 _ 66: 0F 72. F2, 01
	por	xmm2, xmm8				; 26D8 _ 66 41: 0F EB. D0
	movdqa	xmm8, xmm2				; 26DD _ 66 44: 0F 6F. C2
	pslld	xmm10, 7				; 26E2 _ 66 41: 0F 72. F2, 07
	psrld	xmm11, 25				; 26E8 _ 66 41: 0F 72. D3, 19
	por	xmm10, xmm11				; 26EE _ 66 45: 0F EB. D3
	pxor	xmm1, xmm2				; 26F3 _ 66: 0F EF. CA
	movd	xmm11, dword [r12+1430H]		; 26F7 _ 66 45: 0F 6E. 9C 24, 00001430
	pxor	xmm1, xmm10				; 2701 _ 66 41: 0F EF. CA
	pxor	xmm0, xmm10				; 2706 _ 66 41: 0F EF. C2
	pslld	xmm8, 7 				; 270B _ 66 41: 0F 72. F0, 07
	pxor	xmm0, xmm8				; 2711 _ 66 41: 0F EF. C0
	movdqa	xmm8, xmm1				; 2716 _ 66 44: 0F 6F. C1
	psrld	xmm1, 27				; 271B _ 66: 0F 72. D1, 1B
	pshufd	xmm11, xmm11, 0 			; 2720 _ 66 45: 0F 70. DB, 00
	pslld	xmm8, 5 				; 2726 _ 66 41: 0F 72. F0, 05
	por	xmm8, xmm1				; 272C _ 66 44: 0F EB. C1
	movdqa	xmm1, xmm0				; 2731 _ 66: 0F 6F. C8
	psrld	xmm0, 10				; 2735 _ 66: 0F 72. D0, 0A
	pxor	xmm8, xmm11				; 273A _ 66 45: 0F EF. C3
	movd	xmm11, dword [r12+1434H]		; 273F _ 66 45: 0F 6E. 9C 24, 00001434
	pslld	xmm1, 22				; 2749 _ 66: 0F 72. F1, 16
	por	xmm1, xmm0				; 274E _ 66: 0F EB. C8
	pshufd	xmm11, xmm11, 0 			; 2752 _ 66 45: 0F 70. DB, 00
	pxor	xmm2, xmm11				; 2758 _ 66 41: 0F EF. D3
	movd	xmm11, dword [r12+1438H]		; 275D _ 66 45: 0F 6E. 9C 24, 00001438
	pshufd	xmm11, xmm11, 0 			; 2767 _ 66 45: 0F 70. DB, 00
	pxor	xmm1, xmm11				; 276D _ 66 41: 0F EF. CB
	movd	xmm11, dword [r12+143CH]		; 2772 _ 66 45: 0F 6E. 9C 24, 0000143C
	pshufd	xmm11, xmm11, 0 			; 277C _ 66 45: 0F 70. DB, 00
	pxor	xmm10, xmm11				; 2782 _ 66 45: 0F EF. D3
	movdqa	xmm11, xmm8				; 2787 _ 66 45: 0F 6F. D8
	por	xmm11, xmm10				; 278C _ 66 45: 0F EB. DA
	pxor	xmm10, xmm2				; 2791 _ 66 44: 0F EF. D2
	pand	xmm2, xmm8				; 2796 _ 66 41: 0F DB. D0
	pxor	xmm8, xmm1				; 279B _ 66 44: 0F EF. C1
	pxor	xmm1, xmm10				; 27A0 _ 66 41: 0F EF. CA
	pand	xmm10, xmm11				; 27A5 _ 66 45: 0F DB. D3
	por	xmm8, xmm2				; 27AA _ 66 44: 0F EB. C2
	pxor	xmm10, xmm8				; 27AF _ 66 45: 0F EF. D0
	pxor	xmm11, xmm2				; 27B4 _ 66 44: 0F EF. DA
	pand	xmm8, xmm11				; 27B9 _ 66 45: 0F DB. C3
	pxor	xmm2, xmm10				; 27BE _ 66 41: 0F EF. D2
	pxor	xmm8, xmm1				; 27C3 _ 66 44: 0F EF. C1
	por	xmm2, xmm11				; 27C8 _ 66 41: 0F EB. D3
	pxor	xmm2, xmm1				; 27CD _ 66: 0F EF. D1
	movdqa	xmm0, xmm2				; 27D1 _ 66: 0F 6F. C2
	pxor	xmm11, xmm10				; 27D5 _ 66 45: 0F EF. DA
	por	xmm0, xmm10				; 27DA _ 66 41: 0F EB. C2
	pxor	xmm11, xmm0				; 27DF _ 66 44: 0F EF. D8
	movdqa	xmm1, xmm11				; 27E4 _ 66 41: 0F 6F. CB
	psrld	xmm11, 19				; 27E9 _ 66 41: 0F 72. D3, 13
	pslld	xmm1, 13				; 27EF _ 66: 0F 72. F1, 0D
	por	xmm1, xmm11				; 27F4 _ 66 41: 0F EB. CB
	movdqa	xmm11, xmm10				; 27F9 _ 66 45: 0F 6F. DA
	psrld	xmm10, 29				; 27FE _ 66 41: 0F 72. D2, 1D
	pxor	xmm2, xmm1				; 2804 _ 66: 0F EF. D1
	pslld	xmm11, 3				; 2808 _ 66 41: 0F 72. F3, 03
	por	xmm11, xmm10				; 280E _ 66 45: 0F EB. DA
	movdqa	xmm10, xmm1				; 2813 _ 66 44: 0F 6F. D1
	pxor	xmm2, xmm11				; 2818 _ 66 41: 0F EF. D3
	movdqa	xmm0, xmm2				; 281D _ 66: 0F 6F. C2
	pxor	xmm8, xmm11				; 2821 _ 66 45: 0F EF. C3
	pslld	xmm10, 3				; 2826 _ 66 41: 0F 72. F2, 03
	pxor	xmm8, xmm10				; 282C _ 66 45: 0F EF. C2
	movdqa	xmm10, xmm8				; 2831 _ 66 45: 0F 6F. D0
	pslld	xmm0, 1 				; 2836 _ 66: 0F 72. F0, 01
	psrld	xmm2, 31				; 283B _ 66: 0F 72. D2, 1F
	por	xmm0, xmm2				; 2840 _ 66: 0F EB. C2
	movdqa	xmm2, xmm0				; 2844 _ 66: 0F 6F. D0
	pslld	xmm10, 7				; 2848 _ 66 41: 0F 72. F2, 07
	psrld	xmm8, 25				; 284E _ 66 41: 0F 72. D0, 19
	por	xmm10, xmm8				; 2854 _ 66 45: 0F EB. D0
	pxor	xmm1, xmm0				; 2859 _ 66: 0F EF. C8
	pxor	xmm1, xmm10				; 285D _ 66 41: 0F EF. CA
	pxor	xmm11, xmm10				; 2862 _ 66 45: 0F EF. DA
	pslld	xmm2, 7 				; 2867 _ 66: 0F 72. F2, 07
	pxor	xmm11, xmm2				; 286C _ 66 44: 0F EF. DA
	movdqa	xmm2, xmm1				; 2871 _ 66: 0F 6F. D1
	movdqa	xmm8, xmm11				; 2875 _ 66 45: 0F 6F. C3
	psrld	xmm1, 27				; 287A _ 66: 0F 72. D1, 1B
	pslld	xmm2, 5 				; 287F _ 66: 0F 72. F2, 05
	por	xmm2, xmm1				; 2884 _ 66: 0F EB. D1
	pslld	xmm8, 22				; 2888 _ 66 41: 0F 72. F0, 16
	psrld	xmm11, 10				; 288E _ 66 41: 0F 72. D3, 0A
	por	xmm8, xmm11				; 2894 _ 66 45: 0F EB. C3
	movd	xmm11, dword [r12+1440H]		; 2899 _ 66 45: 0F 6E. 9C 24, 00001440
	pshufd	xmm11, xmm11, 0 			; 28A3 _ 66 45: 0F 70. DB, 00
	pxor	xmm2, xmm11				; 28A9 _ 66 41: 0F EF. D3
	movd	xmm11, dword [r12+1444H]		; 28AE _ 66 45: 0F 6E. 9C 24, 00001444
	pshufd	xmm11, xmm11, 0 			; 28B8 _ 66 45: 0F 70. DB, 00
	pxor	xmm0, xmm11				; 28BE _ 66 41: 0F EF. C3
	movd	xmm11, dword [r12+1448H]		; 28C3 _ 66 45: 0F 6E. 9C 24, 00001448
	pshufd	xmm11, xmm11, 0 			; 28CD _ 66 45: 0F 70. DB, 00
	pxor	xmm8, xmm11				; 28D3 _ 66 45: 0F EF. C3
	movd	xmm11, dword [r12+144CH]		; 28D8 _ 66 45: 0F 6E. 9C 24, 0000144C
	pshufd	xmm11, xmm11, 0 			; 28E2 _ 66 45: 0F 70. DB, 00
	pxor	xmm10, xmm11				; 28E8 _ 66 45: 0F EF. D3
	pxor	xmm0, xmm10				; 28ED _ 66 41: 0F EF. C2
	movdqa	xmm11, xmm0				; 28F2 _ 66 44: 0F 6F. D8
	pxor	xmm10, xmm6				; 28F7 _ 66 44: 0F EF. D6
	pxor	xmm8, xmm10				; 28FC _ 66 45: 0F EF. C2
	pxor	xmm10, xmm2				; 2901 _ 66 44: 0F EF. D2
	pand	xmm11, xmm10				; 2906 _ 66 45: 0F DB. DA
	pxor	xmm11, xmm8				; 290B _ 66 45: 0F EF. D8
	movdqa	xmm1, xmm11				; 2910 _ 66 41: 0F 6F. CB
	pxor	xmm0, xmm10				; 2915 _ 66 41: 0F EF. C2
	pxor	xmm2, xmm0				; 291A _ 66: 0F EF. D0
	pand	xmm8, xmm0				; 291E _ 66 44: 0F DB. C0
	pxor	xmm8, xmm2				; 2923 _ 66 44: 0F EF. C2
	pand	xmm2, xmm11				; 2928 _ 66 41: 0F DB. D3
	pxor	xmm10, xmm2				; 292D _ 66 44: 0F EF. D2
	por	xmm0, xmm11				; 2932 _ 66 41: 0F EB. C3
	pxor	xmm0, xmm2				; 2937 _ 66: 0F EF. C2
	por	xmm2, xmm10				; 293B _ 66 41: 0F EB. D2
	pxor	xmm2, xmm8				; 2940 _ 66 41: 0F EF. D0
	pand	xmm8, xmm10				; 2945 _ 66 45: 0F DB. C2
	pxor	xmm2, xmm6				; 294A _ 66: 0F EF. D6
	pxor	xmm0, xmm8				; 294E _ 66 41: 0F EF. C0
	movdqa	xmm8, xmm2				; 2953 _ 66 44: 0F 6F. C2
	pslld	xmm1, 13				; 2958 _ 66: 0F 72. F1, 0D
	psrld	xmm11, 19				; 295D _ 66 41: 0F 72. D3, 13
	por	xmm1, xmm11				; 2963 _ 66 41: 0F EB. CB
	pslld	xmm8, 3 				; 2968 _ 66 41: 0F 72. F0, 03
	psrld	xmm2, 29				; 296E _ 66: 0F 72. D2, 1D
	por	xmm8, xmm2				; 2973 _ 66 44: 0F EB. C2
	movdqa	xmm2, xmm1				; 2978 _ 66: 0F 6F. D1
	pxor	xmm0, xmm1				; 297C _ 66: 0F EF. C1
	pxor	xmm0, xmm8				; 2980 _ 66 41: 0F EF. C0
	movdqa	xmm11, xmm0				; 2985 _ 66 44: 0F 6F. D8
	pxor	xmm10, xmm8				; 298A _ 66 45: 0F EF. D0
	pslld	xmm2, 3 				; 298F _ 66: 0F 72. F2, 03
	pxor	xmm10, xmm2				; 2994 _ 66 44: 0F EF. D2
	movdqa	xmm2, xmm10				; 2999 _ 66 41: 0F 6F. D2
	pslld	xmm11, 1				; 299E _ 66 41: 0F 72. F3, 01
	psrld	xmm0, 31				; 29A4 _ 66: 0F 72. D0, 1F
	por	xmm11, xmm0				; 29A9 _ 66 44: 0F EB. D8
	pslld	xmm2, 7 				; 29AE _ 66: 0F 72. F2, 07
	psrld	xmm10, 25				; 29B3 _ 66 41: 0F 72. D2, 19
	por	xmm2, xmm10				; 29B9 _ 66 41: 0F EB. D2
	movdqa	xmm10, xmm11				; 29BE _ 66 45: 0F 6F. D3
	pxor	xmm1, xmm11				; 29C3 _ 66 41: 0F EF. CB
	pxor	xmm1, xmm2				; 29C8 _ 66: 0F EF. CA
	movdqa	xmm0, xmm1				; 29CC _ 66: 0F 6F. C1
	pxor	xmm8, xmm2				; 29D0 _ 66 44: 0F EF. C2
	pslld	xmm10, 7				; 29D5 _ 66 41: 0F 72. F2, 07
	pxor	xmm8, xmm10				; 29DB _ 66 45: 0F EF. C2
	movdqa	xmm10, xmm8				; 29E0 _ 66 45: 0F 6F. D0
	pslld	xmm0, 5 				; 29E5 _ 66: 0F 72. F0, 05
	psrld	xmm1, 27				; 29EA _ 66: 0F 72. D1, 1B
	por	xmm0, xmm1				; 29EF _ 66: 0F EB. C1
	pslld	xmm10, 22				; 29F3 _ 66 41: 0F 72. F2, 16
	psrld	xmm8, 10				; 29F9 _ 66 41: 0F 72. D0, 0A
	por	xmm10, xmm8				; 29FF _ 66 45: 0F EB. D0
	movd	xmm8, dword [r12+1450H] 		; 2A04 _ 66 45: 0F 6E. 84 24, 00001450
	pshufd	xmm8, xmm8, 0				; 2A0E _ 66 45: 0F 70. C0, 00
	pxor	xmm0, xmm8				; 2A14 _ 66 41: 0F EF. C0
	movd	xmm8, dword [r12+1454H] 		; 2A19 _ 66 45: 0F 6E. 84 24, 00001454
	pshufd	xmm8, xmm8, 0				; 2A23 _ 66 45: 0F 70. C0, 00
	pxor	xmm11, xmm8				; 2A29 _ 66 45: 0F EF. D8
	pxor	xmm0, xmm11				; 2A2E _ 66 41: 0F EF. C3
	movd	xmm8, dword [r12+1458H] 		; 2A33 _ 66 45: 0F 6E. 84 24, 00001458
	pshufd	xmm8, xmm8, 0				; 2A3D _ 66 45: 0F 70. C0, 00
	pxor	xmm10, xmm8				; 2A43 _ 66 45: 0F EF. D0
	movd	xmm8, dword [r12+145CH] 		; 2A48 _ 66 45: 0F 6E. 84 24, 0000145C
	pshufd	xmm8, xmm8, 0				; 2A52 _ 66 45: 0F 70. C0, 00
	pxor	xmm2, xmm8				; 2A58 _ 66 41: 0F EF. D0
	pxor	xmm11, xmm2				; 2A5D _ 66 44: 0F EF. DA
	movdqa	xmm8, xmm11				; 2A62 _ 66 45: 0F 6F. C3
	pxor	xmm2, xmm6				; 2A67 _ 66: 0F EF. D6
	pxor	xmm10, xmm2				; 2A6B _ 66 44: 0F EF. D2
	pand	xmm8, xmm0				; 2A70 _ 66 44: 0F DB. C0
	pxor	xmm8, xmm10				; 2A75 _ 66 45: 0F EF. C2
	movdqa	xmm1, xmm8				; 2A7A _ 66 41: 0F 6F. C8
	por	xmm10, xmm11				; 2A7F _ 66 45: 0F EB. D3
	pxor	xmm11, xmm2				; 2A84 _ 66 44: 0F EF. DA
	pand	xmm2, xmm8				; 2A89 _ 66 41: 0F DB. D0
	pxor	xmm2, xmm0				; 2A8E _ 66: 0F EF. D0
	pxor	xmm11, xmm8				; 2A92 _ 66 45: 0F EF. D8
	pxor	xmm11, xmm10				; 2A97 _ 66 45: 0F EF. DA
	pxor	xmm10, xmm0				; 2A9C _ 66 44: 0F EF. D0
	pand	xmm0, xmm2				; 2AA1 _ 66: 0F DB. C2
	pxor	xmm10, xmm6				; 2AA5 _ 66 44: 0F EF. D6
	pxor	xmm0, xmm11				; 2AAA _ 66 41: 0F EF. C3
	por	xmm11, xmm2				; 2AAF _ 66 44: 0F EB. DA
	pxor	xmm11, xmm10				; 2AB4 _ 66 45: 0F EF. DA
	movdqa	xmm10, xmm0				; 2AB9 _ 66 44: 0F 6F. D0
	pslld	xmm1, 13				; 2ABE _ 66: 0F 72. F1, 0D
	psrld	xmm8, 19				; 2AC3 _ 66 41: 0F 72. D0, 13
	por	xmm1, xmm8				; 2AC9 _ 66 41: 0F EB. C8
	movdqa	xmm8, xmm1				; 2ACE _ 66 44: 0F 6F. C1
	pslld	xmm10, 3				; 2AD3 _ 66 41: 0F 72. F2, 03
	psrld	xmm0, 29				; 2AD9 _ 66: 0F 72. D0, 1D
	por	xmm10, xmm0				; 2ADE _ 66 44: 0F EB. D0
	pxor	xmm2, xmm1				; 2AE3 _ 66: 0F EF. D1
	pxor	xmm2, xmm10				; 2AE7 _ 66 41: 0F EF. D2
	pxor	xmm11, xmm10				; 2AEC _ 66 45: 0F EF. DA
	pslld	xmm8, 3 				; 2AF1 _ 66 41: 0F 72. F0, 03
	pxor	xmm11, xmm8				; 2AF7 _ 66 45: 0F EF. D8
	movdqa	xmm8, xmm2				; 2AFC _ 66 44: 0F 6F. C2
	psrld	xmm2, 31				; 2B01 _ 66: 0F 72. D2, 1F
	pslld	xmm8, 1 				; 2B06 _ 66 41: 0F 72. F0, 01
	por	xmm8, xmm2				; 2B0C _ 66 44: 0F EB. C2
	movdqa	xmm2, xmm11				; 2B11 _ 66 41: 0F 6F. D3
	psrld	xmm11, 25				; 2B16 _ 66 41: 0F 72. D3, 19
	pxor	xmm1, xmm8				; 2B1C _ 66 41: 0F EF. C8
	pslld	xmm2, 7 				; 2B21 _ 66: 0F 72. F2, 07
	por	xmm2, xmm11				; 2B26 _ 66 41: 0F EB. D3
	movdqa	xmm11, xmm8				; 2B2B _ 66 45: 0F 6F. D8
	pxor	xmm1, xmm2				; 2B30 _ 66: 0F EF. CA
	pxor	xmm10, xmm2				; 2B34 _ 66 44: 0F EF. D2
	pslld	xmm11, 7				; 2B39 _ 66 41: 0F 72. F3, 07
	pxor	xmm10, xmm11				; 2B3F _ 66 45: 0F EF. D3
	movdqa	xmm11, xmm1				; 2B44 _ 66 44: 0F 6F. D9
	movdqa	xmm0, xmm10				; 2B49 _ 66 41: 0F 6F. C2
	psrld	xmm1, 27				; 2B4E _ 66: 0F 72. D1, 1B
	pslld	xmm11, 5				; 2B53 _ 66 41: 0F 72. F3, 05
	por	xmm11, xmm1				; 2B59 _ 66 44: 0F EB. D9
	pslld	xmm0, 22				; 2B5E _ 66: 0F 72. F0, 16
	psrld	xmm10, 10				; 2B63 _ 66 41: 0F 72. D2, 0A
	por	xmm0, xmm10				; 2B69 _ 66 41: 0F EB. C2
	movd	xmm10, dword [r12+1460H]		; 2B6E _ 66 45: 0F 6E. 94 24, 00001460
	pshufd	xmm10, xmm10, 0 			; 2B78 _ 66 45: 0F 70. D2, 00
	pxor	xmm11, xmm10				; 2B7E _ 66 45: 0F EF. DA
	movd	xmm10, dword [r12+1464H]		; 2B83 _ 66 45: 0F 6E. 94 24, 00001464
	pshufd	xmm10, xmm10, 0 			; 2B8D _ 66 45: 0F 70. D2, 00
	pxor	xmm8, xmm10				; 2B93 _ 66 45: 0F EF. C2
	movd	xmm10, dword [r12+1468H]		; 2B98 _ 66 45: 0F 6E. 94 24, 00001468
	pshufd	xmm10, xmm10, 0 			; 2BA2 _ 66 45: 0F 70. D2, 00
	pxor	xmm0, xmm10				; 2BA8 _ 66 41: 0F EF. C2
	pxor	xmm0, xmm6				; 2BAD _ 66: 0F EF. C6
	movd	xmm10, dword [r12+146CH]		; 2BB1 _ 66 45: 0F 6E. 94 24, 0000146C
	pshufd	xmm10, xmm10, 0 			; 2BBB _ 66 45: 0F 70. D2, 00
	pxor	xmm2, xmm10				; 2BC1 _ 66 41: 0F EF. D2
	movdqa	xmm1, xmm2				; 2BC6 _ 66: 0F 6F. CA
	pand	xmm1, xmm11				; 2BCA _ 66 41: 0F DB. CB
	pxor	xmm11, xmm2				; 2BCF _ 66 44: 0F EF. DA
	pxor	xmm1, xmm0				; 2BD4 _ 66: 0F EF. C8
	por	xmm0, xmm2				; 2BD8 _ 66: 0F EB. C2
	pxor	xmm8, xmm1				; 2BDC _ 66 44: 0F EF. C1
	pxor	xmm0, xmm11				; 2BE1 _ 66 41: 0F EF. C3
	por	xmm11, xmm8				; 2BE6 _ 66 45: 0F EB. D8
	pxor	xmm0, xmm8				; 2BEB _ 66 41: 0F EF. C0
	pxor	xmm2, xmm11				; 2BF0 _ 66 41: 0F EF. D3
	por	xmm11, xmm1				; 2BF5 _ 66 44: 0F EB. D9
	pxor	xmm11, xmm0				; 2BFA _ 66 44: 0F EF. D8
	pxor	xmm2, xmm1				; 2BFF _ 66: 0F EF. D1
	pxor	xmm2, xmm11				; 2C03 _ 66 41: 0F EF. D3
	movdqa	xmm10, xmm2				; 2C08 _ 66 44: 0F 6F. D2
	pxor	xmm1, xmm6				; 2C0D _ 66: 0F EF. CE
	pand	xmm0, xmm2				; 2C11 _ 66: 0F DB. C2
	pxor	xmm1, xmm0				; 2C15 _ 66: 0F EF. C8
	movdqa	xmm0, xmm11				; 2C19 _ 66 41: 0F 6F. C3
	psrld	xmm11, 19				; 2C1E _ 66 41: 0F 72. D3, 13
	pslld	xmm10, 3				; 2C24 _ 66 41: 0F 72. F2, 03
	pslld	xmm0, 13				; 2C2A _ 66: 0F 72. F0, 0D
	por	xmm0, xmm11				; 2C2F _ 66 41: 0F EB. C3
	psrld	xmm2, 29				; 2C34 _ 66: 0F 72. D2, 1D
	por	xmm10, xmm2				; 2C39 _ 66 44: 0F EB. D2
	movdqa	xmm2, xmm0				; 2C3E _ 66: 0F 6F. D0
	pxor	xmm8, xmm0				; 2C42 _ 66 44: 0F EF. C0
	pxor	xmm8, xmm10				; 2C47 _ 66 45: 0F EF. C2
	pxor	xmm1, xmm10				; 2C4C _ 66 41: 0F EF. CA
	pslld	xmm2, 3 				; 2C51 _ 66: 0F 72. F2, 03
	pxor	xmm1, xmm2				; 2C56 _ 66: 0F EF. CA
	movdqa	xmm2, xmm8				; 2C5A _ 66 41: 0F 6F. D0
	psrld	xmm8, 31				; 2C5F _ 66 41: 0F 72. D0, 1F
	pslld	xmm2, 1 				; 2C65 _ 66: 0F 72. F2, 01
	por	xmm2, xmm8				; 2C6A _ 66 41: 0F EB. D0
	movdqa	xmm8, xmm1				; 2C6F _ 66 44: 0F 6F. C1
	movdqa	xmm11, xmm2				; 2C74 _ 66 44: 0F 6F. DA
	psrld	xmm1, 25				; 2C79 _ 66: 0F 72. D1, 19
	pslld	xmm8, 7 				; 2C7E _ 66 41: 0F 72. F0, 07
	por	xmm8, xmm1				; 2C84 _ 66 44: 0F EB. C1
	pxor	xmm0, xmm2				; 2C89 _ 66: 0F EF. C2
	pxor	xmm0, xmm8				; 2C8D _ 66 41: 0F EF. C0
	movdqa	xmm1, xmm0				; 2C92 _ 66: 0F 6F. C8
	pxor	xmm10, xmm8				; 2C96 _ 66 45: 0F EF. D0
	pslld	xmm11, 7				; 2C9B _ 66 41: 0F 72. F3, 07
	pxor	xmm10, xmm11				; 2CA1 _ 66 45: 0F EF. D3
	movdqa	xmm11, xmm10				; 2CA6 _ 66 45: 0F 6F. DA
	pslld	xmm1, 5 				; 2CAB _ 66: 0F 72. F1, 05
	psrld	xmm0, 27				; 2CB0 _ 66: 0F 72. D0, 1B
	por	xmm1, xmm0				; 2CB5 _ 66: 0F EB. C8
	pslld	xmm11, 22				; 2CB9 _ 66 41: 0F 72. F3, 16
	psrld	xmm10, 10				; 2CBF _ 66 41: 0F 72. D2, 0A
	por	xmm11, xmm10				; 2CC5 _ 66 45: 0F EB. DA
	movd	xmm10, dword [r12+1470H]		; 2CCA _ 66 45: 0F 6E. 94 24, 00001470
	pshufd	xmm10, xmm10, 0 			; 2CD4 _ 66 45: 0F 70. D2, 00
	pxor	xmm1, xmm10				; 2CDA _ 66 41: 0F EF. CA
	movd	xmm10, dword [r12+1474H]		; 2CDF _ 66 45: 0F 6E. 94 24, 00001474
	pshufd	xmm10, xmm10, 0 			; 2CE9 _ 66 45: 0F 70. D2, 00
	pxor	xmm2, xmm10				; 2CEF _ 66 41: 0F EF. D2
	movdqa	xmm0, xmm2				; 2CF4 _ 66: 0F 6F. C2
	movd	xmm10, dword [r12+1478H]		; 2CF8 _ 66 45: 0F 6E. 94 24, 00001478
	pshufd	xmm10, xmm10, 0 			; 2D02 _ 66 45: 0F 70. D2, 00
	pxor	xmm11, xmm10				; 2D08 _ 66 45: 0F EF. DA
	por	xmm0, xmm11				; 2D0D _ 66 41: 0F EB. C3
	pxor	xmm2, xmm11				; 2D12 _ 66 41: 0F EF. D3
	movd	xmm10, dword [r12+147CH]		; 2D17 _ 66 45: 0F 6E. 94 24, 0000147C
	pshufd	xmm10, xmm10, 0 			; 2D21 _ 66 45: 0F 70. D2, 00
	pxor	xmm8, xmm10				; 2D27 _ 66 45: 0F EF. C2
	pxor	xmm0, xmm8				; 2D2C _ 66 41: 0F EF. C0
	pxor	xmm11, xmm0				; 2D31 _ 66 44: 0F EF. D8
	por	xmm8, xmm2				; 2D36 _ 66 44: 0F EB. C2
	pand	xmm8, xmm1				; 2D3B _ 66 44: 0F DB. C1
	pxor	xmm2, xmm11				; 2D40 _ 66 41: 0F EF. D3
	pxor	xmm8, xmm0				; 2D45 _ 66 44: 0F EF. C0
	por	xmm0, xmm2				; 2D4A _ 66: 0F EB. C2
	pxor	xmm0, xmm1				; 2D4E _ 66: 0F EF. C1
	por	xmm1, xmm2				; 2D52 _ 66: 0F EB. CA
	pxor	xmm1, xmm11				; 2D56 _ 66 41: 0F EF. CB
	pxor	xmm0, xmm2				; 2D5B _ 66: 0F EF. C2
	pxor	xmm11, xmm0				; 2D5F _ 66 44: 0F EF. D8
	pand	xmm0, xmm1				; 2D64 _ 66: 0F DB. C1
	pxor	xmm0, xmm2				; 2D68 _ 66: 0F EF. C2
	pxor	xmm11, xmm6				; 2D6C _ 66 44: 0F EF. DE
	por	xmm11, xmm1				; 2D71 _ 66 44: 0F EB. D9
	pxor	xmm2, xmm11				; 2D76 _ 66 41: 0F EF. D3
	movd	xmm11, dword [r12+1480H]		; 2D7B _ 66 45: 0F 6E. 9C 24, 00001480
	pshufd	xmm10, xmm11, 0 			; 2D85 _ 66 45: 0F 70. D3, 00
	pxor	xmm2, xmm10				; 2D8B _ 66 41: 0F EF. D2
	movd	xmm11, dword [r12+1484H]		; 2D90 _ 66 45: 0F 6E. 9C 24, 00001484
	pshufd	xmm10, xmm11, 0 			; 2D9A _ 66 45: 0F 70. D3, 00
	pxor	xmm8, xmm10				; 2DA0 _ 66 45: 0F EF. C2
	movd	xmm11, dword [r12+1488H]		; 2DA5 _ 66 45: 0F 6E. 9C 24, 00001488
	pshufd	xmm10, xmm11, 0 			; 2DAF _ 66 45: 0F 70. D3, 00
	pxor	xmm0, xmm10				; 2DB5 _ 66 41: 0F EF. C2
	movd	xmm11, dword [r12+148CH]		; 2DBA _ 66 45: 0F 6E. 9C 24, 0000148C
	pshufd	xmm10, xmm11, 0 			; 2DC4 _ 66 45: 0F 70. D3, 00
	movdqa	xmm11, xmm2				; 2DCA _ 66 44: 0F 6F. DA
	pxor	xmm1, xmm10				; 2DCF _ 66 41: 0F EF. CA
	movdqa	xmm10, xmm0				; 2DD4 _ 66 44: 0F 6F. D0
	punpckldq xmm11, xmm8				; 2DD9 _ 66 45: 0F 62. D8
	punpckhdq xmm2, xmm8				; 2DDE _ 66 41: 0F 6A. D0
	punpckldq xmm10, xmm1				; 2DE3 _ 66 44: 0F 62. D1
	punpckhdq xmm0, xmm1				; 2DE8 _ 66: 0F 6A. C1
	movdqa	xmm8, xmm11				; 2DEC _ 66 45: 0F 6F. C3
	punpckhqdq xmm11, xmm10 			; 2DF1 _ 66 45: 0F 6D. DA
	pxor	xmm11, xmm5				; 2DF6 _ 66 44: 0F EF. DD
	movdqu	oword [rbp+10H], xmm11			; 2DFB _ F3 44: 0F 7F. 5D, 10
	punpcklqdq xmm8, xmm10				; 2E01 _ 66 45: 0F 6C. C2
	movdqa	xmm10, xmm2				; 2E06 _ 66 44: 0F 6F. D2
	punpckhqdq xmm2, xmm0				; 2E0B _ 66: 0F 6D. D0
	pxor	xmm8, xmm9				; 2E0F _ 66 45: 0F EF. C1
	movdqu	oword [rbp], xmm8			; 2E14 _ F3 44: 0F 7F. 45, 00
	movdqa	xmm9, xmm3				; 2E1A _ 66 44: 0F 6F. CB
	punpcklqdq xmm10, xmm0				; 2E1F _ 66 44: 0F 6C. D0
	pxor	xmm10, xmm4				; 2E24 _ 66 44: 0F EF. D4
	movdqu	oword [rbp+20H], xmm10			; 2E29 _ F3 44: 0F 7F. 55, 20
	pxor	xmm2, xmm3				; 2E2F _ 66: 0F EF. D3
	movdqu	oword [rbp+30H], xmm2			; 2E33 _ F3: 0F 7F. 55, 30
	movdqa	xmm2, xmm3				; 2E38 _ 66: 0F 6F. D3
	psllq	xmm9, 1 				; 2E3C _ 66 41: 0F 73. F1, 01
	psraw	xmm3, 8 				; 2E42 _ 66: 0F 71. E3, 08
	pslldq	xmm2, 8 				; 2E47 _ 66: 0F 73. FA, 08
	psrldq	xmm2, 7 				; 2E4C _ 66: 0F 73. DA, 07
	psrlq	xmm2, 7 				; 2E51 _ 66: 0F 73. D2, 07
	por	xmm9, xmm2				; 2E56 _ 66 44: 0F EB. CA
	psrldq	xmm3, 15				; 2E5B _ 66: 0F 73. DB, 0F
	pand	xmm3, xmm7				; 2E60 _ 66: 0F DB. DF
	pxor	xmm9, xmm3				; 2E64 _ 66 44: 0F EF. CB
	add	r13, 64 				; 2E69 _ 49: 83. C5, 40
	add	rbp, 64 				; 2E6D _ 48: 83. C5, 40
	inc	r10d					; 2E71 _ 41: FF. C2
	cmp	r10d, 8 				; 2E74 _ 41: 83. FA, 08
	jl	?_003					; 2E78 _ 0F 8C, FFFFD23A
	add	r14, -512				; 2E7E _ 49: 81. C6, FFFFFE00
	jne	?_001					; 2E85 _ 0F 85, FFFFD1F6
	movaps	xmm6, oword [rsp+70H]			; 2E8B _ 0F 28. 74 24, 70
	movaps	xmm7, oword [rsp+60H]			; 2E90 _ 0F 28. 7C 24, 60
	movaps	xmm8, oword [rsp+50H]			; 2E95 _ 44: 0F 28. 44 24, 50
	movaps	xmm9, oword [rsp+40H]			; 2E9B _ 44: 0F 28. 4C 24, 40
	movaps	xmm10, oword [rsp+30H]			; 2EA1 _ 44: 0F 28. 54 24, 30
	movaps	xmm11, oword [rsp+20H]			; 2EA7 _ 44: 0F 28. 5C 24, 20
	add	rsp, 160				; 2EAD _ 48: 81. C4, 000000A0
	pop	rbp					; 2EB4 _ 5D
	pop	r12					; 2EB5 _ 41: 5C
	pop	r13					; 2EB7 _ 41: 5D
	pop	r14					; 2EB9 _ 41: 5E
	pop	r15					; 2EBB _ 41: 5F
	ret						; 2EBD _ C3
; xts_serpent_sse2_encrypt End of function

	nop						; 2EBE _ 90
	nop						; 2EBF _ 90

ALIGN	16
xts_serpent_sse2_decrypt:; Function begin
	push	r15					; 2EC0 _ 41: 57
	push	r14					; 2EC2 _ 41: 56
	push	r13					; 2EC4 _ 41: 55
	push	r12					; 2EC6 _ 41: 54
	push	rbp					; 2EC8 _ 55
	sub	rsp, 160				; 2EC9 _ 48: 81. EC, 000000A0
	mov	rax, qword [rsp+0F0H]			; 2ED0 _ 48: 8B. 84 24, 000000F0
	movaps	oword [rsp+70H], xmm6			; 2ED8 _ 0F 29. 74 24, 70
	movaps	oword [rsp+60H], xmm7			; 2EDD _ 0F 29. 7C 24, 60
	movaps	oword [rsp+50H], xmm8			; 2EE2 _ 44: 0F 29. 44 24, 50
	movaps	oword [rsp+40H], xmm9			; 2EE8 _ 44: 0F 29. 4C 24, 40
	movaps	oword [rsp+30H], xmm10			; 2EEE _ 44: 0F 29. 54 24, 30
	movaps	oword [rsp+20H], xmm11			; 2EF4 _ 44: 0F 29. 5C 24, 20
	shr	r9, 9					; 2EFA _ 49: C1. E9, 09
	mov	qword [rsp+80H], r9			; 2EFE _ 4C: 89. 8C 24, 00000080
	lea	r9, [rax+2710H] 			; 2F06 _ 4C: 8D. 88, 00002710
	mov	qword [rsp+88H], 0			; 2F0D _ 48: C7. 84 24, 00000088, 00000000
	mov	r10d, 135				; 2F19 _ 41: BA, 00000087
	mov	r12, rax				; 2F1F _ 49: 89. C4
	movd	xmm1, r10d				; 2F22 _ 66 41: 0F 6E. CA
	movdqa	xmm7, xmm1				; 2F27 _ 66: 0F 6F. F9
	mov	rbp, rdx				; 2F2B _ 48: 89. D5
	mov	r13, rcx				; 2F2E _ 49: 89. CD
	mov	r14, r8 				; 2F31 _ 4D: 89. C6
	mov	r15, r9 				; 2F34 _ 4D: 89. CF
	pcmpeqd xmm0, xmm0				; 2F37 _ 66: 0F 76. C0
	movdqa	xmm6, xmm0				; 2F3B _ 66: 0F 6F. F0
	jmp	?_005					; 2F3F _ EB, 0A

?_004:	movdqa	oword [rsp+90H], xmm8			; 2F41 _ 66 44: 0F 7F. 84 24, 00000090
?_005:	inc	qword [rsp+80H] 			; 2F4B _ 48: FF. 84 24, 00000080
	lea	rcx, [rsp+80H]				; 2F53 _ 48: 8D. 8C 24, 00000080
	mov	r8, r15 				; 2F5B _ 4D: 89. F8
	lea	rdx, [rsp+90H]				; 2F5E _ 48: 8D. 94 24, 00000090
	call	serpent256_encrypt			; 2F66 _ E8, 00000000(rel)
	movdqa	xmm8, oword [rsp+90H]			; 2F6B _ 66 44: 0F 6F. 84 24, 00000090
	xor	r10d, r10d				; 2F75 _ 45: 33. D2
?_006:	movdqa	xmm5, xmm8				; 2F78 _ 66 41: 0F 6F. E8
	movdqa	xmm4, xmm8				; 2F7D _ 66 41: 0F 6F. E0
	movdqa	xmm3, xmm8				; 2F82 _ 66 41: 0F 6F. D8
	movdqu	xmm10, oword [r13+10H]			; 2F87 _ F3 45: 0F 6F. 55, 10
	psllq	xmm5, 1 				; 2F8D _ 66: 0F 73. F5, 01
	pslldq	xmm4, 8 				; 2F92 _ 66: 0F 73. FC, 08
	psrldq	xmm4, 7 				; 2F97 _ 66: 0F 73. DC, 07
	psrlq	xmm4, 7 				; 2F9C _ 66: 0F 73. D4, 07
	por	xmm5, xmm4				; 2FA1 _ 66: 0F EB. EC
	psraw	xmm3, 8 				; 2FA5 _ 66: 0F 71. E3, 08
	psrldq	xmm3, 15				; 2FAA _ 66: 0F 73. DB, 0F
	pand	xmm3, xmm7				; 2FAF _ 66: 0F DB. DF
	pxor	xmm5, xmm3				; 2FB3 _ 66: 0F EF. EB
	movdqa	xmm4, xmm5				; 2FB7 _ 66: 0F 6F. E5
	movdqa	xmm11, xmm5				; 2FBB _ 66 44: 0F 6F. DD
	movdqa	xmm9, xmm5				; 2FC0 _ 66 44: 0F 6F. CD
	psllq	xmm4, 1 				; 2FC5 _ 66: 0F 73. F4, 01
	pslldq	xmm11, 8				; 2FCA _ 66 41: 0F 73. FB, 08
	psrldq	xmm11, 7				; 2FD0 _ 66 41: 0F 73. DB, 07
	psrlq	xmm11, 7				; 2FD6 _ 66 41: 0F 73. D3, 07
	por	xmm4, xmm11				; 2FDC _ 66 41: 0F EB. E3
	psraw	xmm9, 8 				; 2FE1 _ 66 41: 0F 71. E1, 08
	psrldq	xmm9, 15				; 2FE7 _ 66 41: 0F 73. D9, 0F
	pand	xmm9, xmm7				; 2FED _ 66 44: 0F DB. CF
	pxor	xmm4, xmm9				; 2FF2 _ 66 41: 0F EF. E1
	movdqa	xmm3, xmm4				; 2FF7 _ 66: 0F 6F. DC
	movdqa	xmm2, xmm4				; 2FFB _ 66: 0F 6F. D4
	movdqa	xmm11, xmm4				; 2FFF _ 66 44: 0F 6F. DC
	movdqu	xmm9, oword [r13+20H]			; 3004 _ F3 45: 0F 6F. 4D, 20
	psllq	xmm3, 1 				; 300A _ 66: 0F 73. F3, 01
	pslldq	xmm2, 8 				; 300F _ 66: 0F 73. FA, 08
	psrldq	xmm2, 7 				; 3014 _ 66: 0F 73. DA, 07
	psrlq	xmm2, 7 				; 3019 _ 66: 0F 73. D2, 07
	por	xmm3, xmm2				; 301E _ 66: 0F EB. DA
	movdqu	xmm2, oword [r13+30H]			; 3022 _ F3 41: 0F 6F. 55, 30
	psraw	xmm11, 8				; 3028 _ 66 41: 0F 71. E3, 08
	psrldq	xmm11, 15				; 302E _ 66 41: 0F 73. DB, 0F
	pand	xmm11, xmm7				; 3034 _ 66 44: 0F DB. DF
	pxor	xmm3, xmm11				; 3039 _ 66 41: 0F EF. DB
	movdqu	xmm11, oword [r13]			; 303E _ F3 45: 0F 6F. 5D, 00
	pxor	xmm10, xmm5				; 3044 _ 66 44: 0F EF. D5
	pxor	xmm9, xmm4				; 3049 _ 66 44: 0F EF. CC
	movdqa	xmm0, xmm9				; 304E _ 66 41: 0F 6F. C1
	pxor	xmm11, xmm8				; 3053 _ 66 45: 0F EF. D8
	movdqa	xmm1, xmm11				; 3058 _ 66 41: 0F 6F. CB
	pxor	xmm2, xmm3				; 305D _ 66: 0F EF. D3
	punpckldq xmm0, xmm2				; 3061 _ 66: 0F 62. C2
	punpckldq xmm1, xmm10				; 3065 _ 66 41: 0F 62. CA
	punpckhdq xmm11, xmm10				; 306A _ 66 45: 0F 6A. DA
	movdqa	xmm10, xmm1				; 306F _ 66 44: 0F 6F. D1
	punpckhdq xmm9, xmm2				; 3074 _ 66 44: 0F 6A. CA
	movdqa	xmm2, xmm11				; 3079 _ 66 41: 0F 6F. D3
	punpcklqdq xmm10, xmm0				; 307E _ 66 44: 0F 6C. D0
	punpckhqdq xmm1, xmm0				; 3083 _ 66: 0F 6D. C8
	punpcklqdq xmm2, xmm9				; 3087 _ 66 41: 0F 6C. D1
	punpckhqdq xmm11, xmm9				; 308C _ 66 45: 0F 6D. D9
	movd	xmm9, dword [r12+1480H] 		; 3091 _ 66 45: 0F 6E. 8C 24, 00001480
	pshufd	xmm9, xmm9, 0				; 309B _ 66 45: 0F 70. C9, 00
	pxor	xmm10, xmm9				; 30A1 _ 66 45: 0F EF. D1
	movd	xmm9, dword [r12+1484H] 		; 30A6 _ 66 45: 0F 6E. 8C 24, 00001484
	pshufd	xmm9, xmm9, 0				; 30B0 _ 66 45: 0F 70. C9, 00
	pxor	xmm1, xmm9				; 30B6 _ 66 41: 0F EF. C9
	movd	xmm9, dword [r12+1488H] 		; 30BB _ 66 45: 0F 6E. 8C 24, 00001488
	pshufd	xmm9, xmm9, 0				; 30C5 _ 66 45: 0F 70. C9, 00
	pxor	xmm2, xmm9				; 30CB _ 66 41: 0F EF. D1
	movd	xmm9, dword [r12+148CH] 		; 30D0 _ 66 45: 0F 6E. 8C 24, 0000148C
	pshufd	xmm9, xmm9, 0				; 30DA _ 66 45: 0F 70. C9, 00
	pxor	xmm11, xmm9				; 30E0 _ 66 45: 0F EF. D9
	movdqa	xmm9, xmm2				; 30E5 _ 66 44: 0F 6F. CA
	por	xmm2, xmm11				; 30EA _ 66 41: 0F EB. D3
	pxor	xmm9, xmm10				; 30EF _ 66 45: 0F EF. CA
	pand	xmm10, xmm11				; 30F4 _ 66 45: 0F DB. D3
	pxor	xmm9, xmm6				; 30F9 _ 66 44: 0F EF. CE
	pxor	xmm11, xmm1				; 30FE _ 66 44: 0F EF. D9
	por	xmm1, xmm10				; 3103 _ 66 41: 0F EB. CA
	pxor	xmm10, xmm9				; 3108 _ 66 45: 0F EF. D1
	pand	xmm9, xmm2				; 310D _ 66 44: 0F DB. CA
	pand	xmm11, xmm2				; 3112 _ 66 44: 0F DB. DA
	pxor	xmm1, xmm9				; 3117 _ 66 41: 0F EF. C9
	pxor	xmm9, xmm10				; 311C _ 66 45: 0F EF. CA
	por	xmm10, xmm9				; 3121 _ 66 45: 0F EB. D1
	pxor	xmm2, xmm1				; 3126 _ 66: 0F EF. D1
	pxor	xmm10, xmm11				; 312A _ 66 45: 0F EF. D3
	pxor	xmm11, xmm2				; 312F _ 66 44: 0F EF. DA
	por	xmm2, xmm10				; 3134 _ 66 41: 0F EB. D2
	pxor	xmm11, xmm9				; 3139 _ 66 45: 0F EF. D9
	pxor	xmm2, xmm9				; 313E _ 66 41: 0F EF. D1
	movd	xmm9, dword [r12+1470H] 		; 3143 _ 66 45: 0F 6E. 8C 24, 00001470
	pshufd	xmm9, xmm9, 0				; 314D _ 66 45: 0F 70. C9, 00
	pxor	xmm11, xmm9				; 3153 _ 66 45: 0F EF. D9
	movdqa	xmm0, xmm11				; 3158 _ 66 41: 0F 6F. C3
	psrld	xmm11, 5				; 315D _ 66 41: 0F 72. D3, 05
	movd	xmm9, dword [r12+1474H] 		; 3163 _ 66 45: 0F 6E. 8C 24, 00001474
	pshufd	xmm9, xmm9, 0				; 316D _ 66 45: 0F 70. C9, 00
	pxor	xmm10, xmm9				; 3173 _ 66 45: 0F EF. D1
	pslld	xmm0, 27				; 3178 _ 66: 0F 72. F0, 1B
	por	xmm0, xmm11				; 317D _ 66 41: 0F EB. C3
	movdqa	xmm11, xmm10				; 3182 _ 66 45: 0F 6F. DA
	pxor	xmm0, xmm10				; 3187 _ 66 41: 0F EF. C2
	movd	xmm9, dword [r12+1478H] 		; 318C _ 66 45: 0F 6E. 8C 24, 00001478
	pshufd	xmm9, xmm9, 0				; 3196 _ 66 45: 0F 70. C9, 00
	pxor	xmm1, xmm9				; 319C _ 66 41: 0F EF. C9
	pslld	xmm11, 7				; 31A1 _ 66 41: 0F 72. F3, 07
	movd	xmm9, dword [r12+147CH] 		; 31A7 _ 66 45: 0F 6E. 8C 24, 0000147C
	pshufd	xmm9, xmm9, 0				; 31B1 _ 66 45: 0F 70. C9, 00
	pxor	xmm2, xmm9				; 31B7 _ 66 41: 0F EF. D1
	movdqa	xmm9, xmm1				; 31BC _ 66 44: 0F 6F. C9
	psrld	xmm1, 22				; 31C1 _ 66: 0F 72. D1, 16
	pxor	xmm0, xmm2				; 31C6 _ 66: 0F EF. C2
	pslld	xmm9, 10				; 31CA _ 66 41: 0F 72. F1, 0A
	por	xmm9, xmm1				; 31D0 _ 66 44: 0F EB. C9
	movdqa	xmm1, xmm2				; 31D5 _ 66: 0F 6F. CA
	pxor	xmm9, xmm2				; 31D9 _ 66 44: 0F EF. CA
	pxor	xmm9, xmm11				; 31DE _ 66 45: 0F EF. CB
	movdqa	xmm11, xmm10				; 31E3 _ 66 45: 0F 6F. DA
	pslld	xmm1, 25				; 31E8 _ 66: 0F 72. F1, 19
	psrld	xmm2, 7 				; 31ED _ 66: 0F 72. D2, 07
	por	xmm1, xmm2				; 31F2 _ 66: 0F EB. CA
	movdqa	xmm2, xmm0				; 31F6 _ 66: 0F 6F. D0
	pslld	xmm11, 31				; 31FA _ 66 41: 0F 72. F3, 1F
	psrld	xmm10, 1				; 3200 _ 66 41: 0F 72. D2, 01
	por	xmm11, xmm10				; 3206 _ 66 45: 0F EB. DA
	movdqa	xmm10, xmm0				; 320B _ 66 44: 0F 6F. D0
	pxor	xmm1, xmm9				; 3210 _ 66 41: 0F EF. C9
	pxor	xmm11, xmm0				; 3215 _ 66 44: 0F EF. D8
	pslld	xmm10, 3				; 321A _ 66 41: 0F 72. F2, 03
	pxor	xmm1, xmm10				; 3220 _ 66 41: 0F EF. CA
	movdqa	xmm10, xmm9				; 3225 _ 66 45: 0F 6F. D1
	pxor	xmm11, xmm9				; 322A _ 66 45: 0F EF. D9
	psrld	xmm9, 3 				; 322F _ 66 41: 0F 72. D1, 03
	pslld	xmm10, 29				; 3235 _ 66 41: 0F 72. F2, 1D
	por	xmm10, xmm9				; 323B _ 66 45: 0F EB. D1
	pslld	xmm2, 19				; 3240 _ 66: 0F 72. F2, 13
	psrld	xmm0, 13				; 3245 _ 66: 0F 72. D0, 0D
	por	xmm2, xmm0				; 324A _ 66: 0F EB. D0
	movdqa	xmm0, xmm10				; 324E _ 66 41: 0F 6F. C2
	pxor	xmm2, xmm10				; 3253 _ 66 41: 0F EF. D2
	movd	xmm9, dword [r12+1460H] 		; 3258 _ 66 45: 0F 6E. 8C 24, 00001460
	pxor	xmm10, xmm1				; 3262 _ 66 44: 0F EF. D1
	pand	xmm0, xmm2				; 3267 _ 66: 0F DB. C2
	pxor	xmm0, xmm6				; 326B _ 66: 0F EF. C6
	pxor	xmm1, xmm11				; 326F _ 66 41: 0F EF. CB
	pxor	xmm0, xmm1				; 3274 _ 66: 0F EF. C1
	por	xmm10, xmm2				; 3278 _ 66 44: 0F EB. D2
	pxor	xmm2, xmm0				; 327D _ 66: 0F EF. D0
	pxor	xmm1, xmm10				; 3281 _ 66 41: 0F EF. CA
	pxor	xmm10, xmm11				; 3286 _ 66 45: 0F EF. D3
	pand	xmm11, xmm1				; 328B _ 66 44: 0F DB. D9
	pxor	xmm11, xmm2				; 3290 _ 66 44: 0F EF. DA
	pxor	xmm2, xmm1				; 3295 _ 66: 0F EF. D1
	por	xmm2, xmm0				; 3299 _ 66: 0F EB. D0
	pxor	xmm1, xmm11				; 329D _ 66 41: 0F EF. CB
	pxor	xmm10, xmm2				; 32A2 _ 66 44: 0F EF. D2
	pshufd	xmm2, xmm9, 0				; 32A7 _ 66 41: 0F 70. D1, 00
	pxor	xmm11, xmm2				; 32AD _ 66 44: 0F EF. DA
	movd	xmm9, dword [r12+1464H] 		; 32B2 _ 66 45: 0F 6E. 8C 24, 00001464
	pshufd	xmm2, xmm9, 0				; 32BC _ 66 41: 0F 70. D1, 00
	movd	xmm9, dword [r12+1468H] 		; 32C2 _ 66 45: 0F 6E. 8C 24, 00001468
	pxor	xmm0, xmm2				; 32CC _ 66: 0F EF. C2
	pshufd	xmm2, xmm9, 0				; 32D0 _ 66 41: 0F 70. D1, 00
	pxor	xmm10, xmm2				; 32D6 _ 66 44: 0F EF. D2
	movd	xmm9, dword [r12+146CH] 		; 32DB _ 66 45: 0F 6E. 8C 24, 0000146C
	pshufd	xmm2, xmm9, 0				; 32E5 _ 66 41: 0F 70. D1, 00
	movdqa	xmm9, xmm10				; 32EB _ 66 45: 0F 6F. CA
	pxor	xmm1, xmm2				; 32F0 _ 66: 0F EF. CA
	movdqa	xmm2, xmm11				; 32F4 _ 66 41: 0F 6F. D3
	pslld	xmm9, 10				; 32F9 _ 66 41: 0F 72. F1, 0A
	psrld	xmm10, 22				; 32FF _ 66 41: 0F 72. D2, 16
	por	xmm9, xmm10				; 3305 _ 66 45: 0F EB. CA
	movdqa	xmm10, xmm1				; 330A _ 66 44: 0F 6F. D1
	pslld	xmm2, 27				; 330F _ 66: 0F 72. F2, 1B
	psrld	xmm11, 5				; 3314 _ 66 41: 0F 72. D3, 05
	por	xmm2, xmm11				; 331A _ 66 41: 0F EB. D3
	movdqa	xmm11, xmm0				; 331F _ 66 44: 0F 6F. D8
	pxor	xmm9, xmm1				; 3324 _ 66 44: 0F EF. C9
	pxor	xmm2, xmm0				; 3329 _ 66: 0F EF. D0
	pslld	xmm11, 7				; 332D _ 66 41: 0F 72. F3, 07
	pxor	xmm9, xmm11				; 3333 _ 66 45: 0F EF. CB
	movdqa	xmm11, xmm0				; 3338 _ 66 44: 0F 6F. D8
	pxor	xmm2, xmm1				; 333D _ 66: 0F EF. D1
	pslld	xmm10, 25				; 3341 _ 66 41: 0F 72. F2, 19
	psrld	xmm1, 7 				; 3347 _ 66: 0F 72. D1, 07
	por	xmm10, xmm1				; 334C _ 66 44: 0F EB. D1
	movdqa	xmm1, xmm2				; 3351 _ 66: 0F 6F. CA
	pslld	xmm11, 31				; 3355 _ 66 41: 0F 72. F3, 1F
	psrld	xmm0, 1 				; 335B _ 66: 0F 72. D0, 01
	por	xmm11, xmm0				; 3360 _ 66 44: 0F EB. D8
	movdqa	xmm0, xmm2				; 3365 _ 66: 0F 6F. C2
	pxor	xmm10, xmm9				; 3369 _ 66 45: 0F EF. D1
	pxor	xmm11, xmm2				; 336E _ 66 44: 0F EF. DA
	pslld	xmm0, 3 				; 3373 _ 66: 0F 72. F0, 03
	pxor	xmm10, xmm0				; 3378 _ 66 44: 0F EF. D0
	movdqa	xmm0, xmm9				; 337D _ 66 41: 0F 6F. C1
	pxor	xmm11, xmm9				; 3382 _ 66 45: 0F EF. D9
	psrld	xmm9, 3 				; 3387 _ 66 41: 0F 72. D1, 03
	pslld	xmm0, 29				; 338D _ 66: 0F 72. F0, 1D
	por	xmm0, xmm9				; 3392 _ 66 41: 0F EB. C1
	movdqa	xmm9, xmm10				; 3397 _ 66 45: 0F 6F. CA
	pslld	xmm1, 19				; 339C _ 66: 0F 72. F1, 13
	psrld	xmm2, 13				; 33A1 _ 66: 0F 72. D2, 0D
	por	xmm1, xmm2				; 33A6 _ 66: 0F EB. CA
	movd	xmm2, dword [r12+1450H] 		; 33AA _ 66 41: 0F 6E. 94 24, 00001450
	pxor	xmm11, xmm6				; 33B4 _ 66 44: 0F EF. DE
	pxor	xmm0, xmm11				; 33B9 _ 66 41: 0F EF. C3
	por	xmm9, xmm1				; 33BE _ 66 44: 0F EB. C9
	pxor	xmm9, xmm0				; 33C3 _ 66 44: 0F EF. C8
	por	xmm0, xmm11				; 33C8 _ 66 41: 0F EB. C3
	pand	xmm0, xmm1				; 33CD _ 66: 0F DB. C1
	pxor	xmm10, xmm9				; 33D1 _ 66 45: 0F EF. D1
	pxor	xmm0, xmm10				; 33D6 _ 66 41: 0F EF. C2
	por	xmm10, xmm1				; 33DB _ 66 44: 0F EB. D1
	pxor	xmm10, xmm11				; 33E0 _ 66 45: 0F EF. D3
	pand	xmm11, xmm0				; 33E5 _ 66 44: 0F DB. D8
	pxor	xmm11, xmm9				; 33EA _ 66 45: 0F EF. D9
	pxor	xmm10, xmm0				; 33EF _ 66 44: 0F EF. D0
	pand	xmm9, xmm10				; 33F4 _ 66 45: 0F DB. CA
	pxor	xmm10, xmm11				; 33F9 _ 66 45: 0F EF. D3
	pxor	xmm9, xmm10				; 33FE _ 66 45: 0F EF. CA
	pxor	xmm10, xmm6				; 3403 _ 66 44: 0F EF. D6
	pxor	xmm9, xmm1				; 3408 _ 66 44: 0F EF. C9
	pshufd	xmm2, xmm2, 0				; 340D _ 66: 0F 70. D2, 00
	pxor	xmm11, xmm2				; 3412 _ 66 44: 0F EF. DA
	movd	xmm2, dword [r12+1454H] 		; 3417 _ 66 41: 0F 6E. 94 24, 00001454
	pshufd	xmm2, xmm2, 0				; 3421 _ 66: 0F 70. D2, 00
	pxor	xmm10, xmm2				; 3426 _ 66 44: 0F EF. D2
	movd	xmm2, dword [r12+1458H] 		; 342B _ 66 41: 0F 6E. 94 24, 00001458
	pshufd	xmm2, xmm2, 0				; 3435 _ 66: 0F 70. D2, 00
	pxor	xmm9, xmm2				; 343A _ 66 44: 0F EF. CA
	movd	xmm2, dword [r12+145CH] 		; 343F _ 66 41: 0F 6E. 94 24, 0000145C
	pshufd	xmm2, xmm2, 0				; 3449 _ 66: 0F 70. D2, 00
	pxor	xmm0, xmm2				; 344E _ 66: 0F EF. C2
	movdqa	xmm2, xmm9				; 3452 _ 66 41: 0F 6F. D1
	psrld	xmm9, 22				; 3457 _ 66 41: 0F 72. D1, 16
	pslld	xmm2, 10				; 345D _ 66: 0F 72. F2, 0A
	por	xmm2, xmm9				; 3462 _ 66 41: 0F EB. D1
	movdqa	xmm9, xmm11				; 3467 _ 66 45: 0F 6F. CB
	psrld	xmm11, 5				; 346C _ 66 41: 0F 72. D3, 05
	pxor	xmm2, xmm0				; 3472 _ 66: 0F EF. D0
	pslld	xmm9, 27				; 3476 _ 66 41: 0F 72. F1, 1B
	por	xmm9, xmm11				; 347C _ 66 45: 0F EB. CB
	movdqa	xmm11, xmm10				; 3481 _ 66 45: 0F 6F. DA
	pxor	xmm9, xmm10				; 3486 _ 66 45: 0F EF. CA
	pxor	xmm9, xmm0				; 348B _ 66 44: 0F EF. C8
	pslld	xmm11, 7				; 3490 _ 66 41: 0F 72. F3, 07
	pxor	xmm2, xmm11				; 3496 _ 66 41: 0F EF. D3
	movdqa	xmm11, xmm0				; 349B _ 66 44: 0F 6F. D8
	movdqa	xmm1, xmm2				; 34A0 _ 66: 0F 6F. CA
	psrld	xmm0, 7 				; 34A4 _ 66: 0F 72. D0, 07
	pslld	xmm11, 25				; 34A9 _ 66 41: 0F 72. F3, 19
	por	xmm11, xmm0				; 34AF _ 66 44: 0F EB. D8
	movdqa	xmm0, xmm10				; 34B4 _ 66 41: 0F 6F. C2
	psrld	xmm10, 1				; 34B9 _ 66 41: 0F 72. D2, 01
	pxor	xmm11, xmm2				; 34BF _ 66 44: 0F EF. DA
	pslld	xmm0, 31				; 34C4 _ 66: 0F 72. F0, 1F
	por	xmm0, xmm10				; 34C9 _ 66 41: 0F EB. C2
	movdqa	xmm10, xmm9				; 34CE _ 66 45: 0F 6F. D1
	pxor	xmm0, xmm9				; 34D3 _ 66 41: 0F EF. C1
	pxor	xmm0, xmm2				; 34D8 _ 66: 0F EF. C2
	pslld	xmm10, 3				; 34DC _ 66 41: 0F 72. F2, 03
	pxor	xmm11, xmm10				; 34E2 _ 66 45: 0F EF. DA
	pslld	xmm1, 29				; 34E7 _ 66: 0F 72. F1, 1D
	psrld	xmm2, 3 				; 34EC _ 66: 0F 72. D2, 03
	por	xmm1, xmm2				; 34F1 _ 66: 0F EB. CA
	movdqa	xmm2, xmm9				; 34F5 _ 66 41: 0F 6F. D1
	movdqa	xmm10, xmm1				; 34FA _ 66 44: 0F 6F. D1
	psrld	xmm9, 13				; 34FF _ 66 41: 0F 72. D1, 0D
	pslld	xmm2, 19				; 3505 _ 66: 0F 72. F2, 13
	por	xmm2, xmm9				; 350A _ 66 41: 0F EB. D1
	pand	xmm10, xmm11				; 350F _ 66 45: 0F DB. D3
	pxor	xmm10, xmm0				; 3514 _ 66 44: 0F EF. D0
	por	xmm0, xmm11				; 3519 _ 66 41: 0F EB. C3
	movd	xmm9, dword [r12+1440H] 		; 351E _ 66 45: 0F 6E. 8C 24, 00001440
	pand	xmm0, xmm2				; 3528 _ 66: 0F DB. C2
	pxor	xmm1, xmm10				; 352C _ 66 41: 0F EF. CA
	pxor	xmm1, xmm0				; 3531 _ 66: 0F EF. C8
	pand	xmm0, xmm10				; 3535 _ 66 41: 0F DB. C2
	pxor	xmm2, xmm6				; 353A _ 66: 0F EF. D6
	pxor	xmm11, xmm1				; 353E _ 66 44: 0F EF. D9
	pxor	xmm0, xmm11				; 3543 _ 66 41: 0F EF. C3
	pand	xmm11, xmm2				; 3548 _ 66 44: 0F DB. DA
	pxor	xmm11, xmm10				; 354D _ 66 45: 0F EF. DA
	pxor	xmm2, xmm0				; 3552 _ 66: 0F EF. D0
	pand	xmm10, xmm2				; 3556 _ 66 44: 0F DB. D2
	pxor	xmm11, xmm2				; 355B _ 66 44: 0F EF. DA
	pxor	xmm10, xmm1				; 3560 _ 66 44: 0F EF. D1
	por	xmm10, xmm11				; 3565 _ 66 45: 0F EB. D3
	pxor	xmm11, xmm2				; 356A _ 66 44: 0F EF. DA
	pxor	xmm10, xmm0				; 356F _ 66 44: 0F EF. D0
	pshufd	xmm9, xmm9, 0				; 3574 _ 66 45: 0F 70. C9, 00
	pxor	xmm2, xmm9				; 357A _ 66 41: 0F EF. D1
	movdqa	xmm0, xmm2				; 357F _ 66: 0F 6F. C2
	psrld	xmm2, 5 				; 3583 _ 66: 0F 72. D2, 05
	pslld	xmm0, 27				; 3588 _ 66: 0F 72. F0, 1B
	por	xmm0, xmm2				; 358D _ 66: 0F EB. C2
	movd	xmm9, dword [r12+1444H] 		; 3591 _ 66 45: 0F 6E. 8C 24, 00001444
	pshufd	xmm9, xmm9, 0				; 359B _ 66 45: 0F 70. C9, 00
	pxor	xmm11, xmm9				; 35A1 _ 66 45: 0F EF. D9
	movdqa	xmm2, xmm11				; 35A6 _ 66 41: 0F 6F. D3
	pxor	xmm0, xmm11				; 35AB _ 66 41: 0F EF. C3
	pslld	xmm2, 7 				; 35B0 _ 66: 0F 72. F2, 07
	movd	xmm9, dword [r12+1448H] 		; 35B5 _ 66 45: 0F 6E. 8C 24, 00001448
	pshufd	xmm9, xmm9, 0				; 35BF _ 66 45: 0F 70. C9, 00
	pxor	xmm10, xmm9				; 35C5 _ 66 45: 0F EF. D1
	movd	xmm9, dword [r12+144CH] 		; 35CA _ 66 45: 0F 6E. 8C 24, 0000144C
	pshufd	xmm9, xmm9, 0				; 35D4 _ 66 45: 0F 70. C9, 00
	pxor	xmm1, xmm9				; 35DA _ 66 41: 0F EF. C9
	movdqa	xmm9, xmm10				; 35DF _ 66 45: 0F 6F. CA
	psrld	xmm10, 22				; 35E4 _ 66 41: 0F 72. D2, 16
	pxor	xmm0, xmm1				; 35EA _ 66: 0F EF. C1
	pslld	xmm9, 10				; 35EE _ 66 41: 0F 72. F1, 0A
	por	xmm9, xmm10				; 35F4 _ 66 45: 0F EB. CA
	movdqa	xmm10, xmm1				; 35F9 _ 66 44: 0F 6F. D1
	pxor	xmm9, xmm1				; 35FE _ 66 44: 0F EF. C9
	pxor	xmm9, xmm2				; 3603 _ 66 44: 0F EF. CA
	movdqa	xmm2, xmm11				; 3608 _ 66 41: 0F 6F. D3
	pslld	xmm10, 25				; 360D _ 66 41: 0F 72. F2, 19
	psrld	xmm1, 7 				; 3613 _ 66: 0F 72. D1, 07
	por	xmm10, xmm1				; 3618 _ 66 44: 0F EB. D1
	movdqa	xmm1, xmm9				; 361D _ 66 41: 0F 6F. C9
	pslld	xmm2, 31				; 3622 _ 66: 0F 72. F2, 1F
	psrld	xmm11, 1				; 3627 _ 66 41: 0F 72. D3, 01
	por	xmm2, xmm11				; 362D _ 66 41: 0F EB. D3
	movdqa	xmm11, xmm0				; 3632 _ 66 44: 0F 6F. D8
	pxor	xmm10, xmm9				; 3637 _ 66 45: 0F EF. D1
	pxor	xmm2, xmm0				; 363C _ 66: 0F EF. D0
	pslld	xmm11, 3				; 3640 _ 66 41: 0F 72. F3, 03
	pxor	xmm10, xmm11				; 3646 _ 66 45: 0F EF. D3
	movdqa	xmm11, xmm0				; 364B _ 66 44: 0F 6F. D8
	pxor	xmm2, xmm9				; 3650 _ 66 41: 0F EF. D1
	pslld	xmm1, 29				; 3655 _ 66: 0F 72. F1, 1D
	psrld	xmm9, 3 				; 365A _ 66 41: 0F 72. D1, 03
	por	xmm1, xmm9				; 3660 _ 66 41: 0F EB. C9
	movdqa	xmm9, xmm1				; 3665 _ 66 44: 0F 6F. C9
	pslld	xmm11, 19				; 366A _ 66 41: 0F 72. F3, 13
	psrld	xmm0, 13				; 3670 _ 66: 0F 72. D0, 0D
	por	xmm11, xmm0				; 3675 _ 66 44: 0F EB. D8
	pxor	xmm9, xmm2				; 367A _ 66 44: 0F EF. CA
	pxor	xmm11, xmm9				; 367F _ 66 45: 0F EF. D9
	pand	xmm1, xmm9				; 3684 _ 66 41: 0F DB. C9
	pxor	xmm1, xmm11				; 3689 _ 66 41: 0F EF. CB
	pand	xmm11, xmm2				; 368E _ 66 44: 0F DB. DA
	movd	xmm0, dword [r12+1430H] 		; 3693 _ 66 41: 0F 6E. 84 24, 00001430
	pxor	xmm2, xmm10				; 369D _ 66 41: 0F EF. D2
	por	xmm10, xmm1				; 36A2 _ 66 44: 0F EB. D1
	pxor	xmm9, xmm10				; 36A7 _ 66 45: 0F EF. CA
	pxor	xmm11, xmm10				; 36AC _ 66 45: 0F EF. DA
	pxor	xmm2, xmm1				; 36B1 _ 66: 0F EF. D1
	pand	xmm10, xmm9				; 36B5 _ 66 45: 0F DB. D1
	pxor	xmm10, xmm2				; 36BA _ 66 44: 0F EF. D2
	pxor	xmm2, xmm11				; 36BF _ 66 41: 0F EF. D3
	por	xmm2, xmm9				; 36C4 _ 66 41: 0F EB. D1
	pxor	xmm11, xmm10				; 36C9 _ 66 45: 0F EF. DA
	pxor	xmm2, xmm1				; 36CE _ 66: 0F EF. D1
	pxor	xmm11, xmm2				; 36D2 _ 66 44: 0F EF. DA
	pshufd	xmm0, xmm0, 0				; 36D7 _ 66: 0F 70. C0, 00
	pxor	xmm9, xmm0				; 36DC _ 66 44: 0F EF. C8
	movdqa	xmm1, xmm9				; 36E1 _ 66 41: 0F 6F. C9
	psrld	xmm9, 5 				; 36E6 _ 66 41: 0F 72. D1, 05
	pslld	xmm1, 27				; 36EC _ 66: 0F 72. F1, 1B
	por	xmm1, xmm9				; 36F1 _ 66 41: 0F EB. C9
	movd	xmm0, dword [r12+1434H] 		; 36F6 _ 66 41: 0F 6E. 84 24, 00001434
	pshufd	xmm0, xmm0, 0				; 3700 _ 66: 0F 70. C0, 00
	pxor	xmm2, xmm0				; 3705 _ 66: 0F EF. D0
	pxor	xmm1, xmm2				; 3709 _ 66: 0F EF. CA
	movd	xmm0, dword [r12+1438H] 		; 370D _ 66 41: 0F 6E. 84 24, 00001438
	pshufd	xmm0, xmm0, 0				; 3717 _ 66: 0F 70. C0, 00
	pxor	xmm10, xmm0				; 371C _ 66 44: 0F EF. D0
	movd	xmm0, dword [r12+143CH] 		; 3721 _ 66 41: 0F 6E. 84 24, 0000143C
	pshufd	xmm0, xmm0, 0				; 372B _ 66: 0F 70. C0, 00
	pxor	xmm11, xmm0				; 3730 _ 66 44: 0F EF. D8
	movdqa	xmm0, xmm10				; 3735 _ 66 41: 0F 6F. C2
	movdqa	xmm9, xmm11				; 373A _ 66 45: 0F 6F. CB
	psrld	xmm10, 22				; 373F _ 66 41: 0F 72. D2, 16
	pslld	xmm0, 10				; 3745 _ 66: 0F 72. F0, 0A
	por	xmm0, xmm10				; 374A _ 66 41: 0F EB. C2
	movdqa	xmm10, xmm2				; 374F _ 66 44: 0F 6F. D2
	pxor	xmm0, xmm11				; 3754 _ 66 41: 0F EF. C3
	pxor	xmm1, xmm11				; 3759 _ 66 41: 0F EF. CB
	pslld	xmm10, 7				; 375E _ 66 41: 0F 72. F2, 07
	pxor	xmm0, xmm10				; 3764 _ 66 41: 0F EF. C2
	movdqa	xmm10, xmm1				; 3769 _ 66 44: 0F 6F. D1
	pslld	xmm9, 25				; 376E _ 66 41: 0F 72. F1, 19
	psrld	xmm11, 7				; 3774 _ 66 41: 0F 72. D3, 07
	por	xmm9, xmm11				; 377A _ 66 45: 0F EB. CB
	movdqa	xmm11, xmm2				; 377F _ 66 44: 0F 6F. DA
	psrld	xmm2, 1 				; 3784 _ 66: 0F 72. D2, 01
	pxor	xmm9, xmm0				; 3789 _ 66 44: 0F EF. C8
	pslld	xmm11, 31				; 378E _ 66 41: 0F 72. F3, 1F
	por	xmm11, xmm2				; 3794 _ 66 44: 0F EB. DA
	movdqa	xmm2, xmm1				; 3799 _ 66: 0F 6F. D1
	pxor	xmm11, xmm1				; 379D _ 66 44: 0F EF. D9
	pxor	xmm11, xmm0				; 37A2 _ 66 44: 0F EF. D8
	pslld	xmm2, 3 				; 37A7 _ 66: 0F 72. F2, 03
	pxor	xmm9, xmm2				; 37AC _ 66 44: 0F EF. CA
	movdqa	xmm2, xmm0				; 37B1 _ 66: 0F 6F. D0
	psrld	xmm0, 3 				; 37B5 _ 66: 0F 72. D0, 03
	pslld	xmm10, 19				; 37BA _ 66 41: 0F 72. F2, 13
	pslld	xmm2, 29				; 37C0 _ 66: 0F 72. F2, 1D
	por	xmm2, xmm0				; 37C5 _ 66: 0F EB. D0
	psrld	xmm1, 13				; 37C9 _ 66: 0F 72. D1, 0D
	por	xmm10, xmm1				; 37CE _ 66 44: 0F EB. D1
	pxor	xmm2, xmm9				; 37D3 _ 66 41: 0F EF. D1
	pxor	xmm9, xmm10				; 37D8 _ 66 45: 0F EF. CA
	movdqa	xmm1, xmm9				; 37DD _ 66 41: 0F 6F. C9
	pand	xmm1, xmm2				; 37E2 _ 66: 0F DB. CA
	pxor	xmm1, xmm11				; 37E6 _ 66 41: 0F EF. CB
	por	xmm11, xmm2				; 37EB _ 66 44: 0F EB. DA
	pxor	xmm11, xmm9				; 37F0 _ 66 45: 0F EF. D9
	pand	xmm9, xmm1				; 37F5 _ 66 44: 0F DB. C9
	pxor	xmm2, xmm1				; 37FA _ 66: 0F EF. D1
	pand	xmm9, xmm10				; 37FE _ 66 45: 0F DB. CA
	pxor	xmm9, xmm2				; 3803 _ 66 44: 0F EF. CA
	pand	xmm2, xmm11				; 3808 _ 66 41: 0F DB. D3
	por	xmm2, xmm10				; 380D _ 66 41: 0F EB. D2
	pxor	xmm1, xmm6				; 3812 _ 66: 0F EF. CE
	movdqa	xmm0, xmm1				; 3816 _ 66: 0F 6F. C1
	pxor	xmm2, xmm1				; 381A _ 66: 0F EF. D1
	pxor	xmm10, xmm1				; 381E _ 66 44: 0F EF. D1
	pxor	xmm0, xmm9				; 3823 _ 66 41: 0F EF. C1
	pand	xmm10, xmm11				; 3828 _ 66 45: 0F DB. D3
	pxor	xmm0, xmm10				; 382D _ 66 41: 0F EF. C2
	movd	xmm10, dword [r12+1420H]		; 3832 _ 66 45: 0F 6E. 94 24, 00001420
	pshufd	xmm10, xmm10, 0 			; 383C _ 66 45: 0F 70. D2, 00
	pxor	xmm11, xmm10				; 3842 _ 66 45: 0F EF. DA
	movd	xmm10, dword [r12+1424H]		; 3847 _ 66 45: 0F 6E. 94 24, 00001424
	pshufd	xmm10, xmm10, 0 			; 3851 _ 66 45: 0F 70. D2, 00
	pxor	xmm9, xmm10				; 3857 _ 66 45: 0F EF. CA
	movd	xmm10, dword [r12+1428H]		; 385C _ 66 45: 0F 6E. 94 24, 00001428
	pshufd	xmm10, xmm10, 0 			; 3866 _ 66 45: 0F 70. D2, 00
	pxor	xmm2, xmm10				; 386C _ 66 41: 0F EF. D2
	movd	xmm10, dword [r12+142CH]		; 3871 _ 66 45: 0F 6E. 94 24, 0000142C
	movdqa	xmm1, xmm2				; 387B _ 66: 0F 6F. CA
	pshufd	xmm10, xmm10, 0 			; 387F _ 66 45: 0F 70. D2, 00
	pxor	xmm0, xmm10				; 3885 _ 66 41: 0F EF. C2
	movdqa	xmm10, xmm11				; 388A _ 66 45: 0F 6F. D3
	pslld	xmm1, 10				; 388F _ 66: 0F 72. F1, 0A
	psrld	xmm2, 22				; 3894 _ 66: 0F 72. D2, 16
	por	xmm1, xmm2				; 3899 _ 66: 0F EB. CA
	movdqa	xmm2, xmm9				; 389D _ 66 41: 0F 6F. D1
	pslld	xmm10, 27				; 38A2 _ 66 41: 0F 72. F2, 1B
	psrld	xmm11, 5				; 38A8 _ 66 41: 0F 72. D3, 05
	por	xmm10, xmm11				; 38AE _ 66 45: 0F EB. D3
	movdqa	xmm11, xmm9				; 38B3 _ 66 45: 0F 6F. D9
	pxor	xmm1, xmm0				; 38B8 _ 66: 0F EF. C8
	pslld	xmm2, 7 				; 38BC _ 66: 0F 72. F2, 07
	pxor	xmm1, xmm2				; 38C1 _ 66: 0F EF. CA
	movdqa	xmm2, xmm0				; 38C5 _ 66: 0F 6F. D0
	pxor	xmm10, xmm9				; 38C9 _ 66 45: 0F EF. D1
	pxor	xmm10, xmm0				; 38CE _ 66 44: 0F EF. D0
	pslld	xmm2, 25				; 38D3 _ 66: 0F 72. F2, 19
	psrld	xmm0, 7 				; 38D8 _ 66: 0F 72. D0, 07
	por	xmm2, xmm0				; 38DD _ 66: 0F EB. D0
	movdqa	xmm0, xmm1				; 38E1 _ 66: 0F 6F. C1
	pslld	xmm11, 31				; 38E5 _ 66 41: 0F 72. F3, 1F
	psrld	xmm9, 1 				; 38EB _ 66 41: 0F 72. D1, 01
	por	xmm11, xmm9				; 38F1 _ 66 45: 0F EB. D9
	movdqa	xmm9, xmm10				; 38F6 _ 66 45: 0F 6F. CA
	pxor	xmm2, xmm1				; 38FB _ 66: 0F EF. D1
	pxor	xmm11, xmm10				; 38FF _ 66 45: 0F EF. DA
	pslld	xmm9, 3 				; 3904 _ 66 41: 0F 72. F1, 03
	pxor	xmm2, xmm9				; 390A _ 66 41: 0F EF. D1
	movdqa	xmm9, xmm10				; 390F _ 66 45: 0F 6F. CA
	pxor	xmm11, xmm1				; 3914 _ 66 44: 0F EF. D9
	pslld	xmm0, 29				; 3919 _ 66: 0F 72. F0, 1D
	psrld	xmm1, 3 				; 391E _ 66: 0F 72. D1, 03
	por	xmm0, xmm1				; 3923 _ 66: 0F EB. C1
	pslld	xmm9, 19				; 3927 _ 66 41: 0F 72. F1, 13
	psrld	xmm10, 13				; 392D _ 66 41: 0F 72. D2, 0D
	por	xmm9, xmm10				; 3933 _ 66 45: 0F EB. CA
	movdqa	xmm10, xmm11				; 3938 _ 66 45: 0F 6F. D3
	pxor	xmm11, xmm0				; 393D _ 66 44: 0F EF. D8
	pxor	xmm10, xmm2				; 3942 _ 66 44: 0F EF. D2
	pand	xmm2, xmm10				; 3947 _ 66 41: 0F DB. D2
	pxor	xmm2, xmm9				; 394C _ 66 41: 0F EF. D1
	por	xmm9, xmm10				; 3951 _ 66 45: 0F EB. CA
	pxor	xmm0, xmm2				; 3956 _ 66: 0F EF. C2
	pxor	xmm9, xmm11				; 395A _ 66 45: 0F EF. CB
	por	xmm9, xmm0				; 395F _ 66 44: 0F EB. C8
	pxor	xmm10, xmm2				; 3964 _ 66 44: 0F EF. D2
	pxor	xmm9, xmm10				; 3969 _ 66 45: 0F EF. CA
	por	xmm10, xmm2				; 396E _ 66 44: 0F EB. D2
	pxor	xmm10, xmm9				; 3973 _ 66 45: 0F EF. D1
	pxor	xmm11, xmm6				; 3978 _ 66 44: 0F EF. DE
	pxor	xmm11, xmm10				; 397D _ 66 45: 0F EF. DA
	por	xmm10, xmm9				; 3982 _ 66 45: 0F EB. D1
	pxor	xmm10, xmm9				; 3987 _ 66 45: 0F EF. D1
	por	xmm10, xmm11				; 398C _ 66 45: 0F EB. D3
	pxor	xmm2, xmm10				; 3991 _ 66 41: 0F EF. D2
	movd	xmm10, dword [r12+1410H]		; 3996 _ 66 45: 0F 6E. 94 24, 00001410
	pshufd	xmm10, xmm10, 0 			; 39A0 _ 66 45: 0F 70. D2, 00
	pxor	xmm11, xmm10				; 39A6 _ 66 45: 0F EF. DA
	movdqa	xmm1, xmm11				; 39AB _ 66 41: 0F 6F. CB
	psrld	xmm11, 5				; 39B0 _ 66 41: 0F 72. D3, 05
	pslld	xmm1, 27				; 39B6 _ 66: 0F 72. F1, 1B
	por	xmm1, xmm11				; 39BB _ 66 41: 0F EB. CB
	movd	xmm10, dword [r12+1414H]		; 39C0 _ 66 45: 0F 6E. 94 24, 00001414
	pshufd	xmm10, xmm10, 0 			; 39CA _ 66 45: 0F 70. D2, 00
	pxor	xmm9, xmm10				; 39D0 _ 66 45: 0F EF. CA
	movdqa	xmm11, xmm9				; 39D5 _ 66 45: 0F 6F. D9
	pxor	xmm1, xmm9				; 39DA _ 66 41: 0F EF. C9
	pslld	xmm11, 7				; 39DF _ 66 41: 0F 72. F3, 07
	movd	xmm10, dword [r12+1418H]		; 39E5 _ 66 45: 0F 6E. 94 24, 00001418
	pshufd	xmm10, xmm10, 0 			; 39EF _ 66 45: 0F 70. D2, 00
	pxor	xmm2, xmm10				; 39F5 _ 66 41: 0F EF. D2
	movd	xmm10, dword [r12+141CH]		; 39FA _ 66 45: 0F 6E. 94 24, 0000141C
	pshufd	xmm10, xmm10, 0 			; 3A04 _ 66 45: 0F 70. D2, 00
	pxor	xmm0, xmm10				; 3A0A _ 66 41: 0F EF. C2
	movdqa	xmm10, xmm2				; 3A0F _ 66 44: 0F 6F. D2
	psrld	xmm2, 22				; 3A14 _ 66: 0F 72. D2, 16
	pxor	xmm1, xmm0				; 3A19 _ 66: 0F EF. C8
	pslld	xmm10, 10				; 3A1D _ 66 41: 0F 72. F2, 0A
	por	xmm10, xmm2				; 3A23 _ 66 44: 0F EB. D2
	movdqa	xmm2, xmm0				; 3A28 _ 66: 0F 6F. D0
	pxor	xmm10, xmm0				; 3A2C _ 66 44: 0F EF. D0
	pxor	xmm10, xmm11				; 3A31 _ 66 45: 0F EF. D3
	movdqa	xmm11, xmm9				; 3A36 _ 66 45: 0F 6F. D9
	pslld	xmm2, 25				; 3A3B _ 66: 0F 72. F2, 19
	psrld	xmm0, 7 				; 3A40 _ 66: 0F 72. D0, 07
	por	xmm2, xmm0				; 3A45 _ 66: 0F EB. D0
	movdqa	xmm0, xmm1				; 3A49 _ 66: 0F 6F. C1
	pslld	xmm11, 31				; 3A4D _ 66 41: 0F 72. F3, 1F
	psrld	xmm9, 1 				; 3A53 _ 66 41: 0F 72. D1, 01
	por	xmm11, xmm9				; 3A59 _ 66 45: 0F EB. D9
	movdqa	xmm9, xmm1				; 3A5E _ 66 44: 0F 6F. C9
	pxor	xmm2, xmm10				; 3A63 _ 66 41: 0F EF. D2
	pxor	xmm11, xmm1				; 3A68 _ 66 44: 0F EF. D9
	pslld	xmm9, 3 				; 3A6D _ 66 41: 0F 72. F1, 03
	pxor	xmm2, xmm9				; 3A73 _ 66 41: 0F EF. D1
	movdqa	xmm9, xmm10				; 3A78 _ 66 45: 0F 6F. CA
	pxor	xmm11, xmm10				; 3A7D _ 66 45: 0F EF. DA
	psrld	xmm10, 3				; 3A82 _ 66 41: 0F 72. D2, 03
	pslld	xmm9, 29				; 3A88 _ 66 41: 0F 72. F1, 1D
	por	xmm9, xmm10				; 3A8E _ 66 45: 0F EB. CA
	movdqa	xmm10, xmm11				; 3A93 _ 66 45: 0F 6F. D3
	pslld	xmm0, 19				; 3A98 _ 66: 0F 72. F0, 13
	psrld	xmm1, 13				; 3A9D _ 66: 0F 72. D1, 0D
	por	xmm0, xmm1				; 3AA2 _ 66: 0F EB. C1
	pxor	xmm9, xmm6				; 3AA6 _ 66 44: 0F EF. CE
	por	xmm10, xmm0				; 3AAB _ 66 44: 0F EB. D0
	pxor	xmm11, xmm6				; 3AB0 _ 66 44: 0F EF. DE
	pxor	xmm10, xmm9				; 3AB5 _ 66 45: 0F EF. D1
	por	xmm9, xmm11				; 3ABA _ 66 45: 0F EB. CB
	pxor	xmm10, xmm2				; 3ABF _ 66 44: 0F EF. D2
	pxor	xmm0, xmm11				; 3AC4 _ 66 41: 0F EF. C3
	pxor	xmm9, xmm0				; 3AC9 _ 66 44: 0F EF. C8
	pand	xmm0, xmm2				; 3ACE _ 66: 0F DB. C2
	pxor	xmm11, xmm0				; 3AD2 _ 66 44: 0F EF. D8
	por	xmm0, xmm10				; 3AD7 _ 66 41: 0F EB. C2
	pxor	xmm0, xmm9				; 3ADC _ 66 41: 0F EF. C1
	pxor	xmm2, xmm11				; 3AE1 _ 66 41: 0F EF. D3
	pxor	xmm9, xmm10				; 3AE6 _ 66 45: 0F EF. CA
	pxor	xmm2, xmm0				; 3AEB _ 66: 0F EF. D0
	pxor	xmm2, xmm10				; 3AEF _ 66 41: 0F EF. D2
	pand	xmm9, xmm2				; 3AF4 _ 66 44: 0F DB. CA
	pxor	xmm11, xmm9				; 3AF9 _ 66 45: 0F EF. D9
	movd	xmm9, dword [r12+1400H] 		; 3AFE _ 66 45: 0F 6E. 8C 24, 00001400
	pshufd	xmm9, xmm9, 0				; 3B08 _ 66 45: 0F 70. C9, 00
	pxor	xmm0, xmm9				; 3B0E _ 66 41: 0F EF. C1
	movd	xmm9, dword [r12+1404H] 		; 3B13 _ 66 45: 0F 6E. 8C 24, 00001404
	pshufd	xmm9, xmm9, 0				; 3B1D _ 66 45: 0F 70. C9, 00
	pxor	xmm11, xmm9				; 3B23 _ 66 45: 0F EF. D9
	movd	xmm9, dword [r12+1408H] 		; 3B28 _ 66 45: 0F 6E. 8C 24, 00001408
	pshufd	xmm9, xmm9, 0				; 3B32 _ 66 45: 0F 70. C9, 00
	pxor	xmm10, xmm9				; 3B38 _ 66 45: 0F EF. D1
	movdqa	xmm1, xmm10				; 3B3D _ 66 41: 0F 6F. CA
	psrld	xmm10, 22				; 3B42 _ 66 41: 0F 72. D2, 16
	pslld	xmm1, 10				; 3B48 _ 66: 0F 72. F1, 0A
	por	xmm1, xmm10				; 3B4D _ 66 41: 0F EB. CA
	movdqa	xmm10, xmm0				; 3B52 _ 66 44: 0F 6F. D0
	movd	xmm9, dword [r12+140CH] 		; 3B57 _ 66 45: 0F 6E. 8C 24, 0000140C
	pshufd	xmm9, xmm9, 0				; 3B61 _ 66 45: 0F 70. C9, 00
	pxor	xmm2, xmm9				; 3B67 _ 66 41: 0F EF. D1
	movdqa	xmm9, xmm11				; 3B6C _ 66 45: 0F 6F. CB
	pslld	xmm10, 27				; 3B71 _ 66 41: 0F 72. F2, 1B
	psrld	xmm0, 5 				; 3B77 _ 66: 0F 72. D0, 05
	por	xmm10, xmm0				; 3B7C _ 66 44: 0F EB. D0
	movdqa	xmm0, xmm11				; 3B81 _ 66 41: 0F 6F. C3
	pxor	xmm1, xmm2				; 3B86 _ 66: 0F EF. CA
	pslld	xmm9, 7 				; 3B8A _ 66 41: 0F 72. F1, 07
	pxor	xmm1, xmm9				; 3B90 _ 66 41: 0F EF. C9
	movdqa	xmm9, xmm2				; 3B95 _ 66 44: 0F 6F. CA
	pxor	xmm10, xmm11				; 3B9A _ 66 45: 0F EF. D3
	pxor	xmm10, xmm2				; 3B9F _ 66 44: 0F EF. D2
	pslld	xmm9, 25				; 3BA4 _ 66 41: 0F 72. F1, 19
	psrld	xmm2, 7 				; 3BAA _ 66: 0F 72. D2, 07
	por	xmm9, xmm2				; 3BAF _ 66 44: 0F EB. CA
	movdqa	xmm2, xmm10				; 3BB4 _ 66 41: 0F 6F. D2
	pslld	xmm0, 31				; 3BB9 _ 66: 0F 72. F0, 1F
	psrld	xmm11, 1				; 3BBE _ 66 41: 0F 72. D3, 01
	por	xmm0, xmm11				; 3BC4 _ 66 41: 0F EB. C3
	movdqa	xmm11, xmm10				; 3BC9 _ 66 45: 0F 6F. DA
	pxor	xmm9, xmm1				; 3BCE _ 66 44: 0F EF. C9
	pslld	xmm2, 3 				; 3BD3 _ 66: 0F 72. F2, 03
	pxor	xmm9, xmm2				; 3BD8 _ 66 44: 0F EF. CA
	movdqa	xmm2, xmm1				; 3BDD _ 66: 0F 6F. D1
	pxor	xmm0, xmm10				; 3BE1 _ 66 41: 0F EF. C2
	pxor	xmm0, xmm1				; 3BE6 _ 66: 0F EF. C1
	pslld	xmm2, 29				; 3BEA _ 66: 0F 72. F2, 1D
	psrld	xmm1, 3 				; 3BEF _ 66: 0F 72. D1, 03
	por	xmm2, xmm1				; 3BF4 _ 66: 0F EB. D1
	pslld	xmm11, 19				; 3BF8 _ 66 41: 0F 72. F3, 13
	psrld	xmm10, 13				; 3BFE _ 66 41: 0F 72. D2, 0D
	por	xmm11, xmm10				; 3C04 _ 66 45: 0F EB. DA
	movdqa	xmm10, xmm2				; 3C09 _ 66 44: 0F 6F. D2
	por	xmm2, xmm9				; 3C0E _ 66 41: 0F EB. D1
	pxor	xmm10, xmm11				; 3C13 _ 66 45: 0F EF. D3
	pand	xmm11, xmm9				; 3C18 _ 66 45: 0F DB. D9
	pxor	xmm10, xmm6				; 3C1D _ 66 44: 0F EF. D6
	pxor	xmm9, xmm0				; 3C22 _ 66 44: 0F EF. C8
	por	xmm0, xmm11				; 3C27 _ 66 41: 0F EB. C3
	pxor	xmm11, xmm10				; 3C2C _ 66 45: 0F EF. DA
	pand	xmm10, xmm2				; 3C31 _ 66 44: 0F DB. D2
	pand	xmm9, xmm2				; 3C36 _ 66 44: 0F DB. CA
	pxor	xmm0, xmm10				; 3C3B _ 66 41: 0F EF. C2
	pxor	xmm10, xmm11				; 3C40 _ 66 45: 0F EF. D3
	por	xmm11, xmm10				; 3C45 _ 66 45: 0F EB. DA
	pxor	xmm2, xmm0				; 3C4A _ 66: 0F EF. D0
	pxor	xmm11, xmm9				; 3C4E _ 66 45: 0F EF. D9
	pxor	xmm9, xmm2				; 3C53 _ 66 44: 0F EF. CA
	por	xmm2, xmm11				; 3C58 _ 66 41: 0F EB. D3
	pxor	xmm9, xmm10				; 3C5D _ 66 45: 0F EF. CA
	pxor	xmm2, xmm10				; 3C62 _ 66 41: 0F EF. D2
	movd	xmm10, dword [r12+13F0H]		; 3C67 _ 66 45: 0F 6E. 94 24, 000013F0
	pshufd	xmm10, xmm10, 0 			; 3C71 _ 66 45: 0F 70. D2, 00
	pxor	xmm9, xmm10				; 3C77 _ 66 45: 0F EF. CA
	movd	xmm10, dword [r12+13F4H]		; 3C7C _ 66 45: 0F 6E. 94 24, 000013F4
	pshufd	xmm10, xmm10, 0 			; 3C86 _ 66 45: 0F 70. D2, 00
	pxor	xmm11, xmm10				; 3C8C _ 66 45: 0F EF. DA
	movd	xmm10, dword [r12+13F8H]		; 3C91 _ 66 45: 0F 6E. 94 24, 000013F8
	pshufd	xmm10, xmm10, 0 			; 3C9B _ 66 45: 0F 70. D2, 00
	pxor	xmm0, xmm10				; 3CA1 _ 66 41: 0F EF. C2
	movdqa	xmm1, xmm0				; 3CA6 _ 66: 0F 6F. C8
	psrld	xmm0, 22				; 3CAA _ 66: 0F 72. D0, 16
	movd	xmm10, dword [r12+13FCH]		; 3CAF _ 66 45: 0F 6E. 94 24, 000013FC
	pshufd	xmm10, xmm10, 0 			; 3CB9 _ 66 45: 0F 70. D2, 00
	pxor	xmm2, xmm10				; 3CBF _ 66 41: 0F EF. D2
	movdqa	xmm10, xmm9				; 3CC4 _ 66 45: 0F 6F. D1
	pslld	xmm1, 10				; 3CC9 _ 66: 0F 72. F1, 0A
	por	xmm1, xmm0				; 3CCE _ 66: 0F EB. C8
	movdqa	xmm0, xmm2				; 3CD2 _ 66: 0F 6F. C2
	pslld	xmm10, 27				; 3CD6 _ 66 41: 0F 72. F2, 1B
	psrld	xmm9, 5 				; 3CDC _ 66 41: 0F 72. D1, 05
	por	xmm10, xmm9				; 3CE2 _ 66 45: 0F EB. D1
	movdqa	xmm9, xmm11				; 3CE7 _ 66 45: 0F 6F. CB
	pxor	xmm1, xmm2				; 3CEC _ 66: 0F EF. CA
	pxor	xmm10, xmm11				; 3CF0 _ 66 45: 0F EF. D3
	pslld	xmm9, 7 				; 3CF5 _ 66 41: 0F 72. F1, 07
	pxor	xmm1, xmm9				; 3CFB _ 66 41: 0F EF. C9
	movdqa	xmm9, xmm11				; 3D00 _ 66 45: 0F 6F. CB
	pxor	xmm10, xmm2				; 3D05 _ 66 44: 0F EF. D2
	pslld	xmm0, 25				; 3D0A _ 66: 0F 72. F0, 19
	psrld	xmm2, 7 				; 3D0F _ 66: 0F 72. D2, 07
	por	xmm0, xmm2				; 3D14 _ 66: 0F EB. C2
	movdqa	xmm2, xmm1				; 3D18 _ 66: 0F 6F. D1
	pslld	xmm9, 31				; 3D1C _ 66 41: 0F 72. F1, 1F
	psrld	xmm11, 1				; 3D22 _ 66 41: 0F 72. D3, 01
	por	xmm9, xmm11				; 3D28 _ 66 45: 0F EB. CB
	movdqa	xmm11, xmm10				; 3D2D _ 66 45: 0F 6F. DA
	pxor	xmm0, xmm1				; 3D32 _ 66: 0F EF. C1
	pxor	xmm9, xmm10				; 3D36 _ 66 45: 0F EF. CA
	pslld	xmm11, 3				; 3D3B _ 66 41: 0F 72. F3, 03
	pxor	xmm0, xmm11				; 3D41 _ 66 41: 0F EF. C3
	movdqa	xmm11, xmm10				; 3D46 _ 66 45: 0F 6F. DA
	pxor	xmm9, xmm1				; 3D4B _ 66 44: 0F EF. C9
	pslld	xmm2, 29				; 3D50 _ 66: 0F 72. F2, 1D
	psrld	xmm1, 3 				; 3D55 _ 66: 0F 72. D1, 03
	por	xmm2, xmm1				; 3D5A _ 66: 0F EB. D1
	pslld	xmm11, 19				; 3D5E _ 66 41: 0F 72. F3, 13
	psrld	xmm10, 13				; 3D64 _ 66 41: 0F 72. D2, 0D
	por	xmm11, xmm10				; 3D6A _ 66 45: 0F EB. DA
	movdqa	xmm10, xmm2				; 3D6F _ 66 44: 0F 6F. D2
	pxor	xmm11, xmm2				; 3D74 _ 66 44: 0F EF. DA
	pxor	xmm2, xmm0				; 3D79 _ 66: 0F EF. D0
	pand	xmm10, xmm11				; 3D7D _ 66 45: 0F DB. D3
	pxor	xmm10, xmm6				; 3D82 _ 66 44: 0F EF. D6
	pxor	xmm0, xmm9				; 3D87 _ 66 41: 0F EF. C1
	pxor	xmm10, xmm0				; 3D8C _ 66 44: 0F EF. D0
	por	xmm2, xmm11				; 3D91 _ 66 41: 0F EB. D3
	pxor	xmm11, xmm10				; 3D96 _ 66 45: 0F EF. DA
	pxor	xmm0, xmm2				; 3D9B _ 66: 0F EF. C2
	pxor	xmm2, xmm9				; 3D9F _ 66 41: 0F EF. D1
	pand	xmm9, xmm0				; 3DA4 _ 66 44: 0F DB. C8
	pxor	xmm9, xmm11				; 3DA9 _ 66 45: 0F EF. CB
	pxor	xmm11, xmm0				; 3DAE _ 66 44: 0F EF. D8
	por	xmm11, xmm10				; 3DB3 _ 66 45: 0F EB. DA
	pxor	xmm0, xmm9				; 3DB8 _ 66 41: 0F EF. C1
	pxor	xmm2, xmm11				; 3DBD _ 66 41: 0F EF. D3
	movd	xmm11, dword [r12+13E0H]		; 3DC2 _ 66 45: 0F 6E. 9C 24, 000013E0
	pshufd	xmm11, xmm11, 0 			; 3DCC _ 66 45: 0F 70. DB, 00
	pxor	xmm9, xmm11				; 3DD2 _ 66 45: 0F EF. CB
	movd	xmm11, dword [r12+13E4H]		; 3DD7 _ 66 45: 0F 6E. 9C 24, 000013E4
	pshufd	xmm11, xmm11, 0 			; 3DE1 _ 66 45: 0F 70. DB, 00
	pxor	xmm10, xmm11				; 3DE7 _ 66 45: 0F EF. D3
	movd	xmm11, dword [r12+13E8H]		; 3DEC _ 66 45: 0F 6E. 9C 24, 000013E8
	pshufd	xmm11, xmm11, 0 			; 3DF6 _ 66 45: 0F 70. DB, 00
	pxor	xmm2, xmm11				; 3DFC _ 66 41: 0F EF. D3
	movdqa	xmm1, xmm2				; 3E01 _ 66: 0F 6F. CA
	movd	xmm11, dword [r12+13ECH]		; 3E05 _ 66 45: 0F 6E. 9C 24, 000013EC
	pshufd	xmm11, xmm11, 0 			; 3E0F _ 66 45: 0F 70. DB, 00
	pxor	xmm0, xmm11				; 3E15 _ 66 41: 0F EF. C3
	movdqa	xmm11, xmm9				; 3E1A _ 66 45: 0F 6F. D9
	pslld	xmm1, 10				; 3E1F _ 66: 0F 72. F1, 0A
	psrld	xmm2, 22				; 3E24 _ 66: 0F 72. D2, 16
	por	xmm1, xmm2				; 3E29 _ 66: 0F EB. CA
	movdqa	xmm2, xmm10				; 3E2D _ 66 41: 0F 6F. D2
	pslld	xmm11, 27				; 3E32 _ 66 41: 0F 72. F3, 1B
	psrld	xmm9, 5 				; 3E38 _ 66 41: 0F 72. D1, 05
	por	xmm11, xmm9				; 3E3E _ 66 45: 0F EB. D9
	pxor	xmm1, xmm0				; 3E43 _ 66: 0F EF. C8
	pslld	xmm2, 7 				; 3E47 _ 66: 0F 72. F2, 07
	pxor	xmm1, xmm2				; 3E4C _ 66: 0F EF. CA
	movdqa	xmm2, xmm0				; 3E50 _ 66: 0F 6F. D0
	pxor	xmm11, xmm10				; 3E54 _ 66 45: 0F EF. DA
	pxor	xmm11, xmm0				; 3E59 _ 66 44: 0F EF. D8
	movdqa	xmm9, xmm11				; 3E5E _ 66 45: 0F 6F. CB
	pslld	xmm2, 25				; 3E63 _ 66: 0F 72. F2, 19
	psrld	xmm0, 7 				; 3E68 _ 66: 0F 72. D0, 07
	por	xmm2, xmm0				; 3E6D _ 66: 0F EB. D0
	movdqa	xmm0, xmm10				; 3E71 _ 66 41: 0F 6F. C2
	psrld	xmm10, 1				; 3E76 _ 66 41: 0F 72. D2, 01
	pxor	xmm2, xmm1				; 3E7C _ 66: 0F EF. D1
	pslld	xmm0, 31				; 3E80 _ 66: 0F 72. F0, 1F
	por	xmm0, xmm10				; 3E85 _ 66 41: 0F EB. C2
	movdqa	xmm10, xmm11				; 3E8A _ 66 45: 0F 6F. D3
	pxor	xmm0, xmm11				; 3E8F _ 66 41: 0F EF. C3
	pxor	xmm0, xmm1				; 3E94 _ 66: 0F EF. C1
	pslld	xmm10, 3				; 3E98 _ 66 41: 0F 72. F2, 03
	pxor	xmm2, xmm10				; 3E9E _ 66 41: 0F EF. D2
	movdqa	xmm10, xmm1				; 3EA3 _ 66 44: 0F 6F. D1
	psrld	xmm1, 3 				; 3EA8 _ 66: 0F 72. D1, 03
	pslld	xmm9, 19				; 3EAD _ 66 41: 0F 72. F1, 13
	pslld	xmm10, 29				; 3EB3 _ 66 41: 0F 72. F2, 1D
	por	xmm10, xmm1				; 3EB9 _ 66 44: 0F EB. D1
	psrld	xmm11, 13				; 3EBE _ 66 41: 0F 72. D3, 0D
	por	xmm9, xmm11				; 3EC4 _ 66 45: 0F EB. CB
	movdqa	xmm11, xmm2				; 3EC9 _ 66 44: 0F 6F. DA
	pxor	xmm0, xmm6				; 3ECE _ 66: 0F EF. C6
	pxor	xmm10, xmm0				; 3ED2 _ 66 44: 0F EF. D0
	por	xmm11, xmm9				; 3ED7 _ 66 45: 0F EB. D9
	pxor	xmm11, xmm10				; 3EDC _ 66 45: 0F EF. DA
	por	xmm10, xmm0				; 3EE1 _ 66 44: 0F EB. D0
	pand	xmm10, xmm9				; 3EE6 _ 66 45: 0F DB. D1
	pxor	xmm2, xmm11				; 3EEB _ 66 41: 0F EF. D3
	pxor	xmm10, xmm2				; 3EF0 _ 66 44: 0F EF. D2
	por	xmm2, xmm9				; 3EF5 _ 66 41: 0F EB. D1
	pxor	xmm2, xmm0				; 3EFA _ 66: 0F EF. D0
	pand	xmm0, xmm10				; 3EFE _ 66 41: 0F DB. C2
	pxor	xmm0, xmm11				; 3F03 _ 66 41: 0F EF. C3
	pxor	xmm2, xmm10				; 3F08 _ 66 41: 0F EF. D2
	pand	xmm11, xmm2				; 3F0D _ 66 44: 0F DB. DA
	pxor	xmm2, xmm0				; 3F12 _ 66: 0F EF. D0
	pxor	xmm11, xmm2				; 3F16 _ 66 44: 0F EF. DA
	pxor	xmm2, xmm6				; 3F1B _ 66: 0F EF. D6
	pxor	xmm11, xmm9				; 3F1F _ 66 45: 0F EF. D9
	movd	xmm9, dword [r12+13D0H] 		; 3F24 _ 66 45: 0F 6E. 8C 24, 000013D0
	pshufd	xmm9, xmm9, 0				; 3F2E _ 66 45: 0F 70. C9, 00
	pxor	xmm0, xmm9				; 3F34 _ 66 41: 0F EF. C1
	movd	xmm9, dword [r12+13D4H] 		; 3F39 _ 66 45: 0F 6E. 8C 24, 000013D4
	pshufd	xmm9, xmm9, 0				; 3F43 _ 66 45: 0F 70. C9, 00
	pxor	xmm2, xmm9				; 3F49 _ 66 41: 0F EF. D1
	movd	xmm9, dword [r12+13D8H] 		; 3F4E _ 66 45: 0F 6E. 8C 24, 000013D8
	pshufd	xmm9, xmm9, 0				; 3F58 _ 66 45: 0F 70. C9, 00
	pxor	xmm11, xmm9				; 3F5E _ 66 45: 0F EF. D9
	movdqa	xmm1, xmm11				; 3F63 _ 66 41: 0F 6F. CB
	psrld	xmm11, 22				; 3F68 _ 66 41: 0F 72. D3, 16
	pslld	xmm1, 10				; 3F6E _ 66: 0F 72. F1, 0A
	por	xmm1, xmm11				; 3F73 _ 66 41: 0F EB. CB
	movdqa	xmm11, xmm2				; 3F78 _ 66 44: 0F 6F. DA
	pslld	xmm11, 7				; 3F7D _ 66 41: 0F 72. F3, 07
	movd	xmm9, dword [r12+13DCH] 		; 3F83 _ 66 45: 0F 6E. 8C 24, 000013DC
	pshufd	xmm9, xmm9, 0				; 3F8D _ 66 45: 0F 70. C9, 00
	pxor	xmm10, xmm9				; 3F93 _ 66 45: 0F EF. D1
	movdqa	xmm9, xmm0				; 3F98 _ 66 44: 0F 6F. C8
	psrld	xmm0, 5 				; 3F9D _ 66: 0F 72. D0, 05
	pxor	xmm1, xmm10				; 3FA2 _ 66 41: 0F EF. CA
	pslld	xmm9, 27				; 3FA7 _ 66 41: 0F 72. F1, 1B
	por	xmm9, xmm0				; 3FAD _ 66 44: 0F EB. C8
	movdqa	xmm0, xmm2				; 3FB2 _ 66: 0F 6F. C2
	pxor	xmm1, xmm11				; 3FB6 _ 66 41: 0F EF. CB
	movdqa	xmm11, xmm10				; 3FBB _ 66 45: 0F 6F. DA
	pxor	xmm9, xmm2				; 3FC0 _ 66 44: 0F EF. CA
	pxor	xmm9, xmm10				; 3FC5 _ 66 45: 0F EF. CA
	pslld	xmm11, 25				; 3FCA _ 66 41: 0F 72. F3, 19
	psrld	xmm10, 7				; 3FD0 _ 66 41: 0F 72. D2, 07
	por	xmm11, xmm10				; 3FD6 _ 66 45: 0F EB. DA
	movdqa	xmm10, xmm9				; 3FDB _ 66 45: 0F 6F. D1
	pslld	xmm0, 31				; 3FE0 _ 66: 0F 72. F0, 1F
	psrld	xmm2, 1 				; 3FE5 _ 66: 0F 72. D2, 01
	por	xmm0, xmm2				; 3FEA _ 66: 0F EB. C2
	movdqa	xmm2, xmm9				; 3FEE _ 66 41: 0F 6F. D1
	pxor	xmm11, xmm1				; 3FF3 _ 66 44: 0F EF. D9
	pxor	xmm0, xmm9				; 3FF8 _ 66 41: 0F EF. C1
	pslld	xmm2, 3 				; 3FFD _ 66: 0F 72. F2, 03
	pxor	xmm11, xmm2				; 4002 _ 66 44: 0F EF. DA
	movdqa	xmm2, xmm1				; 4007 _ 66: 0F 6F. D1
	pxor	xmm0, xmm1				; 400B _ 66: 0F EF. C1
	psrld	xmm1, 3 				; 400F _ 66: 0F 72. D1, 03
	pslld	xmm2, 29				; 4014 _ 66: 0F 72. F2, 1D
	por	xmm2, xmm1				; 4019 _ 66: 0F EB. D1
	pslld	xmm10, 19				; 401D _ 66 41: 0F 72. F2, 13
	psrld	xmm9, 13				; 4023 _ 66 41: 0F 72. D1, 0D
	por	xmm10, xmm9				; 4029 _ 66 45: 0F EB. D1
	movdqa	xmm9, xmm2				; 402E _ 66 44: 0F 6F. CA
	pand	xmm9, xmm11				; 4033 _ 66 45: 0F DB. CB
	pxor	xmm9, xmm0				; 4038 _ 66 44: 0F EF. C8
	por	xmm0, xmm11				; 403D _ 66 41: 0F EB. C3
	pand	xmm0, xmm10				; 4042 _ 66 41: 0F DB. C2
	pxor	xmm2, xmm9				; 4047 _ 66 41: 0F EF. D1
	pxor	xmm2, xmm0				; 404C _ 66: 0F EF. D0
	pand	xmm0, xmm9				; 4050 _ 66 41: 0F DB. C1
	pxor	xmm10, xmm6				; 4055 _ 66 44: 0F EF. D6
	pxor	xmm11, xmm2				; 405A _ 66 44: 0F EF. DA
	pxor	xmm0, xmm11				; 405F _ 66 41: 0F EF. C3
	pand	xmm11, xmm10				; 4064 _ 66 45: 0F DB. DA
	pxor	xmm11, xmm9				; 4069 _ 66 45: 0F EF. D9
	pxor	xmm10, xmm0				; 406E _ 66 44: 0F EF. D0
	pand	xmm9, xmm10				; 4073 _ 66 45: 0F DB. CA
	pxor	xmm11, xmm10				; 4078 _ 66 45: 0F EF. DA
	pxor	xmm9, xmm2				; 407D _ 66 44: 0F EF. CA
	por	xmm9, xmm11				; 4082 _ 66 45: 0F EB. CB
	pxor	xmm11, xmm10				; 4087 _ 66 45: 0F EF. DA
	pxor	xmm9, xmm0				; 408C _ 66 44: 0F EF. C8
	movd	xmm0, dword [r12+13C0H] 		; 4091 _ 66 41: 0F 6E. 84 24, 000013C0
	pshufd	xmm0, xmm0, 0				; 409B _ 66: 0F 70. C0, 00
	pxor	xmm10, xmm0				; 40A0 _ 66 44: 0F EF. D0
	movd	xmm0, dword [r12+13C4H] 		; 40A5 _ 66 41: 0F 6E. 84 24, 000013C4
	pshufd	xmm0, xmm0, 0				; 40AF _ 66: 0F 70. C0, 00
	pxor	xmm11, xmm0				; 40B4 _ 66 44: 0F EF. D8
	movd	xmm0, dword [r12+13C8H] 		; 40B9 _ 66 41: 0F 6E. 84 24, 000013C8
	pshufd	xmm0, xmm0, 0				; 40C3 _ 66: 0F 70. C0, 00
	pxor	xmm9, xmm0				; 40C8 _ 66 44: 0F EF. C8
	movdqa	xmm1, xmm9				; 40CD _ 66 41: 0F 6F. C9
	psrld	xmm9, 22				; 40D2 _ 66 41: 0F 72. D1, 16
	pslld	xmm1, 10				; 40D8 _ 66: 0F 72. F1, 0A
	por	xmm1, xmm9				; 40DD _ 66 41: 0F EB. C9
	movd	xmm0, dword [r12+13CCH] 		; 40E2 _ 66 41: 0F 6E. 84 24, 000013CC
	pshufd	xmm0, xmm0, 0				; 40EC _ 66: 0F 70. C0, 00
	pxor	xmm2, xmm0				; 40F1 _ 66: 0F EF. D0
	movdqa	xmm0, xmm10				; 40F5 _ 66 41: 0F 6F. C2
	psrld	xmm10, 5				; 40FA _ 66 41: 0F 72. D2, 05
	pxor	xmm1, xmm2				; 4100 _ 66: 0F EF. CA
	pslld	xmm0, 27				; 4104 _ 66: 0F 72. F0, 1B
	por	xmm0, xmm10				; 4109 _ 66 41: 0F EB. C2
	movdqa	xmm10, xmm11				; 410E _ 66 45: 0F 6F. D3
	pxor	xmm0, xmm11				; 4113 _ 66 41: 0F EF. C3
	pxor	xmm0, xmm2				; 4118 _ 66: 0F EF. C2
	movdqa	xmm9, xmm0				; 411C _ 66 44: 0F 6F. C8
	pslld	xmm10, 7				; 4121 _ 66 41: 0F 72. F2, 07
	pxor	xmm1, xmm10				; 4127 _ 66 41: 0F EF. CA
	movdqa	xmm10, xmm2				; 412C _ 66 44: 0F 6F. D2
	psrld	xmm2, 7 				; 4131 _ 66: 0F 72. D2, 07
	pslld	xmm9, 3 				; 4136 _ 66 41: 0F 72. F1, 03
	pslld	xmm10, 25				; 413C _ 66 41: 0F 72. F2, 19
	por	xmm10, xmm2				; 4142 _ 66 44: 0F EB. D2
	movdqa	xmm2, xmm11				; 4147 _ 66 41: 0F 6F. D3
	psrld	xmm11, 1				; 414C _ 66 41: 0F 72. D3, 01
	pxor	xmm10, xmm1				; 4152 _ 66 44: 0F EF. D1
	pslld	xmm2, 31				; 4157 _ 66: 0F 72. F2, 1F
	por	xmm2, xmm11				; 415C _ 66 41: 0F EB. D3
	movdqa	xmm11, xmm1				; 4161 _ 66 44: 0F 6F. D9
	pxor	xmm10, xmm9				; 4166 _ 66 45: 0F EF. D1
	pxor	xmm2, xmm0				; 416B _ 66: 0F EF. D0
	pxor	xmm2, xmm1				; 416F _ 66: 0F EF. D1
	pslld	xmm11, 29				; 4173 _ 66 41: 0F 72. F3, 1D
	psrld	xmm1, 3 				; 4179 _ 66: 0F 72. D1, 03
	por	xmm11, xmm1				; 417E _ 66 44: 0F EB. D9
	movdqa	xmm1, xmm0				; 4183 _ 66: 0F 6F. C8
	pslld	xmm1, 19				; 4187 _ 66: 0F 72. F1, 13
	movdqa	xmm9, xmm11				; 418C _ 66 45: 0F 6F. CB
	psrld	xmm0, 13				; 4191 _ 66: 0F 72. D0, 0D
	por	xmm1, xmm0				; 4196 _ 66: 0F EB. C8
	pxor	xmm9, xmm2				; 419A _ 66 44: 0F EF. CA
	pxor	xmm1, xmm9				; 419F _ 66 41: 0F EF. C9
	pand	xmm11, xmm9				; 41A4 _ 66 45: 0F DB. D9
	pxor	xmm11, xmm1				; 41A9 _ 66 44: 0F EF. D9
	pand	xmm1, xmm2				; 41AE _ 66: 0F DB. CA
	pxor	xmm2, xmm10				; 41B2 _ 66 41: 0F EF. D2
	por	xmm10, xmm11				; 41B7 _ 66 45: 0F EB. D3
	pxor	xmm9, xmm10				; 41BC _ 66 45: 0F EF. CA
	pxor	xmm1, xmm10				; 41C1 _ 66 41: 0F EF. CA
	pxor	xmm2, xmm11				; 41C6 _ 66 41: 0F EF. D3
	pand	xmm10, xmm9				; 41CB _ 66 45: 0F DB. D1
	pxor	xmm10, xmm2				; 41D0 _ 66 44: 0F EF. D2
	pxor	xmm2, xmm1				; 41D5 _ 66: 0F EF. D1
	por	xmm2, xmm9				; 41D9 _ 66 41: 0F EB. D1
	pxor	xmm1, xmm10				; 41DE _ 66 41: 0F EF. CA
	pxor	xmm2, xmm11				; 41E3 _ 66 41: 0F EF. D3
	pxor	xmm1, xmm2				; 41E8 _ 66: 0F EF. CA
	movd	xmm11, dword [r12+13B0H]		; 41EC _ 66 45: 0F 6E. 9C 24, 000013B0
	pshufd	xmm11, xmm11, 0 			; 41F6 _ 66 45: 0F 70. DB, 00
	pxor	xmm9, xmm11				; 41FC _ 66 45: 0F EF. CB
	movd	xmm11, dword [r12+13B4H]		; 4201 _ 66 45: 0F 6E. 9C 24, 000013B4
	pshufd	xmm11, xmm11, 0 			; 420B _ 66 45: 0F 70. DB, 00
	pxor	xmm2, xmm11				; 4211 _ 66 41: 0F EF. D3
	movd	xmm11, dword [r12+13B8H]		; 4216 _ 66 45: 0F 6E. 9C 24, 000013B8
	pshufd	xmm11, xmm11, 0 			; 4220 _ 66 45: 0F 70. DB, 00
	pxor	xmm10, xmm11				; 4226 _ 66 45: 0F EF. D3
	movdqa	xmm0, xmm10				; 422B _ 66 41: 0F 6F. C2
	psrld	xmm10, 22				; 4230 _ 66 41: 0F 72. D2, 16
	pslld	xmm0, 10				; 4236 _ 66: 0F 72. F0, 0A
	movd	xmm11, dword [r12+13BCH]		; 423B _ 66 45: 0F 6E. 9C 24, 000013BC
	pshufd	xmm11, xmm11, 0 			; 4245 _ 66 45: 0F 70. DB, 00
	pxor	xmm1, xmm11				; 424B _ 66 41: 0F EF. CB
	movdqa	xmm11, xmm9				; 4250 _ 66 45: 0F 6F. D9
	por	xmm0, xmm10				; 4255 _ 66 41: 0F EB. C2
	movdqa	xmm10, xmm2				; 425A _ 66 44: 0F 6F. D2
	pslld	xmm11, 27				; 425F _ 66 41: 0F 72. F3, 1B
	psrld	xmm9, 5 				; 4265 _ 66 41: 0F 72. D1, 05
	por	xmm11, xmm9				; 426B _ 66 45: 0F EB. D9
	movdqa	xmm9, xmm1				; 4270 _ 66 44: 0F 6F. C9
	pxor	xmm0, xmm1				; 4275 _ 66: 0F EF. C1
	pslld	xmm10, 7				; 4279 _ 66 41: 0F 72. F2, 07
	pxor	xmm0, xmm10				; 427F _ 66 41: 0F EF. C2
	movdqa	xmm10, xmm2				; 4284 _ 66 44: 0F 6F. D2
	pxor	xmm11, xmm2				; 4289 _ 66 44: 0F EF. DA
	pxor	xmm11, xmm1				; 428E _ 66 44: 0F EF. D9
	pslld	xmm9, 25				; 4293 _ 66 41: 0F 72. F1, 19
	psrld	xmm1, 7 				; 4299 _ 66: 0F 72. D1, 07
	por	xmm9, xmm1				; 429E _ 66 44: 0F EB. C9
	movdqa	xmm1, xmm0				; 42A3 _ 66: 0F 6F. C8
	pslld	xmm10, 31				; 42A7 _ 66 41: 0F 72. F2, 1F
	psrld	xmm2, 1 				; 42AD _ 66: 0F 72. D2, 01
	por	xmm10, xmm2				; 42B2 _ 66 44: 0F EB. D2
	movdqa	xmm2, xmm11				; 42B7 _ 66 41: 0F 6F. D3
	pxor	xmm9, xmm0				; 42BC _ 66 44: 0F EF. C8
	pxor	xmm10, xmm11				; 42C1 _ 66 45: 0F EF. D3
	pslld	xmm2, 3 				; 42C6 _ 66: 0F 72. F2, 03
	pxor	xmm9, xmm2				; 42CB _ 66 44: 0F EF. CA
	movdqa	xmm2, xmm11				; 42D0 _ 66 41: 0F 6F. D3
	pxor	xmm10, xmm0				; 42D5 _ 66 44: 0F EF. D0
	pslld	xmm1, 29				; 42DA _ 66: 0F 72. F1, 1D
	psrld	xmm0, 3 				; 42DF _ 66: 0F 72. D0, 03
	por	xmm1, xmm0				; 42E4 _ 66: 0F EB. C8
	pslld	xmm2, 19				; 42E8 _ 66: 0F 72. F2, 13
	psrld	xmm11, 13				; 42ED _ 66 41: 0F 72. D3, 0D
	por	xmm2, xmm11				; 42F3 _ 66 41: 0F EB. D3
	pxor	xmm1, xmm9				; 42F8 _ 66 41: 0F EF. C9
	pxor	xmm9, xmm2				; 42FD _ 66 44: 0F EF. CA
	movdqa	xmm11, xmm9				; 4302 _ 66 45: 0F 6F. D9
	pand	xmm11, xmm1				; 4307 _ 66 44: 0F DB. D9
	pxor	xmm11, xmm10				; 430C _ 66 45: 0F EF. DA
	por	xmm10, xmm1				; 4311 _ 66 44: 0F EB. D1
	pxor	xmm10, xmm9				; 4316 _ 66 45: 0F EF. D1
	pand	xmm9, xmm11				; 431B _ 66 45: 0F DB. CB
	pxor	xmm1, xmm11				; 4320 _ 66 41: 0F EF. CB
	pand	xmm9, xmm2				; 4325 _ 66 44: 0F DB. CA
	pxor	xmm9, xmm1				; 432A _ 66 44: 0F EF. C9
	pand	xmm1, xmm10				; 432F _ 66 41: 0F DB. CA
	por	xmm1, xmm2				; 4334 _ 66: 0F EB. CA
	pxor	xmm11, xmm6				; 4338 _ 66 44: 0F EF. DE
	movdqa	xmm0, xmm11				; 433D _ 66 41: 0F 6F. C3
	pxor	xmm1, xmm11				; 4342 _ 66 41: 0F EF. CB
	pxor	xmm2, xmm11				; 4347 _ 66 41: 0F EF. D3
	pxor	xmm0, xmm9				; 434C _ 66 41: 0F EF. C1
	pand	xmm2, xmm10				; 4351 _ 66 41: 0F DB. D2
	pxor	xmm0, xmm2				; 4356 _ 66: 0F EF. C2
	movd	xmm2, dword [r12+13A0H] 		; 435A _ 66 41: 0F 6E. 94 24, 000013A0
	pshufd	xmm11, xmm2, 0				; 4364 _ 66 44: 0F 70. DA, 00
	pxor	xmm10, xmm11				; 436A _ 66 45: 0F EF. D3
	movd	xmm2, dword [r12+13A4H] 		; 436F _ 66 41: 0F 6E. 94 24, 000013A4
	pshufd	xmm11, xmm2, 0				; 4379 _ 66 44: 0F 70. DA, 00
	pxor	xmm9, xmm11				; 437F _ 66 45: 0F EF. CB
	movd	xmm2, dword [r12+13A8H] 		; 4384 _ 66 41: 0F 6E. 94 24, 000013A8
	pshufd	xmm11, xmm2, 0				; 438E _ 66 44: 0F 70. DA, 00
	movd	xmm2, dword [r12+13ACH] 		; 4394 _ 66 41: 0F 6E. 94 24, 000013AC
	pxor	xmm1, xmm11				; 439E _ 66 41: 0F EF. CB
	pshufd	xmm11, xmm2, 0				; 43A3 _ 66 44: 0F 70. DA, 00
	movdqa	xmm2, xmm1				; 43A9 _ 66: 0F 6F. D1
	pxor	xmm0, xmm11				; 43AD _ 66 41: 0F EF. C3
	movdqa	xmm11, xmm10				; 43B2 _ 66 45: 0F 6F. DA
	pslld	xmm2, 10				; 43B7 _ 66: 0F 72. F2, 0A
	psrld	xmm1, 22				; 43BC _ 66: 0F 72. D1, 16
	por	xmm2, xmm1				; 43C1 _ 66: 0F EB. D1
	movdqa	xmm1, xmm0				; 43C5 _ 66: 0F 6F. C8
	pslld	xmm11, 27				; 43C9 _ 66 41: 0F 72. F3, 1B
	psrld	xmm10, 5				; 43CF _ 66 41: 0F 72. D2, 05
	por	xmm11, xmm10				; 43D5 _ 66 45: 0F EB. DA
	movdqa	xmm10, xmm9				; 43DA _ 66 45: 0F 6F. D1
	pxor	xmm2, xmm0				; 43DF _ 66: 0F EF. D0
	pxor	xmm11, xmm9				; 43E3 _ 66 45: 0F EF. D9
	pslld	xmm10, 7				; 43E8 _ 66 41: 0F 72. F2, 07
	pxor	xmm2, xmm10				; 43EE _ 66 41: 0F EF. D2
	pxor	xmm11, xmm0				; 43F3 _ 66 44: 0F EF. D8
	movd	xmm10, dword [r12+1390H]		; 43F8 _ 66 45: 0F 6E. 94 24, 00001390
	pslld	xmm1, 25				; 4402 _ 66: 0F 72. F1, 19
	psrld	xmm0, 7 				; 4407 _ 66: 0F 72. D0, 07
	por	xmm1, xmm0				; 440C _ 66: 0F EB. C8
	movdqa	xmm0, xmm9				; 4410 _ 66 41: 0F 6F. C1
	psrld	xmm9, 1 				; 4415 _ 66 41: 0F 72. D1, 01
	pxor	xmm1, xmm2				; 441B _ 66: 0F EF. CA
	pslld	xmm0, 31				; 441F _ 66: 0F 72. F0, 1F
	por	xmm0, xmm9				; 4424 _ 66 41: 0F EB. C1
	movdqa	xmm9, xmm11				; 4429 _ 66 45: 0F 6F. CB
	pxor	xmm0, xmm11				; 442E _ 66 41: 0F EF. C3
	pxor	xmm0, xmm2				; 4433 _ 66: 0F EF. C2
	pslld	xmm9, 3 				; 4437 _ 66 41: 0F 72. F1, 03
	pxor	xmm1, xmm9				; 443D _ 66 41: 0F EF. C9
	movdqa	xmm9, xmm2				; 4442 _ 66 44: 0F 6F. CA
	psrld	xmm2, 3 				; 4447 _ 66: 0F 72. D2, 03
	pslld	xmm9, 29				; 444C _ 66 41: 0F 72. F1, 1D
	por	xmm9, xmm2				; 4452 _ 66 44: 0F EB. CA
	movdqa	xmm2, xmm11				; 4457 _ 66 41: 0F 6F. D3
	psrld	xmm11, 13				; 445C _ 66 41: 0F 72. D3, 0D
	pslld	xmm2, 19				; 4462 _ 66: 0F 72. F2, 13
	por	xmm2, xmm11				; 4467 _ 66 41: 0F EB. D3
	movdqa	xmm11, xmm0				; 446C _ 66 44: 0F 6F. D8
	pxor	xmm0, xmm9				; 4471 _ 66 41: 0F EF. C1
	pxor	xmm11, xmm1				; 4476 _ 66 44: 0F EF. D9
	pand	xmm1, xmm11				; 447B _ 66 41: 0F DB. CB
	pxor	xmm1, xmm2				; 4480 _ 66: 0F EF. CA
	por	xmm2, xmm11				; 4484 _ 66 41: 0F EB. D3
	pxor	xmm9, xmm1				; 4489 _ 66 44: 0F EF. C9
	pxor	xmm2, xmm0				; 448E _ 66: 0F EF. D0
	por	xmm2, xmm9				; 4492 _ 66 41: 0F EB. D1
	pxor	xmm11, xmm1				; 4497 _ 66 44: 0F EF. D9
	pxor	xmm2, xmm11				; 449C _ 66 41: 0F EF. D3
	por	xmm11, xmm1				; 44A1 _ 66 44: 0F EB. D9
	pxor	xmm11, xmm2				; 44A6 _ 66 44: 0F EF. DA
	pxor	xmm0, xmm6				; 44AB _ 66: 0F EF. C6
	pxor	xmm0, xmm11				; 44AF _ 66 41: 0F EF. C3
	por	xmm11, xmm2				; 44B4 _ 66 44: 0F EB. DA
	pxor	xmm11, xmm2				; 44B9 _ 66 44: 0F EF. DA
	por	xmm11, xmm0				; 44BE _ 66 44: 0F EB. D8
	pxor	xmm1, xmm11				; 44C3 _ 66 41: 0F EF. CB
	pshufd	xmm11, xmm10, 0 			; 44C8 _ 66 45: 0F 70. DA, 00
	pxor	xmm0, xmm11				; 44CE _ 66 41: 0F EF. C3
	movd	xmm10, dword [r12+1394H]		; 44D3 _ 66 45: 0F 6E. 94 24, 00001394
	pshufd	xmm11, xmm10, 0 			; 44DD _ 66 45: 0F 70. DA, 00
	pxor	xmm2, xmm11				; 44E3 _ 66 41: 0F EF. D3
	movd	xmm10, dword [r12+1398H]		; 44E8 _ 66 45: 0F 6E. 94 24, 00001398
	pshufd	xmm11, xmm10, 0 			; 44F2 _ 66 45: 0F 70. DA, 00
	pxor	xmm1, xmm11				; 44F8 _ 66 41: 0F EF. CB
	movd	xmm10, dword [r12+139CH]		; 44FD _ 66 45: 0F 6E. 94 24, 0000139C
	pshufd	xmm11, xmm10, 0 			; 4507 _ 66 45: 0F 70. DA, 00
	movdqa	xmm10, xmm2				; 450D _ 66 44: 0F 6F. D2
	pxor	xmm9, xmm11				; 4512 _ 66 45: 0F EF. CB
	movdqa	xmm11, xmm1				; 4517 _ 66 44: 0F 6F. D9
	psrld	xmm1, 22				; 451C _ 66: 0F 72. D1, 16
	pslld	xmm10, 7				; 4521 _ 66 41: 0F 72. F2, 07
	pslld	xmm11, 10				; 4527 _ 66 41: 0F 72. F3, 0A
	por	xmm11, xmm1				; 452D _ 66 44: 0F EB. D9
	movdqa	xmm1, xmm0				; 4532 _ 66: 0F 6F. C8
	psrld	xmm0, 5 				; 4536 _ 66: 0F 72. D0, 05
	pxor	xmm11, xmm9				; 453B _ 66 45: 0F EF. D9
	pslld	xmm1, 27				; 4540 _ 66: 0F 72. F1, 1B
	por	xmm1, xmm0				; 4545 _ 66: 0F EB. C8
	movdqa	xmm0, xmm2				; 4549 _ 66: 0F 6F. C2
	pxor	xmm11, xmm10				; 454D _ 66 45: 0F EF. DA
	movdqa	xmm10, xmm9				; 4552 _ 66 45: 0F 6F. D1
	pxor	xmm1, xmm2				; 4557 _ 66: 0F EF. CA
	pxor	xmm1, xmm9				; 455B _ 66 41: 0F EF. C9
	pslld	xmm10, 25				; 4560 _ 66 41: 0F 72. F2, 19
	psrld	xmm9, 7 				; 4566 _ 66 41: 0F 72. D1, 07
	por	xmm10, xmm9				; 456C _ 66 45: 0F EB. D1
	movdqa	xmm9, xmm11				; 4571 _ 66 45: 0F 6F. CB
	pslld	xmm0, 31				; 4576 _ 66: 0F 72. F0, 1F
	psrld	xmm2, 1 				; 457B _ 66: 0F 72. D2, 01
	por	xmm0, xmm2				; 4580 _ 66: 0F EB. C2
	movdqa	xmm2, xmm1				; 4584 _ 66: 0F 6F. D1
	pxor	xmm10, xmm11				; 4588 _ 66 45: 0F EF. D3
	pxor	xmm0, xmm1				; 458D _ 66: 0F EF. C1
	pslld	xmm2, 3 				; 4591 _ 66: 0F 72. F2, 03
	pxor	xmm10, xmm2				; 4596 _ 66 44: 0F EF. D2
	movdqa	xmm2, xmm1				; 459B _ 66: 0F 6F. D1
	pxor	xmm0, xmm11				; 459F _ 66 41: 0F EF. C3
	pslld	xmm9, 29				; 45A4 _ 66 41: 0F 72. F1, 1D
	psrld	xmm11, 3				; 45AA _ 66 41: 0F 72. D3, 03
	por	xmm9, xmm11				; 45B0 _ 66 45: 0F EB. CB
	pslld	xmm2, 19				; 45B5 _ 66: 0F 72. F2, 13
	psrld	xmm1, 13				; 45BA _ 66: 0F 72. D1, 0D
	por	xmm2, xmm1				; 45BF _ 66: 0F EB. D1
	movdqa	xmm1, xmm0				; 45C3 _ 66: 0F 6F. C8
	pxor	xmm9, xmm6				; 45C7 _ 66 44: 0F EF. CE
	pxor	xmm0, xmm6				; 45CC _ 66: 0F EF. C6
	por	xmm1, xmm2				; 45D0 _ 66: 0F EB. CA
	pxor	xmm1, xmm9				; 45D4 _ 66 41: 0F EF. C9
	por	xmm9, xmm0				; 45D9 _ 66 44: 0F EB. C8
	pxor	xmm1, xmm10				; 45DE _ 66 41: 0F EF. CA
	pxor	xmm2, xmm0				; 45E3 _ 66: 0F EF. D0
	pxor	xmm9, xmm2				; 45E7 _ 66 44: 0F EF. CA
	pand	xmm2, xmm10				; 45EC _ 66 41: 0F DB. D2
	pxor	xmm0, xmm2				; 45F1 _ 66: 0F EF. C2
	por	xmm2, xmm1				; 45F5 _ 66: 0F EB. D1
	pxor	xmm2, xmm9				; 45F9 _ 66 41: 0F EF. D1
	pxor	xmm10, xmm0				; 45FE _ 66 44: 0F EF. D0
	pxor	xmm9, xmm1				; 4603 _ 66 44: 0F EF. C9
	pxor	xmm10, xmm2				; 4608 _ 66 44: 0F EF. D2
	pxor	xmm10, xmm1				; 460D _ 66 44: 0F EF. D1
	pand	xmm9, xmm10				; 4612 _ 66 45: 0F DB. CA
	pxor	xmm0, xmm9				; 4617 _ 66 41: 0F EF. C1
	movd	xmm9, dword [r12+1380H] 		; 461C _ 66 45: 0F 6E. 8C 24, 00001380
	pshufd	xmm11, xmm9, 0				; 4626 _ 66 45: 0F 70. D9, 00
	pxor	xmm2, xmm11				; 462C _ 66 41: 0F EF. D3
	movd	xmm9, dword [r12+1384H] 		; 4631 _ 66 45: 0F 6E. 8C 24, 00001384
	pshufd	xmm11, xmm9, 0				; 463B _ 66 45: 0F 70. D9, 00
	pxor	xmm0, xmm11				; 4641 _ 66 41: 0F EF. C3
	movd	xmm9, dword [r12+1388H] 		; 4646 _ 66 45: 0F 6E. 8C 24, 00001388
	pshufd	xmm11, xmm9, 0				; 4650 _ 66 45: 0F 70. D9, 00
	movd	xmm9, dword [r12+138CH] 		; 4656 _ 66 45: 0F 6E. 8C 24, 0000138C
	pxor	xmm1, xmm11				; 4660 _ 66 41: 0F EF. CB
	pshufd	xmm11, xmm9, 0				; 4665 _ 66 45: 0F 70. D9, 00
	movdqa	xmm9, xmm2				; 466B _ 66 44: 0F 6F. CA
	pxor	xmm10, xmm11				; 4670 _ 66 45: 0F EF. D3
	movdqa	xmm11, xmm1				; 4675 _ 66 44: 0F 6F. D9
	psrld	xmm1, 22				; 467A _ 66: 0F 72. D1, 16
	pslld	xmm9, 27				; 467F _ 66 41: 0F 72. F1, 1B
	pslld	xmm11, 10				; 4685 _ 66 41: 0F 72. F3, 0A
	por	xmm11, xmm1				; 468B _ 66 44: 0F EB. D9
	movdqa	xmm1, xmm10				; 4690 _ 66 41: 0F 6F. CA
	psrld	xmm2, 5 				; 4695 _ 66: 0F 72. D2, 05
	por	xmm9, xmm2				; 469A _ 66 44: 0F EB. CA
	movdqa	xmm2, xmm0				; 469F _ 66: 0F 6F. D0
	pxor	xmm11, xmm10				; 46A3 _ 66 45: 0F EF. DA
	pxor	xmm9, xmm0				; 46A8 _ 66 44: 0F EF. C8
	pslld	xmm2, 7 				; 46AD _ 66: 0F 72. F2, 07
	pxor	xmm11, xmm2				; 46B2 _ 66 44: 0F EF. DA
	movdqa	xmm2, xmm0				; 46B7 _ 66: 0F 6F. D0
	pxor	xmm9, xmm10				; 46BB _ 66 45: 0F EF. CA
	pslld	xmm1, 25				; 46C0 _ 66: 0F 72. F1, 19
	psrld	xmm10, 7				; 46C5 _ 66 41: 0F 72. D2, 07
	por	xmm1, xmm10				; 46CB _ 66 41: 0F EB. CA
	movdqa	xmm10, xmm9				; 46D0 _ 66 45: 0F 6F. D1
	pslld	xmm2, 31				; 46D5 _ 66: 0F 72. F2, 1F
	psrld	xmm0, 1 				; 46DA _ 66: 0F 72. D0, 01
	por	xmm2, xmm0				; 46DF _ 66: 0F EB. D0
	movdqa	xmm0, xmm11				; 46E3 _ 66 41: 0F 6F. C3
	pxor	xmm1, xmm11				; 46E8 _ 66 41: 0F EF. CB
	pslld	xmm10, 3				; 46ED _ 66 41: 0F 72. F2, 03
	pxor	xmm1, xmm10				; 46F3 _ 66 41: 0F EF. CA
	movdqa	xmm10, xmm9				; 46F8 _ 66 45: 0F 6F. D1
	pxor	xmm2, xmm9				; 46FD _ 66 41: 0F EF. D1
	pxor	xmm2, xmm11				; 4702 _ 66 41: 0F EF. D3
	pslld	xmm0, 29				; 4707 _ 66: 0F 72. F0, 1D
	psrld	xmm11, 3				; 470C _ 66 41: 0F 72. D3, 03
	por	xmm0, xmm11				; 4712 _ 66 41: 0F EB. C3
	movd	xmm11, dword [r12+1370H]		; 4717 _ 66 45: 0F 6E. 9C 24, 00001370
	pslld	xmm10, 19				; 4721 _ 66 41: 0F 72. F2, 13
	psrld	xmm9, 13				; 4727 _ 66 41: 0F 72. D1, 0D
	por	xmm10, xmm9				; 472D _ 66 45: 0F EB. D1
	movdqa	xmm9, xmm0				; 4732 _ 66 44: 0F 6F. C8
	por	xmm0, xmm1				; 4737 _ 66: 0F EB. C1
	pxor	xmm9, xmm10				; 473B _ 66 45: 0F EF. CA
	pand	xmm10, xmm1				; 4740 _ 66 44: 0F DB. D1
	pxor	xmm9, xmm6				; 4745 _ 66 44: 0F EF. CE
	pxor	xmm1, xmm2				; 474A _ 66: 0F EF. CA
	por	xmm2, xmm10				; 474E _ 66 41: 0F EB. D2
	pxor	xmm10, xmm9				; 4753 _ 66 45: 0F EF. D1
	pand	xmm9, xmm0				; 4758 _ 66 44: 0F DB. C8
	pand	xmm1, xmm0				; 475D _ 66: 0F DB. C8
	pxor	xmm2, xmm9				; 4761 _ 66 41: 0F EF. D1
	pxor	xmm9, xmm10				; 4766 _ 66 45: 0F EF. CA
	por	xmm10, xmm9				; 476B _ 66 45: 0F EB. D1
	pxor	xmm0, xmm2				; 4770 _ 66: 0F EF. C2
	pxor	xmm10, xmm1				; 4774 _ 66 44: 0F EF. D1
	pxor	xmm1, xmm0				; 4779 _ 66: 0F EF. C8
	por	xmm0, xmm10				; 477D _ 66 41: 0F EB. C2
	pxor	xmm1, xmm9				; 4782 _ 66 41: 0F EF. C9
	pxor	xmm0, xmm9				; 4787 _ 66 41: 0F EF. C1
	pshufd	xmm9, xmm11, 0				; 478C _ 66 45: 0F 70. CB, 00
	pxor	xmm1, xmm9				; 4792 _ 66 41: 0F EF. C9
	movd	xmm11, dword [r12+1374H]		; 4797 _ 66 45: 0F 6E. 9C 24, 00001374
	pshufd	xmm9, xmm11, 0				; 47A1 _ 66 45: 0F 70. CB, 00
	pxor	xmm10, xmm9				; 47A7 _ 66 45: 0F EF. D1
	movd	xmm11, dword [r12+1378H]		; 47AC _ 66 45: 0F 6E. 9C 24, 00001378
	pshufd	xmm9, xmm11, 0				; 47B6 _ 66 45: 0F 70. CB, 00
	pxor	xmm2, xmm9				; 47BC _ 66 41: 0F EF. D1
	movd	xmm11, dword [r12+137CH]		; 47C1 _ 66 45: 0F 6E. 9C 24, 0000137C
	pshufd	xmm9, xmm11, 0				; 47CB _ 66 45: 0F 70. CB, 00
	movdqa	xmm11, xmm10				; 47D1 _ 66 45: 0F 6F. DA
	pxor	xmm0, xmm9				; 47D6 _ 66 41: 0F EF. C1
	movdqa	xmm9, xmm2				; 47DB _ 66 44: 0F 6F. CA
	psrld	xmm2, 22				; 47E0 _ 66: 0F 72. D2, 16
	pslld	xmm11, 7				; 47E5 _ 66 41: 0F 72. F3, 07
	pslld	xmm9, 10				; 47EB _ 66 41: 0F 72. F1, 0A
	por	xmm9, xmm2				; 47F1 _ 66 44: 0F EB. CA
	movdqa	xmm2, xmm1				; 47F6 _ 66: 0F 6F. D1
	psrld	xmm1, 5 				; 47FA _ 66: 0F 72. D1, 05
	pxor	xmm9, xmm0				; 47FF _ 66 44: 0F EF. C8
	pslld	xmm2, 27				; 4804 _ 66: 0F 72. F2, 1B
	por	xmm2, xmm1				; 4809 _ 66: 0F EB. D1
	movdqa	xmm1, xmm0				; 480D _ 66: 0F 6F. C8
	pxor	xmm9, xmm11				; 4811 _ 66 45: 0F EF. CB
	pxor	xmm2, xmm10				; 4816 _ 66 41: 0F EF. D2
	pxor	xmm2, xmm0				; 481B _ 66: 0F EF. D0
	movdqa	xmm11, xmm2				; 481F _ 66 44: 0F 6F. DA
	pslld	xmm1, 25				; 4824 _ 66: 0F 72. F1, 19
	psrld	xmm0, 7 				; 4829 _ 66: 0F 72. D0, 07
	por	xmm1, xmm0				; 482E _ 66: 0F EB. C8
	movdqa	xmm0, xmm10				; 4832 _ 66 41: 0F 6F. C2
	psrld	xmm10, 1				; 4837 _ 66 41: 0F 72. D2, 01
	pxor	xmm1, xmm9				; 483D _ 66 41: 0F EF. C9
	pslld	xmm0, 31				; 4842 _ 66: 0F 72. F0, 1F
	por	xmm0, xmm10				; 4847 _ 66 41: 0F EB. C2
	movdqa	xmm10, xmm2				; 484C _ 66 44: 0F 6F. D2
	pxor	xmm0, xmm2				; 4851 _ 66: 0F EF. C2
	pxor	xmm0, xmm9				; 4855 _ 66 41: 0F EF. C1
	pslld	xmm10, 3				; 485A _ 66 41: 0F 72. F2, 03
	pxor	xmm1, xmm10				; 4860 _ 66 41: 0F EF. CA
	movdqa	xmm10, xmm9				; 4865 _ 66 45: 0F 6F. D1
	psrld	xmm9, 3 				; 486A _ 66 41: 0F 72. D1, 03
	pslld	xmm11, 19				; 4870 _ 66 41: 0F 72. F3, 13
	pslld	xmm10, 29				; 4876 _ 66 41: 0F 72. F2, 1D
	por	xmm10, xmm9				; 487C _ 66 45: 0F EB. D1
	movdqa	xmm9, xmm10				; 4881 _ 66 45: 0F 6F. CA
	psrld	xmm2, 13				; 4886 _ 66: 0F 72. D2, 0D
	por	xmm11, xmm2				; 488B _ 66 44: 0F EB. DA
	pxor	xmm11, xmm10				; 4890 _ 66 45: 0F EF. DA
	pand	xmm9, xmm11				; 4895 _ 66 45: 0F DB. CB
	pxor	xmm10, xmm1				; 489A _ 66 44: 0F EF. D1
	pxor	xmm9, xmm6				; 489F _ 66 44: 0F EF. CE
	pxor	xmm1, xmm0				; 48A4 _ 66: 0F EF. C8
	movd	xmm2, dword [r12+1360H] 		; 48A8 _ 66 41: 0F 6E. 94 24, 00001360
	pxor	xmm9, xmm1				; 48B2 _ 66 44: 0F EF. C9
	por	xmm10, xmm11				; 48B7 _ 66 45: 0F EB. D3
	pxor	xmm11, xmm9				; 48BC _ 66 45: 0F EF. D9
	pxor	xmm1, xmm10				; 48C1 _ 66 41: 0F EF. CA
	pxor	xmm10, xmm0				; 48C6 _ 66 44: 0F EF. D0
	pand	xmm0, xmm1				; 48CB _ 66: 0F DB. C1
	pxor	xmm0, xmm11				; 48CF _ 66 41: 0F EF. C3
	pxor	xmm11, xmm1				; 48D4 _ 66 44: 0F EF. D9
	por	xmm11, xmm9				; 48D9 _ 66 45: 0F EB. D9
	pxor	xmm1, xmm0				; 48DE _ 66: 0F EF. C8
	pxor	xmm10, xmm11				; 48E2 _ 66 45: 0F EF. D3
	pshufd	xmm11, xmm2, 0				; 48E7 _ 66 44: 0F 70. DA, 00
	movd	xmm2, dword [r12+1364H] 		; 48ED _ 66 41: 0F 6E. 94 24, 00001364
	pxor	xmm0, xmm11				; 48F7 _ 66 41: 0F EF. C3
	pshufd	xmm11, xmm2, 0				; 48FC _ 66 44: 0F 70. DA, 00
	pxor	xmm9, xmm11				; 4902 _ 66 45: 0F EF. CB
	movd	xmm2, dword [r12+1368H] 		; 4907 _ 66 41: 0F 6E. 94 24, 00001368
	pshufd	xmm11, xmm2, 0				; 4911 _ 66 44: 0F 70. DA, 00
	pxor	xmm10, xmm11				; 4917 _ 66 45: 0F EF. D3
	movd	xmm2, dword [r12+136CH] 		; 491C _ 66 41: 0F 6E. 94 24, 0000136C
	pshufd	xmm11, xmm2, 0				; 4926 _ 66 44: 0F 70. DA, 00
	movdqa	xmm2, xmm10				; 492C _ 66 41: 0F 6F. D2
	pxor	xmm1, xmm11				; 4931 _ 66 41: 0F EF. CB
	movdqa	xmm11, xmm9				; 4936 _ 66 45: 0F 6F. D9
	pslld	xmm2, 10				; 493B _ 66: 0F 72. F2, 0A
	psrld	xmm10, 22				; 4940 _ 66 41: 0F 72. D2, 16
	por	xmm2, xmm10				; 4946 _ 66 41: 0F EB. D2
	movdqa	xmm10, xmm0				; 494B _ 66 44: 0F 6F. D0
	psrld	xmm0, 5 				; 4950 _ 66: 0F 72. D0, 05
	pxor	xmm2, xmm1				; 4955 _ 66: 0F EF. D1
	pslld	xmm10, 27				; 4959 _ 66 41: 0F 72. F2, 1B
	por	xmm10, xmm0				; 495F _ 66 44: 0F EB. D0
	movdqa	xmm0, xmm1				; 4964 _ 66: 0F 6F. C1
	pslld	xmm11, 7				; 4968 _ 66 41: 0F 72. F3, 07
	pxor	xmm2, xmm11				; 496E _ 66 41: 0F EF. D3
	movdqa	xmm11, xmm9				; 4973 _ 66 45: 0F 6F. D9
	pxor	xmm10, xmm9				; 4978 _ 66 45: 0F EF. D1
	pxor	xmm10, xmm1				; 497D _ 66 44: 0F EF. D1
	pslld	xmm0, 25				; 4982 _ 66: 0F 72. F0, 19
	psrld	xmm1, 7 				; 4987 _ 66: 0F 72. D1, 07
	por	xmm0, xmm1				; 498C _ 66: 0F EB. C1
	movdqa	xmm1, xmm2				; 4990 _ 66: 0F 6F. CA
	pslld	xmm11, 31				; 4994 _ 66 41: 0F 72. F3, 1F
	psrld	xmm9, 1 				; 499A _ 66 41: 0F 72. D1, 01
	por	xmm11, xmm9				; 49A0 _ 66 45: 0F EB. D9
	movdqa	xmm9, xmm10				; 49A5 _ 66 45: 0F 6F. CA
	pxor	xmm0, xmm2				; 49AA _ 66: 0F EF. C2
	pxor	xmm11, xmm10				; 49AE _ 66 45: 0F EF. DA
	pslld	xmm9, 3 				; 49B3 _ 66 41: 0F 72. F1, 03
	pxor	xmm0, xmm9				; 49B9 _ 66 41: 0F EF. C1
	movdqa	xmm9, xmm0				; 49BE _ 66 44: 0F 6F. C8
	pxor	xmm11, xmm2				; 49C3 _ 66 44: 0F EF. DA
	pslld	xmm1, 29				; 49C8 _ 66: 0F 72. F1, 1D
	psrld	xmm2, 3 				; 49CD _ 66: 0F 72. D2, 03
	por	xmm1, xmm2				; 49D2 _ 66: 0F EB. CA
	movdqa	xmm2, xmm10				; 49D6 _ 66 41: 0F 6F. D2
	psrld	xmm10, 13				; 49DB _ 66 41: 0F 72. D2, 0D
	pxor	xmm11, xmm6				; 49E1 _ 66 44: 0F EF. DE
	pslld	xmm2, 19				; 49E6 _ 66: 0F 72. F2, 13
	por	xmm2, xmm10				; 49EB _ 66 41: 0F EB. D2
	pxor	xmm1, xmm11				; 49F0 _ 66 41: 0F EF. CB
	por	xmm9, xmm2				; 49F5 _ 66 44: 0F EB. CA
	pxor	xmm9, xmm1				; 49FA _ 66 44: 0F EF. C9
	movd	xmm10, dword [r12+1350H]		; 49FF _ 66 45: 0F 6E. 94 24, 00001350
	por	xmm1, xmm11				; 4A09 _ 66 41: 0F EB. CB
	pand	xmm1, xmm2				; 4A0E _ 66: 0F DB. CA
	pxor	xmm0, xmm9				; 4A12 _ 66 41: 0F EF. C1
	pxor	xmm1, xmm0				; 4A17 _ 66: 0F EF. C8
	por	xmm0, xmm2				; 4A1B _ 66: 0F EB. C2
	pxor	xmm0, xmm11				; 4A1F _ 66 41: 0F EF. C3
	pand	xmm11, xmm1				; 4A24 _ 66 44: 0F DB. D9
	pxor	xmm11, xmm9				; 4A29 _ 66 45: 0F EF. D9
	pxor	xmm0, xmm1				; 4A2E _ 66: 0F EF. C1
	pand	xmm9, xmm0				; 4A32 _ 66 44: 0F DB. C8
	pxor	xmm0, xmm11				; 4A37 _ 66 41: 0F EF. C3
	pxor	xmm9, xmm0				; 4A3C _ 66 44: 0F EF. C8
	pxor	xmm0, xmm6				; 4A41 _ 66: 0F EF. C6
	pxor	xmm9, xmm2				; 4A45 _ 66 44: 0F EF. CA
	pshufd	xmm2, xmm10, 0				; 4A4A _ 66 41: 0F 70. D2, 00
	pxor	xmm11, xmm2				; 4A50 _ 66 44: 0F EF. DA
	movd	xmm10, dword [r12+1354H]		; 4A55 _ 66 45: 0F 6E. 94 24, 00001354
	pshufd	xmm2, xmm10, 0				; 4A5F _ 66 41: 0F 70. D2, 00
	pxor	xmm0, xmm2				; 4A65 _ 66: 0F EF. C2
	movd	xmm2, dword [r12+1358H] 		; 4A69 _ 66 41: 0F 6E. 94 24, 00001358
	pshufd	xmm10, xmm2, 0				; 4A73 _ 66 44: 0F 70. D2, 00
	pxor	xmm9, xmm10				; 4A79 _ 66 45: 0F EF. CA
	movd	xmm10, dword [r12+135CH]		; 4A7E _ 66 45: 0F 6E. 94 24, 0000135C
	pshufd	xmm2, xmm10, 0				; 4A88 _ 66 41: 0F 70. D2, 00
	pxor	xmm1, xmm2				; 4A8E _ 66: 0F EF. CA
	movdqa	xmm2, xmm9				; 4A92 _ 66 41: 0F 6F. D1
	psrld	xmm9, 22				; 4A97 _ 66 41: 0F 72. D1, 16
	pslld	xmm2, 10				; 4A9D _ 66: 0F 72. F2, 0A
	por	xmm2, xmm9				; 4AA2 _ 66 41: 0F EB. D1
	movdqa	xmm9, xmm11				; 4AA7 _ 66 45: 0F 6F. CB
	psrld	xmm11, 5				; 4AAC _ 66 41: 0F 72. D3, 05
	pxor	xmm2, xmm1				; 4AB2 _ 66: 0F EF. D1
	pslld	xmm9, 27				; 4AB6 _ 66 41: 0F 72. F1, 1B
	por	xmm9, xmm11				; 4ABC _ 66 45: 0F EB. CB
	movdqa	xmm11, xmm0				; 4AC1 _ 66 44: 0F 6F. D8
	pxor	xmm9, xmm0				; 4AC6 _ 66 44: 0F EF. C8
	pxor	xmm9, xmm1				; 4ACB _ 66 44: 0F EF. C9
	movdqa	xmm10, xmm9				; 4AD0 _ 66 45: 0F 6F. D1
	pslld	xmm11, 7				; 4AD5 _ 66 41: 0F 72. F3, 07
	pxor	xmm2, xmm11				; 4ADB _ 66 41: 0F EF. D3
	movdqa	xmm11, xmm1				; 4AE0 _ 66 44: 0F 6F. D9
	psrld	xmm1, 7 				; 4AE5 _ 66: 0F 72. D1, 07
	pslld	xmm10, 3				; 4AEA _ 66 41: 0F 72. F2, 03
	pslld	xmm11, 25				; 4AF0 _ 66 41: 0F 72. F3, 19
	por	xmm11, xmm1				; 4AF6 _ 66 44: 0F EB. D9
	movdqa	xmm1, xmm0				; 4AFB _ 66: 0F 6F. C8
	psrld	xmm0, 1 				; 4AFF _ 66: 0F 72. D0, 01
	pxor	xmm11, xmm2				; 4B04 _ 66 44: 0F EF. DA
	pslld	xmm1, 31				; 4B09 _ 66: 0F 72. F1, 1F
	por	xmm1, xmm0				; 4B0E _ 66: 0F EB. C8
	movdqa	xmm0, xmm9				; 4B12 _ 66 41: 0F 6F. C1
	pxor	xmm11, xmm10				; 4B17 _ 66 45: 0F EF. DA
	movdqa	xmm10, xmm2				; 4B1C _ 66 44: 0F 6F. D2
	pxor	xmm1, xmm9				; 4B21 _ 66 41: 0F EF. C9
	pxor	xmm1, xmm2				; 4B26 _ 66: 0F EF. CA
	pslld	xmm10, 29				; 4B2A _ 66 41: 0F 72. F2, 1D
	psrld	xmm2, 3 				; 4B30 _ 66: 0F 72. D2, 03
	por	xmm10, xmm2				; 4B35 _ 66 44: 0F EB. D2
	movdqa	xmm2, xmm10				; 4B3A _ 66 41: 0F 6F. D2
	pslld	xmm0, 19				; 4B3F _ 66: 0F 72. F0, 13
	psrld	xmm9, 13				; 4B44 _ 66 41: 0F 72. D1, 0D
	por	xmm0, xmm9				; 4B4A _ 66 41: 0F EB. C1
	movd	xmm9, dword [r12+1340H] 		; 4B4F _ 66 45: 0F 6E. 8C 24, 00001340
	pand	xmm2, xmm11				; 4B59 _ 66 41: 0F DB. D3
	pxor	xmm2, xmm1				; 4B5E _ 66: 0F EF. D1
	por	xmm1, xmm11				; 4B62 _ 66 41: 0F EB. CB
	pand	xmm1, xmm0				; 4B67 _ 66: 0F DB. C8
	pxor	xmm10, xmm2				; 4B6B _ 66 44: 0F EF. D2
	pxor	xmm10, xmm1				; 4B70 _ 66 44: 0F EF. D1
	pand	xmm1, xmm2				; 4B75 _ 66: 0F DB. CA
	pxor	xmm0, xmm6				; 4B79 _ 66: 0F EF. C6
	pxor	xmm11, xmm10				; 4B7D _ 66 45: 0F EF. DA
	pxor	xmm1, xmm11				; 4B82 _ 66 41: 0F EF. CB
	pand	xmm11, xmm0				; 4B87 _ 66 44: 0F DB. D8
	pxor	xmm11, xmm2				; 4B8C _ 66 44: 0F EF. DA
	pxor	xmm0, xmm1				; 4B91 _ 66: 0F EF. C1
	pand	xmm2, xmm0				; 4B95 _ 66: 0F DB. D0
	pxor	xmm11, xmm0				; 4B99 _ 66 44: 0F EF. D8
	pxor	xmm2, xmm10				; 4B9E _ 66 41: 0F EF. D2
	por	xmm2, xmm11				; 4BA3 _ 66 41: 0F EB. D3
	pxor	xmm11, xmm0				; 4BA8 _ 66 44: 0F EF. D8
	pxor	xmm2, xmm1				; 4BAD _ 66: 0F EF. D1
	pshufd	xmm9, xmm9, 0				; 4BB1 _ 66 45: 0F 70. C9, 00
	pxor	xmm0, xmm9				; 4BB7 _ 66 41: 0F EF. C1
	movd	xmm9, dword [r12+1344H] 		; 4BBC _ 66 45: 0F 6E. 8C 24, 00001344
	pshufd	xmm9, xmm9, 0				; 4BC6 _ 66 45: 0F 70. C9, 00
	pxor	xmm11, xmm9				; 4BCC _ 66 45: 0F EF. D9
	movd	xmm9, dword [r12+1348H] 		; 4BD1 _ 66 45: 0F 6E. 8C 24, 00001348
	pshufd	xmm9, xmm9, 0				; 4BDB _ 66 45: 0F 70. C9, 00
	pxor	xmm2, xmm9				; 4BE1 _ 66 41: 0F EF. D1
	movdqa	xmm1, xmm2				; 4BE6 _ 66: 0F 6F. CA
	psrld	xmm2, 22				; 4BEA _ 66: 0F 72. D2, 16
	pslld	xmm1, 10				; 4BEF _ 66: 0F 72. F1, 0A
	por	xmm1, xmm2				; 4BF4 _ 66: 0F EB. CA
	movdqa	xmm2, xmm0				; 4BF8 _ 66: 0F 6F. D0
	movd	xmm9, dword [r12+134CH] 		; 4BFC _ 66 45: 0F 6E. 8C 24, 0000134C
	pshufd	xmm9, xmm9, 0				; 4C06 _ 66 45: 0F 70. C9, 00
	pxor	xmm10, xmm9				; 4C0C _ 66 45: 0F EF. D1
	movdqa	xmm9, xmm11				; 4C11 _ 66 45: 0F 6F. CB
	pslld	xmm2, 27				; 4C16 _ 66: 0F 72. F2, 1B
	psrld	xmm0, 5 				; 4C1B _ 66: 0F 72. D0, 05
	por	xmm2, xmm0				; 4C20 _ 66: 0F EB. D0
	movdqa	xmm0, xmm11				; 4C24 _ 66 41: 0F 6F. C3
	pxor	xmm1, xmm10				; 4C29 _ 66 41: 0F EF. CA
	pslld	xmm9, 7 				; 4C2E _ 66 41: 0F 72. F1, 07
	pxor	xmm1, xmm9				; 4C34 _ 66 41: 0F EF. C9
	movdqa	xmm9, xmm10				; 4C39 _ 66 45: 0F 6F. CA
	pxor	xmm2, xmm11				; 4C3E _ 66 41: 0F EF. D3
	pxor	xmm2, xmm10				; 4C43 _ 66 41: 0F EF. D2
	pslld	xmm9, 25				; 4C48 _ 66 41: 0F 72. F1, 19
	psrld	xmm10, 7				; 4C4E _ 66 41: 0F 72. D2, 07
	por	xmm9, xmm10				; 4C54 _ 66 45: 0F EB. CA
	movdqa	xmm10, xmm1				; 4C59 _ 66 44: 0F 6F. D1
	pslld	xmm0, 31				; 4C5E _ 66: 0F 72. F0, 1F
	psrld	xmm11, 1				; 4C63 _ 66 41: 0F 72. D3, 01
	por	xmm0, xmm11				; 4C69 _ 66 41: 0F EB. C3
	movdqa	xmm11, xmm2				; 4C6E _ 66 44: 0F 6F. DA
	pxor	xmm9, xmm1				; 4C73 _ 66 44: 0F EF. C9
	pxor	xmm0, xmm2				; 4C78 _ 66: 0F EF. C2
	pslld	xmm11, 3				; 4C7C _ 66 41: 0F 72. F3, 03
	pxor	xmm9, xmm11				; 4C82 _ 66 45: 0F EF. CB
	movdqa	xmm11, xmm2				; 4C87 _ 66 44: 0F 6F. DA
	pxor	xmm0, xmm1				; 4C8C _ 66: 0F EF. C1
	pslld	xmm10, 29				; 4C90 _ 66 41: 0F 72. F2, 1D
	psrld	xmm1, 3 				; 4C96 _ 66: 0F 72. D1, 03
	por	xmm10, xmm1				; 4C9B _ 66 44: 0F EB. D1
	movdqa	xmm1, xmm10				; 4CA0 _ 66 41: 0F 6F. CA
	pslld	xmm11, 19				; 4CA5 _ 66 41: 0F 72. F3, 13
	psrld	xmm2, 13				; 4CAB _ 66: 0F 72. D2, 0D
	por	xmm11, xmm2				; 4CB0 _ 66 44: 0F EB. DA
	pxor	xmm1, xmm0				; 4CB5 _ 66: 0F EF. C8
	pxor	xmm11, xmm1				; 4CB9 _ 66 44: 0F EF. D9
	movd	xmm2, dword [r12+1330H] 		; 4CBE _ 66 41: 0F 6E. 94 24, 00001330
	pand	xmm10, xmm1				; 4CC8 _ 66 44: 0F DB. D1
	pxor	xmm10, xmm11				; 4CCD _ 66 45: 0F EF. D3
	pand	xmm11, xmm0				; 4CD2 _ 66 44: 0F DB. D8
	pxor	xmm0, xmm9				; 4CD7 _ 66 41: 0F EF. C1
	por	xmm9, xmm10				; 4CDC _ 66 45: 0F EB. CA
	pxor	xmm1, xmm9				; 4CE1 _ 66 41: 0F EF. C9
	pxor	xmm11, xmm9				; 4CE6 _ 66 45: 0F EF. D9
	pxor	xmm0, xmm10				; 4CEB _ 66 41: 0F EF. C2
	pand	xmm9, xmm1				; 4CF0 _ 66 44: 0F DB. C9
	pxor	xmm9, xmm0				; 4CF5 _ 66 44: 0F EF. C8
	pxor	xmm0, xmm11				; 4CFA _ 66 41: 0F EF. C3
	por	xmm0, xmm1				; 4CFF _ 66: 0F EB. C1
	pxor	xmm11, xmm9				; 4D03 _ 66 45: 0F EF. D9
	pxor	xmm0, xmm10				; 4D08 _ 66 41: 0F EF. C2
	pxor	xmm11, xmm0				; 4D0D _ 66 44: 0F EF. D8
	pshufd	xmm10, xmm2, 0				; 4D12 _ 66 44: 0F 70. D2, 00
	pxor	xmm1, xmm10				; 4D18 _ 66 41: 0F EF. CA
	movd	xmm2, dword [r12+1334H] 		; 4D1D _ 66 41: 0F 6E. 94 24, 00001334
	pshufd	xmm10, xmm2, 0				; 4D27 _ 66 44: 0F 70. D2, 00
	movd	xmm2, dword [r12+1338H] 		; 4D2D _ 66 41: 0F 6E. 94 24, 00001338
	pxor	xmm0, xmm10				; 4D37 _ 66 41: 0F EF. C2
	pshufd	xmm10, xmm2, 0				; 4D3C _ 66 44: 0F 70. D2, 00
	pxor	xmm9, xmm10				; 4D42 _ 66 45: 0F EF. CA
	movd	xmm2, dword [r12+133CH] 		; 4D47 _ 66 41: 0F 6E. 94 24, 0000133C
	pshufd	xmm10, xmm2, 0				; 4D51 _ 66 44: 0F 70. D2, 00
	movdqa	xmm2, xmm9				; 4D57 _ 66 41: 0F 6F. D1
	pxor	xmm11, xmm10				; 4D5C _ 66 45: 0F EF. DA
	movdqa	xmm10, xmm1				; 4D61 _ 66 44: 0F 6F. D1
	pslld	xmm2, 10				; 4D66 _ 66: 0F 72. F2, 0A
	psrld	xmm9, 22				; 4D6B _ 66 41: 0F 72. D1, 16
	por	xmm2, xmm9				; 4D71 _ 66 41: 0F EB. D1
	movdqa	xmm9, xmm0				; 4D76 _ 66 44: 0F 6F. C8
	pslld	xmm10, 27				; 4D7B _ 66 41: 0F 72. F2, 1B
	psrld	xmm1, 5 				; 4D81 _ 66: 0F 72. D1, 05
	por	xmm10, xmm1				; 4D86 _ 66 44: 0F EB. D1
	pxor	xmm2, xmm11				; 4D8B _ 66 41: 0F EF. D3
	pslld	xmm9, 7 				; 4D90 _ 66 41: 0F 72. F1, 07
	pxor	xmm2, xmm9				; 4D96 _ 66 41: 0F EF. D1
	movdqa	xmm9, xmm11				; 4D9B _ 66 45: 0F 6F. CB
	pxor	xmm10, xmm0				; 4DA0 _ 66 44: 0F EF. D0
	pxor	xmm10, xmm11				; 4DA5 _ 66 45: 0F EF. D3
	movdqa	xmm1, xmm10				; 4DAA _ 66 41: 0F 6F. CA
	pslld	xmm9, 25				; 4DAF _ 66 41: 0F 72. F1, 19
	psrld	xmm11, 7				; 4DB5 _ 66 41: 0F 72. D3, 07
	por	xmm9, xmm11				; 4DBB _ 66 45: 0F EB. CB
	movdqa	xmm11, xmm0				; 4DC0 _ 66 44: 0F 6F. D8
	psrld	xmm0, 1 				; 4DC5 _ 66: 0F 72. D0, 01
	pxor	xmm9, xmm2				; 4DCA _ 66 44: 0F EF. CA
	pslld	xmm11, 31				; 4DCF _ 66 41: 0F 72. F3, 1F
	por	xmm11, xmm0				; 4DD5 _ 66 44: 0F EB. D8
	movdqa	xmm0, xmm10				; 4DDA _ 66 41: 0F 6F. C2
	pxor	xmm11, xmm10				; 4DDF _ 66 45: 0F EF. DA
	pxor	xmm11, xmm2				; 4DE4 _ 66 44: 0F EF. DA
	pslld	xmm0, 3 				; 4DE9 _ 66: 0F 72. F0, 03
	pxor	xmm9, xmm0				; 4DEE _ 66 44: 0F EF. C8
	movdqa	xmm0, xmm2				; 4DF3 _ 66: 0F 6F. C2
	psrld	xmm2, 3 				; 4DF7 _ 66: 0F 72. D2, 03
	pslld	xmm1, 19				; 4DFC _ 66: 0F 72. F1, 13
	pslld	xmm0, 29				; 4E01 _ 66: 0F 72. F0, 1D
	por	xmm0, xmm2				; 4E06 _ 66: 0F EB. C2
	psrld	xmm10, 13				; 4E0A _ 66 41: 0F 72. D2, 0D
	por	xmm1, xmm10				; 4E10 _ 66 41: 0F EB. CA
	pxor	xmm0, xmm9				; 4E15 _ 66 41: 0F EF. C1
	pxor	xmm9, xmm1				; 4E1A _ 66 44: 0F EF. C9
	movdqa	xmm2, xmm9				; 4E1F _ 66 41: 0F 6F. D1
	pand	xmm2, xmm0				; 4E24 _ 66: 0F DB. D0
	pxor	xmm2, xmm11				; 4E28 _ 66 41: 0F EF. D3
	por	xmm11, xmm0				; 4E2D _ 66 44: 0F EB. D8
	pxor	xmm11, xmm9				; 4E32 _ 66 45: 0F EF. D9
	pand	xmm9, xmm2				; 4E37 _ 66 44: 0F DB. CA
	pxor	xmm0, xmm2				; 4E3C _ 66: 0F EF. C2
	pand	xmm9, xmm1				; 4E40 _ 66 44: 0F DB. C9
	pxor	xmm9, xmm0				; 4E45 _ 66 44: 0F EF. C8
	pand	xmm0, xmm11				; 4E4A _ 66 41: 0F DB. C3
	por	xmm0, xmm1				; 4E4F _ 66: 0F EB. C1
	pxor	xmm2, xmm6				; 4E53 _ 66: 0F EF. D6
	movdqa	xmm10, xmm2				; 4E57 _ 66 44: 0F 6F. D2
	pxor	xmm0, xmm2				; 4E5C _ 66: 0F EF. C2
	pxor	xmm1, xmm2				; 4E60 _ 66: 0F EF. CA
	pxor	xmm10, xmm9				; 4E64 _ 66 45: 0F EF. D1
	pand	xmm1, xmm11				; 4E69 _ 66 41: 0F DB. CB
	movd	xmm2, dword [r12+1320H] 		; 4E6E _ 66 41: 0F 6E. 94 24, 00001320
	pxor	xmm10, xmm1				; 4E78 _ 66 44: 0F EF. D1
	pshufd	xmm2, xmm2, 0				; 4E7D _ 66: 0F 70. D2, 00
	pxor	xmm11, xmm2				; 4E82 _ 66 44: 0F EF. DA
	movd	xmm2, dword [r12+1324H] 		; 4E87 _ 66 41: 0F 6E. 94 24, 00001324
	pshufd	xmm2, xmm2, 0				; 4E91 _ 66: 0F 70. D2, 00
	pxor	xmm9, xmm2				; 4E96 _ 66 44: 0F EF. CA
	movd	xmm2, dword [r12+1328H] 		; 4E9B _ 66 41: 0F 6E. 94 24, 00001328
	pshufd	xmm2, xmm2, 0				; 4EA5 _ 66: 0F 70. D2, 00
	pxor	xmm0, xmm2				; 4EAA _ 66: 0F EF. C2
	movdqa	xmm1, xmm0				; 4EAE _ 66: 0F 6F. C8
	psrld	xmm0, 22				; 4EB2 _ 66: 0F 72. D0, 16
	pslld	xmm1, 10				; 4EB7 _ 66: 0F 72. F1, 0A
	por	xmm1, xmm0				; 4EBC _ 66: 0F EB. C8
	movd	xmm2, dword [r12+132CH] 		; 4EC0 _ 66 41: 0F 6E. 94 24, 0000132C
	pshufd	xmm2, xmm2, 0				; 4ECA _ 66: 0F 70. D2, 00
	pxor	xmm10, xmm2				; 4ECF _ 66 44: 0F EF. D2
	movdqa	xmm2, xmm11				; 4ED4 _ 66 41: 0F 6F. D3
	psrld	xmm11, 5				; 4ED9 _ 66 41: 0F 72. D3, 05
	pxor	xmm1, xmm10				; 4EDF _ 66 41: 0F EF. CA
	pslld	xmm2, 27				; 4EE4 _ 66: 0F 72. F2, 1B
	por	xmm2, xmm11				; 4EE9 _ 66 41: 0F EB. D3
	movdqa	xmm11, xmm9				; 4EEE _ 66 45: 0F 6F. D9
	pxor	xmm2, xmm9				; 4EF3 _ 66 41: 0F EF. D1
	pxor	xmm2, xmm10				; 4EF8 _ 66 41: 0F EF. D2
	pslld	xmm11, 7				; 4EFD _ 66 41: 0F 72. F3, 07
	pxor	xmm1, xmm11				; 4F03 _ 66 41: 0F EF. CB
	movdqa	xmm11, xmm10				; 4F08 _ 66 45: 0F 6F. DA
	movdqa	xmm0, xmm1				; 4F0D _ 66: 0F 6F. C1
	psrld	xmm10, 7				; 4F11 _ 66 41: 0F 72. D2, 07
	pslld	xmm11, 25				; 4F17 _ 66 41: 0F 72. F3, 19
	por	xmm11, xmm10				; 4F1D _ 66 45: 0F EB. DA
	movdqa	xmm10, xmm9				; 4F22 _ 66 45: 0F 6F. D1
	psrld	xmm9, 1 				; 4F27 _ 66 41: 0F 72. D1, 01
	pxor	xmm11, xmm1				; 4F2D _ 66 44: 0F EF. D9
	pslld	xmm10, 31				; 4F32 _ 66 41: 0F 72. F2, 1F
	por	xmm10, xmm9				; 4F38 _ 66 45: 0F EB. D1
	movdqa	xmm9, xmm2				; 4F3D _ 66 44: 0F 6F. CA
	pxor	xmm10, xmm2				; 4F42 _ 66 44: 0F EF. D2
	pxor	xmm10, xmm1				; 4F47 _ 66 44: 0F EF. D1
	pslld	xmm9, 3 				; 4F4C _ 66 41: 0F 72. F1, 03
	pxor	xmm11, xmm9				; 4F52 _ 66 45: 0F EF. D9
	movdqa	xmm9, xmm2				; 4F57 _ 66 44: 0F 6F. CA
	pslld	xmm0, 29				; 4F5C _ 66: 0F 72. F0, 1D
	psrld	xmm1, 3 				; 4F61 _ 66: 0F 72. D1, 03
	por	xmm0, xmm1				; 4F66 _ 66: 0F EB. C1
	pslld	xmm9, 19				; 4F6A _ 66 41: 0F 72. F1, 13
	psrld	xmm2, 13				; 4F70 _ 66: 0F 72. D2, 0D
	por	xmm9, xmm2				; 4F75 _ 66 44: 0F EB. CA
	movdqa	xmm2, xmm10				; 4F7A _ 66 41: 0F 6F. D2
	pxor	xmm10, xmm0				; 4F7F _ 66 44: 0F EF. D0
	pxor	xmm2, xmm11				; 4F84 _ 66 41: 0F EF. D3
	pand	xmm11, xmm2				; 4F89 _ 66 44: 0F DB. DA
	pxor	xmm11, xmm9				; 4F8E _ 66 45: 0F EF. D9
	por	xmm9, xmm2				; 4F93 _ 66 44: 0F EB. CA
	pxor	xmm0, xmm11				; 4F98 _ 66 41: 0F EF. C3
	pxor	xmm9, xmm10				; 4F9D _ 66 45: 0F EF. CA
	por	xmm9, xmm0				; 4FA2 _ 66 44: 0F EB. C8
	pxor	xmm2, xmm11				; 4FA7 _ 66 41: 0F EF. D3
	pxor	xmm9, xmm2				; 4FAC _ 66 44: 0F EF. CA
	por	xmm2, xmm11				; 4FB1 _ 66 41: 0F EB. D3
	pxor	xmm2, xmm9				; 4FB6 _ 66 41: 0F EF. D1
	pxor	xmm10, xmm6				; 4FBB _ 66 44: 0F EF. D6
	pxor	xmm10, xmm2				; 4FC0 _ 66 44: 0F EF. D2
	por	xmm2, xmm9				; 4FC5 _ 66 41: 0F EB. D1
	pxor	xmm2, xmm9				; 4FCA _ 66 41: 0F EF. D1
	por	xmm2, xmm10				; 4FCF _ 66 41: 0F EB. D2
	pxor	xmm11, xmm2				; 4FD4 _ 66 44: 0F EF. DA
	movd	xmm2, dword [r12+1310H] 		; 4FD9 _ 66 41: 0F 6E. 94 24, 00001310
	pshufd	xmm2, xmm2, 0				; 4FE3 _ 66: 0F 70. D2, 00
	pxor	xmm10, xmm2				; 4FE8 _ 66 44: 0F EF. D2
	movd	xmm2, dword [r12+1314H] 		; 4FED _ 66 41: 0F 6E. 94 24, 00001314
	pshufd	xmm2, xmm2, 0				; 4FF7 _ 66: 0F 70. D2, 00
	pxor	xmm9, xmm2				; 4FFC _ 66 44: 0F EF. CA
	movd	xmm2, dword [r12+1318H] 		; 5001 _ 66 41: 0F 6E. 94 24, 00001318
	pshufd	xmm2, xmm2, 0				; 500B _ 66: 0F 70. D2, 00
	pxor	xmm11, xmm2				; 5010 _ 66 44: 0F EF. DA
	movd	xmm2, dword [r12+131CH] 		; 5015 _ 66 41: 0F 6E. 94 24, 0000131C
	pshufd	xmm2, xmm2, 0				; 501F _ 66: 0F 70. D2, 00
	pxor	xmm0, xmm2				; 5024 _ 66: 0F EF. C2
	movdqa	xmm2, xmm11				; 5028 _ 66 41: 0F 6F. D3
	movdqa	xmm1, xmm0				; 502D _ 66: 0F 6F. C8
	psrld	xmm11, 22				; 5031 _ 66 41: 0F 72. D3, 16
	pslld	xmm2, 10				; 5037 _ 66: 0F 72. F2, 0A
	por	xmm2, xmm11				; 503C _ 66 41: 0F EB. D3
	movdqa	xmm11, xmm10				; 5041 _ 66 45: 0F 6F. DA
	psrld	xmm10, 5				; 5046 _ 66 41: 0F 72. D2, 05
	pxor	xmm2, xmm0				; 504C _ 66: 0F EF. D0
	pslld	xmm11, 27				; 5050 _ 66 41: 0F 72. F3, 1B
	por	xmm11, xmm10				; 5056 _ 66 45: 0F EB. DA
	movdqa	xmm10, xmm9				; 505B _ 66 45: 0F 6F. D1
	pxor	xmm11, xmm9				; 5060 _ 66 45: 0F EF. D9
	pxor	xmm11, xmm0				; 5065 _ 66 44: 0F EF. D8
	pslld	xmm10, 7				; 506A _ 66 41: 0F 72. F2, 07
	pxor	xmm2, xmm10				; 5070 _ 66 41: 0F EF. D2
	movdqa	xmm10, xmm9				; 5075 _ 66 45: 0F 6F. D1
	pslld	xmm1, 25				; 507A _ 66: 0F 72. F1, 19
	psrld	xmm0, 7 				; 507F _ 66: 0F 72. D0, 07
	por	xmm1, xmm0				; 5084 _ 66: 0F EB. C8
	movdqa	xmm0, xmm11				; 5088 _ 66 41: 0F 6F. C3
	pslld	xmm10, 31				; 508D _ 66 41: 0F 72. F2, 1F
	psrld	xmm9, 1 				; 5093 _ 66 41: 0F 72. D1, 01
	por	xmm10, xmm9				; 5099 _ 66 45: 0F EB. D1
	movdqa	xmm9, xmm11				; 509E _ 66 45: 0F 6F. CB
	pxor	xmm1, xmm2				; 50A3 _ 66: 0F EF. CA
	pxor	xmm10, xmm11				; 50A7 _ 66 45: 0F EF. D3
	pslld	xmm9, 3 				; 50AC _ 66 41: 0F 72. F1, 03
	pxor	xmm1, xmm9				; 50B2 _ 66 41: 0F EF. C9
	movdqa	xmm9, xmm2				; 50B7 _ 66 44: 0F 6F. CA
	pxor	xmm10, xmm2				; 50BC _ 66 44: 0F EF. D2
	psrld	xmm2, 3 				; 50C1 _ 66: 0F 72. D2, 03
	pslld	xmm9, 29				; 50C6 _ 66 41: 0F 72. F1, 1D
	por	xmm9, xmm2				; 50CC _ 66 44: 0F EB. CA
	pslld	xmm0, 19				; 50D1 _ 66: 0F 72. F0, 13
	psrld	xmm11, 13				; 50D6 _ 66 41: 0F 72. D3, 0D
	por	xmm0, xmm11				; 50DC _ 66 41: 0F EB. C3
	movdqa	xmm11, xmm10				; 50E1 _ 66 45: 0F 6F. DA
	pxor	xmm9, xmm6				; 50E6 _ 66 44: 0F EF. CE
	pxor	xmm10, xmm6				; 50EB _ 66 44: 0F EF. D6
	por	xmm11, xmm0				; 50F0 _ 66 44: 0F EB. D8
	pxor	xmm11, xmm9				; 50F5 _ 66 45: 0F EF. D9
	por	xmm9, xmm10				; 50FA _ 66 45: 0F EB. CA
	pxor	xmm11, xmm1				; 50FF _ 66 44: 0F EF. D9
	pxor	xmm0, xmm10				; 5104 _ 66 41: 0F EF. C2
	pxor	xmm9, xmm0				; 5109 _ 66 44: 0F EF. C8
	pand	xmm0, xmm1				; 510E _ 66: 0F DB. C1
	pxor	xmm10, xmm0				; 5112 _ 66 44: 0F EF. D0
	por	xmm0, xmm11				; 5117 _ 66 41: 0F EB. C3
	pxor	xmm0, xmm9				; 511C _ 66 41: 0F EF. C1
	pxor	xmm1, xmm10				; 5121 _ 66 41: 0F EF. CA
	pxor	xmm9, xmm11				; 5126 _ 66 45: 0F EF. CB
	pxor	xmm1, xmm0				; 512B _ 66: 0F EF. C8
	pxor	xmm1, xmm11				; 512F _ 66 41: 0F EF. CB
	pand	xmm9, xmm1				; 5134 _ 66 44: 0F DB. C9
	pxor	xmm10, xmm9				; 5139 _ 66 45: 0F EF. D1
	movd	xmm9, dword [r12+1300H] 		; 513E _ 66 45: 0F 6E. 8C 24, 00001300
	pshufd	xmm2, xmm9, 0				; 5148 _ 66 41: 0F 70. D1, 00
	pxor	xmm0, xmm2				; 514E _ 66: 0F EF. C2
	movd	xmm9, dword [r12+1304H] 		; 5152 _ 66 45: 0F 6E. 8C 24, 00001304
	pshufd	xmm2, xmm9, 0				; 515C _ 66 41: 0F 70. D1, 00
	pxor	xmm10, xmm2				; 5162 _ 66 44: 0F EF. D2
	movd	xmm9, dword [r12+1308H] 		; 5167 _ 66 45: 0F 6E. 8C 24, 00001308
	pshufd	xmm2, xmm9, 0				; 5171 _ 66 41: 0F 70. D1, 00
	movd	xmm9, dword [r12+130CH] 		; 5177 _ 66 45: 0F 6E. 8C 24, 0000130C
	pxor	xmm11, xmm2				; 5181 _ 66 44: 0F EF. DA
	pshufd	xmm2, xmm9, 0				; 5186 _ 66 41: 0F 70. D1, 00
	movdqa	xmm9, xmm0				; 518C _ 66 44: 0F 6F. C8
	pxor	xmm1, xmm2				; 5191 _ 66: 0F EF. CA
	movdqa	xmm2, xmm11				; 5195 _ 66 41: 0F 6F. D3
	psrld	xmm11, 22				; 519A _ 66 41: 0F 72. D3, 16
	pslld	xmm9, 27				; 51A0 _ 66 41: 0F 72. F1, 1B
	pslld	xmm2, 10				; 51A6 _ 66: 0F 72. F2, 0A
	por	xmm2, xmm11				; 51AB _ 66 41: 0F EB. D3
	movdqa	xmm11, xmm10				; 51B0 _ 66 45: 0F 6F. DA
	psrld	xmm0, 5 				; 51B5 _ 66: 0F 72. D0, 05
	por	xmm9, xmm0				; 51BA _ 66 44: 0F EB. C8
	movdqa	xmm0, xmm10				; 51BF _ 66 41: 0F 6F. C2
	pxor	xmm2, xmm1				; 51C4 _ 66: 0F EF. D1
	pslld	xmm11, 7				; 51C8 _ 66 41: 0F 72. F3, 07
	pxor	xmm2, xmm11				; 51CE _ 66 41: 0F EF. D3
	movdqa	xmm11, xmm1				; 51D3 _ 66 44: 0F 6F. D9
	pxor	xmm9, xmm10				; 51D8 _ 66 45: 0F EF. CA
	pxor	xmm9, xmm1				; 51DD _ 66 44: 0F EF. C9
	pslld	xmm11, 25				; 51E2 _ 66 41: 0F 72. F3, 19
	psrld	xmm1, 7 				; 51E8 _ 66: 0F 72. D1, 07
	por	xmm11, xmm1				; 51ED _ 66 44: 0F EB. D9
	pslld	xmm0, 31				; 51F2 _ 66: 0F 72. F0, 1F
	psrld	xmm10, 1				; 51F7 _ 66 41: 0F 72. D2, 01
	por	xmm0, xmm10				; 51FD _ 66 41: 0F EB. C2
	movdqa	xmm10, xmm9				; 5202 _ 66 45: 0F 6F. D1
	pxor	xmm11, xmm2				; 5207 _ 66 44: 0F EF. DA
	pxor	xmm0, xmm9				; 520C _ 66 41: 0F EF. C1
	pslld	xmm10, 3				; 5211 _ 66 41: 0F 72. F2, 03
	pxor	xmm11, xmm10				; 5217 _ 66 45: 0F EF. DA
	movdqa	xmm10, xmm2				; 521C _ 66 44: 0F 6F. D2
	pxor	xmm0, xmm2				; 5221 _ 66: 0F EF. C2
	psrld	xmm2, 3 				; 5225 _ 66: 0F 72. D2, 03
	pslld	xmm10, 29				; 522A _ 66 41: 0F 72. F2, 1D
	por	xmm10, xmm2				; 5230 _ 66 44: 0F EB. D2
	movdqa	xmm2, xmm9				; 5235 _ 66 41: 0F 6F. D1
	psrld	xmm9, 13				; 523A _ 66 41: 0F 72. D1, 0D
	pslld	xmm2, 19				; 5240 _ 66: 0F 72. F2, 13
	por	xmm2, xmm9				; 5245 _ 66 41: 0F EB. D1
	movdqa	xmm9, xmm10				; 524A _ 66 45: 0F 6F. CA
	por	xmm10, xmm11				; 524F _ 66 45: 0F EB. D3
	pxor	xmm9, xmm2				; 5254 _ 66 44: 0F EF. CA
	pand	xmm2, xmm11				; 5259 _ 66 41: 0F DB. D3
	pxor	xmm9, xmm6				; 525E _ 66 44: 0F EF. CE
	pxor	xmm11, xmm0				; 5263 _ 66 44: 0F EF. D8
	por	xmm0, xmm2				; 5268 _ 66: 0F EB. C2
	pxor	xmm2, xmm9				; 526C _ 66 41: 0F EF. D1
	pand	xmm9, xmm10				; 5271 _ 66 45: 0F DB. CA
	pand	xmm11, xmm10				; 5276 _ 66 45: 0F DB. DA
	pxor	xmm0, xmm9				; 527B _ 66 41: 0F EF. C1
	pxor	xmm9, xmm2				; 5280 _ 66 44: 0F EF. CA
	por	xmm2, xmm9				; 5285 _ 66 41: 0F EB. D1
	pxor	xmm10, xmm0				; 528A _ 66 44: 0F EF. D0
	pxor	xmm2, xmm11				; 528F _ 66 41: 0F EF. D3
	pxor	xmm11, xmm10				; 5294 _ 66 45: 0F EF. DA
	por	xmm10, xmm2				; 5299 _ 66 44: 0F EB. D2
	pxor	xmm11, xmm9				; 529E _ 66 45: 0F EF. D9
	pxor	xmm10, xmm9				; 52A3 _ 66 45: 0F EF. D1
	movd	xmm9, dword [r12+12F0H] 		; 52A8 _ 66 45: 0F 6E. 8C 24, 000012F0
	pshufd	xmm9, xmm9, 0				; 52B2 _ 66 45: 0F 70. C9, 00
	pxor	xmm11, xmm9				; 52B8 _ 66 45: 0F EF. D9
	movd	xmm9, dword [r12+12F4H] 		; 52BD _ 66 45: 0F 6E. 8C 24, 000012F4
	pshufd	xmm9, xmm9, 0				; 52C7 _ 66 45: 0F 70. C9, 00
	pxor	xmm2, xmm9				; 52CD _ 66 41: 0F EF. D1
	movd	xmm9, dword [r12+12F8H] 		; 52D2 _ 66 45: 0F 6E. 8C 24, 000012F8
	pshufd	xmm9, xmm9, 0				; 52DC _ 66 45: 0F 70. C9, 00
	pxor	xmm0, xmm9				; 52E2 _ 66 41: 0F EF. C1
	movdqa	xmm1, xmm0				; 52E7 _ 66: 0F 6F. C8
	psrld	xmm0, 22				; 52EB _ 66: 0F 72. D0, 16
	movd	xmm9, dword [r12+12FCH] 		; 52F0 _ 66 45: 0F 6E. 8C 24, 000012FC
	pshufd	xmm9, xmm9, 0				; 52FA _ 66 45: 0F 70. C9, 00
	pxor	xmm10, xmm9				; 5300 _ 66 45: 0F EF. D1
	movdqa	xmm9, xmm11				; 5305 _ 66 45: 0F 6F. CB
	pslld	xmm1, 10				; 530A _ 66: 0F 72. F1, 0A
	por	xmm1, xmm0				; 530F _ 66: 0F EB. C8
	movdqa	xmm0, xmm10				; 5313 _ 66 41: 0F 6F. C2
	pslld	xmm9, 27				; 5318 _ 66 41: 0F 72. F1, 1B
	psrld	xmm11, 5				; 531E _ 66 41: 0F 72. D3, 05
	por	xmm9, xmm11				; 5324 _ 66 45: 0F EB. CB
	movdqa	xmm11, xmm2				; 5329 _ 66 44: 0F 6F. DA
	pxor	xmm1, xmm10				; 532E _ 66 41: 0F EF. CA
	pxor	xmm9, xmm2				; 5333 _ 66 44: 0F EF. CA
	pslld	xmm11, 7				; 5338 _ 66 41: 0F 72. F3, 07
	pxor	xmm1, xmm11				; 533E _ 66 41: 0F EF. CB
	pxor	xmm9, xmm10				; 5343 _ 66 45: 0F EF. CA
	pslld	xmm0, 25				; 5348 _ 66: 0F 72. F0, 19
	psrld	xmm10, 7				; 534D _ 66 41: 0F 72. D2, 07
	por	xmm0, xmm10				; 5353 _ 66 41: 0F EB. C2
	movdqa	xmm10, xmm2				; 5358 _ 66 44: 0F 6F. D2
	psrld	xmm2, 1 				; 535D _ 66: 0F 72. D2, 01
	pxor	xmm0, xmm1				; 5362 _ 66: 0F EF. C1
	pslld	xmm10, 31				; 5366 _ 66 41: 0F 72. F2, 1F
	por	xmm10, xmm2				; 536C _ 66 44: 0F EB. D2
	movdqa	xmm2, xmm9				; 5371 _ 66 41: 0F 6F. D1
	pxor	xmm10, xmm9				; 5376 _ 66 45: 0F EF. D1
	pslld	xmm2, 3 				; 537B _ 66: 0F 72. F2, 03
	pxor	xmm0, xmm2				; 5380 _ 66: 0F EF. C2
	movdqa	xmm2, xmm1				; 5384 _ 66: 0F 6F. D1
	movdqa	xmm11, xmm9				; 5388 _ 66 45: 0F 6F. D9
	pxor	xmm10, xmm1				; 538D _ 66 44: 0F EF. D1
	pslld	xmm2, 29				; 5392 _ 66: 0F 72. F2, 1D
	psrld	xmm1, 3 				; 5397 _ 66: 0F 72. D1, 03
	por	xmm2, xmm1				; 539C _ 66: 0F EB. D1
	pslld	xmm11, 19				; 53A0 _ 66 41: 0F 72. F3, 13
	psrld	xmm9, 13				; 53A6 _ 66 41: 0F 72. D1, 0D
	por	xmm11, xmm9				; 53AC _ 66 45: 0F EB. D9
	movdqa	xmm9, xmm2				; 53B1 _ 66 44: 0F 6F. CA
	pxor	xmm11, xmm2				; 53B6 _ 66 44: 0F EF. DA
	pxor	xmm2, xmm0				; 53BB _ 66: 0F EF. D0
	pand	xmm9, xmm11				; 53BF _ 66 45: 0F DB. CB
	pxor	xmm9, xmm6				; 53C4 _ 66 44: 0F EF. CE
	pxor	xmm0, xmm10				; 53C9 _ 66 41: 0F EF. C2
	pxor	xmm9, xmm0				; 53CE _ 66 44: 0F EF. C8
	por	xmm2, xmm11				; 53D3 _ 66 41: 0F EB. D3
	pxor	xmm11, xmm9				; 53D8 _ 66 45: 0F EF. D9
	pxor	xmm0, xmm2				; 53DD _ 66: 0F EF. C2
	pxor	xmm2, xmm10				; 53E1 _ 66 41: 0F EF. D2
	pand	xmm10, xmm0				; 53E6 _ 66 44: 0F DB. D0
	pxor	xmm10, xmm11				; 53EB _ 66 45: 0F EF. D3
	pxor	xmm11, xmm0				; 53F0 _ 66 44: 0F EF. D8
	por	xmm11, xmm9				; 53F5 _ 66 45: 0F EB. D9
	pxor	xmm0, xmm10				; 53FA _ 66 41: 0F EF. C2
	pxor	xmm2, xmm11				; 53FF _ 66 41: 0F EF. D3
	movd	xmm11, dword [r12+12E0H]		; 5404 _ 66 45: 0F 6E. 9C 24, 000012E0
	pshufd	xmm11, xmm11, 0 			; 540E _ 66 45: 0F 70. DB, 00
	pxor	xmm10, xmm11				; 5414 _ 66 45: 0F EF. D3
	movd	xmm11, dword [r12+12E4H]		; 5419 _ 66 45: 0F 6E. 9C 24, 000012E4
	pshufd	xmm11, xmm11, 0 			; 5423 _ 66 45: 0F 70. DB, 00
	pxor	xmm9, xmm11				; 5429 _ 66 45: 0F EF. CB
	movd	xmm11, dword [r12+12E8H]		; 542E _ 66 45: 0F 6E. 9C 24, 000012E8
	pshufd	xmm11, xmm11, 0 			; 5438 _ 66 45: 0F 70. DB, 00
	pxor	xmm2, xmm11				; 543E _ 66 41: 0F EF. D3
	movdqa	xmm1, xmm2				; 5443 _ 66: 0F 6F. CA
	psrld	xmm2, 22				; 5447 _ 66: 0F 72. D2, 16
	pslld	xmm1, 10				; 544C _ 66: 0F 72. F1, 0A
	movd	xmm11, dword [r12+12ECH]		; 5451 _ 66 45: 0F 6E. 9C 24, 000012EC
	pshufd	xmm11, xmm11, 0 			; 545B _ 66 45: 0F 70. DB, 00
	pxor	xmm0, xmm11				; 5461 _ 66 41: 0F EF. C3
	movdqa	xmm11, xmm10				; 5466 _ 66 45: 0F 6F. DA
	por	xmm1, xmm2				; 546B _ 66: 0F EB. CA
	movdqa	xmm2, xmm9				; 546F _ 66 41: 0F 6F. D1
	pslld	xmm11, 27				; 5474 _ 66 41: 0F 72. F3, 1B
	psrld	xmm10, 5				; 547A _ 66 41: 0F 72. D2, 05
	por	xmm11, xmm10				; 5480 _ 66 45: 0F EB. DA
	movdqa	xmm10, xmm9				; 5485 _ 66 45: 0F 6F. D1
	pxor	xmm1, xmm0				; 548A _ 66: 0F EF. C8
	pxor	xmm11, xmm9				; 548E _ 66 45: 0F EF. D9
	pslld	xmm10, 7				; 5493 _ 66 41: 0F 72. F2, 07
	pxor	xmm1, xmm10				; 5499 _ 66 41: 0F EF. CA
	movdqa	xmm10, xmm0				; 549E _ 66 44: 0F 6F. D0
	pxor	xmm11, xmm0				; 54A3 _ 66 44: 0F EF. D8
	psrld	xmm0, 7 				; 54A8 _ 66: 0F 72. D0, 07
	pslld	xmm10, 25				; 54AD _ 66 41: 0F 72. F2, 19
	por	xmm10, xmm0				; 54B3 _ 66 44: 0F EB. D0
	movdqa	xmm0, xmm1				; 54B8 _ 66: 0F 6F. C1
	pslld	xmm2, 31				; 54BC _ 66: 0F 72. F2, 1F
	psrld	xmm9, 1 				; 54C1 _ 66 41: 0F 72. D1, 01
	por	xmm2, xmm9				; 54C7 _ 66 41: 0F EB. D1
	movdqa	xmm9, xmm11				; 54CC _ 66 45: 0F 6F. CB
	pxor	xmm10, xmm1				; 54D1 _ 66 44: 0F EF. D1
	pxor	xmm2, xmm11				; 54D6 _ 66 41: 0F EF. D3
	pslld	xmm9, 3 				; 54DB _ 66 41: 0F 72. F1, 03
	pxor	xmm10, xmm9				; 54E1 _ 66 45: 0F EF. D1
	movdqa	xmm9, xmm10				; 54E6 _ 66 45: 0F 6F. CA
	pxor	xmm2, xmm1				; 54EB _ 66: 0F EF. D1
	pslld	xmm0, 29				; 54EF _ 66: 0F 72. F0, 1D
	psrld	xmm1, 3 				; 54F4 _ 66: 0F 72. D1, 03
	por	xmm0, xmm1				; 54F9 _ 66: 0F EB. C1
	movdqa	xmm1, xmm11				; 54FD _ 66 41: 0F 6F. CB
	psrld	xmm11, 13				; 5502 _ 66 41: 0F 72. D3, 0D
	pxor	xmm2, xmm6				; 5508 _ 66: 0F EF. D6
	pslld	xmm1, 19				; 550C _ 66: 0F 72. F1, 13
	por	xmm1, xmm11				; 5511 _ 66 41: 0F EB. CB
	pxor	xmm0, xmm2				; 5516 _ 66: 0F EF. C2
	por	xmm9, xmm1				; 551A _ 66 44: 0F EB. C9
	pxor	xmm9, xmm0				; 551F _ 66 44: 0F EF. C8
	por	xmm0, xmm2				; 5524 _ 66: 0F EB. C2
	pand	xmm0, xmm1				; 5528 _ 66: 0F DB. C1
	movd	xmm11, dword [r12+12D0H]		; 552C _ 66 45: 0F 6E. 9C 24, 000012D0
	pxor	xmm10, xmm9				; 5536 _ 66 45: 0F EF. D1
	pxor	xmm0, xmm10				; 553B _ 66 41: 0F EF. C2
	por	xmm10, xmm1				; 5540 _ 66 44: 0F EB. D1
	pxor	xmm10, xmm2				; 5545 _ 66 44: 0F EF. D2
	pand	xmm2, xmm0				; 554A _ 66: 0F DB. D0
	pxor	xmm2, xmm9				; 554E _ 66 41: 0F EF. D1
	pxor	xmm10, xmm0				; 5553 _ 66 44: 0F EF. D0
	pand	xmm9, xmm10				; 5558 _ 66 45: 0F DB. CA
	pxor	xmm10, xmm2				; 555D _ 66 44: 0F EF. D2
	pxor	xmm9, xmm10				; 5562 _ 66 45: 0F EF. CA
	pxor	xmm10, xmm6				; 5567 _ 66 44: 0F EF. D6
	pxor	xmm9, xmm1				; 556C _ 66 44: 0F EF. C9
	pshufd	xmm11, xmm11, 0 			; 5571 _ 66 45: 0F 70. DB, 00
	pxor	xmm2, xmm11				; 5577 _ 66 41: 0F EF. D3
	movdqa	xmm1, xmm2				; 557C _ 66: 0F 6F. CA
	psrld	xmm2, 5 				; 5580 _ 66: 0F 72. D2, 05
	pslld	xmm1, 27				; 5585 _ 66: 0F 72. F1, 1B
	por	xmm1, xmm2				; 558A _ 66: 0F EB. CA
	movd	xmm11, dword [r12+12D4H]		; 558E _ 66 45: 0F 6E. 9C 24, 000012D4
	pshufd	xmm11, xmm11, 0 			; 5598 _ 66 45: 0F 70. DB, 00
	pxor	xmm10, xmm11				; 559E _ 66 45: 0F EF. D3
	movdqa	xmm2, xmm10				; 55A3 _ 66 41: 0F 6F. D2
	pxor	xmm1, xmm10				; 55A8 _ 66 41: 0F EF. CA
	pslld	xmm2, 7 				; 55AD _ 66: 0F 72. F2, 07
	movd	xmm11, dword [r12+12D8H]		; 55B2 _ 66 45: 0F 6E. 9C 24, 000012D8
	pshufd	xmm11, xmm11, 0 			; 55BC _ 66 45: 0F 70. DB, 00
	pxor	xmm9, xmm11				; 55C2 _ 66 45: 0F EF. CB
	movd	xmm11, dword [r12+12DCH]		; 55C7 _ 66 45: 0F 6E. 9C 24, 000012DC
	pshufd	xmm11, xmm11, 0 			; 55D1 _ 66 45: 0F 70. DB, 00
	pxor	xmm0, xmm11				; 55D7 _ 66 41: 0F EF. C3
	movdqa	xmm11, xmm9				; 55DC _ 66 45: 0F 6F. D9
	psrld	xmm9, 22				; 55E1 _ 66 41: 0F 72. D1, 16
	pxor	xmm1, xmm0				; 55E7 _ 66: 0F EF. C8
	pslld	xmm11, 10				; 55EB _ 66 41: 0F 72. F3, 0A
	por	xmm11, xmm9				; 55F1 _ 66 45: 0F EB. D9
	movdqa	xmm9, xmm10				; 55F6 _ 66 45: 0F 6F. CA
	pxor	xmm11, xmm0				; 55FB _ 66 44: 0F EF. D8
	pxor	xmm11, xmm2				; 5600 _ 66 44: 0F EF. DA
	movdqa	xmm2, xmm0				; 5605 _ 66: 0F 6F. D0
	psrld	xmm0, 7 				; 5609 _ 66: 0F 72. D0, 07
	pslld	xmm9, 31				; 560E _ 66 41: 0F 72. F1, 1F
	pslld	xmm2, 25				; 5614 _ 66: 0F 72. F2, 19
	por	xmm2, xmm0				; 5619 _ 66: 0F EB. D0
	movdqa	xmm0, xmm11				; 561D _ 66 41: 0F 6F. C3
	psrld	xmm10, 1				; 5622 _ 66 41: 0F 72. D2, 01
	por	xmm9, xmm10				; 5628 _ 66 45: 0F EB. CA
	movdqa	xmm10, xmm1				; 562D _ 66 44: 0F 6F. D1
	pxor	xmm2, xmm11				; 5632 _ 66 41: 0F EF. D3
	pxor	xmm9, xmm1				; 5637 _ 66 44: 0F EF. C9
	pslld	xmm10, 3				; 563C _ 66 41: 0F 72. F2, 03
	pxor	xmm2, xmm10				; 5642 _ 66 41: 0F EF. D2
	movdqa	xmm10, xmm1				; 5647 _ 66 44: 0F 6F. D1
	pxor	xmm9, xmm11				; 564C _ 66 45: 0F EF. CB
	pslld	xmm0, 29				; 5651 _ 66: 0F 72. F0, 1D
	psrld	xmm11, 3				; 5656 _ 66 41: 0F 72. D3, 03
	por	xmm0, xmm11				; 565C _ 66 41: 0F EB. C3
	pslld	xmm10, 19				; 5661 _ 66 41: 0F 72. F2, 13
	psrld	xmm1, 13				; 5667 _ 66: 0F 72. D1, 0D
	por	xmm10, xmm1				; 566C _ 66 44: 0F EB. D1
	movdqa	xmm1, xmm0				; 5671 _ 66: 0F 6F. C8
	pand	xmm1, xmm2				; 5675 _ 66: 0F DB. CA
	pxor	xmm1, xmm9				; 5679 _ 66 41: 0F EF. C9
	por	xmm9, xmm2				; 567E _ 66 44: 0F EB. CA
	pand	xmm9, xmm10				; 5683 _ 66 45: 0F DB. CA
	pxor	xmm0, xmm1				; 5688 _ 66: 0F EF. C1
	pxor	xmm0, xmm9				; 568C _ 66 41: 0F EF. C1
	pand	xmm9, xmm1				; 5691 _ 66 44: 0F DB. C9
	pxor	xmm10, xmm6				; 5696 _ 66 44: 0F EF. D6
	pxor	xmm2, xmm0				; 569B _ 66: 0F EF. D0
	pxor	xmm9, xmm2				; 569F _ 66 44: 0F EF. CA
	pand	xmm2, xmm10				; 56A4 _ 66 41: 0F DB. D2
	pxor	xmm2, xmm1				; 56A9 _ 66: 0F EF. D1
	pxor	xmm10, xmm9				; 56AD _ 66 45: 0F EF. D1
	pand	xmm1, xmm10				; 56B2 _ 66 41: 0F DB. CA
	pxor	xmm2, xmm10				; 56B7 _ 66 41: 0F EF. D2
	pxor	xmm1, xmm0				; 56BC _ 66: 0F EF. C8
	por	xmm1, xmm2				; 56C0 _ 66: 0F EB. CA
	pxor	xmm2, xmm10				; 56C4 _ 66 41: 0F EF. D2
	pxor	xmm1, xmm9				; 56C9 _ 66 41: 0F EF. C9
	movd	xmm9, dword [r12+12C0H] 		; 56CE _ 66 45: 0F 6E. 8C 24, 000012C0
	pshufd	xmm11, xmm9, 0				; 56D8 _ 66 45: 0F 70. D9, 00
	pxor	xmm10, xmm11				; 56DE _ 66 45: 0F EF. D3
	movd	xmm9, dword [r12+12C4H] 		; 56E3 _ 66 45: 0F 6E. 8C 24, 000012C4
	pshufd	xmm11, xmm9, 0				; 56ED _ 66 45: 0F 70. D9, 00
	pxor	xmm2, xmm11				; 56F3 _ 66 41: 0F EF. D3
	movd	xmm9, dword [r12+12C8H] 		; 56F8 _ 66 45: 0F 6E. 8C 24, 000012C8
	pshufd	xmm11, xmm9, 0				; 5702 _ 66 45: 0F 70. D9, 00
	pxor	xmm1, xmm11				; 5708 _ 66 41: 0F EF. CB
	movd	xmm9, dword [r12+12CCH] 		; 570D _ 66 45: 0F 6E. 8C 24, 000012CC
	pshufd	xmm11, xmm9, 0				; 5717 _ 66 45: 0F 70. D9, 00
	movdqa	xmm9, xmm1				; 571D _ 66 44: 0F 6F. C9
	pxor	xmm0, xmm11				; 5722 _ 66 41: 0F EF. C3
	movdqa	xmm11, xmm10				; 5727 _ 66 45: 0F 6F. DA
	pslld	xmm9, 10				; 572C _ 66 41: 0F 72. F1, 0A
	psrld	xmm1, 22				; 5732 _ 66: 0F 72. D1, 16
	por	xmm9, xmm1				; 5737 _ 66 44: 0F EB. C9
	movdqa	xmm1, xmm2				; 573C _ 66: 0F 6F. CA
	pslld	xmm11, 27				; 5740 _ 66 41: 0F 72. F3, 1B
	psrld	xmm10, 5				; 5746 _ 66 41: 0F 72. D2, 05
	por	xmm11, xmm10				; 574C _ 66 45: 0F EB. DA
	movdqa	xmm10, xmm2				; 5751 _ 66 44: 0F 6F. D2
	pxor	xmm9, xmm0				; 5756 _ 66 44: 0F EF. C8
	pxor	xmm11, xmm2				; 575B _ 66 44: 0F EF. DA
	pslld	xmm10, 7				; 5760 _ 66 41: 0F 72. F2, 07
	pxor	xmm9, xmm10				; 5766 _ 66 45: 0F EF. CA
	movdqa	xmm10, xmm0				; 576B _ 66 44: 0F 6F. D0
	pxor	xmm11, xmm0				; 5770 _ 66 44: 0F EF. D8
	psrld	xmm0, 7 				; 5775 _ 66: 0F 72. D0, 07
	pslld	xmm10, 25				; 577A _ 66 41: 0F 72. F2, 19
	por	xmm10, xmm0				; 5780 _ 66 44: 0F EB. D0
	movdqa	xmm0, xmm9				; 5785 _ 66 41: 0F 6F. C1
	pslld	xmm1, 31				; 578A _ 66: 0F 72. F1, 1F
	psrld	xmm2, 1 				; 578F _ 66: 0F 72. D2, 01
	por	xmm1, xmm2				; 5794 _ 66: 0F EB. CA
	movdqa	xmm2, xmm11				; 5798 _ 66 41: 0F 6F. D3
	pxor	xmm10, xmm9				; 579D _ 66 45: 0F EF. D1
	pxor	xmm1, xmm11				; 57A2 _ 66 41: 0F EF. CB
	pslld	xmm2, 3 				; 57A7 _ 66: 0F 72. F2, 03
	pxor	xmm10, xmm2				; 57AC _ 66 44: 0F EF. D2
	movdqa	xmm2, xmm11				; 57B1 _ 66 41: 0F 6F. D3
	pxor	xmm1, xmm9				; 57B6 _ 66 41: 0F EF. C9
	pslld	xmm0, 29				; 57BB _ 66: 0F 72. F0, 1D
	psrld	xmm9, 3 				; 57C0 _ 66 41: 0F 72. D1, 03
	por	xmm0, xmm9				; 57C6 _ 66 41: 0F EB. C1
	movdqa	xmm9, xmm0				; 57CB _ 66 44: 0F 6F. C8
	pslld	xmm2, 19				; 57D0 _ 66: 0F 72. F2, 13
	psrld	xmm11, 13				; 57D5 _ 66 41: 0F 72. D3, 0D
	por	xmm2, xmm11				; 57DB _ 66 41: 0F EB. D3
	pxor	xmm9, xmm1				; 57E0 _ 66 44: 0F EF. C9
	pxor	xmm2, xmm9				; 57E5 _ 66 41: 0F EF. D1
	pand	xmm0, xmm9				; 57EA _ 66 41: 0F DB. C1
	pxor	xmm0, xmm2				; 57EF _ 66: 0F EF. C2
	pand	xmm2, xmm1				; 57F3 _ 66: 0F DB. D1
	movd	xmm11, dword [r12+12B0H]		; 57F7 _ 66 45: 0F 6E. 9C 24, 000012B0
	pxor	xmm1, xmm10				; 5801 _ 66 41: 0F EF. CA
	por	xmm10, xmm0				; 5806 _ 66 44: 0F EB. D0
	pxor	xmm9, xmm10				; 580B _ 66 45: 0F EF. CA
	pxor	xmm2, xmm10				; 5810 _ 66 41: 0F EF. D2
	pxor	xmm1, xmm0				; 5815 _ 66: 0F EF. C8
	pand	xmm10, xmm9				; 5819 _ 66 45: 0F DB. D1
	pxor	xmm10, xmm1				; 581E _ 66 44: 0F EF. D1
	pxor	xmm1, xmm2				; 5823 _ 66: 0F EF. CA
	por	xmm1, xmm9				; 5827 _ 66 41: 0F EB. C9
	pxor	xmm2, xmm10				; 582C _ 66 41: 0F EF. D2
	pxor	xmm1, xmm0				; 5831 _ 66: 0F EF. C8
	pxor	xmm2, xmm1				; 5835 _ 66: 0F EF. D1
	pshufd	xmm11, xmm11, 0 			; 5839 _ 66 45: 0F 70. DB, 00
	pxor	xmm9, xmm11				; 583F _ 66 45: 0F EF. CB
	movdqa	xmm0, xmm9				; 5844 _ 66 41: 0F 6F. C1
	psrld	xmm9, 5 				; 5849 _ 66 41: 0F 72. D1, 05
	pslld	xmm0, 27				; 584F _ 66: 0F 72. F0, 1B
	por	xmm0, xmm9				; 5854 _ 66 41: 0F EB. C1
	movd	xmm11, dword [r12+12B4H]		; 5859 _ 66 45: 0F 6E. 9C 24, 000012B4
	pshufd	xmm11, xmm11, 0 			; 5863 _ 66 45: 0F 70. DB, 00
	pxor	xmm1, xmm11				; 5869 _ 66 41: 0F EF. CB
	movdqa	xmm9, xmm1				; 586E _ 66 44: 0F 6F. C9
	pxor	xmm0, xmm1				; 5873 _ 66: 0F EF. C1
	pslld	xmm9, 7 				; 5877 _ 66 41: 0F 72. F1, 07
	movd	xmm11, dword [r12+12B8H]		; 587D _ 66 45: 0F 6E. 9C 24, 000012B8
	pshufd	xmm11, xmm11, 0 			; 5887 _ 66 45: 0F 70. DB, 00
	pxor	xmm10, xmm11				; 588D _ 66 45: 0F EF. D3
	movd	xmm11, dword [r12+12BCH]		; 5892 _ 66 45: 0F 6E. 9C 24, 000012BC
	pshufd	xmm11, xmm11, 0 			; 589C _ 66 45: 0F 70. DB, 00
	pxor	xmm2, xmm11				; 58A2 _ 66 41: 0F EF. D3
	movdqa	xmm11, xmm10				; 58A7 _ 66 45: 0F 6F. DA
	psrld	xmm10, 22				; 58AC _ 66 41: 0F 72. D2, 16
	pxor	xmm0, xmm2				; 58B2 _ 66: 0F EF. C2
	pslld	xmm11, 10				; 58B6 _ 66 41: 0F 72. F3, 0A
	por	xmm11, xmm10				; 58BC _ 66 45: 0F EB. DA
	movdqa	xmm10, xmm2				; 58C1 _ 66 44: 0F 6F. D2
	pxor	xmm11, xmm2				; 58C6 _ 66 44: 0F EF. DA
	pxor	xmm11, xmm9				; 58CB _ 66 45: 0F EF. D9
	movdqa	xmm9, xmm0				; 58D0 _ 66 44: 0F 6F. C8
	pslld	xmm10, 25				; 58D5 _ 66 41: 0F 72. F2, 19
	psrld	xmm2, 7 				; 58DB _ 66: 0F 72. D2, 07
	por	xmm10, xmm2				; 58E0 _ 66 44: 0F EB. D2
	movdqa	xmm2, xmm1				; 58E5 _ 66: 0F 6F. D1
	psrld	xmm1, 1 				; 58E9 _ 66: 0F 72. D1, 01
	pxor	xmm10, xmm11				; 58EE _ 66 45: 0F EF. D3
	pslld	xmm2, 31				; 58F3 _ 66: 0F 72. F2, 1F
	por	xmm2, xmm1				; 58F8 _ 66: 0F EB. D1
	movdqa	xmm1, xmm11				; 58FC _ 66 41: 0F 6F. CB
	pslld	xmm9, 3 				; 5901 _ 66 41: 0F 72. F1, 03
	pxor	xmm10, xmm9				; 5907 _ 66 45: 0F EF. D1
	pxor	xmm2, xmm0				; 590C _ 66: 0F EF. D0
	pxor	xmm2, xmm11				; 5910 _ 66 41: 0F EF. D3
	pslld	xmm1, 29				; 5915 _ 66: 0F 72. F1, 1D
	psrld	xmm11, 3				; 591A _ 66 41: 0F 72. D3, 03
	por	xmm1, xmm11				; 5920 _ 66 41: 0F EB. CB
	movdqa	xmm11, xmm0				; 5925 _ 66 44: 0F 6F. D8
	psrld	xmm0, 13				; 592A _ 66: 0F 72. D0, 0D
	pxor	xmm1, xmm10				; 592F _ 66 41: 0F EF. CA
	pslld	xmm11, 19				; 5934 _ 66 41: 0F 72. F3, 13
	por	xmm11, xmm0				; 593A _ 66 44: 0F EB. D8
	pxor	xmm10, xmm11				; 593F _ 66 45: 0F EF. D3
	movdqa	xmm9, xmm10				; 5944 _ 66 45: 0F 6F. CA
	pand	xmm9, xmm1				; 5949 _ 66 44: 0F DB. C9
	pxor	xmm9, xmm2				; 594E _ 66 44: 0F EF. CA
	por	xmm2, xmm1				; 5953 _ 66: 0F EB. D1
	pxor	xmm2, xmm10				; 5957 _ 66 41: 0F EF. D2
	pand	xmm10, xmm9				; 595C _ 66 45: 0F DB. D1
	pxor	xmm1, xmm9				; 5961 _ 66 41: 0F EF. C9
	pand	xmm10, xmm11				; 5966 _ 66 45: 0F DB. D3
	pxor	xmm10, xmm1				; 596B _ 66 44: 0F EF. D1
	pand	xmm1, xmm2				; 5970 _ 66: 0F DB. CA
	por	xmm1, xmm11				; 5974 _ 66 41: 0F EB. CB
	pxor	xmm9, xmm6				; 5979 _ 66 44: 0F EF. CE
	movdqa	xmm0, xmm9				; 597E _ 66 41: 0F 6F. C1
	pxor	xmm1, xmm9				; 5983 _ 66 41: 0F EF. C9
	pxor	xmm11, xmm9				; 5988 _ 66 45: 0F EF. D9
	pxor	xmm0, xmm10				; 598D _ 66 41: 0F EF. C2
	pand	xmm11, xmm2				; 5992 _ 66 44: 0F DB. DA
	pxor	xmm0, xmm11				; 5997 _ 66 41: 0F EF. C3
	movd	xmm11, dword [r12+12A0H]		; 599C _ 66 45: 0F 6E. 9C 24, 000012A0
	pshufd	xmm11, xmm11, 0 			; 59A6 _ 66 45: 0F 70. DB, 00
	pxor	xmm2, xmm11				; 59AC _ 66 41: 0F EF. D3
	movd	xmm9, dword [r12+12A4H] 		; 59B1 _ 66 45: 0F 6E. 8C 24, 000012A4
	pshufd	xmm11, xmm9, 0				; 59BB _ 66 45: 0F 70. D9, 00
	pxor	xmm10, xmm11				; 59C1 _ 66 45: 0F EF. D3
	movd	xmm9, dword [r12+12A8H] 		; 59C6 _ 66 45: 0F 6E. 8C 24, 000012A8
	pshufd	xmm11, xmm9, 0				; 59D0 _ 66 45: 0F 70. D9, 00
	pxor	xmm1, xmm11				; 59D6 _ 66 41: 0F EF. CB
	movd	xmm9, dword [r12+12ACH] 		; 59DB _ 66 45: 0F 6E. 8C 24, 000012AC
	pshufd	xmm11, xmm9, 0				; 59E5 _ 66 45: 0F 70. D9, 00
	pxor	xmm0, xmm11				; 59EB _ 66 41: 0F EF. C3
	movdqa	xmm11, xmm1				; 59F0 _ 66 44: 0F 6F. D9
	movdqa	xmm9, xmm0				; 59F5 _ 66 44: 0F 6F. C8
	psrld	xmm1, 22				; 59FA _ 66: 0F 72. D1, 16
	pslld	xmm11, 10				; 59FF _ 66 41: 0F 72. F3, 0A
	por	xmm11, xmm1				; 5A05 _ 66 44: 0F EB. D9
	movdqa	xmm1, xmm2				; 5A0A _ 66: 0F 6F. CA
	psrld	xmm2, 5 				; 5A0E _ 66: 0F 72. D2, 05
	pxor	xmm11, xmm0				; 5A13 _ 66 44: 0F EF. D8
	pslld	xmm1, 27				; 5A18 _ 66: 0F 72. F1, 1B
	por	xmm1, xmm2				; 5A1D _ 66: 0F EB. CA
	movdqa	xmm2, xmm10				; 5A21 _ 66 41: 0F 6F. D2
	pxor	xmm1, xmm10				; 5A26 _ 66 41: 0F EF. CA
	pxor	xmm1, xmm0				; 5A2B _ 66: 0F EF. C8
	pslld	xmm2, 7 				; 5A2F _ 66: 0F 72. F2, 07
	pxor	xmm11, xmm2				; 5A34 _ 66 44: 0F EF. DA
	pslld	xmm9, 25				; 5A39 _ 66 41: 0F 72. F1, 19
	psrld	xmm0, 7 				; 5A3F _ 66: 0F 72. D0, 07
	por	xmm9, xmm0				; 5A44 _ 66 44: 0F EB. C8
	movdqa	xmm0, xmm10				; 5A49 _ 66 41: 0F 6F. C2
	psrld	xmm10, 1				; 5A4E _ 66 41: 0F 72. D2, 01
	pxor	xmm9, xmm11				; 5A54 _ 66 45: 0F EF. CB
	pslld	xmm0, 31				; 5A59 _ 66: 0F 72. F0, 1F
	por	xmm0, xmm10				; 5A5E _ 66 41: 0F EB. C2
	movdqa	xmm10, xmm1				; 5A63 _ 66 44: 0F 6F. D1
	pxor	xmm0, xmm1				; 5A68 _ 66: 0F EF. C1
	pxor	xmm0, xmm11				; 5A6C _ 66 41: 0F EF. C3
	movdqa	xmm2, xmm0				; 5A71 _ 66: 0F 6F. D0
	pslld	xmm10, 3				; 5A75 _ 66 41: 0F 72. F2, 03
	pxor	xmm9, xmm10				; 5A7B _ 66 45: 0F EF. CA
	movdqa	xmm10, xmm11				; 5A80 _ 66 45: 0F 6F. D3
	psrld	xmm11, 3				; 5A85 _ 66 41: 0F 72. D3, 03
	pxor	xmm2, xmm9				; 5A8B _ 66 41: 0F EF. D1
	pslld	xmm10, 29				; 5A90 _ 66 41: 0F 72. F2, 1D
	por	xmm10, xmm11				; 5A96 _ 66 45: 0F EB. D3
	movdqa	xmm11, xmm1				; 5A9B _ 66 44: 0F 6F. D9
	psrld	xmm1, 13				; 5AA0 _ 66: 0F 72. D1, 0D
	pand	xmm9, xmm2				; 5AA5 _ 66 44: 0F DB. CA
	pslld	xmm11, 19				; 5AAA _ 66 41: 0F 72. F3, 13
	por	xmm11, xmm1				; 5AB0 _ 66 44: 0F EB. D9
	pxor	xmm0, xmm10				; 5AB5 _ 66 41: 0F EF. C2
	pxor	xmm9, xmm11				; 5ABA _ 66 45: 0F EF. CB
	por	xmm11, xmm2				; 5ABF _ 66 44: 0F EB. DA
	pxor	xmm10, xmm9				; 5AC4 _ 66 45: 0F EF. D1
	pxor	xmm11, xmm0				; 5AC9 _ 66 44: 0F EF. D8
	por	xmm11, xmm10				; 5ACE _ 66 45: 0F EB. DA
	pxor	xmm2, xmm9				; 5AD3 _ 66 41: 0F EF. D1
	pxor	xmm11, xmm2				; 5AD8 _ 66 44: 0F EF. DA
	por	xmm2, xmm9				; 5ADD _ 66 41: 0F EB. D1
	pxor	xmm2, xmm11				; 5AE2 _ 66 41: 0F EF. D3
	pxor	xmm0, xmm6				; 5AE7 _ 66: 0F EF. C6
	pxor	xmm0, xmm2				; 5AEB _ 66: 0F EF. C2
	por	xmm2, xmm11				; 5AEF _ 66 41: 0F EB. D3
	pxor	xmm2, xmm11				; 5AF4 _ 66 41: 0F EF. D3
	por	xmm2, xmm0				; 5AF9 _ 66: 0F EB. D0
	pxor	xmm9, xmm2				; 5AFD _ 66 44: 0F EF. CA
	movd	xmm2, dword [r12+1290H] 		; 5B02 _ 66 41: 0F 6E. 94 24, 00001290
	pshufd	xmm2, xmm2, 0				; 5B0C _ 66: 0F 70. D2, 00
	pxor	xmm0, xmm2				; 5B11 _ 66: 0F EF. C2
	movd	xmm2, dword [r12+1294H] 		; 5B15 _ 66 41: 0F 6E. 94 24, 00001294
	pshufd	xmm2, xmm2, 0				; 5B1F _ 66: 0F 70. D2, 00
	pxor	xmm11, xmm2				; 5B24 _ 66 44: 0F EF. DA
	movd	xmm2, dword [r12+1298H] 		; 5B29 _ 66 41: 0F 6E. 94 24, 00001298
	pshufd	xmm2, xmm2, 0				; 5B33 _ 66: 0F 70. D2, 00
	pxor	xmm9, xmm2				; 5B38 _ 66 44: 0F EF. CA
	movdqa	xmm1, xmm9				; 5B3D _ 66 41: 0F 6F. C9
	psrld	xmm9, 22				; 5B42 _ 66 41: 0F 72. D1, 16
	pslld	xmm1, 10				; 5B48 _ 66: 0F 72. F1, 0A
	por	xmm1, xmm9				; 5B4D _ 66 41: 0F EB. C9
	movdqa	xmm9, xmm0				; 5B52 _ 66 44: 0F 6F. C8
	movd	xmm2, dword [r12+129CH] 		; 5B57 _ 66 41: 0F 6E. 94 24, 0000129C
	pshufd	xmm2, xmm2, 0				; 5B61 _ 66: 0F 70. D2, 00
	pxor	xmm10, xmm2				; 5B66 _ 66 44: 0F EF. D2
	movdqa	xmm2, xmm11				; 5B6B _ 66 41: 0F 6F. D3
	pslld	xmm9, 27				; 5B70 _ 66 41: 0F 72. F1, 1B
	psrld	xmm0, 5 				; 5B76 _ 66: 0F 72. D0, 05
	por	xmm9, xmm0				; 5B7B _ 66 44: 0F EB. C8
	movdqa	xmm0, xmm10				; 5B80 _ 66 41: 0F 6F. C2
	pxor	xmm1, xmm10				; 5B85 _ 66 41: 0F EF. CA
	pslld	xmm2, 7 				; 5B8A _ 66: 0F 72. F2, 07
	pxor	xmm1, xmm2				; 5B8F _ 66: 0F EF. CA
	movdqa	xmm2, xmm1				; 5B93 _ 66: 0F 6F. D1
	pxor	xmm9, xmm11				; 5B97 _ 66 45: 0F EF. CB
	pxor	xmm9, xmm10				; 5B9C _ 66 45: 0F EF. CA
	pslld	xmm0, 25				; 5BA1 _ 66: 0F 72. F0, 19
	psrld	xmm10, 7				; 5BA6 _ 66 41: 0F 72. D2, 07
	por	xmm0, xmm10				; 5BAC _ 66 41: 0F EB. C2
	movdqa	xmm10, xmm11				; 5BB1 _ 66 45: 0F 6F. D3
	psrld	xmm11, 1				; 5BB6 _ 66 41: 0F 72. D3, 01
	pxor	xmm0, xmm1				; 5BBC _ 66: 0F EF. C1
	pslld	xmm10, 31				; 5BC0 _ 66 41: 0F 72. F2, 1F
	por	xmm10, xmm11				; 5BC6 _ 66 45: 0F EB. D3
	movdqa	xmm11, xmm9				; 5BCB _ 66 45: 0F 6F. D9
	pxor	xmm10, xmm9				; 5BD0 _ 66 45: 0F EF. D1
	pxor	xmm10, xmm1				; 5BD5 _ 66 44: 0F EF. D1
	pslld	xmm11, 3				; 5BDA _ 66 41: 0F 72. F3, 03
	pxor	xmm0, xmm11				; 5BE0 _ 66 41: 0F EF. C3
	movdqa	xmm11, xmm9				; 5BE5 _ 66 45: 0F 6F. D9
	pslld	xmm2, 29				; 5BEA _ 66: 0F 72. F2, 1D
	psrld	xmm1, 3 				; 5BEF _ 66: 0F 72. D1, 03
	por	xmm2, xmm1				; 5BF4 _ 66: 0F EB. D1
	movdqa	xmm1, xmm10				; 5BF8 _ 66 41: 0F 6F. CA
	pslld	xmm11, 19				; 5BFD _ 66 41: 0F 72. F3, 13
	psrld	xmm9, 13				; 5C03 _ 66 41: 0F 72. D1, 0D
	por	xmm11, xmm9				; 5C09 _ 66 45: 0F EB. D9
	pxor	xmm2, xmm6				; 5C0E _ 66: 0F EF. D6
	por	xmm1, xmm11				; 5C12 _ 66 41: 0F EB. CB
	movd	xmm9, dword [r12+1280H] 		; 5C17 _ 66 45: 0F 6E. 8C 24, 00001280
	pxor	xmm10, xmm6				; 5C21 _ 66 44: 0F EF. D6
	pxor	xmm1, xmm2				; 5C26 _ 66: 0F EF. CA
	por	xmm2, xmm10				; 5C2A _ 66 41: 0F EB. D2
	pxor	xmm1, xmm0				; 5C2F _ 66: 0F EF. C8
	pxor	xmm11, xmm10				; 5C33 _ 66 45: 0F EF. DA
	pxor	xmm2, xmm11				; 5C38 _ 66 41: 0F EF. D3
	pand	xmm11, xmm0				; 5C3D _ 66 44: 0F DB. D8
	pxor	xmm10, xmm11				; 5C42 _ 66 45: 0F EF. D3
	por	xmm11, xmm1				; 5C47 _ 66 44: 0F EB. D9
	pxor	xmm11, xmm2				; 5C4C _ 66 44: 0F EF. DA
	pxor	xmm0, xmm10				; 5C51 _ 66 41: 0F EF. C2
	pxor	xmm2, xmm1				; 5C56 _ 66: 0F EF. D1
	pxor	xmm0, xmm11				; 5C5A _ 66 41: 0F EF. C3
	pxor	xmm0, xmm1				; 5C5F _ 66: 0F EF. C1
	pand	xmm2, xmm0				; 5C63 _ 66: 0F DB. D0
	pxor	xmm10, xmm2				; 5C67 _ 66 44: 0F EF. D2
	pshufd	xmm2, xmm9, 0				; 5C6C _ 66 41: 0F 70. D1, 00
	pxor	xmm11, xmm2				; 5C72 _ 66 44: 0F EF. DA
	add	r13, 64 				; 5C77 _ 49: 83. C5, 40
	inc	r10d					; 5C7B _ 41: FF. C2
	movd	xmm9, dword [r12+1284H] 		; 5C7E _ 66 45: 0F 6E. 8C 24, 00001284
	pshufd	xmm2, xmm9, 0				; 5C88 _ 66 41: 0F 70. D1, 00
	pxor	xmm10, xmm2				; 5C8E _ 66 44: 0F EF. D2
	movd	xmm9, dword [r12+1288H] 		; 5C93 _ 66 45: 0F 6E. 8C 24, 00001288
	pshufd	xmm2, xmm9, 0				; 5C9D _ 66 41: 0F 70. D1, 00
	movd	xmm9, dword [r12+128CH] 		; 5CA3 _ 66 45: 0F 6E. 8C 24, 0000128C
	pxor	xmm1, xmm2				; 5CAD _ 66: 0F EF. CA
	pshufd	xmm2, xmm9, 0				; 5CB1 _ 66 41: 0F 70. D1, 00
	movdqa	xmm9, xmm11				; 5CB7 _ 66 45: 0F 6F. CB
	pxor	xmm0, xmm2				; 5CBC _ 66: 0F EF. C2
	movdqa	xmm2, xmm1				; 5CC0 _ 66: 0F 6F. D1
	punpckldq xmm9, xmm10				; 5CC4 _ 66 45: 0F 62. CA
	punpckhdq xmm11, xmm10				; 5CC9 _ 66 45: 0F 6A. DA
	movdqa	xmm10, xmm9				; 5CCE _ 66 45: 0F 6F. D1
	punpckldq xmm2, xmm0				; 5CD3 _ 66: 0F 62. D0
	punpckhdq xmm1, xmm0				; 5CD7 _ 66: 0F 6A. C8
	punpcklqdq xmm10, xmm2				; 5CDB _ 66 44: 0F 6C. D2
	punpckhqdq xmm9, xmm2				; 5CE0 _ 66 44: 0F 6D. CA
	movdqa	xmm2, xmm11				; 5CE5 _ 66 41: 0F 6F. D3
	punpckhqdq xmm11, xmm1				; 5CEA _ 66 44: 0F 6D. D9
	pxor	xmm10, xmm8				; 5CEF _ 66 45: 0F EF. D0
	movdqu	oword [rbp], xmm10			; 5CF4 _ F3 44: 0F 7F. 55, 00
	movdqa	xmm8, xmm3				; 5CFA _ 66 44: 0F 6F. C3
	punpcklqdq xmm2, xmm1				; 5CFF _ 66: 0F 6C. D1
	pxor	xmm9, xmm5				; 5D03 _ 66 44: 0F EF. CD
	movdqu	oword [rbp+10H], xmm9			; 5D08 _ F3 44: 0F 7F. 4D, 10
	pxor	xmm2, xmm4				; 5D0E _ 66: 0F EF. D4
	movdqu	oword [rbp+20H], xmm2			; 5D12 _ F3: 0F 7F. 55, 20
	movdqa	xmm4, xmm3				; 5D17 _ 66: 0F 6F. E3
	pxor	xmm11, xmm3				; 5D1B _ 66 44: 0F EF. DB
	movdqu	oword [rbp+30H], xmm11			; 5D20 _ F3 44: 0F 7F. 5D, 30
	psllq	xmm8, 1 				; 5D26 _ 66 41: 0F 73. F0, 01
	pslldq	xmm4, 8 				; 5D2C _ 66: 0F 73. FC, 08
	psrldq	xmm4, 7 				; 5D31 _ 66: 0F 73. DC, 07
	psrlq	xmm4, 7 				; 5D36 _ 66: 0F 73. D4, 07
	por	xmm8, xmm4				; 5D3B _ 66 44: 0F EB. C4
	psraw	xmm3, 8 				; 5D40 _ 66: 0F 71. E3, 08
	psrldq	xmm3, 15				; 5D45 _ 66: 0F 73. DB, 0F
	pand	xmm3, xmm7				; 5D4A _ 66: 0F DB. DF
	pxor	xmm8, xmm3				; 5D4E _ 66 44: 0F EF. C3
	add	rbp, 64 				; 5D53 _ 48: 83. C5, 40
	cmp	r10d, 8 				; 5D57 _ 41: 83. FA, 08
	jl	?_006					; 5D5B _ 0F 8C, FFFFD217
	add	r14, -512				; 5D61 _ 49: 81. C6, FFFFFE00
	jne	?_004					; 5D68 _ 0F 85, FFFFD1D3
	movaps	xmm6, oword [rsp+70H]			; 5D6E _ 0F 28. 74 24, 70
	movaps	xmm7, oword [rsp+60H]			; 5D73 _ 0F 28. 7C 24, 60
	movaps	xmm8, oword [rsp+50H]			; 5D78 _ 44: 0F 28. 44 24, 50
	movaps	xmm9, oword [rsp+40H]			; 5D7E _ 44: 0F 28. 4C 24, 40
	movaps	xmm10, oword [rsp+30H]			; 5D84 _ 44: 0F 28. 54 24, 30
	movaps	xmm11, oword [rsp+20H]			; 5D8A _ 44: 0F 28. 5C 24, 20
	add	rsp, 160				; 5D90 _ 48: 81. C4, 000000A0
	pop	rbp					; 5D97 _ 5D
	pop	r12					; 5D98 _ 41: 5C
	pop	r13					; 5D9A _ 41: 5D
	pop	r14					; 5D9C _ 41: 5E
	pop	r15					; 5D9E _ 41: 5F
	ret						; 5DA0 _ C3
; xts_serpent_sse2_decrypt End of function

; Filling space: 0FH
; Filler type: lea with same source and destination
;       db 48H, 8DH, 0B4H, 26H, 00H, 00H, 00H, 00H
;       db 48H, 8DH, 0BFH, 00H, 00H, 00H, 00H

ALIGN	16

xts_serpent_sse2_available:; Function begin
	push	rsi					; 5DB0 _ 56
	push	rbx					; 5DB1 _ 53
	sub	rsp, 56 				; 5DB2 _ 48: 83. EC, 38
	mov	eax, 1					; 5DB6 _ B8, 00000001
	lea	rsi, [rsp+20H]				; 5DBB _ 48: 8D. 74 24, 20
	cpuid						; 5DC0 _ 0F A2
	mov	dword [rsi], eax			; 5DC2 _ 89. 06
	mov	dword [rsi+4H], ebx			; 5DC4 _ 89. 5E, 04
	mov	dword [rsi+8H], ecx			; 5DC7 _ 89. 4E, 08
	mov	dword [rsi+0CH], edx			; 5DCA _ 89. 56, 0C
	mov	eax, dword [rsp+2CH]			; 5DCD _ 8B. 44 24, 2C
	and	eax, 4000000H				; 5DD1 _ 25, 04000000
	shr	eax, 26 				; 5DD6 _ C1. E8, 1A
	add	rsp, 56 				; 5DD9 _ 48: 83. C4, 38
	pop	rbx					; 5DDD _ 5B
	pop	rsi					; 5DDE _ 5E
	ret						; 5DDF _ C3
; xts_serpent_sse2_available End of function




