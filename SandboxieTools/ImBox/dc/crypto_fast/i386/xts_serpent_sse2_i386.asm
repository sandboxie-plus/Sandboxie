; this code compiled with Intel® C++ Compiler Version 11.1.835.200
;
; Disassembly of file: xts_serpent_sse2.obj
; Mon May 10 08:12:48 2010
; Mode: 32 bits
; Syntax: YASM/NASM
; Instruction set: SSE2


global _xts_serpent_sse2_encrypt@24
global _xts_serpent_sse2_decrypt@24
global _xts_serpent_sse2_available@0

extern _serpent256_encrypt@12				; near

SECTION .text	align=16 execute			; section number 2, code
;  Communal section not supported by YASM

_xts_serpent_sse2_encrypt@24:; Function begin
	push	ebp					; 0000 _ 55
	mov	ebp, esp				; 0001 _ 8B. EC
	and	esp, 0FFFFFFF0H 			; 0003 _ 83. E4, F0
	push	edi					; 0006 _ 57
	push	esi					; 0007 _ 56
	push	ebx					; 0008 _ 53
	sub	esp, 116				; 0009 _ 83. EC, 74
	mov	edx, dword [ebp+8H]			; 000C _ 8B. 55, 08
	mov	eax, dword [ebp+0CH]			; 000F _ 8B. 45, 0C
	mov	esi, dword [ebp+18H]			; 0012 _ 8B. 75, 18
	mov	ebx, dword [ebp+1CH]			; 0015 _ 8B. 5D, 1C
	mov	edi, dword [ebp+14H]			; 0018 _ 8B. 7D, 14
	mov	dword [esp+24H], eax			; 001B _ 89. 44 24, 24
	mov	ecx, esi				; 001F _ 8B. CE
	mov	dword [esp+28H], edx			; 0021 _ 89. 54 24, 28
	shl	ecx, 23 				; 0025 _ C1. E1, 17
	shr	esi, 9					; 0028 _ C1. EE, 09
	mov	dword [esp+4H], esi			; 002B _ 89. 74 24, 04
	mov	esi, dword [ebp+10H]			; 002F _ 8B. 75, 10
	shr	edi, 9					; 0032 _ C1. EF, 09
	or	ecx, edi				; 0035 _ 0B. CF
	lea	ebx, [ebx+5710H]			; 0037 _ 8D. 9B, 00005710
	mov	dword [esp], ecx			; 003D _ 89. 0C 24
	xor	ecx, ecx				; 0040 _ 33. C9
	mov	dword [esp+8H], ecx			; 0042 _ 89. 4C 24, 08
	mov	dword [esp+0CH], ecx			; 0046 _ 89. 4C 24, 0C
	mov	edi, 135				; 004A _ BF, 00000087
	movd	xmm1, edi				; 004F _ 66: 0F 6E. CF
	movdqa	oword [esp+30H], xmm1			; 0053 _ 66: 0F 7F. 4C 24, 30
	lea	edi, [esp+10H]				; 0059 _ 8D. 7C 24, 10
	jmp	?_002					; 005D _ EB, 06

?_001:	movdqa	oword [esp+10H], xmm0			; 005F _ 66: 0F 7F. 44 24, 10
?_002:	add	dword [esp], 1				; 0065 _ 83. 04 24, 01
	adc	dword [esp+4H], 0			; 0069 _ 83. 54 24, 04, 00
	push	ebx					; 006E _ 53
	push	edi					; 006F _ 57
	lea	eax, [esp+8H]				; 0070 _ 8D. 44 24, 08
	push	eax					; 0074 _ 50
	call	_serpent256_encrypt@12			; 0075 _ E8, 00000000(rel)
	movdqa	xmm0, oword [esp+10H]			; 007A _ 66: 0F 6F. 44 24, 10
	mov	edx, dword [esp+24H]			; 0080 _ 8B. 54 24, 24
	mov	ecx, dword [ebp+1CH]			; 0084 _ 8B. 4D, 1C
	movdqa	oword [esp+60H], xmm0			; 0087 _ 66: 0F 7F. 44 24, 60
	mov	dword [esp+20H], esi			; 008D _ 89. 74 24, 20
	mov	esi, dword [esp+28H]			; 0091 _ 8B. 74 24, 28
	xor	eax, eax				; 0095 _ 33. C0
	pcmpeqd xmm4, xmm4				; 0097 _ 66: 0F 76. E4
?_003:	movdqa	xmm5, oword [esp+60H]			; 009B _ 66: 0F 6F. 6C 24, 60
	movdqa	xmm7, xmm5				; 00A1 _ 66: 0F 6F. FD
	movdqa	xmm3, xmm5				; 00A5 _ 66: 0F 6F. DD
	movdqa	xmm6, xmm5				; 00A9 _ 66: 0F 6F. F5
	psllq	xmm7, 1 				; 00AD _ 66: 0F 73. F7, 01
	pslldq	xmm3, 8 				; 00B2 _ 66: 0F 73. FB, 08
	movdqa	xmm1, oword [esp+30H]			; 00B7 _ 66: 0F 6F. 4C 24, 30
	psrldq	xmm3, 7 				; 00BD _ 66: 0F 73. DB, 07
	psrlq	xmm3, 7 				; 00C2 _ 66: 0F 73. D3, 07
	por	xmm7, xmm3				; 00C7 _ 66: 0F EB. FB
	psraw	xmm6, 8 				; 00CB _ 66: 0F 71. E6, 08
	psrldq	xmm6, 15				; 00D0 _ 66: 0F 73. DE, 0F
	pand	xmm6, xmm1				; 00D5 _ 66: 0F DB. F1
	pxor	xmm7, xmm6				; 00D9 _ 66: 0F EF. FE
	movdqa	oword [esp+40H], xmm7			; 00DD _ 66: 0F 7F. 7C 24, 40
	movdqa	xmm2, xmm7				; 00E3 _ 66: 0F 6F. D7
	movdqa	xmm0, xmm7				; 00E7 _ 66: 0F 6F. C7
	movdqa	xmm3, xmm7				; 00EB _ 66: 0F 6F. DF
	psllq	xmm2, 1 				; 00EF _ 66: 0F 73. F2, 01
	pslldq	xmm0, 8 				; 00F4 _ 66: 0F 73. F8, 08
	psrldq	xmm0, 7 				; 00F9 _ 66: 0F 73. D8, 07
	psrlq	xmm0, 7 				; 00FE _ 66: 0F 73. D0, 07
	por	xmm2, xmm0				; 0103 _ 66: 0F EB. D0
	psraw	xmm3, 8 				; 0107 _ 66: 0F 71. E3, 08
	psrldq	xmm3, 15				; 010C _ 66: 0F 73. DB, 0F
	pand	xmm3, xmm1				; 0111 _ 66: 0F DB. D9
	pxor	xmm2, xmm3				; 0115 _ 66: 0F EF. D3
	movdqa	oword [esp+50H], xmm2			; 0119 _ 66: 0F 7F. 54 24, 50
	movdqa	xmm3, xmm2				; 011F _ 66: 0F 6F. DA
	movdqa	xmm6, xmm2				; 0123 _ 66: 0F 6F. F2
	movdqa	xmm0, xmm2				; 0127 _ 66: 0F 6F. C2
	psllq	xmm3, 1 				; 012B _ 66: 0F 73. F3, 01
	pslldq	xmm6, 8 				; 0130 _ 66: 0F 73. FE, 08
	psrldq	xmm6, 7 				; 0135 _ 66: 0F 73. DE, 07
	psrlq	xmm6, 7 				; 013A _ 66: 0F 73. D6, 07
	por	xmm3, xmm6				; 013F _ 66: 0F EB. DE
	movdqu	xmm6, oword [esi+10H]			; 0143 _ F3: 0F 6F. 76, 10
	psraw	xmm0, 8 				; 0148 _ 66: 0F 71. E0, 08
	psrldq	xmm0, 15				; 014D _ 66: 0F 73. D8, 0F
	pand	xmm0, xmm1				; 0152 _ 66: 0F DB. C1
	movdqu	xmm1, oword [esi+20H]			; 0156 _ F3: 0F 6F. 4E, 20
	pxor	xmm3, xmm0				; 015B _ 66: 0F EF. D8
	movdqu	xmm0, oword [esi]			; 015F _ F3: 0F 6F. 06
	pxor	xmm6, xmm7				; 0163 _ 66: 0F EF. F7
	pxor	xmm1, xmm2				; 0167 _ 66: 0F EF. CA
	movdqa	xmm7, xmm1				; 016B _ 66: 0F 6F. F9
	movdqu	xmm2, oword [esi+30H]			; 016F _ F3: 0F 6F. 56, 30
	pxor	xmm0, xmm5				; 0174 _ 66: 0F EF. C5
	movdqa	xmm5, xmm0				; 0178 _ 66: 0F 6F. E8
	pxor	xmm2, xmm3				; 017C _ 66: 0F EF. D3
	punpckldq xmm7, xmm2				; 0180 _ 66: 0F 62. FA
	punpckldq xmm5, xmm6				; 0184 _ 66: 0F 62. EE
	punpckhdq xmm0, xmm6				; 0188 _ 66: 0F 6A. C6
	movdqa	xmm6, xmm0				; 018C _ 66: 0F 6F. F0
	punpckhdq xmm1, xmm2				; 0190 _ 66: 0F 6A. CA
	movdqa	xmm2, xmm5				; 0194 _ 66: 0F 6F. D5
	punpckhqdq xmm5, xmm7				; 0198 _ 66: 0F 6D. EF
	punpcklqdq xmm6, xmm1				; 019C _ 66: 0F 6C. F1
	punpcklqdq xmm2, xmm7				; 01A0 _ 66: 0F 6C. D7
	movd	xmm7, dword [ecx+2A84H] 		; 01A4 _ 66: 0F 6E. B9, 00002A84
	punpckhqdq xmm0, xmm1				; 01AC _ 66: 0F 6D. C1
	movd	xmm1, dword [ecx+2A80H] 		; 01B0 _ 66: 0F 6E. 89, 00002A80
	pshufd	xmm1, xmm1, 0				; 01B8 _ 66: 0F 70. C9, 00
	pxor	xmm2, xmm1				; 01BD _ 66: 0F EF. D1
	pshufd	xmm1, xmm7, 0				; 01C1 _ 66: 0F 70. CF, 00
	pxor	xmm5, xmm1				; 01C6 _ 66: 0F EF. E9
	movd	xmm7, dword [ecx+2A88H] 		; 01CA _ 66: 0F 6E. B9, 00002A88
	pshufd	xmm1, xmm7, 0				; 01D2 _ 66: 0F 70. CF, 00
	movd	xmm7, dword [ecx+2A8CH] 		; 01D7 _ 66: 0F 6E. B9, 00002A8C
	pxor	xmm6, xmm1				; 01DF _ 66: 0F EF. F1
	pshufd	xmm1, xmm7, 0				; 01E3 _ 66: 0F 70. CF, 00
	movdqa	xmm7, xmm5				; 01E8 _ 66: 0F 6F. FD
	pxor	xmm0, xmm1				; 01EC _ 66: 0F EF. C1
	pxor	xmm0, xmm2				; 01F0 _ 66: 0F EF. C2
	pand	xmm7, xmm0				; 01F4 _ 66: 0F DB. F8
	pxor	xmm5, xmm6				; 01F8 _ 66: 0F EF. EE
	pxor	xmm7, xmm2				; 01FC _ 66: 0F EF. FA
	por	xmm2, xmm0				; 0200 _ 66: 0F EB. D0
	pxor	xmm2, xmm5				; 0204 _ 66: 0F EF. D5
	pxor	xmm5, xmm0				; 0208 _ 66: 0F EF. E8
	pxor	xmm0, xmm6				; 020C _ 66: 0F EF. C6
	por	xmm6, xmm7				; 0210 _ 66: 0F EB. F7
	pxor	xmm6, xmm5				; 0214 _ 66: 0F EF. F5
	pxor	xmm5, xmm4				; 0218 _ 66: 0F EF. EC
	por	xmm5, xmm7				; 021C _ 66: 0F EB. EF
	pxor	xmm7, xmm0				; 0220 _ 66: 0F EF. F8
	pxor	xmm7, xmm5				; 0224 _ 66: 0F EF. FD
	por	xmm0, xmm2				; 0228 _ 66: 0F EB. C2
	pxor	xmm7, xmm0				; 022C _ 66: 0F EF. F8
	movdqa	xmm1, xmm7				; 0230 _ 66: 0F 6F. CF
	pxor	xmm5, xmm0				; 0234 _ 66: 0F EF. E8
	psrld	xmm7, 19				; 0238 _ 66: 0F 72. D7, 13
	pslld	xmm1, 13				; 023D _ 66: 0F 72. F1, 0D
	por	xmm1, xmm7				; 0242 _ 66: 0F EB. CF
	movdqa	xmm7, xmm6				; 0246 _ 66: 0F 6F. FE
	psrld	xmm6, 29				; 024A _ 66: 0F 72. D6, 1D
	pxor	xmm5, xmm1				; 024F _ 66: 0F EF. E9
	pslld	xmm7, 3 				; 0253 _ 66: 0F 72. F7, 03
	por	xmm7, xmm6				; 0258 _ 66: 0F EB. FE
	movdqa	xmm6, xmm1				; 025C _ 66: 0F 6F. F1
	pxor	xmm5, xmm7				; 0260 _ 66: 0F EF. EF
	movdqa	xmm0, xmm5				; 0264 _ 66: 0F 6F. C5
	pxor	xmm2, xmm7				; 0268 _ 66: 0F EF. D7
	pslld	xmm6, 3 				; 026C _ 66: 0F 72. F6, 03
	pxor	xmm2, xmm6				; 0271 _ 66: 0F EF. D6
	movdqa	xmm6, xmm2				; 0275 _ 66: 0F 6F. F2
	pslld	xmm0, 1 				; 0279 _ 66: 0F 72. F0, 01
	psrld	xmm5, 31				; 027E _ 66: 0F 72. D5, 1F
	por	xmm0, xmm5				; 0283 _ 66: 0F EB. C5
	movdqa	xmm5, xmm0				; 0287 _ 66: 0F 6F. E8
	pslld	xmm6, 7 				; 028B _ 66: 0F 72. F6, 07
	psrld	xmm2, 25				; 0290 _ 66: 0F 72. D2, 19
	por	xmm6, xmm2				; 0295 _ 66: 0F EB. F2
	pxor	xmm1, xmm0				; 0299 _ 66: 0F EF. C8
	pxor	xmm1, xmm6				; 029D _ 66: 0F EF. CE
	pxor	xmm7, xmm6				; 02A1 _ 66: 0F EF. FE
	pslld	xmm5, 7 				; 02A5 _ 66: 0F 72. F5, 07
	pxor	xmm7, xmm5				; 02AA _ 66: 0F EF. FD
	movdqa	xmm5, xmm1				; 02AE _ 66: 0F 6F. E9
	movdqa	xmm2, xmm7				; 02B2 _ 66: 0F 6F. D7
	psrld	xmm1, 27				; 02B6 _ 66: 0F 72. D1, 1B
	pslld	xmm5, 5 				; 02BB _ 66: 0F 72. F5, 05
	por	xmm5, xmm1				; 02C0 _ 66: 0F EB. E9
	movd	xmm1, dword [ecx+2A90H] 		; 02C4 _ 66: 0F 6E. 89, 00002A90
	pslld	xmm2, 22				; 02CC _ 66: 0F 72. F2, 16
	psrld	xmm7, 10				; 02D1 _ 66: 0F 72. D7, 0A
	por	xmm2, xmm7				; 02D6 _ 66: 0F EB. D7
	pshufd	xmm7, xmm1, 0				; 02DA _ 66: 0F 70. F9, 00
	pxor	xmm5, xmm7				; 02DF _ 66: 0F EF. EF
	pxor	xmm5, xmm4				; 02E3 _ 66: 0F EF. EC
	movd	xmm1, dword [ecx+2A94H] 		; 02E7 _ 66: 0F 6E. 89, 00002A94
	pshufd	xmm7, xmm1, 0				; 02EF _ 66: 0F 70. F9, 00
	pxor	xmm0, xmm7				; 02F4 _ 66: 0F EF. C7
	movd	xmm1, dword [ecx+2A98H] 		; 02F8 _ 66: 0F 6E. 89, 00002A98
	pshufd	xmm7, xmm1, 0				; 0300 _ 66: 0F 70. F9, 00
	pxor	xmm2, xmm7				; 0305 _ 66: 0F EF. D7
	pxor	xmm2, xmm4				; 0309 _ 66: 0F EF. D4
	movd	xmm1, dword [ecx+2A9CH] 		; 030D _ 66: 0F 6E. 89, 00002A9C
	pshufd	xmm7, xmm1, 0				; 0315 _ 66: 0F 70. F9, 00
	movdqa	xmm1, xmm5				; 031A _ 66: 0F 6F. CD
	pxor	xmm6, xmm7				; 031E _ 66: 0F EF. F7
	pand	xmm1, xmm0				; 0322 _ 66: 0F DB. C8
	pxor	xmm2, xmm1				; 0326 _ 66: 0F EF. D1
	por	xmm1, xmm6				; 032A _ 66: 0F EB. CE
	pxor	xmm6, xmm2				; 032E _ 66: 0F EF. F2
	pxor	xmm0, xmm1				; 0332 _ 66: 0F EF. C1
	pxor	xmm1, xmm5				; 0336 _ 66: 0F EF. CD
	por	xmm5, xmm0				; 033A _ 66: 0F EB. E8
	pxor	xmm0, xmm6				; 033E _ 66: 0F EF. C6
	por	xmm2, xmm1				; 0342 _ 66: 0F EB. D1
	pand	xmm2, xmm5				; 0346 _ 66: 0F DB. D5
	pxor	xmm1, xmm0				; 034A _ 66: 0F EF. C8
	pand	xmm0, xmm2				; 034E _ 66: 0F DB. C2
	pxor	xmm0, xmm1				; 0352 _ 66: 0F EF. C1
	pand	xmm1, xmm2				; 0356 _ 66: 0F DB. CA
	pxor	xmm5, xmm1				; 035A _ 66: 0F EF. E9
	movdqa	xmm1, xmm2				; 035E _ 66: 0F 6F. CA
	psrld	xmm2, 19				; 0362 _ 66: 0F 72. D2, 13
	pslld	xmm1, 13				; 0367 _ 66: 0F 72. F1, 0D
	por	xmm1, xmm2				; 036C _ 66: 0F EB. CA
	movdqa	xmm2, xmm6				; 0370 _ 66: 0F 6F. D6
	psrld	xmm6, 29				; 0374 _ 66: 0F 72. D6, 1D
	pxor	xmm5, xmm1				; 0379 _ 66: 0F EF. E9
	pslld	xmm2, 3 				; 037D _ 66: 0F 72. F2, 03
	por	xmm2, xmm6				; 0382 _ 66: 0F EB. D6
	movdqa	xmm6, xmm1				; 0386 _ 66: 0F 6F. F1
	pxor	xmm5, xmm2				; 038A _ 66: 0F EF. EA
	pxor	xmm0, xmm2				; 038E _ 66: 0F EF. C2
	pslld	xmm6, 3 				; 0392 _ 66: 0F 72. F6, 03
	pxor	xmm0, xmm6				; 0397 _ 66: 0F EF. C6
	movdqa	xmm6, xmm5				; 039B _ 66: 0F 6F. F5
	psrld	xmm5, 31				; 039F _ 66: 0F 72. D5, 1F
	pslld	xmm6, 1 				; 03A4 _ 66: 0F 72. F6, 01
	por	xmm6, xmm5				; 03A9 _ 66: 0F EB. F5
	movdqa	xmm5, xmm0				; 03AD _ 66: 0F 6F. E8
	psrld	xmm0, 25				; 03B1 _ 66: 0F 72. D0, 19
	pxor	xmm1, xmm6				; 03B6 _ 66: 0F EF. CE
	pslld	xmm5, 7 				; 03BA _ 66: 0F 72. F5, 07
	por	xmm5, xmm0				; 03BF _ 66: 0F EB. E8
	movdqa	xmm0, xmm6				; 03C3 _ 66: 0F 6F. C6
	pxor	xmm1, xmm5				; 03C7 _ 66: 0F EF. CD
	movdqa	xmm7, xmm1				; 03CB _ 66: 0F 6F. F9
	pxor	xmm2, xmm5				; 03CF _ 66: 0F EF. D5
	pslld	xmm0, 7 				; 03D3 _ 66: 0F 72. F0, 07
	pxor	xmm2, xmm0				; 03D8 _ 66: 0F EF. D0
	pslld	xmm7, 5 				; 03DC _ 66: 0F 72. F7, 05
	psrld	xmm1, 27				; 03E1 _ 66: 0F 72. D1, 1B
	movd	xmm0, dword [ecx+2AA4H] 		; 03E6 _ 66: 0F 6E. 81, 00002AA4
	por	xmm7, xmm1				; 03EE _ 66: 0F EB. F9
	movdqa	xmm1, xmm2				; 03F2 _ 66: 0F 6F. CA
	psrld	xmm2, 10				; 03F6 _ 66: 0F 72. D2, 0A
	pslld	xmm1, 22				; 03FB _ 66: 0F 72. F1, 16
	por	xmm1, xmm2				; 0400 _ 66: 0F EB. CA
	movd	xmm2, dword [ecx+2AA0H] 		; 0404 _ 66: 0F 6E. 91, 00002AA0
	pshufd	xmm2, xmm2, 0				; 040C _ 66: 0F 70. D2, 00
	pxor	xmm7, xmm2				; 0411 _ 66: 0F EF. FA
	pshufd	xmm2, xmm0, 0				; 0415 _ 66: 0F 70. D0, 00
	pxor	xmm6, xmm2				; 041A _ 66: 0F EF. F2
	movd	xmm0, dword [ecx+2AA8H] 		; 041E _ 66: 0F 6E. 81, 00002AA8
	pshufd	xmm2, xmm0, 0				; 0426 _ 66: 0F 70. D0, 00
	pxor	xmm1, xmm2				; 042B _ 66: 0F EF. CA
	movd	xmm0, dword [ecx+2AACH] 		; 042F _ 66: 0F 6E. 81, 00002AAC
	pshufd	xmm2, xmm0, 0				; 0437 _ 66: 0F 70. D0, 00
	movdqa	xmm0, xmm7				; 043C _ 66: 0F 6F. C7
	pxor	xmm5, xmm2				; 0440 _ 66: 0F EF. EA
	pand	xmm0, xmm1				; 0444 _ 66: 0F DB. C1
	pxor	xmm0, xmm5				; 0448 _ 66: 0F EF. C5
	pxor	xmm1, xmm6				; 044C _ 66: 0F EF. CE
	pxor	xmm1, xmm0				; 0450 _ 66: 0F EF. C8
	por	xmm5, xmm7				; 0454 _ 66: 0F EB. EF
	pxor	xmm5, xmm6				; 0458 _ 66: 0F EF. EE
	movdqa	xmm2, xmm5				; 045C _ 66: 0F 6F. D5
	movdqa	xmm6, xmm1				; 0460 _ 66: 0F 6F. F1
	pxor	xmm7, xmm1				; 0464 _ 66: 0F EF. F9
	por	xmm2, xmm7				; 0468 _ 66: 0F EB. D7
	pxor	xmm2, xmm0				; 046C _ 66: 0F EF. D0
	pand	xmm0, xmm5				; 0470 _ 66: 0F DB. C5
	pxor	xmm7, xmm0				; 0474 _ 66: 0F EF. F8
	pxor	xmm5, xmm2				; 0478 _ 66: 0F EF. EA
	pxor	xmm5, xmm7				; 047C _ 66: 0F EF. EF
	movdqa	xmm0, xmm5				; 0480 _ 66: 0F 6F. C5
	pxor	xmm7, xmm4				; 0484 _ 66: 0F EF. FC
	pslld	xmm6, 13				; 0488 _ 66: 0F 72. F6, 0D
	psrld	xmm1, 19				; 048D _ 66: 0F 72. D1, 13
	por	xmm6, xmm1				; 0492 _ 66: 0F EB. F1
	pslld	xmm0, 3 				; 0496 _ 66: 0F 72. F0, 03
	psrld	xmm5, 29				; 049B _ 66: 0F 72. D5, 1D
	por	xmm0, xmm5				; 04A0 _ 66: 0F EB. C5
	movdqa	xmm5, xmm6				; 04A4 _ 66: 0F 6F. EE
	pxor	xmm2, xmm6				; 04A8 _ 66: 0F EF. D6
	pxor	xmm2, xmm0				; 04AC _ 66: 0F EF. D0
	movdqa	xmm1, xmm2				; 04B0 _ 66: 0F 6F. CA
	pxor	xmm7, xmm0				; 04B4 _ 66: 0F EF. F8
	pslld	xmm5, 3 				; 04B8 _ 66: 0F 72. F5, 03
	pxor	xmm7, xmm5				; 04BD _ 66: 0F EF. FD
	movdqa	xmm5, xmm7				; 04C1 _ 66: 0F 6F. EF
	pslld	xmm1, 1 				; 04C5 _ 66: 0F 72. F1, 01
	psrld	xmm2, 31				; 04CA _ 66: 0F 72. D2, 1F
	por	xmm1, xmm2				; 04CF _ 66: 0F EB. CA
	pslld	xmm5, 7 				; 04D3 _ 66: 0F 72. F5, 07
	psrld	xmm7, 25				; 04D8 _ 66: 0F 72. D7, 19
	por	xmm5, xmm7				; 04DD _ 66: 0F EB. EF
	movdqa	xmm7, xmm1				; 04E1 _ 66: 0F 6F. F9
	pxor	xmm6, xmm1				; 04E5 _ 66: 0F EF. F1
	pxor	xmm6, xmm5				; 04E9 _ 66: 0F EF. F5
	movdqa	xmm2, xmm6				; 04ED _ 66: 0F 6F. D6
	pxor	xmm0, xmm5				; 04F1 _ 66: 0F EF. C5
	pslld	xmm7, 7 				; 04F5 _ 66: 0F 72. F7, 07
	pxor	xmm0, xmm7				; 04FA _ 66: 0F EF. C7
	pslld	xmm2, 5 				; 04FE _ 66: 0F 72. F2, 05
	psrld	xmm6, 27				; 0503 _ 66: 0F 72. D6, 1B
	movd	xmm7, dword [ecx+2AB0H] 		; 0508 _ 66: 0F 6E. B9, 00002AB0
	por	xmm2, xmm6				; 0510 _ 66: 0F EB. D6
	movdqa	xmm6, xmm0				; 0514 _ 66: 0F 6F. F0
	psrld	xmm0, 10				; 0518 _ 66: 0F 72. D0, 0A
	pslld	xmm6, 22				; 051D _ 66: 0F 72. F6, 16
	por	xmm6, xmm0				; 0522 _ 66: 0F EB. F0
	pshufd	xmm0, xmm7, 0				; 0526 _ 66: 0F 70. C7, 00
	movd	xmm7, dword [ecx+2AB4H] 		; 052B _ 66: 0F 6E. B9, 00002AB4
	pxor	xmm2, xmm0				; 0533 _ 66: 0F EF. D0
	pshufd	xmm0, xmm7, 0				; 0537 _ 66: 0F 70. C7, 00
	pxor	xmm1, xmm0				; 053C _ 66: 0F EF. C8
	movd	xmm7, dword [ecx+2AB8H] 		; 0540 _ 66: 0F 6E. B9, 00002AB8
	pshufd	xmm0, xmm7, 0				; 0548 _ 66: 0F 70. C7, 00
	pxor	xmm6, xmm0				; 054D _ 66: 0F EF. F0
	movd	xmm7, dword [ecx+2ABCH] 		; 0551 _ 66: 0F 6E. B9, 00002ABC
	pshufd	xmm0, xmm7, 0				; 0559 _ 66: 0F 70. C7, 00
	pxor	xmm5, xmm0				; 055E _ 66: 0F EF. E8
	movdqa	xmm0, xmm2				; 0562 _ 66: 0F 6F. C2
	por	xmm0, xmm5				; 0566 _ 66: 0F EB. C5
	pxor	xmm5, xmm1				; 056A _ 66: 0F EF. E9
	pand	xmm1, xmm2				; 056E _ 66: 0F DB. CA
	pxor	xmm2, xmm6				; 0572 _ 66: 0F EF. D6
	pxor	xmm6, xmm5				; 0576 _ 66: 0F EF. F5
	pand	xmm5, xmm0				; 057A _ 66: 0F DB. E8
	por	xmm2, xmm1				; 057E _ 66: 0F EB. D1
	pxor	xmm5, xmm2				; 0582 _ 66: 0F EF. EA
	pxor	xmm0, xmm1				; 0586 _ 66: 0F EF. C1
	pand	xmm2, xmm0				; 058A _ 66: 0F DB. D0
	pxor	xmm1, xmm5				; 058E _ 66: 0F EF. CD
	pxor	xmm2, xmm6				; 0592 _ 66: 0F EF. D6
	por	xmm1, xmm0				; 0596 _ 66: 0F EB. C8
	pxor	xmm1, xmm6				; 059A _ 66: 0F EF. CE
	movdqa	xmm6, xmm1				; 059E _ 66: 0F 6F. F1
	pxor	xmm0, xmm5				; 05A2 _ 66: 0F EF. C5
	por	xmm6, xmm5				; 05A6 _ 66: 0F EB. F5
	pxor	xmm0, xmm6				; 05AA _ 66: 0F EF. C6
	movdqa	xmm7, xmm0				; 05AE _ 66: 0F 6F. F8
	psrld	xmm0, 19				; 05B2 _ 66: 0F 72. D0, 13
	pslld	xmm7, 13				; 05B7 _ 66: 0F 72. F7, 0D
	por	xmm7, xmm0				; 05BC _ 66: 0F EB. F8
	movdqa	xmm0, xmm5				; 05C0 _ 66: 0F 6F. C5
	psrld	xmm5, 29				; 05C4 _ 66: 0F 72. D5, 1D
	pxor	xmm1, xmm7				; 05C9 _ 66: 0F EF. CF
	pslld	xmm0, 3 				; 05CD _ 66: 0F 72. F0, 03
	por	xmm0, xmm5				; 05D2 _ 66: 0F EB. C5
	movdqa	xmm5, xmm7				; 05D6 _ 66: 0F 6F. EF
	pxor	xmm1, xmm0				; 05DA _ 66: 0F EF. C8
	movdqa	xmm6, xmm1				; 05DE _ 66: 0F 6F. F1
	pxor	xmm2, xmm0				; 05E2 _ 66: 0F EF. D0
	pslld	xmm5, 3 				; 05E6 _ 66: 0F 72. F5, 03
	pxor	xmm2, xmm5				; 05EB _ 66: 0F EF. D5
	movdqa	xmm5, xmm2				; 05EF _ 66: 0F 6F. EA
	pslld	xmm6, 1 				; 05F3 _ 66: 0F 72. F6, 01
	psrld	xmm1, 31				; 05F8 _ 66: 0F 72. D1, 1F
	por	xmm6, xmm1				; 05FD _ 66: 0F EB. F1
	movdqa	xmm1, xmm6				; 0601 _ 66: 0F 6F. CE
	pslld	xmm5, 7 				; 0605 _ 66: 0F 72. F5, 07
	psrld	xmm2, 25				; 060A _ 66: 0F 72. D2, 19
	por	xmm5, xmm2				; 060F _ 66: 0F EB. EA
	pxor	xmm7, xmm6				; 0613 _ 66: 0F EF. FE
	pxor	xmm7, xmm5				; 0617 _ 66: 0F EF. FD
	pxor	xmm0, xmm5				; 061B _ 66: 0F EF. C5
	pslld	xmm1, 7 				; 061F _ 66: 0F 72. F1, 07
	pxor	xmm0, xmm1				; 0624 _ 66: 0F EF. C1
	movdqa	xmm1, xmm7				; 0628 _ 66: 0F 6F. CF
	movdqa	xmm2, xmm0				; 062C _ 66: 0F 6F. D0
	psrld	xmm7, 27				; 0630 _ 66: 0F 72. D7, 1B
	pslld	xmm1, 5 				; 0635 _ 66: 0F 72. F1, 05
	por	xmm1, xmm7				; 063A _ 66: 0F EB. CF
	pslld	xmm2, 22				; 063E _ 66: 0F 72. F2, 16
	movd	xmm7, dword [ecx+2AC0H] 		; 0643 _ 66: 0F 6E. B9, 00002AC0
	psrld	xmm0, 10				; 064B _ 66: 0F 72. D0, 0A
	por	xmm2, xmm0				; 0650 _ 66: 0F EB. D0
	pshufd	xmm0, xmm7, 0				; 0654 _ 66: 0F 70. C7, 00
	pxor	xmm1, xmm0				; 0659 _ 66: 0F EF. C8
	movd	xmm7, dword [ecx+2AC4H] 		; 065D _ 66: 0F 6E. B9, 00002AC4
	pshufd	xmm0, xmm7, 0				; 0665 _ 66: 0F 70. C7, 00
	pxor	xmm6, xmm0				; 066A _ 66: 0F EF. F0
	movd	xmm7, dword [ecx+2AC8H] 		; 066E _ 66: 0F 6E. B9, 00002AC8
	pshufd	xmm0, xmm7, 0				; 0676 _ 66: 0F 70. C7, 00
	pxor	xmm2, xmm0				; 067B _ 66: 0F EF. D0
	movd	xmm7, dword [ecx+2ACCH] 		; 067F _ 66: 0F 6E. B9, 00002ACC
	pshufd	xmm0, xmm7, 0				; 0687 _ 66: 0F 70. C7, 00
	pxor	xmm5, xmm0				; 068C _ 66: 0F EF. E8
	pxor	xmm6, xmm5				; 0690 _ 66: 0F EF. F5
	movdqa	xmm0, xmm6				; 0694 _ 66: 0F 6F. C6
	pxor	xmm5, xmm4				; 0698 _ 66: 0F EF. EC
	pxor	xmm2, xmm5				; 069C _ 66: 0F EF. D5
	pxor	xmm5, xmm1				; 06A0 _ 66: 0F EF. E9
	pand	xmm0, xmm5				; 06A4 _ 66: 0F DB. C5
	pxor	xmm0, xmm2				; 06A8 _ 66: 0F EF. C2
	movdqa	xmm7, xmm0				; 06AC _ 66: 0F 6F. F8
	pxor	xmm6, xmm5				; 06B0 _ 66: 0F EF. F5
	pxor	xmm1, xmm6				; 06B4 _ 66: 0F EF. CE
	pand	xmm2, xmm6				; 06B8 _ 66: 0F DB. D6
	pxor	xmm2, xmm1				; 06BC _ 66: 0F EF. D1
	pand	xmm1, xmm0				; 06C0 _ 66: 0F DB. C8
	pxor	xmm5, xmm1				; 06C4 _ 66: 0F EF. E9
	por	xmm6, xmm0				; 06C8 _ 66: 0F EB. F0
	pxor	xmm6, xmm1				; 06CC _ 66: 0F EF. F1
	por	xmm1, xmm5				; 06D0 _ 66: 0F EB. CD
	pxor	xmm1, xmm2				; 06D4 _ 66: 0F EF. CA
	pand	xmm2, xmm5				; 06D8 _ 66: 0F DB. D5
	pxor	xmm1, xmm4				; 06DC _ 66: 0F EF. CC
	pxor	xmm6, xmm2				; 06E0 _ 66: 0F EF. F2
	movdqa	xmm2, xmm1				; 06E4 _ 66: 0F 6F. D1
	pslld	xmm7, 13				; 06E8 _ 66: 0F 72. F7, 0D
	psrld	xmm0, 19				; 06ED _ 66: 0F 72. D0, 13
	por	xmm7, xmm0				; 06F2 _ 66: 0F EB. F8
	pslld	xmm2, 3 				; 06F6 _ 66: 0F 72. F2, 03
	psrld	xmm1, 29				; 06FB _ 66: 0F 72. D1, 1D
	por	xmm2, xmm1				; 0700 _ 66: 0F EB. D1
	movdqa	xmm1, xmm7				; 0704 _ 66: 0F 6F. CF
	pxor	xmm6, xmm7				; 0708 _ 66: 0F EF. F7
	pxor	xmm6, xmm2				; 070C _ 66: 0F EF. F2
	pxor	xmm5, xmm2				; 0710 _ 66: 0F EF. EA
	pslld	xmm1, 3 				; 0714 _ 66: 0F 72. F1, 03
	pxor	xmm5, xmm1				; 0719 _ 66: 0F EF. E9
	movdqa	xmm1, xmm6				; 071D _ 66: 0F 6F. CE
	psrld	xmm6, 31				; 0721 _ 66: 0F 72. D6, 1F
	pslld	xmm1, 1 				; 0726 _ 66: 0F 72. F1, 01
	por	xmm1, xmm6				; 072B _ 66: 0F EB. CE
	movdqa	xmm6, xmm5				; 072F _ 66: 0F 6F. F5
	psrld	xmm5, 25				; 0733 _ 66: 0F 72. D5, 19
	pxor	xmm7, xmm1				; 0738 _ 66: 0F EF. F9
	pslld	xmm6, 7 				; 073C _ 66: 0F 72. F6, 07
	por	xmm6, xmm5				; 0741 _ 66: 0F EB. F5
	movdqa	xmm5, xmm1				; 0745 _ 66: 0F 6F. E9
	pxor	xmm7, xmm6				; 0749 _ 66: 0F EF. FE
	movdqa	xmm0, xmm7				; 074D _ 66: 0F 6F. C7
	pxor	xmm2, xmm6				; 0751 _ 66: 0F EF. D6
	pslld	xmm5, 7 				; 0755 _ 66: 0F 72. F5, 07
	pxor	xmm2, xmm5				; 075A _ 66: 0F EF. D5
	pslld	xmm0, 5 				; 075E _ 66: 0F 72. F0, 05
	psrld	xmm7, 27				; 0763 _ 66: 0F 72. D7, 1B
	por	xmm0, xmm7				; 0768 _ 66: 0F EB. C7
	movdqa	xmm7, xmm2				; 076C _ 66: 0F 6F. FA
	psrld	xmm2, 10				; 0770 _ 66: 0F 72. D2, 0A
	pslld	xmm7, 22				; 0775 _ 66: 0F 72. F7, 16
	por	xmm7, xmm2				; 077A _ 66: 0F EB. FA
	movd	xmm2, dword [ecx+2AD0H] 		; 077E _ 66: 0F 6E. 91, 00002AD0
	pshufd	xmm5, xmm2, 0				; 0786 _ 66: 0F 70. EA, 00
	pxor	xmm0, xmm5				; 078B _ 66: 0F EF. C5
	movd	xmm2, dword [ecx+2AD4H] 		; 078F _ 66: 0F 6E. 91, 00002AD4
	pshufd	xmm5, xmm2, 0				; 0797 _ 66: 0F 70. EA, 00
	pxor	xmm1, xmm5				; 079C _ 66: 0F EF. CD
	pxor	xmm0, xmm1				; 07A0 _ 66: 0F EF. C1
	movd	xmm2, dword [ecx+2AD8H] 		; 07A4 _ 66: 0F 6E. 91, 00002AD8
	pshufd	xmm5, xmm2, 0				; 07AC _ 66: 0F 70. EA, 00
	movd	xmm2, dword [ecx+2ADCH] 		; 07B1 _ 66: 0F 6E. 91, 00002ADC
	pxor	xmm7, xmm5				; 07B9 _ 66: 0F EF. FD
	pshufd	xmm5, xmm2, 0				; 07BD _ 66: 0F 70. EA, 00
	pxor	xmm6, xmm5				; 07C2 _ 66: 0F EF. F5
	pxor	xmm1, xmm6				; 07C6 _ 66: 0F EF. CE
	movdqa	xmm2, xmm1				; 07CA _ 66: 0F 6F. D1
	pxor	xmm6, xmm4				; 07CE _ 66: 0F EF. F4
	pxor	xmm7, xmm6				; 07D2 _ 66: 0F EF. FE
	pand	xmm2, xmm0				; 07D6 _ 66: 0F DB. D0
	pxor	xmm2, xmm7				; 07DA _ 66: 0F EF. D7
	movdqa	xmm5, xmm2				; 07DE _ 66: 0F 6F. EA
	por	xmm7, xmm1				; 07E2 _ 66: 0F EB. F9
	pxor	xmm1, xmm6				; 07E6 _ 66: 0F EF. CE
	pand	xmm6, xmm2				; 07EA _ 66: 0F DB. F2
	pxor	xmm6, xmm0				; 07EE _ 66: 0F EF. F0
	pxor	xmm1, xmm2				; 07F2 _ 66: 0F EF. CA
	pxor	xmm1, xmm7				; 07F6 _ 66: 0F EF. CF
	pxor	xmm7, xmm0				; 07FA _ 66: 0F EF. F8
	pand	xmm0, xmm6				; 07FE _ 66: 0F DB. C6
	pxor	xmm7, xmm4				; 0802 _ 66: 0F EF. FC
	pxor	xmm0, xmm1				; 0806 _ 66: 0F EF. C1
	por	xmm1, xmm6				; 080A _ 66: 0F EB. CE
	pxor	xmm1, xmm7				; 080E _ 66: 0F EF. CF
	pslld	xmm5, 13				; 0812 _ 66: 0F 72. F5, 0D
	psrld	xmm2, 19				; 0817 _ 66: 0F 72. D2, 13
	por	xmm5, xmm2				; 081C _ 66: 0F EB. EA
	movdqa	xmm2, xmm0				; 0820 _ 66: 0F 6F. D0
	movdqa	xmm7, xmm5				; 0824 _ 66: 0F 6F. FD
	psrld	xmm0, 29				; 0828 _ 66: 0F 72. D0, 1D
	pslld	xmm2, 3 				; 082D _ 66: 0F 72. F2, 03
	por	xmm2, xmm0				; 0832 _ 66: 0F EB. D0
	pxor	xmm6, xmm5				; 0836 _ 66: 0F EF. F5
	pxor	xmm6, xmm2				; 083A _ 66: 0F EF. F2
	movdqa	xmm0, xmm6				; 083E _ 66: 0F 6F. C6
	pxor	xmm1, xmm2				; 0842 _ 66: 0F EF. CA
	pslld	xmm7, 3 				; 0846 _ 66: 0F 72. F7, 03
	pxor	xmm1, xmm7				; 084B _ 66: 0F EF. CF
	pslld	xmm0, 1 				; 084F _ 66: 0F 72. F0, 01
	psrld	xmm6, 31				; 0854 _ 66: 0F 72. D6, 1F
	por	xmm0, xmm6				; 0859 _ 66: 0F EB. C6
	movdqa	xmm6, xmm1				; 085D _ 66: 0F 6F. F1
	psrld	xmm1, 25				; 0861 _ 66: 0F 72. D1, 19
	pxor	xmm5, xmm0				; 0866 _ 66: 0F EF. E8
	pslld	xmm6, 7 				; 086A _ 66: 0F 72. F6, 07
	por	xmm6, xmm1				; 086F _ 66: 0F EB. F1
	movdqa	xmm7, xmm0				; 0873 _ 66: 0F 6F. F8
	pxor	xmm5, xmm6				; 0877 _ 66: 0F EF. EE
	pxor	xmm2, xmm6				; 087B _ 66: 0F EF. D6
	pslld	xmm7, 7 				; 087F _ 66: 0F 72. F7, 07
	pxor	xmm2, xmm7				; 0884 _ 66: 0F EF. D7
	movdqa	xmm7, xmm5				; 0888 _ 66: 0F 6F. FD
	movdqa	xmm1, xmm2				; 088C _ 66: 0F 6F. CA
	psrld	xmm5, 27				; 0890 _ 66: 0F 72. D5, 1B
	pslld	xmm7, 5 				; 0895 _ 66: 0F 72. F7, 05
	por	xmm7, xmm5				; 089A _ 66: 0F EB. FD
	pslld	xmm1, 22				; 089E _ 66: 0F 72. F1, 16
	movd	xmm5, dword [ecx+2AE0H] 		; 08A3 _ 66: 0F 6E. A9, 00002AE0
	psrld	xmm2, 10				; 08AB _ 66: 0F 72. D2, 0A
	por	xmm1, xmm2				; 08B0 _ 66: 0F EB. CA
	pshufd	xmm2, xmm5, 0				; 08B4 _ 66: 0F 70. D5, 00
	pxor	xmm7, xmm2				; 08B9 _ 66: 0F EF. FA
	movd	xmm5, dword [ecx+2AE4H] 		; 08BD _ 66: 0F 6E. A9, 00002AE4
	pshufd	xmm2, xmm5, 0				; 08C5 _ 66: 0F 70. D5, 00
	pxor	xmm0, xmm2				; 08CA _ 66: 0F EF. C2
	movd	xmm5, dword [ecx+2AE8H] 		; 08CE _ 66: 0F 6E. A9, 00002AE8
	pshufd	xmm2, xmm5, 0				; 08D6 _ 66: 0F 70. D5, 00
	pxor	xmm1, xmm2				; 08DB _ 66: 0F EF. CA
	pxor	xmm1, xmm4				; 08DF _ 66: 0F EF. CC
	movd	xmm5, dword [ecx+2AECH] 		; 08E3 _ 66: 0F 6E. A9, 00002AEC
	pshufd	xmm2, xmm5, 0				; 08EB _ 66: 0F 70. D5, 00
	pxor	xmm6, xmm2				; 08F0 _ 66: 0F EF. F2
	movdqa	xmm2, xmm6				; 08F4 _ 66: 0F 6F. D6
	pand	xmm2, xmm7				; 08F8 _ 66: 0F DB. D7
	pxor	xmm7, xmm6				; 08FC _ 66: 0F EF. FE
	pxor	xmm2, xmm1				; 0900 _ 66: 0F EF. D1
	por	xmm1, xmm6				; 0904 _ 66: 0F EB. CE
	pxor	xmm0, xmm2				; 0908 _ 66: 0F EF. C2
	pxor	xmm1, xmm7				; 090C _ 66: 0F EF. CF
	por	xmm7, xmm0				; 0910 _ 66: 0F EB. F8
	pxor	xmm1, xmm0				; 0914 _ 66: 0F EF. C8
	pxor	xmm6, xmm7				; 0918 _ 66: 0F EF. F7
	por	xmm7, xmm2				; 091C _ 66: 0F EB. FA
	pxor	xmm7, xmm1				; 0920 _ 66: 0F EF. F9
	movdqa	xmm5, xmm7				; 0924 _ 66: 0F 6F. EF
	pxor	xmm6, xmm2				; 0928 _ 66: 0F EF. F2
	pxor	xmm6, xmm7				; 092C _ 66: 0F EF. F7
	pxor	xmm2, xmm4				; 0930 _ 66: 0F EF. D4
	pand	xmm1, xmm6				; 0934 _ 66: 0F DB. CE
	pxor	xmm2, xmm1				; 0938 _ 66: 0F EF. D1
	movdqa	xmm1, xmm6				; 093C _ 66: 0F 6F. CE
	pslld	xmm5, 13				; 0940 _ 66: 0F 72. F5, 0D
	psrld	xmm7, 19				; 0945 _ 66: 0F 72. D7, 13
	por	xmm5, xmm7				; 094A _ 66: 0F EB. EF
	pslld	xmm1, 3 				; 094E _ 66: 0F 72. F1, 03
	psrld	xmm6, 29				; 0953 _ 66: 0F 72. D6, 1D
	por	xmm1, xmm6				; 0958 _ 66: 0F EB. CE
	movdqa	xmm6, xmm5				; 095C _ 66: 0F 6F. F5
	pxor	xmm0, xmm5				; 0960 _ 66: 0F EF. C5
	pxor	xmm0, xmm1				; 0964 _ 66: 0F EF. C1
	pxor	xmm2, xmm1				; 0968 _ 66: 0F EF. D1
	pslld	xmm6, 3 				; 096C _ 66: 0F 72. F6, 03
	pxor	xmm2, xmm6				; 0971 _ 66: 0F EF. D6
	movdqa	xmm6, xmm0				; 0975 _ 66: 0F 6F. F0
	movdqa	xmm7, xmm2				; 0979 _ 66: 0F 6F. FA
	psrld	xmm0, 31				; 097D _ 66: 0F 72. D0, 1F
	pslld	xmm6, 1 				; 0982 _ 66: 0F 72. F6, 01
	por	xmm6, xmm0				; 0987 _ 66: 0F EB. F0
	movdqa	xmm0, xmm6				; 098B _ 66: 0F 6F. C6
	pslld	xmm7, 7 				; 098F _ 66: 0F 72. F7, 07
	psrld	xmm2, 25				; 0994 _ 66: 0F 72. D2, 19
	por	xmm7, xmm2				; 0999 _ 66: 0F EB. FA
	pxor	xmm5, xmm6				; 099D _ 66: 0F EF. EE
	pxor	xmm5, xmm7				; 09A1 _ 66: 0F EF. EF
	pxor	xmm1, xmm7				; 09A5 _ 66: 0F EF. CF
	movd	xmm2, dword [ecx+2AF4H] 		; 09A9 _ 66: 0F 6E. 91, 00002AF4
	pslld	xmm0, 7 				; 09B1 _ 66: 0F 72. F0, 07
	pxor	xmm1, xmm0				; 09B6 _ 66: 0F EF. C8
	movdqa	xmm0, xmm5				; 09BA _ 66: 0F 6F. C5
	psrld	xmm5, 27				; 09BE _ 66: 0F 72. D5, 1B
	pslld	xmm0, 5 				; 09C3 _ 66: 0F 72. F0, 05
	por	xmm0, xmm5				; 09C8 _ 66: 0F EB. C5
	movdqa	xmm5, xmm1				; 09CC _ 66: 0F 6F. E9
	psrld	xmm1, 10				; 09D0 _ 66: 0F 72. D1, 0A
	pslld	xmm5, 22				; 09D5 _ 66: 0F 72. F5, 16
	por	xmm5, xmm1				; 09DA _ 66: 0F EB. E9
	movd	xmm1, dword [ecx+2AF0H] 		; 09DE _ 66: 0F 6E. 89, 00002AF0
	pshufd	xmm1, xmm1, 0				; 09E6 _ 66: 0F 70. C9, 00
	pxor	xmm0, xmm1				; 09EB _ 66: 0F EF. C1
	pshufd	xmm1, xmm2, 0				; 09EF _ 66: 0F 70. CA, 00
	pxor	xmm6, xmm1				; 09F4 _ 66: 0F EF. F1
	movd	xmm2, dword [ecx+2AF8H] 		; 09F8 _ 66: 0F 6E. 91, 00002AF8
	pshufd	xmm1, xmm2, 0				; 0A00 _ 66: 0F 70. CA, 00
	pxor	xmm5, xmm1				; 0A05 _ 66: 0F EF. E9
	movd	xmm2, dword [ecx+2AFCH] 		; 0A09 _ 66: 0F 6E. 91, 00002AFC
	pshufd	xmm1, xmm2, 0				; 0A11 _ 66: 0F 70. CA, 00
	pxor	xmm7, xmm1				; 0A16 _ 66: 0F EF. F9
	movdqa	xmm1, xmm6				; 0A1A _ 66: 0F 6F. CE
	pxor	xmm6, xmm5				; 0A1E _ 66: 0F EF. F5
	por	xmm1, xmm5				; 0A22 _ 66: 0F EB. CD
	pxor	xmm1, xmm7				; 0A26 _ 66: 0F EF. CF
	pxor	xmm5, xmm1				; 0A2A _ 66: 0F EF. E9
	por	xmm7, xmm6				; 0A2E _ 66: 0F EB. FE
	pand	xmm7, xmm0				; 0A32 _ 66: 0F DB. F8
	pxor	xmm6, xmm5				; 0A36 _ 66: 0F EF. F5
	pxor	xmm7, xmm1				; 0A3A _ 66: 0F EF. F9
	por	xmm1, xmm6				; 0A3E _ 66: 0F EB. CE
	pxor	xmm1, xmm0				; 0A42 _ 66: 0F EF. C8
	por	xmm0, xmm6				; 0A46 _ 66: 0F EB. C6
	pxor	xmm0, xmm5				; 0A4A _ 66: 0F EF. C5
	pxor	xmm1, xmm6				; 0A4E _ 66: 0F EF. CE
	pxor	xmm5, xmm1				; 0A52 _ 66: 0F EF. E9
	pand	xmm1, xmm0				; 0A56 _ 66: 0F DB. C8
	pxor	xmm1, xmm6				; 0A5A _ 66: 0F EF. CE
	pxor	xmm5, xmm4				; 0A5E _ 66: 0F EF. EC
	por	xmm5, xmm0				; 0A62 _ 66: 0F EB. E8
	pxor	xmm6, xmm5				; 0A66 _ 66: 0F EF. F5
	movdqa	xmm2, xmm6				; 0A6A _ 66: 0F 6F. D6
	psrld	xmm6, 19				; 0A6E _ 66: 0F 72. D6, 13
	pslld	xmm2, 13				; 0A73 _ 66: 0F 72. F2, 0D
	por	xmm2, xmm6				; 0A78 _ 66: 0F EB. D6
	movdqa	xmm6, xmm1				; 0A7C _ 66: 0F 6F. F1
	movdqa	xmm5, xmm2				; 0A80 _ 66: 0F 6F. EA
	psrld	xmm1, 29				; 0A84 _ 66: 0F 72. D1, 1D
	pslld	xmm6, 3 				; 0A89 _ 66: 0F 72. F6, 03
	por	xmm6, xmm1				; 0A8E _ 66: 0F EB. F1
	pxor	xmm7, xmm2				; 0A92 _ 66: 0F EF. FA
	pxor	xmm7, xmm6				; 0A96 _ 66: 0F EF. FE
	movdqa	xmm1, xmm7				; 0A9A _ 66: 0F 6F. CF
	pxor	xmm0, xmm6				; 0A9E _ 66: 0F EF. C6
	pslld	xmm5, 3 				; 0AA2 _ 66: 0F 72. F5, 03
	pxor	xmm0, xmm5				; 0AA7 _ 66: 0F EF. C5
	movdqa	xmm5, xmm0				; 0AAB _ 66: 0F 6F. E8
	pslld	xmm1, 1 				; 0AAF _ 66: 0F 72. F1, 01
	psrld	xmm7, 31				; 0AB4 _ 66: 0F 72. D7, 1F
	por	xmm1, xmm7				; 0AB9 _ 66: 0F EB. CF
	movdqa	xmm7, xmm1				; 0ABD _ 66: 0F 6F. F9
	pslld	xmm5, 7 				; 0AC1 _ 66: 0F 72. F5, 07
	psrld	xmm0, 25				; 0AC6 _ 66: 0F 72. D0, 19
	por	xmm5, xmm0				; 0ACB _ 66: 0F EB. E8
	pxor	xmm2, xmm1				; 0ACF _ 66: 0F EF. D1
	pxor	xmm2, xmm5				; 0AD3 _ 66: 0F EF. D5
	pxor	xmm6, xmm5				; 0AD7 _ 66: 0F EF. F5
	pslld	xmm7, 7 				; 0ADB _ 66: 0F 72. F7, 07
	pxor	xmm6, xmm7				; 0AE0 _ 66: 0F EF. F7
	movdqa	xmm7, xmm2				; 0AE4 _ 66: 0F 6F. FA
	movdqa	xmm0, xmm6				; 0AE8 _ 66: 0F 6F. C6
	psrld	xmm2, 27				; 0AEC _ 66: 0F 72. D2, 1B
	pslld	xmm7, 5 				; 0AF1 _ 66: 0F 72. F7, 05
	por	xmm7, xmm2				; 0AF6 _ 66: 0F EB. FA
	pslld	xmm0, 22				; 0AFA _ 66: 0F 72. F0, 16
	psrld	xmm6, 10				; 0AFF _ 66: 0F 72. D6, 0A
	por	xmm0, xmm6				; 0B04 _ 66: 0F EB. C6
	movd	xmm6, dword [ecx+2B00H] 		; 0B08 _ 66: 0F 6E. B1, 00002B00
	pshufd	xmm2, xmm6, 0				; 0B10 _ 66: 0F 70. D6, 00
	pxor	xmm7, xmm2				; 0B15 _ 66: 0F EF. FA
	movd	xmm6, dword [ecx+2B04H] 		; 0B19 _ 66: 0F 6E. B1, 00002B04
	pshufd	xmm2, xmm6, 0				; 0B21 _ 66: 0F 70. D6, 00
	pxor	xmm1, xmm2				; 0B26 _ 66: 0F EF. CA
	movd	xmm6, dword [ecx+2B08H] 		; 0B2A _ 66: 0F 6E. B1, 00002B08
	pshufd	xmm2, xmm6, 0				; 0B32 _ 66: 0F 70. D6, 00
	pxor	xmm0, xmm2				; 0B37 _ 66: 0F EF. C2
	movd	xmm6, dword [ecx+2B0CH] 		; 0B3B _ 66: 0F 6E. B1, 00002B0C
	pshufd	xmm2, xmm6, 0				; 0B43 _ 66: 0F 70. D6, 00
	pxor	xmm5, xmm2				; 0B48 _ 66: 0F EF. EA
	movdqa	xmm2, xmm1				; 0B4C _ 66: 0F 6F. D1
	pxor	xmm5, xmm7				; 0B50 _ 66: 0F EF. EF
	pxor	xmm1, xmm0				; 0B54 _ 66: 0F EF. C8
	pand	xmm2, xmm5				; 0B58 _ 66: 0F DB. D5
	pxor	xmm2, xmm7				; 0B5C _ 66: 0F EF. D7
	por	xmm7, xmm5				; 0B60 _ 66: 0F EB. FD
	pxor	xmm7, xmm1				; 0B64 _ 66: 0F EF. F9
	pxor	xmm1, xmm5				; 0B68 _ 66: 0F EF. CD
	pxor	xmm5, xmm0				; 0B6C _ 66: 0F EF. E8
	por	xmm0, xmm2				; 0B70 _ 66: 0F EB. C2
	pxor	xmm0, xmm1				; 0B74 _ 66: 0F EF. C1
	pxor	xmm1, xmm4				; 0B78 _ 66: 0F EF. CC
	por	xmm1, xmm2				; 0B7C _ 66: 0F EB. CA
	pxor	xmm2, xmm5				; 0B80 _ 66: 0F EF. D5
	pxor	xmm2, xmm1				; 0B84 _ 66: 0F EF. D1
	por	xmm5, xmm7				; 0B88 _ 66: 0F EB. EF
	pxor	xmm2, xmm5				; 0B8C _ 66: 0F EF. D5
	movdqa	xmm6, xmm2				; 0B90 _ 66: 0F 6F. F2
	pxor	xmm1, xmm5				; 0B94 _ 66: 0F EF. CD
	psrld	xmm2, 19				; 0B98 _ 66: 0F 72. D2, 13
	pslld	xmm6, 13				; 0B9D _ 66: 0F 72. F6, 0D
	por	xmm6, xmm2				; 0BA2 _ 66: 0F EB. F2
	movdqa	xmm2, xmm0				; 0BA6 _ 66: 0F 6F. D0
	psrld	xmm0, 29				; 0BAA _ 66: 0F 72. D0, 1D
	pxor	xmm1, xmm6				; 0BAF _ 66: 0F EF. CE
	pslld	xmm2, 3 				; 0BB3 _ 66: 0F 72. F2, 03
	por	xmm2, xmm0				; 0BB8 _ 66: 0F EB. D0
	movdqa	xmm0, xmm6				; 0BBC _ 66: 0F 6F. C6
	pxor	xmm1, xmm2				; 0BC0 _ 66: 0F EF. CA
	pxor	xmm7, xmm2				; 0BC4 _ 66: 0F EF. FA
	pslld	xmm0, 3 				; 0BC8 _ 66: 0F 72. F0, 03
	pxor	xmm7, xmm0				; 0BCD _ 66: 0F EF. F8
	movdqa	xmm0, xmm1				; 0BD1 _ 66: 0F 6F. C1
	movdqa	xmm5, xmm7				; 0BD5 _ 66: 0F 6F. EF
	psrld	xmm1, 31				; 0BD9 _ 66: 0F 72. D1, 1F
	pslld	xmm0, 1 				; 0BDE _ 66: 0F 72. F0, 01
	por	xmm0, xmm1				; 0BE3 _ 66: 0F EB. C1
	movdqa	xmm1, xmm0				; 0BE7 _ 66: 0F 6F. C8
	pslld	xmm5, 7 				; 0BEB _ 66: 0F 72. F5, 07
	psrld	xmm7, 25				; 0BF0 _ 66: 0F 72. D7, 19
	por	xmm5, xmm7				; 0BF5 _ 66: 0F EB. EF
	pxor	xmm6, xmm0				; 0BF9 _ 66: 0F EF. F0
	pxor	xmm6, xmm5				; 0BFD _ 66: 0F EF. F5
	pxor	xmm2, xmm5				; 0C01 _ 66: 0F EF. D5
	pslld	xmm1, 7 				; 0C05 _ 66: 0F 72. F1, 07
	pxor	xmm2, xmm1				; 0C0A _ 66: 0F EF. D1
	movdqa	xmm1, xmm6				; 0C0E _ 66: 0F 6F. CE
	movdqa	xmm7, xmm2				; 0C12 _ 66: 0F 6F. FA
	psrld	xmm6, 27				; 0C16 _ 66: 0F 72. D6, 1B
	pslld	xmm1, 5 				; 0C1B _ 66: 0F 72. F1, 05
	por	xmm1, xmm6				; 0C20 _ 66: 0F EB. CE
	pslld	xmm7, 22				; 0C24 _ 66: 0F 72. F7, 16
	psrld	xmm2, 10				; 0C29 _ 66: 0F 72. D2, 0A
	por	xmm7, xmm2				; 0C2E _ 66: 0F EB. FA
	movd	xmm6, dword [ecx+2B10H] 		; 0C32 _ 66: 0F 6E. B1, 00002B10
	pshufd	xmm2, xmm6, 0				; 0C3A _ 66: 0F 70. D6, 00
	pxor	xmm1, xmm2				; 0C3F _ 66: 0F EF. CA
	movd	xmm6, dword [ecx+2B14H] 		; 0C43 _ 66: 0F 6E. B1, 00002B14
	pshufd	xmm2, xmm6, 0				; 0C4B _ 66: 0F 70. D6, 00
	pxor	xmm0, xmm2				; 0C50 _ 66: 0F EF. C2
	pxor	xmm1, xmm4				; 0C54 _ 66: 0F EF. CC
	movd	xmm6, dword [ecx+2B18H] 		; 0C58 _ 66: 0F 6E. B1, 00002B18
	pshufd	xmm2, xmm6, 0				; 0C60 _ 66: 0F 70. D6, 00
	pxor	xmm7, xmm2				; 0C65 _ 66: 0F EF. FA
	pxor	xmm7, xmm4				; 0C69 _ 66: 0F EF. FC
	movd	xmm6, dword [ecx+2B1CH] 		; 0C6D _ 66: 0F 6E. B1, 00002B1C
	pshufd	xmm2, xmm6, 0				; 0C75 _ 66: 0F 70. D6, 00
	movdqa	xmm6, xmm1				; 0C7A _ 66: 0F 6F. F1
	pxor	xmm5, xmm2				; 0C7E _ 66: 0F EF. EA
	pand	xmm6, xmm0				; 0C82 _ 66: 0F DB. F0
	pxor	xmm7, xmm6				; 0C86 _ 66: 0F EF. FE
	por	xmm6, xmm5				; 0C8A _ 66: 0F EB. F5
	pxor	xmm5, xmm7				; 0C8E _ 66: 0F EF. EF
	pxor	xmm0, xmm6				; 0C92 _ 66: 0F EF. C6
	pxor	xmm6, xmm1				; 0C96 _ 66: 0F EF. F1
	por	xmm1, xmm0				; 0C9A _ 66: 0F EB. C8
	pxor	xmm0, xmm5				; 0C9E _ 66: 0F EF. C5
	por	xmm7, xmm6				; 0CA2 _ 66: 0F EB. FE
	pand	xmm7, xmm1				; 0CA6 _ 66: 0F DB. F9
	pxor	xmm6, xmm0				; 0CAA _ 66: 0F EF. F0
	pand	xmm0, xmm7				; 0CAE _ 66: 0F DB. C7
	pxor	xmm0, xmm6				; 0CB2 _ 66: 0F EF. C6
	pand	xmm6, xmm7				; 0CB6 _ 66: 0F DB. F7
	pxor	xmm1, xmm6				; 0CBA _ 66: 0F EF. CE
	movdqa	xmm6, xmm7				; 0CBE _ 66: 0F 6F. F7
	psrld	xmm7, 19				; 0CC2 _ 66: 0F 72. D7, 13
	pslld	xmm6, 13				; 0CC7 _ 66: 0F 72. F6, 0D
	por	xmm6, xmm7				; 0CCC _ 66: 0F EB. F7
	movdqa	xmm7, xmm5				; 0CD0 _ 66: 0F 6F. FD
	psrld	xmm5, 29				; 0CD4 _ 66: 0F 72. D5, 1D
	pxor	xmm1, xmm6				; 0CD9 _ 66: 0F EF. CE
	pslld	xmm7, 3 				; 0CDD _ 66: 0F 72. F7, 03
	por	xmm7, xmm5				; 0CE2 _ 66: 0F EB. FD
	movdqa	xmm5, xmm6				; 0CE6 _ 66: 0F 6F. EE
	pxor	xmm1, xmm7				; 0CEA _ 66: 0F EF. CF
	pxor	xmm0, xmm7				; 0CEE _ 66: 0F EF. C7
	pslld	xmm5, 3 				; 0CF2 _ 66: 0F 72. F5, 03
	pxor	xmm0, xmm5				; 0CF7 _ 66: 0F EF. C5
	movdqa	xmm5, xmm1				; 0CFB _ 66: 0F 6F. E9
	movdqa	xmm2, xmm0				; 0CFF _ 66: 0F 6F. D0
	psrld	xmm1, 31				; 0D03 _ 66: 0F 72. D1, 1F
	pslld	xmm5, 1 				; 0D08 _ 66: 0F 72. F5, 01
	por	xmm5, xmm1				; 0D0D _ 66: 0F EB. E9
	pslld	xmm2, 7 				; 0D11 _ 66: 0F 72. F2, 07
	psrld	xmm0, 25				; 0D16 _ 66: 0F 72. D0, 19
	por	xmm2, xmm0				; 0D1B _ 66: 0F EB. D0
	movdqa	xmm0, xmm5				; 0D1F _ 66: 0F 6F. C5
	pxor	xmm6, xmm5				; 0D23 _ 66: 0F EF. F5
	pxor	xmm6, xmm2				; 0D27 _ 66: 0F EF. F2
	movdqa	xmm1, xmm6				; 0D2B _ 66: 0F 6F. CE
	pxor	xmm7, xmm2				; 0D2F _ 66: 0F EF. FA
	pslld	xmm0, 7 				; 0D33 _ 66: 0F 72. F0, 07
	pxor	xmm7, xmm0				; 0D38 _ 66: 0F EF. F8
	movdqa	xmm0, xmm7				; 0D3C _ 66: 0F 6F. C7
	pslld	xmm1, 5 				; 0D40 _ 66: 0F 72. F1, 05
	psrld	xmm6, 27				; 0D45 _ 66: 0F 72. D6, 1B
	por	xmm1, xmm6				; 0D4A _ 66: 0F EB. CE
	pslld	xmm0, 22				; 0D4E _ 66: 0F 72. F0, 16
	psrld	xmm7, 10				; 0D53 _ 66: 0F 72. D7, 0A
	por	xmm0, xmm7				; 0D58 _ 66: 0F EB. C7
	movd	xmm7, dword [ecx+2B20H] 		; 0D5C _ 66: 0F 6E. B9, 00002B20
	movd	xmm6, dword [ecx+2B24H] 		; 0D64 _ 66: 0F 6E. B1, 00002B24
	pshufd	xmm7, xmm7, 0				; 0D6C _ 66: 0F 70. FF, 00
	pxor	xmm1, xmm7				; 0D71 _ 66: 0F EF. CF
	pshufd	xmm7, xmm6, 0				; 0D75 _ 66: 0F 70. FE, 00
	pxor	xmm5, xmm7				; 0D7A _ 66: 0F EF. EF
	movd	xmm6, dword [ecx+2B28H] 		; 0D7E _ 66: 0F 6E. B1, 00002B28
	pshufd	xmm7, xmm6, 0				; 0D86 _ 66: 0F 70. FE, 00
	pxor	xmm0, xmm7				; 0D8B _ 66: 0F EF. C7
	movd	xmm6, dword [ecx+2B2CH] 		; 0D8F _ 66: 0F 6E. B1, 00002B2C
	pshufd	xmm7, xmm6, 0				; 0D97 _ 66: 0F 70. FE, 00
	movdqa	xmm6, xmm1				; 0D9C _ 66: 0F 6F. F1
	pxor	xmm2, xmm7				; 0DA0 _ 66: 0F EF. D7
	pand	xmm6, xmm0				; 0DA4 _ 66: 0F DB. F0
	pxor	xmm6, xmm2				; 0DA8 _ 66: 0F EF. F2
	pxor	xmm0, xmm5				; 0DAC _ 66: 0F EF. C5
	pxor	xmm0, xmm6				; 0DB0 _ 66: 0F EF. C6
	por	xmm2, xmm1				; 0DB4 _ 66: 0F EB. D1
	pxor	xmm2, xmm5				; 0DB8 _ 66: 0F EF. D5
	movdqa	xmm7, xmm2				; 0DBC _ 66: 0F 6F. FA
	pxor	xmm1, xmm0				; 0DC0 _ 66: 0F EF. C8
	por	xmm7, xmm1				; 0DC4 _ 66: 0F EB. F9
	pxor	xmm7, xmm6				; 0DC8 _ 66: 0F EF. FE
	pand	xmm6, xmm2				; 0DCC _ 66: 0F DB. F2
	pxor	xmm1, xmm6				; 0DD0 _ 66: 0F EF. CE
	movdqa	xmm6, xmm0				; 0DD4 _ 66: 0F 6F. F0
	pxor	xmm2, xmm7				; 0DD8 _ 66: 0F EF. D7
	pxor	xmm2, xmm1				; 0DDC _ 66: 0F EF. D1
	movdqa	xmm5, xmm2				; 0DE0 _ 66: 0F 6F. EA
	pxor	xmm1, xmm4				; 0DE4 _ 66: 0F EF. CC
	pslld	xmm6, 13				; 0DE8 _ 66: 0F 72. F6, 0D
	psrld	xmm0, 19				; 0DED _ 66: 0F 72. D0, 13
	por	xmm6, xmm0				; 0DF2 _ 66: 0F EB. F0
	pslld	xmm5, 3 				; 0DF6 _ 66: 0F 72. F5, 03
	psrld	xmm2, 29				; 0DFB _ 66: 0F 72. D2, 1D
	por	xmm5, xmm2				; 0E00 _ 66: 0F EB. EA
	movdqa	xmm2, xmm6				; 0E04 _ 66: 0F 6F. D6
	pxor	xmm7, xmm6				; 0E08 _ 66: 0F EF. FE
	pxor	xmm7, xmm5				; 0E0C _ 66: 0F EF. FD
	movdqa	xmm0, xmm7				; 0E10 _ 66: 0F 6F. C7
	pxor	xmm1, xmm5				; 0E14 _ 66: 0F EF. CD
	pslld	xmm2, 3 				; 0E18 _ 66: 0F 72. F2, 03
	pxor	xmm1, xmm2				; 0E1D _ 66: 0F EF. CA
	pslld	xmm0, 1 				; 0E21 _ 66: 0F 72. F0, 01
	psrld	xmm7, 31				; 0E26 _ 66: 0F 72. D7, 1F
	por	xmm0, xmm7				; 0E2B _ 66: 0F EB. C7
	movdqa	xmm7, xmm1				; 0E2F _ 66: 0F 6F. F9
	psrld	xmm1, 25				; 0E33 _ 66: 0F 72. D1, 19
	pxor	xmm6, xmm0				; 0E38 _ 66: 0F EF. F0
	pslld	xmm7, 7 				; 0E3C _ 66: 0F 72. F7, 07
	por	xmm7, xmm1				; 0E41 _ 66: 0F EB. F9
	movdqa	xmm1, xmm0				; 0E45 _ 66: 0F 6F. C8
	pxor	xmm6, xmm7				; 0E49 _ 66: 0F EF. F7
	pxor	xmm5, xmm7				; 0E4D _ 66: 0F EF. EF
	pslld	xmm1, 7 				; 0E51 _ 66: 0F 72. F1, 07
	pxor	xmm5, xmm1				; 0E56 _ 66: 0F EF. E9
	movdqa	xmm1, xmm6				; 0E5A _ 66: 0F 6F. CE
	movdqa	xmm2, xmm5				; 0E5E _ 66: 0F 6F. D5
	psrld	xmm6, 27				; 0E62 _ 66: 0F 72. D6, 1B
	pslld	xmm1, 5 				; 0E67 _ 66: 0F 72. F1, 05
	por	xmm1, xmm6				; 0E6C _ 66: 0F EB. CE
	pslld	xmm2, 22				; 0E70 _ 66: 0F 72. F2, 16
	psrld	xmm5, 10				; 0E75 _ 66: 0F 72. D5, 0A
	por	xmm2, xmm5				; 0E7A _ 66: 0F EB. D5
	movd	xmm5, dword [ecx+2B30H] 		; 0E7E _ 66: 0F 6E. A9, 00002B30
	movd	xmm6, dword [ecx+2B34H] 		; 0E86 _ 66: 0F 6E. B1, 00002B34
	pshufd	xmm5, xmm5, 0				; 0E8E _ 66: 0F 70. ED, 00
	pxor	xmm1, xmm5				; 0E93 _ 66: 0F EF. CD
	pshufd	xmm5, xmm6, 0				; 0E97 _ 66: 0F 70. EE, 00
	pxor	xmm0, xmm5				; 0E9C _ 66: 0F EF. C5
	movd	xmm6, dword [ecx+2B38H] 		; 0EA0 _ 66: 0F 6E. B1, 00002B38
	pshufd	xmm5, xmm6, 0				; 0EA8 _ 66: 0F 70. EE, 00
	pxor	xmm2, xmm5				; 0EAD _ 66: 0F EF. D5
	movd	xmm6, dword [ecx+2B3CH] 		; 0EB1 _ 66: 0F 6E. B1, 00002B3C
	pshufd	xmm5, xmm6, 0				; 0EB9 _ 66: 0F 70. EE, 00
	movdqa	xmm6, xmm1				; 0EBE _ 66: 0F 6F. F1
	pxor	xmm7, xmm5				; 0EC2 _ 66: 0F EF. FD
	por	xmm6, xmm7				; 0EC6 _ 66: 0F EB. F7
	pxor	xmm7, xmm0				; 0ECA _ 66: 0F EF. F8
	pand	xmm0, xmm1				; 0ECE _ 66: 0F DB. C1
	pxor	xmm1, xmm2				; 0ED2 _ 66: 0F EF. CA
	pxor	xmm2, xmm7				; 0ED6 _ 66: 0F EF. D7
	pand	xmm7, xmm6				; 0EDA _ 66: 0F DB. FE
	por	xmm1, xmm0				; 0EDE _ 66: 0F EB. C8
	pxor	xmm7, xmm1				; 0EE2 _ 66: 0F EF. F9
	pxor	xmm6, xmm0				; 0EE6 _ 66: 0F EF. F0
	pand	xmm1, xmm6				; 0EEA _ 66: 0F DB. CE
	pxor	xmm0, xmm7				; 0EEE _ 66: 0F EF. C7
	pxor	xmm1, xmm2				; 0EF2 _ 66: 0F EF. CA
	por	xmm0, xmm6				; 0EF6 _ 66: 0F EB. C6
	pxor	xmm0, xmm2				; 0EFA _ 66: 0F EF. C2
	movdqa	xmm2, xmm0				; 0EFE _ 66: 0F 6F. D0
	pxor	xmm6, xmm7				; 0F02 _ 66: 0F EF. F7
	por	xmm2, xmm7				; 0F06 _ 66: 0F EB. D7
	pxor	xmm6, xmm2				; 0F0A _ 66: 0F EF. F2
	movdqa	xmm5, xmm6				; 0F0E _ 66: 0F 6F. EE
	movdqa	xmm2, xmm7				; 0F12 _ 66: 0F 6F. D7
	psrld	xmm6, 19				; 0F16 _ 66: 0F 72. D6, 13
	pslld	xmm5, 13				; 0F1B _ 66: 0F 72. F5, 0D
	por	xmm5, xmm6				; 0F20 _ 66: 0F EB. EE
	pslld	xmm2, 3 				; 0F24 _ 66: 0F 72. F2, 03
	psrld	xmm7, 29				; 0F29 _ 66: 0F 72. D7, 1D
	por	xmm2, xmm7				; 0F2E _ 66: 0F EB. D7
	movdqa	xmm7, xmm5				; 0F32 _ 66: 0F 6F. FD
	pxor	xmm0, xmm5				; 0F36 _ 66: 0F EF. C5
	pxor	xmm0, xmm2				; 0F3A _ 66: 0F EF. C2
	pxor	xmm1, xmm2				; 0F3E _ 66: 0F EF. CA
	pslld	xmm7, 3 				; 0F42 _ 66: 0F 72. F7, 03
	pxor	xmm1, xmm7				; 0F47 _ 66: 0F EF. CF
	movdqa	xmm7, xmm0				; 0F4B _ 66: 0F 6F. F8
	psrld	xmm0, 31				; 0F4F _ 66: 0F 72. D0, 1F
	pslld	xmm7, 1 				; 0F54 _ 66: 0F 72. F7, 01
	por	xmm7, xmm0				; 0F59 _ 66: 0F EB. F8
	movdqa	xmm0, xmm1				; 0F5D _ 66: 0F 6F. C1
	psrld	xmm1, 25				; 0F61 _ 66: 0F 72. D1, 19
	pxor	xmm5, xmm7				; 0F66 _ 66: 0F EF. EF
	pslld	xmm0, 7 				; 0F6A _ 66: 0F 72. F0, 07
	por	xmm0, xmm1				; 0F6F _ 66: 0F EB. C1
	movdqa	xmm1, xmm7				; 0F73 _ 66: 0F 6F. CF
	pxor	xmm5, xmm0				; 0F77 _ 66: 0F EF. E8
	pxor	xmm2, xmm0				; 0F7B _ 66: 0F EF. D0
	pslld	xmm1, 7 				; 0F7F _ 66: 0F 72. F1, 07
	pxor	xmm2, xmm1				; 0F84 _ 66: 0F EF. D1
	movdqa	xmm1, xmm5				; 0F88 _ 66: 0F 6F. CD
	psrld	xmm5, 27				; 0F8C _ 66: 0F 72. D5, 1B
	pslld	xmm1, 5 				; 0F91 _ 66: 0F 72. F1, 05
	por	xmm1, xmm5				; 0F96 _ 66: 0F EB. CD
	movdqa	xmm5, xmm2				; 0F9A _ 66: 0F 6F. EA
	psrld	xmm2, 10				; 0F9E _ 66: 0F 72. D2, 0A
	pslld	xmm5, 22				; 0FA3 _ 66: 0F 72. F5, 16
	por	xmm5, xmm2				; 0FA8 _ 66: 0F EB. EA
	movd	xmm2, dword [ecx+2B40H] 		; 0FAC _ 66: 0F 6E. 91, 00002B40
	pshufd	xmm6, xmm2, 0				; 0FB4 _ 66: 0F 70. F2, 00
	pxor	xmm1, xmm6				; 0FB9 _ 66: 0F EF. CE
	movd	xmm2, dword [ecx+2B44H] 		; 0FBD _ 66: 0F 6E. 91, 00002B44
	pshufd	xmm6, xmm2, 0				; 0FC5 _ 66: 0F 70. F2, 00
	movd	xmm2, dword [ecx+2B48H] 		; 0FCA _ 66: 0F 6E. 91, 00002B48
	pxor	xmm7, xmm6				; 0FD2 _ 66: 0F EF. FE
	pshufd	xmm6, xmm2, 0				; 0FD6 _ 66: 0F 70. F2, 00
	pxor	xmm5, xmm6				; 0FDB _ 66: 0F EF. EE
	movd	xmm2, dword [ecx+2B4CH] 		; 0FDF _ 66: 0F 6E. 91, 00002B4C
	pshufd	xmm6, xmm2, 0				; 0FE7 _ 66: 0F 70. F2, 00
	pxor	xmm0, xmm6				; 0FEC _ 66: 0F EF. C6
	pxor	xmm7, xmm0				; 0FF0 _ 66: 0F EF. F8
	movdqa	xmm2, xmm7				; 0FF4 _ 66: 0F 6F. D7
	pxor	xmm0, xmm4				; 0FF8 _ 66: 0F EF. C4
	pxor	xmm5, xmm0				; 0FFC _ 66: 0F EF. E8
	pxor	xmm0, xmm1				; 1000 _ 66: 0F EF. C1
	pand	xmm2, xmm0				; 1004 _ 66: 0F DB. D0
	pxor	xmm2, xmm5				; 1008 _ 66: 0F EF. D5
	pxor	xmm7, xmm0				; 100C _ 66: 0F EF. F8
	pxor	xmm1, xmm7				; 1010 _ 66: 0F EF. CF
	movdqa	xmm6, xmm2				; 1014 _ 66: 0F 6F. F2
	pand	xmm5, xmm7				; 1018 _ 66: 0F DB. EF
	pxor	xmm5, xmm1				; 101C _ 66: 0F EF. E9
	pand	xmm1, xmm2				; 1020 _ 66: 0F DB. CA
	pxor	xmm0, xmm1				; 1024 _ 66: 0F EF. C1
	por	xmm7, xmm2				; 1028 _ 66: 0F EB. FA
	pxor	xmm7, xmm1				; 102C _ 66: 0F EF. F9
	por	xmm1, xmm0				; 1030 _ 66: 0F EB. C8
	pxor	xmm1, xmm5				; 1034 _ 66: 0F EF. CD
	pand	xmm5, xmm0				; 1038 _ 66: 0F DB. E8
	pxor	xmm1, xmm4				; 103C _ 66: 0F EF. CC
	pxor	xmm7, xmm5				; 1040 _ 66: 0F EF. FD
	pslld	xmm6, 13				; 1044 _ 66: 0F 72. F6, 0D
	psrld	xmm2, 19				; 1049 _ 66: 0F 72. D2, 13
	por	xmm6, xmm2				; 104E _ 66: 0F EB. F2
	movdqa	xmm2, xmm1				; 1052 _ 66: 0F 6F. D1
	psrld	xmm1, 29				; 1056 _ 66: 0F 72. D1, 1D
	pxor	xmm7, xmm6				; 105B _ 66: 0F EF. FE
	pslld	xmm2, 3 				; 105F _ 66: 0F 72. F2, 03
	por	xmm2, xmm1				; 1064 _ 66: 0F EB. D1
	movdqa	xmm1, xmm6				; 1068 _ 66: 0F 6F. CE
	pxor	xmm7, xmm2				; 106C _ 66: 0F EF. FA
	movdqa	xmm5, xmm7				; 1070 _ 66: 0F 6F. EF
	pxor	xmm0, xmm2				; 1074 _ 66: 0F EF. C2
	pslld	xmm1, 3 				; 1078 _ 66: 0F 72. F1, 03
	pxor	xmm0, xmm1				; 107D _ 66: 0F EF. C1
	movdqa	xmm1, xmm0				; 1081 _ 66: 0F 6F. C8
	pslld	xmm5, 1 				; 1085 _ 66: 0F 72. F5, 01
	psrld	xmm7, 31				; 108A _ 66: 0F 72. D7, 1F
	por	xmm5, xmm7				; 108F _ 66: 0F EB. EF
	movdqa	xmm7, xmm5				; 1093 _ 66: 0F 6F. FD
	pslld	xmm1, 7 				; 1097 _ 66: 0F 72. F1, 07
	psrld	xmm0, 25				; 109C _ 66: 0F 72. D0, 19
	por	xmm1, xmm0				; 10A1 _ 66: 0F EB. C8
	pxor	xmm6, xmm5				; 10A5 _ 66: 0F EF. F5
	movd	xmm0, dword [ecx+2B50H] 		; 10A9 _ 66: 0F 6E. 81, 00002B50
	pxor	xmm6, xmm1				; 10B1 _ 66: 0F EF. F1
	pxor	xmm2, xmm1				; 10B5 _ 66: 0F EF. D1
	pslld	xmm7, 7 				; 10B9 _ 66: 0F 72. F7, 07
	pxor	xmm2, xmm7				; 10BE _ 66: 0F EF. D7
	movdqa	xmm7, xmm6				; 10C2 _ 66: 0F 6F. FE
	psrld	xmm6, 27				; 10C6 _ 66: 0F 72. D6, 1B
	pslld	xmm7, 5 				; 10CB _ 66: 0F 72. F7, 05
	por	xmm7, xmm6				; 10D0 _ 66: 0F EB. FE
	movdqa	xmm6, xmm2				; 10D4 _ 66: 0F 6F. F2
	psrld	xmm2, 10				; 10D8 _ 66: 0F 72. D2, 0A
	pslld	xmm6, 22				; 10DD _ 66: 0F 72. F6, 16
	por	xmm6, xmm2				; 10E2 _ 66: 0F EB. F2
	pshufd	xmm2, xmm0, 0				; 10E6 _ 66: 0F 70. D0, 00
	pxor	xmm7, xmm2				; 10EB _ 66: 0F EF. FA
	movd	xmm0, dword [ecx+2B54H] 		; 10EF _ 66: 0F 6E. 81, 00002B54
	pshufd	xmm2, xmm0, 0				; 10F7 _ 66: 0F 70. D0, 00
	pxor	xmm5, xmm2				; 10FC _ 66: 0F EF. EA
	pxor	xmm7, xmm5				; 1100 _ 66: 0F EF. FD
	movd	xmm0, dword [ecx+2B58H] 		; 1104 _ 66: 0F 6E. 81, 00002B58
	pshufd	xmm2, xmm0, 0				; 110C _ 66: 0F 70. D0, 00
	movd	xmm0, dword [ecx+2B5CH] 		; 1111 _ 66: 0F 6E. 81, 00002B5C
	pxor	xmm6, xmm2				; 1119 _ 66: 0F EF. F2
	pshufd	xmm2, xmm0, 0				; 111D _ 66: 0F 70. D0, 00
	pxor	xmm1, xmm2				; 1122 _ 66: 0F EF. CA
	pxor	xmm5, xmm1				; 1126 _ 66: 0F EF. E9
	movdqa	xmm0, xmm5				; 112A _ 66: 0F 6F. C5
	pxor	xmm1, xmm4				; 112E _ 66: 0F EF. CC
	pxor	xmm6, xmm1				; 1132 _ 66: 0F EF. F1
	pand	xmm0, xmm7				; 1136 _ 66: 0F DB. C7
	pxor	xmm0, xmm6				; 113A _ 66: 0F EF. C6
	movdqa	xmm2, xmm0				; 113E _ 66: 0F 6F. D0
	por	xmm6, xmm5				; 1142 _ 66: 0F EB. F5
	pxor	xmm5, xmm1				; 1146 _ 66: 0F EF. E9
	pand	xmm1, xmm0				; 114A _ 66: 0F DB. C8
	pxor	xmm1, xmm7				; 114E _ 66: 0F EF. CF
	pxor	xmm5, xmm0				; 1152 _ 66: 0F EF. E8
	pxor	xmm5, xmm6				; 1156 _ 66: 0F EF. EE
	pxor	xmm6, xmm7				; 115A _ 66: 0F EF. F7
	pand	xmm7, xmm1				; 115E _ 66: 0F DB. F9
	pxor	xmm6, xmm4				; 1162 _ 66: 0F EF. F4
	pxor	xmm7, xmm5				; 1166 _ 66: 0F EF. FD
	por	xmm5, xmm1				; 116A _ 66: 0F EB. E9
	pxor	xmm5, xmm6				; 116E _ 66: 0F EF. EE
	movdqa	xmm6, xmm7				; 1172 _ 66: 0F 6F. F7
	pslld	xmm2, 13				; 1176 _ 66: 0F 72. F2, 0D
	psrld	xmm0, 19				; 117B _ 66: 0F 72. D0, 13
	por	xmm2, xmm0				; 1180 _ 66: 0F EB. D0
	pslld	xmm6, 3 				; 1184 _ 66: 0F 72. F6, 03
	psrld	xmm7, 29				; 1189 _ 66: 0F 72. D7, 1D
	por	xmm6, xmm7				; 118E _ 66: 0F EB. F7
	movdqa	xmm7, xmm2				; 1192 _ 66: 0F 6F. FA
	pxor	xmm1, xmm2				; 1196 _ 66: 0F EF. CA
	pxor	xmm1, xmm6				; 119A _ 66: 0F EF. CE
	movdqa	xmm0, xmm1				; 119E _ 66: 0F 6F. C1
	pxor	xmm5, xmm6				; 11A2 _ 66: 0F EF. EE
	pslld	xmm7, 3 				; 11A6 _ 66: 0F 72. F7, 03
	pxor	xmm5, xmm7				; 11AB _ 66: 0F EF. EF
	pslld	xmm0, 1 				; 11AF _ 66: 0F 72. F0, 01
	psrld	xmm1, 31				; 11B4 _ 66: 0F 72. D1, 1F
	por	xmm0, xmm1				; 11B9 _ 66: 0F EB. C1
	movdqa	xmm1, xmm5				; 11BD _ 66: 0F 6F. CD
	psrld	xmm5, 25				; 11C1 _ 66: 0F 72. D5, 19
	pxor	xmm2, xmm0				; 11C6 _ 66: 0F EF. D0
	pslld	xmm1, 7 				; 11CA _ 66: 0F 72. F1, 07
	por	xmm1, xmm5				; 11CF _ 66: 0F EB. CD
	movdqa	xmm5, xmm0				; 11D3 _ 66: 0F 6F. E8
	pxor	xmm2, xmm1				; 11D7 _ 66: 0F EF. D1
	movdqa	xmm7, xmm2				; 11DB _ 66: 0F 6F. FA
	pxor	xmm6, xmm1				; 11DF _ 66: 0F EF. F1
	pslld	xmm5, 7 				; 11E3 _ 66: 0F 72. F5, 07
	pxor	xmm6, xmm5				; 11E8 _ 66: 0F EF. F5
	movdqa	xmm5, xmm6				; 11EC _ 66: 0F 6F. EE
	pslld	xmm7, 5 				; 11F0 _ 66: 0F 72. F7, 05
	psrld	xmm2, 27				; 11F5 _ 66: 0F 72. D2, 1B
	por	xmm7, xmm2				; 11FA _ 66: 0F EB. FA
	movd	xmm2, dword [ecx+2B64H] 		; 11FE _ 66: 0F 6E. 91, 00002B64
	pslld	xmm5, 22				; 1206 _ 66: 0F 72. F5, 16
	psrld	xmm6, 10				; 120B _ 66: 0F 72. D6, 0A
	por	xmm5, xmm6				; 1210 _ 66: 0F EB. EE
	movd	xmm6, dword [ecx+2B60H] 		; 1214 _ 66: 0F 6E. B1, 00002B60
	pshufd	xmm6, xmm6, 0				; 121C _ 66: 0F 70. F6, 00
	pxor	xmm7, xmm6				; 1221 _ 66: 0F EF. FE
	pshufd	xmm6, xmm2, 0				; 1225 _ 66: 0F 70. F2, 00
	movd	xmm2, dword [ecx+2B68H] 		; 122A _ 66: 0F 6E. 91, 00002B68
	pxor	xmm0, xmm6				; 1232 _ 66: 0F EF. C6
	pshufd	xmm6, xmm2, 0				; 1236 _ 66: 0F 70. F2, 00
	pxor	xmm5, xmm6				; 123B _ 66: 0F EF. EE
	pxor	xmm5, xmm4				; 123F _ 66: 0F EF. EC
	movd	xmm2, dword [ecx+2B6CH] 		; 1243 _ 66: 0F 6E. 91, 00002B6C
	pshufd	xmm6, xmm2, 0				; 124B _ 66: 0F 70. F2, 00
	pxor	xmm1, xmm6				; 1250 _ 66: 0F EF. CE
	movdqa	xmm2, xmm1				; 1254 _ 66: 0F 6F. D1
	pand	xmm2, xmm7				; 1258 _ 66: 0F DB. D7
	pxor	xmm7, xmm1				; 125C _ 66: 0F EF. F9
	pxor	xmm2, xmm5				; 1260 _ 66: 0F EF. D5
	por	xmm5, xmm1				; 1264 _ 66: 0F EB. E9
	pxor	xmm0, xmm2				; 1268 _ 66: 0F EF. C2
	pxor	xmm5, xmm7				; 126C _ 66: 0F EF. EF
	por	xmm7, xmm0				; 1270 _ 66: 0F EB. F8
	pxor	xmm5, xmm0				; 1274 _ 66: 0F EF. E8
	pxor	xmm1, xmm7				; 1278 _ 66: 0F EF. CF
	por	xmm7, xmm2				; 127C _ 66: 0F EB. FA
	pxor	xmm7, xmm5				; 1280 _ 66: 0F EF. FD
	movdqa	xmm6, xmm7				; 1284 _ 66: 0F 6F. F7
	pxor	xmm1, xmm2				; 1288 _ 66: 0F EF. CA
	pxor	xmm1, xmm7				; 128C _ 66: 0F EF. CF
	pxor	xmm2, xmm4				; 1290 _ 66: 0F EF. D4
	pand	xmm5, xmm1				; 1294 _ 66: 0F DB. E9
	pxor	xmm2, xmm5				; 1298 _ 66: 0F EF. D5
	movdqa	xmm5, xmm1				; 129C _ 66: 0F 6F. E9
	pslld	xmm6, 13				; 12A0 _ 66: 0F 72. F6, 0D
	psrld	xmm7, 19				; 12A5 _ 66: 0F 72. D7, 13
	por	xmm6, xmm7				; 12AA _ 66: 0F EB. F7
	pslld	xmm5, 3 				; 12AE _ 66: 0F 72. F5, 03
	psrld	xmm1, 29				; 12B3 _ 66: 0F 72. D1, 1D
	por	xmm5, xmm1				; 12B8 _ 66: 0F EB. E9
	movdqa	xmm1, xmm6				; 12BC _ 66: 0F 6F. CE
	pxor	xmm0, xmm6				; 12C0 _ 66: 0F EF. C6
	pxor	xmm0, xmm5				; 12C4 _ 66: 0F EF. C5
	movdqa	xmm7, xmm0				; 12C8 _ 66: 0F 6F. F8
	pxor	xmm2, xmm5				; 12CC _ 66: 0F EF. D5
	pslld	xmm1, 3 				; 12D0 _ 66: 0F 72. F1, 03
	pxor	xmm2, xmm1				; 12D5 _ 66: 0F EF. D1
	movdqa	xmm1, xmm2				; 12D9 _ 66: 0F 6F. CA
	pslld	xmm7, 1 				; 12DD _ 66: 0F 72. F7, 01
	psrld	xmm0, 31				; 12E2 _ 66: 0F 72. D0, 1F
	por	xmm7, xmm0				; 12E7 _ 66: 0F EB. F8
	movdqa	xmm0, xmm7				; 12EB _ 66: 0F 6F. C7
	pslld	xmm1, 7 				; 12EF _ 66: 0F 72. F1, 07
	psrld	xmm2, 25				; 12F4 _ 66: 0F 72. D2, 19
	por	xmm1, xmm2				; 12F9 _ 66: 0F EB. CA
	pxor	xmm6, xmm7				; 12FD _ 66: 0F EF. F7
	pxor	xmm6, xmm1				; 1301 _ 66: 0F EF. F1
	movdqa	xmm2, xmm6				; 1305 _ 66: 0F 6F. D6
	pxor	xmm5, xmm1				; 1309 _ 66: 0F EF. E9
	pslld	xmm0, 7 				; 130D _ 66: 0F 72. F0, 07
	pxor	xmm5, xmm0				; 1312 _ 66: 0F EF. E8
	movdqa	xmm0, xmm5				; 1316 _ 66: 0F 6F. C5
	pslld	xmm2, 5 				; 131A _ 66: 0F 72. F2, 05
	psrld	xmm6, 27				; 131F _ 66: 0F 72. D6, 1B
	por	xmm2, xmm6				; 1324 _ 66: 0F EB. D6
	pslld	xmm0, 22				; 1328 _ 66: 0F 72. F0, 16
	psrld	xmm5, 10				; 132D _ 66: 0F 72. D5, 0A
	por	xmm0, xmm5				; 1332 _ 66: 0F EB. C5
	movd	xmm5, dword [ecx+2B70H] 		; 1336 _ 66: 0F 6E. A9, 00002B70
	pshufd	xmm6, xmm5, 0				; 133E _ 66: 0F 70. F5, 00
	movd	xmm5, dword [ecx+2B74H] 		; 1343 _ 66: 0F 6E. A9, 00002B74
	pxor	xmm2, xmm6				; 134B _ 66: 0F EF. D6
	pshufd	xmm6, xmm5, 0				; 134F _ 66: 0F 70. F5, 00
	pxor	xmm7, xmm6				; 1354 _ 66: 0F EF. FE
	movd	xmm5, dword [ecx+2B78H] 		; 1358 _ 66: 0F 6E. A9, 00002B78
	pshufd	xmm6, xmm5, 0				; 1360 _ 66: 0F 70. F5, 00
	pxor	xmm0, xmm6				; 1365 _ 66: 0F EF. C6
	movd	xmm5, dword [ecx+2B7CH] 		; 1369 _ 66: 0F 6E. A9, 00002B7C
	pshufd	xmm6, xmm5, 0				; 1371 _ 66: 0F 70. F5, 00
	movdqa	xmm5, xmm7				; 1376 _ 66: 0F 6F. EF
	pxor	xmm1, xmm6				; 137A _ 66: 0F EF. CE
	pxor	xmm7, xmm0				; 137E _ 66: 0F EF. F8
	por	xmm5, xmm0				; 1382 _ 66: 0F EB. E8
	pxor	xmm5, xmm1				; 1386 _ 66: 0F EF. E9
	pxor	xmm0, xmm5				; 138A _ 66: 0F EF. C5
	por	xmm1, xmm7				; 138E _ 66: 0F EB. CF
	pand	xmm1, xmm2				; 1392 _ 66: 0F DB. CA
	pxor	xmm7, xmm0				; 1396 _ 66: 0F EF. F8
	pxor	xmm1, xmm5				; 139A _ 66: 0F EF. CD
	por	xmm5, xmm7				; 139E _ 66: 0F EB. EF
	pxor	xmm5, xmm2				; 13A2 _ 66: 0F EF. EA
	por	xmm2, xmm7				; 13A6 _ 66: 0F EB. D7
	pxor	xmm2, xmm0				; 13AA _ 66: 0F EF. D0
	pxor	xmm5, xmm7				; 13AE _ 66: 0F EF. EF
	pxor	xmm0, xmm5				; 13B2 _ 66: 0F EF. C5
	pand	xmm5, xmm2				; 13B6 _ 66: 0F DB. EA
	pxor	xmm5, xmm7				; 13BA _ 66: 0F EF. EF
	movdqa	xmm6, xmm5				; 13BE _ 66: 0F 6F. F5
	pxor	xmm0, xmm4				; 13C2 _ 66: 0F EF. C4
	por	xmm0, xmm2				; 13C6 _ 66: 0F EB. C2
	pxor	xmm7, xmm0				; 13CA _ 66: 0F EF. F8
	movdqa	xmm0, xmm7				; 13CE _ 66: 0F 6F. C7
	psrld	xmm7, 19				; 13D2 _ 66: 0F 72. D7, 13
	pslld	xmm6, 3 				; 13D7 _ 66: 0F 72. F6, 03
	pslld	xmm0, 13				; 13DC _ 66: 0F 72. F0, 0D
	por	xmm0, xmm7				; 13E1 _ 66: 0F EB. C7
	movdqa	xmm7, xmm0				; 13E5 _ 66: 0F 6F. F8
	psrld	xmm5, 29				; 13E9 _ 66: 0F 72. D5, 1D
	por	xmm6, xmm5				; 13EE _ 66: 0F EB. F5
	pxor	xmm1, xmm0				; 13F2 _ 66: 0F EF. C8
	pxor	xmm1, xmm6				; 13F6 _ 66: 0F EF. CE
	pxor	xmm2, xmm6				; 13FA _ 66: 0F EF. D6
	pslld	xmm7, 3 				; 13FE _ 66: 0F 72. F7, 03
	pxor	xmm2, xmm7				; 1403 _ 66: 0F EF. D7
	movdqa	xmm7, xmm1				; 1407 _ 66: 0F 6F. F9
	movdqa	xmm5, xmm2				; 140B _ 66: 0F 6F. EA
	psrld	xmm1, 31				; 140F _ 66: 0F 72. D1, 1F
	pslld	xmm7, 1 				; 1414 _ 66: 0F 72. F7, 01
	por	xmm7, xmm1				; 1419 _ 66: 0F EB. F9
	pslld	xmm5, 7 				; 141D _ 66: 0F 72. F5, 07
	psrld	xmm2, 25				; 1422 _ 66: 0F 72. D2, 19
	por	xmm5, xmm2				; 1427 _ 66: 0F EB. EA
	movdqa	xmm2, xmm7				; 142B _ 66: 0F 6F. D7
	pxor	xmm0, xmm7				; 142F _ 66: 0F EF. C7
	pxor	xmm0, xmm5				; 1433 _ 66: 0F EF. C5
	pxor	xmm6, xmm5				; 1437 _ 66: 0F EF. F5
	pslld	xmm2, 7 				; 143B _ 66: 0F 72. F2, 07
	pxor	xmm6, xmm2				; 1440 _ 66: 0F EF. F2
	movdqa	xmm2, xmm0				; 1444 _ 66: 0F 6F. D0
	movdqa	xmm1, xmm6				; 1448 _ 66: 0F 6F. CE
	psrld	xmm0, 27				; 144C _ 66: 0F 72. D0, 1B
	pslld	xmm2, 5 				; 1451 _ 66: 0F 72. F2, 05
	por	xmm2, xmm0				; 1456 _ 66: 0F EB. D0
	movd	xmm0, dword [ecx+2B80H] 		; 145A _ 66: 0F 6E. 81, 00002B80
	pslld	xmm1, 22				; 1462 _ 66: 0F 72. F1, 16
	psrld	xmm6, 10				; 1467 _ 66: 0F 72. D6, 0A
	por	xmm1, xmm6				; 146C _ 66: 0F EB. CE
	pshufd	xmm6, xmm0, 0				; 1470 _ 66: 0F 70. F0, 00
	pxor	xmm2, xmm6				; 1475 _ 66: 0F EF. D6
	movd	xmm0, dword [ecx+2B84H] 		; 1479 _ 66: 0F 6E. 81, 00002B84
	pshufd	xmm6, xmm0, 0				; 1481 _ 66: 0F 70. F0, 00
	movd	xmm0, dword [ecx+2B88H] 		; 1486 _ 66: 0F 6E. 81, 00002B88
	pxor	xmm7, xmm6				; 148E _ 66: 0F EF. FE
	pshufd	xmm6, xmm0, 0				; 1492 _ 66: 0F 70. F0, 00
	pxor	xmm1, xmm6				; 1497 _ 66: 0F EF. CE
	movd	xmm0, dword [ecx+2B8CH] 		; 149B _ 66: 0F 6E. 81, 00002B8C
	pshufd	xmm6, xmm0, 0				; 14A3 _ 66: 0F 70. F0, 00
	movdqa	xmm0, xmm7				; 14A8 _ 66: 0F 6F. C7
	pxor	xmm5, xmm6				; 14AC _ 66: 0F EF. EE
	pxor	xmm5, xmm2				; 14B0 _ 66: 0F EF. EA
	pand	xmm0, xmm5				; 14B4 _ 66: 0F DB. C5
	pxor	xmm7, xmm1				; 14B8 _ 66: 0F EF. F9
	pxor	xmm0, xmm2				; 14BC _ 66: 0F EF. C2
	por	xmm2, xmm5				; 14C0 _ 66: 0F EB. D5
	pxor	xmm2, xmm7				; 14C4 _ 66: 0F EF. D7
	pxor	xmm7, xmm5				; 14C8 _ 66: 0F EF. FD
	pxor	xmm5, xmm1				; 14CC _ 66: 0F EF. E9
	por	xmm1, xmm0				; 14D0 _ 66: 0F EB. C8
	pxor	xmm1, xmm7				; 14D4 _ 66: 0F EF. CF
	pxor	xmm7, xmm4				; 14D8 _ 66: 0F EF. FC
	por	xmm7, xmm0				; 14DC _ 66: 0F EB. F8
	pxor	xmm0, xmm5				; 14E0 _ 66: 0F EF. C5
	pxor	xmm0, xmm7				; 14E4 _ 66: 0F EF. C7
	por	xmm5, xmm2				; 14E8 _ 66: 0F EB. EA
	pxor	xmm0, xmm5				; 14EC _ 66: 0F EF. C5
	movdqa	xmm6, xmm0				; 14F0 _ 66: 0F 6F. F0
	pxor	xmm7, xmm5				; 14F4 _ 66: 0F EF. FD
	psrld	xmm0, 19				; 14F8 _ 66: 0F 72. D0, 13
	pslld	xmm6, 13				; 14FD _ 66: 0F 72. F6, 0D
	por	xmm6, xmm0				; 1502 _ 66: 0F EB. F0
	movdqa	xmm0, xmm1				; 1506 _ 66: 0F 6F. C1
	psrld	xmm1, 29				; 150A _ 66: 0F 72. D1, 1D
	pxor	xmm7, xmm6				; 150F _ 66: 0F EF. FE
	pslld	xmm0, 3 				; 1513 _ 66: 0F 72. F0, 03
	por	xmm0, xmm1				; 1518 _ 66: 0F EB. C1
	movdqa	xmm1, xmm6				; 151C _ 66: 0F 6F. CE
	pxor	xmm7, xmm0				; 1520 _ 66: 0F EF. F8
	movdqa	xmm5, xmm7				; 1524 _ 66: 0F 6F. EF
	pxor	xmm2, xmm0				; 1528 _ 66: 0F EF. D0
	pslld	xmm1, 3 				; 152C _ 66: 0F 72. F1, 03
	pxor	xmm2, xmm1				; 1531 _ 66: 0F EF. D1
	movdqa	xmm1, xmm2				; 1535 _ 66: 0F 6F. CA
	pslld	xmm5, 1 				; 1539 _ 66: 0F 72. F5, 01
	psrld	xmm7, 31				; 153E _ 66: 0F 72. D7, 1F
	por	xmm5, xmm7				; 1543 _ 66: 0F EB. EF
	movdqa	xmm7, xmm5				; 1547 _ 66: 0F 6F. FD
	pslld	xmm1, 7 				; 154B _ 66: 0F 72. F1, 07
	psrld	xmm2, 25				; 1550 _ 66: 0F 72. D2, 19
	por	xmm1, xmm2				; 1555 _ 66: 0F EB. CA
	pxor	xmm6, xmm5				; 1559 _ 66: 0F EF. F5
	pxor	xmm6, xmm1				; 155D _ 66: 0F EF. F1
	movd	xmm2, dword [ecx+2B90H] 		; 1561 _ 66: 0F 6E. 91, 00002B90
	pxor	xmm0, xmm1				; 1569 _ 66: 0F EF. C1
	pslld	xmm7, 7 				; 156D _ 66: 0F 72. F7, 07
	pxor	xmm0, xmm7				; 1572 _ 66: 0F EF. C7
	movdqa	xmm7, xmm6				; 1576 _ 66: 0F 6F. FE
	psrld	xmm6, 27				; 157A _ 66: 0F 72. D6, 1B
	pslld	xmm7, 5 				; 157F _ 66: 0F 72. F7, 05
	por	xmm7, xmm6				; 1584 _ 66: 0F EB. FE
	movdqa	xmm6, xmm0				; 1588 _ 66: 0F 6F. F0
	psrld	xmm0, 10				; 158C _ 66: 0F 72. D0, 0A
	pslld	xmm6, 22				; 1591 _ 66: 0F 72. F6, 16
	por	xmm6, xmm0				; 1596 _ 66: 0F EB. F0
	pshufd	xmm0, xmm2, 0				; 159A _ 66: 0F 70. C2, 00
	movd	xmm2, dword [ecx+2B94H] 		; 159F _ 66: 0F 6E. 91, 00002B94
	pxor	xmm7, xmm0				; 15A7 _ 66: 0F EF. F8
	pshufd	xmm0, xmm2, 0				; 15AB _ 66: 0F 70. C2, 00
	pxor	xmm5, xmm0				; 15B0 _ 66: 0F EF. E8
	pxor	xmm7, xmm4				; 15B4 _ 66: 0F EF. FC
	movd	xmm2, dword [ecx+2B98H] 		; 15B8 _ 66: 0F 6E. 91, 00002B98
	pshufd	xmm0, xmm2, 0				; 15C0 _ 66: 0F 70. C2, 00
	pxor	xmm6, xmm0				; 15C5 _ 66: 0F EF. F0
	movd	xmm2, dword [ecx+2B9CH] 		; 15C9 _ 66: 0F 6E. 91, 00002B9C
	pshufd	xmm0, xmm2, 0				; 15D1 _ 66: 0F 70. C2, 00
	movdqa	xmm2, xmm7				; 15D6 _ 66: 0F 6F. D7
	pxor	xmm1, xmm0				; 15DA _ 66: 0F EF. C8
	pxor	xmm6, xmm4				; 15DE _ 66: 0F EF. F4
	pand	xmm2, xmm5				; 15E2 _ 66: 0F DB. D5
	pxor	xmm6, xmm2				; 15E6 _ 66: 0F EF. F2
	por	xmm2, xmm1				; 15EA _ 66: 0F EB. D1
	pxor	xmm1, xmm6				; 15EE _ 66: 0F EF. CE
	pxor	xmm5, xmm2				; 15F2 _ 66: 0F EF. EA
	pxor	xmm2, xmm7				; 15F6 _ 66: 0F EF. D7
	por	xmm7, xmm5				; 15FA _ 66: 0F EB. FD
	pxor	xmm5, xmm1				; 15FE _ 66: 0F EF. E9
	por	xmm6, xmm2				; 1602 _ 66: 0F EB. F2
	pand	xmm6, xmm7				; 1606 _ 66: 0F DB. F7
	movdqa	xmm0, xmm6				; 160A _ 66: 0F 6F. C6
	pxor	xmm2, xmm5				; 160E _ 66: 0F EF. D5
	pand	xmm5, xmm6				; 1612 _ 66: 0F DB. EE
	pxor	xmm5, xmm2				; 1616 _ 66: 0F EF. EA
	pand	xmm2, xmm6				; 161A _ 66: 0F DB. D6
	pxor	xmm7, xmm2				; 161E _ 66: 0F EF. FA
	pslld	xmm0, 13				; 1622 _ 66: 0F 72. F0, 0D
	psrld	xmm6, 19				; 1627 _ 66: 0F 72. D6, 13
	por	xmm0, xmm6				; 162C _ 66: 0F EB. C6
	movdqa	xmm6, xmm1				; 1630 _ 66: 0F 6F. F1
	psrld	xmm1, 29				; 1634 _ 66: 0F 72. D1, 1D
	pxor	xmm7, xmm0				; 1639 _ 66: 0F EF. F8
	pslld	xmm6, 3 				; 163D _ 66: 0F 72. F6, 03
	por	xmm6, xmm1				; 1642 _ 66: 0F EB. F1
	movdqa	xmm1, xmm0				; 1646 _ 66: 0F 6F. C8
	pxor	xmm7, xmm6				; 164A _ 66: 0F EF. FE
	pxor	xmm5, xmm6				; 164E _ 66: 0F EF. EE
	pslld	xmm1, 3 				; 1652 _ 66: 0F 72. F1, 03
	pxor	xmm5, xmm1				; 1657 _ 66: 0F EF. E9
	movdqa	xmm1, xmm7				; 165B _ 66: 0F 6F. CF
	psrld	xmm7, 31				; 165F _ 66: 0F 72. D7, 1F
	pslld	xmm1, 1 				; 1664 _ 66: 0F 72. F1, 01
	por	xmm1, xmm7				; 1669 _ 66: 0F EB. CF
	movdqa	xmm7, xmm5				; 166D _ 66: 0F 6F. FD
	psrld	xmm5, 25				; 1671 _ 66: 0F 72. D5, 19
	pxor	xmm0, xmm1				; 1676 _ 66: 0F EF. C1
	pslld	xmm7, 7 				; 167A _ 66: 0F 72. F7, 07
	por	xmm7, xmm5				; 167F _ 66: 0F EB. FD
	movdqa	xmm5, xmm1				; 1683 _ 66: 0F 6F. E9
	pxor	xmm0, xmm7				; 1687 _ 66: 0F EF. C7
	movdqa	xmm2, xmm0				; 168B _ 66: 0F 6F. D0
	pxor	xmm6, xmm7				; 168F _ 66: 0F EF. F7
	pslld	xmm5, 7 				; 1693 _ 66: 0F 72. F5, 07
	pxor	xmm6, xmm5				; 1698 _ 66: 0F EF. F5
	movdqa	xmm5, xmm6				; 169C _ 66: 0F 6F. EE
	pslld	xmm2, 5 				; 16A0 _ 66: 0F 72. F2, 05
	psrld	xmm0, 27				; 16A5 _ 66: 0F 72. D0, 1B
	por	xmm2, xmm0				; 16AA _ 66: 0F EB. D0
	pslld	xmm5, 22				; 16AE _ 66: 0F 72. F5, 16
	psrld	xmm6, 10				; 16B3 _ 66: 0F 72. D6, 0A
	por	xmm5, xmm6				; 16B8 _ 66: 0F EB. EE
	movd	xmm6, dword [ecx+2BA0H] 		; 16BC _ 66: 0F 6E. B1, 00002BA0
	pshufd	xmm0, xmm6, 0				; 16C4 _ 66: 0F 70. C6, 00
	pxor	xmm2, xmm0				; 16C9 _ 66: 0F EF. D0
	movd	xmm6, dword [ecx+2BA4H] 		; 16CD _ 66: 0F 6E. B1, 00002BA4
	pshufd	xmm0, xmm6, 0				; 16D5 _ 66: 0F 70. C6, 00
	pxor	xmm1, xmm0				; 16DA _ 66: 0F EF. C8
	movd	xmm6, dword [ecx+2BA8H] 		; 16DE _ 66: 0F 6E. B1, 00002BA8
	pshufd	xmm0, xmm6, 0				; 16E6 _ 66: 0F 70. C6, 00
	movd	xmm6, dword [ecx+2BACH] 		; 16EB _ 66: 0F 6E. B1, 00002BAC
	pxor	xmm5, xmm0				; 16F3 _ 66: 0F EF. E8
	pshufd	xmm0, xmm6, 0				; 16F7 _ 66: 0F 70. C6, 00
	movdqa	xmm6, xmm2				; 16FC _ 66: 0F 6F. F2
	pxor	xmm7, xmm0				; 1700 _ 66: 0F EF. F8
	pand	xmm6, xmm5				; 1704 _ 66: 0F DB. F5
	pxor	xmm6, xmm7				; 1708 _ 66: 0F EF. F7
	pxor	xmm5, xmm1				; 170C _ 66: 0F EF. E9
	pxor	xmm5, xmm6				; 1710 _ 66: 0F EF. EE
	movdqa	xmm0, xmm5				; 1714 _ 66: 0F 6F. C5
	por	xmm7, xmm2				; 1718 _ 66: 0F EB. FA
	pxor	xmm7, xmm1				; 171C _ 66: 0F EF. F9
	movdqa	xmm1, xmm7				; 1720 _ 66: 0F 6F. CF
	pxor	xmm2, xmm5				; 1724 _ 66: 0F EF. D5
	pslld	xmm0, 13				; 1728 _ 66: 0F 72. F0, 0D
	por	xmm1, xmm2				; 172D _ 66: 0F EB. CA
	pxor	xmm1, xmm6				; 1731 _ 66: 0F EF. CE
	pand	xmm6, xmm7				; 1735 _ 66: 0F DB. F7
	pxor	xmm2, xmm6				; 1739 _ 66: 0F EF. D6
	pxor	xmm7, xmm1				; 173D _ 66: 0F EF. F9
	pxor	xmm7, xmm2				; 1741 _ 66: 0F EF. FA
	movdqa	xmm6, xmm7				; 1745 _ 66: 0F 6F. F7
	pxor	xmm2, xmm4				; 1749 _ 66: 0F EF. D4
	psrld	xmm5, 19				; 174D _ 66: 0F 72. D5, 13
	por	xmm0, xmm5				; 1752 _ 66: 0F EB. C5
	pslld	xmm6, 3 				; 1756 _ 66: 0F 72. F6, 03
	psrld	xmm7, 29				; 175B _ 66: 0F 72. D7, 1D
	por	xmm6, xmm7				; 1760 _ 66: 0F EB. F7
	movdqa	xmm7, xmm0				; 1764 _ 66: 0F 6F. F8
	pxor	xmm1, xmm0				; 1768 _ 66: 0F EF. C8
	pxor	xmm1, xmm6				; 176C _ 66: 0F EF. CE
	movdqa	xmm5, xmm1				; 1770 _ 66: 0F 6F. E9
	pxor	xmm2, xmm6				; 1774 _ 66: 0F EF. D6
	pslld	xmm7, 3 				; 1778 _ 66: 0F 72. F7, 03
	pxor	xmm2, xmm7				; 177D _ 66: 0F EF. D7
	pslld	xmm5, 1 				; 1781 _ 66: 0F 72. F5, 01
	psrld	xmm1, 31				; 1786 _ 66: 0F 72. D1, 1F
	por	xmm5, xmm1				; 178B _ 66: 0F EB. E9
	movdqa	xmm1, xmm2				; 178F _ 66: 0F 6F. CA
	psrld	xmm2, 25				; 1793 _ 66: 0F 72. D2, 19
	pxor	xmm0, xmm5				; 1798 _ 66: 0F EF. C5
	pslld	xmm1, 7 				; 179C _ 66: 0F 72. F1, 07
	por	xmm1, xmm2				; 17A1 _ 66: 0F EB. CA
	movdqa	xmm2, xmm5				; 17A5 _ 66: 0F 6F. D5
	pxor	xmm0, xmm1				; 17A9 _ 66: 0F EF. C1
	pxor	xmm6, xmm1				; 17AD _ 66: 0F EF. F1
	pslld	xmm2, 7 				; 17B1 _ 66: 0F 72. F2, 07
	pxor	xmm6, xmm2				; 17B6 _ 66: 0F EF. F2
	movdqa	xmm2, xmm0				; 17BA _ 66: 0F 6F. D0
	psrld	xmm0, 27				; 17BE _ 66: 0F 72. D0, 1B
	pslld	xmm2, 5 				; 17C3 _ 66: 0F 72. F2, 05
	por	xmm2, xmm0				; 17C8 _ 66: 0F EB. D0
	movdqa	xmm0, xmm6				; 17CC _ 66: 0F 6F. C6
	psrld	xmm6, 10				; 17D0 _ 66: 0F 72. D6, 0A
	pslld	xmm0, 22				; 17D5 _ 66: 0F 72. F0, 16
	por	xmm0, xmm6				; 17DA _ 66: 0F EB. C6
	movd	xmm6, dword [ecx+2BB0H] 		; 17DE _ 66: 0F 6E. B1, 00002BB0
	pshufd	xmm7, xmm6, 0				; 17E6 _ 66: 0F 70. FE, 00
	pxor	xmm2, xmm7				; 17EB _ 66: 0F EF. D7
	movd	xmm6, dword [ecx+2BB4H] 		; 17EF _ 66: 0F 6E. B1, 00002BB4
	pshufd	xmm7, xmm6, 0				; 17F7 _ 66: 0F 70. FE, 00
	movd	xmm6, dword [ecx+2BB8H] 		; 17FC _ 66: 0F 6E. B1, 00002BB8
	pxor	xmm5, xmm7				; 1804 _ 66: 0F EF. EF
	pshufd	xmm7, xmm6, 0				; 1808 _ 66: 0F 70. FE, 00
	pxor	xmm0, xmm7				; 180D _ 66: 0F EF. C7
	movd	xmm6, dword [ecx+2BBCH] 		; 1811 _ 66: 0F 6E. B1, 00002BBC
	pshufd	xmm7, xmm6, 0				; 1819 _ 66: 0F 70. FE, 00
	pxor	xmm1, xmm7				; 181E _ 66: 0F EF. CF
	movdqa	xmm7, xmm2				; 1822 _ 66: 0F 6F. FA
	por	xmm7, xmm1				; 1826 _ 66: 0F EB. F9
	pxor	xmm1, xmm5				; 182A _ 66: 0F EF. CD
	pand	xmm5, xmm2				; 182E _ 66: 0F DB. EA
	pxor	xmm2, xmm0				; 1832 _ 66: 0F EF. D0
	pxor	xmm0, xmm1				; 1836 _ 66: 0F EF. C1
	pand	xmm1, xmm7				; 183A _ 66: 0F DB. CF
	por	xmm2, xmm5				; 183E _ 66: 0F EB. D5
	pxor	xmm1, xmm2				; 1842 _ 66: 0F EF. CA
	pxor	xmm7, xmm5				; 1846 _ 66: 0F EF. FD
	pand	xmm2, xmm7				; 184A _ 66: 0F DB. D7
	pxor	xmm5, xmm1				; 184E _ 66: 0F EF. E9
	pxor	xmm2, xmm0				; 1852 _ 66: 0F EF. D0
	por	xmm5, xmm7				; 1856 _ 66: 0F EB. EF
	pxor	xmm5, xmm0				; 185A _ 66: 0F EF. E8
	movdqa	xmm0, xmm5				; 185E _ 66: 0F 6F. C5
	pxor	xmm7, xmm1				; 1862 _ 66: 0F EF. F9
	por	xmm0, xmm1				; 1866 _ 66: 0F EB. C1
	pxor	xmm7, xmm0				; 186A _ 66: 0F EF. F8
	movdqa	xmm6, xmm7				; 186E _ 66: 0F 6F. F7
	movdqa	xmm0, xmm1				; 1872 _ 66: 0F 6F. C1
	psrld	xmm7, 19				; 1876 _ 66: 0F 72. D7, 13
	pslld	xmm6, 13				; 187B _ 66: 0F 72. F6, 0D
	por	xmm6, xmm7				; 1880 _ 66: 0F EB. F7
	pslld	xmm0, 3 				; 1884 _ 66: 0F 72. F0, 03
	psrld	xmm1, 29				; 1889 _ 66: 0F 72. D1, 1D
	por	xmm0, xmm1				; 188E _ 66: 0F EB. C1
	movdqa	xmm1, xmm6				; 1892 _ 66: 0F 6F. CE
	pxor	xmm5, xmm6				; 1896 _ 66: 0F EF. EE
	pxor	xmm5, xmm0				; 189A _ 66: 0F EF. E8
	pxor	xmm2, xmm0				; 189E _ 66: 0F EF. D0
	pslld	xmm1, 3 				; 18A2 _ 66: 0F 72. F1, 03
	pxor	xmm2, xmm1				; 18A7 _ 66: 0F EF. D1
	movdqa	xmm1, xmm5				; 18AB _ 66: 0F 6F. CD
	movdqa	xmm7, xmm2				; 18AF _ 66: 0F 6F. FA
	psrld	xmm5, 31				; 18B3 _ 66: 0F 72. D5, 1F
	pslld	xmm1, 1 				; 18B8 _ 66: 0F 72. F1, 01
	por	xmm1, xmm5				; 18BD _ 66: 0F EB. CD
	movdqa	xmm5, xmm1				; 18C1 _ 66: 0F 6F. E9
	pslld	xmm7, 7 				; 18C5 _ 66: 0F 72. F7, 07
	psrld	xmm2, 25				; 18CA _ 66: 0F 72. D2, 19
	por	xmm7, xmm2				; 18CF _ 66: 0F EB. FA
	pxor	xmm6, xmm1				; 18D3 _ 66: 0F EF. F1
	movd	xmm2, dword [ecx+2BC0H] 		; 18D7 _ 66: 0F 6E. 91, 00002BC0
	pxor	xmm6, xmm7				; 18DF _ 66: 0F EF. F7
	pxor	xmm0, xmm7				; 18E3 _ 66: 0F EF. C7
	pslld	xmm5, 7 				; 18E7 _ 66: 0F 72. F5, 07
	pxor	xmm0, xmm5				; 18EC _ 66: 0F EF. C5
	movdqa	xmm5, xmm6				; 18F0 _ 66: 0F 6F. EE
	psrld	xmm6, 27				; 18F4 _ 66: 0F 72. D6, 1B
	pslld	xmm5, 5 				; 18F9 _ 66: 0F 72. F5, 05
	por	xmm5, xmm6				; 18FE _ 66: 0F EB. EE
	movdqa	xmm6, xmm0				; 1902 _ 66: 0F 6F. F0
	psrld	xmm0, 10				; 1906 _ 66: 0F 72. D0, 0A
	pslld	xmm6, 22				; 190B _ 66: 0F 72. F6, 16
	por	xmm6, xmm0				; 1910 _ 66: 0F EB. F0
	pshufd	xmm0, xmm2, 0				; 1914 _ 66: 0F 70. C2, 00
	pxor	xmm5, xmm0				; 1919 _ 66: 0F EF. E8
	movd	xmm2, dword [ecx+2BC4H] 		; 191D _ 66: 0F 6E. 91, 00002BC4
	pshufd	xmm0, xmm2, 0				; 1925 _ 66: 0F 70. C2, 00
	pxor	xmm1, xmm0				; 192A _ 66: 0F EF. C8
	movd	xmm2, dword [ecx+2BC8H] 		; 192E _ 66: 0F 6E. 91, 00002BC8
	pshufd	xmm0, xmm2, 0				; 1936 _ 66: 0F 70. C2, 00
	pxor	xmm6, xmm0				; 193B _ 66: 0F EF. F0
	movd	xmm2, dword [ecx+2BCCH] 		; 193F _ 66: 0F 6E. 91, 00002BCC
	pshufd	xmm0, xmm2, 0				; 1947 _ 66: 0F 70. C2, 00
	pxor	xmm7, xmm0				; 194C _ 66: 0F EF. F8
	pxor	xmm1, xmm7				; 1950 _ 66: 0F EF. CF
	movdqa	xmm2, xmm1				; 1954 _ 66: 0F 6F. D1
	pxor	xmm7, xmm4				; 1958 _ 66: 0F EF. FC
	pxor	xmm6, xmm7				; 195C _ 66: 0F EF. F7
	pxor	xmm7, xmm5				; 1960 _ 66: 0F EF. FD
	pand	xmm2, xmm7				; 1964 _ 66: 0F DB. D7
	pxor	xmm2, xmm6				; 1968 _ 66: 0F EF. D6
	movdqa	xmm0, xmm2				; 196C _ 66: 0F 6F. C2
	pxor	xmm1, xmm7				; 1970 _ 66: 0F EF. CF
	pxor	xmm5, xmm1				; 1974 _ 66: 0F EF. E9
	pand	xmm6, xmm1				; 1978 _ 66: 0F DB. F1
	pxor	xmm6, xmm5				; 197C _ 66: 0F EF. F5
	pand	xmm5, xmm2				; 1980 _ 66: 0F DB. EA
	pxor	xmm7, xmm5				; 1984 _ 66: 0F EF. FD
	por	xmm1, xmm2				; 1988 _ 66: 0F EB. CA
	pxor	xmm1, xmm5				; 198C _ 66: 0F EF. CD
	por	xmm5, xmm7				; 1990 _ 66: 0F EB. EF
	pxor	xmm5, xmm6				; 1994 _ 66: 0F EF. EE
	pand	xmm6, xmm7				; 1998 _ 66: 0F DB. F7
	pxor	xmm5, xmm4				; 199C _ 66: 0F EF. EC
	pxor	xmm1, xmm6				; 19A0 _ 66: 0F EF. CE
	movdqa	xmm6, xmm5				; 19A4 _ 66: 0F 6F. F5
	pslld	xmm0, 13				; 19A8 _ 66: 0F 72. F0, 0D
	psrld	xmm2, 19				; 19AD _ 66: 0F 72. D2, 13
	por	xmm0, xmm2				; 19B2 _ 66: 0F EB. C2
	pslld	xmm6, 3 				; 19B6 _ 66: 0F 72. F6, 03
	psrld	xmm5, 29				; 19BB _ 66: 0F 72. D5, 1D
	por	xmm6, xmm5				; 19C0 _ 66: 0F EB. F5
	movdqa	xmm5, xmm0				; 19C4 _ 66: 0F 6F. E8
	pxor	xmm1, xmm0				; 19C8 _ 66: 0F EF. C8
	pxor	xmm1, xmm6				; 19CC _ 66: 0F EF. CE
	pxor	xmm7, xmm6				; 19D0 _ 66: 0F EF. FE
	pslld	xmm5, 3 				; 19D4 _ 66: 0F 72. F5, 03
	pxor	xmm7, xmm5				; 19D9 _ 66: 0F EF. FD
	movdqa	xmm5, xmm1				; 19DD _ 66: 0F 6F. E9
	movdqa	xmm2, xmm7				; 19E1 _ 66: 0F 6F. D7
	psrld	xmm1, 31				; 19E5 _ 66: 0F 72. D1, 1F
	pslld	xmm5, 1 				; 19EA _ 66: 0F 72. F5, 01
	por	xmm5, xmm1				; 19EF _ 66: 0F EB. E9
	pslld	xmm2, 7 				; 19F3 _ 66: 0F 72. F2, 07
	psrld	xmm7, 25				; 19F8 _ 66: 0F 72. D7, 19
	por	xmm2, xmm7				; 19FD _ 66: 0F EB. D7
	movdqa	xmm7, xmm5				; 1A01 _ 66: 0F 6F. FD
	pxor	xmm0, xmm5				; 1A05 _ 66: 0F EF. C5
	pxor	xmm0, xmm2				; 1A09 _ 66: 0F EF. C2
	movdqa	xmm1, xmm0				; 1A0D _ 66: 0F 6F. C8
	pxor	xmm6, xmm2				; 1A11 _ 66: 0F EF. F2
	pslld	xmm7, 7 				; 1A15 _ 66: 0F 72. F7, 07
	pxor	xmm6, xmm7				; 1A1A _ 66: 0F EF. F7
	pslld	xmm1, 5 				; 1A1E _ 66: 0F 72. F1, 05
	psrld	xmm0, 27				; 1A23 _ 66: 0F 72. D0, 1B
	por	xmm1, xmm0				; 1A28 _ 66: 0F EB. C8
	movdqa	xmm0, xmm6				; 1A2C _ 66: 0F 6F. C6
	psrld	xmm6, 10				; 1A30 _ 66: 0F 72. D6, 0A
	pslld	xmm0, 22				; 1A35 _ 66: 0F 72. F0, 16
	por	xmm0, xmm6				; 1A3A _ 66: 0F EB. C6
	movd	xmm6, dword [ecx+2BD0H] 		; 1A3E _ 66: 0F 6E. B1, 00002BD0
	pshufd	xmm6, xmm6, 0				; 1A46 _ 66: 0F 70. F6, 00
	pxor	xmm1, xmm6				; 1A4B _ 66: 0F EF. CE
	movd	xmm7, dword [ecx+2BD4H] 		; 1A4F _ 66: 0F 6E. B9, 00002BD4
	pshufd	xmm6, xmm7, 0				; 1A57 _ 66: 0F 70. F7, 00
	pxor	xmm5, xmm6				; 1A5C _ 66: 0F EF. EE
	pxor	xmm1, xmm5				; 1A60 _ 66: 0F EF. CD
	movd	xmm7, dword [ecx+2BD8H] 		; 1A64 _ 66: 0F 6E. B9, 00002BD8
	pshufd	xmm6, xmm7, 0				; 1A6C _ 66: 0F 70. F7, 00
	pxor	xmm0, xmm6				; 1A71 _ 66: 0F EF. C6
	movd	xmm7, dword [ecx+2BDCH] 		; 1A75 _ 66: 0F 6E. B9, 00002BDC
	pshufd	xmm6, xmm7, 0				; 1A7D _ 66: 0F 70. F7, 00
	pxor	xmm2, xmm6				; 1A82 _ 66: 0F EF. D6
	pxor	xmm5, xmm2				; 1A86 _ 66: 0F EF. EA
	movdqa	xmm7, xmm5				; 1A8A _ 66: 0F 6F. FD
	pxor	xmm2, xmm4				; 1A8E _ 66: 0F EF. D4
	pxor	xmm0, xmm2				; 1A92 _ 66: 0F EF. C2
	pand	xmm7, xmm1				; 1A96 _ 66: 0F DB. F9
	pxor	xmm7, xmm0				; 1A9A _ 66: 0F EF. F8
	movdqa	xmm6, xmm7				; 1A9E _ 66: 0F 6F. F7
	por	xmm0, xmm5				; 1AA2 _ 66: 0F EB. C5
	pxor	xmm5, xmm2				; 1AA6 _ 66: 0F EF. EA
	pand	xmm2, xmm7				; 1AAA _ 66: 0F DB. D7
	pxor	xmm2, xmm1				; 1AAE _ 66: 0F EF. D1
	pxor	xmm5, xmm7				; 1AB2 _ 66: 0F EF. EF
	pxor	xmm5, xmm0				; 1AB6 _ 66: 0F EF. E8
	pxor	xmm0, xmm1				; 1ABA _ 66: 0F EF. C1
	pand	xmm1, xmm2				; 1ABE _ 66: 0F DB. CA
	pxor	xmm0, xmm4				; 1AC2 _ 66: 0F EF. C4
	pxor	xmm1, xmm5				; 1AC6 _ 66: 0F EF. CD
	por	xmm5, xmm2				; 1ACA _ 66: 0F EB. EA
	pxor	xmm5, xmm0				; 1ACE _ 66: 0F EF. E8
	movdqa	xmm0, xmm1				; 1AD2 _ 66: 0F 6F. C1
	pslld	xmm6, 13				; 1AD6 _ 66: 0F 72. F6, 0D
	psrld	xmm7, 19				; 1ADB _ 66: 0F 72. D7, 13
	por	xmm6, xmm7				; 1AE0 _ 66: 0F EB. F7
	pslld	xmm0, 3 				; 1AE4 _ 66: 0F 72. F0, 03
	psrld	xmm1, 29				; 1AE9 _ 66: 0F 72. D1, 1D
	por	xmm0, xmm1				; 1AEE _ 66: 0F EB. C1
	movdqa	xmm1, xmm6				; 1AF2 _ 66: 0F 6F. CE
	pxor	xmm2, xmm6				; 1AF6 _ 66: 0F EF. D6
	pxor	xmm2, xmm0				; 1AFA _ 66: 0F EF. D0
	pxor	xmm5, xmm0				; 1AFE _ 66: 0F EF. E8
	pslld	xmm1, 3 				; 1B02 _ 66: 0F 72. F1, 03
	pxor	xmm5, xmm1				; 1B07 _ 66: 0F EF. E9
	movdqa	xmm1, xmm2				; 1B0B _ 66: 0F 6F. CA
	movdqa	xmm7, xmm5				; 1B0F _ 66: 0F 6F. FD
	psrld	xmm2, 31				; 1B13 _ 66: 0F 72. D2, 1F
	pslld	xmm1, 1 				; 1B18 _ 66: 0F 72. F1, 01
	por	xmm1, xmm2				; 1B1D _ 66: 0F EB. CA
	movdqa	xmm2, xmm1				; 1B21 _ 66: 0F 6F. D1
	pslld	xmm7, 7 				; 1B25 _ 66: 0F 72. F7, 07
	psrld	xmm5, 25				; 1B2A _ 66: 0F 72. D5, 19
	por	xmm7, xmm5				; 1B2F _ 66: 0F EB. FD
	pxor	xmm6, xmm1				; 1B33 _ 66: 0F EF. F1
	pxor	xmm6, xmm7				; 1B37 _ 66: 0F EF. F7
	movdqa	xmm5, xmm6				; 1B3B _ 66: 0F 6F. EE
	pxor	xmm0, xmm7				; 1B3F _ 66: 0F EF. C7
	pslld	xmm2, 7 				; 1B43 _ 66: 0F 72. F2, 07
	pxor	xmm0, xmm2				; 1B48 _ 66: 0F EF. C2
	movdqa	xmm2, xmm0				; 1B4C _ 66: 0F 6F. D0
	pslld	xmm5, 5 				; 1B50 _ 66: 0F 72. F5, 05
	psrld	xmm6, 27				; 1B55 _ 66: 0F 72. D6, 1B
	por	xmm5, xmm6				; 1B5A _ 66: 0F EB. EE
	pslld	xmm2, 22				; 1B5E _ 66: 0F 72. F2, 16
	psrld	xmm0, 10				; 1B63 _ 66: 0F 72. D0, 0A
	por	xmm2, xmm0				; 1B68 _ 66: 0F EB. D0
	movd	xmm0, dword [ecx+2BE0H] 		; 1B6C _ 66: 0F 6E. 81, 00002BE0
	pshufd	xmm6, xmm0, 0				; 1B74 _ 66: 0F 70. F0, 00
	pxor	xmm5, xmm6				; 1B79 _ 66: 0F EF. EE
	movd	xmm0, dword [ecx+2BE4H] 		; 1B7D _ 66: 0F 6E. 81, 00002BE4
	pshufd	xmm6, xmm0, 0				; 1B85 _ 66: 0F 70. F0, 00
	pxor	xmm1, xmm6				; 1B8A _ 66: 0F EF. CE
	movd	xmm0, dword [ecx+2BE8H] 		; 1B8E _ 66: 0F 6E. 81, 00002BE8
	pshufd	xmm6, xmm0, 0				; 1B96 _ 66: 0F 70. F0, 00
	movd	xmm0, dword [ecx+2BECH] 		; 1B9B _ 66: 0F 6E. 81, 00002BEC
	pxor	xmm2, xmm6				; 1BA3 _ 66: 0F EF. D6
	pshufd	xmm6, xmm0, 0				; 1BA7 _ 66: 0F 70. F0, 00
	pxor	xmm7, xmm6				; 1BAC _ 66: 0F EF. FE
	movdqa	xmm0, xmm7				; 1BB0 _ 66: 0F 6F. C7
	pxor	xmm2, xmm4				; 1BB4 _ 66: 0F EF. D4
	pand	xmm0, xmm5				; 1BB8 _ 66: 0F DB. C5
	pxor	xmm5, xmm7				; 1BBC _ 66: 0F EF. EF
	pxor	xmm0, xmm2				; 1BC0 _ 66: 0F EF. C2
	por	xmm2, xmm7				; 1BC4 _ 66: 0F EB. D7
	pxor	xmm1, xmm0				; 1BC8 _ 66: 0F EF. C8
	pxor	xmm2, xmm5				; 1BCC _ 66: 0F EF. D5
	por	xmm5, xmm1				; 1BD0 _ 66: 0F EB. E9
	pxor	xmm2, xmm1				; 1BD4 _ 66: 0F EF. D1
	pxor	xmm7, xmm5				; 1BD8 _ 66: 0F EF. FD
	por	xmm5, xmm0				; 1BDC _ 66: 0F EB. E8
	pxor	xmm5, xmm2				; 1BE0 _ 66: 0F EF. EA
	movdqa	xmm6, xmm5				; 1BE4 _ 66: 0F 6F. F5
	pxor	xmm7, xmm0				; 1BE8 _ 66: 0F EF. F8
	pxor	xmm7, xmm5				; 1BEC _ 66: 0F EF. FD
	pxor	xmm0, xmm4				; 1BF0 _ 66: 0F EF. C4
	pand	xmm2, xmm7				; 1BF4 _ 66: 0F DB. D7
	pxor	xmm0, xmm2				; 1BF8 _ 66: 0F EF. C2
	pslld	xmm6, 13				; 1BFC _ 66: 0F 72. F6, 0D
	psrld	xmm5, 19				; 1C01 _ 66: 0F 72. D5, 13
	por	xmm6, xmm5				; 1C06 _ 66: 0F EB. F5
	movdqa	xmm5, xmm7				; 1C0A _ 66: 0F 6F. EF
	psrld	xmm7, 29				; 1C0E _ 66: 0F 72. D7, 1D
	pxor	xmm1, xmm6				; 1C13 _ 66: 0F EF. CE
	pslld	xmm5, 3 				; 1C17 _ 66: 0F 72. F5, 03
	por	xmm5, xmm7				; 1C1C _ 66: 0F EB. EF
	movdqa	xmm7, xmm6				; 1C20 _ 66: 0F 6F. FE
	pxor	xmm1, xmm5				; 1C24 _ 66: 0F EF. CD
	movdqa	xmm2, xmm1				; 1C28 _ 66: 0F 6F. D1
	pxor	xmm0, xmm5				; 1C2C _ 66: 0F EF. C5
	pslld	xmm7, 3 				; 1C30 _ 66: 0F 72. F7, 03
	pxor	xmm0, xmm7				; 1C35 _ 66: 0F EF. C7
	movd	xmm7, dword [ecx+2BF4H] 		; 1C39 _ 66: 0F 6E. B9, 00002BF4
	pslld	xmm2, 1 				; 1C41 _ 66: 0F 72. F2, 01
	psrld	xmm1, 31				; 1C46 _ 66: 0F 72. D1, 1F
	por	xmm2, xmm1				; 1C4B _ 66: 0F EB. D1
	movdqa	xmm1, xmm0				; 1C4F _ 66: 0F 6F. C8
	psrld	xmm0, 25				; 1C53 _ 66: 0F 72. D0, 19
	pxor	xmm6, xmm2				; 1C58 _ 66: 0F EF. F2
	pslld	xmm1, 7 				; 1C5C _ 66: 0F 72. F1, 07
	por	xmm1, xmm0				; 1C61 _ 66: 0F EB. C8
	movdqa	xmm0, xmm2				; 1C65 _ 66: 0F 6F. C2
	pxor	xmm6, xmm1				; 1C69 _ 66: 0F EF. F1
	pxor	xmm5, xmm1				; 1C6D _ 66: 0F EF. E9
	pslld	xmm0, 7 				; 1C71 _ 66: 0F 72. F0, 07
	pxor	xmm5, xmm0				; 1C76 _ 66: 0F EF. E8
	movdqa	xmm0, xmm6				; 1C7A _ 66: 0F 6F. C6
	psrld	xmm6, 27				; 1C7E _ 66: 0F 72. D6, 1B
	pslld	xmm0, 5 				; 1C83 _ 66: 0F 72. F0, 05
	por	xmm0, xmm6				; 1C88 _ 66: 0F EB. C6
	movdqa	xmm6, xmm5				; 1C8C _ 66: 0F 6F. F5
	psrld	xmm5, 10				; 1C90 _ 66: 0F 72. D5, 0A
	pslld	xmm6, 22				; 1C95 _ 66: 0F 72. F6, 16
	por	xmm6, xmm5				; 1C9A _ 66: 0F EB. F5
	movd	xmm5, dword [ecx+2BF0H] 		; 1C9E _ 66: 0F 6E. A9, 00002BF0
	pshufd	xmm5, xmm5, 0				; 1CA6 _ 66: 0F 70. ED, 00
	pxor	xmm0, xmm5				; 1CAB _ 66: 0F EF. C5
	pshufd	xmm5, xmm7, 0				; 1CAF _ 66: 0F 70. EF, 00
	movd	xmm7, dword [ecx+2BF8H] 		; 1CB4 _ 66: 0F 6E. B9, 00002BF8
	pxor	xmm2, xmm5				; 1CBC _ 66: 0F EF. D5
	pshufd	xmm5, xmm7, 0				; 1CC0 _ 66: 0F 70. EF, 00
	pxor	xmm6, xmm5				; 1CC5 _ 66: 0F EF. F5
	movd	xmm7, dword [ecx+2BFCH] 		; 1CC9 _ 66: 0F 6E. B9, 00002BFC
	pshufd	xmm5, xmm7, 0				; 1CD1 _ 66: 0F 70. EF, 00
	movdqa	xmm7, xmm2				; 1CD6 _ 66: 0F 6F. FA
	pxor	xmm1, xmm5				; 1CDA _ 66: 0F EF. CD
	pxor	xmm2, xmm6				; 1CDE _ 66: 0F EF. D6
	por	xmm7, xmm6				; 1CE2 _ 66: 0F EB. FE
	pxor	xmm7, xmm1				; 1CE6 _ 66: 0F EF. F9
	pxor	xmm6, xmm7				; 1CEA _ 66: 0F EF. F7
	por	xmm1, xmm2				; 1CEE _ 66: 0F EB. CA
	pand	xmm1, xmm0				; 1CF2 _ 66: 0F DB. C8
	pxor	xmm2, xmm6				; 1CF6 _ 66: 0F EF. D6
	pxor	xmm1, xmm7				; 1CFA _ 66: 0F EF. CF
	por	xmm7, xmm2				; 1CFE _ 66: 0F EB. FA
	pxor	xmm7, xmm0				; 1D02 _ 66: 0F EF. F8
	por	xmm0, xmm2				; 1D06 _ 66: 0F EB. C2
	pxor	xmm0, xmm6				; 1D0A _ 66: 0F EF. C6
	pxor	xmm7, xmm2				; 1D0E _ 66: 0F EF. FA
	pxor	xmm6, xmm7				; 1D12 _ 66: 0F EF. F7
	pand	xmm7, xmm0				; 1D16 _ 66: 0F DB. F8
	pxor	xmm7, xmm2				; 1D1A _ 66: 0F EF. FA
	pxor	xmm6, xmm4				; 1D1E _ 66: 0F EF. F4
	por	xmm6, xmm0				; 1D22 _ 66: 0F EB. F0
	pxor	xmm2, xmm6				; 1D26 _ 66: 0F EF. D6
	movdqa	xmm5, xmm2				; 1D2A _ 66: 0F 6F. EA
	movdqa	xmm6, xmm7				; 1D2E _ 66: 0F 6F. F7
	psrld	xmm2, 19				; 1D32 _ 66: 0F 72. D2, 13
	pslld	xmm5, 13				; 1D37 _ 66: 0F 72. F5, 0D
	por	xmm5, xmm2				; 1D3C _ 66: 0F EB. EA
	movdqa	xmm2, xmm5				; 1D40 _ 66: 0F 6F. D5
	pslld	xmm6, 3 				; 1D44 _ 66: 0F 72. F6, 03
	psrld	xmm7, 29				; 1D49 _ 66: 0F 72. D7, 1D
	por	xmm6, xmm7				; 1D4E _ 66: 0F EB. F7
	pxor	xmm1, xmm5				; 1D52 _ 66: 0F EF. CD
	pxor	xmm1, xmm6				; 1D56 _ 66: 0F EF. CE
	movdqa	xmm7, xmm1				; 1D5A _ 66: 0F 6F. F9
	pxor	xmm0, xmm6				; 1D5E _ 66: 0F EF. C6
	pslld	xmm2, 3 				; 1D62 _ 66: 0F 72. F2, 03
	pxor	xmm0, xmm2				; 1D67 _ 66: 0F EF. C2
	pslld	xmm7, 1 				; 1D6B _ 66: 0F 72. F7, 01
	psrld	xmm1, 31				; 1D70 _ 66: 0F 72. D1, 1F
	por	xmm7, xmm1				; 1D75 _ 66: 0F EB. F9
	movdqa	xmm1, xmm0				; 1D79 _ 66: 0F 6F. C8
	psrld	xmm0, 25				; 1D7D _ 66: 0F 72. D0, 19
	pxor	xmm5, xmm7				; 1D82 _ 66: 0F EF. EF
	pslld	xmm1, 7 				; 1D86 _ 66: 0F 72. F1, 07
	por	xmm1, xmm0				; 1D8B _ 66: 0F EB. C8
	movdqa	xmm0, xmm7				; 1D8F _ 66: 0F 6F. C7
	pxor	xmm5, xmm1				; 1D93 _ 66: 0F EF. E9
	pxor	xmm6, xmm1				; 1D97 _ 66: 0F EF. F1
	pslld	xmm0, 7 				; 1D9B _ 66: 0F 72. F0, 07
	pxor	xmm6, xmm0				; 1DA0 _ 66: 0F EF. F0
	movdqa	xmm0, xmm5				; 1DA4 _ 66: 0F 6F. C5
	psrld	xmm5, 27				; 1DA8 _ 66: 0F 72. D5, 1B
	pslld	xmm0, 5 				; 1DAD _ 66: 0F 72. F0, 05
	por	xmm0, xmm5				; 1DB2 _ 66: 0F EB. C5
	movdqa	xmm5, xmm6				; 1DB6 _ 66: 0F 6F. EE
	psrld	xmm6, 10				; 1DBA _ 66: 0F 72. D6, 0A
	pslld	xmm5, 22				; 1DBF _ 66: 0F 72. F5, 16
	por	xmm5, xmm6				; 1DC4 _ 66: 0F EB. EE
	movd	xmm6, dword [ecx+2C00H] 		; 1DC8 _ 66: 0F 6E. B1, 00002C00
	pshufd	xmm2, xmm6, 0				; 1DD0 _ 66: 0F 70. D6, 00
	pxor	xmm0, xmm2				; 1DD5 _ 66: 0F EF. C2
	movd	xmm6, dword [ecx+2C04H] 		; 1DD9 _ 66: 0F 6E. B1, 00002C04
	pshufd	xmm2, xmm6, 0				; 1DE1 _ 66: 0F 70. D6, 00
	pxor	xmm7, xmm2				; 1DE6 _ 66: 0F EF. FA
	movd	xmm6, dword [ecx+2C08H] 		; 1DEA _ 66: 0F 6E. B1, 00002C08
	pshufd	xmm2, xmm6, 0				; 1DF2 _ 66: 0F 70. D6, 00
	movd	xmm6, dword [ecx+2C0CH] 		; 1DF7 _ 66: 0F 6E. B1, 00002C0C
	pxor	xmm5, xmm2				; 1DFF _ 66: 0F EF. EA
	pshufd	xmm2, xmm6, 0				; 1E03 _ 66: 0F 70. D6, 00
	pxor	xmm1, xmm2				; 1E08 _ 66: 0F EF. CA
	movdqa	xmm2, xmm7				; 1E0C _ 66: 0F 6F. D7
	pxor	xmm1, xmm0				; 1E10 _ 66: 0F EF. C8
	pxor	xmm7, xmm5				; 1E14 _ 66: 0F EF. FD
	pand	xmm2, xmm1				; 1E18 _ 66: 0F DB. D1
	pxor	xmm2, xmm0				; 1E1C _ 66: 0F EF. D0
	por	xmm0, xmm1				; 1E20 _ 66: 0F EB. C1
	pxor	xmm0, xmm7				; 1E24 _ 66: 0F EF. C7
	pxor	xmm7, xmm1				; 1E28 _ 66: 0F EF. F9
	pxor	xmm1, xmm5				; 1E2C _ 66: 0F EF. CD
	por	xmm5, xmm2				; 1E30 _ 66: 0F EB. EA
	pxor	xmm5, xmm7				; 1E34 _ 66: 0F EF. EF
	pxor	xmm7, xmm4				; 1E38 _ 66: 0F EF. FC
	por	xmm7, xmm2				; 1E3C _ 66: 0F EB. FA
	pxor	xmm2, xmm1				; 1E40 _ 66: 0F EF. D1
	pxor	xmm2, xmm7				; 1E44 _ 66: 0F EF. D7
	por	xmm1, xmm0				; 1E48 _ 66: 0F EB. C8
	pxor	xmm2, xmm1				; 1E4C _ 66: 0F EF. D1
	movdqa	xmm6, xmm2				; 1E50 _ 66: 0F 6F. F2
	pxor	xmm7, xmm1				; 1E54 _ 66: 0F EF. F9
	psrld	xmm2, 19				; 1E58 _ 66: 0F 72. D2, 13
	pslld	xmm6, 13				; 1E5D _ 66: 0F 72. F6, 0D
	por	xmm6, xmm2				; 1E62 _ 66: 0F EB. F2
	movdqa	xmm2, xmm5				; 1E66 _ 66: 0F 6F. D5
	movdqa	xmm1, xmm6				; 1E6A _ 66: 0F 6F. CE
	psrld	xmm5, 29				; 1E6E _ 66: 0F 72. D5, 1D
	pslld	xmm2, 3 				; 1E73 _ 66: 0F 72. F2, 03
	por	xmm2, xmm5				; 1E78 _ 66: 0F EB. D5
	pxor	xmm7, xmm6				; 1E7C _ 66: 0F EF. FE
	pxor	xmm7, xmm2				; 1E80 _ 66: 0F EF. FA
	pxor	xmm0, xmm2				; 1E84 _ 66: 0F EF. C2
	pslld	xmm1, 3 				; 1E88 _ 66: 0F 72. F1, 03
	pxor	xmm0, xmm1				; 1E8D _ 66: 0F EF. C1
	movdqa	xmm1, xmm7				; 1E91 _ 66: 0F 6F. CF
	movdqa	xmm5, xmm0				; 1E95 _ 66: 0F 6F. E8
	psrld	xmm7, 31				; 1E99 _ 66: 0F 72. D7, 1F
	pslld	xmm1, 1 				; 1E9E _ 66: 0F 72. F1, 01
	por	xmm1, xmm7				; 1EA3 _ 66: 0F EB. CF
	movdqa	xmm7, xmm1				; 1EA7 _ 66: 0F 6F. F9
	pslld	xmm5, 7 				; 1EAB _ 66: 0F 72. F5, 07
	psrld	xmm0, 25				; 1EB0 _ 66: 0F 72. D0, 19
	por	xmm5, xmm0				; 1EB5 _ 66: 0F EB. E8
	pxor	xmm6, xmm1				; 1EB9 _ 66: 0F EF. F1
	pxor	xmm6, xmm5				; 1EBD _ 66: 0F EF. F5
	movdqa	xmm0, xmm6				; 1EC1 _ 66: 0F 6F. C6
	pxor	xmm2, xmm5				; 1EC5 _ 66: 0F EF. D5
	pslld	xmm7, 7 				; 1EC9 _ 66: 0F 72. F7, 07
	pxor	xmm2, xmm7				; 1ECE _ 66: 0F EF. D7
	pslld	xmm0, 5 				; 1ED2 _ 66: 0F 72. F0, 05
	psrld	xmm6, 27				; 1ED7 _ 66: 0F 72. D6, 1B
	por	xmm0, xmm6				; 1EDC _ 66: 0F EB. C6
	movdqa	xmm6, xmm2				; 1EE0 _ 66: 0F 6F. F2
	psrld	xmm2, 10				; 1EE4 _ 66: 0F 72. D2, 0A
	pslld	xmm6, 22				; 1EE9 _ 66: 0F 72. F6, 16
	por	xmm6, xmm2				; 1EEE _ 66: 0F EB. F2
	movd	xmm2, dword [ecx+2C10H] 		; 1EF2 _ 66: 0F 6E. 91, 00002C10
	pshufd	xmm7, xmm2, 0				; 1EFA _ 66: 0F 70. FA, 00
	pxor	xmm0, xmm7				; 1EFF _ 66: 0F EF. C7
	pxor	xmm0, xmm4				; 1F03 _ 66: 0F EF. C4
	movd	xmm2, dword [ecx+2C14H] 		; 1F07 _ 66: 0F 6E. 91, 00002C14
	pshufd	xmm7, xmm2, 0				; 1F0F _ 66: 0F 70. FA, 00
	pxor	xmm1, xmm7				; 1F14 _ 66: 0F EF. CF
	movd	xmm2, dword [ecx+2C18H] 		; 1F18 _ 66: 0F 6E. 91, 00002C18
	pshufd	xmm7, xmm2, 0				; 1F20 _ 66: 0F 70. FA, 00
	pxor	xmm6, xmm7				; 1F25 _ 66: 0F EF. F7
	pxor	xmm6, xmm4				; 1F29 _ 66: 0F EF. F4
	movd	xmm2, dword [ecx+2C1CH] 		; 1F2D _ 66: 0F 6E. 91, 00002C1C
	pshufd	xmm7, xmm2, 0				; 1F35 _ 66: 0F 70. FA, 00
	movdqa	xmm2, xmm0				; 1F3A _ 66: 0F 6F. D0
	pxor	xmm5, xmm7				; 1F3E _ 66: 0F EF. EF
	pand	xmm2, xmm1				; 1F42 _ 66: 0F DB. D1
	pxor	xmm6, xmm2				; 1F46 _ 66: 0F EF. F2
	por	xmm2, xmm5				; 1F4A _ 66: 0F EB. D5
	pxor	xmm5, xmm6				; 1F4E _ 66: 0F EF. EE
	pxor	xmm1, xmm2				; 1F52 _ 66: 0F EF. CA
	pxor	xmm2, xmm0				; 1F56 _ 66: 0F EF. D0
	por	xmm0, xmm1				; 1F5A _ 66: 0F EB. C1
	pxor	xmm1, xmm5				; 1F5E _ 66: 0F EF. CD
	por	xmm6, xmm2				; 1F62 _ 66: 0F EB. F2
	pand	xmm6, xmm0				; 1F66 _ 66: 0F DB. F0
	pxor	xmm2, xmm1				; 1F6A _ 66: 0F EF. D1
	pand	xmm1, xmm6				; 1F6E _ 66: 0F DB. CE
	pxor	xmm1, xmm2				; 1F72 _ 66: 0F EF. CA
	pand	xmm2, xmm6				; 1F76 _ 66: 0F DB. D6
	pxor	xmm0, xmm2				; 1F7A _ 66: 0F EF. C2
	movdqa	xmm2, xmm6				; 1F7E _ 66: 0F 6F. D6
	psrld	xmm6, 19				; 1F82 _ 66: 0F 72. D6, 13
	pslld	xmm2, 13				; 1F87 _ 66: 0F 72. F2, 0D
	por	xmm2, xmm6				; 1F8C _ 66: 0F EB. D6
	movdqa	xmm6, xmm5				; 1F90 _ 66: 0F 6F. F5
	psrld	xmm5, 29				; 1F94 _ 66: 0F 72. D5, 1D
	pxor	xmm0, xmm2				; 1F99 _ 66: 0F EF. C2
	pslld	xmm6, 3 				; 1F9D _ 66: 0F 72. F6, 03
	por	xmm6, xmm5				; 1FA2 _ 66: 0F EB. F5
	movdqa	xmm5, xmm2				; 1FA6 _ 66: 0F 6F. EA
	pxor	xmm0, xmm6				; 1FAA _ 66: 0F EF. C6
	pxor	xmm1, xmm6				; 1FAE _ 66: 0F EF. CE
	pslld	xmm5, 3 				; 1FB2 _ 66: 0F 72. F5, 03
	pxor	xmm1, xmm5				; 1FB7 _ 66: 0F EF. CD
	movdqa	xmm5, xmm0				; 1FBB _ 66: 0F 6F. E8
	psrld	xmm0, 31				; 1FBF _ 66: 0F 72. D0, 1F
	pslld	xmm5, 1 				; 1FC4 _ 66: 0F 72. F5, 01
	por	xmm5, xmm0				; 1FC9 _ 66: 0F EB. E8
	movdqa	xmm0, xmm1				; 1FCD _ 66: 0F 6F. C1
	psrld	xmm1, 25				; 1FD1 _ 66: 0F 72. D1, 19
	pxor	xmm2, xmm5				; 1FD6 _ 66: 0F EF. D5
	pslld	xmm0, 7 				; 1FDA _ 66: 0F 72. F0, 07
	por	xmm0, xmm1				; 1FDF _ 66: 0F EB. C1
	movdqa	xmm1, xmm5				; 1FE3 _ 66: 0F 6F. CD
	pxor	xmm2, xmm0				; 1FE7 _ 66: 0F EF. D0
	movdqa	xmm7, xmm2				; 1FEB _ 66: 0F 6F. FA
	pxor	xmm6, xmm0				; 1FEF _ 66: 0F EF. F0
	pslld	xmm1, 7 				; 1FF3 _ 66: 0F 72. F1, 07
	pxor	xmm6, xmm1				; 1FF8 _ 66: 0F EF. F1
	movdqa	xmm1, xmm6				; 1FFC _ 66: 0F 6F. CE
	pslld	xmm7, 5 				; 2000 _ 66: 0F 72. F7, 05
	psrld	xmm2, 27				; 2005 _ 66: 0F 72. D2, 1B
	por	xmm7, xmm2				; 200A _ 66: 0F EB. FA
	pslld	xmm1, 22				; 200E _ 66: 0F 72. F1, 16
	psrld	xmm6, 10				; 2013 _ 66: 0F 72. D6, 0A
	por	xmm1, xmm6				; 2018 _ 66: 0F EB. CE
	movd	xmm6, dword [ecx+2C20H] 		; 201C _ 66: 0F 6E. B1, 00002C20
	pshufd	xmm2, xmm6, 0				; 2024 _ 66: 0F 70. D6, 00
	pxor	xmm7, xmm2				; 2029 _ 66: 0F EF. FA
	movd	xmm6, dword [ecx+2C24H] 		; 202D _ 66: 0F 6E. B1, 00002C24
	pshufd	xmm2, xmm6, 0				; 2035 _ 66: 0F 70. D6, 00
	pxor	xmm5, xmm2				; 203A _ 66: 0F EF. EA
	movd	xmm6, dword [ecx+2C28H] 		; 203E _ 66: 0F 6E. B1, 00002C28
	pshufd	xmm2, xmm6, 0				; 2046 _ 66: 0F 70. D6, 00
	pxor	xmm1, xmm2				; 204B _ 66: 0F EF. CA
	movd	xmm6, dword [ecx+2C2CH] 		; 204F _ 66: 0F 6E. B1, 00002C2C
	pshufd	xmm2, xmm6, 0				; 2057 _ 66: 0F 70. D6, 00
	movdqa	xmm6, xmm7				; 205C _ 66: 0F 6F. F7
	pxor	xmm0, xmm2				; 2060 _ 66: 0F EF. C2
	pand	xmm6, xmm1				; 2064 _ 66: 0F DB. F1
	pxor	xmm6, xmm0				; 2068 _ 66: 0F EF. F0
	pxor	xmm1, xmm5				; 206C _ 66: 0F EF. CD
	pxor	xmm1, xmm6				; 2070 _ 66: 0F EF. CE
	movdqa	xmm2, xmm1				; 2074 _ 66: 0F 6F. D1
	por	xmm0, xmm7				; 2078 _ 66: 0F EB. C7
	pxor	xmm0, xmm5				; 207C _ 66: 0F EF. C5
	movdqa	xmm5, xmm0				; 2080 _ 66: 0F 6F. E8
	pxor	xmm7, xmm1				; 2084 _ 66: 0F EF. F9
	pslld	xmm2, 13				; 2088 _ 66: 0F 72. F2, 0D
	por	xmm5, xmm7				; 208D _ 66: 0F EB. EF
	pxor	xmm5, xmm6				; 2091 _ 66: 0F EF. EE
	pand	xmm6, xmm0				; 2095 _ 66: 0F DB. F0
	pxor	xmm7, xmm6				; 2099 _ 66: 0F EF. FE
	pxor	xmm0, xmm5				; 209D _ 66: 0F EF. C5
	pxor	xmm0, xmm7				; 20A1 _ 66: 0F EF. C7
	pxor	xmm7, xmm4				; 20A5 _ 66: 0F EF. FC
	psrld	xmm1, 19				; 20A9 _ 66: 0F 72. D1, 13
	por	xmm2, xmm1				; 20AE _ 66: 0F EB. D1
	movdqa	xmm1, xmm0				; 20B2 _ 66: 0F 6F. C8
	psrld	xmm0, 29				; 20B6 _ 66: 0F 72. D0, 1D
	pxor	xmm5, xmm2				; 20BB _ 66: 0F EF. EA
	pslld	xmm1, 3 				; 20BF _ 66: 0F 72. F1, 03
	por	xmm1, xmm0				; 20C4 _ 66: 0F EB. C8
	movdqa	xmm0, xmm2				; 20C8 _ 66: 0F 6F. C2
	pxor	xmm5, xmm1				; 20CC _ 66: 0F EF. E9
	pxor	xmm7, xmm1				; 20D0 _ 66: 0F EF. F9
	pslld	xmm0, 3 				; 20D4 _ 66: 0F 72. F0, 03
	pxor	xmm7, xmm0				; 20D9 _ 66: 0F EF. F8
	movdqa	xmm0, xmm5				; 20DD _ 66: 0F 6F. C5
	movdqa	xmm6, xmm7				; 20E1 _ 66: 0F 6F. F7
	psrld	xmm5, 31				; 20E5 _ 66: 0F 72. D5, 1F
	pslld	xmm0, 1 				; 20EA _ 66: 0F 72. F0, 01
	por	xmm0, xmm5				; 20EF _ 66: 0F EB. C5
	pslld	xmm6, 7 				; 20F3 _ 66: 0F 72. F6, 07
	psrld	xmm7, 25				; 20F8 _ 66: 0F 72. D7, 19
	por	xmm6, xmm7				; 20FD _ 66: 0F EB. F7
	movdqa	xmm7, xmm0				; 2101 _ 66: 0F 6F. F8
	pxor	xmm2, xmm0				; 2105 _ 66: 0F EF. D0
	pxor	xmm2, xmm6				; 2109 _ 66: 0F EF. D6
	movdqa	xmm5, xmm2				; 210D _ 66: 0F 6F. EA
	pxor	xmm1, xmm6				; 2111 _ 66: 0F EF. CE
	pslld	xmm7, 7 				; 2115 _ 66: 0F 72. F7, 07
	pxor	xmm1, xmm7				; 211A _ 66: 0F EF. CF
	pslld	xmm5, 5 				; 211E _ 66: 0F 72. F5, 05
	movd	xmm7, dword [ecx+2C34H] 		; 2123 _ 66: 0F 6E. B9, 00002C34
	psrld	xmm2, 27				; 212B _ 66: 0F 72. D2, 1B
	por	xmm5, xmm2				; 2130 _ 66: 0F EB. EA
	movdqa	xmm2, xmm1				; 2134 _ 66: 0F 6F. D1
	psrld	xmm1, 10				; 2138 _ 66: 0F 72. D1, 0A
	pslld	xmm2, 22				; 213D _ 66: 0F 72. F2, 16
	por	xmm2, xmm1				; 2142 _ 66: 0F EB. D1
	movd	xmm1, dword [ecx+2C30H] 		; 2146 _ 66: 0F 6E. 89, 00002C30
	pshufd	xmm1, xmm1, 0				; 214E _ 66: 0F 70. C9, 00
	pxor	xmm5, xmm1				; 2153 _ 66: 0F EF. E9
	pshufd	xmm1, xmm7, 0				; 2157 _ 66: 0F 70. CF, 00
	pxor	xmm0, xmm1				; 215C _ 66: 0F EF. C1
	movd	xmm7, dword [ecx+2C38H] 		; 2160 _ 66: 0F 6E. B9, 00002C38
	pshufd	xmm1, xmm7, 0				; 2168 _ 66: 0F 70. CF, 00
	pxor	xmm2, xmm1				; 216D _ 66: 0F EF. D1
	movd	xmm7, dword [ecx+2C3CH] 		; 2171 _ 66: 0F 6E. B9, 00002C3C
	pshufd	xmm1, xmm7, 0				; 2179 _ 66: 0F 70. CF, 00
	movdqa	xmm7, xmm5				; 217E _ 66: 0F 6F. FD
	pxor	xmm6, xmm1				; 2182 _ 66: 0F EF. F1
	por	xmm7, xmm6				; 2186 _ 66: 0F EB. FE
	pxor	xmm6, xmm0				; 218A _ 66: 0F EF. F0
	pand	xmm0, xmm5				; 218E _ 66: 0F DB. C5
	pxor	xmm5, xmm2				; 2192 _ 66: 0F EF. EA
	pxor	xmm2, xmm6				; 2196 _ 66: 0F EF. D6
	pand	xmm6, xmm7				; 219A _ 66: 0F DB. F7
	por	xmm5, xmm0				; 219E _ 66: 0F EB. E8
	pxor	xmm6, xmm5				; 21A2 _ 66: 0F EF. F5
	pxor	xmm7, xmm0				; 21A6 _ 66: 0F EF. F8
	pand	xmm5, xmm7				; 21AA _ 66: 0F DB. EF
	pxor	xmm0, xmm6				; 21AE _ 66: 0F EF. C6
	pxor	xmm5, xmm2				; 21B2 _ 66: 0F EF. EA
	por	xmm0, xmm7				; 21B6 _ 66: 0F EB. C7
	pxor	xmm0, xmm2				; 21BA _ 66: 0F EF. C2
	movdqa	xmm2, xmm0				; 21BE _ 66: 0F 6F. D0
	pxor	xmm7, xmm6				; 21C2 _ 66: 0F EF. FE
	por	xmm2, xmm6				; 21C6 _ 66: 0F EB. D6
	pxor	xmm7, xmm2				; 21CA _ 66: 0F EF. FA
	movdqa	xmm2, xmm7				; 21CE _ 66: 0F 6F. D7
	psrld	xmm7, 19				; 21D2 _ 66: 0F 72. D7, 13
	pslld	xmm2, 13				; 21D7 _ 66: 0F 72. F2, 0D
	por	xmm2, xmm7				; 21DC _ 66: 0F EB. D7
	movdqa	xmm7, xmm6				; 21E0 _ 66: 0F 6F. FE
	psrld	xmm6, 29				; 21E4 _ 66: 0F 72. D6, 1D
	pxor	xmm0, xmm2				; 21E9 _ 66: 0F EF. C2
	pslld	xmm7, 3 				; 21ED _ 66: 0F 72. F7, 03
	por	xmm7, xmm6				; 21F2 _ 66: 0F EB. FE
	movdqa	xmm6, xmm2				; 21F6 _ 66: 0F 6F. F2
	pxor	xmm0, xmm7				; 21FA _ 66: 0F EF. C7
	movdqa	xmm1, xmm0				; 21FE _ 66: 0F 6F. C8
	pxor	xmm5, xmm7				; 2202 _ 66: 0F EF. EF
	pslld	xmm6, 3 				; 2206 _ 66: 0F 72. F6, 03
	pxor	xmm5, xmm6				; 220B _ 66: 0F EF. EE
	movdqa	xmm6, xmm5				; 220F _ 66: 0F 6F. F5
	pslld	xmm1, 1 				; 2213 _ 66: 0F 72. F1, 01
	psrld	xmm0, 31				; 2218 _ 66: 0F 72. D0, 1F
	por	xmm1, xmm0				; 221D _ 66: 0F EB. C8
	pslld	xmm6, 7 				; 2221 _ 66: 0F 72. F6, 07
	psrld	xmm5, 25				; 2226 _ 66: 0F 72. D5, 19
	por	xmm6, xmm5				; 222B _ 66: 0F EB. F5
	movdqa	xmm5, xmm1				; 222F _ 66: 0F 6F. E9
	pxor	xmm2, xmm1				; 2233 _ 66: 0F EF. D1
	pxor	xmm2, xmm6				; 2237 _ 66: 0F EF. D6
	movdqa	xmm0, xmm2				; 223B _ 66: 0F 6F. C2
	pxor	xmm7, xmm6				; 223F _ 66: 0F EF. FE
	pslld	xmm5, 7 				; 2243 _ 66: 0F 72. F5, 07
	pxor	xmm7, xmm5				; 2248 _ 66: 0F EF. FD
	movdqa	xmm5, xmm7				; 224C _ 66: 0F 6F. EF
	pslld	xmm0, 5 				; 2250 _ 66: 0F 72. F0, 05
	psrld	xmm2, 27				; 2255 _ 66: 0F 72. D2, 1B
	por	xmm0, xmm2				; 225A _ 66: 0F EB. C2
	movd	xmm2, dword [ecx+2C40H] 		; 225E _ 66: 0F 6E. 91, 00002C40
	pslld	xmm5, 22				; 2266 _ 66: 0F 72. F5, 16
	psrld	xmm7, 10				; 226B _ 66: 0F 72. D7, 0A
	por	xmm5, xmm7				; 2270 _ 66: 0F EB. EF
	pshufd	xmm7, xmm2, 0				; 2274 _ 66: 0F 70. FA, 00
	pxor	xmm0, xmm7				; 2279 _ 66: 0F EF. C7
	movd	xmm2, dword [ecx+2C44H] 		; 227D _ 66: 0F 6E. 91, 00002C44
	pshufd	xmm7, xmm2, 0				; 2285 _ 66: 0F 70. FA, 00
	movd	xmm2, dword [ecx+2C48H] 		; 228A _ 66: 0F 6E. 91, 00002C48
	pxor	xmm1, xmm7				; 2292 _ 66: 0F EF. CF
	pshufd	xmm7, xmm2, 0				; 2296 _ 66: 0F 70. FA, 00
	pxor	xmm5, xmm7				; 229B _ 66: 0F EF. EF
	movd	xmm2, dword [ecx+2C4CH] 		; 229F _ 66: 0F 6E. 91, 00002C4C
	pshufd	xmm7, xmm2, 0				; 22A7 _ 66: 0F 70. FA, 00
	pxor	xmm6, xmm7				; 22AC _ 66: 0F EF. F7
	pxor	xmm1, xmm6				; 22B0 _ 66: 0F EF. CE
	movdqa	xmm7, xmm1				; 22B4 _ 66: 0F 6F. F9
	pxor	xmm6, xmm4				; 22B8 _ 66: 0F EF. F4
	pxor	xmm5, xmm6				; 22BC _ 66: 0F EF. EE
	pxor	xmm6, xmm0				; 22C0 _ 66: 0F EF. F0
	pand	xmm7, xmm6				; 22C4 _ 66: 0F DB. FE
	pxor	xmm7, xmm5				; 22C8 _ 66: 0F EF. FD
	movdqa	xmm2, xmm7				; 22CC _ 66: 0F 6F. D7
	pxor	xmm1, xmm6				; 22D0 _ 66: 0F EF. CE
	pxor	xmm0, xmm1				; 22D4 _ 66: 0F EF. C1
	pand	xmm5, xmm1				; 22D8 _ 66: 0F DB. E9
	pxor	xmm5, xmm0				; 22DC _ 66: 0F EF. E8
	pand	xmm0, xmm7				; 22E0 _ 66: 0F DB. C7
	pxor	xmm6, xmm0				; 22E4 _ 66: 0F EF. F0
	por	xmm1, xmm7				; 22E8 _ 66: 0F EB. CF
	pxor	xmm1, xmm0				; 22EC _ 66: 0F EF. C8
	por	xmm0, xmm6				; 22F0 _ 66: 0F EB. C6
	pxor	xmm0, xmm5				; 22F4 _ 66: 0F EF. C5
	pand	xmm5, xmm6				; 22F8 _ 66: 0F DB. EE
	pxor	xmm0, xmm4				; 22FC _ 66: 0F EF. C4
	pxor	xmm1, xmm5				; 2300 _ 66: 0F EF. CD
	movdqa	xmm5, xmm0				; 2304 _ 66: 0F 6F. E8
	pslld	xmm2, 13				; 2308 _ 66: 0F 72. F2, 0D
	psrld	xmm7, 19				; 230D _ 66: 0F 72. D7, 13
	por	xmm2, xmm7				; 2312 _ 66: 0F EB. D7
	pslld	xmm5, 3 				; 2316 _ 66: 0F 72. F5, 03
	psrld	xmm0, 29				; 231B _ 66: 0F 72. D0, 1D
	por	xmm5, xmm0				; 2320 _ 66: 0F EB. E8
	movdqa	xmm0, xmm2				; 2324 _ 66: 0F 6F. C2
	pxor	xmm1, xmm2				; 2328 _ 66: 0F EF. CA
	pxor	xmm1, xmm5				; 232C _ 66: 0F EF. CD
	movdqa	xmm7, xmm1				; 2330 _ 66: 0F 6F. F9
	pxor	xmm6, xmm5				; 2334 _ 66: 0F EF. F5
	pslld	xmm0, 3 				; 2338 _ 66: 0F 72. F0, 03
	pxor	xmm6, xmm0				; 233D _ 66: 0F EF. F0
	movdqa	xmm0, xmm6				; 2341 _ 66: 0F 6F. C6
	pslld	xmm7, 1 				; 2345 _ 66: 0F 72. F7, 01
	psrld	xmm1, 31				; 234A _ 66: 0F 72. D1, 1F
	por	xmm7, xmm1				; 234F _ 66: 0F EB. F9
	movdqa	xmm1, xmm7				; 2353 _ 66: 0F 6F. CF
	pslld	xmm0, 7 				; 2357 _ 66: 0F 72. F0, 07
	psrld	xmm6, 25				; 235C _ 66: 0F 72. D6, 19
	por	xmm0, xmm6				; 2361 _ 66: 0F EB. C6
	pxor	xmm2, xmm7				; 2365 _ 66: 0F EF. D7
	pxor	xmm2, xmm0				; 2369 _ 66: 0F EF. D0
	pxor	xmm5, xmm0				; 236D _ 66: 0F EF. E8
	pslld	xmm1, 7 				; 2371 _ 66: 0F 72. F1, 07
	pxor	xmm5, xmm1				; 2376 _ 66: 0F EF. E9
	movdqa	xmm1, xmm2				; 237A _ 66: 0F 6F. CA
	movdqa	xmm6, xmm5				; 237E _ 66: 0F 6F. F5
	psrld	xmm2, 27				; 2382 _ 66: 0F 72. D2, 1B
	pslld	xmm1, 5 				; 2387 _ 66: 0F 72. F1, 05
	por	xmm1, xmm2				; 238C _ 66: 0F EB. CA
	pslld	xmm6, 22				; 2390 _ 66: 0F 72. F6, 16
	psrld	xmm5, 10				; 2395 _ 66: 0F 72. D5, 0A
	movd	xmm2, dword [ecx+2C54H] 		; 239A _ 66: 0F 6E. 91, 00002C54
	por	xmm6, xmm5				; 23A2 _ 66: 0F EB. F5
	movd	xmm5, dword [ecx+2C50H] 		; 23A6 _ 66: 0F 6E. A9, 00002C50
	pshufd	xmm5, xmm5, 0				; 23AE _ 66: 0F 70. ED, 00
	pxor	xmm1, xmm5				; 23B3 _ 66: 0F EF. CD
	pshufd	xmm5, xmm2, 0				; 23B7 _ 66: 0F 70. EA, 00
	pxor	xmm7, xmm5				; 23BC _ 66: 0F EF. FD
	pxor	xmm1, xmm7				; 23C0 _ 66: 0F EF. CF
	movd	xmm2, dword [ecx+2C58H] 		; 23C4 _ 66: 0F 6E. 91, 00002C58
	pshufd	xmm5, xmm2, 0				; 23CC _ 66: 0F 70. EA, 00
	pxor	xmm6, xmm5				; 23D1 _ 66: 0F EF. F5
	movd	xmm2, dword [ecx+2C5CH] 		; 23D5 _ 66: 0F 6E. 91, 00002C5C
	pshufd	xmm5, xmm2, 0				; 23DD _ 66: 0F 70. EA, 00
	pxor	xmm0, xmm5				; 23E2 _ 66: 0F EF. C5
	pxor	xmm7, xmm0				; 23E6 _ 66: 0F EF. F8
	movdqa	xmm5, xmm7				; 23EA _ 66: 0F 6F. EF
	pxor	xmm0, xmm4				; 23EE _ 66: 0F EF. C4
	pxor	xmm6, xmm0				; 23F2 _ 66: 0F EF. F0
	pand	xmm5, xmm1				; 23F6 _ 66: 0F DB. E9
	pxor	xmm5, xmm6				; 23FA _ 66: 0F EF. EE
	movdqa	xmm2, xmm5				; 23FE _ 66: 0F 6F. D5
	por	xmm6, xmm7				; 2402 _ 66: 0F EB. F7
	pxor	xmm7, xmm0				; 2406 _ 66: 0F EF. F8
	pand	xmm0, xmm5				; 240A _ 66: 0F DB. C5
	pxor	xmm0, xmm1				; 240E _ 66: 0F EF. C1
	pxor	xmm7, xmm5				; 2412 _ 66: 0F EF. FD
	pxor	xmm7, xmm6				; 2416 _ 66: 0F EF. FE
	pxor	xmm6, xmm1				; 241A _ 66: 0F EF. F1
	pand	xmm1, xmm0				; 241E _ 66: 0F DB. C8
	pxor	xmm6, xmm4				; 2422 _ 66: 0F EF. F4
	pxor	xmm1, xmm7				; 2426 _ 66: 0F EF. CF
	por	xmm7, xmm0				; 242A _ 66: 0F EB. F8
	pxor	xmm7, xmm6				; 242E _ 66: 0F EF. FE
	movdqa	xmm6, xmm1				; 2432 _ 66: 0F 6F. F1
	pslld	xmm2, 13				; 2436 _ 66: 0F 72. F2, 0D
	psrld	xmm5, 19				; 243B _ 66: 0F 72. D5, 13
	por	xmm2, xmm5				; 2440 _ 66: 0F EB. D5
	pslld	xmm6, 3 				; 2444 _ 66: 0F 72. F6, 03
	psrld	xmm1, 29				; 2449 _ 66: 0F 72. D1, 1D
	por	xmm6, xmm1				; 244E _ 66: 0F EB. F1
	movdqa	xmm1, xmm2				; 2452 _ 66: 0F 6F. CA
	pxor	xmm0, xmm2				; 2456 _ 66: 0F EF. C2
	pxor	xmm0, xmm6				; 245A _ 66: 0F EF. C6
	movdqa	xmm5, xmm0				; 245E _ 66: 0F 6F. E8
	pxor	xmm7, xmm6				; 2462 _ 66: 0F EF. FE
	pslld	xmm1, 3 				; 2466 _ 66: 0F 72. F1, 03
	pxor	xmm7, xmm1				; 246B _ 66: 0F EF. F9
	pslld	xmm5, 1 				; 246F _ 66: 0F 72. F5, 01
	psrld	xmm0, 31				; 2474 _ 66: 0F 72. D0, 1F
	por	xmm5, xmm0				; 2479 _ 66: 0F EB. E8
	movdqa	xmm0, xmm7				; 247D _ 66: 0F 6F. C7
	psrld	xmm7, 25				; 2481 _ 66: 0F 72. D7, 19
	pxor	xmm2, xmm5				; 2486 _ 66: 0F EF. D5
	pslld	xmm0, 7 				; 248A _ 66: 0F 72. F0, 07
	por	xmm0, xmm7				; 248F _ 66: 0F EB. C7
	movdqa	xmm7, xmm5				; 2493 _ 66: 0F 6F. FD
	pxor	xmm2, xmm0				; 2497 _ 66: 0F EF. D0
	pxor	xmm6, xmm0				; 249B _ 66: 0F EF. F0
	pslld	xmm7, 7 				; 249F _ 66: 0F 72. F7, 07
	pxor	xmm6, xmm7				; 24A4 _ 66: 0F EF. F7
	movdqa	xmm7, xmm2				; 24A8 _ 66: 0F 6F. FA
	movdqa	xmm1, xmm6				; 24AC _ 66: 0F 6F. CE
	psrld	xmm2, 27				; 24B0 _ 66: 0F 72. D2, 1B
	pslld	xmm7, 5 				; 24B5 _ 66: 0F 72. F7, 05
	por	xmm7, xmm2				; 24BA _ 66: 0F EB. FA
	pslld	xmm1, 22				; 24BE _ 66: 0F 72. F1, 16
	psrld	xmm6, 10				; 24C3 _ 66: 0F 72. D6, 0A
	por	xmm1, xmm6				; 24C8 _ 66: 0F EB. CE
	movd	xmm6, dword [ecx+2C60H] 		; 24CC _ 66: 0F 6E. B1, 00002C60
	pshufd	xmm2, xmm6, 0				; 24D4 _ 66: 0F 70. D6, 00
	pxor	xmm7, xmm2				; 24D9 _ 66: 0F EF. FA
	movd	xmm6, dword [ecx+2C64H] 		; 24DD _ 66: 0F 6E. B1, 00002C64
	pshufd	xmm2, xmm6, 0				; 24E5 _ 66: 0F 70. D6, 00
	pxor	xmm5, xmm2				; 24EA _ 66: 0F EF. EA
	movd	xmm6, dword [ecx+2C68H] 		; 24EE _ 66: 0F 6E. B1, 00002C68
	pshufd	xmm2, xmm6, 0				; 24F6 _ 66: 0F 70. D6, 00
	pxor	xmm1, xmm2				; 24FB _ 66: 0F EF. CA
	pxor	xmm1, xmm4				; 24FF _ 66: 0F EF. CC
	movd	xmm6, dword [ecx+2C6CH] 		; 2503 _ 66: 0F 6E. B1, 00002C6C
	pshufd	xmm2, xmm6, 0				; 250B _ 66: 0F 70. D6, 00
	pxor	xmm0, xmm2				; 2510 _ 66: 0F EF. C2
	movdqa	xmm2, xmm0				; 2514 _ 66: 0F 6F. D0
	pand	xmm2, xmm7				; 2518 _ 66: 0F DB. D7
	pxor	xmm7, xmm0				; 251C _ 66: 0F EF. F8
	pxor	xmm2, xmm1				; 2520 _ 66: 0F EF. D1
	por	xmm1, xmm0				; 2524 _ 66: 0F EB. C8
	pxor	xmm5, xmm2				; 2528 _ 66: 0F EF. EA
	pxor	xmm1, xmm7				; 252C _ 66: 0F EF. CF
	por	xmm7, xmm5				; 2530 _ 66: 0F EB. FD
	pxor	xmm1, xmm5				; 2534 _ 66: 0F EF. CD
	pxor	xmm0, xmm7				; 2538 _ 66: 0F EF. C7
	por	xmm7, xmm2				; 253C _ 66: 0F EB. FA
	pxor	xmm7, xmm1				; 2540 _ 66: 0F EF. F9
	pxor	xmm0, xmm2				; 2544 _ 66: 0F EF. C2
	pxor	xmm0, xmm7				; 2548 _ 66: 0F EF. C7
	movdqa	xmm6, xmm0				; 254C _ 66: 0F 6F. F0
	pxor	xmm2, xmm4				; 2550 _ 66: 0F EF. D4
	pand	xmm1, xmm0				; 2554 _ 66: 0F DB. C8
	pxor	xmm2, xmm1				; 2558 _ 66: 0F EF. D1
	movdqa	xmm1, xmm7				; 255C _ 66: 0F 6F. CF
	psrld	xmm7, 19				; 2560 _ 66: 0F 72. D7, 13
	pslld	xmm6, 3 				; 2565 _ 66: 0F 72. F6, 03
	pslld	xmm1, 13				; 256A _ 66: 0F 72. F1, 0D
	por	xmm1, xmm7				; 256F _ 66: 0F EB. CF
	psrld	xmm0, 29				; 2573 _ 66: 0F 72. D0, 1D
	por	xmm6, xmm0				; 2578 _ 66: 0F EB. F0
	movdqa	xmm0, xmm1				; 257C _ 66: 0F 6F. C1
	pxor	xmm5, xmm1				; 2580 _ 66: 0F EF. E9
	pxor	xmm5, xmm6				; 2584 _ 66: 0F EF. EE
	pxor	xmm2, xmm6				; 2588 _ 66: 0F EF. D6
	pslld	xmm0, 3 				; 258C _ 66: 0F 72. F0, 03
	pxor	xmm2, xmm0				; 2591 _ 66: 0F EF. D0
	movdqa	xmm0, xmm5				; 2595 _ 66: 0F 6F. C5
	psrld	xmm5, 31				; 2599 _ 66: 0F 72. D5, 1F
	pslld	xmm0, 1 				; 259E _ 66: 0F 72. F0, 01
	por	xmm0, xmm5				; 25A3 _ 66: 0F EB. C5
	movdqa	xmm5, xmm2				; 25A7 _ 66: 0F 6F. EA
	movdqa	xmm7, xmm0				; 25AB _ 66: 0F 6F. F8
	psrld	xmm2, 25				; 25AF _ 66: 0F 72. D2, 19
	pslld	xmm5, 7 				; 25B4 _ 66: 0F 72. F5, 07
	por	xmm5, xmm2				; 25B9 _ 66: 0F EB. EA
	pxor	xmm1, xmm0				; 25BD _ 66: 0F EF. C8
	pxor	xmm1, xmm5				; 25C1 _ 66: 0F EF. CD
	movdqa	xmm2, xmm1				; 25C5 _ 66: 0F 6F. D1
	pxor	xmm6, xmm5				; 25C9 _ 66: 0F EF. F5
	pslld	xmm7, 7 				; 25CD _ 66: 0F 72. F7, 07
	pxor	xmm6, xmm7				; 25D2 _ 66: 0F EF. F7
	movdqa	xmm7, xmm6				; 25D6 _ 66: 0F 6F. FE
	pslld	xmm2, 5 				; 25DA _ 66: 0F 72. F2, 05
	psrld	xmm1, 27				; 25DF _ 66: 0F 72. D1, 1B
	por	xmm2, xmm1				; 25E4 _ 66: 0F EB. D1
	pslld	xmm7, 22				; 25E8 _ 66: 0F 72. F7, 16
	psrld	xmm6, 10				; 25ED _ 66: 0F 72. D6, 0A
	por	xmm7, xmm6				; 25F2 _ 66: 0F EB. FE
	movd	xmm1, dword [ecx+2C70H] 		; 25F6 _ 66: 0F 6E. 89, 00002C70
	movd	xmm6, dword [ecx+2C74H] 		; 25FE _ 66: 0F 6E. B1, 00002C74
	pshufd	xmm1, xmm1, 0				; 2606 _ 66: 0F 70. C9, 00
	pxor	xmm2, xmm1				; 260B _ 66: 0F EF. D1
	pshufd	xmm1, xmm6, 0				; 260F _ 66: 0F 70. CE, 00
	pxor	xmm0, xmm1				; 2614 _ 66: 0F EF. C1
	movd	xmm6, dword [ecx+2C78H] 		; 2618 _ 66: 0F 6E. B1, 00002C78
	pshufd	xmm1, xmm6, 0				; 2620 _ 66: 0F 70. CE, 00
	pxor	xmm7, xmm1				; 2625 _ 66: 0F EF. F9
	movd	xmm6, dword [ecx+2C7CH] 		; 2629 _ 66: 0F 6E. B1, 00002C7C
	pshufd	xmm1, xmm6, 0				; 2631 _ 66: 0F 70. CE, 00
	pxor	xmm5, xmm1				; 2636 _ 66: 0F EF. E9
	movdqa	xmm1, xmm0				; 263A _ 66: 0F 6F. C8
	pxor	xmm0, xmm7				; 263E _ 66: 0F EF. C7
	por	xmm1, xmm7				; 2642 _ 66: 0F EB. CF
	pxor	xmm1, xmm5				; 2646 _ 66: 0F EF. CD
	pxor	xmm7, xmm1				; 264A _ 66: 0F EF. F9
	por	xmm5, xmm0				; 264E _ 66: 0F EB. E8
	pand	xmm5, xmm2				; 2652 _ 66: 0F DB. EA
	pxor	xmm0, xmm7				; 2656 _ 66: 0F EF. C7
	pxor	xmm5, xmm1				; 265A _ 66: 0F EF. E9
	por	xmm1, xmm0				; 265E _ 66: 0F EB. C8
	pxor	xmm1, xmm2				; 2662 _ 66: 0F EF. CA
	por	xmm2, xmm0				; 2666 _ 66: 0F EB. D0
	pxor	xmm2, xmm7				; 266A _ 66: 0F EF. D7
	pxor	xmm1, xmm0				; 266E _ 66: 0F EF. C8
	pxor	xmm7, xmm1				; 2672 _ 66: 0F EF. F9
	pand	xmm1, xmm2				; 2676 _ 66: 0F DB. CA
	pxor	xmm1, xmm0				; 267A _ 66: 0F EF. C8
	pxor	xmm7, xmm4				; 267E _ 66: 0F EF. FC
	por	xmm7, xmm2				; 2682 _ 66: 0F EB. FA
	pxor	xmm0, xmm7				; 2686 _ 66: 0F EF. C7
	movd	xmm7, dword [ecx+2C80H] 		; 268A _ 66: 0F 6E. B9, 00002C80
	pshufd	xmm6, xmm7, 0				; 2692 _ 66: 0F 70. F7, 00
	pxor	xmm0, xmm6				; 2697 _ 66: 0F EF. C6
	movd	xmm7, dword [ecx+2C84H] 		; 269B _ 66: 0F 6E. B9, 00002C84
	pshufd	xmm6, xmm7, 0				; 26A3 _ 66: 0F 70. F7, 00
	pxor	xmm5, xmm6				; 26A8 _ 66: 0F EF. EE
	movd	xmm7, dword [ecx+2C88H] 		; 26AC _ 66: 0F 6E. B9, 00002C88
	pshufd	xmm6, xmm7, 0				; 26B4 _ 66: 0F 70. F7, 00
	pxor	xmm1, xmm6				; 26B9 _ 66: 0F EF. CE
	movd	xmm7, dword [ecx+2C8CH] 		; 26BD _ 66: 0F 6E. B9, 00002C8C
	pshufd	xmm6, xmm7, 0				; 26C5 _ 66: 0F 70. F7, 00
	movdqa	xmm7, xmm0				; 26CA _ 66: 0F 6F. F8
	pxor	xmm2, xmm6				; 26CE _ 66: 0F EF. D6
	movdqa	xmm6, xmm1				; 26D2 _ 66: 0F 6F. F1
	punpckldq xmm7, xmm5				; 26D6 _ 66: 0F 62. FD
	punpckhdq xmm0, xmm5				; 26DA _ 66: 0F 6A. C5
	punpckldq xmm6, xmm2				; 26DE _ 66: 0F 62. F2
	punpckhdq xmm1, xmm2				; 26E2 _ 66: 0F 6A. CA
	movdqa	xmm2, xmm7				; 26E6 _ 66: 0F 6F. D7
	movdqa	xmm5, xmm0				; 26EA _ 66: 0F 6F. E8
	punpckhqdq xmm7, xmm6				; 26EE _ 66: 0F 6D. FE
	pxor	xmm7, oword [esp+40H]			; 26F2 _ 66: 0F EF. 7C 24, 40
	movdqu	oword [edx+10H], xmm7			; 26F8 _ F3: 0F 7F. 7A, 10
	punpcklqdq xmm2, xmm6				; 26FD _ 66: 0F 6C. D6
	pxor	xmm2, oword [esp+60H]			; 2701 _ 66: 0F EF. 54 24, 60
	movdqu	oword [edx], xmm2			; 2707 _ F3: 0F 7F. 12
	punpcklqdq xmm5, xmm1				; 270B _ 66: 0F 6C. E9
	pxor	xmm5, oword [esp+50H]			; 270F _ 66: 0F EF. 6C 24, 50
	movdqu	oword [edx+20H], xmm5			; 2715 _ F3: 0F 7F. 6A, 20
	punpckhqdq xmm0, xmm1				; 271A _ 66: 0F 6D. C1
	movdqa	xmm1, xmm3				; 271E _ 66: 0F 6F. CB
	pxor	xmm0, xmm3				; 2722 _ 66: 0F EF. C3
	movdqu	oword [edx+30H], xmm0			; 2726 _ F3: 0F 7F. 42, 30
	movdqa	xmm0, xmm3				; 272B _ 66: 0F 6F. C3
	psllq	xmm1, 1 				; 272F _ 66: 0F 73. F1, 01
	psraw	xmm3, 8 				; 2734 _ 66: 0F 71. E3, 08
	pslldq	xmm0, 8 				; 2739 _ 66: 0F 73. F8, 08
	psrldq	xmm0, 7 				; 273E _ 66: 0F 73. D8, 07
	psrlq	xmm0, 7 				; 2743 _ 66: 0F 73. D0, 07
	por	xmm1, xmm0				; 2748 _ 66: 0F EB. C8
	psrldq	xmm3, 15				; 274C _ 66: 0F 73. DB, 0F
	pand	xmm3, oword [esp+30H]			; 2751 _ 66: 0F DB. 5C 24, 30
	pxor	xmm1, xmm3				; 2757 _ 66: 0F EF. CB
	movdqa	oword [esp+60H], xmm1			; 275B _ 66: 0F 7F. 4C 24, 60
	add	esi, 64 				; 2761 _ 83. C6, 40
	add	edx, 64 				; 2764 _ 83. C2, 40
	inc	eax					; 2767 _ 40
	cmp	eax, 8					; 2768 _ 83. F8, 08
	jl	?_003					; 276B _ 0F 8C, FFFFD92A
	movdqa	xmm0, oword [esp+60H]			; 2771 _ 66: 0F 6F. 44 24, 60
	mov	dword [esp+24H], edx			; 2777 _ 89. 54 24, 24
	mov	dword [esp+28H], esi			; 277B _ 89. 74 24, 28
	mov	esi, dword [esp+20H]			; 277F _ 8B. 74 24, 20
	add	esi, -512				; 2783 _ 81. C6, FFFFFE00
	jne	?_001					; 2789 _ 0F 85, FFFFD8D0
	add	esp, 116				; 278F _ 83. C4, 74
	pop	ebx					; 2792 _ 5B
	pop	esi					; 2793 _ 5E
	pop	edi					; 2794 _ 5F
	mov	esp, ebp				; 2795 _ 8B. E5
	pop	ebp					; 2797 _ 5D
	ret	24					; 2798 _ C2, 0018
; _xts_serpent_sse2_encrypt@24 End of function

	nop						; 279B _ 90
; Filling space: 4H
; Filler type: lea with same source and destination
;       db 8DH, 74H, 26H, 00H

ALIGN	8


_xts_serpent_sse2_decrypt@24:; Function begin
	push	ebp					; 0000 _ 55
	mov	ebp, esp				; 0001 _ 8B. EC
	and	esp, 0FFFFFFF0H 			; 0003 _ 83. E4, F0
	push	edi					; 0006 _ 57
	push	esi					; 0007 _ 56
	push	ebx					; 0008 _ 53
	sub	esp, 116				; 0009 _ 83. EC, 74
	mov	edx, dword [ebp+8H]			; 000C _ 8B. 55, 08
	mov	eax, dword [ebp+0CH]			; 000F _ 8B. 45, 0C
	mov	esi, dword [ebp+18H]			; 0012 _ 8B. 75, 18
	mov	ebx, dword [ebp+1CH]			; 0015 _ 8B. 5D, 1C
	mov	edi, dword [ebp+14H]			; 0018 _ 8B. 7D, 14
	mov	dword [esp+24H], eax			; 001B _ 89. 44 24, 24
	mov	ecx, esi				; 001F _ 8B. CE
	mov	dword [esp+28H], edx			; 0021 _ 89. 54 24, 28
	shl	ecx, 23 				; 0025 _ C1. E1, 17
	shr	esi, 9					; 0028 _ C1. EE, 09
	mov	dword [esp+4H], esi			; 002B _ 89. 74 24, 04
	mov	esi, dword [ebp+10H]			; 002F _ 8B. 75, 10
	shr	edi, 9					; 0032 _ C1. EF, 09
	or	ecx, edi				; 0035 _ 0B. CF
	lea	ebx, [ebx+5710H]			; 0037 _ 8D. 9B, 00005710
	mov	dword [esp], ecx			; 003D _ 89. 0C 24
	xor	ecx, ecx				; 0040 _ 33. C9
	mov	dword [esp+8H], ecx			; 0042 _ 89. 4C 24, 08
	mov	dword [esp+0CH], ecx			; 0046 _ 89. 4C 24, 0C
	mov	edi, 135				; 004A _ BF, 00000087
	movd	xmm1, edi				; 004F _ 66: 0F 6E. CF
	movdqa	oword [esp+30H], xmm1			; 0053 _ 66: 0F 7F. 4C 24, 30
	lea	edi, [esp+10H]				; 0059 _ 8D. 7C 24, 10
	jmp	?_005					; 005D _ EB, 06

?_004:	movdqa	oword [esp+10H], xmm4			; 005F _ 66: 0F 7F. 64 24, 10
?_005:	add	dword [esp], 1				; 0065 _ 83. 04 24, 01
	adc	dword [esp+4H], 0			; 0069 _ 83. 54 24, 04, 00
	push	ebx					; 006E _ 53
	push	edi					; 006F _ 57
	lea	eax, [esp+8H]				; 0070 _ 8D. 44 24, 08
	push	eax					; 0074 _ 50
	call	_serpent256_encrypt@12			; 0075 _ E8, 00000000(rel)
	movdqa	xmm4, oword [esp+10H]			; 007A _ 66: 0F 6F. 64 24, 10
	mov	edx, dword [esp+24H]			; 0080 _ 8B. 54 24, 24
	mov	ecx, dword [ebp+1CH]			; 0084 _ 8B. 4D, 1C
	mov	dword [esp+20H], esi			; 0087 _ 89. 74 24, 20
	xor	eax, eax				; 008B _ 33. C0
	mov	esi, dword [esp+28H]			; 008D _ 8B. 74 24, 28
?_006:	movdqa	xmm1, xmm4				; 0091 _ 66: 0F 6F. CC
	movdqa	xmm7, xmm4				; 0095 _ 66: 0F 6F. FC
	movdqa	xmm5, xmm4				; 0099 _ 66: 0F 6F. EC
	movdqa	xmm0, oword [esp+30H]			; 009D _ 66: 0F 6F. 44 24, 30
	psllq	xmm1, 1 				; 00A3 _ 66: 0F 73. F1, 01
	pslldq	xmm7, 8 				; 00A8 _ 66: 0F 73. FF, 08
	psrldq	xmm7, 7 				; 00AD _ 66: 0F 73. DF, 07
	psrlq	xmm7, 7 				; 00B2 _ 66: 0F 73. D7, 07
	por	xmm1, xmm7				; 00B7 _ 66: 0F EB. CF
	psraw	xmm5, 8 				; 00BB _ 66: 0F 71. E5, 08
	psrldq	xmm5, 15				; 00C0 _ 66: 0F 73. DD, 0F
	pand	xmm5, xmm0				; 00C5 _ 66: 0F DB. E8
	pxor	xmm1, xmm5				; 00C9 _ 66: 0F EF. CD
	movdqa	oword [esp+40H], xmm1			; 00CD _ 66: 0F 7F. 4C 24, 40
	movdqa	xmm5, xmm1				; 00D3 _ 66: 0F 6F. E9
	movdqa	xmm2, xmm1				; 00D7 _ 66: 0F 6F. D1
	movdqa	xmm6, xmm1				; 00DB _ 66: 0F 6F. F1
	psllq	xmm5, 1 				; 00DF _ 66: 0F 73. F5, 01
	pslldq	xmm2, 8 				; 00E4 _ 66: 0F 73. FA, 08
	psrldq	xmm2, 7 				; 00E9 _ 66: 0F 73. DA, 07
	psrlq	xmm2, 7 				; 00EE _ 66: 0F 73. D2, 07
	por	xmm5, xmm2				; 00F3 _ 66: 0F EB. EA
	psraw	xmm6, 8 				; 00F7 _ 66: 0F 71. E6, 08
	psrldq	xmm6, 15				; 00FC _ 66: 0F 73. DE, 0F
	pand	xmm6, xmm0				; 0101 _ 66: 0F DB. F0
	pxor	xmm5, xmm6				; 0105 _ 66: 0F EF. EE
	movdqu	xmm6, oword [esi+20H]			; 0109 _ F3: 0F 6F. 76, 20
	movdqa	oword [esp+50H], xmm5			; 010E _ 66: 0F 7F. 6C 24, 50
	movdqa	xmm7, xmm5				; 0114 _ 66: 0F 6F. FD
	movdqa	xmm3, xmm5				; 0118 _ 66: 0F 6F. DD
	movdqa	xmm2, xmm5				; 011C _ 66: 0F 6F. D5
	psllq	xmm7, 1 				; 0120 _ 66: 0F 73. F7, 01
	pslldq	xmm3, 8 				; 0125 _ 66: 0F 73. FB, 08
	psrldq	xmm3, 7 				; 012A _ 66: 0F 73. DB, 07
	psrlq	xmm3, 7 				; 012F _ 66: 0F 73. D3, 07
	por	xmm7, xmm3				; 0134 _ 66: 0F EB. FB
	movdqu	xmm3, oword [esi]			; 0138 _ F3: 0F 6F. 1E
	psraw	xmm2, 8 				; 013C _ 66: 0F 71. E2, 08
	psrldq	xmm2, 15				; 0141 _ 66: 0F 73. DA, 0F
	pand	xmm2, xmm0				; 0146 _ 66: 0F DB. D0
	movdqu	xmm0, oword [esi+10H]			; 014A _ F3: 0F 6F. 46, 10
	pxor	xmm7, xmm2				; 014F _ 66: 0F EF. FA
	pxor	xmm3, xmm4				; 0153 _ 66: 0F EF. DC
	pxor	xmm0, xmm1				; 0157 _ 66: 0F EF. C1
	movdqu	xmm1, oword [esi+30H]			; 015B _ F3: 0F 6F. 4E, 30
	pxor	xmm6, xmm5				; 0160 _ 66: 0F EF. F5
	movdqa	xmm5, xmm3				; 0164 _ 66: 0F 6F. EB
	movdqa	xmm2, xmm6				; 0168 _ 66: 0F 6F. D6
	pxor	xmm1, xmm7				; 016C _ 66: 0F EF. CF
	punpckldq xmm5, xmm0				; 0170 _ 66: 0F 62. E8
	punpckldq xmm2, xmm1				; 0174 _ 66: 0F 62. D1
	punpckhdq xmm3, xmm0				; 0178 _ 66: 0F 6A. D8
	punpckhdq xmm6, xmm1				; 017C _ 66: 0F 6A. F1
	movdqa	xmm1, xmm5				; 0180 _ 66: 0F 6F. CD
	punpckhqdq xmm5, xmm2				; 0184 _ 66: 0F 6D. EA
	punpcklqdq xmm1, xmm2				; 0188 _ 66: 0F 6C. CA
	movdqa	xmm2, xmm3				; 018C _ 66: 0F 6F. D3
	punpckhqdq xmm3, xmm6				; 0190 _ 66: 0F 6D. DE
	punpcklqdq xmm2, xmm6				; 0194 _ 66: 0F 6C. D6
	movd	xmm0, dword [ecx+2C80H] 		; 0198 _ 66: 0F 6E. 81, 00002C80
	pshufd	xmm0, xmm0, 0				; 01A0 _ 66: 0F 70. C0, 00
	pxor	xmm1, xmm0				; 01A5 _ 66: 0F EF. C8
	movd	xmm6, dword [ecx+2C84H] 		; 01A9 _ 66: 0F 6E. B1, 00002C84
	pshufd	xmm0, xmm6, 0				; 01B1 _ 66: 0F 70. C6, 00
	movd	xmm6, dword [ecx+2C88H] 		; 01B6 _ 66: 0F 6E. B1, 00002C88
	pxor	xmm5, xmm0				; 01BE _ 66: 0F EF. E8
	pshufd	xmm0, xmm6, 0				; 01C2 _ 66: 0F 70. C6, 00
	pxor	xmm2, xmm0				; 01C7 _ 66: 0F EF. D0
	movd	xmm6, dword [ecx+2C8CH] 		; 01CB _ 66: 0F 6E. B1, 00002C8C
	pshufd	xmm0, xmm6, 0				; 01D3 _ 66: 0F 70. C6, 00
	movdqa	xmm6, xmm2				; 01D8 _ 66: 0F 6F. F2
	pxor	xmm3, xmm0				; 01DC _ 66: 0F EF. D8
	por	xmm2, xmm3				; 01E0 _ 66: 0F EB. D3
	pxor	xmm6, xmm1				; 01E4 _ 66: 0F EF. F1
	pand	xmm1, xmm3				; 01E8 _ 66: 0F DB. CB
	pxor	xmm3, xmm5				; 01EC _ 66: 0F EF. DD
	por	xmm5, xmm1				; 01F0 _ 66: 0F EB. E9
	pand	xmm3, xmm2				; 01F4 _ 66: 0F DB. DA
	pcmpeqd xmm0, xmm0				; 01F8 _ 66: 0F 76. C0
	pxor	xmm6, xmm0				; 01FC _ 66: 0F EF. F0
	pxor	xmm1, xmm6				; 0200 _ 66: 0F EF. CE
	pand	xmm6, xmm2				; 0204 _ 66: 0F DB. F2
	pxor	xmm5, xmm6				; 0208 _ 66: 0F EF. EE
	pxor	xmm6, xmm1				; 020C _ 66: 0F EF. F1
	por	xmm1, xmm6				; 0210 _ 66: 0F EB. CE
	pxor	xmm2, xmm5				; 0214 _ 66: 0F EF. D5
	pxor	xmm1, xmm3				; 0218 _ 66: 0F EF. CB
	pxor	xmm3, xmm2				; 021C _ 66: 0F EF. DA
	por	xmm2, xmm1				; 0220 _ 66: 0F EB. D1
	pxor	xmm3, xmm6				; 0224 _ 66: 0F EF. DE
	pxor	xmm2, xmm6				; 0228 _ 66: 0F EF. D6
	movd	xmm6, dword [ecx+2C70H] 		; 022C _ 66: 0F 6E. B1, 00002C70
	pshufd	xmm6, xmm6, 0				; 0234 _ 66: 0F 70. F6, 00
	pxor	xmm3, xmm6				; 0239 _ 66: 0F EF. DE
	movd	xmm6, dword [ecx+2C74H] 		; 023D _ 66: 0F 6E. B1, 00002C74
	pshufd	xmm6, xmm6, 0				; 0245 _ 66: 0F 70. F6, 00
	pxor	xmm1, xmm6				; 024A _ 66: 0F EF. CE
	movd	xmm6, dword [ecx+2C78H] 		; 024E _ 66: 0F 6E. B1, 00002C78
	pshufd	xmm6, xmm6, 0				; 0256 _ 66: 0F 70. F6, 00
	pxor	xmm5, xmm6				; 025B _ 66: 0F EF. EE
	movd	xmm6, dword [ecx+2C7CH] 		; 025F _ 66: 0F 6E. B1, 00002C7C
	pshufd	xmm6, xmm6, 0				; 0267 _ 66: 0F 70. F6, 00
	pxor	xmm2, xmm6				; 026C _ 66: 0F EF. D6
	movdqa	xmm6, xmm5				; 0270 _ 66: 0F 6F. F5
	psrld	xmm5, 22				; 0274 _ 66: 0F 72. D5, 16
	pslld	xmm6, 10				; 0279 _ 66: 0F 72. F6, 0A
	por	xmm6, xmm5				; 027E _ 66: 0F EB. F5
	movdqa	xmm5, xmm3				; 0282 _ 66: 0F 6F. EB
	psrld	xmm3, 5 				; 0286 _ 66: 0F 72. D3, 05
	pxor	xmm6, xmm2				; 028B _ 66: 0F EF. F2
	pslld	xmm5, 27				; 028F _ 66: 0F 72. F5, 1B
	por	xmm5, xmm3				; 0294 _ 66: 0F EB. EB
	movdqa	xmm3, xmm1				; 0298 _ 66: 0F 6F. D9
	pxor	xmm5, xmm1				; 029C _ 66: 0F EF. E9
	pxor	xmm5, xmm2				; 02A0 _ 66: 0F EF. EA
	pslld	xmm3, 7 				; 02A4 _ 66: 0F 72. F3, 07
	pxor	xmm6, xmm3				; 02A9 _ 66: 0F EF. F3
	movdqa	xmm3, xmm2				; 02AD _ 66: 0F 6F. DA
	psrld	xmm2, 7 				; 02B1 _ 66: 0F 72. D2, 07
	pslld	xmm3, 25				; 02B6 _ 66: 0F 72. F3, 19
	por	xmm3, xmm2				; 02BB _ 66: 0F EB. DA
	movdqa	xmm2, xmm1				; 02BF _ 66: 0F 6F. D1
	psrld	xmm1, 1 				; 02C3 _ 66: 0F 72. D1, 01
	pxor	xmm3, xmm6				; 02C8 _ 66: 0F EF. DE
	pslld	xmm2, 31				; 02CC _ 66: 0F 72. F2, 1F
	por	xmm2, xmm1				; 02D1 _ 66: 0F EB. D1
	movdqa	xmm1, xmm5				; 02D5 _ 66: 0F 6F. CD
	pxor	xmm2, xmm5				; 02D9 _ 66: 0F EF. D5
	pxor	xmm2, xmm6				; 02DD _ 66: 0F EF. D6
	pslld	xmm1, 3 				; 02E1 _ 66: 0F 72. F1, 03
	pxor	xmm3, xmm1				; 02E6 _ 66: 0F EF. D9
	movdqa	xmm1, xmm6				; 02EA _ 66: 0F 6F. CE
	psrld	xmm6, 3 				; 02EE _ 66: 0F 72. D6, 03
	pslld	xmm1, 29				; 02F3 _ 66: 0F 72. F1, 1D
	por	xmm1, xmm6				; 02F8 _ 66: 0F EB. CE
	movdqa	xmm6, xmm5				; 02FC _ 66: 0F 6F. F5
	psrld	xmm5, 13				; 0300 _ 66: 0F 72. D5, 0D
	pslld	xmm6, 19				; 0305 _ 66: 0F 72. F6, 13
	por	xmm6, xmm5				; 030A _ 66: 0F EB. F5
	movdqa	xmm5, xmm1				; 030E _ 66: 0F 6F. E9
	pxor	xmm6, xmm1				; 0312 _ 66: 0F EF. F1
	pxor	xmm1, xmm3				; 0316 _ 66: 0F EF. CB
	pand	xmm5, xmm6				; 031A _ 66: 0F DB. EE
	pxor	xmm5, xmm0				; 031E _ 66: 0F EF. E8
	pxor	xmm3, xmm2				; 0322 _ 66: 0F EF. DA
	pxor	xmm5, xmm3				; 0326 _ 66: 0F EF. EB
	por	xmm1, xmm6				; 032A _ 66: 0F EB. CE
	pxor	xmm6, xmm5				; 032E _ 66: 0F EF. F5
	pxor	xmm3, xmm1				; 0332 _ 66: 0F EF. D9
	pxor	xmm1, xmm2				; 0336 _ 66: 0F EF. CA
	pand	xmm2, xmm3				; 033A _ 66: 0F DB. D3
	pxor	xmm2, xmm6				; 033E _ 66: 0F EF. D6
	pxor	xmm6, xmm3				; 0342 _ 66: 0F EF. F3
	por	xmm6, xmm5				; 0346 _ 66: 0F EB. F5
	pxor	xmm3, xmm2				; 034A _ 66: 0F EF. DA
	pxor	xmm1, xmm6				; 034E _ 66: 0F EF. CE
	movd	xmm6, dword [ecx+2C60H] 		; 0352 _ 66: 0F 6E. B1, 00002C60
	pshufd	xmm6, xmm6, 0				; 035A _ 66: 0F 70. F6, 00
	pxor	xmm2, xmm6				; 035F _ 66: 0F EF. D6
	movd	xmm6, dword [ecx+2C64H] 		; 0363 _ 66: 0F 6E. B1, 00002C64
	pshufd	xmm6, xmm6, 0				; 036B _ 66: 0F 70. F6, 00
	pxor	xmm5, xmm6				; 0370 _ 66: 0F EF. EE
	movd	xmm6, dword [ecx+2C68H] 		; 0374 _ 66: 0F 6E. B1, 00002C68
	pshufd	xmm6, xmm6, 0				; 037C _ 66: 0F 70. F6, 00
	pxor	xmm1, xmm6				; 0381 _ 66: 0F EF. CE
	movd	xmm6, dword [ecx+2C6CH] 		; 0385 _ 66: 0F 6E. B1, 00002C6C
	pshufd	xmm6, xmm6, 0				; 038D _ 66: 0F 70. F6, 00
	pxor	xmm3, xmm6				; 0392 _ 66: 0F EF. DE
	movdqa	xmm6, xmm1				; 0396 _ 66: 0F 6F. F1
	psrld	xmm1, 22				; 039A _ 66: 0F 72. D1, 16
	pslld	xmm6, 10				; 039F _ 66: 0F 72. F6, 0A
	por	xmm6, xmm1				; 03A4 _ 66: 0F EB. F1
	movdqa	xmm1, xmm2				; 03A8 _ 66: 0F 6F. CA
	psrld	xmm2, 5 				; 03AC _ 66: 0F 72. D2, 05
	pxor	xmm6, xmm3				; 03B1 _ 66: 0F EF. F3
	pslld	xmm1, 27				; 03B5 _ 66: 0F 72. F1, 1B
	por	xmm1, xmm2				; 03BA _ 66: 0F EB. CA
	movdqa	xmm2, xmm5				; 03BE _ 66: 0F 6F. D5
	pxor	xmm1, xmm5				; 03C2 _ 66: 0F EF. CD
	pxor	xmm1, xmm3				; 03C6 _ 66: 0F EF. CB
	pslld	xmm2, 7 				; 03CA _ 66: 0F 72. F2, 07
	pxor	xmm6, xmm2				; 03CF _ 66: 0F EF. F2
	movdqa	xmm2, xmm3				; 03D3 _ 66: 0F 6F. D3
	psrld	xmm3, 7 				; 03D7 _ 66: 0F 72. D3, 07
	pslld	xmm2, 25				; 03DC _ 66: 0F 72. F2, 19
	por	xmm2, xmm3				; 03E1 _ 66: 0F EB. D3
	movdqa	xmm3, xmm5				; 03E5 _ 66: 0F 6F. DD
	psrld	xmm5, 1 				; 03E9 _ 66: 0F 72. D5, 01
	pxor	xmm2, xmm6				; 03EE _ 66: 0F EF. D6
	pslld	xmm3, 31				; 03F2 _ 66: 0F 72. F3, 1F
	por	xmm3, xmm5				; 03F7 _ 66: 0F EB. DD
	movdqa	xmm5, xmm1				; 03FB _ 66: 0F 6F. E9
	pxor	xmm3, xmm1				; 03FF _ 66: 0F EF. D9
	pxor	xmm3, xmm6				; 0403 _ 66: 0F EF. DE
	pslld	xmm5, 3 				; 0407 _ 66: 0F 72. F5, 03
	pxor	xmm2, xmm5				; 040C _ 66: 0F EF. D5
	movdqa	xmm5, xmm6				; 0410 _ 66: 0F 6F. EE
	psrld	xmm6, 3 				; 0414 _ 66: 0F 72. D6, 03
	pxor	xmm3, xmm0				; 0419 _ 66: 0F EF. D8
	pslld	xmm5, 29				; 041D _ 66: 0F 72. F5, 1D
	por	xmm5, xmm6				; 0422 _ 66: 0F EB. EE
	movdqa	xmm6, xmm1				; 0426 _ 66: 0F 6F. F1
	psrld	xmm1, 13				; 042A _ 66: 0F 72. D1, 0D
	pxor	xmm5, xmm3				; 042F _ 66: 0F EF. EB
	pslld	xmm6, 19				; 0433 _ 66: 0F 72. F6, 13
	por	xmm6, xmm1				; 0438 _ 66: 0F EB. F1
	movdqa	xmm1, xmm2				; 043C _ 66: 0F 6F. CA
	por	xmm1, xmm6				; 0440 _ 66: 0F EB. CE
	pxor	xmm1, xmm5				; 0444 _ 66: 0F EF. CD
	por	xmm5, xmm3				; 0448 _ 66: 0F EB. EB
	pand	xmm5, xmm6				; 044C _ 66: 0F DB. EE
	pxor	xmm2, xmm1				; 0450 _ 66: 0F EF. D1
	pxor	xmm5, xmm2				; 0454 _ 66: 0F EF. EA
	por	xmm2, xmm6				; 0458 _ 66: 0F EB. D6
	pxor	xmm2, xmm3				; 045C _ 66: 0F EF. D3
	pand	xmm3, xmm5				; 0460 _ 66: 0F DB. DD
	pxor	xmm3, xmm1				; 0464 _ 66: 0F EF. D9
	pxor	xmm2, xmm5				; 0468 _ 66: 0F EF. D5
	pand	xmm1, xmm2				; 046C _ 66: 0F DB. CA
	pxor	xmm2, xmm3				; 0470 _ 66: 0F EF. D3
	pxor	xmm1, xmm2				; 0474 _ 66: 0F EF. CA
	pxor	xmm2, xmm0				; 0478 _ 66: 0F EF. D0
	pxor	xmm1, xmm6				; 047C _ 66: 0F EF. CE
	movd	xmm6, dword [ecx+2C50H] 		; 0480 _ 66: 0F 6E. B1, 00002C50
	pshufd	xmm6, xmm6, 0				; 0488 _ 66: 0F 70. F6, 00
	pxor	xmm3, xmm6				; 048D _ 66: 0F EF. DE
	movd	xmm6, dword [ecx+2C54H] 		; 0491 _ 66: 0F 6E. B1, 00002C54
	pshufd	xmm6, xmm6, 0				; 0499 _ 66: 0F 70. F6, 00
	pxor	xmm2, xmm6				; 049E _ 66: 0F EF. D6
	movd	xmm6, dword [ecx+2C58H] 		; 04A2 _ 66: 0F 6E. B1, 00002C58
	pshufd	xmm6, xmm6, 0				; 04AA _ 66: 0F 70. F6, 00
	pxor	xmm1, xmm6				; 04AF _ 66: 0F EF. CE
	movd	xmm6, dword [ecx+2C5CH] 		; 04B3 _ 66: 0F 6E. B1, 00002C5C
	pshufd	xmm6, xmm6, 0				; 04BB _ 66: 0F 70. F6, 00
	pxor	xmm5, xmm6				; 04C0 _ 66: 0F EF. EE
	movdqa	xmm6, xmm1				; 04C4 _ 66: 0F 6F. F1
	psrld	xmm1, 22				; 04C8 _ 66: 0F 72. D1, 16
	pslld	xmm6, 10				; 04CD _ 66: 0F 72. F6, 0A
	por	xmm6, xmm1				; 04D2 _ 66: 0F EB. F1
	movdqa	xmm1, xmm3				; 04D6 _ 66: 0F 6F. CB
	psrld	xmm3, 5 				; 04DA _ 66: 0F 72. D3, 05
	pxor	xmm6, xmm5				; 04DF _ 66: 0F EF. F5
	pslld	xmm1, 27				; 04E3 _ 66: 0F 72. F1, 1B
	por	xmm1, xmm3				; 04E8 _ 66: 0F EB. CB
	movdqa	xmm3, xmm2				; 04EC _ 66: 0F 6F. DA
	pxor	xmm1, xmm2				; 04F0 _ 66: 0F EF. CA
	pxor	xmm1, xmm5				; 04F4 _ 66: 0F EF. CD
	pslld	xmm3, 7 				; 04F8 _ 66: 0F 72. F3, 07
	pxor	xmm6, xmm3				; 04FD _ 66: 0F EF. F3
	movdqa	xmm3, xmm5				; 0501 _ 66: 0F 6F. DD
	psrld	xmm5, 7 				; 0505 _ 66: 0F 72. D5, 07
	pslld	xmm3, 25				; 050A _ 66: 0F 72. F3, 19
	por	xmm3, xmm5				; 050F _ 66: 0F EB. DD
	movdqa	xmm5, xmm2				; 0513 _ 66: 0F 6F. EA
	psrld	xmm2, 1 				; 0517 _ 66: 0F 72. D2, 01
	pxor	xmm3, xmm6				; 051C _ 66: 0F EF. DE
	pslld	xmm5, 31				; 0520 _ 66: 0F 72. F5, 1F
	por	xmm5, xmm2				; 0525 _ 66: 0F EB. EA
	movdqa	xmm2, xmm1				; 0529 _ 66: 0F 6F. D1
	pxor	xmm5, xmm1				; 052D _ 66: 0F EF. E9
	pxor	xmm5, xmm6				; 0531 _ 66: 0F EF. EE
	pslld	xmm2, 3 				; 0535 _ 66: 0F 72. F2, 03
	pxor	xmm3, xmm2				; 053A _ 66: 0F EF. DA
	movdqa	xmm2, xmm6				; 053E _ 66: 0F 6F. D6
	psrld	xmm6, 3 				; 0542 _ 66: 0F 72. D6, 03
	pslld	xmm2, 29				; 0547 _ 66: 0F 72. F2, 1D
	por	xmm2, xmm6				; 054C _ 66: 0F EB. D6
	movdqa	xmm6, xmm1				; 0550 _ 66: 0F 6F. F1
	psrld	xmm1, 13				; 0554 _ 66: 0F 72. D1, 0D
	pslld	xmm6, 19				; 0559 _ 66: 0F 72. F6, 13
	por	xmm6, xmm1				; 055E _ 66: 0F EB. F1
	movdqa	xmm1, xmm2				; 0562 _ 66: 0F 6F. CA
	pand	xmm1, xmm3				; 0566 _ 66: 0F DB. CB
	pxor	xmm1, xmm5				; 056A _ 66: 0F EF. CD
	por	xmm5, xmm3				; 056E _ 66: 0F EB. EB
	pand	xmm5, xmm6				; 0572 _ 66: 0F DB. EE
	pxor	xmm2, xmm1				; 0576 _ 66: 0F EF. D1
	pxor	xmm2, xmm5				; 057A _ 66: 0F EF. D5
	pand	xmm5, xmm1				; 057E _ 66: 0F DB. E9
	pxor	xmm6, xmm0				; 0582 _ 66: 0F EF. F0
	pxor	xmm3, xmm2				; 0586 _ 66: 0F EF. DA
	pxor	xmm5, xmm3				; 058A _ 66: 0F EF. EB
	pand	xmm3, xmm6				; 058E _ 66: 0F DB. DE
	pxor	xmm3, xmm1				; 0592 _ 66: 0F EF. D9
	pxor	xmm6, xmm5				; 0596 _ 66: 0F EF. F5
	pand	xmm1, xmm6				; 059A _ 66: 0F DB. CE
	pxor	xmm3, xmm6				; 059E _ 66: 0F EF. DE
	pxor	xmm1, xmm2				; 05A2 _ 66: 0F EF. CA
	por	xmm1, xmm3				; 05A6 _ 66: 0F EB. CB
	pxor	xmm3, xmm6				; 05AA _ 66: 0F EF. DE
	pxor	xmm1, xmm5				; 05AE _ 66: 0F EF. CD
	movd	xmm5, dword [ecx+2C40H] 		; 05B2 _ 66: 0F 6E. A9, 00002C40
	pshufd	xmm5, xmm5, 0				; 05BA _ 66: 0F 70. ED, 00
	pxor	xmm6, xmm5				; 05BF _ 66: 0F EF. F5
	movd	xmm5, dword [ecx+2C44H] 		; 05C3 _ 66: 0F 6E. A9, 00002C44
	pshufd	xmm5, xmm5, 0				; 05CB _ 66: 0F 70. ED, 00
	pxor	xmm3, xmm5				; 05D0 _ 66: 0F EF. DD
	movd	xmm5, dword [ecx+2C48H] 		; 05D4 _ 66: 0F 6E. A9, 00002C48
	pshufd	xmm5, xmm5, 0				; 05DC _ 66: 0F 70. ED, 00
	pxor	xmm1, xmm5				; 05E1 _ 66: 0F EF. CD
	movd	xmm5, dword [ecx+2C4CH] 		; 05E5 _ 66: 0F 6E. A9, 00002C4C
	pshufd	xmm5, xmm5, 0				; 05ED _ 66: 0F 70. ED, 00
	pxor	xmm2, xmm5				; 05F2 _ 66: 0F EF. D5
	movdqa	xmm5, xmm1				; 05F6 _ 66: 0F 6F. E9
	psrld	xmm1, 22				; 05FA _ 66: 0F 72. D1, 16
	pslld	xmm5, 10				; 05FF _ 66: 0F 72. F5, 0A
	por	xmm5, xmm1				; 0604 _ 66: 0F EB. E9
	movdqa	xmm1, xmm6				; 0608 _ 66: 0F 6F. CE
	psrld	xmm6, 5 				; 060C _ 66: 0F 72. D6, 05
	pxor	xmm5, xmm2				; 0611 _ 66: 0F EF. EA
	pslld	xmm1, 27				; 0615 _ 66: 0F 72. F1, 1B
	por	xmm1, xmm6				; 061A _ 66: 0F EB. CE
	movdqa	xmm6, xmm3				; 061E _ 66: 0F 6F. F3
	pxor	xmm1, xmm3				; 0622 _ 66: 0F EF. CB
	pxor	xmm1, xmm2				; 0626 _ 66: 0F EF. CA
	pslld	xmm6, 7 				; 062A _ 66: 0F 72. F6, 07
	pxor	xmm5, xmm6				; 062F _ 66: 0F EF. EE
	movdqa	xmm6, xmm2				; 0633 _ 66: 0F 6F. F2
	psrld	xmm2, 7 				; 0637 _ 66: 0F 72. D2, 07
	pslld	xmm6, 25				; 063C _ 66: 0F 72. F6, 19
	por	xmm6, xmm2				; 0641 _ 66: 0F EB. F2
	movdqa	xmm2, xmm3				; 0645 _ 66: 0F 6F. D3
	psrld	xmm3, 1 				; 0649 _ 66: 0F 72. D3, 01
	pxor	xmm6, xmm5				; 064E _ 66: 0F EF. F5
	pslld	xmm2, 31				; 0652 _ 66: 0F 72. F2, 1F
	por	xmm2, xmm3				; 0657 _ 66: 0F EB. D3
	movdqa	xmm3, xmm1				; 065B _ 66: 0F 6F. D9
	pxor	xmm2, xmm1				; 065F _ 66: 0F EF. D1
	pxor	xmm2, xmm5				; 0663 _ 66: 0F EF. D5
	pslld	xmm3, 3 				; 0667 _ 66: 0F 72. F3, 03
	pxor	xmm6, xmm3				; 066C _ 66: 0F EF. F3
	movdqa	xmm3, xmm5				; 0670 _ 66: 0F 6F. DD
	psrld	xmm5, 3 				; 0674 _ 66: 0F 72. D5, 03
	pslld	xmm3, 29				; 0679 _ 66: 0F 72. F3, 1D
	por	xmm3, xmm5				; 067E _ 66: 0F EB. DD
	movdqa	xmm5, xmm1				; 0682 _ 66: 0F 6F. E9
	psrld	xmm1, 13				; 0686 _ 66: 0F 72. D1, 0D
	pslld	xmm5, 19				; 068B _ 66: 0F 72. F5, 13
	por	xmm5, xmm1				; 0690 _ 66: 0F EB. E9
	movdqa	xmm1, xmm3				; 0694 _ 66: 0F 6F. CB
	pxor	xmm1, xmm2				; 0698 _ 66: 0F EF. CA
	pxor	xmm5, xmm1				; 069C _ 66: 0F EF. E9
	pand	xmm3, xmm1				; 06A0 _ 66: 0F DB. D9
	pxor	xmm3, xmm5				; 06A4 _ 66: 0F EF. DD
	pand	xmm5, xmm2				; 06A8 _ 66: 0F DB. EA
	pxor	xmm2, xmm6				; 06AC _ 66: 0F EF. D6
	por	xmm6, xmm3				; 06B0 _ 66: 0F EB. F3
	pxor	xmm1, xmm6				; 06B4 _ 66: 0F EF. CE
	pxor	xmm5, xmm6				; 06B8 _ 66: 0F EF. EE
	pxor	xmm2, xmm3				; 06BC _ 66: 0F EF. D3
	pand	xmm6, xmm1				; 06C0 _ 66: 0F DB. F1
	pxor	xmm6, xmm2				; 06C4 _ 66: 0F EF. F2
	pxor	xmm2, xmm5				; 06C8 _ 66: 0F EF. D5
	por	xmm2, xmm1				; 06CC _ 66: 0F EB. D1
	pxor	xmm5, xmm6				; 06D0 _ 66: 0F EF. EE
	pxor	xmm2, xmm3				; 06D4 _ 66: 0F EF. D3
	pxor	xmm5, xmm2				; 06D8 _ 66: 0F EF. EA
	movd	xmm3, dword [ecx+2C30H] 		; 06DC _ 66: 0F 6E. 99, 00002C30
	pshufd	xmm3, xmm3, 0				; 06E4 _ 66: 0F 70. DB, 00
	pxor	xmm1, xmm3				; 06E9 _ 66: 0F EF. CB
	movd	xmm3, dword [ecx+2C34H] 		; 06ED _ 66: 0F 6E. 99, 00002C34
	pshufd	xmm3, xmm3, 0				; 06F5 _ 66: 0F 70. DB, 00
	pxor	xmm2, xmm3				; 06FA _ 66: 0F EF. D3
	movd	xmm3, dword [ecx+2C38H] 		; 06FE _ 66: 0F 6E. 99, 00002C38
	pshufd	xmm3, xmm3, 0				; 0706 _ 66: 0F 70. DB, 00
	pxor	xmm6, xmm3				; 070B _ 66: 0F EF. F3
	movd	xmm3, dword [ecx+2C3CH] 		; 070F _ 66: 0F 6E. 99, 00002C3C
	pshufd	xmm3, xmm3, 0				; 0717 _ 66: 0F 70. DB, 00
	pxor	xmm5, xmm3				; 071C _ 66: 0F EF. EB
	movdqa	xmm3, xmm6				; 0720 _ 66: 0F 6F. DE
	psrld	xmm6, 22				; 0724 _ 66: 0F 72. D6, 16
	pslld	xmm3, 10				; 0729 _ 66: 0F 72. F3, 0A
	por	xmm3, xmm6				; 072E _ 66: 0F EB. DE
	movdqa	xmm6, xmm1				; 0732 _ 66: 0F 6F. F1
	psrld	xmm1, 5 				; 0736 _ 66: 0F 72. D1, 05
	pxor	xmm3, xmm5				; 073B _ 66: 0F EF. DD
	pslld	xmm6, 27				; 073F _ 66: 0F 72. F6, 1B
	por	xmm6, xmm1				; 0744 _ 66: 0F EB. F1
	movdqa	xmm1, xmm2				; 0748 _ 66: 0F 6F. CA
	pxor	xmm6, xmm2				; 074C _ 66: 0F EF. F2
	pxor	xmm6, xmm5				; 0750 _ 66: 0F EF. F5
	pslld	xmm1, 7 				; 0754 _ 66: 0F 72. F1, 07
	pxor	xmm3, xmm1				; 0759 _ 66: 0F EF. D9
	movdqa	xmm1, xmm5				; 075D _ 66: 0F 6F. CD
	psrld	xmm5, 7 				; 0761 _ 66: 0F 72. D5, 07
	pslld	xmm1, 25				; 0766 _ 66: 0F 72. F1, 19
	por	xmm1, xmm5				; 076B _ 66: 0F EB. CD
	movdqa	xmm5, xmm2				; 076F _ 66: 0F 6F. EA
	psrld	xmm2, 1 				; 0773 _ 66: 0F 72. D2, 01
	pxor	xmm1, xmm3				; 0778 _ 66: 0F EF. CB
	pslld	xmm5, 31				; 077C _ 66: 0F 72. F5, 1F
	por	xmm5, xmm2				; 0781 _ 66: 0F EB. EA
	movdqa	xmm2, xmm6				; 0785 _ 66: 0F 6F. D6
	pxor	xmm5, xmm6				; 0789 _ 66: 0F EF. EE
	pxor	xmm5, xmm3				; 078D _ 66: 0F EF. EB
	pslld	xmm2, 3 				; 0791 _ 66: 0F 72. F2, 03
	pxor	xmm1, xmm2				; 0796 _ 66: 0F EF. CA
	movdqa	xmm2, xmm3				; 079A _ 66: 0F 6F. D3
	psrld	xmm3, 3 				; 079E _ 66: 0F 72. D3, 03
	pslld	xmm2, 29				; 07A3 _ 66: 0F 72. F2, 1D
	por	xmm2, xmm3				; 07A8 _ 66: 0F EB. D3
	movdqa	xmm3, xmm6				; 07AC _ 66: 0F 6F. DE
	psrld	xmm6, 13				; 07B0 _ 66: 0F 72. D6, 0D
	pxor	xmm2, xmm1				; 07B5 _ 66: 0F EF. D1
	pslld	xmm3, 19				; 07B9 _ 66: 0F 72. F3, 13
	por	xmm3, xmm6				; 07BE _ 66: 0F EB. DE
	pxor	xmm1, xmm3				; 07C2 _ 66: 0F EF. CB
	movdqa	xmm6, xmm1				; 07C6 _ 66: 0F 6F. F1
	pand	xmm6, xmm2				; 07CA _ 66: 0F DB. F2
	pxor	xmm6, xmm5				; 07CE _ 66: 0F EF. F5
	por	xmm5, xmm2				; 07D2 _ 66: 0F EB. EA
	pxor	xmm5, xmm1				; 07D6 _ 66: 0F EF. E9
	pand	xmm1, xmm6				; 07DA _ 66: 0F DB. CE
	pxor	xmm2, xmm6				; 07DE _ 66: 0F EF. D6
	pand	xmm1, xmm3				; 07E2 _ 66: 0F DB. CB
	pxor	xmm1, xmm2				; 07E6 _ 66: 0F EF. CA
	pand	xmm2, xmm5				; 07EA _ 66: 0F DB. D5
	por	xmm2, xmm3				; 07EE _ 66: 0F EB. D3
	pxor	xmm6, xmm0				; 07F2 _ 66: 0F EF. F0
	movdqa	xmm0, xmm6				; 07F6 _ 66: 0F 6F. C6
	pxor	xmm2, xmm6				; 07FA _ 66: 0F EF. D6
	pxor	xmm3, xmm6				; 07FE _ 66: 0F EF. DE
	movd	xmm6, dword [ecx+2C20H] 		; 0802 _ 66: 0F 6E. B1, 00002C20
	pxor	xmm0, xmm1				; 080A _ 66: 0F EF. C1
	pand	xmm3, xmm5				; 080E _ 66: 0F DB. DD
	pxor	xmm0, xmm3				; 0812 _ 66: 0F EF. C3
	pshufd	xmm3, xmm6, 0				; 0816 _ 66: 0F 70. DE, 00
	pxor	xmm5, xmm3				; 081B _ 66: 0F EF. EB
	movd	xmm6, dword [ecx+2C24H] 		; 081F _ 66: 0F 6E. B1, 00002C24
	pshufd	xmm3, xmm6, 0				; 0827 _ 66: 0F 70. DE, 00
	movd	xmm6, dword [ecx+2C28H] 		; 082C _ 66: 0F 6E. B1, 00002C28
	pxor	xmm1, xmm3				; 0834 _ 66: 0F EF. CB
	pshufd	xmm3, xmm6, 0				; 0838 _ 66: 0F 70. DE, 00
	pxor	xmm2, xmm3				; 083D _ 66: 0F EF. D3
	movd	xmm3, dword [ecx+2C2CH] 		; 0841 _ 66: 0F 6E. 99, 00002C2C
	pshufd	xmm6, xmm3, 0				; 0849 _ 66: 0F 70. F3, 00
	movdqa	xmm3, xmm2				; 084E _ 66: 0F 6F. DA
	pxor	xmm0, xmm6				; 0852 _ 66: 0F EF. C6
	movdqa	xmm6, xmm5				; 0856 _ 66: 0F 6F. F5
	pslld	xmm3, 10				; 085A _ 66: 0F 72. F3, 0A
	psrld	xmm2, 22				; 085F _ 66: 0F 72. D2, 16
	por	xmm3, xmm2				; 0864 _ 66: 0F EB. DA
	movdqa	xmm2, xmm1				; 0868 _ 66: 0F 6F. D1
	pslld	xmm6, 27				; 086C _ 66: 0F 72. F6, 1B
	psrld	xmm5, 5 				; 0871 _ 66: 0F 72. D5, 05
	por	xmm6, xmm5				; 0876 _ 66: 0F EB. F5
	movdqa	xmm5, xmm0				; 087A _ 66: 0F 6F. E8
	pxor	xmm3, xmm0				; 087E _ 66: 0F EF. D8
	pslld	xmm2, 7 				; 0882 _ 66: 0F 72. F2, 07
	pxor	xmm3, xmm2				; 0887 _ 66: 0F EF. DA
	movdqa	xmm2, xmm1				; 088B _ 66: 0F 6F. D1
	pxor	xmm6, xmm1				; 088F _ 66: 0F EF. F1
	pxor	xmm6, xmm0				; 0893 _ 66: 0F EF. F0
	pslld	xmm5, 25				; 0897 _ 66: 0F 72. F5, 19
	psrld	xmm0, 7 				; 089C _ 66: 0F 72. D0, 07
	por	xmm5, xmm0				; 08A1 _ 66: 0F EB. E8
	movdqa	xmm0, xmm6				; 08A5 _ 66: 0F 6F. C6
	pslld	xmm2, 31				; 08A9 _ 66: 0F 72. F2, 1F
	psrld	xmm1, 1 				; 08AE _ 66: 0F 72. D1, 01
	por	xmm2, xmm1				; 08B3 _ 66: 0F EB. D1
	movdqa	xmm1, xmm6				; 08B7 _ 66: 0F 6F. CE
	pxor	xmm5, xmm3				; 08BB _ 66: 0F EF. EB
	pxor	xmm2, xmm6				; 08BF _ 66: 0F EF. D6
	pslld	xmm1, 3 				; 08C3 _ 66: 0F 72. F1, 03
	pxor	xmm5, xmm1				; 08C8 _ 66: 0F EF. E9
	movdqa	xmm1, xmm3				; 08CC _ 66: 0F 6F. CB
	pxor	xmm2, xmm3				; 08D0 _ 66: 0F EF. D3
	psrld	xmm3, 3 				; 08D4 _ 66: 0F 72. D3, 03
	pslld	xmm1, 29				; 08D9 _ 66: 0F 72. F1, 1D
	por	xmm1, xmm3				; 08DE _ 66: 0F EB. CB
	movdqa	xmm3, xmm2				; 08E2 _ 66: 0F 6F. DA
	pslld	xmm0, 19				; 08E6 _ 66: 0F 72. F0, 13
	psrld	xmm6, 13				; 08EB _ 66: 0F 72. D6, 0D
	por	xmm0, xmm6				; 08F0 _ 66: 0F EB. C6
	pxor	xmm3, xmm5				; 08F4 _ 66: 0F EF. DD
	pand	xmm5, xmm3				; 08F8 _ 66: 0F DB. EB
	pxor	xmm2, xmm1				; 08FC _ 66: 0F EF. D1
	pxor	xmm5, xmm0				; 0900 _ 66: 0F EF. E8
	por	xmm0, xmm3				; 0904 _ 66: 0F EB. C3
	pxor	xmm1, xmm5				; 0908 _ 66: 0F EF. CD
	pxor	xmm0, xmm2				; 090C _ 66: 0F EF. C2
	por	xmm0, xmm1				; 0910 _ 66: 0F EB. C1
	pxor	xmm3, xmm5				; 0914 _ 66: 0F EF. DD
	pxor	xmm0, xmm3				; 0918 _ 66: 0F EF. C3
	por	xmm3, xmm5				; 091C _ 66: 0F EB. DD
	pxor	xmm3, xmm0				; 0920 _ 66: 0F EF. D8
	pcmpeqd xmm6, xmm6				; 0924 _ 66: 0F 76. F6
	pxor	xmm2, xmm6				; 0928 _ 66: 0F EF. D6
	pxor	xmm2, xmm3				; 092C _ 66: 0F EF. D3
	por	xmm3, xmm0				; 0930 _ 66: 0F EB. D8
	pxor	xmm3, xmm0				; 0934 _ 66: 0F EF. D8
	por	xmm3, xmm2				; 0938 _ 66: 0F EB. DA
	pxor	xmm5, xmm3				; 093C _ 66: 0F EF. EB
	movd	xmm3, dword [ecx+2C10H] 		; 0940 _ 66: 0F 6E. 99, 00002C10
	pshufd	xmm3, xmm3, 0				; 0948 _ 66: 0F 70. DB, 00
	pxor	xmm2, xmm3				; 094D _ 66: 0F EF. D3
	movd	xmm3, dword [ecx+2C14H] 		; 0951 _ 66: 0F 6E. 99, 00002C14
	pshufd	xmm3, xmm3, 0				; 0959 _ 66: 0F 70. DB, 00
	pxor	xmm0, xmm3				; 095E _ 66: 0F EF. C3
	movd	xmm3, dword [ecx+2C18H] 		; 0962 _ 66: 0F 6E. 99, 00002C18
	pshufd	xmm3, xmm3, 0				; 096A _ 66: 0F 70. DB, 00
	pxor	xmm5, xmm3				; 096F _ 66: 0F EF. EB
	movd	xmm3, dword [ecx+2C1CH] 		; 0973 _ 66: 0F 6E. 99, 00002C1C
	pshufd	xmm3, xmm3, 0				; 097B _ 66: 0F 70. DB, 00
	pxor	xmm1, xmm3				; 0980 _ 66: 0F EF. CB
	movdqa	xmm3, xmm5				; 0984 _ 66: 0F 6F. DD
	psrld	xmm5, 22				; 0988 _ 66: 0F 72. D5, 16
	pslld	xmm3, 10				; 098D _ 66: 0F 72. F3, 0A
	por	xmm3, xmm5				; 0992 _ 66: 0F EB. DD
	movdqa	xmm5, xmm2				; 0996 _ 66: 0F 6F. EA
	psrld	xmm2, 5 				; 099A _ 66: 0F 72. D2, 05
	pxor	xmm3, xmm1				; 099F _ 66: 0F EF. D9
	pslld	xmm5, 27				; 09A3 _ 66: 0F 72. F5, 1B
	por	xmm5, xmm2				; 09A8 _ 66: 0F EB. EA
	movdqa	xmm2, xmm0				; 09AC _ 66: 0F 6F. D0
	pxor	xmm5, xmm0				; 09B0 _ 66: 0F EF. E8
	pxor	xmm5, xmm1				; 09B4 _ 66: 0F EF. E9
	pslld	xmm2, 7 				; 09B8 _ 66: 0F 72. F2, 07
	pxor	xmm3, xmm2				; 09BD _ 66: 0F EF. DA
	movdqa	xmm2, xmm1				; 09C1 _ 66: 0F 6F. D1
	psrld	xmm1, 7 				; 09C5 _ 66: 0F 72. D1, 07
	pslld	xmm2, 25				; 09CA _ 66: 0F 72. F2, 19
	por	xmm2, xmm1				; 09CF _ 66: 0F EB. D1
	movdqa	xmm1, xmm0				; 09D3 _ 66: 0F 6F. C8
	psrld	xmm0, 1 				; 09D7 _ 66: 0F 72. D0, 01
	pxor	xmm2, xmm3				; 09DC _ 66: 0F EF. D3
	pslld	xmm1, 31				; 09E0 _ 66: 0F 72. F1, 1F
	por	xmm1, xmm0				; 09E5 _ 66: 0F EB. C8
	movdqa	xmm0, xmm5				; 09E9 _ 66: 0F 6F. C5
	pxor	xmm1, xmm5				; 09ED _ 66: 0F EF. CD
	pxor	xmm1, xmm3				; 09F1 _ 66: 0F EF. CB
	pslld	xmm0, 3 				; 09F5 _ 66: 0F 72. F0, 03
	pxor	xmm2, xmm0				; 09FA _ 66: 0F EF. D0
	movdqa	xmm0, xmm3				; 09FE _ 66: 0F 6F. C3
	psrld	xmm3, 3 				; 0A02 _ 66: 0F 72. D3, 03
	pslld	xmm0, 29				; 0A07 _ 66: 0F 72. F0, 1D
	por	xmm0, xmm3				; 0A0C _ 66: 0F EB. C3
	movdqa	xmm3, xmm5				; 0A10 _ 66: 0F 6F. DD
	psrld	xmm5, 13				; 0A14 _ 66: 0F 72. D5, 0D
	pxor	xmm0, xmm6				; 0A19 _ 66: 0F EF. C6
	pslld	xmm3, 19				; 0A1D _ 66: 0F 72. F3, 13
	por	xmm3, xmm5				; 0A22 _ 66: 0F EB. DD
	movdqa	xmm5, xmm1				; 0A26 _ 66: 0F 6F. E9
	pxor	xmm1, xmm6				; 0A2A _ 66: 0F EF. CE
	por	xmm5, xmm3				; 0A2E _ 66: 0F EB. EB
	pxor	xmm5, xmm0				; 0A32 _ 66: 0F EF. E8
	por	xmm0, xmm1				; 0A36 _ 66: 0F EB. C1
	pxor	xmm5, xmm2				; 0A3A _ 66: 0F EF. EA
	pxor	xmm3, xmm1				; 0A3E _ 66: 0F EF. D9
	pxor	xmm0, xmm3				; 0A42 _ 66: 0F EF. C3
	pand	xmm3, xmm2				; 0A46 _ 66: 0F DB. DA
	pxor	xmm1, xmm3				; 0A4A _ 66: 0F EF. CB
	por	xmm3, xmm5				; 0A4E _ 66: 0F EB. DD
	pxor	xmm3, xmm0				; 0A52 _ 66: 0F EF. D8
	pxor	xmm2, xmm1				; 0A56 _ 66: 0F EF. D1
	pxor	xmm0, xmm5				; 0A5A _ 66: 0F EF. C5
	pxor	xmm2, xmm3				; 0A5E _ 66: 0F EF. D3
	pxor	xmm2, xmm5				; 0A62 _ 66: 0F EF. D5
	pand	xmm0, xmm2				; 0A66 _ 66: 0F DB. C2
	pxor	xmm1, xmm0				; 0A6A _ 66: 0F EF. C8
	movd	xmm0, dword [ecx+2C00H] 		; 0A6E _ 66: 0F 6E. 81, 00002C00
	pshufd	xmm0, xmm0, 0				; 0A76 _ 66: 0F 70. C0, 00
	pxor	xmm3, xmm0				; 0A7B _ 66: 0F EF. D8
	movd	xmm0, dword [ecx+2C04H] 		; 0A7F _ 66: 0F 6E. 81, 00002C04
	pshufd	xmm0, xmm0, 0				; 0A87 _ 66: 0F 70. C0, 00
	pxor	xmm1, xmm0				; 0A8C _ 66: 0F EF. C8
	movd	xmm0, dword [ecx+2C08H] 		; 0A90 _ 66: 0F 6E. 81, 00002C08
	pshufd	xmm0, xmm0, 0				; 0A98 _ 66: 0F 70. C0, 00
	pxor	xmm5, xmm0				; 0A9D _ 66: 0F EF. E8
	movd	xmm0, dword [ecx+2C0CH] 		; 0AA1 _ 66: 0F 6E. 81, 00002C0C
	pshufd	xmm0, xmm0, 0				; 0AA9 _ 66: 0F 70. C0, 00
	pxor	xmm2, xmm0				; 0AAE _ 66: 0F EF. D0
	movdqa	xmm0, xmm5				; 0AB2 _ 66: 0F 6F. C5
	psrld	xmm5, 22				; 0AB6 _ 66: 0F 72. D5, 16
	pslld	xmm0, 10				; 0ABB _ 66: 0F 72. F0, 0A
	por	xmm0, xmm5				; 0AC0 _ 66: 0F EB. C5
	movdqa	xmm5, xmm3				; 0AC4 _ 66: 0F 6F. EB
	psrld	xmm3, 5 				; 0AC8 _ 66: 0F 72. D3, 05
	pxor	xmm0, xmm2				; 0ACD _ 66: 0F EF. C2
	pslld	xmm5, 27				; 0AD1 _ 66: 0F 72. F5, 1B
	por	xmm5, xmm3				; 0AD6 _ 66: 0F EB. EB
	movdqa	xmm3, xmm1				; 0ADA _ 66: 0F 6F. D9
	pxor	xmm5, xmm1				; 0ADE _ 66: 0F EF. E9
	pxor	xmm5, xmm2				; 0AE2 _ 66: 0F EF. EA
	pslld	xmm3, 7 				; 0AE6 _ 66: 0F 72. F3, 07
	pxor	xmm0, xmm3				; 0AEB _ 66: 0F EF. C3
	movdqa	xmm3, xmm2				; 0AEF _ 66: 0F 6F. DA
	psrld	xmm2, 7 				; 0AF3 _ 66: 0F 72. D2, 07
	pslld	xmm3, 25				; 0AF8 _ 66: 0F 72. F3, 19
	por	xmm3, xmm2				; 0AFD _ 66: 0F EB. DA
	movdqa	xmm2, xmm1				; 0B01 _ 66: 0F 6F. D1
	psrld	xmm1, 1 				; 0B05 _ 66: 0F 72. D1, 01
	pxor	xmm3, xmm0				; 0B0A _ 66: 0F EF. D8
	pslld	xmm2, 31				; 0B0E _ 66: 0F 72. F2, 1F
	por	xmm2, xmm1				; 0B13 _ 66: 0F EB. D1
	movdqa	xmm1, xmm5				; 0B17 _ 66: 0F 6F. CD
	pxor	xmm2, xmm5				; 0B1B _ 66: 0F EF. D5
	pxor	xmm2, xmm0				; 0B1F _ 66: 0F EF. D0
	pslld	xmm1, 3 				; 0B23 _ 66: 0F 72. F1, 03
	pxor	xmm3, xmm1				; 0B28 _ 66: 0F EF. D9
	movdqa	xmm1, xmm0				; 0B2C _ 66: 0F 6F. C8
	psrld	xmm0, 3 				; 0B30 _ 66: 0F 72. D0, 03
	pslld	xmm1, 29				; 0B35 _ 66: 0F 72. F1, 1D
	por	xmm1, xmm0				; 0B3A _ 66: 0F EB. C8
	movdqa	xmm0, xmm5				; 0B3E _ 66: 0F 6F. C5
	psrld	xmm5, 13				; 0B42 _ 66: 0F 72. D5, 0D
	pslld	xmm0, 19				; 0B47 _ 66: 0F 72. F0, 13
	por	xmm0, xmm5				; 0B4C _ 66: 0F EB. C5
	movdqa	xmm5, xmm1				; 0B50 _ 66: 0F 6F. E9
	por	xmm1, xmm3				; 0B54 _ 66: 0F EB. CB
	pxor	xmm5, xmm0				; 0B58 _ 66: 0F EF. E8
	pand	xmm0, xmm3				; 0B5C _ 66: 0F DB. C3
	pxor	xmm5, xmm6				; 0B60 _ 66: 0F EF. EE
	pxor	xmm3, xmm2				; 0B64 _ 66: 0F EF. DA
	por	xmm2, xmm0				; 0B68 _ 66: 0F EB. D0
	pxor	xmm0, xmm5				; 0B6C _ 66: 0F EF. C5
	pand	xmm5, xmm1				; 0B70 _ 66: 0F DB. E9
	pand	xmm3, xmm1				; 0B74 _ 66: 0F DB. D9
	pxor	xmm2, xmm5				; 0B78 _ 66: 0F EF. D5
	pxor	xmm5, xmm0				; 0B7C _ 66: 0F EF. E8
	por	xmm0, xmm5				; 0B80 _ 66: 0F EB. C5
	pxor	xmm1, xmm2				; 0B84 _ 66: 0F EF. CA
	pxor	xmm0, xmm3				; 0B88 _ 66: 0F EF. C3
	pxor	xmm3, xmm1				; 0B8C _ 66: 0F EF. D9
	por	xmm1, xmm0				; 0B90 _ 66: 0F EB. C8
	pxor	xmm3, xmm5				; 0B94 _ 66: 0F EF. DD
	pxor	xmm1, xmm5				; 0B98 _ 66: 0F EF. CD
	movd	xmm5, dword [ecx+2BF0H] 		; 0B9C _ 66: 0F 6E. A9, 00002BF0
	pshufd	xmm5, xmm5, 0				; 0BA4 _ 66: 0F 70. ED, 00
	pxor	xmm3, xmm5				; 0BA9 _ 66: 0F EF. DD
	movd	xmm5, dword [ecx+2BF4H] 		; 0BAD _ 66: 0F 6E. A9, 00002BF4
	pshufd	xmm5, xmm5, 0				; 0BB5 _ 66: 0F 70. ED, 00
	pxor	xmm0, xmm5				; 0BBA _ 66: 0F EF. C5
	movd	xmm5, dword [ecx+2BF8H] 		; 0BBE _ 66: 0F 6E. A9, 00002BF8
	pshufd	xmm5, xmm5, 0				; 0BC6 _ 66: 0F 70. ED, 00
	pxor	xmm2, xmm5				; 0BCB _ 66: 0F EF. D5
	movd	xmm5, dword [ecx+2BFCH] 		; 0BCF _ 66: 0F 6E. A9, 00002BFC
	pshufd	xmm5, xmm5, 0				; 0BD7 _ 66: 0F 70. ED, 00
	pxor	xmm1, xmm5				; 0BDC _ 66: 0F EF. CD
	movdqa	xmm5, xmm2				; 0BE0 _ 66: 0F 6F. EA
	psrld	xmm2, 22				; 0BE4 _ 66: 0F 72. D2, 16
	pslld	xmm5, 10				; 0BE9 _ 66: 0F 72. F5, 0A
	por	xmm5, xmm2				; 0BEE _ 66: 0F EB. EA
	movdqa	xmm2, xmm3				; 0BF2 _ 66: 0F 6F. D3
	psrld	xmm3, 5 				; 0BF6 _ 66: 0F 72. D3, 05
	pxor	xmm5, xmm1				; 0BFB _ 66: 0F EF. E9
	pslld	xmm2, 27				; 0BFF _ 66: 0F 72. F2, 1B
	por	xmm2, xmm3				; 0C04 _ 66: 0F EB. D3
	movdqa	xmm3, xmm0				; 0C08 _ 66: 0F 6F. D8
	pxor	xmm2, xmm0				; 0C0C _ 66: 0F EF. D0
	pxor	xmm2, xmm1				; 0C10 _ 66: 0F EF. D1
	pslld	xmm3, 7 				; 0C14 _ 66: 0F 72. F3, 07
	pxor	xmm5, xmm3				; 0C19 _ 66: 0F EF. EB
	movdqa	xmm3, xmm1				; 0C1D _ 66: 0F 6F. D9
	psrld	xmm1, 7 				; 0C21 _ 66: 0F 72. D1, 07
	pslld	xmm3, 25				; 0C26 _ 66: 0F 72. F3, 19
	por	xmm3, xmm1				; 0C2B _ 66: 0F EB. D9
	movdqa	xmm1, xmm0				; 0C2F _ 66: 0F 6F. C8
	psrld	xmm0, 1 				; 0C33 _ 66: 0F 72. D0, 01
	pxor	xmm3, xmm5				; 0C38 _ 66: 0F EF. DD
	pslld	xmm1, 31				; 0C3C _ 66: 0F 72. F1, 1F
	por	xmm1, xmm0				; 0C41 _ 66: 0F EB. C8
	movdqa	xmm0, xmm2				; 0C45 _ 66: 0F 6F. C2
	pxor	xmm1, xmm2				; 0C49 _ 66: 0F EF. CA
	pxor	xmm1, xmm5				; 0C4D _ 66: 0F EF. CD
	pslld	xmm0, 3 				; 0C51 _ 66: 0F 72. F0, 03
	pxor	xmm3, xmm0				; 0C56 _ 66: 0F EF. D8
	movdqa	xmm0, xmm5				; 0C5A _ 66: 0F 6F. C5
	psrld	xmm5, 3 				; 0C5E _ 66: 0F 72. D5, 03
	pslld	xmm0, 29				; 0C63 _ 66: 0F 72. F0, 1D
	por	xmm0, xmm5				; 0C68 _ 66: 0F EB. C5
	movdqa	xmm5, xmm2				; 0C6C _ 66: 0F 6F. EA
	psrld	xmm2, 13				; 0C70 _ 66: 0F 72. D2, 0D
	pslld	xmm5, 19				; 0C75 _ 66: 0F 72. F5, 13
	por	xmm5, xmm2				; 0C7A _ 66: 0F EB. EA
	movdqa	xmm2, xmm0				; 0C7E _ 66: 0F 6F. D0
	pxor	xmm5, xmm0				; 0C82 _ 66: 0F EF. E8
	pxor	xmm0, xmm3				; 0C86 _ 66: 0F EF. C3
	pand	xmm2, xmm5				; 0C8A _ 66: 0F DB. D5
	pxor	xmm2, xmm6				; 0C8E _ 66: 0F EF. D6
	pxor	xmm3, xmm1				; 0C92 _ 66: 0F EF. D9
	pxor	xmm2, xmm3				; 0C96 _ 66: 0F EF. D3
	por	xmm0, xmm5				; 0C9A _ 66: 0F EB. C5
	pxor	xmm5, xmm2				; 0C9E _ 66: 0F EF. EA
	pxor	xmm3, xmm0				; 0CA2 _ 66: 0F EF. D8
	pxor	xmm0, xmm1				; 0CA6 _ 66: 0F EF. C1
	pand	xmm1, xmm3				; 0CAA _ 66: 0F DB. CB
	pxor	xmm1, xmm5				; 0CAE _ 66: 0F EF. CD
	pxor	xmm5, xmm3				; 0CB2 _ 66: 0F EF. EB
	por	xmm5, xmm2				; 0CB6 _ 66: 0F EB. EA
	pxor	xmm3, xmm1				; 0CBA _ 66: 0F EF. D9
	pxor	xmm0, xmm5				; 0CBE _ 66: 0F EF. C5
	movd	xmm5, dword [ecx+2BE0H] 		; 0CC2 _ 66: 0F 6E. A9, 00002BE0
	pshufd	xmm5, xmm5, 0				; 0CCA _ 66: 0F 70. ED, 00
	pxor	xmm1, xmm5				; 0CCF _ 66: 0F EF. CD
	movd	xmm5, dword [ecx+2BE4H] 		; 0CD3 _ 66: 0F 6E. A9, 00002BE4
	pshufd	xmm5, xmm5, 0				; 0CDB _ 66: 0F 70. ED, 00
	pxor	xmm2, xmm5				; 0CE0 _ 66: 0F EF. D5
	movd	xmm5, dword [ecx+2BE8H] 		; 0CE4 _ 66: 0F 6E. A9, 00002BE8
	pshufd	xmm5, xmm5, 0				; 0CEC _ 66: 0F 70. ED, 00
	pxor	xmm0, xmm5				; 0CF1 _ 66: 0F EF. C5
	movd	xmm5, dword [ecx+2BECH] 		; 0CF5 _ 66: 0F 6E. A9, 00002BEC
	pshufd	xmm5, xmm5, 0				; 0CFD _ 66: 0F 70. ED, 00
	pxor	xmm3, xmm5				; 0D02 _ 66: 0F EF. DD
	movdqa	xmm5, xmm0				; 0D06 _ 66: 0F 6F. E8
	psrld	xmm0, 22				; 0D0A _ 66: 0F 72. D0, 16
	pslld	xmm5, 10				; 0D0F _ 66: 0F 72. F5, 0A
	por	xmm5, xmm0				; 0D14 _ 66: 0F EB. E8
	movdqa	xmm0, xmm1				; 0D18 _ 66: 0F 6F. C1
	psrld	xmm1, 5 				; 0D1C _ 66: 0F 72. D1, 05
	pxor	xmm5, xmm3				; 0D21 _ 66: 0F EF. EB
	pslld	xmm0, 27				; 0D25 _ 66: 0F 72. F0, 1B
	por	xmm0, xmm1				; 0D2A _ 66: 0F EB. C1
	movdqa	xmm1, xmm2				; 0D2E _ 66: 0F 6F. CA
	pxor	xmm0, xmm2				; 0D32 _ 66: 0F EF. C2
	pxor	xmm0, xmm3				; 0D36 _ 66: 0F EF. C3
	pslld	xmm1, 7 				; 0D3A _ 66: 0F 72. F1, 07
	pxor	xmm5, xmm1				; 0D3F _ 66: 0F EF. E9
	movdqa	xmm1, xmm3				; 0D43 _ 66: 0F 6F. CB
	psrld	xmm3, 7 				; 0D47 _ 66: 0F 72. D3, 07
	pslld	xmm1, 25				; 0D4C _ 66: 0F 72. F1, 19
	por	xmm1, xmm3				; 0D51 _ 66: 0F EB. CB
	movdqa	xmm3, xmm2				; 0D55 _ 66: 0F 6F. DA
	psrld	xmm2, 1 				; 0D59 _ 66: 0F 72. D2, 01
	pxor	xmm1, xmm5				; 0D5E _ 66: 0F EF. CD
	pslld	xmm3, 31				; 0D62 _ 66: 0F 72. F3, 1F
	por	xmm3, xmm2				; 0D67 _ 66: 0F EB. DA
	movdqa	xmm2, xmm0				; 0D6B _ 66: 0F 6F. D0
	pxor	xmm3, xmm0				; 0D6F _ 66: 0F EF. D8
	pxor	xmm3, xmm5				; 0D73 _ 66: 0F EF. DD
	pslld	xmm2, 3 				; 0D77 _ 66: 0F 72. F2, 03
	pxor	xmm1, xmm2				; 0D7C _ 66: 0F EF. CA
	movdqa	xmm2, xmm5				; 0D80 _ 66: 0F 6F. D5
	psrld	xmm5, 3 				; 0D84 _ 66: 0F 72. D5, 03
	pxor	xmm3, xmm6				; 0D89 _ 66: 0F EF. DE
	pslld	xmm2, 29				; 0D8D _ 66: 0F 72. F2, 1D
	por	xmm2, xmm5				; 0D92 _ 66: 0F EB. D5
	movdqa	xmm5, xmm0				; 0D96 _ 66: 0F 6F. E8
	psrld	xmm0, 13				; 0D9A _ 66: 0F 72. D0, 0D
	pxor	xmm2, xmm3				; 0D9F _ 66: 0F EF. D3
	pslld	xmm5, 19				; 0DA3 _ 66: 0F 72. F5, 13
	por	xmm5, xmm0				; 0DA8 _ 66: 0F EB. E8
	movdqa	xmm0, xmm1				; 0DAC _ 66: 0F 6F. C1
	por	xmm0, xmm5				; 0DB0 _ 66: 0F EB. C5
	pxor	xmm0, xmm2				; 0DB4 _ 66: 0F EF. C2
	por	xmm2, xmm3				; 0DB8 _ 66: 0F EB. D3
	pand	xmm2, xmm5				; 0DBC _ 66: 0F DB. D5
	pxor	xmm1, xmm0				; 0DC0 _ 66: 0F EF. C8
	pxor	xmm2, xmm1				; 0DC4 _ 66: 0F EF. D1
	por	xmm1, xmm5				; 0DC8 _ 66: 0F EB. CD
	pxor	xmm1, xmm3				; 0DCC _ 66: 0F EF. CB
	pand	xmm3, xmm2				; 0DD0 _ 66: 0F DB. DA
	pxor	xmm3, xmm0				; 0DD4 _ 66: 0F EF. D8
	pxor	xmm1, xmm2				; 0DD8 _ 66: 0F EF. CA
	pand	xmm0, xmm1				; 0DDC _ 66: 0F DB. C1
	pxor	xmm1, xmm3				; 0DE0 _ 66: 0F EF. CB
	pxor	xmm0, xmm1				; 0DE4 _ 66: 0F EF. C1
	pxor	xmm1, xmm6				; 0DE8 _ 66: 0F EF. CE
	pxor	xmm0, xmm5				; 0DEC _ 66: 0F EF. C5
	movd	xmm5, dword [ecx+2BD0H] 		; 0DF0 _ 66: 0F 6E. A9, 00002BD0
	pshufd	xmm5, xmm5, 0				; 0DF8 _ 66: 0F 70. ED, 00
	pxor	xmm3, xmm5				; 0DFD _ 66: 0F EF. DD
	movd	xmm5, dword [ecx+2BD4H] 		; 0E01 _ 66: 0F 6E. A9, 00002BD4
	pshufd	xmm5, xmm5, 0				; 0E09 _ 66: 0F 70. ED, 00
	pxor	xmm1, xmm5				; 0E0E _ 66: 0F EF. CD
	movd	xmm5, dword [ecx+2BD8H] 		; 0E12 _ 66: 0F 6E. A9, 00002BD8
	pshufd	xmm5, xmm5, 0				; 0E1A _ 66: 0F 70. ED, 00
	pxor	xmm0, xmm5				; 0E1F _ 66: 0F EF. C5
	movd	xmm5, dword [ecx+2BDCH] 		; 0E23 _ 66: 0F 6E. A9, 00002BDC
	pshufd	xmm5, xmm5, 0				; 0E2B _ 66: 0F 70. ED, 00
	pxor	xmm2, xmm5				; 0E30 _ 66: 0F EF. D5
	movdqa	xmm5, xmm0				; 0E34 _ 66: 0F 6F. E8
	psrld	xmm0, 22				; 0E38 _ 66: 0F 72. D0, 16
	pslld	xmm5, 10				; 0E3D _ 66: 0F 72. F5, 0A
	por	xmm5, xmm0				; 0E42 _ 66: 0F EB. E8
	movdqa	xmm0, xmm3				; 0E46 _ 66: 0F 6F. C3
	psrld	xmm3, 5 				; 0E4A _ 66: 0F 72. D3, 05
	pxor	xmm5, xmm2				; 0E4F _ 66: 0F EF. EA
	pslld	xmm0, 27				; 0E53 _ 66: 0F 72. F0, 1B
	por	xmm0, xmm3				; 0E58 _ 66: 0F EB. C3
	movdqa	xmm3, xmm1				; 0E5C _ 66: 0F 6F. D9
	pxor	xmm0, xmm1				; 0E60 _ 66: 0F EF. C1
	pxor	xmm0, xmm2				; 0E64 _ 66: 0F EF. C2
	pslld	xmm3, 7 				; 0E68 _ 66: 0F 72. F3, 07
	pxor	xmm5, xmm3				; 0E6D _ 66: 0F EF. EB
	movdqa	xmm3, xmm2				; 0E71 _ 66: 0F 6F. DA
	psrld	xmm2, 7 				; 0E75 _ 66: 0F 72. D2, 07
	pslld	xmm3, 25				; 0E7A _ 66: 0F 72. F3, 19
	por	xmm3, xmm2				; 0E7F _ 66: 0F EB. DA
	movdqa	xmm2, xmm1				; 0E83 _ 66: 0F 6F. D1
	psrld	xmm1, 1 				; 0E87 _ 66: 0F 72. D1, 01
	pxor	xmm3, xmm5				; 0E8C _ 66: 0F EF. DD
	pslld	xmm2, 31				; 0E90 _ 66: 0F 72. F2, 1F
	por	xmm2, xmm1				; 0E95 _ 66: 0F EB. D1
	movdqa	xmm1, xmm0				; 0E99 _ 66: 0F 6F. C8
	pxor	xmm2, xmm0				; 0E9D _ 66: 0F EF. D0
	pxor	xmm2, xmm5				; 0EA1 _ 66: 0F EF. D5
	pslld	xmm1, 3 				; 0EA5 _ 66: 0F 72. F1, 03
	pxor	xmm3, xmm1				; 0EAA _ 66: 0F EF. D9
	movdqa	xmm1, xmm5				; 0EAE _ 66: 0F 6F. CD
	psrld	xmm5, 3 				; 0EB2 _ 66: 0F 72. D5, 03
	pslld	xmm1, 29				; 0EB7 _ 66: 0F 72. F1, 1D
	por	xmm1, xmm5				; 0EBC _ 66: 0F EB. CD
	movdqa	xmm5, xmm0				; 0EC0 _ 66: 0F 6F. E8
	psrld	xmm0, 13				; 0EC4 _ 66: 0F 72. D0, 0D
	pslld	xmm5, 19				; 0EC9 _ 66: 0F 72. F5, 13
	por	xmm5, xmm0				; 0ECE _ 66: 0F EB. E8
	movdqa	xmm0, xmm1				; 0ED2 _ 66: 0F 6F. C1
	pand	xmm0, xmm3				; 0ED6 _ 66: 0F DB. C3
	pxor	xmm0, xmm2				; 0EDA _ 66: 0F EF. C2
	por	xmm2, xmm3				; 0EDE _ 66: 0F EB. D3
	pand	xmm2, xmm5				; 0EE2 _ 66: 0F DB. D5
	pxor	xmm1, xmm0				; 0EE6 _ 66: 0F EF. C8
	pxor	xmm1, xmm2				; 0EEA _ 66: 0F EF. CA
	pand	xmm2, xmm0				; 0EEE _ 66: 0F DB. D0
	pxor	xmm5, xmm6				; 0EF2 _ 66: 0F EF. EE
	pxor	xmm3, xmm1				; 0EF6 _ 66: 0F EF. D9
	pxor	xmm2, xmm3				; 0EFA _ 66: 0F EF. D3
	pand	xmm3, xmm5				; 0EFE _ 66: 0F DB. DD
	pxor	xmm3, xmm0				; 0F02 _ 66: 0F EF. D8
	movd	xmm6, dword [ecx+2BC0H] 		; 0F06 _ 66: 0F 6E. B1, 00002BC0
	pxor	xmm5, xmm2				; 0F0E _ 66: 0F EF. EA
	pand	xmm0, xmm5				; 0F12 _ 66: 0F DB. C5
	pxor	xmm3, xmm5				; 0F16 _ 66: 0F EF. DD
	pxor	xmm0, xmm1				; 0F1A _ 66: 0F EF. C1
	por	xmm0, xmm3				; 0F1E _ 66: 0F EB. C3
	pxor	xmm3, xmm5				; 0F22 _ 66: 0F EF. DD
	pxor	xmm0, xmm2				; 0F26 _ 66: 0F EF. C2
	pshufd	xmm6, xmm6, 0				; 0F2A _ 66: 0F 70. F6, 00
	pxor	xmm5, xmm6				; 0F2F _ 66: 0F EF. EE
	movd	xmm2, dword [ecx+2BC4H] 		; 0F33 _ 66: 0F 6E. 91, 00002BC4
	pshufd	xmm6, xmm2, 0				; 0F3B _ 66: 0F 70. F2, 00
	pxor	xmm3, xmm6				; 0F40 _ 66: 0F EF. DE
	movd	xmm2, dword [ecx+2BC8H] 		; 0F44 _ 66: 0F 6E. 91, 00002BC8
	pshufd	xmm6, xmm2, 0				; 0F4C _ 66: 0F 70. F2, 00
	movd	xmm2, dword [ecx+2BCCH] 		; 0F51 _ 66: 0F 6E. 91, 00002BCC
	pxor	xmm0, xmm6				; 0F59 _ 66: 0F EF. C6
	pshufd	xmm6, xmm2, 0				; 0F5D _ 66: 0F 70. F2, 00
	pxor	xmm1, xmm6				; 0F62 _ 66: 0F EF. CE
	movdqa	xmm6, xmm0				; 0F66 _ 66: 0F 6F. F0
	movdqa	xmm2, xmm1				; 0F6A _ 66: 0F 6F. D1
	psrld	xmm0, 22				; 0F6E _ 66: 0F 72. D0, 16
	pslld	xmm6, 10				; 0F73 _ 66: 0F 72. F6, 0A
	por	xmm6, xmm0				; 0F78 _ 66: 0F EB. F0
	movdqa	xmm0, xmm5				; 0F7C _ 66: 0F 6F. C5
	psrld	xmm5, 5 				; 0F80 _ 66: 0F 72. D5, 05
	pxor	xmm6, xmm1				; 0F85 _ 66: 0F EF. F1
	pslld	xmm0, 27				; 0F89 _ 66: 0F 72. F0, 1B
	por	xmm0, xmm5				; 0F8E _ 66: 0F EB. C5
	movdqa	xmm5, xmm3				; 0F92 _ 66: 0F 6F. EB
	pxor	xmm0, xmm3				; 0F96 _ 66: 0F EF. C3
	pxor	xmm0, xmm1				; 0F9A _ 66: 0F EF. C1
	pslld	xmm5, 7 				; 0F9E _ 66: 0F 72. F5, 07
	pxor	xmm6, xmm5				; 0FA3 _ 66: 0F EF. F5
	movdqa	xmm5, xmm6				; 0FA7 _ 66: 0F 6F. EE
	pslld	xmm2, 25				; 0FAB _ 66: 0F 72. F2, 19
	psrld	xmm1, 7 				; 0FB0 _ 66: 0F 72. D1, 07
	por	xmm2, xmm1				; 0FB5 _ 66: 0F EB. D1
	movdqa	xmm1, xmm3				; 0FB9 _ 66: 0F 6F. CB
	psrld	xmm3, 1 				; 0FBD _ 66: 0F 72. D3, 01
	pxor	xmm2, xmm6				; 0FC2 _ 66: 0F EF. D6
	pslld	xmm1, 31				; 0FC6 _ 66: 0F 72. F1, 1F
	por	xmm1, xmm3				; 0FCB _ 66: 0F EB. CB
	movdqa	xmm3, xmm0				; 0FCF _ 66: 0F 6F. D8
	pxor	xmm1, xmm0				; 0FD3 _ 66: 0F EF. C8
	pxor	xmm1, xmm6				; 0FD7 _ 66: 0F EF. CE
	pslld	xmm3, 3 				; 0FDB _ 66: 0F 72. F3, 03
	pxor	xmm2, xmm3				; 0FE0 _ 66: 0F EF. D3
	movdqa	xmm3, xmm0				; 0FE4 _ 66: 0F 6F. D8
	pslld	xmm5, 29				; 0FE8 _ 66: 0F 72. F5, 1D
	psrld	xmm6, 3 				; 0FED _ 66: 0F 72. D6, 03
	por	xmm5, xmm6				; 0FF2 _ 66: 0F EB. EE
	pslld	xmm3, 19				; 0FF6 _ 66: 0F 72. F3, 13
	movd	xmm6, dword [ecx+2BB0H] 		; 0FFB _ 66: 0F 6E. B1, 00002BB0
	movdqa	oword [esp+60H], xmm7			; 1003 _ 66: 0F 7F. 7C 24, 60
	movdqa	xmm7, xmm5				; 1009 _ 66: 0F 6F. FD
	psrld	xmm0, 13				; 100D _ 66: 0F 72. D0, 0D
	por	xmm3, xmm0				; 1012 _ 66: 0F EB. D8
	pxor	xmm7, xmm1				; 1016 _ 66: 0F EF. F9
	pxor	xmm3, xmm7				; 101A _ 66: 0F EF. DF
	pand	xmm5, xmm7				; 101E _ 66: 0F DB. EF
	pxor	xmm5, xmm3				; 1022 _ 66: 0F EF. EB
	pand	xmm3, xmm1				; 1026 _ 66: 0F DB. D9
	pxor	xmm1, xmm2				; 102A _ 66: 0F EF. CA
	por	xmm2, xmm5				; 102E _ 66: 0F EB. D5
	pxor	xmm7, xmm2				; 1032 _ 66: 0F EF. FA
	pxor	xmm3, xmm2				; 1036 _ 66: 0F EF. DA
	pxor	xmm1, xmm5				; 103A _ 66: 0F EF. CD
	pand	xmm2, xmm7				; 103E _ 66: 0F DB. D7
	pxor	xmm2, xmm1				; 1042 _ 66: 0F EF. D1
	pxor	xmm1, xmm3				; 1046 _ 66: 0F EF. CB
	por	xmm1, xmm7				; 104A _ 66: 0F EB. CF
	pxor	xmm3, xmm2				; 104E _ 66: 0F EF. DA
	pxor	xmm1, xmm5				; 1052 _ 66: 0F EF. CD
	pxor	xmm3, xmm1				; 1056 _ 66: 0F EF. D9
	pshufd	xmm0, xmm6, 0				; 105A _ 66: 0F 70. C6, 00
	movd	xmm6, dword [ecx+2BB4H] 		; 105F _ 66: 0F 6E. B1, 00002BB4
	pxor	xmm7, xmm0				; 1067 _ 66: 0F EF. F8
	pshufd	xmm5, xmm6, 0				; 106B _ 66: 0F 70. EE, 00
	pxor	xmm1, xmm5				; 1070 _ 66: 0F EF. CD
	movd	xmm0, dword [ecx+2BB8H] 		; 1074 _ 66: 0F 6E. 81, 00002BB8
	pshufd	xmm6, xmm0, 0				; 107C _ 66: 0F 70. F0, 00
	pxor	xmm2, xmm6				; 1081 _ 66: 0F EF. D6
	movdqa	xmm6, xmm2				; 1085 _ 66: 0F 6F. F2
	movd	xmm5, dword [ecx+2BBCH] 		; 1089 _ 66: 0F 6E. A9, 00002BBC
	pshufd	xmm0, xmm5, 0				; 1091 _ 66: 0F 70. C5, 00
	pxor	xmm3, xmm0				; 1096 _ 66: 0F EF. D8
	movdqa	xmm0, xmm7				; 109A _ 66: 0F 6F. C7
	movdqa	xmm5, xmm3				; 109E _ 66: 0F 6F. EB
	pslld	xmm6, 10				; 10A2 _ 66: 0F 72. F6, 0A
	psrld	xmm2, 22				; 10A7 _ 66: 0F 72. D2, 16
	por	xmm6, xmm2				; 10AC _ 66: 0F EB. F2
	movdqa	xmm2, xmm1				; 10B0 _ 66: 0F 6F. D1
	pslld	xmm0, 27				; 10B4 _ 66: 0F 72. F0, 1B
	psrld	xmm7, 5 				; 10B9 _ 66: 0F 72. D7, 05
	por	xmm0, xmm7				; 10BE _ 66: 0F EB. C7
	pxor	xmm6, xmm3				; 10C2 _ 66: 0F EF. F3
	pslld	xmm2, 7 				; 10C6 _ 66: 0F 72. F2, 07
	pxor	xmm6, xmm2				; 10CB _ 66: 0F EF. F2
	pxor	xmm0, xmm1				; 10CF _ 66: 0F EF. C1
	pxor	xmm0, xmm3				; 10D3 _ 66: 0F EF. C3
	movdqa	xmm2, xmm0				; 10D7 _ 66: 0F 6F. D0
	pslld	xmm5, 25				; 10DB _ 66: 0F 72. F5, 19
	psrld	xmm3, 7 				; 10E0 _ 66: 0F 72. D3, 07
	por	xmm5, xmm3				; 10E5 _ 66: 0F EB. EB
	movdqa	xmm3, xmm1				; 10E9 _ 66: 0F 6F. D9
	psrld	xmm1, 1 				; 10ED _ 66: 0F 72. D1, 01
	pxor	xmm5, xmm6				; 10F2 _ 66: 0F EF. EE
	pslld	xmm3, 31				; 10F6 _ 66: 0F 72. F3, 1F
	por	xmm3, xmm1				; 10FB _ 66: 0F EB. D9
	movdqa	xmm1, xmm0				; 10FF _ 66: 0F 6F. C8
	pxor	xmm3, xmm0				; 1103 _ 66: 0F EF. D8
	pxor	xmm3, xmm6				; 1107 _ 66: 0F EF. DE
	pslld	xmm1, 3 				; 110B _ 66: 0F 72. F1, 03
	pxor	xmm5, xmm1				; 1110 _ 66: 0F EF. E9
	movdqa	xmm1, xmm6				; 1114 _ 66: 0F 6F. CE
	psrld	xmm6, 3 				; 1118 _ 66: 0F 72. D6, 03
	pslld	xmm2, 19				; 111D _ 66: 0F 72. F2, 13
	pslld	xmm1, 29				; 1122 _ 66: 0F 72. F1, 1D
	por	xmm1, xmm6				; 1127 _ 66: 0F EB. CE
	psrld	xmm0, 13				; 112B _ 66: 0F 72. D0, 0D
	por	xmm2, xmm0				; 1130 _ 66: 0F EB. D0
	pxor	xmm1, xmm5				; 1134 _ 66: 0F EF. CD
	pxor	xmm5, xmm2				; 1138 _ 66: 0F EF. EA
	movdqa	xmm7, xmm5				; 113C _ 66: 0F 6F. FD
	pand	xmm7, xmm1				; 1140 _ 66: 0F DB. F9
	pxor	xmm7, xmm3				; 1144 _ 66: 0F EF. FB
	por	xmm3, xmm1				; 1148 _ 66: 0F EB. D9
	pxor	xmm3, xmm5				; 114C _ 66: 0F EF. DD
	pand	xmm5, xmm7				; 1150 _ 66: 0F DB. EF
	pxor	xmm1, xmm7				; 1154 _ 66: 0F EF. CF
	pand	xmm5, xmm2				; 1158 _ 66: 0F DB. EA
	pxor	xmm5, xmm1				; 115C _ 66: 0F EF. E9
	pand	xmm1, xmm3				; 1160 _ 66: 0F DB. CB
	por	xmm1, xmm2				; 1164 _ 66: 0F EB. CA
	pcmpeqd xmm6, xmm6				; 1168 _ 66: 0F 76. F6
	pxor	xmm7, xmm6				; 116C _ 66: 0F EF. FE
	movdqa	xmm0, xmm7				; 1170 _ 66: 0F 6F. C7
	pxor	xmm1, xmm7				; 1174 _ 66: 0F EF. CF
	pxor	xmm2, xmm7				; 1178 _ 66: 0F EF. D7
	pxor	xmm0, xmm5				; 117C _ 66: 0F EF. C5
	pand	xmm2, xmm3				; 1180 _ 66: 0F DB. D3
	pxor	xmm0, xmm2				; 1184 _ 66: 0F EF. C2
	movd	xmm2, dword [ecx+2BA0H] 		; 1188 _ 66: 0F 6E. 91, 00002BA0
	pshufd	xmm7, xmm2, 0				; 1190 _ 66: 0F 70. FA, 00
	pxor	xmm3, xmm7				; 1195 _ 66: 0F EF. DF
	movd	xmm2, dword [ecx+2BA4H] 		; 1199 _ 66: 0F 6E. 91, 00002BA4
	pshufd	xmm7, xmm2, 0				; 11A1 _ 66: 0F 70. FA, 00
	pxor	xmm5, xmm7				; 11A6 _ 66: 0F EF. EF
	movd	xmm2, dword [ecx+2BA8H] 		; 11AA _ 66: 0F 6E. 91, 00002BA8
	pshufd	xmm7, xmm2, 0				; 11B2 _ 66: 0F 70. FA, 00
	pxor	xmm1, xmm7				; 11B7 _ 66: 0F EF. CF
	movd	xmm2, dword [ecx+2BACH] 		; 11BB _ 66: 0F 6E. 91, 00002BAC
	pshufd	xmm7, xmm2, 0				; 11C3 _ 66: 0F 70. FA, 00
	movdqa	xmm2, xmm1				; 11C8 _ 66: 0F 6F. D1
	pxor	xmm0, xmm7				; 11CC _ 66: 0F EF. C7
	psrld	xmm1, 22				; 11D0 _ 66: 0F 72. D1, 16
	pslld	xmm2, 10				; 11D5 _ 66: 0F 72. F2, 0A
	por	xmm2, xmm1				; 11DA _ 66: 0F EB. D1
	movdqa	xmm1, xmm3				; 11DE _ 66: 0F 6F. CB
	psrld	xmm3, 5 				; 11E2 _ 66: 0F 72. D3, 05
	pxor	xmm2, xmm0				; 11E7 _ 66: 0F EF. D0
	pslld	xmm1, 27				; 11EB _ 66: 0F 72. F1, 1B
	por	xmm1, xmm3				; 11F0 _ 66: 0F EB. CB
	movdqa	xmm3, xmm5				; 11F4 _ 66: 0F 6F. DD
	pxor	xmm1, xmm5				; 11F8 _ 66: 0F EF. CD
	pxor	xmm1, xmm0				; 11FC _ 66: 0F EF. C8
	pslld	xmm3, 7 				; 1200 _ 66: 0F 72. F3, 07
	pxor	xmm2, xmm3				; 1205 _ 66: 0F EF. D3
	movdqa	xmm3, xmm0				; 1209 _ 66: 0F 6F. D8
	movdqa	xmm7, xmm2				; 120D _ 66: 0F 6F. FA
	psrld	xmm0, 7 				; 1211 _ 66: 0F 72. D0, 07
	pslld	xmm3, 25				; 1216 _ 66: 0F 72. F3, 19
	por	xmm3, xmm0				; 121B _ 66: 0F EB. D8
	movdqa	xmm0, xmm5				; 121F _ 66: 0F 6F. C5
	psrld	xmm5, 1 				; 1223 _ 66: 0F 72. D5, 01
	pxor	xmm3, xmm2				; 1228 _ 66: 0F EF. DA
	pslld	xmm0, 31				; 122C _ 66: 0F 72. F0, 1F
	por	xmm0, xmm5				; 1231 _ 66: 0F EB. C5
	movdqa	xmm5, xmm1				; 1235 _ 66: 0F 6F. E9
	pxor	xmm0, xmm1				; 1239 _ 66: 0F EF. C1
	pxor	xmm0, xmm2				; 123D _ 66: 0F EF. C2
	pslld	xmm5, 3 				; 1241 _ 66: 0F 72. F5, 03
	pxor	xmm3, xmm5				; 1246 _ 66: 0F EF. DD
	movdqa	xmm5, xmm1				; 124A _ 66: 0F 6F. E9
	pslld	xmm7, 29				; 124E _ 66: 0F 72. F7, 1D
	psrld	xmm2, 3 				; 1253 _ 66: 0F 72. D2, 03
	por	xmm7, xmm2				; 1258 _ 66: 0F EB. FA
	pslld	xmm5, 19				; 125C _ 66: 0F 72. F5, 13
	psrld	xmm1, 13				; 1261 _ 66: 0F 72. D1, 0D
	por	xmm5, xmm1				; 1266 _ 66: 0F EB. E9
	movdqa	xmm1, xmm0				; 126A _ 66: 0F 6F. C8
	pxor	xmm0, xmm7				; 126E _ 66: 0F EF. C7
	pxor	xmm1, xmm3				; 1272 _ 66: 0F EF. CB
	pand	xmm3, xmm1				; 1276 _ 66: 0F DB. D9
	pxor	xmm3, xmm5				; 127A _ 66: 0F EF. DD
	por	xmm5, xmm1				; 127E _ 66: 0F EB. E9
	pxor	xmm7, xmm3				; 1282 _ 66: 0F EF. FB
	pxor	xmm5, xmm0				; 1286 _ 66: 0F EF. E8
	por	xmm5, xmm7				; 128A _ 66: 0F EB. EF
	pxor	xmm1, xmm3				; 128E _ 66: 0F EF. CB
	pxor	xmm5, xmm1				; 1292 _ 66: 0F EF. E9
	por	xmm1, xmm3				; 1296 _ 66: 0F EB. CB
	pxor	xmm1, xmm5				; 129A _ 66: 0F EF. CD
	pxor	xmm0, xmm6				; 129E _ 66: 0F EF. C6
	pxor	xmm0, xmm1				; 12A2 _ 66: 0F EF. C1
	por	xmm1, xmm5				; 12A6 _ 66: 0F EB. CD
	pxor	xmm1, xmm5				; 12AA _ 66: 0F EF. CD
	por	xmm1, xmm0				; 12AE _ 66: 0F EB. C8
	pxor	xmm3, xmm1				; 12B2 _ 66: 0F EF. D9
	movd	xmm1, dword [ecx+2B90H] 		; 12B6 _ 66: 0F 6E. 89, 00002B90
	pshufd	xmm2, xmm1, 0				; 12BE _ 66: 0F 70. D1, 00
	pxor	xmm0, xmm2				; 12C3 _ 66: 0F EF. C2
	movd	xmm1, dword [ecx+2B94H] 		; 12C7 _ 66: 0F 6E. 89, 00002B94
	pshufd	xmm2, xmm1, 0				; 12CF _ 66: 0F 70. D1, 00
	pxor	xmm5, xmm2				; 12D4 _ 66: 0F EF. EA
	movd	xmm1, dword [ecx+2B98H] 		; 12D8 _ 66: 0F 6E. 89, 00002B98
	pshufd	xmm2, xmm1, 0				; 12E0 _ 66: 0F 70. D1, 00
	movd	xmm1, dword [ecx+2B9CH] 		; 12E5 _ 66: 0F 6E. 89, 00002B9C
	pxor	xmm3, xmm2				; 12ED _ 66: 0F EF. DA
	pshufd	xmm2, xmm1, 0				; 12F1 _ 66: 0F 70. D1, 00
	movdqa	xmm1, xmm3				; 12F6 _ 66: 0F 6F. CB
	pxor	xmm7, xmm2				; 12FA _ 66: 0F EF. FA
	movdqa	xmm2, xmm5				; 12FE _ 66: 0F 6F. D5
	pslld	xmm1, 10				; 1302 _ 66: 0F 72. F1, 0A
	psrld	xmm3, 22				; 1307 _ 66: 0F 72. D3, 16
	por	xmm1, xmm3				; 130C _ 66: 0F EB. CB
	movdqa	xmm3, xmm0				; 1310 _ 66: 0F 6F. D8
	psrld	xmm0, 5 				; 1314 _ 66: 0F 72. D0, 05
	pxor	xmm1, xmm7				; 1319 _ 66: 0F EF. CF
	pslld	xmm3, 27				; 131D _ 66: 0F 72. F3, 1B
	por	xmm3, xmm0				; 1322 _ 66: 0F EB. D8
	movdqa	xmm0, xmm5				; 1326 _ 66: 0F 6F. C5
	pxor	xmm3, xmm5				; 132A _ 66: 0F EF. DD
	pxor	xmm3, xmm7				; 132E _ 66: 0F EF. DF
	pslld	xmm0, 7 				; 1332 _ 66: 0F 72. F0, 07
	pxor	xmm1, xmm0				; 1337 _ 66: 0F EF. C8
	movdqa	xmm0, xmm7				; 133B _ 66: 0F 6F. C7
	psrld	xmm7, 7 				; 133F _ 66: 0F 72. D7, 07
	pslld	xmm2, 31				; 1344 _ 66: 0F 72. F2, 1F
	pslld	xmm0, 25				; 1349 _ 66: 0F 72. F0, 19
	por	xmm0, xmm7				; 134E _ 66: 0F EB. C7
	movdqa	xmm7, xmm3				; 1352 _ 66: 0F 6F. FB
	psrld	xmm5, 1 				; 1356 _ 66: 0F 72. D5, 01
	por	xmm2, xmm5				; 135B _ 66: 0F EB. D5
	movdqa	xmm5, xmm1				; 135F _ 66: 0F 6F. E9
	pxor	xmm0, xmm1				; 1363 _ 66: 0F EF. C1
	pslld	xmm7, 3 				; 1367 _ 66: 0F 72. F7, 03
	pxor	xmm0, xmm7				; 136C _ 66: 0F EF. C7
	movdqa	xmm7, xmm3				; 1370 _ 66: 0F 6F. FB
	pxor	xmm2, xmm3				; 1374 _ 66: 0F EF. D3
	pxor	xmm2, xmm1				; 1378 _ 66: 0F EF. D1
	pslld	xmm5, 29				; 137C _ 66: 0F 72. F5, 1D
	psrld	xmm1, 3 				; 1381 _ 66: 0F 72. D1, 03
	por	xmm5, xmm1				; 1386 _ 66: 0F EB. E9
	movdqa	xmm1, xmm2				; 138A _ 66: 0F 6F. CA
	pslld	xmm7, 19				; 138E _ 66: 0F 72. F7, 13
	psrld	xmm3, 13				; 1393 _ 66: 0F 72. D3, 0D
	por	xmm7, xmm3				; 1398 _ 66: 0F EB. FB
	pxor	xmm5, xmm6				; 139C _ 66: 0F EF. EE
	por	xmm1, xmm7				; 13A0 _ 66: 0F EB. CF
	movd	xmm3, dword [ecx+2B80H] 		; 13A4 _ 66: 0F 6E. 99, 00002B80
	pxor	xmm2, xmm6				; 13AC _ 66: 0F EF. D6
	pxor	xmm1, xmm5				; 13B0 _ 66: 0F EF. CD
	por	xmm5, xmm2				; 13B4 _ 66: 0F EB. EA
	pxor	xmm1, xmm0				; 13B8 _ 66: 0F EF. C8
	pxor	xmm7, xmm2				; 13BC _ 66: 0F EF. FA
	pxor	xmm5, xmm7				; 13C0 _ 66: 0F EF. EF
	pand	xmm7, xmm0				; 13C4 _ 66: 0F DB. F8
	pxor	xmm2, xmm7				; 13C8 _ 66: 0F EF. D7
	por	xmm7, xmm1				; 13CC _ 66: 0F EB. F9
	pxor	xmm7, xmm5				; 13D0 _ 66: 0F EF. FD
	pxor	xmm0, xmm2				; 13D4 _ 66: 0F EF. C2
	pxor	xmm5, xmm1				; 13D8 _ 66: 0F EF. E9
	pxor	xmm0, xmm7				; 13DC _ 66: 0F EF. C7
	pxor	xmm0, xmm1				; 13E0 _ 66: 0F EF. C1
	pand	xmm5, xmm0				; 13E4 _ 66: 0F DB. E8
	pxor	xmm2, xmm5				; 13E8 _ 66: 0F EF. D5
	pshufd	xmm3, xmm3, 0				; 13EC _ 66: 0F 70. DB, 00
	pxor	xmm7, xmm3				; 13F1 _ 66: 0F EF. FB
	movd	xmm5, dword [ecx+2B84H] 		; 13F5 _ 66: 0F 6E. A9, 00002B84
	pshufd	xmm3, xmm5, 0				; 13FD _ 66: 0F 70. DD, 00
	pxor	xmm2, xmm3				; 1402 _ 66: 0F EF. D3
	movd	xmm5, dword [ecx+2B88H] 		; 1406 _ 66: 0F 6E. A9, 00002B88
	pshufd	xmm3, xmm5, 0				; 140E _ 66: 0F 70. DD, 00
	pxor	xmm1, xmm3				; 1413 _ 66: 0F EF. CB
	movd	xmm5, dword [ecx+2B8CH] 		; 1417 _ 66: 0F 6E. A9, 00002B8C
	pshufd	xmm3, xmm5, 0				; 141F _ 66: 0F 70. DD, 00
	movdqa	xmm5, xmm7				; 1424 _ 66: 0F 6F. EF
	pxor	xmm0, xmm3				; 1428 _ 66: 0F EF. C3
	movdqa	xmm3, xmm1				; 142C _ 66: 0F 6F. D9
	psrld	xmm1, 22				; 1430 _ 66: 0F 72. D1, 16
	pslld	xmm5, 27				; 1435 _ 66: 0F 72. F5, 1B
	pslld	xmm3, 10				; 143A _ 66: 0F 72. F3, 0A
	por	xmm3, xmm1				; 143F _ 66: 0F EB. D9
	movdqa	xmm1, xmm2				; 1443 _ 66: 0F 6F. CA
	psrld	xmm7, 5 				; 1447 _ 66: 0F 72. D7, 05
	por	xmm5, xmm7				; 144C _ 66: 0F EB. EF
	movdqa	xmm7, xmm0				; 1450 _ 66: 0F 6F. F8
	pxor	xmm3, xmm0				; 1454 _ 66: 0F EF. D8
	pslld	xmm1, 7 				; 1458 _ 66: 0F 72. F1, 07
	pxor	xmm3, xmm1				; 145D _ 66: 0F EF. D9
	movdqa	xmm1, xmm2				; 1461 _ 66: 0F 6F. CA
	pxor	xmm5, xmm2				; 1465 _ 66: 0F EF. EA
	pxor	xmm5, xmm0				; 1469 _ 66: 0F EF. E8
	pslld	xmm7, 25				; 146D _ 66: 0F 72. F7, 19
	psrld	xmm0, 7 				; 1472 _ 66: 0F 72. D0, 07
	por	xmm7, xmm0				; 1477 _ 66: 0F EB. F8
	movdqa	xmm0, xmm3				; 147B _ 66: 0F 6F. C3
	pslld	xmm1, 31				; 147F _ 66: 0F 72. F1, 1F
	psrld	xmm2, 1 				; 1484 _ 66: 0F 72. D2, 01
	por	xmm1, xmm2				; 1489 _ 66: 0F EB. CA
	movdqa	xmm2, xmm5				; 148D _ 66: 0F 6F. D5
	pxor	xmm7, xmm3				; 1491 _ 66: 0F EF. FB
	pxor	xmm1, xmm5				; 1495 _ 66: 0F EF. CD
	pslld	xmm2, 3 				; 1499 _ 66: 0F 72. F2, 03
	pxor	xmm7, xmm2				; 149E _ 66: 0F EF. FA
	movdqa	xmm2, xmm5				; 14A2 _ 66: 0F 6F. D5
	pxor	xmm1, xmm3				; 14A6 _ 66: 0F EF. CB
	pslld	xmm0, 29				; 14AA _ 66: 0F 72. F0, 1D
	psrld	xmm3, 3 				; 14AF _ 66: 0F 72. D3, 03
	por	xmm0, xmm3				; 14B4 _ 66: 0F EB. C3
	pslld	xmm2, 19				; 14B8 _ 66: 0F 72. F2, 13
	psrld	xmm5, 13				; 14BD _ 66: 0F 72. D5, 0D
	por	xmm2, xmm5				; 14C2 _ 66: 0F EB. D5
	movdqa	xmm5, xmm0				; 14C6 _ 66: 0F 6F. E8
	por	xmm0, xmm7				; 14CA _ 66: 0F EB. C7
	pxor	xmm5, xmm2				; 14CE _ 66: 0F EF. EA
	pand	xmm2, xmm7				; 14D2 _ 66: 0F DB. D7
	pxor	xmm5, xmm6				; 14D6 _ 66: 0F EF. EE
	pxor	xmm7, xmm1				; 14DA _ 66: 0F EF. F9
	movd	xmm3, dword [ecx+2B70H] 		; 14DE _ 66: 0F 6E. 99, 00002B70
	por	xmm1, xmm2				; 14E6 _ 66: 0F EB. CA
	pxor	xmm2, xmm5				; 14EA _ 66: 0F EF. D5
	pand	xmm5, xmm0				; 14EE _ 66: 0F DB. E8
	pand	xmm7, xmm0				; 14F2 _ 66: 0F DB. F8
	pxor	xmm1, xmm5				; 14F6 _ 66: 0F EF. CD
	pxor	xmm5, xmm2				; 14FA _ 66: 0F EF. EA
	por	xmm2, xmm5				; 14FE _ 66: 0F EB. D5
	pxor	xmm0, xmm1				; 1502 _ 66: 0F EF. C1
	pxor	xmm2, xmm7				; 1506 _ 66: 0F EF. D7
	pxor	xmm7, xmm0				; 150A _ 66: 0F EF. F8
	por	xmm0, xmm2				; 150E _ 66: 0F EB. C2
	pxor	xmm7, xmm5				; 1512 _ 66: 0F EF. FD
	pxor	xmm0, xmm5				; 1516 _ 66: 0F EF. C5
	pshufd	xmm5, xmm3, 0				; 151A _ 66: 0F 70. EB, 00
	pxor	xmm7, xmm5				; 151F _ 66: 0F EF. FD
	movd	xmm3, dword [ecx+2B74H] 		; 1523 _ 66: 0F 6E. 99, 00002B74
	pshufd	xmm5, xmm3, 0				; 152B _ 66: 0F 70. EB, 00
	pxor	xmm2, xmm5				; 1530 _ 66: 0F EF. D5
	movd	xmm3, dword [ecx+2B78H] 		; 1534 _ 66: 0F 6E. 99, 00002B78
	pshufd	xmm5, xmm3, 0				; 153C _ 66: 0F 70. EB, 00
	movd	xmm3, dword [ecx+2B7CH] 		; 1541 _ 66: 0F 6E. 99, 00002B7C
	pxor	xmm1, xmm5				; 1549 _ 66: 0F EF. CD
	pshufd	xmm5, xmm3, 0				; 154D _ 66: 0F 70. EB, 00
	movdqa	xmm3, xmm7				; 1552 _ 66: 0F 6F. DF
	pxor	xmm0, xmm5				; 1556 _ 66: 0F EF. C5
	movdqa	xmm5, xmm1				; 155A _ 66: 0F 6F. E9
	psrld	xmm1, 22				; 155E _ 66: 0F 72. D1, 16
	pslld	xmm3, 27				; 1563 _ 66: 0F 72. F3, 1B
	pslld	xmm5, 10				; 1568 _ 66: 0F 72. F5, 0A
	por	xmm5, xmm1				; 156D _ 66: 0F EB. E9
	movdqa	xmm1, xmm0				; 1571 _ 66: 0F 6F. C8
	psrld	xmm7, 5 				; 1575 _ 66: 0F 72. D7, 05
	por	xmm3, xmm7				; 157A _ 66: 0F EB. DF
	movdqa	xmm7, xmm2				; 157E _ 66: 0F 6F. FA
	pxor	xmm5, xmm0				; 1582 _ 66: 0F EF. E8
	pxor	xmm3, xmm2				; 1586 _ 66: 0F EF. DA
	pslld	xmm7, 7 				; 158A _ 66: 0F 72. F7, 07
	pxor	xmm5, xmm7				; 158F _ 66: 0F EF. EF
	movdqa	xmm7, xmm2				; 1593 _ 66: 0F 6F. FA
	pxor	xmm3, xmm0				; 1597 _ 66: 0F EF. D8
	pslld	xmm1, 25				; 159B _ 66: 0F 72. F1, 19
	psrld	xmm0, 7 				; 15A0 _ 66: 0F 72. D0, 07
	por	xmm1, xmm0				; 15A5 _ 66: 0F EB. C8
	movdqa	xmm0, xmm3				; 15A9 _ 66: 0F 6F. C3
	pslld	xmm7, 31				; 15AD _ 66: 0F 72. F7, 1F
	psrld	xmm2, 1 				; 15B2 _ 66: 0F 72. D2, 01
	por	xmm7, xmm2				; 15B7 _ 66: 0F EB. FA
	movdqa	xmm2, xmm3				; 15BB _ 66: 0F 6F. D3
	pxor	xmm1, xmm5				; 15BF _ 66: 0F EF. CD
	pslld	xmm0, 3 				; 15C3 _ 66: 0F 72. F0, 03
	pxor	xmm1, xmm0				; 15C8 _ 66: 0F EF. C8
	movdqa	xmm0, xmm5				; 15CC _ 66: 0F 6F. C5
	pxor	xmm7, xmm3				; 15D0 _ 66: 0F EF. FB
	pxor	xmm7, xmm5				; 15D4 _ 66: 0F EF. FD
	pslld	xmm0, 29				; 15D8 _ 66: 0F 72. F0, 1D
	psrld	xmm5, 3 				; 15DD _ 66: 0F 72. D5, 03
	por	xmm0, xmm5				; 15E2 _ 66: 0F EB. C5
	pslld	xmm2, 19				; 15E6 _ 66: 0F 72. F2, 13
	psrld	xmm3, 13				; 15EB _ 66: 0F 72. D3, 0D
	por	xmm2, xmm3				; 15F0 _ 66: 0F EB. D3
	movdqa	xmm3, xmm0				; 15F4 _ 66: 0F 6F. D8
	pxor	xmm2, xmm0				; 15F8 _ 66: 0F EF. D0
	pxor	xmm0, xmm1				; 15FC _ 66: 0F EF. C1
	pand	xmm3, xmm2				; 1600 _ 66: 0F DB. DA
	pxor	xmm3, xmm6				; 1604 _ 66: 0F EF. DE
	pxor	xmm1, xmm7				; 1608 _ 66: 0F EF. CF
	pxor	xmm3, xmm1				; 160C _ 66: 0F EF. D9
	por	xmm0, xmm2				; 1610 _ 66: 0F EB. C2
	pxor	xmm2, xmm3				; 1614 _ 66: 0F EF. D3
	pxor	xmm1, xmm0				; 1618 _ 66: 0F EF. C8
	pxor	xmm0, xmm7				; 161C _ 66: 0F EF. C7
	pand	xmm7, xmm1				; 1620 _ 66: 0F DB. F9
	pxor	xmm7, xmm2				; 1624 _ 66: 0F EF. FA
	pxor	xmm2, xmm1				; 1628 _ 66: 0F EF. D1
	por	xmm2, xmm3				; 162C _ 66: 0F EB. D3
	pxor	xmm1, xmm7				; 1630 _ 66: 0F EF. CF
	pxor	xmm0, xmm2				; 1634 _ 66: 0F EF. C2
	movd	xmm2, dword [ecx+2B60H] 		; 1638 _ 66: 0F 6E. 91, 00002B60
	pshufd	xmm5, xmm2, 0				; 1640 _ 66: 0F 70. EA, 00
	pxor	xmm7, xmm5				; 1645 _ 66: 0F EF. FD
	movd	xmm2, dword [ecx+2B64H] 		; 1649 _ 66: 0F 6E. 91, 00002B64
	pshufd	xmm5, xmm2, 0				; 1651 _ 66: 0F 70. EA, 00
	pxor	xmm3, xmm5				; 1656 _ 66: 0F EF. DD
	movd	xmm2, dword [ecx+2B68H] 		; 165A _ 66: 0F 6E. 91, 00002B68
	pshufd	xmm5, xmm2, 0				; 1662 _ 66: 0F 70. EA, 00
	pxor	xmm0, xmm5				; 1667 _ 66: 0F EF. C5
	movd	xmm2, dword [ecx+2B6CH] 		; 166B _ 66: 0F 6E. 91, 00002B6C
	pshufd	xmm5, xmm2, 0				; 1673 _ 66: 0F 70. EA, 00
	movdqa	xmm2, xmm0				; 1678 _ 66: 0F 6F. D0
	pxor	xmm1, xmm5				; 167C _ 66: 0F EF. CD
	movdqa	xmm5, xmm7				; 1680 _ 66: 0F 6F. EF
	pslld	xmm2, 10				; 1684 _ 66: 0F 72. F2, 0A
	psrld	xmm0, 22				; 1689 _ 66: 0F 72. D0, 16
	por	xmm2, xmm0				; 168E _ 66: 0F EB. D0
	movdqa	xmm0, xmm1				; 1692 _ 66: 0F 6F. C1
	pslld	xmm5, 27				; 1696 _ 66: 0F 72. F5, 1B
	psrld	xmm7, 5 				; 169B _ 66: 0F 72. D7, 05
	por	xmm5, xmm7				; 16A0 _ 66: 0F EB. EF
	movdqa	xmm7, xmm3				; 16A4 _ 66: 0F 6F. FB
	pxor	xmm2, xmm1				; 16A8 _ 66: 0F EF. D1
	pxor	xmm5, xmm3				; 16AC _ 66: 0F EF. EB
	pslld	xmm7, 7 				; 16B0 _ 66: 0F 72. F7, 07
	pxor	xmm2, xmm7				; 16B5 _ 66: 0F EF. D7
	pxor	xmm5, xmm1				; 16B9 _ 66: 0F EF. E9
	movdqa	xmm7, xmm5				; 16BD _ 66: 0F 6F. FD
	pslld	xmm0, 25				; 16C1 _ 66: 0F 72. F0, 19
	psrld	xmm1, 7 				; 16C6 _ 66: 0F 72. D1, 07
	por	xmm0, xmm1				; 16CB _ 66: 0F EB. C1
	movdqa	xmm1, xmm3				; 16CF _ 66: 0F 6F. CB
	psrld	xmm3, 1 				; 16D3 _ 66: 0F 72. D3, 01
	pxor	xmm0, xmm2				; 16D8 _ 66: 0F EF. C2
	pslld	xmm1, 31				; 16DC _ 66: 0F 72. F1, 1F
	por	xmm1, xmm3				; 16E1 _ 66: 0F EB. CB
	movdqa	xmm3, xmm5				; 16E5 _ 66: 0F 6F. DD
	pxor	xmm1, xmm5				; 16E9 _ 66: 0F EF. CD
	pxor	xmm1, xmm2				; 16ED _ 66: 0F EF. CA
	pslld	xmm3, 3 				; 16F1 _ 66: 0F 72. F3, 03
	pxor	xmm0, xmm3				; 16F6 _ 66: 0F EF. C3
	movdqa	xmm3, xmm2				; 16FA _ 66: 0F 6F. DA
	psrld	xmm2, 3 				; 16FE _ 66: 0F 72. D2, 03
	pslld	xmm7, 19				; 1703 _ 66: 0F 72. F7, 13
	pslld	xmm3, 29				; 1708 _ 66: 0F 72. F3, 1D
	por	xmm3, xmm2				; 170D _ 66: 0F EB. DA
	movdqa	xmm2, xmm0				; 1711 _ 66: 0F 6F. D0
	psrld	xmm5, 13				; 1715 _ 66: 0F 72. D5, 0D
	por	xmm7, xmm5				; 171A _ 66: 0F EB. FD
	pxor	xmm1, xmm6				; 171E _ 66: 0F EF. CE
	pxor	xmm3, xmm1				; 1722 _ 66: 0F EF. D9
	por	xmm2, xmm7				; 1726 _ 66: 0F EB. D7
	pxor	xmm2, xmm3				; 172A _ 66: 0F EF. D3
	por	xmm3, xmm1				; 172E _ 66: 0F EB. D9
	pand	xmm3, xmm7				; 1732 _ 66: 0F DB. DF
	pxor	xmm0, xmm2				; 1736 _ 66: 0F EF. C2
	pxor	xmm3, xmm0				; 173A _ 66: 0F EF. D8
	por	xmm0, xmm7				; 173E _ 66: 0F EB. C7
	pxor	xmm0, xmm1				; 1742 _ 66: 0F EF. C1
	pand	xmm1, xmm3				; 1746 _ 66: 0F DB. CB
	pxor	xmm1, xmm2				; 174A _ 66: 0F EF. CA
	pxor	xmm0, xmm3				; 174E _ 66: 0F EF. C3
	pand	xmm2, xmm0				; 1752 _ 66: 0F DB. D0
	pxor	xmm0, xmm1				; 1756 _ 66: 0F EF. C1
	pxor	xmm2, xmm0				; 175A _ 66: 0F EF. D0
	pxor	xmm0, xmm6				; 175E _ 66: 0F EF. C6
	pxor	xmm2, xmm7				; 1762 _ 66: 0F EF. D7
	movd	xmm6, dword [ecx+2B50H] 		; 1766 _ 66: 0F 6E. B1, 00002B50
	pshufd	xmm5, xmm6, 0				; 176E _ 66: 0F 70. EE, 00
	movd	xmm6, dword [ecx+2B54H] 		; 1773 _ 66: 0F 6E. B1, 00002B54
	pxor	xmm1, xmm5				; 177B _ 66: 0F EF. CD
	pshufd	xmm7, xmm6, 0				; 177F _ 66: 0F 70. FE, 00
	pxor	xmm0, xmm7				; 1784 _ 66: 0F EF. C7
	movdqa	xmm7, oword [esp+60H]			; 1788 _ 66: 0F 6F. 7C 24, 60
	movd	xmm5, dword [ecx+2B58H] 		; 178E _ 66: 0F 6E. A9, 00002B58
	movdqa	oword [esp+60H], xmm7			; 1796 _ 66: 0F 7F. 7C 24, 60
	movdqa	xmm7, xmm1				; 179C _ 66: 0F 6F. F9
	pshufd	xmm6, xmm5, 0				; 17A0 _ 66: 0F 70. F5, 00
	pxor	xmm2, xmm6				; 17A5 _ 66: 0F EF. D6
	movd	xmm5, dword [ecx+2B5CH] 		; 17A9 _ 66: 0F 6E. A9, 00002B5C
	pshufd	xmm6, xmm5, 0				; 17B1 _ 66: 0F 70. F5, 00
	pxor	xmm3, xmm6				; 17B6 _ 66: 0F EF. DE
	movdqa	xmm6, xmm2				; 17BA _ 66: 0F 6F. F2
	psrld	xmm2, 22				; 17BE _ 66: 0F 72. D2, 16
	pslld	xmm7, 27				; 17C3 _ 66: 0F 72. F7, 1B
	pslld	xmm6, 10				; 17C8 _ 66: 0F 72. F6, 0A
	por	xmm6, xmm2				; 17CD _ 66: 0F EB. F2
	movdqa	xmm2, xmm0				; 17D1 _ 66: 0F 6F. D0
	psrld	xmm1, 5 				; 17D5 _ 66: 0F 72. D1, 05
	por	xmm7, xmm1				; 17DA _ 66: 0F EB. F9
	movdqa	xmm1, xmm0				; 17DE _ 66: 0F 6F. C8
	pxor	xmm6, xmm3				; 17E2 _ 66: 0F EF. F3
	pslld	xmm2, 7 				; 17E6 _ 66: 0F 72. F2, 07
	pxor	xmm6, xmm2				; 17EB _ 66: 0F EF. F2
	movdqa	xmm2, xmm3				; 17EF _ 66: 0F 6F. D3
	movdqa	xmm5, xmm6				; 17F3 _ 66: 0F 6F. EE
	pxor	xmm7, xmm0				; 17F7 _ 66: 0F EF. F8
	pxor	xmm7, xmm3				; 17FB _ 66: 0F EF. FB
	pslld	xmm2, 25				; 17FF _ 66: 0F 72. F2, 19
	psrld	xmm3, 7 				; 1804 _ 66: 0F 72. D3, 07
	por	xmm2, xmm3				; 1809 _ 66: 0F EB. D3
	movdqa	xmm3, xmm7				; 180D _ 66: 0F 6F. DF
	pslld	xmm1, 31				; 1811 _ 66: 0F 72. F1, 1F
	psrld	xmm0, 1 				; 1816 _ 66: 0F 72. D0, 01
	por	xmm1, xmm0				; 181B _ 66: 0F EB. C8
	pxor	xmm2, xmm6				; 181F _ 66: 0F EF. D6
	pslld	xmm3, 3 				; 1823 _ 66: 0F 72. F3, 03
	pxor	xmm2, xmm3				; 1828 _ 66: 0F EF. D3
	movdqa	xmm3, xmm7				; 182C _ 66: 0F 6F. DF
	pxor	xmm1, xmm7				; 1830 _ 66: 0F EF. CF
	pxor	xmm1, xmm6				; 1834 _ 66: 0F EF. CE
	pslld	xmm5, 29				; 1838 _ 66: 0F 72. F5, 1D
	psrld	xmm6, 3 				; 183D _ 66: 0F 72. D6, 03
	por	xmm5, xmm6				; 1842 _ 66: 0F EB. EE
	movdqa	xmm0, xmm5				; 1846 _ 66: 0F 6F. C5
	pslld	xmm3, 19				; 184A _ 66: 0F 72. F3, 13
	psrld	xmm7, 13				; 184F _ 66: 0F 72. D7, 0D
	por	xmm3, xmm7				; 1854 _ 66: 0F EB. DF
	pand	xmm0, xmm2				; 1858 _ 66: 0F DB. C2
	pxor	xmm0, xmm1				; 185C _ 66: 0F EF. C1
	por	xmm1, xmm2				; 1860 _ 66: 0F EB. CA
	pand	xmm1, xmm3				; 1864 _ 66: 0F DB. CB
	pxor	xmm5, xmm0				; 1868 _ 66: 0F EF. E8
	pxor	xmm5, xmm1				; 186C _ 66: 0F EF. E9
	pand	xmm1, xmm0				; 1870 _ 66: 0F DB. C8
	pxor	xmm2, xmm5				; 1874 _ 66: 0F EF. D5
	pxor	xmm1, xmm2				; 1878 _ 66: 0F EF. CA
	pcmpeqd xmm6, xmm6				; 187C _ 66: 0F 76. F6
	pxor	xmm3, xmm6				; 1880 _ 66: 0F EF. DE
	pand	xmm2, xmm3				; 1884 _ 66: 0F DB. D3
	pxor	xmm2, xmm0				; 1888 _ 66: 0F EF. D0
	pxor	xmm3, xmm1				; 188C _ 66: 0F EF. D9
	pand	xmm0, xmm3				; 1890 _ 66: 0F DB. C3
	pxor	xmm2, xmm3				; 1894 _ 66: 0F EF. D3
	pxor	xmm0, xmm5				; 1898 _ 66: 0F EF. C5
	por	xmm0, xmm2				; 189C _ 66: 0F EB. C2
	pxor	xmm2, xmm3				; 18A0 _ 66: 0F EF. D3
	pxor	xmm0, xmm1				; 18A4 _ 66: 0F EF. C1
	movd	xmm1, dword [ecx+2B40H] 		; 18A8 _ 66: 0F 6E. 89, 00002B40
	pshufd	xmm7, xmm1, 0				; 18B0 _ 66: 0F 70. F9, 00
	pxor	xmm3, xmm7				; 18B5 _ 66: 0F EF. DF
	movd	xmm1, dword [ecx+2B44H] 		; 18B9 _ 66: 0F 6E. 89, 00002B44
	pshufd	xmm7, xmm1, 0				; 18C1 _ 66: 0F 70. F9, 00
	pxor	xmm2, xmm7				; 18C6 _ 66: 0F EF. D7
	movd	xmm1, dword [ecx+2B48H] 		; 18CA _ 66: 0F 6E. 89, 00002B48
	pshufd	xmm7, xmm1, 0				; 18D2 _ 66: 0F 70. F9, 00
	pxor	xmm0, xmm7				; 18D7 _ 66: 0F EF. C7
	movd	xmm1, dword [ecx+2B4CH] 		; 18DB _ 66: 0F 6E. 89, 00002B4C
	pshufd	xmm7, xmm1, 0				; 18E3 _ 66: 0F 70. F9, 00
	movdqa	xmm1, xmm0				; 18E8 _ 66: 0F 6F. C8
	pxor	xmm5, xmm7				; 18EC _ 66: 0F EF. EF
	psrld	xmm0, 22				; 18F0 _ 66: 0F 72. D0, 16
	pslld	xmm1, 10				; 18F5 _ 66: 0F 72. F1, 0A
	por	xmm1, xmm0				; 18FA _ 66: 0F EB. C8
	movdqa	xmm0, xmm3				; 18FE _ 66: 0F 6F. C3
	psrld	xmm3, 5 				; 1902 _ 66: 0F 72. D3, 05
	pxor	xmm1, xmm5				; 1907 _ 66: 0F EF. CD
	pslld	xmm0, 27				; 190B _ 66: 0F 72. F0, 1B
	por	xmm0, xmm3				; 1910 _ 66: 0F EB. C3
	movdqa	xmm3, xmm2				; 1914 _ 66: 0F 6F. DA
	pxor	xmm0, xmm2				; 1918 _ 66: 0F EF. C2
	pxor	xmm0, xmm5				; 191C _ 66: 0F EF. C5
	movdqa	xmm7, xmm0				; 1920 _ 66: 0F 6F. F8
	pslld	xmm3, 7 				; 1924 _ 66: 0F 72. F3, 07
	pxor	xmm1, xmm3				; 1929 _ 66: 0F EF. CB
	movdqa	xmm3, xmm5				; 192D _ 66: 0F 6F. DD
	psrld	xmm5, 7 				; 1931 _ 66: 0F 72. D5, 07
	pslld	xmm7, 19				; 1936 _ 66: 0F 72. F7, 13
	pslld	xmm3, 25				; 193B _ 66: 0F 72. F3, 19
	por	xmm3, xmm5				; 1940 _ 66: 0F EB. DD
	movdqa	xmm5, xmm2				; 1944 _ 66: 0F 6F. EA
	psrld	xmm2, 1 				; 1948 _ 66: 0F 72. D2, 01
	pxor	xmm3, xmm1				; 194D _ 66: 0F EF. D9
	pslld	xmm5, 31				; 1951 _ 66: 0F 72. F5, 1F
	por	xmm5, xmm2				; 1956 _ 66: 0F EB. EA
	movdqa	xmm2, xmm0				; 195A _ 66: 0F 6F. D0
	pxor	xmm5, xmm0				; 195E _ 66: 0F EF. E8
	pxor	xmm5, xmm1				; 1962 _ 66: 0F EF. E9
	pslld	xmm2, 3 				; 1966 _ 66: 0F 72. F2, 03
	pxor	xmm3, xmm2				; 196B _ 66: 0F EF. DA
	movdqa	xmm2, xmm1				; 196F _ 66: 0F 6F. D1
	psrld	xmm1, 3 				; 1973 _ 66: 0F 72. D1, 03
	psrld	xmm0, 13				; 1978 _ 66: 0F 72. D0, 0D
	pslld	xmm2, 29				; 197D _ 66: 0F 72. F2, 1D
	por	xmm2, xmm1				; 1982 _ 66: 0F EB. D1
	por	xmm7, xmm0				; 1986 _ 66: 0F EB. F8
	movdqa	xmm0, xmm2				; 198A _ 66: 0F 6F. C2
	pxor	xmm0, xmm5				; 198E _ 66: 0F EF. C5
	pxor	xmm7, xmm0				; 1992 _ 66: 0F EF. F8
	pand	xmm2, xmm0				; 1996 _ 66: 0F DB. D0
	pxor	xmm2, xmm7				; 199A _ 66: 0F EF. D7
	pand	xmm7, xmm5				; 199E _ 66: 0F DB. FD
	pxor	xmm5, xmm3				; 19A2 _ 66: 0F EF. EB
	por	xmm3, xmm2				; 19A6 _ 66: 0F EB. DA
	pxor	xmm0, xmm3				; 19AA _ 66: 0F EF. C3
	pxor	xmm7, xmm3				; 19AE _ 66: 0F EF. FB
	pxor	xmm5, xmm2				; 19B2 _ 66: 0F EF. EA
	pand	xmm3, xmm0				; 19B6 _ 66: 0F DB. D8
	pxor	xmm3, xmm5				; 19BA _ 66: 0F EF. DD
	pxor	xmm5, xmm7				; 19BE _ 66: 0F EF. EF
	por	xmm5, xmm0				; 19C2 _ 66: 0F EB. E8
	pxor	xmm7, xmm3				; 19C6 _ 66: 0F EF. FB
	pxor	xmm5, xmm2				; 19CA _ 66: 0F EF. EA
	pxor	xmm7, xmm5				; 19CE _ 66: 0F EF. FD
	movd	xmm2, dword [ecx+2B30H] 		; 19D2 _ 66: 0F 6E. 91, 00002B30
	pshufd	xmm1, xmm2, 0				; 19DA _ 66: 0F 70. CA, 00
	pxor	xmm0, xmm1				; 19DF _ 66: 0F EF. C1
	movd	xmm2, dword [ecx+2B34H] 		; 19E3 _ 66: 0F 6E. 91, 00002B34
	pshufd	xmm1, xmm2, 0				; 19EB _ 66: 0F 70. CA, 00
	pxor	xmm5, xmm1				; 19F0 _ 66: 0F EF. E9
	movd	xmm2, dword [ecx+2B38H] 		; 19F4 _ 66: 0F 6E. 91, 00002B38
	pshufd	xmm1, xmm2, 0				; 19FC _ 66: 0F 70. CA, 00
	pxor	xmm3, xmm1				; 1A01 _ 66: 0F EF. D9
	movd	xmm2, dword [ecx+2B3CH] 		; 1A05 _ 66: 0F 6E. 91, 00002B3C
	pshufd	xmm1, xmm2, 0				; 1A0D _ 66: 0F 70. CA, 00
	movdqa	xmm2, xmm0				; 1A12 _ 66: 0F 6F. D0
	pxor	xmm7, xmm1				; 1A16 _ 66: 0F EF. F9
	movdqa	xmm1, xmm3				; 1A1A _ 66: 0F 6F. CB
	psrld	xmm3, 22				; 1A1E _ 66: 0F 72. D3, 16
	pslld	xmm2, 27				; 1A23 _ 66: 0F 72. F2, 1B
	pslld	xmm1, 10				; 1A28 _ 66: 0F 72. F1, 0A
	por	xmm1, xmm3				; 1A2D _ 66: 0F EB. CB
	movdqa	xmm3, xmm7				; 1A31 _ 66: 0F 6F. DF
	psrld	xmm0, 5 				; 1A35 _ 66: 0F 72. D0, 05
	por	xmm2, xmm0				; 1A3A _ 66: 0F EB. D0
	movdqa	xmm0, xmm5				; 1A3E _ 66: 0F 6F. C5
	pxor	xmm1, xmm7				; 1A42 _ 66: 0F EF. CF
	pxor	xmm2, xmm5				; 1A46 _ 66: 0F EF. D5
	pslld	xmm0, 7 				; 1A4A _ 66: 0F 72. F0, 07
	pxor	xmm1, xmm0				; 1A4F _ 66: 0F EF. C8
	movdqa	xmm0, xmm5				; 1A53 _ 66: 0F 6F. C5
	pxor	xmm2, xmm7				; 1A57 _ 66: 0F EF. D7
	pslld	xmm3, 25				; 1A5B _ 66: 0F 72. F3, 19
	psrld	xmm7, 7 				; 1A60 _ 66: 0F 72. D7, 07
	por	xmm3, xmm7				; 1A65 _ 66: 0F EB. DF
	movdqa	xmm7, xmm1				; 1A69 _ 66: 0F 6F. F9
	pslld	xmm0, 31				; 1A6D _ 66: 0F 72. F0, 1F
	psrld	xmm5, 1 				; 1A72 _ 66: 0F 72. D5, 01
	por	xmm0, xmm5				; 1A77 _ 66: 0F EB. C5
	movdqa	xmm5, xmm2				; 1A7B _ 66: 0F 6F. EA
	pxor	xmm3, xmm1				; 1A7F _ 66: 0F EF. D9
	pxor	xmm0, xmm2				; 1A83 _ 66: 0F EF. C2
	pslld	xmm5, 3 				; 1A87 _ 66: 0F 72. F5, 03
	pxor	xmm3, xmm5				; 1A8C _ 66: 0F EF. DD
	movdqa	xmm5, xmm2				; 1A90 _ 66: 0F 6F. EA
	pxor	xmm0, xmm1				; 1A94 _ 66: 0F EF. C1
	pslld	xmm7, 29				; 1A98 _ 66: 0F 72. F7, 1D
	psrld	xmm1, 3 				; 1A9D _ 66: 0F 72. D1, 03
	por	xmm7, xmm1				; 1AA2 _ 66: 0F EB. F9
	pslld	xmm5, 19				; 1AA6 _ 66: 0F 72. F5, 13
	psrld	xmm2, 13				; 1AAB _ 66: 0F 72. D2, 0D
	por	xmm5, xmm2				; 1AB0 _ 66: 0F EB. EA
	pxor	xmm7, xmm3				; 1AB4 _ 66: 0F EF. FB
	pxor	xmm3, xmm5				; 1AB8 _ 66: 0F EF. DD
	movdqa	xmm2, xmm3				; 1ABC _ 66: 0F 6F. D3
	pand	xmm2, xmm7				; 1AC0 _ 66: 0F DB. D7
	pxor	xmm2, xmm0				; 1AC4 _ 66: 0F EF. D0
	por	xmm0, xmm7				; 1AC8 _ 66: 0F EB. C7
	pxor	xmm0, xmm3				; 1ACC _ 66: 0F EF. C3
	pand	xmm3, xmm2				; 1AD0 _ 66: 0F DB. DA
	pxor	xmm7, xmm2				; 1AD4 _ 66: 0F EF. FA
	pand	xmm3, xmm5				; 1AD8 _ 66: 0F DB. DD
	pxor	xmm3, xmm7				; 1ADC _ 66: 0F EF. DF
	pand	xmm7, xmm0				; 1AE0 _ 66: 0F DB. F8
	por	xmm7, xmm5				; 1AE4 _ 66: 0F EB. FD
	pxor	xmm2, xmm6				; 1AE8 _ 66: 0F EF. D6
	movdqa	xmm1, xmm2				; 1AEC _ 66: 0F 6F. CA
	pxor	xmm7, xmm2				; 1AF0 _ 66: 0F EF. FA
	pxor	xmm5, xmm2				; 1AF4 _ 66: 0F EF. EA
	pxor	xmm1, xmm3				; 1AF8 _ 66: 0F EF. CB
	pand	xmm5, xmm0				; 1AFC _ 66: 0F DB. E8
	pxor	xmm1, xmm5				; 1B00 _ 66: 0F EF. CD
	movd	xmm5, dword [ecx+2B20H] 		; 1B04 _ 66: 0F 6E. A9, 00002B20
	pshufd	xmm5, xmm5, 0				; 1B0C _ 66: 0F 70. ED, 00
	pxor	xmm0, xmm5				; 1B11 _ 66: 0F EF. C5
	movd	xmm2, dword [ecx+2B24H] 		; 1B15 _ 66: 0F 6E. 91, 00002B24
	pshufd	xmm5, xmm2, 0				; 1B1D _ 66: 0F 70. EA, 00
	pxor	xmm3, xmm5				; 1B22 _ 66: 0F EF. DD
	movd	xmm2, dword [ecx+2B28H] 		; 1B26 _ 66: 0F 6E. 91, 00002B28
	pshufd	xmm5, xmm2, 0				; 1B2E _ 66: 0F 70. EA, 00
	pxor	xmm7, xmm5				; 1B33 _ 66: 0F EF. FD
	movd	xmm2, dword [ecx+2B2CH] 		; 1B37 _ 66: 0F 6E. 91, 00002B2C
	pshufd	xmm5, xmm2, 0				; 1B3F _ 66: 0F 70. EA, 00
	movdqa	xmm2, xmm7				; 1B44 _ 66: 0F 6F. D7
	pxor	xmm1, xmm5				; 1B48 _ 66: 0F EF. CD
	movdqa	xmm5, xmm0				; 1B4C _ 66: 0F 6F. E8
	pslld	xmm2, 10				; 1B50 _ 66: 0F 72. F2, 0A
	psrld	xmm7, 22				; 1B55 _ 66: 0F 72. D7, 16
	por	xmm2, xmm7				; 1B5A _ 66: 0F EB. D7
	movdqa	xmm7, xmm3				; 1B5E _ 66: 0F 6F. FB
	pslld	xmm5, 27				; 1B62 _ 66: 0F 72. F5, 1B
	psrld	xmm0, 5 				; 1B67 _ 66: 0F 72. D0, 05
	por	xmm5, xmm0				; 1B6C _ 66: 0F EB. E8
	movdqa	xmm0, xmm1				; 1B70 _ 66: 0F 6F. C1
	pxor	xmm2, xmm1				; 1B74 _ 66: 0F EF. D1
	pslld	xmm7, 7 				; 1B78 _ 66: 0F 72. F7, 07
	pxor	xmm2, xmm7				; 1B7D _ 66: 0F EF. D7
	pxor	xmm5, xmm3				; 1B81 _ 66: 0F EF. EB
	pxor	xmm5, xmm1				; 1B85 _ 66: 0F EF. E9
	movdqa	xmm7, xmm5				; 1B89 _ 66: 0F 6F. FD
	pslld	xmm0, 25				; 1B8D _ 66: 0F 72. F0, 19
	psrld	xmm1, 7 				; 1B92 _ 66: 0F 72. D1, 07
	por	xmm0, xmm1				; 1B97 _ 66: 0F EB. C1
	movdqa	xmm1, xmm3				; 1B9B _ 66: 0F 6F. CB
	psrld	xmm3, 1 				; 1B9F _ 66: 0F 72. D3, 01
	pxor	xmm0, xmm2				; 1BA4 _ 66: 0F EF. C2
	pslld	xmm1, 31				; 1BA8 _ 66: 0F 72. F1, 1F
	por	xmm1, xmm3				; 1BAD _ 66: 0F EB. CB
	movdqa	xmm3, xmm5				; 1BB1 _ 66: 0F 6F. DD
	pxor	xmm1, xmm5				; 1BB5 _ 66: 0F EF. CD
	pxor	xmm1, xmm2				; 1BB9 _ 66: 0F EF. CA
	pslld	xmm3, 3 				; 1BBD _ 66: 0F 72. F3, 03
	pxor	xmm0, xmm3				; 1BC2 _ 66: 0F EF. C3
	movdqa	xmm3, xmm2				; 1BC6 _ 66: 0F 6F. DA
	psrld	xmm2, 3 				; 1BCA _ 66: 0F 72. D2, 03
	pslld	xmm7, 19				; 1BCF _ 66: 0F 72. F7, 13
	pslld	xmm3, 29				; 1BD4 _ 66: 0F 72. F3, 1D
	por	xmm3, xmm2				; 1BD9 _ 66: 0F EB. DA
	movdqa	xmm2, xmm1				; 1BDD _ 66: 0F 6F. D1
	psrld	xmm5, 13				; 1BE1 _ 66: 0F 72. D5, 0D
	por	xmm7, xmm5				; 1BE6 _ 66: 0F EB. FD
	pxor	xmm2, xmm0				; 1BEA _ 66: 0F EF. D0
	pand	xmm0, xmm2				; 1BEE _ 66: 0F DB. C2
	pxor	xmm1, xmm3				; 1BF2 _ 66: 0F EF. CB
	pxor	xmm0, xmm7				; 1BF6 _ 66: 0F EF. C7
	por	xmm7, xmm2				; 1BFA _ 66: 0F EB. FA
	movd	xmm5, dword [ecx+2B10H] 		; 1BFE _ 66: 0F 6E. A9, 00002B10
	pxor	xmm3, xmm0				; 1C06 _ 66: 0F EF. D8
	pxor	xmm7, xmm1				; 1C0A _ 66: 0F EF. F9
	por	xmm7, xmm3				; 1C0E _ 66: 0F EB. FB
	pxor	xmm2, xmm0				; 1C12 _ 66: 0F EF. D0
	pxor	xmm7, xmm2				; 1C16 _ 66: 0F EF. FA
	por	xmm2, xmm0				; 1C1A _ 66: 0F EB. D0
	pxor	xmm2, xmm7				; 1C1E _ 66: 0F EF. D7
	pxor	xmm1, xmm6				; 1C22 _ 66: 0F EF. CE
	pxor	xmm1, xmm2				; 1C26 _ 66: 0F EF. CA
	por	xmm2, xmm7				; 1C2A _ 66: 0F EB. D7
	pxor	xmm2, xmm7				; 1C2E _ 66: 0F EF. D7
	por	xmm2, xmm1				; 1C32 _ 66: 0F EB. D1
	pxor	xmm0, xmm2				; 1C36 _ 66: 0F EF. C2
	pshufd	xmm2, xmm5, 0				; 1C3A _ 66: 0F 70. D5, 00
	pxor	xmm1, xmm2				; 1C3F _ 66: 0F EF. CA
	movd	xmm5, dword [ecx+2B14H] 		; 1C43 _ 66: 0F 6E. A9, 00002B14
	pshufd	xmm2, xmm5, 0				; 1C4B _ 66: 0F 70. D5, 00
	pxor	xmm7, xmm2				; 1C50 _ 66: 0F EF. FA
	movd	xmm5, dword [ecx+2B18H] 		; 1C54 _ 66: 0F 6E. A9, 00002B18
	pshufd	xmm2, xmm5, 0				; 1C5C _ 66: 0F 70. D5, 00
	movd	xmm5, dword [ecx+2B1CH] 		; 1C61 _ 66: 0F 6E. A9, 00002B1C
	pxor	xmm0, xmm2				; 1C69 _ 66: 0F EF. C2
	pshufd	xmm2, xmm5, 0				; 1C6D _ 66: 0F 70. D5, 00
	movdqa	xmm5, xmm0				; 1C72 _ 66: 0F 6F. E8
	pxor	xmm3, xmm2				; 1C76 _ 66: 0F EF. DA
	movdqa	xmm2, xmm1				; 1C7A _ 66: 0F 6F. D1
	pslld	xmm5, 10				; 1C7E _ 66: 0F 72. F5, 0A
	psrld	xmm0, 22				; 1C83 _ 66: 0F 72. D0, 16
	por	xmm5, xmm0				; 1C88 _ 66: 0F EB. E8
	movdqa	xmm0, xmm3				; 1C8C _ 66: 0F 6F. C3
	pslld	xmm2, 27				; 1C90 _ 66: 0F 72. F2, 1B
	psrld	xmm1, 5 				; 1C95 _ 66: 0F 72. D1, 05
	por	xmm2, xmm1				; 1C9A _ 66: 0F EB. D1
	movdqa	xmm1, xmm7				; 1C9E _ 66: 0F 6F. CF
	pxor	xmm5, xmm3				; 1CA2 _ 66: 0F EF. EB
	pxor	xmm2, xmm7				; 1CA6 _ 66: 0F EF. D7
	pslld	xmm1, 7 				; 1CAA _ 66: 0F 72. F1, 07
	pxor	xmm5, xmm1				; 1CAF _ 66: 0F EF. E9
	pxor	xmm2, xmm3				; 1CB3 _ 66: 0F EF. D3
	movdqa	xmm1, xmm2				; 1CB7 _ 66: 0F 6F. CA
	pslld	xmm0, 25				; 1CBB _ 66: 0F 72. F0, 19
	psrld	xmm3, 7 				; 1CC0 _ 66: 0F 72. D3, 07
	por	xmm0, xmm3				; 1CC5 _ 66: 0F EB. C3
	movdqa	xmm3, xmm7				; 1CC9 _ 66: 0F 6F. DF
	psrld	xmm7, 1 				; 1CCD _ 66: 0F 72. D7, 01
	pxor	xmm0, xmm5				; 1CD2 _ 66: 0F EF. C5
	pslld	xmm3, 31				; 1CD6 _ 66: 0F 72. F3, 1F
	por	xmm3, xmm7				; 1CDB _ 66: 0F EB. DF
	movdqa	xmm7, xmm2				; 1CDF _ 66: 0F 6F. FA
	pxor	xmm3, xmm2				; 1CE3 _ 66: 0F EF. DA
	pxor	xmm3, xmm5				; 1CE7 _ 66: 0F EF. DD
	pslld	xmm7, 3 				; 1CEB _ 66: 0F 72. F7, 03
	pxor	xmm0, xmm7				; 1CF0 _ 66: 0F EF. C7
	movdqa	xmm7, xmm5				; 1CF4 _ 66: 0F 6F. FD
	psrld	xmm5, 3 				; 1CF8 _ 66: 0F 72. D5, 03
	pslld	xmm1, 19				; 1CFD _ 66: 0F 72. F1, 13
	pslld	xmm7, 29				; 1D02 _ 66: 0F 72. F7, 1D
	por	xmm7, xmm5				; 1D07 _ 66: 0F EB. FD
	movdqa	xmm5, xmm3				; 1D0B _ 66: 0F 6F. EB
	psrld	xmm2, 13				; 1D0F _ 66: 0F 72. D2, 0D
	por	xmm1, xmm2				; 1D14 _ 66: 0F EB. CA
	pxor	xmm7, xmm6				; 1D18 _ 66: 0F EF. FE
	por	xmm5, xmm1				; 1D1C _ 66: 0F EB. E9
	movd	xmm2, dword [ecx+2B00H] 		; 1D20 _ 66: 0F 6E. 91, 00002B00
	pxor	xmm3, xmm6				; 1D28 _ 66: 0F EF. DE
	pxor	xmm5, xmm7				; 1D2C _ 66: 0F EF. EF
	por	xmm7, xmm3				; 1D30 _ 66: 0F EB. FB
	pxor	xmm5, xmm0				; 1D34 _ 66: 0F EF. E8
	pxor	xmm1, xmm3				; 1D38 _ 66: 0F EF. CB
	pxor	xmm7, xmm1				; 1D3C _ 66: 0F EF. F9
	pand	xmm1, xmm0				; 1D40 _ 66: 0F DB. C8
	pxor	xmm3, xmm1				; 1D44 _ 66: 0F EF. D9
	por	xmm1, xmm5				; 1D48 _ 66: 0F EB. CD
	pxor	xmm1, xmm7				; 1D4C _ 66: 0F EF. CF
	pxor	xmm0, xmm3				; 1D50 _ 66: 0F EF. C3
	pxor	xmm7, xmm5				; 1D54 _ 66: 0F EF. FD
	pxor	xmm0, xmm1				; 1D58 _ 66: 0F EF. C1
	pxor	xmm0, xmm5				; 1D5C _ 66: 0F EF. C5
	pand	xmm7, xmm0				; 1D60 _ 66: 0F DB. F8
	pxor	xmm3, xmm7				; 1D64 _ 66: 0F EF. DF
	pshufd	xmm7, xmm2, 0				; 1D68 _ 66: 0F 70. FA, 00
	pxor	xmm1, xmm7				; 1D6D _ 66: 0F EF. CF
	movd	xmm2, dword [ecx+2B04H] 		; 1D71 _ 66: 0F 6E. 91, 00002B04
	pshufd	xmm7, xmm2, 0				; 1D79 _ 66: 0F 70. FA, 00
	pxor	xmm3, xmm7				; 1D7E _ 66: 0F EF. DF
	movd	xmm2, dword [ecx+2B08H] 		; 1D82 _ 66: 0F 6E. 91, 00002B08
	pshufd	xmm7, xmm2, 0				; 1D8A _ 66: 0F 70. FA, 00
	pxor	xmm5, xmm7				; 1D8F _ 66: 0F EF. EF
	movd	xmm2, dword [ecx+2B0CH] 		; 1D93 _ 66: 0F 6E. 91, 00002B0C
	pshufd	xmm7, xmm2, 0				; 1D9B _ 66: 0F 70. FA, 00
	movdqa	xmm2, xmm5				; 1DA0 _ 66: 0F 6F. D5
	pxor	xmm0, xmm7				; 1DA4 _ 66: 0F EF. C7
	movdqa	xmm7, xmm0				; 1DA8 _ 66: 0F 6F. F8
	pslld	xmm2, 10				; 1DAC _ 66: 0F 72. F2, 0A
	psrld	xmm5, 22				; 1DB1 _ 66: 0F 72. D5, 16
	por	xmm2, xmm5				; 1DB6 _ 66: 0F EB. D5
	movdqa	xmm5, xmm1				; 1DBA _ 66: 0F 6F. E9
	psrld	xmm1, 5 				; 1DBE _ 66: 0F 72. D1, 05
	pxor	xmm2, xmm0				; 1DC3 _ 66: 0F EF. D0
	pslld	xmm5, 27				; 1DC7 _ 66: 0F 72. F5, 1B
	por	xmm5, xmm1				; 1DCC _ 66: 0F EB. E9
	movdqa	xmm1, xmm3				; 1DD0 _ 66: 0F 6F. CB
	pxor	xmm5, xmm3				; 1DD4 _ 66: 0F EF. EB
	pxor	xmm5, xmm0				; 1DD8 _ 66: 0F EF. E8
	pslld	xmm1, 7 				; 1DDC _ 66: 0F 72. F1, 07
	pxor	xmm2, xmm1				; 1DE1 _ 66: 0F EF. D1
	movdqa	xmm1, xmm3				; 1DE5 _ 66: 0F 6F. CB
	pslld	xmm7, 25				; 1DE9 _ 66: 0F 72. F7, 19
	psrld	xmm0, 7 				; 1DEE _ 66: 0F 72. D0, 07
	por	xmm7, xmm0				; 1DF3 _ 66: 0F EB. F8
	movdqa	xmm0, xmm5				; 1DF7 _ 66: 0F 6F. C5
	pslld	xmm1, 31				; 1DFB _ 66: 0F 72. F1, 1F
	psrld	xmm3, 1 				; 1E00 _ 66: 0F 72. D3, 01
	por	xmm1, xmm3				; 1E05 _ 66: 0F EB. CB
	movdqa	xmm3, xmm5				; 1E09 _ 66: 0F 6F. DD
	pxor	xmm7, xmm2				; 1E0D _ 66: 0F EF. FA
	pslld	xmm0, 3 				; 1E11 _ 66: 0F 72. F0, 03
	pxor	xmm7, xmm0				; 1E16 _ 66: 0F EF. F8
	movdqa	xmm0, xmm2				; 1E1A _ 66: 0F 6F. C2
	pxor	xmm1, xmm5				; 1E1E _ 66: 0F EF. CD
	pxor	xmm1, xmm2				; 1E22 _ 66: 0F EF. CA
	pslld	xmm0, 29				; 1E26 _ 66: 0F 72. F0, 1D
	psrld	xmm2, 3 				; 1E2B _ 66: 0F 72. D2, 03
	por	xmm0, xmm2				; 1E30 _ 66: 0F EB. C2
	pslld	xmm3, 19				; 1E34 _ 66: 0F 72. F3, 13
	psrld	xmm5, 13				; 1E39 _ 66: 0F 72. D5, 0D
	por	xmm3, xmm5				; 1E3E _ 66: 0F EB. DD
	movdqa	xmm5, xmm0				; 1E42 _ 66: 0F 6F. E8
	por	xmm0, xmm7				; 1E46 _ 66: 0F EB. C7
	pxor	xmm5, xmm3				; 1E4A _ 66: 0F EF. EB
	pand	xmm3, xmm7				; 1E4E _ 66: 0F DB. DF
	pxor	xmm5, xmm6				; 1E52 _ 66: 0F EF. EE
	pxor	xmm7, xmm1				; 1E56 _ 66: 0F EF. F9
	movd	xmm6, dword [ecx+2AF0H] 		; 1E5A _ 66: 0F 6E. B1, 00002AF0
	por	xmm1, xmm3				; 1E62 _ 66: 0F EB. CB
	pxor	xmm3, xmm5				; 1E66 _ 66: 0F EF. DD
	pand	xmm5, xmm0				; 1E6A _ 66: 0F DB. E8
	pand	xmm7, xmm0				; 1E6E _ 66: 0F DB. F8
	pxor	xmm1, xmm5				; 1E72 _ 66: 0F EF. CD
	pxor	xmm5, xmm3				; 1E76 _ 66: 0F EF. EB
	por	xmm3, xmm5				; 1E7A _ 66: 0F EB. DD
	pxor	xmm0, xmm1				; 1E7E _ 66: 0F EF. C1
	pxor	xmm3, xmm7				; 1E82 _ 66: 0F EF. DF
	pxor	xmm7, xmm0				; 1E86 _ 66: 0F EF. F8
	por	xmm0, xmm3				; 1E8A _ 66: 0F EB. C3
	pxor	xmm7, xmm5				; 1E8E _ 66: 0F EF. FD
	pxor	xmm0, xmm5				; 1E92 _ 66: 0F EF. C5
	pshufd	xmm2, xmm6, 0				; 1E96 _ 66: 0F 70. D6, 00
	pxor	xmm7, xmm2				; 1E9B _ 66: 0F EF. FA
	movd	xmm5, dword [ecx+2AF4H] 		; 1E9F _ 66: 0F 6E. A9, 00002AF4
	pshufd	xmm6, xmm5, 0				; 1EA7 _ 66: 0F 70. F5, 00
	pxor	xmm3, xmm6				; 1EAC _ 66: 0F EF. DE
	movd	xmm2, dword [ecx+2AF8H] 		; 1EB0 _ 66: 0F 6E. 91, 00002AF8
	pshufd	xmm5, xmm2, 0				; 1EB8 _ 66: 0F 70. EA, 00
	movd	xmm6, dword [ecx+2AFCH] 		; 1EBD _ 66: 0F 6E. B1, 00002AFC
	pxor	xmm1, xmm5				; 1EC5 _ 66: 0F EF. CD
	movdqa	xmm5, xmm7				; 1EC9 _ 66: 0F 6F. EF
	pshufd	xmm2, xmm6, 0				; 1ECD _ 66: 0F 70. D6, 00
	movdqa	xmm6, xmm3				; 1ED2 _ 66: 0F 6F. F3
	pxor	xmm0, xmm2				; 1ED6 _ 66: 0F EF. C2
	movdqa	xmm2, xmm1				; 1EDA _ 66: 0F 6F. D1
	psrld	xmm1, 22				; 1EDE _ 66: 0F 72. D1, 16
	pslld	xmm5, 27				; 1EE3 _ 66: 0F 72. F5, 1B
	pslld	xmm2, 10				; 1EE8 _ 66: 0F 72. F2, 0A
	por	xmm2, xmm1				; 1EED _ 66: 0F EB. D1
	movdqa	xmm1, xmm3				; 1EF1 _ 66: 0F 6F. CB
	psrld	xmm7, 5 				; 1EF5 _ 66: 0F 72. D7, 05
	por	xmm5, xmm7				; 1EFA _ 66: 0F EB. EF
	movdqa	xmm7, oword [esp+60H]			; 1EFE _ 66: 0F 6F. 7C 24, 60
	pxor	xmm2, xmm0				; 1F04 _ 66: 0F EF. D0
	pslld	xmm1, 7 				; 1F08 _ 66: 0F 72. F1, 07
	pxor	xmm2, xmm1				; 1F0D _ 66: 0F EF. D1
	movdqa	xmm1, xmm0				; 1F11 _ 66: 0F 6F. C8
	pxor	xmm5, xmm3				; 1F15 _ 66: 0F EF. EB
	pxor	xmm5, xmm0				; 1F19 _ 66: 0F EF. E8
	pslld	xmm1, 25				; 1F1D _ 66: 0F 72. F1, 19
	psrld	xmm0, 7 				; 1F22 _ 66: 0F 72. D0, 07
	por	xmm1, xmm0				; 1F27 _ 66: 0F EB. C8
	pslld	xmm6, 31				; 1F2B _ 66: 0F 72. F6, 1F
	psrld	xmm3, 1 				; 1F30 _ 66: 0F 72. D3, 01
	por	xmm6, xmm3				; 1F35 _ 66: 0F EB. F3
	movdqa	xmm3, xmm5				; 1F39 _ 66: 0F 6F. DD
	pxor	xmm1, xmm2				; 1F3D _ 66: 0F EF. CA
	pxor	xmm6, xmm5				; 1F41 _ 66: 0F EF. F5
	pslld	xmm3, 3 				; 1F45 _ 66: 0F 72. F3, 03
	pxor	xmm1, xmm3				; 1F4A _ 66: 0F EF. CB
	movdqa	oword [esp+60H], xmm7			; 1F4E _ 66: 0F 7F. 7C 24, 60
	movdqa	xmm0, xmm2				; 1F54 _ 66: 0F 6F. C2
	movdqa	xmm7, xmm5				; 1F58 _ 66: 0F 6F. FD
	pxor	xmm6, xmm2				; 1F5C _ 66: 0F EF. F2
	pslld	xmm0, 29				; 1F60 _ 66: 0F 72. F0, 1D
	psrld	xmm2, 3 				; 1F65 _ 66: 0F 72. D2, 03
	por	xmm0, xmm2				; 1F6A _ 66: 0F EB. C2
	pslld	xmm7, 19				; 1F6E _ 66: 0F 72. F7, 13
	psrld	xmm5, 13				; 1F73 _ 66: 0F 72. D5, 0D
	por	xmm7, xmm5				; 1F78 _ 66: 0F EB. FD
	movdqa	xmm5, xmm0				; 1F7C _ 66: 0F 6F. E8
	pxor	xmm7, xmm0				; 1F80 _ 66: 0F EF. F8
	pxor	xmm0, xmm1				; 1F84 _ 66: 0F EF. C1
	pand	xmm5, xmm7				; 1F88 _ 66: 0F DB. EF
	pxor	xmm1, xmm6				; 1F8C _ 66: 0F EF. CE
	por	xmm0, xmm7				; 1F90 _ 66: 0F EB. C7
	movd	xmm2, dword [ecx+2AE0H] 		; 1F94 _ 66: 0F 6E. 91, 00002AE0
	pcmpeqd xmm3, xmm3				; 1F9C _ 66: 0F 76. DB
	pxor	xmm5, xmm3				; 1FA0 _ 66: 0F EF. EB
	pxor	xmm5, xmm1				; 1FA4 _ 66: 0F EF. E9
	pxor	xmm7, xmm5				; 1FA8 _ 66: 0F EF. FD
	pxor	xmm1, xmm0				; 1FAC _ 66: 0F EF. C8
	pxor	xmm0, xmm6				; 1FB0 _ 66: 0F EF. C6
	pand	xmm6, xmm1				; 1FB4 _ 66: 0F DB. F1
	pxor	xmm6, xmm7				; 1FB8 _ 66: 0F EF. F7
	pxor	xmm7, xmm1				; 1FBC _ 66: 0F EF. F9
	por	xmm7, xmm5				; 1FC0 _ 66: 0F EB. FD
	pxor	xmm1, xmm6				; 1FC4 _ 66: 0F EF. CE
	pxor	xmm0, xmm7				; 1FC8 _ 66: 0F EF. C7
	movd	xmm7, dword [ecx+2AE4H] 		; 1FCC _ 66: 0F 6E. B9, 00002AE4
	pshufd	xmm2, xmm2, 0				; 1FD4 _ 66: 0F 70. D2, 00
	pxor	xmm6, xmm2				; 1FD9 _ 66: 0F EF. F2
	pshufd	xmm2, xmm7, 0				; 1FDD _ 66: 0F 70. D7, 00
	pxor	xmm5, xmm2				; 1FE2 _ 66: 0F EF. EA
	movd	xmm7, dword [ecx+2AE8H] 		; 1FE6 _ 66: 0F 6E. B9, 00002AE8
	pshufd	xmm2, xmm7, 0				; 1FEE _ 66: 0F 70. D7, 00
	pxor	xmm0, xmm2				; 1FF3 _ 66: 0F EF. C2
	movd	xmm7, dword [ecx+2AECH] 		; 1FF7 _ 66: 0F 6E. B9, 00002AEC
	pshufd	xmm2, xmm7, 0				; 1FFF _ 66: 0F 70. D7, 00
	movdqa	xmm7, xmm6				; 2004 _ 66: 0F 6F. FE
	pxor	xmm1, xmm2				; 2008 _ 66: 0F EF. CA
	movdqa	xmm2, xmm0				; 200C _ 66: 0F 6F. D0
	psrld	xmm0, 22				; 2010 _ 66: 0F 72. D0, 16
	pslld	xmm7, 27				; 2015 _ 66: 0F 72. F7, 1B
	pslld	xmm2, 10				; 201A _ 66: 0F 72. F2, 0A
	por	xmm2, xmm0				; 201F _ 66: 0F EB. D0
	movdqa	xmm0, xmm5				; 2023 _ 66: 0F 6F. C5
	psrld	xmm6, 5 				; 2027 _ 66: 0F 72. D6, 05
	por	xmm7, xmm6				; 202C _ 66: 0F EB. FE
	movdqa	xmm6, xmm1				; 2030 _ 66: 0F 6F. F1
	pxor	xmm2, xmm1				; 2034 _ 66: 0F EF. D1
	pslld	xmm0, 7 				; 2038 _ 66: 0F 72. F0, 07
	pxor	xmm2, xmm0				; 203D _ 66: 0F EF. D0
	movdqa	xmm0, xmm5				; 2041 _ 66: 0F 6F. C5
	pxor	xmm7, xmm5				; 2045 _ 66: 0F EF. FD
	pxor	xmm7, xmm1				; 2049 _ 66: 0F EF. F9
	pslld	xmm6, 25				; 204D _ 66: 0F 72. F6, 19
	psrld	xmm1, 7 				; 2052 _ 66: 0F 72. D1, 07
	por	xmm6, xmm1				; 2057 _ 66: 0F EB. F1
	movdqa	xmm1, xmm7				; 205B _ 66: 0F 6F. CF
	pslld	xmm0, 31				; 205F _ 66: 0F 72. F0, 1F
	psrld	xmm5, 1 				; 2064 _ 66: 0F 72. D5, 01
	por	xmm0, xmm5				; 2069 _ 66: 0F EB. C5
	pxor	xmm6, xmm2				; 206D _ 66: 0F EF. F2
	pslld	xmm1, 3 				; 2071 _ 66: 0F 72. F1, 03
	pxor	xmm6, xmm1				; 2076 _ 66: 0F EF. F1
	movdqa	xmm1, xmm2				; 207A _ 66: 0F 6F. CA
	movdqa	xmm5, xmm6				; 207E _ 66: 0F 6F. EE
	pxor	xmm0, xmm7				; 2082 _ 66: 0F EF. C7
	pxor	xmm0, xmm2				; 2086 _ 66: 0F EF. C2
	pslld	xmm1, 29				; 208A _ 66: 0F 72. F1, 1D
	psrld	xmm2, 3 				; 208F _ 66: 0F 72. D2, 03
	por	xmm1, xmm2				; 2094 _ 66: 0F EB. CA
	movdqa	xmm2, xmm7				; 2098 _ 66: 0F 6F. D7
	psrld	xmm7, 13				; 209C _ 66: 0F 72. D7, 0D
	pxor	xmm0, xmm3				; 20A1 _ 66: 0F EF. C3
	pslld	xmm2, 19				; 20A5 _ 66: 0F 72. F2, 13
	por	xmm2, xmm7				; 20AA _ 66: 0F EB. D7
	pxor	xmm1, xmm0				; 20AE _ 66: 0F EF. C8
	por	xmm5, xmm2				; 20B2 _ 66: 0F EB. EA
	movd	xmm7, dword [ecx+2AD0H] 		; 20B6 _ 66: 0F 6E. B9, 00002AD0
	pxor	xmm5, xmm1				; 20BE _ 66: 0F EF. E9
	por	xmm1, xmm0				; 20C2 _ 66: 0F EB. C8
	pand	xmm1, xmm2				; 20C6 _ 66: 0F DB. CA
	pxor	xmm6, xmm5				; 20CA _ 66: 0F EF. F5
	pxor	xmm1, xmm6				; 20CE _ 66: 0F EF. CE
	por	xmm6, xmm2				; 20D2 _ 66: 0F EB. F2
	pxor	xmm6, xmm0				; 20D6 _ 66: 0F EF. F0
	pand	xmm0, xmm1				; 20DA _ 66: 0F DB. C1
	pxor	xmm0, xmm5				; 20DE _ 66: 0F EF. C5
	pxor	xmm6, xmm1				; 20E2 _ 66: 0F EF. F1
	pand	xmm5, xmm6				; 20E6 _ 66: 0F DB. EE
	pxor	xmm6, xmm0				; 20EA _ 66: 0F EF. F0
	pxor	xmm5, xmm6				; 20EE _ 66: 0F EF. EE
	pxor	xmm6, xmm3				; 20F2 _ 66: 0F EF. F3
	pxor	xmm5, xmm2				; 20F6 _ 66: 0F EF. EA
	pshufd	xmm2, xmm7, 0				; 20FA _ 66: 0F 70. D7, 00
	pxor	xmm0, xmm2				; 20FF _ 66: 0F EF. C2
	movd	xmm7, dword [ecx+2AD4H] 		; 2103 _ 66: 0F 6E. B9, 00002AD4
	pshufd	xmm2, xmm7, 0				; 210B _ 66: 0F 70. D7, 00
	movd	xmm7, dword [ecx+2AD8H] 		; 2110 _ 66: 0F 6E. B9, 00002AD8
	pxor	xmm6, xmm2				; 2118 _ 66: 0F EF. F2
	pshufd	xmm2, xmm7, 0				; 211C _ 66: 0F 70. D7, 00
	pxor	xmm5, xmm2				; 2121 _ 66: 0F EF. EA
	movd	xmm7, dword [ecx+2ADCH] 		; 2125 _ 66: 0F 6E. B9, 00002ADC
	pshufd	xmm2, xmm7, 0				; 212D _ 66: 0F 70. D7, 00
	movdqa	xmm7, xmm5				; 2132 _ 66: 0F 6F. FD
	pxor	xmm1, xmm2				; 2136 _ 66: 0F EF. CA
	movdqa	xmm2, xmm0				; 213A _ 66: 0F 6F. D0
	pslld	xmm7, 10				; 213E _ 66: 0F 72. F7, 0A
	psrld	xmm5, 22				; 2143 _ 66: 0F 72. D5, 16
	por	xmm7, xmm5				; 2148 _ 66: 0F EB. FD
	movdqa	xmm5, xmm6				; 214C _ 66: 0F 6F. EE
	pslld	xmm2, 27				; 2150 _ 66: 0F 72. F2, 1B
	psrld	xmm0, 5 				; 2155 _ 66: 0F 72. D0, 05
	por	xmm2, xmm0				; 215A _ 66: 0F EB. D0
	movdqa	xmm0, xmm1				; 215E _ 66: 0F 6F. C1
	pxor	xmm7, xmm1				; 2162 _ 66: 0F EF. F9
	pslld	xmm5, 7 				; 2166 _ 66: 0F 72. F5, 07
	pxor	xmm7, xmm5				; 216B _ 66: 0F EF. FD
	movdqa	xmm5, xmm6				; 216F _ 66: 0F 6F. EE
	pxor	xmm2, xmm6				; 2173 _ 66: 0F EF. D6
	pxor	xmm2, xmm1				; 2177 _ 66: 0F EF. D1
	pslld	xmm0, 25				; 217B _ 66: 0F 72. F0, 19
	psrld	xmm1, 7 				; 2180 _ 66: 0F 72. D1, 07
	por	xmm0, xmm1				; 2185 _ 66: 0F EB. C1
	movdqa	xmm1, xmm7				; 2189 _ 66: 0F 6F. CF
	pslld	xmm5, 31				; 218D _ 66: 0F 72. F5, 1F
	psrld	xmm6, 1 				; 2192 _ 66: 0F 72. D6, 01
	por	xmm5, xmm6				; 2197 _ 66: 0F EB. EE
	movdqa	xmm6, xmm2				; 219B _ 66: 0F 6F. F2
	pxor	xmm0, xmm7				; 219F _ 66: 0F EF. C7
	pxor	xmm5, xmm2				; 21A3 _ 66: 0F EF. EA
	pslld	xmm6, 3 				; 21A7 _ 66: 0F 72. F6, 03
	pxor	xmm0, xmm6				; 21AC _ 66: 0F EF. C6
	movdqa	xmm6, xmm2				; 21B0 _ 66: 0F 6F. F2
	pxor	xmm5, xmm7				; 21B4 _ 66: 0F EF. EF
	pslld	xmm1, 29				; 21B8 _ 66: 0F 72. F1, 1D
	psrld	xmm7, 3 				; 21BD _ 66: 0F 72. D7, 03
	por	xmm1, xmm7				; 21C2 _ 66: 0F EB. CF
	pslld	xmm6, 19				; 21C6 _ 66: 0F 72. F6, 13
	psrld	xmm2, 13				; 21CB _ 66: 0F 72. D2, 0D
	por	xmm6, xmm2				; 21D0 _ 66: 0F EB. F2
	movdqa	xmm2, xmm1				; 21D4 _ 66: 0F 6F. D1
	movd	xmm7, dword [ecx+2AC0H] 		; 21D8 _ 66: 0F 6E. B9, 00002AC0
	pand	xmm2, xmm0				; 21E0 _ 66: 0F DB. D0
	pxor	xmm2, xmm5				; 21E4 _ 66: 0F EF. D5
	por	xmm5, xmm0				; 21E8 _ 66: 0F EB. E8
	pand	xmm5, xmm6				; 21EC _ 66: 0F DB. EE
	pxor	xmm1, xmm2				; 21F0 _ 66: 0F EF. CA
	pxor	xmm1, xmm5				; 21F4 _ 66: 0F EF. CD
	pand	xmm5, xmm2				; 21F8 _ 66: 0F DB. EA
	pxor	xmm6, xmm3				; 21FC _ 66: 0F EF. F3
	pxor	xmm0, xmm1				; 2200 _ 66: 0F EF. C1
	pxor	xmm5, xmm0				; 2204 _ 66: 0F EF. E8
	pand	xmm0, xmm6				; 2208 _ 66: 0F DB. C6
	pxor	xmm0, xmm2				; 220C _ 66: 0F EF. C2
	pxor	xmm6, xmm5				; 2210 _ 66: 0F EF. F5
	pand	xmm2, xmm6				; 2214 _ 66: 0F DB. D6
	pxor	xmm0, xmm6				; 2218 _ 66: 0F EF. C6
	pxor	xmm2, xmm1				; 221C _ 66: 0F EF. D1
	por	xmm2, xmm0				; 2220 _ 66: 0F EB. D0
	pxor	xmm0, xmm6				; 2224 _ 66: 0F EF. C6
	pxor	xmm2, xmm5				; 2228 _ 66: 0F EF. D5
	pshufd	xmm5, xmm7, 0				; 222C _ 66: 0F 70. EF, 00
	pxor	xmm6, xmm5				; 2231 _ 66: 0F EF. F5
	movd	xmm7, dword [ecx+2AC4H] 		; 2235 _ 66: 0F 6E. B9, 00002AC4
	pshufd	xmm5, xmm7, 0				; 223D _ 66: 0F 70. EF, 00
	pxor	xmm0, xmm5				; 2242 _ 66: 0F EF. C5
	movd	xmm7, dword [ecx+2AC8H] 		; 2246 _ 66: 0F 6E. B9, 00002AC8
	pshufd	xmm5, xmm7, 0				; 224E _ 66: 0F 70. EF, 00
	movd	xmm7, dword [ecx+2ACCH] 		; 2253 _ 66: 0F 6E. B9, 00002ACC
	pxor	xmm2, xmm5				; 225B _ 66: 0F EF. D5
	pshufd	xmm5, xmm7, 0				; 225F _ 66: 0F 70. EF, 00
	movdqa	xmm7, xmm6				; 2264 _ 66: 0F 6F. FE
	pxor	xmm1, xmm5				; 2268 _ 66: 0F EF. CD
	movdqa	xmm5, xmm2				; 226C _ 66: 0F 6F. EA
	psrld	xmm2, 22				; 2270 _ 66: 0F 72. D2, 16
	pslld	xmm7, 27				; 2275 _ 66: 0F 72. F7, 1B
	pslld	xmm5, 10				; 227A _ 66: 0F 72. F5, 0A
	por	xmm5, xmm2				; 227F _ 66: 0F EB. EA
	movdqa	xmm2, xmm0				; 2283 _ 66: 0F 6F. D0
	psrld	xmm6, 5 				; 2287 _ 66: 0F 72. D6, 05
	por	xmm7, xmm6				; 228C _ 66: 0F EB. FE
	movdqa	xmm6, xmm1				; 2290 _ 66: 0F 6F. F1
	pxor	xmm5, xmm1				; 2294 _ 66: 0F EF. E9
	pslld	xmm2, 7 				; 2298 _ 66: 0F 72. F2, 07
	pxor	xmm5, xmm2				; 229D _ 66: 0F EF. EA
	movdqa	xmm2, xmm0				; 22A1 _ 66: 0F 6F. D0
	pxor	xmm7, xmm0				; 22A5 _ 66: 0F EF. F8
	pxor	xmm7, xmm1				; 22A9 _ 66: 0F EF. F9
	pslld	xmm6, 25				; 22AD _ 66: 0F 72. F6, 19
	psrld	xmm1, 7 				; 22B2 _ 66: 0F 72. D1, 07
	por	xmm6, xmm1				; 22B7 _ 66: 0F EB. F1
	movdqa	xmm1, xmm5				; 22BB _ 66: 0F 6F. CD
	pslld	xmm2, 31				; 22BF _ 66: 0F 72. F2, 1F
	psrld	xmm0, 1 				; 22C4 _ 66: 0F 72. D0, 01
	por	xmm2, xmm0				; 22C9 _ 66: 0F EB. D0
	movdqa	xmm0, xmm7				; 22CD _ 66: 0F 6F. C7
	pxor	xmm6, xmm5				; 22D1 _ 66: 0F EF. F5
	pxor	xmm2, xmm7				; 22D5 _ 66: 0F EF. D7
	pslld	xmm0, 3 				; 22D9 _ 66: 0F 72. F0, 03
	pxor	xmm6, xmm0				; 22DE _ 66: 0F EF. F0
	movdqa	xmm0, xmm7				; 22E2 _ 66: 0F 6F. C7
	pxor	xmm2, xmm5				; 22E6 _ 66: 0F EF. D5
	pslld	xmm1, 29				; 22EA _ 66: 0F 72. F1, 1D
	psrld	xmm5, 3 				; 22EF _ 66: 0F 72. D5, 03
	por	xmm1, xmm5				; 22F4 _ 66: 0F EB. CD
	movd	xmm5, dword [ecx+2AB4H] 		; 22F8 _ 66: 0F 6E. A9, 00002AB4
	pslld	xmm0, 19				; 2300 _ 66: 0F 72. F0, 13
	psrld	xmm7, 13				; 2305 _ 66: 0F 72. D7, 0D
	por	xmm0, xmm7				; 230A _ 66: 0F EB. C7
	movdqa	xmm7, xmm1				; 230E _ 66: 0F 6F. F9
	pxor	xmm7, xmm2				; 2312 _ 66: 0F EF. FA
	pxor	xmm0, xmm7				; 2316 _ 66: 0F EF. C7
	pand	xmm1, xmm7				; 231A _ 66: 0F DB. CF
	pxor	xmm1, xmm0				; 231E _ 66: 0F EF. C8
	pand	xmm0, xmm2				; 2322 _ 66: 0F DB. C2
	pxor	xmm2, xmm6				; 2326 _ 66: 0F EF. D6
	por	xmm6, xmm1				; 232A _ 66: 0F EB. F1
	pxor	xmm7, xmm6				; 232E _ 66: 0F EF. FE
	pxor	xmm0, xmm6				; 2332 _ 66: 0F EF. C6
	pxor	xmm2, xmm1				; 2336 _ 66: 0F EF. D1
	pand	xmm6, xmm7				; 233A _ 66: 0F DB. F7
	pxor	xmm6, xmm2				; 233E _ 66: 0F EF. F2
	pxor	xmm2, xmm0				; 2342 _ 66: 0F EF. D0
	por	xmm2, xmm7				; 2346 _ 66: 0F EB. D7
	pxor	xmm0, xmm6				; 234A _ 66: 0F EF. C6
	pxor	xmm2, xmm1				; 234E _ 66: 0F EF. D1
	pxor	xmm0, xmm2				; 2352 _ 66: 0F EF. C2
	movd	xmm1, dword [ecx+2AB0H] 		; 2356 _ 66: 0F 6E. 89, 00002AB0
	pshufd	xmm1, xmm1, 0				; 235E _ 66: 0F 70. C9, 00
	pxor	xmm7, xmm1				; 2363 _ 66: 0F EF. F9
	pshufd	xmm1, xmm5, 0				; 2367 _ 66: 0F 70. CD, 00
	movd	xmm5, dword [ecx+2AB8H] 		; 236C _ 66: 0F 6E. A9, 00002AB8
	pxor	xmm2, xmm1				; 2374 _ 66: 0F EF. D1
	pshufd	xmm1, xmm5, 0				; 2378 _ 66: 0F 70. CD, 00
	pxor	xmm6, xmm1				; 237D _ 66: 0F EF. F1
	movd	xmm5, dword [ecx+2ABCH] 		; 2381 _ 66: 0F 6E. A9, 00002ABC
	pshufd	xmm1, xmm5, 0				; 2389 _ 66: 0F 70. CD, 00
	movdqa	xmm5, xmm6				; 238E _ 66: 0F 6F. EE
	pxor	xmm0, xmm1				; 2392 _ 66: 0F EF. C1
	movdqa	xmm1, xmm7				; 2396 _ 66: 0F 6F. CF
	pslld	xmm5, 10				; 239A _ 66: 0F 72. F5, 0A
	psrld	xmm6, 22				; 239F _ 66: 0F 72. D6, 16
	por	xmm5, xmm6				; 23A4 _ 66: 0F EB. EE
	movdqa	xmm6, xmm2				; 23A8 _ 66: 0F 6F. F2
	pslld	xmm1, 27				; 23AC _ 66: 0F 72. F1, 1B
	psrld	xmm7, 5 				; 23B1 _ 66: 0F 72. D7, 05
	por	xmm1, xmm7				; 23B6 _ 66: 0F EB. CF
	pxor	xmm5, xmm0				; 23BA _ 66: 0F EF. E8
	pslld	xmm6, 7 				; 23BE _ 66: 0F 72. F6, 07
	pxor	xmm5, xmm6				; 23C3 _ 66: 0F EF. EE
	movdqa	xmm6, xmm0				; 23C7 _ 66: 0F 6F. F0
	pxor	xmm1, xmm2				; 23CB _ 66: 0F EF. CA
	pxor	xmm1, xmm0				; 23CF _ 66: 0F EF. C8
	movdqa	xmm7, xmm1				; 23D3 _ 66: 0F 6F. F9
	pslld	xmm6, 25				; 23D7 _ 66: 0F 72. F6, 19
	psrld	xmm0, 7 				; 23DC _ 66: 0F 72. D0, 07
	por	xmm6, xmm0				; 23E1 _ 66: 0F EB. F0
	movdqa	xmm0, xmm2				; 23E5 _ 66: 0F 6F. C2
	psrld	xmm2, 1 				; 23E9 _ 66: 0F 72. D2, 01
	pxor	xmm6, xmm5				; 23EE _ 66: 0F EF. F5
	pslld	xmm0, 31				; 23F2 _ 66: 0F 72. F0, 1F
	por	xmm0, xmm2				; 23F7 _ 66: 0F EB. C2
	movdqa	xmm2, xmm5				; 23FB _ 66: 0F 6F. D5
	pslld	xmm7, 3 				; 23FF _ 66: 0F 72. F7, 03
	pxor	xmm6, xmm7				; 2404 _ 66: 0F EF. F7
	movdqa	xmm7, xmm1				; 2408 _ 66: 0F 6F. F9
	pxor	xmm0, xmm1				; 240C _ 66: 0F EF. C1
	pxor	xmm0, xmm5				; 2410 _ 66: 0F EF. C5
	pslld	xmm2, 29				; 2414 _ 66: 0F 72. F2, 1D
	psrld	xmm5, 3 				; 2419 _ 66: 0F 72. D5, 03
	por	xmm2, xmm5				; 241E _ 66: 0F EB. D5
	pslld	xmm7, 19				; 2422 _ 66: 0F 72. F7, 13
	psrld	xmm1, 13				; 2427 _ 66: 0F 72. D1, 0D
	por	xmm7, xmm1				; 242C _ 66: 0F EB. F9
	pxor	xmm2, xmm6				; 2430 _ 66: 0F EF. D6
	pxor	xmm6, xmm7				; 2434 _ 66: 0F EF. F7
	movdqa	xmm5, xmm6				; 2438 _ 66: 0F 6F. EE
	pand	xmm5, xmm2				; 243C _ 66: 0F DB. EA
	pxor	xmm5, xmm0				; 2440 _ 66: 0F EF. E8
	por	xmm0, xmm2				; 2444 _ 66: 0F EB. C2
	pxor	xmm0, xmm6				; 2448 _ 66: 0F EF. C6
	pand	xmm6, xmm5				; 244C _ 66: 0F DB. F5
	pxor	xmm2, xmm5				; 2450 _ 66: 0F EF. D5
	pand	xmm6, xmm7				; 2454 _ 66: 0F DB. F7
	pxor	xmm6, xmm2				; 2458 _ 66: 0F EF. F2
	pand	xmm2, xmm0				; 245C _ 66: 0F DB. D0
	por	xmm2, xmm7				; 2460 _ 66: 0F EB. D7
	pxor	xmm5, xmm3				; 2464 _ 66: 0F EF. EB
	movdqa	xmm1, xmm5				; 2468 _ 66: 0F 6F. CD
	pxor	xmm2, xmm5				; 246C _ 66: 0F EF. D5
	pxor	xmm7, xmm5				; 2470 _ 66: 0F EF. FD
	pxor	xmm1, xmm6				; 2474 _ 66: 0F EF. CE
	pand	xmm7, xmm0				; 2478 _ 66: 0F DB. F8
	movd	xmm5, dword [ecx+2AA4H] 		; 247C _ 66: 0F 6E. A9, 00002AA4
	pxor	xmm1, xmm7				; 2484 _ 66: 0F EF. CF
	movd	xmm7, dword [ecx+2AA0H] 		; 2488 _ 66: 0F 6E. B9, 00002AA0
	pshufd	xmm7, xmm7, 0				; 2490 _ 66: 0F 70. FF, 00
	pxor	xmm0, xmm7				; 2495 _ 66: 0F EF. C7
	pshufd	xmm7, xmm5, 0				; 2499 _ 66: 0F 70. FD, 00
	pxor	xmm6, xmm7				; 249E _ 66: 0F EF. F7
	movd	xmm5, dword [ecx+2AA8H] 		; 24A2 _ 66: 0F 6E. A9, 00002AA8
	pshufd	xmm7, xmm5, 0				; 24AA _ 66: 0F 70. FD, 00
	movd	xmm5, dword [ecx+2AACH] 		; 24AF _ 66: 0F 6E. A9, 00002AAC
	pxor	xmm2, xmm7				; 24B7 _ 66: 0F EF. D7
	pshufd	xmm7, xmm5, 0				; 24BB _ 66: 0F 70. FD, 00
	movdqa	xmm5, xmm2				; 24C0 _ 66: 0F 6F. EA
	pxor	xmm1, xmm7				; 24C4 _ 66: 0F EF. CF
	movdqa	xmm7, xmm1				; 24C8 _ 66: 0F 6F. F9
	pslld	xmm5, 10				; 24CC _ 66: 0F 72. F5, 0A
	psrld	xmm2, 22				; 24D1 _ 66: 0F 72. D2, 16
	por	xmm5, xmm2				; 24D6 _ 66: 0F EB. EA
	movdqa	xmm2, xmm0				; 24DA _ 66: 0F 6F. D0
	psrld	xmm0, 5 				; 24DE _ 66: 0F 72. D0, 05
	pxor	xmm5, xmm1				; 24E3 _ 66: 0F EF. E9
	pslld	xmm2, 27				; 24E7 _ 66: 0F 72. F2, 1B
	por	xmm2, xmm0				; 24EC _ 66: 0F EB. D0
	movdqa	xmm0, xmm6				; 24F0 _ 66: 0F 6F. C6
	pxor	xmm2, xmm6				; 24F4 _ 66: 0F EF. D6
	pxor	xmm2, xmm1				; 24F8 _ 66: 0F EF. D1
	pslld	xmm0, 7 				; 24FC _ 66: 0F 72. F0, 07
	pxor	xmm5, xmm0				; 2501 _ 66: 0F EF. E8
	pslld	xmm7, 25				; 2505 _ 66: 0F 72. F7, 19
	psrld	xmm1, 7 				; 250A _ 66: 0F 72. D1, 07
	por	xmm7, xmm1				; 250F _ 66: 0F EB. F9
	movdqa	xmm1, xmm6				; 2513 _ 66: 0F 6F. CE
	psrld	xmm6, 1 				; 2517 _ 66: 0F 72. D6, 01
	pxor	xmm7, xmm5				; 251C _ 66: 0F EF. FD
	pslld	xmm1, 31				; 2520 _ 66: 0F 72. F1, 1F
	por	xmm1, xmm6				; 2525 _ 66: 0F EB. CE
	movdqa	xmm6, xmm2				; 2529 _ 66: 0F 6F. F2
	pxor	xmm1, xmm2				; 252D _ 66: 0F EF. CA
	pxor	xmm1, xmm5				; 2531 _ 66: 0F EF. CD
	pslld	xmm6, 3 				; 2535 _ 66: 0F 72. F6, 03
	pxor	xmm7, xmm6				; 253A _ 66: 0F EF. FE
	movdqa	xmm6, xmm5				; 253E _ 66: 0F 6F. F5
	psrld	xmm5, 3 				; 2542 _ 66: 0F 72. D5, 03
	pslld	xmm6, 29				; 2547 _ 66: 0F 72. F6, 1D
	por	xmm6, xmm5				; 254C _ 66: 0F EB. F5
	movdqa	xmm5, xmm2				; 2550 _ 66: 0F 6F. EA
	psrld	xmm2, 13				; 2554 _ 66: 0F 72. D2, 0D
	pslld	xmm5, 19				; 2559 _ 66: 0F 72. F5, 13
	por	xmm5, xmm2				; 255E _ 66: 0F EB. EA
	movdqa	xmm2, xmm1				; 2562 _ 66: 0F 6F. D1
	pxor	xmm1, xmm6				; 2566 _ 66: 0F EF. CE
	pxor	xmm2, xmm7				; 256A _ 66: 0F EF. D7
	pand	xmm7, xmm2				; 256E _ 66: 0F DB. FA
	pxor	xmm7, xmm5				; 2572 _ 66: 0F EF. FD
	por	xmm5, xmm2				; 2576 _ 66: 0F EB. EA
	pxor	xmm6, xmm7				; 257A _ 66: 0F EF. F7
	pxor	xmm5, xmm1				; 257E _ 66: 0F EF. E9
	por	xmm5, xmm6				; 2582 _ 66: 0F EB. EE
	pxor	xmm2, xmm7				; 2586 _ 66: 0F EF. D7
	pxor	xmm5, xmm2				; 258A _ 66: 0F EF. EA
	por	xmm2, xmm7				; 258E _ 66: 0F EB. D7
	pxor	xmm2, xmm5				; 2592 _ 66: 0F EF. D5
	pxor	xmm1, xmm3				; 2596 _ 66: 0F EF. CB
	pxor	xmm1, xmm2				; 259A _ 66: 0F EF. CA
	por	xmm2, xmm5				; 259E _ 66: 0F EB. D5
	pxor	xmm2, xmm5				; 25A2 _ 66: 0F EF. D5
	por	xmm2, xmm1				; 25A6 _ 66: 0F EB. D1
	pxor	xmm7, xmm2				; 25AA _ 66: 0F EF. FA
	movd	xmm2, dword [ecx+2A90H] 		; 25AE _ 66: 0F 6E. 91, 00002A90
	pshufd	xmm0, xmm2, 0				; 25B6 _ 66: 0F 70. C2, 00
	pxor	xmm1, xmm0				; 25BB _ 66: 0F EF. C8
	movd	xmm2, dword [ecx+2A94H] 		; 25BF _ 66: 0F 6E. 91, 00002A94
	pshufd	xmm0, xmm2, 0				; 25C7 _ 66: 0F 70. C2, 00
	pxor	xmm5, xmm0				; 25CC _ 66: 0F EF. E8
	movd	xmm2, dword [ecx+2A98H] 		; 25D0 _ 66: 0F 6E. 91, 00002A98
	pshufd	xmm0, xmm2, 0				; 25D8 _ 66: 0F 70. C2, 00
	pxor	xmm7, xmm0				; 25DD _ 66: 0F EF. F8
	movd	xmm2, dword [ecx+2A9CH] 		; 25E1 _ 66: 0F 6E. 91, 00002A9C
	pshufd	xmm0, xmm2, 0				; 25E9 _ 66: 0F 70. C2, 00
	movdqa	xmm2, xmm7				; 25EE _ 66: 0F 6F. D7
	pxor	xmm6, xmm0				; 25F2 _ 66: 0F EF. F0
	movdqa	xmm0, xmm6				; 25F6 _ 66: 0F 6F. C6
	pslld	xmm2, 10				; 25FA _ 66: 0F 72. F2, 0A
	psrld	xmm7, 22				; 25FF _ 66: 0F 72. D7, 16
	por	xmm2, xmm7				; 2604 _ 66: 0F EB. D7
	movdqa	xmm7, xmm1				; 2608 _ 66: 0F 6F. F9
	psrld	xmm1, 5 				; 260C _ 66: 0F 72. D1, 05
	pxor	xmm2, xmm6				; 2611 _ 66: 0F EF. D6
	pslld	xmm7, 27				; 2615 _ 66: 0F 72. F7, 1B
	por	xmm7, xmm1				; 261A _ 66: 0F EB. F9
	movdqa	xmm1, xmm5				; 261E _ 66: 0F 6F. CD
	pxor	xmm7, xmm5				; 2622 _ 66: 0F EF. FD
	pxor	xmm7, xmm6				; 2626 _ 66: 0F EF. FE
	pslld	xmm1, 7 				; 262A _ 66: 0F 72. F1, 07
	pxor	xmm2, xmm1				; 262F _ 66: 0F EF. D1
	movdqa	xmm1, xmm5				; 2633 _ 66: 0F 6F. CD
	pslld	xmm0, 25				; 2637 _ 66: 0F 72. F0, 19
	psrld	xmm6, 7 				; 263C _ 66: 0F 72. D6, 07
	por	xmm0, xmm6				; 2641 _ 66: 0F EB. C6
	pslld	xmm1, 31				; 2645 _ 66: 0F 72. F1, 1F
	psrld	xmm5, 1 				; 264A _ 66: 0F 72. D5, 01
	por	xmm1, xmm5				; 264F _ 66: 0F EB. CD
	movdqa	xmm5, xmm7				; 2653 _ 66: 0F 6F. EF
	pxor	xmm0, xmm2				; 2657 _ 66: 0F EF. C2
	pxor	xmm1, xmm7				; 265B _ 66: 0F EF. CF
	pslld	xmm5, 3 				; 265F _ 66: 0F 72. F5, 03
	pxor	xmm0, xmm5				; 2664 _ 66: 0F EF. C5
	movdqa	xmm5, xmm2				; 2668 _ 66: 0F 6F. EA
	pxor	xmm1, xmm2				; 266C _ 66: 0F EF. CA
	movdqa	xmm6, xmm1				; 2670 _ 66: 0F 6F. F1
	pslld	xmm5, 29				; 2674 _ 66: 0F 72. F5, 1D
	psrld	xmm2, 3 				; 2679 _ 66: 0F 72. D2, 03
	por	xmm5, xmm2				; 267E _ 66: 0F EB. EA
	movdqa	xmm2, xmm7				; 2682 _ 66: 0F 6F. D7
	psrld	xmm7, 13				; 2686 _ 66: 0F 72. D7, 0D
	pxor	xmm5, xmm3				; 268B _ 66: 0F EF. EB
	pslld	xmm2, 19				; 268F _ 66: 0F 72. F2, 13
	por	xmm2, xmm7				; 2694 _ 66: 0F EB. D7
	movdqa	xmm7, oword [esp+60H]			; 2698 _ 66: 0F 6F. 7C 24, 60
	por	xmm6, xmm2				; 269E _ 66: 0F EB. F2
	pxor	xmm1, xmm3				; 26A2 _ 66: 0F EF. CB
	pxor	xmm6, xmm5				; 26A6 _ 66: 0F EF. F5
	movd	xmm3, dword [ecx+2A80H] 		; 26AA _ 66: 0F 6E. 99, 00002A80
	por	xmm5, xmm1				; 26B2 _ 66: 0F EB. E9
	pxor	xmm6, xmm0				; 26B6 _ 66: 0F EF. F0
	pxor	xmm2, xmm1				; 26BA _ 66: 0F EF. D1
	pxor	xmm5, xmm2				; 26BE _ 66: 0F EF. EA
	pand	xmm2, xmm0				; 26C2 _ 66: 0F DB. D0
	pxor	xmm1, xmm2				; 26C6 _ 66: 0F EF. CA
	por	xmm2, xmm6				; 26CA _ 66: 0F EB. D6
	pxor	xmm2, xmm5				; 26CE _ 66: 0F EF. D5
	pxor	xmm0, xmm1				; 26D2 _ 66: 0F EF. C1
	pxor	xmm5, xmm6				; 26D6 _ 66: 0F EF. EE
	pxor	xmm0, xmm2				; 26DA _ 66: 0F EF. C2
	pxor	xmm0, xmm6				; 26DE _ 66: 0F EF. C6
	pand	xmm5, xmm0				; 26E2 _ 66: 0F DB. E8
	pxor	xmm1, xmm5				; 26E6 _ 66: 0F EF. CD
	pshufd	xmm3, xmm3, 0				; 26EA _ 66: 0F 70. DB, 00
	pxor	xmm2, xmm3				; 26EF _ 66: 0F EF. D3
	add	esi, 64 				; 26F3 _ 83. C6, 40
	inc	eax					; 26F6 _ 40
	movd	xmm5, dword [ecx+2A84H] 		; 26F7 _ 66: 0F 6E. A9, 00002A84
	pshufd	xmm3, xmm5, 0				; 26FF _ 66: 0F 70. DD, 00
	pxor	xmm1, xmm3				; 2704 _ 66: 0F EF. CB
	movd	xmm5, dword [ecx+2A88H] 		; 2708 _ 66: 0F 6E. A9, 00002A88
	pshufd	xmm3, xmm5, 0				; 2710 _ 66: 0F 70. DD, 00
	movd	xmm5, dword [ecx+2A8CH] 		; 2715 _ 66: 0F 6E. A9, 00002A8C
	pxor	xmm6, xmm3				; 271D _ 66: 0F EF. F3
	pshufd	xmm3, xmm5, 0				; 2721 _ 66: 0F 70. DD, 00
	movdqa	xmm5, xmm2				; 2726 _ 66: 0F 6F. EA
	pxor	xmm0, xmm3				; 272A _ 66: 0F EF. C3
	movdqa	xmm3, xmm6				; 272E _ 66: 0F 6F. DE
	punpckldq xmm5, xmm1				; 2732 _ 66: 0F 62. E9
	punpckhdq xmm2, xmm1				; 2736 _ 66: 0F 6A. D1
	movdqa	xmm1, xmm2				; 273A _ 66: 0F 6F. CA
	punpckldq xmm3, xmm0				; 273E _ 66: 0F 62. D8
	punpckhdq xmm6, xmm0				; 2742 _ 66: 0F 6A. F0
	movdqa	xmm0, xmm5				; 2746 _ 66: 0F 6F. C5
	punpckhqdq xmm5, xmm3				; 274A _ 66: 0F 6D. EB
	punpcklqdq xmm1, xmm6				; 274E _ 66: 0F 6C. CE
	pxor	xmm5, oword [esp+40H]			; 2752 _ 66: 0F EF. 6C 24, 40
	pxor	xmm1, oword [esp+50H]			; 2758 _ 66: 0F EF. 4C 24, 50
	movdqu	oword [edx+10H], xmm5			; 275E _ F3: 0F 7F. 6A, 10
	movdqu	oword [edx+20H], xmm1			; 2763 _ F3: 0F 7F. 4A, 20
	punpcklqdq xmm0, xmm3				; 2768 _ 66: 0F 6C. C3
	punpckhqdq xmm2, xmm6				; 276C _ 66: 0F 6D. D6
	pxor	xmm0, xmm4				; 2770 _ 66: 0F EF. C4
	movdqu	oword [edx], xmm0			; 2774 _ F3: 0F 7F. 02
	movdqa	xmm4, xmm7				; 2778 _ 66: 0F 6F. E7
	pxor	xmm2, xmm7				; 277C _ 66: 0F EF. D7
	movdqu	oword [edx+30H], xmm2			; 2780 _ F3: 0F 7F. 52, 30
	movdqa	xmm2, xmm7				; 2785 _ 66: 0F 6F. D7
	psllq	xmm4, 1 				; 2789 _ 66: 0F 73. F4, 01
	psraw	xmm7, 8 				; 278E _ 66: 0F 71. E7, 08
	pslldq	xmm2, 8 				; 2793 _ 66: 0F 73. FA, 08
	psrldq	xmm2, 7 				; 2798 _ 66: 0F 73. DA, 07
	psrlq	xmm2, 7 				; 279D _ 66: 0F 73. D2, 07
	por	xmm4, xmm2				; 27A2 _ 66: 0F EB. E2
	psrldq	xmm7, 15				; 27A6 _ 66: 0F 73. DF, 0F
	pand	xmm7, oword [esp+30H]			; 27AB _ 66: 0F DB. 7C 24, 30
	pxor	xmm4, xmm7				; 27B1 _ 66: 0F EF. E7
	add	edx, 64 				; 27B5 _ 83. C2, 40
	cmp	eax, 8					; 27B8 _ 83. F8, 08
	jl	?_006					; 27BB _ 0F 8C, FFFFD8D0
	mov	dword [esp+24H], edx			; 27C1 _ 89. 54 24, 24
	mov	dword [esp+28H], esi			; 27C5 _ 89. 74 24, 28
	mov	esi, dword [esp+20H]			; 27C9 _ 8B. 74 24, 20
	add	esi, -512				; 27CD _ 81. C6, FFFFFE00
	jne	?_004					; 27D3 _ 0F 85, FFFFD886
	add	esp, 116				; 27D9 _ 83. C4, 74
	pop	ebx					; 27DC _ 5B
	pop	esi					; 27DD _ 5E
	pop	edi					; 27DE _ 5F
	mov	esp, ebp				; 27DF _ 8B. E5
	pop	ebp					; 27E1 _ 5D
	ret	24					; 27E2 _ C2, 0018
; _xts_serpent_sse2_decrypt@24 End of function

; Filling space: 0BH
; Filler type: lea with same source and destination
;       db 8DH, 74H, 26H, 00H, 8DH, 0BCH, 27H, 00H
;       db 00H, 00H, 00H

ALIGN	16


_xts_serpent_sse2_available@0:; Function begin
	push	esi					; 0000 _ 56
	push	ebx					; 0001 _ 53
	sub	esp, 16 				; 0002 _ 83. EC, 10
	mov	eax, 1					; 0005 _ B8, 00000001
	lea	esi, [esp]				; 000A _ 8D. 34 24
	cpuid						; 000D _ 0F A2
	mov	dword [esi], eax			; 000F _ 89. 06
	mov	dword [esi+4H], ebx			; 0011 _ 89. 5E, 04
	mov	dword [esi+8H], ecx			; 0014 _ 89. 4E, 08
	mov	dword [esi+0CH], edx			; 0017 _ 89. 56, 0C
	mov	eax, dword [esp+0CH]			; 001A _ 8B. 44 24, 0C
	and	eax, 4000000H				; 001E _ 25, 04000000
	shr	eax, 26 				; 0023 _ C1. E8, 1A
	add	esp, 16 				; 0026 _ 83. C4, 10
	pop	ebx					; 0029 _ 5B
	pop	esi					; 002A _ 5E
	ret						; 002B _ C3
; _xts_serpent_sse2_available@0 End of function

; Filling space: 4H
; Filler type: lea with same source and destination
;       db 8DH, 74H, 26H, 00H

ALIGN	8


