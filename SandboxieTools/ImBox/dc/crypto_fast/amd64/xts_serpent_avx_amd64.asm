; this code compiled with Intel® C++ Compiler Version 12.0.0.063
;
; Disassembly of file: xts_serpent_sse2.obj
; Thu May 19 19:05:15 2011
; Mode: 64 bits
; Syntax: YASM/NASM
; Instruction set: AVX, x64

default rel

global xts_serpent_avx_encrypt
global xts_serpent_avx_decrypt
global xts_serpent_avx_available

extern serpent256_encrypt				; near

SECTION .text	align=16 execute			; section number 2, code
;  Communal section not supported by YASM

xts_serpent_avx_encrypt:; Function begin
	push	r12					; 0000 _ 41: 54
	push	r13					; 0002 _ 41: 55
	push	r14					; 0004 _ 41: 56
	push	r15					; 0006 _ 41: 57
	push	rbp					; 0008 _ 55
	sub	rsp, 208				; 0009 _ 48: 81. EC, 000000D0
	mov	rbp, rdx				; 0010 _ 48: 89. D5
	mov	r10d, 135				; 0013 _ 41: BA, 00000087
	vpcmpeqd xmm0, xmm0, xmm0			; 0019 _ C5 F9: 76. C0
	mov	rax, qword [rsp+120H]			; 001D _ 48: 8B. 84 24, 00000120
	mov	r12, rax				; 0025 _ 49: 89. C4
	shr	r9, 9					; 0028 _ 49: C1. E9, 09
	mov	r13, rcx				; 002C _ 49: 89. CD
	mov	qword [rsp+0B0H], r9			; 002F _ 4C: 89. 8C 24, 000000B0
	mov	r14, r8 				; 0037 _ 4D: 89. C6
	vmovd	xmm1, r10d				; 003A _ C4 C1 79: 6E. CA
	lea	r9, [rax+2710H] 			; 003F _ 4C: 8D. 88, 00002710
	mov	qword [rsp+0B8H], 0			; 0046 _ 48: C7. 84 24, 000000B8, 00000000
	mov	r15, r9 				; 0052 _ 4D: 89. CF
	vmovups oword [rsp+0A0H], xmm6			; 0055 _ C5 F8: 11. B4 24, 000000A0
	vmovdqa xmm6, xmm0				; 005E _ C5 F9: 6F. F0
	vmovups oword [rsp+90H], xmm7			; 0062 _ C5 F8: 11. BC 24, 00000090
	vmovdqa xmm7, xmm1				; 006B _ C5 F9: 6F. F9
	vmovups oword [rsp+80H], xmm8			; 006F _ C5 78: 11. 84 24, 00000080
	vmovups oword [rsp+70H], xmm9			; 0078 _ C5 78: 11. 4C 24, 70
	vmovups oword [rsp+60H], xmm10			; 007E _ C5 78: 11. 54 24, 60
	vmovups oword [rsp+50H], xmm11			; 0084 _ C5 78: 11. 5C 24, 50
	vmovups oword [rsp+40H], xmm12			; 008A _ C5 78: 11. 64 24, 40
	vmovups oword [rsp+30H], xmm13			; 0090 _ C5 78: 11. 6C 24, 30
	vmovups oword [rsp+20H], xmm14			; 0096 _ C5 78: 11. 74 24, 20
	jmp	?_002					; 009C _ EB, 09

?_001:	vmovdqu oword [rsp+0C0H], xmm3			; 009E _ C5 FA: 7F. 9C 24, 000000C0
?_002:	mov	r8, r15 				; 00A7 _ 4D: 89. F8
	lea	rcx, [rsp+0B0H] 			; 00AA _ 48: 8D. 8C 24, 000000B0
	inc	qword [rsp+0B0H]			; 00B2 _ 48: FF. 84 24, 000000B0
	lea	rdx, [rsp+0C0H] 			; 00BA _ 48: 8D. 94 24, 000000C0
	call	serpent256_encrypt			; 00C2 _ E8, 00000000(rel)
	vmovdqu xmm3, oword [rsp+0C0H]			; 00C7 _ C5 FA: 6F. 9C 24, 000000C0
	xor	r10d, r10d				; 00D0 _ 45: 33. D2
?_003:	vpslldq xmm2, xmm3, 8				; 00D3 _ C5 E9: 73. FB, 08
	vpsllq	xmm4, xmm3, 1				; 00D8 _ C5 D9: 73. F3, 01
	vpsrldq xmm2, xmm2, 7				; 00DD _ C5 E9: 73. DA, 07
	vpsrlq	xmm2, xmm2, 7				; 00E2 _ C5 E9: 73. D2, 07
	vpor	xmm4, xmm4, xmm2			; 00E7 _ C5 D9: EB. E2
	vpsraw	xmm2, xmm3, 8				; 00EB _ C5 E9: 71. E3, 08
	vpsrldq xmm2, xmm2, 15				; 00F0 _ C5 E9: 73. DA, 0F
	vpand	xmm2, xmm2, xmm7			; 00F5 _ C5 E9: DB. D7
	vpxor	xmm2, xmm4, xmm2			; 00F9 _ C5 D9: EF. D2
	vpslldq xmm4, xmm2, 8				; 00FD _ C5 D9: 73. FA, 08
	vpsllq	xmm5, xmm2, 1				; 0102 _ C5 D1: 73. F2, 01
	vpsrldq xmm4, xmm4, 7				; 0107 _ C5 D9: 73. DC, 07
	vpsrlq	xmm4, xmm4, 7				; 010C _ C5 D9: 73. D4, 07
	vpor	xmm5, xmm5, xmm4			; 0111 _ C5 D1: EB. EC
	vpsraw	xmm4, xmm2, 8				; 0115 _ C5 D9: 71. E2, 08
	vpsrldq xmm4, xmm4, 15				; 011A _ C5 D9: 73. DC, 0F
	vpand	xmm4, xmm4, xmm7			; 011F _ C5 D9: DB. E7
	vpxor	xmm1, xmm5, xmm4			; 0123 _ C5 D1: EF. CC
	vpslldq xmm4, xmm1, 8				; 0127 _ C5 D9: 73. F9, 08
	vpsllq	xmm5, xmm1, 1				; 012C _ C5 D1: 73. F1, 01
	vpsrldq xmm4, xmm4, 7				; 0131 _ C5 D9: 73. DC, 07
	vpsrlq	xmm4, xmm4, 7				; 0136 _ C5 D9: 73. D4, 07
	vpor	xmm5, xmm5, xmm4			; 013B _ C5 D1: EB. EC
	vpsraw	xmm4, xmm1, 8				; 013F _ C5 D9: 71. E1, 08
	vpsrldq xmm4, xmm4, 15				; 0144 _ C5 D9: 73. DC, 0F
	vpand	xmm4, xmm4, xmm7			; 0149 _ C5 D9: DB. E7
	vpxor	xmm0, xmm5, xmm4			; 014D _ C5 D1: EF. C4
	vpxor	xmm9, xmm3, oword [r13] 		; 0151 _ C4 41 61: EF. 4D, 00
	vpxor	xmm8, xmm2, oword [r13+10H]		; 0157 _ C4 41 69: EF. 45, 10
	vpxor	xmm14, xmm1, oword [r13+20H]		; 015D _ C4 41 71: EF. 75, 20
	vpxor	xmm13, xmm0, oword [r13+30H]		; 0163 _ C4 41 79: EF. 6D, 30
	vpunpckldq xmm5, xmm9, xmm8			; 0169 _ C4 C1 31: 62. E8
	vpunpckldq xmm4, xmm14, xmm13			; 016E _ C4 C1 09: 62. E5
	vpunpckhdq xmm10, xmm9, xmm8			; 0173 _ C4 41 31: 6A. D0
	vpunpcklqdq xmm9, xmm5, xmm4			; 0178 _ C5 51: 6C. CC
	vpunpckhqdq xmm5, xmm5, xmm4			; 017C _ C5 D1: 6D. EC
	vmovd	xmm4, dword [r12+1284H] 		; 0180 _ C4 C1 79: 6E. A4 24, 00001284
	vpunpckhdq xmm11, xmm14, xmm13			; 018A _ C4 41 09: 6A. DD
	vpshufd xmm14, xmm4, 0				; 018F _ C5 79: 70. F4, 00
	vmovd	xmm12, dword [r12+1280H]		; 0194 _ C4 41 79: 6E. A4 24, 00001280
	vpxor	xmm4, xmm5, xmm14			; 019E _ C4 C1 51: EF. E6
	vpunpcklqdq xmm5, xmm10, xmm11			; 01A3 _ C4 C1 29: 6C. EB
	vpunpckhqdq xmm10, xmm10, xmm11 		; 01A8 _ C4 41 29: 6D. D3
	vmovd	xmm11, dword [r12+128CH]		; 01AD _ C4 41 79: 6E. 9C 24, 0000128C
	vmovd	xmm13, dword [r12+1288H]		; 01B7 _ C4 41 79: 6E. AC 24, 00001288
	vpshufd xmm8, xmm12, 0				; 01C1 _ C4 41 79: 70. C4, 00
	vpshufd xmm11, xmm11, 0 			; 01C7 _ C4 41 79: 70. DB, 00
	vpxor	xmm12, xmm9, xmm8			; 01CD _ C4 41 31: EF. E0
	vpshufd xmm9, xmm13, 0				; 01D2 _ C4 41 79: 70. CD, 00
	vpxor	xmm10, xmm10, xmm11			; 01D8 _ C4 41 29: EF. D3
	vpxor	xmm13, xmm5, xmm9			; 01DD _ C4 41 51: EF. E9
	vpxor	xmm8, xmm10, xmm12			; 01E2 _ C4 41 29: EF. C4
	vpxor	xmm5, xmm4, xmm13			; 01E7 _ C4 C1 59: EF. ED
	vpand	xmm4, xmm4, xmm8			; 01EC _ C4 C1 59: DB. E0
	vpxor	xmm14, xmm4, xmm12			; 01F1 _ C4 41 59: EF. F4
	vpor	xmm12, xmm12, xmm8			; 01F6 _ C4 41 19: EB. E0
	vpxor	xmm10, xmm5, xmm8			; 01FB _ C4 41 51: EF. D0
	vpxor	xmm4, xmm12, xmm5			; 0200 _ C5 99: EF. E5
	vpxor	xmm9, xmm8, xmm13			; 0204 _ C4 41 39: EF. CD
	vpor	xmm11, xmm13, xmm14			; 0209 _ C4 41 11: EB. DE
	vpxor	xmm8, xmm10, xmm6			; 020E _ C5 29: EF. C6
	vpxor	xmm5, xmm11, xmm10			; 0212 _ C4 C1 21: EF. EA
	vpor	xmm10, xmm8, xmm14			; 0217 _ C4 41 39: EB. D6
	vpor	xmm11, xmm9, xmm4			; 021C _ C5 31: EB. DC
	vpxor	xmm9, xmm14, xmm9			; 0220 _ C4 41 09: EF. C9
	vpslld	xmm8, xmm5, 3				; 0225 _ C5 B9: 72. F5, 03
	vpxor	xmm14, xmm9, xmm10			; 022A _ C4 41 31: EF. F2
	vpsrld	xmm5, xmm5, 29				; 022F _ C5 D1: 72. D5, 1D
	vpxor	xmm13, xmm14, xmm11			; 0234 _ C4 41 09: EF. EB
	vpxor	xmm11, xmm10, xmm11			; 0239 _ C4 41 29: EF. DB
	vpslld	xmm12, xmm13, 13			; 023E _ C4 C1 19: 72. F5, 0D
	vpsrld	xmm9, xmm13, 19 			; 0244 _ C4 C1 31: 72. D5, 13
	vpor	xmm12, xmm12, xmm9			; 024A _ C4 41 19: EB. E1
	vpor	xmm5, xmm8, xmm5			; 024F _ C5 B9: EB. ED
	vpxor	xmm10, xmm11, xmm12			; 0253 _ C4 41 21: EF. D4
	vpxor	xmm4, xmm4, xmm5			; 0258 _ C5 D9: EF. E5
	vpxor	xmm13, xmm10, xmm5			; 025C _ C5 29: EF. ED
	vpslld	xmm14, xmm12, 3 			; 0260 _ C4 C1 09: 72. F4, 03
	vpxor	xmm10, xmm4, xmm14			; 0266 _ C4 41 59: EF. D6
	vpslld	xmm4, xmm13, 1				; 026B _ C4 C1 59: 72. F5, 01
	vpsrld	xmm11, xmm13, 31			; 0271 _ C4 C1 21: 72. D5, 1F
	vpslld	xmm9, xmm10, 7				; 0277 _ C4 C1 31: 72. F2, 07
	vpor	xmm8, xmm4, xmm11			; 027D _ C4 41 59: EB. C3
	vpsrld	xmm14, xmm10, 25			; 0282 _ C4 C1 09: 72. D2, 19
	vpor	xmm13, xmm9, xmm14			; 0288 _ C4 41 31: EB. EE
	vpxor	xmm12, xmm12, xmm8			; 028D _ C4 41 19: EF. E0
	vpxor	xmm14, xmm12, xmm13			; 0292 _ C4 41 19: EF. F5
	vpxor	xmm5, xmm5, xmm13			; 0297 _ C4 C1 51: EF. ED
	vmovd	xmm12, dword [r12+129CH]		; 029C _ C4 41 79: 6E. A4 24, 0000129C
	vpslld	xmm4, xmm8, 7				; 02A6 _ C4 C1 59: 72. F0, 07
	vmovd	xmm11, dword [r12+1294H]		; 02AC _ C4 41 79: 6E. 9C 24, 00001294
	vpxor	xmm5, xmm5, xmm4			; 02B6 _ C5 D1: EF. EC
	vpshufd xmm4, xmm12, 0				; 02BA _ C4 C1 79: 70. E4, 00
	vmovd	xmm9, dword [r12+1290H] 		; 02C0 _ C4 41 79: 6E. 8C 24, 00001290
	vpshufd xmm10, xmm11, 0 			; 02CA _ C4 41 79: 70. D3, 00
	vpxor	xmm11, xmm13, xmm4			; 02D0 _ C5 11: EF. DC
	vpslld	xmm13, xmm14, 5 			; 02D4 _ C4 C1 11: 72. F6, 05
	vpsrld	xmm14, xmm14, 27			; 02DA _ C4 C1 09: 72. D6, 1B
	vpshufd xmm9, xmm9, 0				; 02E0 _ C4 41 79: 70. C9, 00
	vpxor	xmm8, xmm8, xmm10			; 02E6 _ C4 41 39: EF. C2
	vmovd	xmm10, dword [r12+1298H]		; 02EB _ C4 41 79: 6E. 94 24, 00001298
	vpor	xmm4, xmm13, xmm14			; 02F5 _ C4 C1 11: EB. E6
	vpslld	xmm13, xmm5, 22 			; 02FA _ C5 91: 72. F5, 16
	vpsrld	xmm5, xmm5, 10				; 02FF _ C5 D1: 72. D5, 0A
	vpshufd xmm10, xmm10, 0 			; 0304 _ C4 41 79: 70. D2, 00
	vpxor	xmm4, xmm4, xmm9			; 030A _ C4 C1 59: EF. E1
	vpor	xmm5, xmm13, xmm5			; 030F _ C5 91: EB. ED
	vpxor	xmm4, xmm4, xmm6			; 0313 _ C5 D9: EF. E6
	vpxor	xmm10, xmm5, xmm10			; 0317 _ C4 41 51: EF. D2
	vpand	xmm12, xmm4, xmm8			; 031C _ C4 41 59: DB. E0
	vpxor	xmm5, xmm10, xmm6			; 0321 _ C5 A9: EF. EE
	vpor	xmm14, xmm12, xmm11			; 0325 _ C4 41 19: EB. F3
	vpxor	xmm9, xmm5, xmm12			; 032A _ C4 41 51: EF. CC
	vpxor	xmm12, xmm14, xmm4			; 032F _ C5 09: EF. E4
	vpxor	xmm13, xmm11, xmm9			; 0333 _ C4 41 21: EF. E9
	vpxor	xmm11, xmm8, xmm14			; 0338 _ C4 41 39: EF. DE
	vpor	xmm8, xmm4, xmm11			; 033D _ C4 41 59: EB. C3
	vpxor	xmm14, xmm11, xmm13			; 0342 _ C4 41 21: EF. F5
	vpor	xmm4, xmm9, xmm12			; 0347 _ C4 C1 31: EB. E4
	vpxor	xmm10, xmm12, xmm14			; 034C _ C4 41 19: EF. D6
	vpand	xmm5, xmm4, xmm8			; 0351 _ C4 C1 59: DB. E8
	vpslld	xmm11, xmm13, 3 			; 0356 _ C4 C1 21: 72. F5, 03
	vpslld	xmm9, xmm5, 13				; 035C _ C5 B1: 72. F5, 0D
	vpsrld	xmm4, xmm5, 19				; 0361 _ C5 D9: 72. D5, 13
	vpand	xmm12, xmm10, xmm5			; 0366 _ C5 29: DB. E5
	vpor	xmm9, xmm9, xmm4			; 036A _ C5 31: EB. CC
	vpsrld	xmm13, xmm13, 29			; 036E _ C4 C1 11: 72. D5, 1D
	vpxor	xmm8, xmm8, xmm12			; 0374 _ C4 41 39: EF. C4
	vpand	xmm5, xmm14, xmm5			; 0379 _ C5 89: DB. ED
	vpor	xmm11, xmm11, xmm13			; 037D _ C4 41 21: EB. DD
	vpxor	xmm4, xmm8, xmm9			; 0382 _ C4 C1 39: EF. E1
	vpxor	xmm10, xmm5, xmm10			; 0387 _ C4 41 51: EF. D2
	vpxor	xmm4, xmm4, xmm11			; 038C _ C4 C1 59: EF. E3
	vpxor	xmm8, xmm10, xmm11			; 0391 _ C4 41 29: EF. C3
	vpslld	xmm14, xmm9, 3				; 0396 _ C4 C1 09: 72. F1, 03
	vpslld	xmm13, xmm4, 1				; 039C _ C5 91: 72. F4, 01
	vpxor	xmm10, xmm8, xmm14			; 03A1 _ C4 41 39: EF. D6
	vpsrld	xmm12, xmm4, 31 			; 03A6 _ C5 99: 72. D4, 1F
	vpor	xmm12, xmm13, xmm12			; 03AB _ C4 41 11: EB. E4
	vpslld	xmm4, xmm10, 7				; 03B0 _ C4 C1 59: 72. F2, 07
	vpsrld	xmm5, xmm10, 25 			; 03B6 _ C4 C1 51: 72. D2, 19
	vpxor	xmm9, xmm9, xmm12			; 03BC _ C4 41 31: EF. CC
	vpor	xmm13, xmm4, xmm5			; 03C1 _ C5 59: EB. ED
	vpslld	xmm4, xmm12, 7				; 03C5 _ C4 C1 59: 72. F4, 07
	vpxor	xmm10, xmm9, xmm13			; 03CB _ C4 41 31: EF. D5
	vpxor	xmm11, xmm11, xmm13			; 03D0 _ C4 41 21: EF. DD
	vpxor	xmm14, xmm11, xmm4			; 03D5 _ C5 21: EF. F4
	vpslld	xmm11, xmm10, 5 			; 03D9 _ C4 C1 21: 72. F2, 05
	vpsrld	xmm5, xmm10, 27 			; 03DF _ C4 C1 51: 72. D2, 1B
	vmovd	xmm9, dword [r12+12A0H] 		; 03E5 _ C4 41 79: 6E. 8C 24, 000012A0
	vpor	xmm8, xmm11, xmm5			; 03EF _ C5 21: EB. C5
	vmovd	xmm11, dword [r12+12A4H]		; 03F3 _ C4 41 79: 6E. 9C 24, 000012A4
	vpshufd xmm4, xmm9, 0				; 03FD _ C4 C1 79: 70. E1, 00
	vpshufd xmm10, xmm11, 0 			; 0403 _ C4 41 79: 70. D3, 00
	vpxor	xmm8, xmm8, xmm4			; 0409 _ C5 39: EF. C4
	vmovd	xmm4, dword [r12+12A8H] 		; 040D _ C4 C1 79: 6E. A4 24, 000012A8
	vpxor	xmm9, xmm12, xmm10			; 0417 _ C4 41 19: EF. CA
	vpslld	xmm12, xmm14, 22			; 041C _ C4 C1 19: 72. F6, 16
	vpsrld	xmm14, xmm14, 10			; 0422 _ C4 C1 09: 72. D6, 0A
	vmovd	xmm5, dword [r12+12ACH] 		; 0428 _ C4 C1 79: 6E. AC 24, 000012AC
	vpor	xmm11, xmm12, xmm14			; 0432 _ C4 41 19: EB. DE
	vpshufd xmm10, xmm4, 0				; 0437 _ C5 79: 70. D4, 00
	vpshufd xmm14, xmm5, 0				; 043C _ C5 79: 70. F5, 00
	vpxor	xmm12, xmm11, xmm10			; 0441 _ C4 41 21: EF. E2
	vpxor	xmm11, xmm13, xmm14			; 0446 _ C4 41 11: EF. DE
	vpand	xmm13, xmm8, xmm12			; 044B _ C4 41 39: DB. EC
	vpxor	xmm5, xmm13, xmm11			; 0450 _ C4 C1 11: EF. EB
	vpxor	xmm4, xmm12, xmm9			; 0455 _ C4 C1 19: EF. E1
	vpxor	xmm10, xmm4, xmm5			; 045A _ C5 59: EF. D5
	vpor	xmm14, xmm11, xmm8			; 045E _ C4 41 21: EB. F0
	vpxor	xmm13, xmm14, xmm9			; 0463 _ C4 41 09: EF. E9
	vpxor	xmm8, xmm8, xmm10			; 0468 _ C4 41 39: EF. C2
	vpor	xmm9, xmm13, xmm8			; 046D _ C4 41 11: EB. C8
	vpxor	xmm11, xmm9, xmm5			; 0472 _ C5 31: EF. DD
	vpand	xmm5, xmm5, xmm13			; 0476 _ C4 C1 51: DB. ED
	vpxor	xmm4, xmm8, xmm5			; 047B _ C5 B9: EF. E5
	vpxor	xmm5, xmm13, xmm11			; 047F _ C4 C1 11: EF. EB
	vpxor	xmm8, xmm5, xmm4			; 0484 _ C5 51: EF. C4
	vpslld	xmm9, xmm10, 13 			; 0488 _ C4 C1 31: 72. F2, 0D
	vpsrld	xmm10, xmm10, 19			; 048E _ C4 C1 29: 72. D2, 13
	vpsrld	xmm5, xmm8, 29				; 0494 _ C4 C1 51: 72. D0, 1D
	vpor	xmm12, xmm9, xmm10			; 049A _ C4 41 31: EB. E2
	vpslld	xmm10, xmm8, 3				; 049F _ C4 C1 29: 72. F0, 03
	vpor	xmm13, xmm10, xmm5			; 04A5 _ C5 29: EB. ED
	vpxor	xmm11, xmm11, xmm12			; 04A9 _ C4 41 21: EF. DC
	vpxor	xmm4, xmm4, xmm6			; 04AE _ C5 D9: EF. E6
	vpxor	xmm11, xmm11, xmm13			; 04B2 _ C4 41 21: EF. DD
	vpxor	xmm14, xmm4, xmm13			; 04B7 _ C4 41 59: EF. F5
	vpslld	xmm4, xmm12, 3				; 04BC _ C4 C1 59: 72. F4, 03
	vpxor	xmm9, xmm14, xmm4			; 04C2 _ C5 09: EF. CC
	vpslld	xmm10, xmm11, 1 			; 04C6 _ C4 C1 29: 72. F3, 01
	vpsrld	xmm5, xmm11, 31 			; 04CC _ C4 C1 51: 72. D3, 1F
	vpslld	xmm8, xmm9, 7				; 04D2 _ C4 C1 39: 72. F1, 07
	vpor	xmm14, xmm10, xmm5			; 04D8 _ C5 29: EB. F5
	vpsrld	xmm4, xmm9, 25				; 04DC _ C4 C1 59: 72. D1, 19
	vpor	xmm8, xmm8, xmm4			; 04E2 _ C5 39: EB. C4
	vpxor	xmm12, xmm12, xmm14			; 04E6 _ C4 41 19: EF. E6
	vpxor	xmm11, xmm12, xmm8			; 04EB _ C4 41 19: EF. D8
	vpxor	xmm13, xmm13, xmm8			; 04F0 _ C4 41 11: EF. E8
	vpslld	xmm4, xmm14, 7				; 04F5 _ C4 C1 59: 72. F6, 07
	vpslld	xmm5, xmm11, 5				; 04FB _ C4 C1 51: 72. F3, 05
	vpsrld	xmm9, xmm11, 27 			; 0501 _ C4 C1 31: 72. D3, 1B
	vpxor	xmm10, xmm13, xmm4			; 0507 _ C5 11: EF. D4
	vmovd	xmm11, dword [r12+12B4H]		; 050B _ C4 41 79: 6E. 9C 24, 000012B4
	vpor	xmm12, xmm5, xmm9			; 0515 _ C4 41 51: EB. E1
	vpshufd xmm5, xmm11, 0				; 051A _ C4 C1 79: 70. EB, 00
	vmovd	xmm13, dword [r12+12B0H]		; 0520 _ C4 41 79: 6E. AC 24, 000012B0
	vpxor	xmm11, xmm14, xmm5			; 052A _ C5 09: EF. DD
	vpslld	xmm14, xmm10, 22			; 052E _ C4 C1 09: 72. F2, 16
	vpsrld	xmm10, xmm10, 10			; 0534 _ C4 C1 29: 72. D2, 0A
	vpshufd xmm4, xmm13, 0				; 053A _ C4 C1 79: 70. E5, 00
	vpor	xmm10, xmm14, xmm10			; 0540 _ C4 41 09: EB. D2
	vmovd	xmm14, dword [r12+12BCH]		; 0545 _ C4 41 79: 6E. B4 24, 000012BC
	vpxor	xmm9, xmm12, xmm4			; 054F _ C5 19: EF. CC
	vmovd	xmm4, dword [r12+12B8H] 		; 0553 _ C4 C1 79: 6E. A4 24, 000012B8
	vpand	xmm12, xmm11, xmm9			; 055D _ C4 41 21: DB. E1
	vpshufd xmm13, xmm14, 0 			; 0562 _ C4 41 79: 70. EE, 00
	vpshufd xmm5, xmm4, 0				; 0568 _ C5 F9: 70. EC, 00
	vpxor	xmm8, xmm8, xmm13			; 056D _ C4 41 39: EF. C5
	vpxor	xmm4, xmm10, xmm5			; 0572 _ C5 A9: EF. E5
	vpor	xmm5, xmm9, xmm8			; 0576 _ C4 C1 31: EB. E8
	vpxor	xmm8, xmm8, xmm11			; 057B _ C4 41 39: EF. C3
	vpxor	xmm10, xmm4, xmm8			; 0580 _ C4 41 59: EF. D0
	vpxor	xmm4, xmm9, xmm4			; 0585 _ C5 B1: EF. E4
	vpor	xmm11, xmm4, xmm12			; 0589 _ C4 41 59: EB. DC
	vpand	xmm9, xmm8, xmm5			; 058E _ C5 39: DB. CD
	vpxor	xmm4, xmm9, xmm11			; 0592 _ C4 C1 31: EF. E3
	vpxor	xmm5, xmm5, xmm12			; 0597 _ C4 C1 51: EF. EC
	vpxor	xmm14, xmm12, xmm4			; 059C _ C5 19: EF. F4
	vpxor	xmm12, xmm5, xmm4			; 05A0 _ C5 51: EF. E4
	vpor	xmm13, xmm14, xmm5			; 05A4 _ C5 09: EB. ED
	vpand	xmm11, xmm11, xmm5			; 05A8 _ C5 21: DB. DD
	vpxor	xmm13, xmm13, xmm10			; 05AC _ C4 41 11: EF. EA
	vpxor	xmm10, xmm11, xmm10			; 05B1 _ C4 41 21: EF. D2
	vpor	xmm9, xmm13, xmm4			; 05B6 _ C5 11: EB. CC
	vpxor	xmm8, xmm12, xmm9			; 05BA _ C4 41 19: EF. C1
	vpslld	xmm9, xmm4, 3				; 05BF _ C5 B1: 72. F4, 03
	vpslld	xmm14, xmm8, 13 			; 05C4 _ C4 C1 09: 72. F0, 0D
	vpsrld	xmm12, xmm8, 19 			; 05CA _ C4 C1 19: 72. D0, 13
	vpor	xmm12, xmm14, xmm12			; 05D0 _ C4 41 09: EB. E4
	vpsrld	xmm4, xmm4, 29				; 05D5 _ C5 D9: 72. D4, 1D
	vpor	xmm14, xmm9, xmm4			; 05DA _ C5 31: EB. F4
	vpxor	xmm13, xmm13, xmm12			; 05DE _ C4 41 11: EF. EC
	vpxor	xmm9, xmm13, xmm14			; 05E3 _ C4 41 11: EF. CE
	vpxor	xmm5, xmm10, xmm14			; 05E8 _ C4 C1 29: EF. EE
	vpslld	xmm4, xmm12, 3				; 05ED _ C4 C1 59: 72. F4, 03
	vpslld	xmm11, xmm9, 1				; 05F3 _ C4 C1 21: 72. F1, 01
	vpxor	xmm8, xmm5, xmm4			; 05F9 _ C5 51: EF. C4
	vpsrld	xmm10, xmm9, 31 			; 05FD _ C4 C1 29: 72. D1, 1F
	vpor	xmm5, xmm11, xmm10			; 0603 _ C4 C1 21: EB. EA
	vpslld	xmm9, xmm8, 7				; 0608 _ C4 C1 31: 72. F0, 07
	vpsrld	xmm13, xmm8, 25 			; 060E _ C4 C1 11: 72. D0, 19
	vpxor	xmm12, xmm12, xmm5			; 0614 _ C5 19: EF. E5
	vpor	xmm10, xmm9, xmm13			; 0618 _ C4 41 31: EB. D5
	vpslld	xmm4, xmm5, 7				; 061D _ C5 D9: 72. F5, 07
	vpxor	xmm9, xmm12, xmm10			; 0622 _ C4 41 19: EF. CA
	vpxor	xmm14, xmm14, xmm10			; 0627 _ C4 41 09: EF. F2
	vpxor	xmm11, xmm14, xmm4			; 062C _ C5 09: EF. DC
	vpslld	xmm8, xmm9, 5				; 0630 _ C4 C1 39: 72. F1, 05
	vpsrld	xmm14, xmm9, 27 			; 0636 _ C4 C1 09: 72. D1, 1B
	vmovd	xmm9, dword [r12+12CCH] 		; 063C _ C4 41 79: 6E. 8C 24, 000012CC
	vpor	xmm12, xmm8, xmm14			; 0646 _ C4 41 39: EB. E6
	vmovd	xmm13, dword [r12+12C0H]		; 064B _ C4 41 79: 6E. AC 24, 000012C0
	vmovd	xmm14, dword [r12+12C4H]		; 0655 _ C4 41 79: 6E. B4 24, 000012C4
	vpshufd xmm8, xmm9, 0				; 065F _ C4 41 79: 70. C1, 00
	vpshufd xmm4, xmm13, 0				; 0665 _ C4 C1 79: 70. E5, 00
	vpxor	xmm9, xmm10, xmm8			; 066B _ C4 41 29: EF. C8
	vpshufd xmm10, xmm14, 0 			; 0670 _ C4 41 79: 70. D6, 00
	vpxor	xmm13, xmm12, xmm4			; 0676 _ C5 19: EF. EC
	vmovd	xmm4, dword [r12+12C8H] 		; 067A _ C4 C1 79: 6E. A4 24, 000012C8
	vpxor	xmm5, xmm5, xmm10			; 0684 _ C4 C1 51: EF. EA
	vpslld	xmm10, xmm11, 22			; 0689 _ C4 C1 29: 72. F3, 16
	vpsrld	xmm11, xmm11, 10			; 068F _ C4 C1 21: 72. D3, 0A
	vpshufd xmm4, xmm4, 0				; 0695 _ C5 F9: 70. E4, 00
	vpor	xmm11, xmm10, xmm11			; 069A _ C4 41 29: EB. DB
	vpxor	xmm14, xmm9, xmm6			; 069F _ C5 31: EF. F6
	vpxor	xmm10, xmm11, xmm4			; 06A3 _ C5 21: EF. D4
	vpxor	xmm8, xmm5, xmm9			; 06A7 _ C4 41 51: EF. C1
	vpxor	xmm4, xmm10, xmm14			; 06AC _ C4 C1 29: EF. E6
	vpxor	xmm10, xmm14, xmm13			; 06B1 _ C4 41 09: EF. D5
	vpand	xmm5, xmm8, xmm10			; 06B6 _ C4 C1 39: DB. EA
	vpxor	xmm12, xmm8, xmm10			; 06BB _ C4 41 39: EF. E2
	vpxor	xmm14, xmm5, xmm4			; 06C0 _ C5 51: EF. F4
	vpxor	xmm13, xmm13, xmm12			; 06C4 _ C4 41 11: EF. EC
	vpand	xmm5, xmm13, xmm14			; 06C9 _ C4 C1 11: DB. EE
	vpand	xmm8, xmm4, xmm12			; 06CE _ C4 41 59: DB. C4
	vpxor	xmm11, xmm10, xmm5			; 06D3 _ C5 29: EF. DD
	vpxor	xmm9, xmm8, xmm13			; 06D7 _ C4 41 39: EF. CD
	vpor	xmm4, xmm5, xmm11			; 06DC _ C4 C1 51: EB. E3
	vpor	xmm12, xmm12, xmm14			; 06E1 _ C4 41 19: EB. E6
	vpxor	xmm10, xmm4, xmm9			; 06E6 _ C4 41 59: EF. D1
	vpslld	xmm8, xmm14, 13 			; 06EB _ C4 C1 39: 72. F6, 0D
	vpxor	xmm4, xmm10, xmm6			; 06F1 _ C5 A9: EF. E6
	vpsrld	xmm13, xmm14, 19			; 06F5 _ C4 C1 11: 72. D6, 13
	vpslld	xmm10, xmm4, 3				; 06FB _ C5 A9: 72. F4, 03
	vpsrld	xmm4, xmm4, 29				; 0700 _ C5 D9: 72. D4, 1D
	vpor	xmm10, xmm10, xmm4			; 0705 _ C5 29: EB. D4
	vpxor	xmm14, xmm12, xmm5			; 0709 _ C5 19: EF. F5
	vpand	xmm4, xmm9, xmm11			; 070D _ C4 C1 31: DB. E3
	vpor	xmm13, xmm8, xmm13			; 0712 _ C4 41 39: EB. ED
	vpxor	xmm5, xmm14, xmm4			; 0717 _ C5 89: EF. EC
	vpxor	xmm11, xmm11, xmm10			; 071B _ C4 41 21: EF. DA
	vpxor	xmm9, xmm5, xmm13			; 0720 _ C4 41 51: EF. CD
	vpslld	xmm8, xmm13, 3				; 0725 _ C4 C1 39: 72. F5, 03
	vpxor	xmm14, xmm9, xmm10			; 072B _ C4 41 31: EF. F2
	vpxor	xmm5, xmm11, xmm8			; 0730 _ C4 C1 21: EF. E8
	vpslld	xmm12, xmm14, 1 			; 0735 _ C4 C1 19: 72. F6, 01
	vpsrld	xmm4, xmm14, 31 			; 073B _ C4 C1 59: 72. D6, 1F
	vpor	xmm11, xmm12, xmm4			; 0741 _ C5 19: EB. DC
	vpslld	xmm9, xmm5, 7				; 0745 _ C5 B1: 72. F5, 07
	vpsrld	xmm8, xmm5, 25				; 074A _ C5 B9: 72. D5, 19
	vpxor	xmm13, xmm13, xmm11			; 074F _ C4 41 11: EF. EB
	vpor	xmm12, xmm9, xmm8			; 0754 _ C4 41 31: EB. E0
	vpslld	xmm4, xmm11, 7				; 0759 _ C4 C1 59: 72. F3, 07
	vpxor	xmm8, xmm13, xmm12			; 075F _ C4 41 11: EF. C4
	vpxor	xmm10, xmm10, xmm12			; 0764 _ C4 41 29: EF. D4
	vmovd	xmm13, dword [r12+12DCH]		; 0769 _ C4 41 79: 6E. AC 24, 000012DC
	vpxor	xmm14, xmm10, xmm4			; 0773 _ C5 29: EF. F4
	vpshufd xmm4, xmm13, 0				; 0777 _ C4 C1 79: 70. E5, 00
	vmovd	xmm5, dword [r12+12D0H] 		; 077D _ C4 C1 79: 6E. AC 24, 000012D0
	vpxor	xmm4, xmm12, xmm4			; 0787 _ C5 99: EF. E4
	vmovd	xmm10, dword [r12+12D4H]		; 078B _ C4 41 79: 6E. 94 24, 000012D4
	vpslld	xmm12, xmm8, 5				; 0795 _ C4 C1 19: 72. F0, 05
	vpsrld	xmm8, xmm8, 27				; 079B _ C4 C1 39: 72. D0, 1B
	vpshufd xmm9, xmm10, 0				; 07A1 _ C4 41 79: 70. CA, 00
	vpor	xmm10, xmm12, xmm8			; 07A7 _ C4 41 19: EB. D0
	vpshufd xmm5, xmm5, 0				; 07AC _ C5 F9: 70. ED, 00
	vpxor	xmm9, xmm11, xmm9			; 07B1 _ C4 41 21: EF. C9
	vpxor	xmm10, xmm10, xmm5			; 07B6 _ C5 29: EF. D5
	vpxor	xmm13, xmm9, xmm4			; 07BA _ C5 31: EF. EC
	vmovd	xmm11, dword [r12+12D8H]		; 07BE _ C4 41 79: 6E. 9C 24, 000012D8
	vpxor	xmm5, xmm10, xmm9			; 07C8 _ C4 C1 29: EF. E9
	vpxor	xmm10, xmm4, xmm6			; 07CD _ C5 59: EF. D6
	vpslld	xmm4, xmm14, 22 			; 07D1 _ C4 C1 59: 72. F6, 16
	vpsrld	xmm14, xmm14, 10			; 07D7 _ C4 C1 09: 72. D6, 0A
	vpshufd xmm11, xmm11, 0 			; 07DD _ C4 41 79: 70. DB, 00
	vpor	xmm4, xmm4, xmm14			; 07E3 _ C4 C1 59: EB. E6
	vpxor	xmm4, xmm4, xmm11			; 07E8 _ C4 C1 59: EF. E3
	vpand	xmm11, xmm13, xmm5			; 07ED _ C5 11: DB. DD
	vpxor	xmm9, xmm4, xmm10			; 07F1 _ C4 41 59: EF. CA
	vpxor	xmm4, xmm11, xmm9			; 07F6 _ C4 C1 21: EF. E1
	vpor	xmm9, xmm9, xmm13			; 07FB _ C4 41 31: EB. CD
	vpand	xmm8, xmm10, xmm4			; 0800 _ C5 29: DB. C4
	vpxor	xmm10, xmm13, xmm10			; 0804 _ C4 41 11: EF. D2
	vpxor	xmm8, xmm8, xmm5			; 0809 _ C5 39: EF. C5
	vpxor	xmm14, xmm10, xmm4			; 080D _ C5 29: EF. F4
	vpxor	xmm14, xmm14, xmm9			; 0811 _ C4 41 09: EF. F1
	vpand	xmm12, xmm5, xmm8			; 0816 _ C4 41 51: DB. E0
	vpxor	xmm11, xmm12, xmm14			; 081B _ C4 41 19: EF. DE
	vpslld	xmm13, xmm4, 13 			; 0820 _ C5 91: 72. F4, 0D
	vpsrld	xmm4, xmm4, 19				; 0825 _ C5 D9: 72. D4, 13
	vpslld	xmm10, xmm11, 3 			; 082A _ C4 C1 29: 72. F3, 03
	vpor	xmm4, xmm13, xmm4			; 0830 _ C5 91: EB. E4
	vpsrld	xmm11, xmm11, 29			; 0834 _ C4 C1 21: 72. D3, 1D
	vpxor	xmm5, xmm9, xmm5			; 083A _ C5 B1: EF. ED
	vpor	xmm11, xmm10, xmm11			; 083E _ C4 41 29: EB. DB
	vpxor	xmm12, xmm8, xmm4			; 0843 _ C5 39: EF. E4
	vpor	xmm8, xmm14, xmm8			; 0847 _ C4 41 09: EB. C0
	vpxor	xmm5, xmm5, xmm6			; 084C _ C5 D1: EF. EE
	vpxor	xmm10, xmm12, xmm11			; 0850 _ C4 41 19: EF. D3
	vpxor	xmm9, xmm8, xmm5			; 0855 _ C5 39: EF. CD
	vpslld	xmm13, xmm4, 3				; 0859 _ C5 91: 72. F4, 03
	vpxor	xmm12, xmm9, xmm11			; 085E _ C4 41 31: EF. E3
	vpslld	xmm14, xmm10, 1 			; 0863 _ C4 C1 09: 72. F2, 01
	vpsrld	xmm10, xmm10, 31			; 0869 _ C4 C1 29: 72. D2, 1F
	vpxor	xmm8, xmm12, xmm13			; 086F _ C4 41 19: EF. C5
	vpor	xmm5, xmm14, xmm10			; 0874 _ C4 C1 09: EB. EA
	vpslld	xmm13, xmm8, 7				; 0879 _ C4 C1 11: 72. F0, 07
	vpsrld	xmm10, xmm8, 25 			; 087F _ C4 C1 29: 72. D0, 19
	vpor	xmm8, xmm13, xmm10			; 0885 _ C4 41 11: EB. C2
	vpxor	xmm4, xmm4, xmm5			; 088A _ C5 D9: EF. E5
	vpxor	xmm4, xmm4, xmm8			; 088E _ C4 C1 59: EF. E0
	vpxor	xmm11, xmm11, xmm8			; 0893 _ C4 41 21: EF. D8
	vmovd	xmm10, dword [r12+12E0H]		; 0898 _ C4 41 79: 6E. 94 24, 000012E0
	vpslld	xmm9, xmm5, 7				; 08A2 _ C5 B1: 72. F5, 07
	vpslld	xmm14, xmm4, 5				; 08A7 _ C5 89: 72. F4, 05
	vpsrld	xmm13, xmm4, 27 			; 08AC _ C5 91: 72. D4, 1B
	vpxor	xmm12, xmm11, xmm9			; 08B1 _ C4 41 21: EF. E1
	vpor	xmm11, xmm14, xmm13			; 08B6 _ C4 41 09: EB. DD
	vpshufd xmm9, xmm10, 0				; 08BB _ C4 41 79: 70. CA, 00
	vmovd	xmm10, dword [r12+12E8H]		; 08C1 _ C4 41 79: 6E. 94 24, 000012E8
	vpxor	xmm14, xmm11, xmm9			; 08CB _ C4 41 21: EF. F1
	vmovd	xmm13, dword [r12+12ECH]		; 08D0 _ C4 41 79: 6E. AC 24, 000012EC
	vpslld	xmm9, xmm12, 22 			; 08DA _ C4 C1 31: 72. F4, 16
	vpsrld	xmm12, xmm12, 10			; 08E0 _ C4 C1 19: 72. D4, 0A
	vpshufd xmm11, xmm13, 0 			; 08E6 _ C4 41 79: 70. DD, 00
	vpor	xmm12, xmm9, xmm12			; 08EC _ C4 41 31: EB. E4
	vpshufd xmm13, xmm10, 0 			; 08F1 _ C4 41 79: 70. EA, 00
	vpxor	xmm8, xmm8, xmm11			; 08F7 _ C4 41 39: EF. C3
	vmovd	xmm4, dword [r12+12E4H] 		; 08FC _ C4 C1 79: 6E. A4 24, 000012E4
	vpxor	xmm10, xmm12, xmm13			; 0906 _ C4 41 19: EF. D5
	vpshufd xmm4, xmm4, 0				; 090B _ C5 F9: 70. E4, 00
	vpxor	xmm11, xmm10, xmm6			; 0910 _ C5 29: EF. DE
	vpxor	xmm9, xmm14, xmm8			; 0914 _ C4 41 09: EF. C8
	vpand	xmm14, xmm8, xmm14			; 0919 _ C4 41 39: DB. F6
	vpxor	xmm10, xmm14, xmm11			; 091E _ C4 41 09: EF. D3
	vpxor	xmm5, xmm5, xmm4			; 0923 _ C5 D1: EF. EC
	vpxor	xmm5, xmm5, xmm10			; 0927 _ C4 C1 51: EF. EA
	vpor	xmm11, xmm11, xmm8			; 092C _ C4 41 21: EB. D8
	vpor	xmm13, xmm9, xmm5			; 0931 _ C5 31: EB. ED
	vpxor	xmm9, xmm11, xmm9			; 0935 _ C4 41 21: EF. C9
	vpxor	xmm9, xmm9, xmm5			; 093A _ C5 31: EF. CD
	vpor	xmm4, xmm13, xmm10			; 093E _ C4 C1 11: EB. E2
	vpxor	xmm8, xmm8, xmm13			; 0943 _ C4 41 39: EF. C5
	vpxor	xmm12, xmm4, xmm9			; 0948 _ C4 41 59: EF. E1
	vpxor	xmm4, xmm8, xmm10			; 094D _ C4 C1 39: EF. E2
	vpslld	xmm8, xmm12, 13 			; 0952 _ C4 C1 39: 72. F4, 0D
	vpxor	xmm4, xmm4, xmm12			; 0958 _ C4 C1 59: EF. E4
	vpsrld	xmm14, xmm12, 19			; 095D _ C4 C1 09: 72. D4, 13
	vpor	xmm14, xmm8, xmm14			; 0963 _ C4 41 39: EB. F6
	vpslld	xmm12, xmm4, 3				; 0968 _ C5 99: 72. F4, 03
	vpsrld	xmm13, xmm4, 29 			; 096D _ C5 91: 72. D4, 1D
	vpxor	xmm5, xmm5, xmm14			; 0972 _ C4 C1 51: EF. EE
	vpor	xmm8, xmm12, xmm13			; 0977 _ C4 41 19: EB. C5
	vpxor	xmm10, xmm10, xmm6			; 097C _ C5 29: EF. D6
	vpxor	xmm12, xmm5, xmm8			; 0980 _ C4 41 51: EF. E0
	vpand	xmm5, xmm9, xmm4			; 0985 _ C5 B1: DB. EC
	vpxor	xmm4, xmm10, xmm5			; 0989 _ C5 A9: EF. E5
	vpslld	xmm10, xmm14, 3 			; 098D _ C4 C1 29: 72. F6, 03
	vpxor	xmm13, xmm4, xmm8			; 0993 _ C4 41 59: EF. E8
	vpslld	xmm11, xmm12, 1 			; 0998 _ C4 C1 21: 72. F4, 01
	vpxor	xmm9, xmm13, xmm10			; 099E _ C4 41 11: EF. CA
	vpsrld	xmm12, xmm12, 31			; 09A3 _ C4 C1 19: 72. D4, 1F
	vpor	xmm4, xmm11, xmm12			; 09A9 _ C4 C1 21: EB. E4
	vpslld	xmm5, xmm9, 7				; 09AE _ C4 C1 51: 72. F1, 07
	vpsrld	xmm12, xmm9, 25 			; 09B4 _ C4 C1 19: 72. D1, 19
	vpxor	xmm14, xmm14, xmm4			; 09BA _ C5 09: EF. F4
	vpor	xmm5, xmm5, xmm12			; 09BE _ C4 C1 51: EB. EC
	vpslld	xmm13, xmm4, 7				; 09C3 _ C5 91: 72. F4, 07
	vpxor	xmm10, xmm14, xmm5			; 09C8 _ C5 09: EF. D5
	vpxor	xmm8, xmm8, xmm5			; 09CC _ C5 39: EF. C5
	vpslld	xmm11, xmm10, 5 			; 09D0 _ C4 C1 21: 72. F2, 05
	vpsrld	xmm9, xmm10, 27 			; 09D6 _ C4 C1 31: 72. D2, 1B
	vmovd	xmm10, dword [r12+12F4H]		; 09DC _ C4 41 79: 6E. 94 24, 000012F4
	vpxor	xmm13, xmm8, xmm13			; 09E6 _ C4 41 39: EF. ED
	vmovd	xmm8, dword [r12+12F0H] 		; 09EB _ C4 41 79: 6E. 84 24, 000012F0
	vpor	xmm14, xmm11, xmm9			; 09F5 _ C4 41 21: EB. F1
	vpshufd xmm11, xmm10, 0 			; 09FA _ C4 41 79: 70. DA, 00
	vpshufd xmm12, xmm8, 0				; 0A00 _ C4 41 79: 70. E0, 00
	vpxor	xmm9, xmm4, xmm11			; 0A06 _ C4 41 59: EF. CB
	vpslld	xmm4, xmm13, 22 			; 0A0B _ C4 C1 59: 72. F5, 16
	vpsrld	xmm8, xmm13, 10 			; 0A11 _ C4 C1 39: 72. D5, 0A
	vpxor	xmm12, xmm14, xmm12			; 0A17 _ C4 41 09: EF. E4
	vpor	xmm14, xmm4, xmm8			; 0A1C _ C4 41 59: EB. F0
	vmovd	xmm4, dword [r12+12F8H] 		; 0A21 _ C4 C1 79: 6E. A4 24, 000012F8
	vmovd	xmm11, dword [r12+12FCH]		; 0A2B _ C4 41 79: 6E. 9C 24, 000012FC
	vpshufd xmm13, xmm4, 0				; 0A35 _ C5 79: 70. EC, 00
	vpshufd xmm4, xmm11, 0				; 0A3A _ C4 C1 79: 70. E3, 00
	vpxor	xmm10, xmm14, xmm13			; 0A40 _ C4 41 09: EF. D5
	vpxor	xmm14, xmm5, xmm4			; 0A45 _ C5 51: EF. F4
	vpor	xmm5, xmm9, xmm10			; 0A49 _ C4 C1 31: EB. EA
	vpxor	xmm13, xmm5, xmm14			; 0A4E _ C4 41 51: EF. EE
	vpxor	xmm9, xmm9, xmm10			; 0A53 _ C4 41 31: EF. CA
	vpxor	xmm4, xmm10, xmm13			; 0A58 _ C4 C1 29: EF. E5
	vpxor	xmm8, xmm9, xmm4			; 0A5D _ C5 31: EF. C4
	vpor	xmm9, xmm14, xmm9			; 0A61 _ C4 41 09: EB. C9
	vpor	xmm5, xmm12, xmm8			; 0A66 _ C4 C1 19: EB. E8
	vpor	xmm11, xmm13, xmm8			; 0A6B _ C4 41 11: EB. D8
	vpxor	xmm10, xmm5, xmm4			; 0A70 _ C5 51: EF. D4
	vpxor	xmm5, xmm11, xmm12			; 0A74 _ C4 C1 21: EF. EC
	vpxor	xmm5, xmm5, xmm8			; 0A79 _ C4 C1 51: EF. E8
	vpand	xmm12, xmm9, xmm12			; 0A7E _ C4 41 31: DB. E4
	vpxor	xmm4, xmm4, xmm5			; 0A83 _ C5 D9: EF. E5
	vpand	xmm11, xmm5, xmm10			; 0A87 _ C4 41 51: DB. DA
	vpxor	xmm5, xmm4, xmm6			; 0A8C _ C5 D9: EF. EE
	vpxor	xmm11, xmm11, xmm8			; 0A90 _ C4 41 21: EF. D8
	vpor	xmm4, xmm5, xmm10			; 0A95 _ C4 C1 51: EB. E2
	vpxor	xmm8, xmm8, xmm4			; 0A9A _ C5 39: EF. C4
	vpslld	xmm5, xmm8, 13				; 0A9E _ C4 C1 51: 72. F0, 0D
	vpsrld	xmm4, xmm8, 19				; 0AA4 _ C4 C1 59: 72. D0, 13
	vpor	xmm5, xmm5, xmm4			; 0AAA _ C5 D1: EB. EC
	vpslld	xmm8, xmm11, 3				; 0AAE _ C4 C1 39: 72. F3, 03
	vpsrld	xmm11, xmm11, 29			; 0AB4 _ C4 C1 21: 72. D3, 1D
	vpxor	xmm4, xmm12, xmm13			; 0ABA _ C4 C1 19: EF. E5
	vpor	xmm11, xmm8, xmm11			; 0ABF _ C4 41 39: EB. DB
	vpxor	xmm8, xmm4, xmm5			; 0AC4 _ C5 59: EF. C5
	vpxor	xmm14, xmm8, xmm11			; 0AC8 _ C4 41 39: EF. F3
	vpxor	xmm10, xmm10, xmm11			; 0ACD _ C4 41 29: EF. D3
	vpslld	xmm4, xmm5, 3				; 0AD2 _ C5 D9: 72. F5, 03
	vpslld	xmm8, xmm14, 1				; 0AD7 _ C4 C1 39: 72. F6, 01
	vpxor	xmm13, xmm10, xmm4			; 0ADD _ C5 29: EF. EC
	vpsrld	xmm12, xmm14, 31			; 0AE1 _ C4 C1 19: 72. D6, 1F
	vpor	xmm14, xmm8, xmm12			; 0AE7 _ C4 41 39: EB. F4
	vpslld	xmm10, xmm13, 7 			; 0AEC _ C4 C1 29: 72. F5, 07
	vpsrld	xmm9, xmm13, 25 			; 0AF2 _ C4 C1 31: 72. D5, 19
	vpxor	xmm5, xmm5, xmm14			; 0AF8 _ C4 C1 51: EF. EE
	vpor	xmm8, xmm10, xmm9			; 0AFD _ C4 41 29: EB. C1
	vpxor	xmm12, xmm5, xmm8			; 0B02 _ C4 41 51: EF. E0
	vpxor	xmm11, xmm11, xmm8			; 0B07 _ C4 41 21: EF. D8
	vpslld	xmm5, xmm14, 7				; 0B0C _ C4 C1 51: 72. F6, 07
	vpslld	xmm13, xmm12, 5 			; 0B12 _ C4 C1 11: 72. F4, 05
	vpsrld	xmm10, xmm12, 27			; 0B18 _ C4 C1 29: 72. D4, 1B
	vpxor	xmm4, xmm11, xmm5			; 0B1E _ C5 A1: EF. E5
	vmovd	xmm12, dword [r12+1304H]		; 0B22 _ C4 41 79: 6E. A4 24, 00001304
	vpor	xmm9, xmm13, xmm10			; 0B2C _ C4 41 11: EB. CA
	vmovd	xmm11, dword [r12+1300H]		; 0B31 _ C4 41 79: 6E. 9C 24, 00001300
	vmovd	xmm10, dword [r12+130CH]		; 0B3B _ C4 41 79: 6E. 94 24, 0000130C
	vpshufd xmm13, xmm12, 0 			; 0B45 _ C4 41 79: 70. EC, 00
	vpshufd xmm5, xmm11, 0				; 0B4B _ C4 C1 79: 70. EB, 00
	vpxor	xmm13, xmm14, xmm13			; 0B51 _ C4 41 09: EF. ED
	vpshufd xmm11, xmm10, 0 			; 0B56 _ C4 41 79: 70. DA, 00
	vpslld	xmm14, xmm4, 22 			; 0B5C _ C5 89: 72. F4, 16
	vpsrld	xmm4, xmm4, 10				; 0B61 _ C5 D9: 72. D4, 0A
	vpxor	xmm5, xmm9, xmm5			; 0B66 _ C5 B1: EF. ED
	vpor	xmm14, xmm14, xmm4			; 0B6A _ C5 09: EB. F4
	vpxor	xmm8, xmm8, xmm11			; 0B6E _ C4 41 39: EF. C3
	vmovd	xmm4, dword [r12+1308H] 		; 0B73 _ C4 C1 79: 6E. A4 24, 00001308
	vpxor	xmm10, xmm8, xmm5			; 0B7D _ C5 39: EF. D5
	vpshufd xmm12, xmm4, 0				; 0B81 _ C5 79: 70. E4, 00
	vpand	xmm4, xmm13, xmm10			; 0B86 _ C4 C1 11: DB. E2
	vpxor	xmm9, xmm14, xmm12			; 0B8B _ C4 41 09: EF. CC
	vpxor	xmm11, xmm4, xmm5			; 0B90 _ C5 59: EF. DD
	vpxor	xmm8, xmm13, xmm9			; 0B94 _ C4 41 11: EF. C1
	vpor	xmm5, xmm5, xmm10			; 0B99 _ C4 C1 51: EB. EA
	vpxor	xmm13, xmm5, xmm8			; 0B9E _ C4 41 51: EF. E8
	vpxor	xmm5, xmm8, xmm10			; 0BA3 _ C4 C1 39: EF. EA
	vpxor	xmm12, xmm10, xmm9			; 0BA8 _ C4 41 29: EF. E1
	vpxor	xmm10, xmm5, xmm6			; 0BAD _ C5 51: EF. D6
	vpor	xmm9, xmm9, xmm11			; 0BB1 _ C4 41 31: EB. CB
	vpor	xmm8, xmm10, xmm11			; 0BB6 _ C4 41 29: EB. C3
	vpxor	xmm4, xmm11, xmm12			; 0BBB _ C4 C1 21: EF. E4
	vpxor	xmm14, xmm9, xmm5			; 0BC0 _ C5 31: EF. F5
	vpor	xmm5, xmm12, xmm13			; 0BC4 _ C4 C1 19: EB. ED
	vpxor	xmm12, xmm4, xmm8			; 0BC9 _ C4 41 59: EF. E0
	vpxor	xmm11, xmm12, xmm5			; 0BCE _ C5 19: EF. DD
	vpslld	xmm12, xmm14, 3 			; 0BD2 _ C4 C1 19: 72. F6, 03
	vpslld	xmm9, xmm11, 13 			; 0BD8 _ C4 C1 31: 72. F3, 0D
	vpsrld	xmm4, xmm11, 19 			; 0BDE _ C4 C1 59: 72. D3, 13
	vpor	xmm4, xmm9, xmm4			; 0BE4 _ C5 B1: EB. E4
	vpsrld	xmm14, xmm14, 29			; 0BE8 _ C4 C1 09: 72. D6, 1D
	vpxor	xmm8, xmm8, xmm5			; 0BEE _ C5 39: EF. C5
	vpor	xmm12, xmm12, xmm14			; 0BF2 _ C4 41 19: EB. E6
	vpxor	xmm5, xmm8, xmm4			; 0BF7 _ C5 B9: EF. EC
	vpxor	xmm13, xmm13, xmm12			; 0BFB _ C4 41 11: EF. EC
	vpxor	xmm8, xmm5, xmm12			; 0C00 _ C4 41 51: EF. C4
	vpslld	xmm5, xmm4, 3				; 0C05 _ C5 D1: 72. F4, 03
	vpxor	xmm10, xmm13, xmm5			; 0C0A _ C5 11: EF. D5
	vpslld	xmm14, xmm8, 1				; 0C0E _ C4 C1 09: 72. F0, 01
	vpsrld	xmm13, xmm8, 31 			; 0C14 _ C4 C1 11: 72. D0, 1F
	vpslld	xmm11, xmm10, 7 			; 0C1A _ C4 C1 21: 72. F2, 07
	vpor	xmm14, xmm14, xmm13			; 0C20 _ C4 41 09: EB. F5
	vpsrld	xmm9, xmm10, 25 			; 0C25 _ C4 C1 31: 72. D2, 19
	vpor	xmm5, xmm11, xmm9			; 0C2B _ C4 C1 21: EB. E9
	vpxor	xmm4, xmm4, xmm14			; 0C30 _ C4 C1 59: EF. E6
	vpxor	xmm9, xmm4, xmm5			; 0C35 _ C5 59: EF. CD
	vpxor	xmm12, xmm12, xmm5			; 0C39 _ C5 19: EF. E5
	vpslld	xmm4, xmm14, 7				; 0C3D _ C4 C1 59: 72. F6, 07
	vmovd	xmm8, dword [r12+1314H] 		; 0C43 _ C4 41 79: 6E. 84 24, 00001314
	vpxor	xmm12, xmm12, xmm4			; 0C4D _ C5 19: EF. E4
	vmovd	xmm4, dword [r12+131CH] 		; 0C51 _ C4 C1 79: 6E. A4 24, 0000131C
	vpshufd xmm13, xmm8, 0				; 0C5B _ C4 41 79: 70. E8, 00
	vpshufd xmm8, xmm4, 0				; 0C61 _ C5 79: 70. C4, 00
	vpxor	xmm14, xmm14, xmm13			; 0C66 _ C4 41 09: EF. F5
	vmovd	xmm11, dword [r12+1310H]		; 0C6B _ C4 41 79: 6E. 9C 24, 00001310
	vpxor	xmm13, xmm5, xmm8			; 0C75 _ C4 41 51: EF. E8
	vpslld	xmm5, xmm9, 5				; 0C7A _ C4 C1 51: 72. F1, 05
	vpsrld	xmm9, xmm9, 27				; 0C80 _ C4 C1 31: 72. D1, 1B
	vmovd	xmm10, dword [r12+1318H]		; 0C86 _ C4 41 79: 6E. 94 24, 00001318
	vpor	xmm5, xmm5, xmm9			; 0C90 _ C4 C1 51: EB. E9
	vpshufd xmm11, xmm11, 0 			; 0C95 _ C4 41 79: 70. DB, 00
	vpslld	xmm4, xmm12, 22 			; 0C9B _ C4 C1 59: 72. F4, 16
	vpsrld	xmm12, xmm12, 10			; 0CA1 _ C4 C1 19: 72. D4, 0A
	vpxor	xmm5, xmm5, xmm11			; 0CA7 _ C4 C1 51: EF. EB
	vpshufd xmm10, xmm10, 0 			; 0CAC _ C4 41 79: 70. D2, 00
	vpor	xmm11, xmm4, xmm12			; 0CB2 _ C4 41 59: EB. DC
	vpxor	xmm8, xmm5, xmm6			; 0CB7 _ C5 51: EF. C6
	vpxor	xmm4, xmm11, xmm10			; 0CBB _ C4 C1 21: EF. E2
	vpand	xmm5, xmm8, xmm14			; 0CC0 _ C4 C1 39: DB. EE
	vpxor	xmm12, xmm4, xmm6			; 0CC5 _ C5 59: EF. E6
	vpxor	xmm11, xmm12, xmm5			; 0CC9 _ C5 19: EF. DD
	vpor	xmm5, xmm5, xmm13			; 0CCD _ C4 C1 51: EB. ED
	vpxor	xmm4, xmm13, xmm11			; 0CD2 _ C4 C1 11: EF. E3
	vpxor	xmm14, xmm14, xmm5			; 0CD7 _ C5 09: EF. F5
	vpxor	xmm12, xmm5, xmm8			; 0CDB _ C4 41 51: EF. E0
	vpor	xmm9, xmm8, xmm14			; 0CE0 _ C4 41 39: EB. CE
	vpxor	xmm5, xmm14, xmm4			; 0CE5 _ C5 89: EF. EC
	vpor	xmm8, xmm11, xmm12			; 0CE9 _ C4 41 21: EB. C4
	vpand	xmm10, xmm8, xmm9			; 0CEE _ C4 41 39: DB. D1
	vpxor	xmm13, xmm12, xmm5			; 0CF3 _ C5 19: EF. ED
	vpslld	xmm8, xmm4, 3				; 0CF7 _ C5 B9: 72. F4, 03
	vpsrld	xmm4, xmm4, 29				; 0CFC _ C5 D9: 72. D4, 1D
	vpslld	xmm14, xmm10, 13			; 0D01 _ C4 C1 09: 72. F2, 0D
	vpsrld	xmm11, xmm10, 19			; 0D07 _ C4 C1 21: 72. D2, 13
	vpor	xmm12, xmm8, xmm4			; 0D0D _ C5 39: EB. E4
	vpand	xmm4, xmm13, xmm10			; 0D11 _ C4 C1 11: DB. E2
	vpor	xmm11, xmm14, xmm11			; 0D16 _ C4 41 09: EB. DB
	vpxor	xmm9, xmm9, xmm4			; 0D1B _ C5 31: EF. CC
	vpand	xmm10, xmm5, xmm10			; 0D1F _ C4 41 51: DB. D2
	vpxor	xmm4, xmm9, xmm11			; 0D24 _ C4 C1 31: EF. E3
	vpxor	xmm13, xmm10, xmm13			; 0D29 _ C4 41 29: EF. ED
	vpxor	xmm14, xmm4, xmm12			; 0D2E _ C4 41 59: EF. F4
	vpxor	xmm5, xmm13, xmm12			; 0D33 _ C4 C1 11: EF. EC
	vpslld	xmm4, xmm11, 3				; 0D38 _ C4 C1 59: 72. F3, 03
	vpxor	xmm13, xmm5, xmm4			; 0D3E _ C5 51: EF. EC
	vpslld	xmm8, xmm14, 1				; 0D42 _ C4 C1 39: 72. F6, 01
	vpsrld	xmm5, xmm14, 31 			; 0D48 _ C4 C1 51: 72. D6, 1F
	vpslld	xmm14, xmm13, 7 			; 0D4E _ C4 C1 09: 72. F5, 07
	vpor	xmm8, xmm8, xmm5			; 0D54 _ C5 39: EB. C5
	vpsrld	xmm10, xmm13, 25			; 0D58 _ C4 C1 29: 72. D5, 19
	vpor	xmm4, xmm14, xmm10			; 0D5E _ C4 C1 09: EB. E2
	vpxor	xmm11, xmm11, xmm8			; 0D63 _ C4 41 21: EF. D8
	vpxor	xmm14, xmm11, xmm4			; 0D68 _ C5 21: EF. F4
	vpxor	xmm12, xmm12, xmm4			; 0D6C _ C5 19: EF. E4
	vpslld	xmm9, xmm8, 7				; 0D70 _ C4 C1 31: 72. F0, 07
	vpsrld	xmm13, xmm14, 27			; 0D76 _ C4 C1 11: 72. D6, 1B
	vmovd	xmm10, dword [r12+1320H]		; 0D7C _ C4 41 79: 6E. 94 24, 00001320
	vpxor	xmm5, xmm12, xmm9			; 0D86 _ C4 C1 19: EF. E9
	vpslld	xmm12, xmm14, 5 			; 0D8B _ C4 C1 19: 72. F6, 05
	vmovd	xmm14, dword [r12+1324H]		; 0D91 _ C4 41 79: 6E. B4 24, 00001324
	vpor	xmm11, xmm12, xmm13			; 0D9B _ C4 41 19: EB. DD
	vpshufd xmm9, xmm10, 0				; 0DA0 _ C4 41 79: 70. CA, 00
	vpshufd xmm12, xmm14, 0 			; 0DA6 _ C4 41 79: 70. E6, 00
	vpxor	xmm9, xmm11, xmm9			; 0DAC _ C4 41 21: EF. C9
	vpxor	xmm11, xmm8, xmm12			; 0DB1 _ C4 41 39: EF. DC
	vpslld	xmm8, xmm5, 22				; 0DB6 _ C5 B9: 72. F5, 16
	vpsrld	xmm5, xmm5, 10				; 0DBB _ C5 D1: 72. D5, 0A
	vpor	xmm8, xmm8, xmm5			; 0DC0 _ C5 39: EB. C5
	vmovd	xmm5, dword [r12+1328H] 		; 0DC4 _ C4 C1 79: 6E. AC 24, 00001328
	vmovd	xmm12, dword [r12+132CH]		; 0DCE _ C4 41 79: 6E. A4 24, 0000132C
	vpshufd xmm14, xmm5, 0				; 0DD8 _ C5 79: 70. F5, 00
	vpshufd xmm13, xmm12, 0 			; 0DDD _ C4 41 79: 70. EC, 00
	vpxor	xmm5, xmm8, xmm14			; 0DE3 _ C4 C1 39: EF. EE
	vpxor	xmm8, xmm4, xmm13			; 0DE8 _ C4 41 59: EF. C5
	vpand	xmm4, xmm9, xmm5			; 0DED _ C5 B1: DB. E5
	vpxor	xmm10, xmm4, xmm8			; 0DF1 _ C4 41 59: EF. D0
	vpxor	xmm4, xmm5, xmm11			; 0DF6 _ C4 C1 51: EF. E3
	vpxor	xmm13, xmm4, xmm10			; 0DFB _ C4 41 59: EF. EA
	vpor	xmm14, xmm8, xmm9			; 0E00 _ C4 41 39: EB. F1
	vpxor	xmm5, xmm14, xmm11			; 0E05 _ C4 C1 09: EF. EB
	vpxor	xmm11, xmm9, xmm13			; 0E0A _ C4 41 31: EF. DD
	vpor	xmm12, xmm5, xmm11			; 0E0F _ C4 41 51: EB. E3
	vpxor	xmm12, xmm12, xmm10			; 0E14 _ C4 41 19: EF. E2
	vpand	xmm10, xmm10, xmm5			; 0E19 _ C5 29: DB. D5
	vpxor	xmm14, xmm11, xmm10			; 0E1D _ C4 41 21: EF. F2
	vpxor	xmm9, xmm5, xmm12			; 0E22 _ C4 41 51: EF. CC
	vpxor	xmm4, xmm9, xmm14			; 0E27 _ C4 C1 31: EF. E6
	vpslld	xmm5, xmm13, 13 			; 0E2C _ C4 C1 51: 72. F5, 0D
	vpsrld	xmm13, xmm13, 19			; 0E32 _ C4 C1 11: 72. D5, 13
	vpxor	xmm14, xmm14, xmm6			; 0E38 _ C5 09: EF. F6
	vpor	xmm8, xmm5, xmm13			; 0E3C _ C4 41 51: EB. C5
	vpslld	xmm5, xmm4, 3				; 0E41 _ C5 D1: 72. F4, 03
	vpsrld	xmm13, xmm4, 29 			; 0E46 _ C5 91: 72. D4, 1D
	vpxor	xmm12, xmm12, xmm8			; 0E4B _ C4 41 19: EF. E0
	vpor	xmm4, xmm5, xmm13			; 0E50 _ C4 C1 51: EB. E5
	vpslld	xmm11, xmm8, 3				; 0E55 _ C4 C1 21: 72. F0, 03
	vpxor	xmm9, xmm12, xmm4			; 0E5B _ C5 19: EF. CC
	vpxor	xmm10, xmm14, xmm4			; 0E5F _ C5 09: EF. D4
	vpxor	xmm12, xmm10, xmm11			; 0E63 _ C4 41 29: EF. E3
	vpslld	xmm5, xmm9, 1				; 0E68 _ C4 C1 51: 72. F1, 01
	vpsrld	xmm14, xmm9, 31 			; 0E6E _ C4 C1 09: 72. D1, 1F
	vpslld	xmm13, xmm12, 7 			; 0E74 _ C4 C1 11: 72. F4, 07
	vpor	xmm5, xmm5, xmm14			; 0E7A _ C4 C1 51: EB. EE
	vpsrld	xmm10, xmm12, 25			; 0E7F _ C4 C1 29: 72. D4, 19
	vpor	xmm9, xmm13, xmm10			; 0E85 _ C4 41 11: EB. CA
	vpxor	xmm8, xmm8, xmm5			; 0E8A _ C5 39: EF. C5
	vpxor	xmm14, xmm8, xmm9			; 0E8E _ C4 41 39: EF. F1
	vpxor	xmm4, xmm4, xmm9			; 0E93 _ C4 C1 59: EF. E1
	vmovd	xmm10, dword [r12+1330H]		; 0E98 _ C4 41 79: 6E. 94 24, 00001330
	vpslld	xmm8, xmm5, 7				; 0EA2 _ C5 B9: 72. F5, 07
	vpslld	xmm12, xmm14, 5 			; 0EA7 _ C4 C1 19: 72. F6, 05
	vpsrld	xmm13, xmm14, 27			; 0EAD _ C4 C1 11: 72. D6, 1B
	vmovd	xmm14, dword [r12+1334H]		; 0EB3 _ C4 41 79: 6E. B4 24, 00001334
	vpxor	xmm11, xmm4, xmm8			; 0EBD _ C4 41 59: EF. D8
	vpshufd xmm8, xmm10, 0				; 0EC2 _ C4 41 79: 70. C2, 00
	vpor	xmm4, xmm12, xmm13			; 0EC8 _ C4 C1 19: EB. E5
	vpshufd xmm12, xmm14, 0 			; 0ECD _ C4 41 79: 70. E6, 00
	vpxor	xmm10, xmm4, xmm8			; 0ED3 _ C4 41 59: EF. D0
	vpxor	xmm8, xmm5, xmm12			; 0ED8 _ C4 41 51: EF. C4
	vpslld	xmm5, xmm11, 22 			; 0EDD _ C4 C1 51: 72. F3, 16
	vpsrld	xmm11, xmm11, 10			; 0EE3 _ C4 C1 21: 72. D3, 0A
	vmovd	xmm12, dword [r12+133CH]		; 0EE9 _ C4 41 79: 6E. A4 24, 0000133C
	vpor	xmm4, xmm5, xmm11			; 0EF3 _ C4 C1 51: EB. E3
	vmovd	xmm5, dword [r12+1338H] 		; 0EF8 _ C4 C1 79: 6E. AC 24, 00001338
	vpshufd xmm14, xmm5, 0				; 0F02 _ C5 79: 70. F5, 00
	vpshufd xmm13, xmm12, 0 			; 0F07 _ C4 41 79: 70. EC, 00
	vpxor	xmm4, xmm4, xmm14			; 0F0D _ C4 C1 59: EF. E6
	vpxor	xmm9, xmm9, xmm13			; 0F12 _ C4 41 31: EF. CD
	vpand	xmm12, xmm8, xmm10			; 0F17 _ C4 41 39: DB. E2
	vpor	xmm13, xmm10, xmm9			; 0F1C _ C4 41 29: EB. E9
	vpxor	xmm14, xmm9, xmm8			; 0F21 _ C4 41 31: EF. F0
	vpxor	xmm5, xmm10, xmm4			; 0F26 _ C5 A9: EF. EC
	vpxor	xmm8, xmm4, xmm14			; 0F2A _ C4 41 59: EF. C6
	vpor	xmm4, xmm5, xmm12			; 0F2F _ C4 C1 51: EB. E4
	vpand	xmm10, xmm14, xmm13			; 0F34 _ C4 41 09: DB. D5
	vpxor	xmm5, xmm10, xmm4			; 0F39 _ C5 A9: EF. EC
	vpxor	xmm14, xmm13, xmm12			; 0F3D _ C4 41 11: EF. F4
	vpxor	xmm12, xmm12, xmm5			; 0F42 _ C5 19: EF. E5
	vpxor	xmm11, xmm14, xmm5			; 0F46 _ C5 09: EF. DD
	vpor	xmm13, xmm12, xmm14			; 0F4A _ C4 41 19: EB. EE
	vpand	xmm4, xmm4, xmm14			; 0F4F _ C4 C1 59: DB. E6
	vpxor	xmm9, xmm13, xmm8			; 0F54 _ C4 41 11: EF. C8
	vpxor	xmm8, xmm4, xmm8			; 0F59 _ C4 41 59: EF. C0
	vpor	xmm12, xmm9, xmm5			; 0F5E _ C5 31: EB. E5
	vpxor	xmm13, xmm11, xmm12			; 0F62 _ C4 41 21: EF. EC
	vpslld	xmm12, xmm5, 3				; 0F67 _ C5 99: 72. F5, 03
	vpslld	xmm10, xmm13, 13			; 0F6C _ C4 C1 29: 72. F5, 0D
	vpsrld	xmm11, xmm13, 19			; 0F72 _ C4 C1 21: 72. D5, 13
	vpor	xmm11, xmm10, xmm11			; 0F78 _ C4 41 29: EB. DB
	vpsrld	xmm5, xmm5, 29				; 0F7D _ C5 D1: 72. D5, 1D
	vpor	xmm10, xmm12, xmm5			; 0F82 _ C5 19: EB. D5
	vpxor	xmm9, xmm9, xmm11			; 0F86 _ C4 41 31: EF. CB
	vpxor	xmm5, xmm9, xmm10			; 0F8B _ C4 C1 31: EF. EA
	vpxor	xmm4, xmm8, xmm10			; 0F90 _ C4 C1 39: EF. E2
	vpslld	xmm8, xmm11, 3				; 0F95 _ C4 C1 39: 72. F3, 03
	vpslld	xmm14, xmm5, 1				; 0F9B _ C5 89: 72. F5, 01
	vpxor	xmm12, xmm4, xmm8			; 0FA0 _ C4 41 59: EF. E0
	vpsrld	xmm5, xmm5, 31				; 0FA5 _ C5 D1: 72. D5, 1F
	vpor	xmm13, xmm14, xmm5			; 0FAA _ C5 09: EB. ED
	vpslld	xmm9, xmm12, 7				; 0FAE _ C4 C1 31: 72. F4, 07
	vpsrld	xmm5, xmm12, 25 			; 0FB4 _ C4 C1 51: 72. D4, 19
	vpxor	xmm11, xmm11, xmm13			; 0FBA _ C4 41 21: EF. DD
	vpor	xmm4, xmm9, xmm5			; 0FBF _ C5 B1: EB. E5
	vpslld	xmm5, xmm13, 7				; 0FC3 _ C4 C1 51: 72. F5, 07
	vpxor	xmm8, xmm11, xmm4			; 0FC9 _ C5 21: EF. C4
	vpxor	xmm10, xmm10, xmm4			; 0FCD _ C5 29: EF. D4
	vpxor	xmm12, xmm10, xmm5			; 0FD1 _ C5 29: EF. E5
	vpslld	xmm14, xmm8, 5				; 0FD5 _ C4 C1 09: 72. F0, 05
	vmovd	xmm10, dword [r12+1340H]		; 0FDB _ C4 41 79: 6E. 94 24, 00001340
	vpsrld	xmm9, xmm8, 27				; 0FE5 _ C4 C1 31: 72. D0, 1B
	vmovd	xmm8, dword [r12+134CH] 		; 0FEB _ C4 41 79: 6E. 84 24, 0000134C
	vpor	xmm11, xmm14, xmm9			; 0FF5 _ C4 41 09: EB. D9
	vpshufd xmm5, xmm10, 0				; 0FFA _ C4 C1 79: 70. EA, 00
	vmovd	xmm10, dword [r12+1344H]		; 1000 _ C4 41 79: 6E. 94 24, 00001344
	vpxor	xmm5, xmm11, xmm5			; 100A _ C5 A1: EF. ED
	vpshufd xmm9, xmm8, 0				; 100E _ C4 41 79: 70. C8, 00
	vpxor	xmm11, xmm4, xmm9			; 1014 _ C4 41 59: EF. D9
	vpslld	xmm9, xmm12, 22 			; 1019 _ C4 C1 31: 72. F4, 16
	vpshufd xmm4, xmm10, 0				; 101F _ C4 C1 79: 70. E2, 00
	vpsrld	xmm10, xmm12, 10			; 1025 _ C4 C1 29: 72. D4, 0A
	vmovd	xmm14, dword [r12+1348H]		; 102B _ C4 41 79: 6E. B4 24, 00001348
	vpxor	xmm13, xmm13, xmm4			; 1035 _ C5 11: EF. EC
	vpxor	xmm4, xmm13, xmm11			; 1039 _ C4 C1 11: EF. E3
	vpor	xmm12, xmm9, xmm10			; 103E _ C4 41 31: EB. E2
	vpshufd xmm13, xmm14, 0 			; 1043 _ C4 41 79: 70. EE, 00
	vpxor	xmm8, xmm11, xmm6			; 1049 _ C5 21: EF. C6
	vpxor	xmm14, xmm12, xmm13			; 104D _ C4 41 19: EF. F5
	vpxor	xmm11, xmm14, xmm8			; 1052 _ C4 41 09: EF. D8
	vpxor	xmm8, xmm8, xmm5			; 1057 _ C5 39: EF. C5
	vpand	xmm9, xmm4, xmm8			; 105B _ C4 41 59: DB. C8
	vpxor	xmm4, xmm4, xmm8			; 1060 _ C4 C1 59: EF. E0
	vpxor	xmm9, xmm9, xmm11			; 1065 _ C4 41 31: EF. CB
	vpxor	xmm10, xmm5, xmm4			; 106A _ C5 51: EF. D4
	vpand	xmm5, xmm11, xmm4			; 106E _ C5 A1: DB. EC
	vpand	xmm12, xmm10, xmm9			; 1072 _ C4 41 29: DB. E1
	vpxor	xmm13, xmm5, xmm10			; 1077 _ C4 41 51: EF. EA
	vpxor	xmm14, xmm8, xmm12			; 107C _ C4 41 39: EF. F4
	vpslld	xmm5, xmm9, 13				; 1081 _ C4 C1 51: 72. F1, 0D
	vpor	xmm8, xmm12, xmm14			; 1087 _ C4 41 19: EB. C6
	vpor	xmm4, xmm4, xmm9			; 108C _ C4 C1 59: EB. E1
	vpxor	xmm11, xmm8, xmm13			; 1091 _ C4 41 39: EF. DD
	vpsrld	xmm8, xmm9, 19				; 1096 _ C4 C1 39: 72. D1, 13
	vpxor	xmm11, xmm11, xmm6			; 109C _ C5 21: EF. DE
	vpor	xmm5, xmm5, xmm8			; 10A0 _ C4 C1 51: EB. E8
	vpslld	xmm10, xmm11, 3 			; 10A5 _ C4 C1 29: 72. F3, 03
	vpsrld	xmm11, xmm11, 29			; 10AB _ C4 C1 21: 72. D3, 1D
	vpor	xmm8, xmm10, xmm11			; 10B1 _ C4 41 29: EB. C3
	vpxor	xmm9, xmm4, xmm12			; 10B6 _ C4 41 59: EF. CC
	vpand	xmm11, xmm13, xmm14			; 10BB _ C4 41 11: DB. DE
	vpxor	xmm14, xmm14, xmm8			; 10C0 _ C4 41 09: EF. F0
	vpxor	xmm4, xmm9, xmm11			; 10C5 _ C4 C1 31: EF. E3
	vpslld	xmm12, xmm5, 3				; 10CA _ C5 99: 72. F5, 03
	vpxor	xmm9, xmm4, xmm5			; 10CF _ C5 59: EF. CD
	vpxor	xmm4, xmm14, xmm12			; 10D3 _ C4 C1 09: EF. E4
	vpxor	xmm13, xmm9, xmm8			; 10D8 _ C4 41 31: EF. E8
	vpslld	xmm9, xmm4, 7				; 10DD _ C5 B1: 72. F4, 07
	vpslld	xmm10, xmm13, 1 			; 10E2 _ C4 C1 29: 72. F5, 01
	vpsrld	xmm11, xmm13, 31			; 10E8 _ C4 C1 21: 72. D5, 1F
	vpor	xmm11, xmm10, xmm11			; 10EE _ C4 41 29: EB. DB
	vpsrld	xmm14, xmm4, 25 			; 10F3 _ C5 89: 72. D4, 19
	vpor	xmm9, xmm9, xmm14			; 10F8 _ C4 41 31: EB. CE
	vpxor	xmm5, xmm5, xmm11			; 10FD _ C4 C1 51: EF. EB
	vpxor	xmm4, xmm5, xmm9			; 1102 _ C4 C1 51: EF. E1
	vpxor	xmm12, xmm8, xmm9			; 1107 _ C4 41 39: EF. E1
	vmovd	xmm8, dword [r12+1354H] 		; 110C _ C4 41 79: 6E. 84 24, 00001354
	vpslld	xmm13, xmm11, 7 			; 1116 _ C4 C1 11: 72. F3, 07
	vmovd	xmm5, dword [r12+135CH] 		; 111C _ C4 C1 79: 6E. AC 24, 0000135C
	vpxor	xmm10, xmm12, xmm13			; 1126 _ C4 41 19: EF. D5
	vpshufd xmm12, xmm8, 0				; 112B _ C4 41 79: 70. E0, 00
	vpshufd xmm8, xmm5, 0				; 1131 _ C5 79: 70. C5, 00
	vpxor	xmm13, xmm11, xmm12			; 1136 _ C4 41 21: EF. EC
	vmovd	xmm14, dword [r12+1350H]		; 113B _ C4 41 79: 6E. B4 24, 00001350
	vpxor	xmm12, xmm9, xmm8			; 1145 _ C4 41 31: EF. E0
	vpslld	xmm9, xmm4, 5				; 114A _ C5 B1: 72. F4, 05
	vpsrld	xmm4, xmm4, 27				; 114F _ C5 D9: 72. D4, 1B
	vpshufd xmm14, xmm14, 0 			; 1154 _ C4 41 79: 70. F6, 00
	vpor	xmm4, xmm9, xmm4			; 115A _ C5 B1: EB. E4
	vpxor	xmm4, xmm4, xmm14			; 115E _ C4 C1 59: EF. E6
	vpxor	xmm9, xmm13, xmm12			; 1163 _ C4 41 11: EF. CC
	vmovd	xmm11, dword [r12+1358H]		; 1168 _ C4 41 79: 6E. 9C 24, 00001358
	vpxor	xmm14, xmm4, xmm13			; 1172 _ C4 41 59: EF. F5
	vpslld	xmm13, xmm10, 22			; 1177 _ C4 C1 11: 72. F2, 16
	vpsrld	xmm10, xmm10, 10			; 117D _ C4 C1 29: 72. D2, 0A
	vpshufd xmm11, xmm11, 0 			; 1183 _ C4 41 79: 70. DB, 00
	vpxor	xmm4, xmm12, xmm6			; 1189 _ C5 99: EF. E6
	vpor	xmm12, xmm13, xmm10			; 118D _ C4 41 11: EB. E2
	vpxor	xmm11, xmm12, xmm11			; 1192 _ C4 41 19: EF. DB
	vpand	xmm12, xmm9, xmm14			; 1197 _ C4 41 31: DB. E6
	vpxor	xmm13, xmm11, xmm4			; 119C _ C5 21: EF. EC
	vpxor	xmm10, xmm12, xmm13			; 11A0 _ C4 41 19: EF. D5
	vpor	xmm12, xmm13, xmm9			; 11A5 _ C4 41 11: EB. E1
	vpand	xmm5, xmm4, xmm10			; 11AA _ C4 C1 59: DB. EA
	vpxor	xmm4, xmm9, xmm4			; 11AF _ C5 B1: EF. E4
	vpxor	xmm13, xmm5, xmm14			; 11B3 _ C4 41 51: EF. EE
	vpxor	xmm9, xmm4, xmm10			; 11B8 _ C4 41 59: EF. CA
	vpxor	xmm8, xmm9, xmm12			; 11BD _ C4 41 31: EF. C4
	vpand	xmm11, xmm14, xmm13			; 11C2 _ C4 41 09: DB. DD
	vpxor	xmm5, xmm11, xmm8			; 11C7 _ C4 C1 21: EF. E8
	vpslld	xmm4, xmm10, 13 			; 11CC _ C4 C1 59: 72. F2, 0D
	vpsrld	xmm10, xmm10, 19			; 11D2 _ C4 C1 29: 72. D2, 13
	vpxor	xmm14, xmm12, xmm14			; 11D8 _ C4 41 19: EF. F6
	vpor	xmm11, xmm4, xmm10			; 11DD _ C4 41 59: EB. DA
	vpslld	xmm4, xmm5, 3				; 11E2 _ C5 D9: 72. F5, 03
	vpsrld	xmm9, xmm5, 29				; 11E7 _ C5 B1: 72. D5, 1D
	vpor	xmm8, xmm8, xmm13			; 11EC _ C4 41 39: EB. C5
	vpxor	xmm14, xmm14, xmm6			; 11F1 _ C5 09: EF. F6
	vpor	xmm9, xmm4, xmm9			; 11F5 _ C4 41 59: EB. C9
	vpxor	xmm5, xmm13, xmm11			; 11FA _ C4 C1 11: EF. EB
	vpxor	xmm12, xmm8, xmm14			; 11FF _ C4 41 39: EF. E6
	vpxor	xmm4, xmm5, xmm9			; 1204 _ C4 C1 51: EF. E1
	vpxor	xmm13, xmm12, xmm9			; 1209 _ C4 41 19: EF. E9
	vpslld	xmm5, xmm11, 3				; 120E _ C4 C1 51: 72. F3, 03
	vpslld	xmm8, xmm4, 1				; 1214 _ C5 B9: 72. F4, 01
	vpxor	xmm10, xmm13, xmm5			; 1219 _ C5 11: EF. D5
	vpsrld	xmm4, xmm4, 31				; 121D _ C5 D9: 72. D4, 1F
	vpor	xmm12, xmm8, xmm4			; 1222 _ C5 39: EB. E4
	vpslld	xmm4, xmm10, 7				; 1226 _ C4 C1 59: 72. F2, 07
	vpsrld	xmm14, xmm10, 25			; 122C _ C4 C1 09: 72. D2, 19
	vpxor	xmm11, xmm11, xmm12			; 1232 _ C4 41 21: EF. DC
	vpor	xmm13, xmm4, xmm14			; 1237 _ C4 41 59: EB. EE
	vpxor	xmm5, xmm11, xmm13			; 123C _ C4 C1 21: EF. ED
	vpxor	xmm9, xmm9, xmm13			; 1241 _ C4 41 31: EF. CD
	vpslld	xmm11, xmm12, 7 			; 1246 _ C4 C1 21: 72. F4, 07
	vpslld	xmm4, xmm5, 5				; 124C _ C5 D9: 72. F5, 05
	vmovd	xmm14, dword [r12+1360H]		; 1251 _ C4 41 79: 6E. B4 24, 00001360
	vpxor	xmm11, xmm9, xmm11			; 125B _ C4 41 31: EF. DB
	vpsrld	xmm9, xmm5, 27				; 1260 _ C5 B1: 72. D5, 1B
	vpshufd xmm10, xmm14, 0 			; 1265 _ C4 41 79: 70. D6, 00
	vpor	xmm8, xmm4, xmm9			; 126B _ C4 41 59: EB. C1
	vmovd	xmm5, dword [r12+1368H] 		; 1270 _ C4 C1 79: 6E. AC 24, 00001368
	vpxor	xmm10, xmm8, xmm10			; 127A _ C4 41 39: EF. D2
	vmovd	xmm4, dword [r12+136CH] 		; 127F _ C4 C1 79: 6E. A4 24, 0000136C
	vpslld	xmm8, xmm11, 22 			; 1289 _ C4 C1 39: 72. F3, 16
	vpsrld	xmm11, xmm11, 10			; 128F _ C4 C1 21: 72. D3, 0A
	vpshufd xmm9, xmm4, 0				; 1295 _ C5 79: 70. CC, 00
	vpor	xmm11, xmm8, xmm11			; 129A _ C4 41 39: EB. DB
	vpshufd xmm5, xmm5, 0				; 129F _ C5 F9: 70. ED, 00
	vpxor	xmm13, xmm13, xmm9			; 12A4 _ C4 41 11: EF. E9
	vmovd	xmm14, dword [r12+1364H]		; 12A9 _ C4 41 79: 6E. B4 24, 00001364
	vpxor	xmm11, xmm11, xmm5			; 12B3 _ C5 21: EF. DD
	vpshufd xmm14, xmm14, 0 			; 12B7 _ C4 41 79: 70. F6, 00
	vpxor	xmm8, xmm11, xmm6			; 12BD _ C5 21: EF. C6
	vpxor	xmm11, xmm10, xmm13			; 12C1 _ C4 41 29: EF. DD
	vpand	xmm10, xmm13, xmm10			; 12C6 _ C4 41 11: DB. D2
	vpxor	xmm5, xmm10, xmm8			; 12CB _ C4 C1 29: EF. E8
	vpxor	xmm12, xmm12, xmm14			; 12D0 _ C4 41 19: EF. E6
	vpxor	xmm10, xmm12, xmm5			; 12D5 _ C5 19: EF. D5
	vpor	xmm4, xmm8, xmm13			; 12D9 _ C4 C1 39: EB. E5
	vpor	xmm14, xmm11, xmm10			; 12DE _ C4 41 21: EB. F2
	vpxor	xmm11, xmm4, xmm11			; 12E3 _ C4 41 59: EF. DB
	vpxor	xmm8, xmm11, xmm10			; 12E8 _ C4 41 21: EF. C2
	vpor	xmm9, xmm14, xmm5			; 12ED _ C5 09: EB. CD
	vpxor	xmm13, xmm13, xmm14			; 12F1 _ C4 41 11: EF. EE
	vpxor	xmm11, xmm9, xmm8			; 12F6 _ C4 41 31: EF. D8
	vpxor	xmm4, xmm13, xmm5			; 12FB _ C5 91: EF. E5
	vpslld	xmm9, xmm11, 13 			; 12FF _ C4 C1 31: 72. F3, 0D
	vpxor	xmm12, xmm4, xmm11			; 1305 _ C4 41 59: EF. E3
	vpsrld	xmm11, xmm11, 19			; 130A _ C4 C1 21: 72. D3, 13
	vpor	xmm9, xmm9, xmm11			; 1310 _ C4 41 31: EB. CB
	vpslld	xmm14, xmm12, 3 			; 1315 _ C4 C1 09: 72. F4, 03
	vpsrld	xmm13, xmm12, 29			; 131B _ C4 C1 11: 72. D4, 1D
	vpxor	xmm5, xmm5, xmm6			; 1321 _ C5 D1: EF. EE
	vpand	xmm12, xmm8, xmm12			; 1325 _ C4 41 39: DB. E4
	vpor	xmm4, xmm14, xmm13			; 132A _ C4 C1 09: EB. E5
	vpxor	xmm10, xmm10, xmm9			; 132F _ C4 41 29: EF. D1
	vpxor	xmm8, xmm5, xmm12			; 1334 _ C4 41 51: EF. C4
	vpxor	xmm14, xmm10, xmm4			; 1339 _ C5 29: EF. F4
	vpxor	xmm11, xmm8, xmm4			; 133D _ C5 39: EF. DC
	vpslld	xmm12, xmm9, 3				; 1341 _ C4 C1 19: 72. F1, 03
	vpslld	xmm13, xmm14, 1 			; 1347 _ C4 C1 11: 72. F6, 01
	vpxor	xmm5, xmm11, xmm12			; 134D _ C4 C1 21: EF. EC
	vpsrld	xmm14, xmm14, 31			; 1352 _ C4 C1 09: 72. D6, 1F
	vpor	xmm11, xmm13, xmm14			; 1358 _ C4 41 11: EB. DE
	vpslld	xmm8, xmm5, 7				; 135D _ C5 B9: 72. F5, 07
	vpsrld	xmm10, xmm5, 25 			; 1362 _ C5 A9: 72. D5, 19
	vpxor	xmm9, xmm9, xmm11			; 1367 _ C4 41 31: EF. CB
	vpor	xmm8, xmm8, xmm10			; 136C _ C4 41 39: EB. C2
	vpxor	xmm14, xmm9, xmm8			; 1371 _ C4 41 31: EF. F0
	vpxor	xmm4, xmm4, xmm8			; 1376 _ C4 C1 59: EF. E0
	vpslld	xmm9, xmm11, 7				; 137B _ C4 C1 31: 72. F3, 07
	vpsrld	xmm12, xmm14, 27			; 1381 _ C4 C1 19: 72. D6, 1B
	vmovd	xmm13, dword [r12+1370H]		; 1387 _ C4 41 79: 6E. AC 24, 00001370
	vpxor	xmm5, xmm4, xmm9			; 1391 _ C4 C1 59: EF. E9
	vpslld	xmm4, xmm14, 5				; 1396 _ C4 C1 59: 72. F6, 05
	vmovd	xmm14, dword [r12+1374H]		; 139C _ C4 41 79: 6E. B4 24, 00001374
	vpor	xmm10, xmm4, xmm12			; 13A6 _ C4 41 59: EB. D4
	vpshufd xmm4, xmm13, 0				; 13AB _ C4 C1 79: 70. E5, 00
	vpshufd xmm12, xmm14, 0 			; 13B1 _ C4 41 79: 70. E6, 00
	vpxor	xmm9, xmm10, xmm4			; 13B7 _ C5 29: EF. CC
	vpxor	xmm10, xmm11, xmm12			; 13BB _ C4 41 21: EF. D4
	vpslld	xmm11, xmm5, 22 			; 13C0 _ C5 A1: 72. F5, 16
	vpsrld	xmm5, xmm5, 10				; 13C5 _ C5 D1: 72. D5, 0A
	vpor	xmm4, xmm11, xmm5			; 13CA _ C5 A1: EB. E5
	vmovd	xmm11, dword [r12+1378H]		; 13CE _ C4 41 79: 6E. 9C 24, 00001378
	vmovd	xmm12, dword [r12+137CH]		; 13D8 _ C4 41 79: 6E. A4 24, 0000137C
	vpshufd xmm14, xmm11, 0 			; 13E2 _ C4 41 79: 70. F3, 00
	vpshufd xmm5, xmm12, 0				; 13E8 _ C4 C1 79: 70. EC, 00
	vpxor	xmm13, xmm4, xmm14			; 13EE _ C4 41 59: EF. EE
	vpxor	xmm5, xmm8, xmm5			; 13F3 _ C5 B9: EF. ED
	vpor	xmm8, xmm10, xmm13			; 13F7 _ C4 41 29: EB. C5
	vpxor	xmm8, xmm8, xmm5			; 13FC _ C5 39: EF. C5
	vpxor	xmm10, xmm10, xmm13			; 1400 _ C4 41 29: EF. D5
	vpxor	xmm11, xmm13, xmm8			; 1405 _ C4 41 11: EF. D8
	vpxor	xmm14, xmm10, xmm11			; 140A _ C4 41 29: EF. F3
	vpor	xmm10, xmm5, xmm10			; 140F _ C4 41 51: EB. D2
	vpor	xmm4, xmm8, xmm14			; 1414 _ C4 C1 39: EB. E6
	vpor	xmm13, xmm9, xmm14			; 1419 _ C4 41 31: EB. EE
	vpxor	xmm12, xmm4, xmm9			; 141E _ C4 41 59: EF. E1
	vpxor	xmm13, xmm13, xmm11			; 1423 _ C4 41 11: EF. EB
	vpxor	xmm12, xmm12, xmm14			; 1428 _ C4 41 19: EF. E6
	vpand	xmm9, xmm10, xmm9			; 142D _ C4 41 29: DB. C9
	vpxor	xmm11, xmm11, xmm12			; 1432 _ C4 41 21: EF. DC
	vpand	xmm4, xmm12, xmm13			; 1437 _ C4 C1 19: DB. E5
	vpxor	xmm11, xmm11, xmm6			; 143C _ C5 21: EF. DE
	vpxor	xmm4, xmm4, xmm14			; 1440 _ C4 C1 59: EF. E6
	vpor	xmm12, xmm11, xmm13			; 1445 _ C4 41 21: EB. E5
	vpxor	xmm5, xmm9, xmm8			; 144A _ C4 C1 31: EF. E8
	vpxor	xmm14, xmm14, xmm12			; 144F _ C4 41 09: EF. F4
	vpslld	xmm11, xmm14, 13			; 1454 _ C4 C1 21: 72. F6, 0D
	vpsrld	xmm14, xmm14, 19			; 145A _ C4 C1 09: 72. D6, 13
	vpor	xmm12, xmm11, xmm14			; 1460 _ C4 41 21: EB. E6
	vpslld	xmm11, xmm4, 3				; 1465 _ C5 A1: 72. F4, 03
	vpsrld	xmm4, xmm4, 29				; 146A _ C5 D9: 72. D4, 1D
	vpxor	xmm8, xmm5, xmm12			; 146F _ C4 41 51: EF. C4
	vpor	xmm11, xmm11, xmm4			; 1474 _ C5 21: EB. DC
	vpslld	xmm9, xmm12, 3				; 1478 _ C4 C1 31: 72. F4, 03
	vpxor	xmm14, xmm8, xmm11			; 147E _ C4 41 39: EF. F3
	vpxor	xmm4, xmm13, xmm11			; 1483 _ C4 C1 11: EF. E3
	vpxor	xmm8, xmm4, xmm9			; 1488 _ C4 41 59: EF. C1
	vpslld	xmm13, xmm14, 1 			; 148D _ C4 C1 11: 72. F6, 01
	vpsrld	xmm5, xmm14, 31 			; 1493 _ C4 C1 51: 72. D6, 1F
	vpslld	xmm10, xmm8, 7				; 1499 _ C4 C1 29: 72. F0, 07
	vpor	xmm9, xmm13, xmm5			; 149F _ C5 11: EB. CD
	vpsrld	xmm4, xmm8, 25				; 14A3 _ C4 C1 59: 72. D0, 19
	vpor	xmm4, xmm10, xmm4			; 14A9 _ C5 A9: EB. E4
	vpxor	xmm12, xmm12, xmm9			; 14AD _ C4 41 19: EF. E1
	vpxor	xmm12, xmm12, xmm4			; 14B2 _ C5 19: EF. E4
	vpxor	xmm11, xmm11, xmm4			; 14B6 _ C5 21: EF. DC
	vpslld	xmm14, xmm9, 7				; 14BA _ C4 C1 09: 72. F1, 07
	vpslld	xmm13, xmm12, 5 			; 14C0 _ C4 C1 11: 72. F4, 05
	vmovd	xmm8, dword [r12+1380H] 		; 14C6 _ C4 41 79: 6E. 84 24, 00001380
	vpxor	xmm11, xmm11, xmm14			; 14D0 _ C4 41 21: EF. DE
	vpsrld	xmm5, xmm12, 27 			; 14D5 _ C4 C1 51: 72. D4, 1B
	vmovd	xmm12, dword [r12+1384H]		; 14DB _ C4 41 79: 6E. A4 24, 00001384
	vpor	xmm10, xmm13, xmm5			; 14E5 _ C5 11: EB. D5
	vpshufd xmm14, xmm8, 0				; 14E9 _ C4 41 79: 70. F0, 00
	vpslld	xmm5, xmm11, 22 			; 14EF _ C4 C1 51: 72. F3, 16
	vmovd	xmm8, dword [r12+138CH] 		; 14F5 _ C4 41 79: 6E. 84 24, 0000138C
	vpsrld	xmm11, xmm11, 10			; 14FF _ C4 C1 21: 72. D3, 0A
	vpshufd xmm13, xmm12, 0 			; 1505 _ C4 41 79: 70. EC, 00
	vpxor	xmm14, xmm10, xmm14			; 150B _ C4 41 29: EF. F6
	vpshufd xmm10, xmm8, 0				; 1510 _ C4 41 79: 70. D0, 00
	vpor	xmm12, xmm5, xmm11			; 1516 _ C4 41 51: EB. E3
	vmovd	xmm11, dword [r12+1388H]		; 151B _ C4 41 79: 6E. 9C 24, 00001388
	vpxor	xmm9, xmm9, xmm13			; 1525 _ C4 41 31: EF. CD
	vpshufd xmm13, xmm11, 0 			; 152A _ C4 41 79: 70. EB, 00
	vpxor	xmm4, xmm4, xmm10			; 1530 _ C4 C1 59: EF. E2
	vpxor	xmm11, xmm12, xmm13			; 1535 _ C4 41 19: EF. DD
	vpxor	xmm8, xmm4, xmm14			; 153A _ C4 41 59: EF. C6
	vpxor	xmm12, xmm9, xmm11			; 153F _ C4 41 31: EF. E3
	vpand	xmm9, xmm9, xmm8			; 1544 _ C4 41 31: DB. C8
	vpxor	xmm10, xmm9, xmm14			; 1549 _ C4 41 31: EF. D6
	vpor	xmm4, xmm14, xmm8			; 154E _ C4 C1 09: EB. E0
	vpxor	xmm14, xmm12, xmm8			; 1553 _ C4 41 19: EF. F0
	vpxor	xmm9, xmm4, xmm12			; 1558 _ C4 41 59: EF. CC
	vpxor	xmm5, xmm8, xmm11			; 155D _ C4 C1 39: EF. EB
	vpor	xmm11, xmm11, xmm10			; 1562 _ C4 41 21: EB. DA
	vpxor	xmm8, xmm14, xmm6			; 1567 _ C5 09: EF. C6
	vpxor	xmm13, xmm11, xmm14			; 156B _ C4 41 21: EF. EE
	vpor	xmm12, xmm8, xmm10			; 1570 _ C4 41 39: EB. E2
	vpor	xmm14, xmm5, xmm9			; 1575 _ C4 41 51: EB. F1
	vpxor	xmm5, xmm10, xmm5			; 157A _ C5 A9: EF. ED
	vpslld	xmm8, xmm13, 3				; 157E _ C4 C1 39: 72. F5, 03
	vpxor	xmm10, xmm5, xmm12			; 1584 _ C4 41 51: EF. D4
	vpsrld	xmm13, xmm13, 29			; 1589 _ C4 C1 11: 72. D5, 1D
	vpxor	xmm11, xmm10, xmm14			; 158F _ C4 41 29: EF. DE
	vpxor	xmm14, xmm12, xmm14			; 1594 _ C4 41 19: EF. F6
	vpslld	xmm4, xmm11, 13 			; 1599 _ C4 C1 59: 72. F3, 0D
	vpsrld	xmm5, xmm11, 19 			; 159F _ C4 C1 51: 72. D3, 13
	vpor	xmm4, xmm4, xmm5			; 15A5 _ C5 D9: EB. E5
	vpor	xmm11, xmm8, xmm13			; 15A9 _ C4 41 39: EB. DD
	vpxor	xmm12, xmm14, xmm4			; 15AE _ C5 09: EF. E4
	vpxor	xmm9, xmm9, xmm11			; 15B2 _ C4 41 31: EF. CB
	vpxor	xmm13, xmm12, xmm11			; 15B7 _ C4 41 19: EF. EB
	vpslld	xmm14, xmm4, 3				; 15BC _ C5 89: 72. F4, 03
	vpxor	xmm5, xmm9, xmm14			; 15C1 _ C4 C1 31: EF. EE
	vpslld	xmm9, xmm13, 1				; 15C6 _ C4 C1 31: 72. F5, 01
	vpsrld	xmm12, xmm13, 31			; 15CC _ C4 C1 19: 72. D5, 1F
	vpslld	xmm8, xmm5, 7				; 15D2 _ C5 B9: 72. F5, 07
	vpor	xmm14, xmm9, xmm12			; 15D7 _ C4 41 31: EB. F4
	vpsrld	xmm10, xmm5, 25 			; 15DC _ C5 A9: 72. D5, 19
	vpor	xmm10, xmm8, xmm10			; 15E1 _ C4 41 39: EB. D2
	vpxor	xmm4, xmm4, xmm14			; 15E6 _ C4 C1 59: EF. E6
	vmovd	xmm12, dword [r12+139CH]		; 15EB _ C4 41 79: 6E. A4 24, 0000139C
	vpxor	xmm8, xmm4, xmm10			; 15F5 _ C4 41 59: EF. C2
	vpxor	xmm11, xmm11, xmm10			; 15FA _ C4 41 21: EF. DA
	vpslld	xmm4, xmm14, 7				; 15FF _ C4 C1 59: 72. F6, 07
	vpshufd xmm13, xmm12, 0 			; 1605 _ C4 41 79: 70. EC, 00
	vpxor	xmm4, xmm11, xmm4			; 160B _ C5 A1: EF. E4
	vmovd	xmm11, dword [r12+1394H]		; 160F _ C4 41 79: 6E. 9C 24, 00001394
	vpxor	xmm13, xmm10, xmm13			; 1619 _ C4 41 29: EF. ED
	vmovd	xmm5, dword [r12+1390H] 		; 161E _ C4 C1 79: 6E. AC 24, 00001390
	vpslld	xmm10, xmm8, 5				; 1628 _ C4 C1 29: 72. F0, 05
	vpshufd xmm9, xmm11, 0				; 162E _ C4 41 79: 70. CB, 00
	vpsrld	xmm8, xmm8, 27				; 1634 _ C4 C1 39: 72. D0, 1B
	vpshufd xmm5, xmm5, 0				; 163A _ C5 F9: 70. ED, 00
	vpxor	xmm14, xmm14, xmm9			; 163F _ C4 41 09: EF. F1
	vpor	xmm9, xmm10, xmm8			; 1644 _ C4 41 29: EB. C8
	vmovd	xmm11, dword [r12+1398H]		; 1649 _ C4 41 79: 6E. 9C 24, 00001398
	vpxor	xmm9, xmm9, xmm5			; 1653 _ C5 31: EF. CD
	vpslld	xmm5, xmm4, 22				; 1657 _ C5 D1: 72. F4, 16
	vpsrld	xmm4, xmm4, 10				; 165C _ C5 D9: 72. D4, 0A
	vpshufd xmm11, xmm11, 0 			; 1661 _ C4 41 79: 70. DB, 00
	vpor	xmm8, xmm5, xmm4			; 1667 _ C5 51: EB. C4
	vpxor	xmm12, xmm9, xmm6			; 166B _ C5 31: EF. E6
	vpxor	xmm10, xmm8, xmm11			; 166F _ C4 41 39: EF. D3
	vpand	xmm9, xmm12, xmm14			; 1674 _ C4 41 19: DB. CE
	vpxor	xmm11, xmm10, xmm6			; 1679 _ C5 29: EF. DE
	vpxor	xmm11, xmm11, xmm9			; 167D _ C4 41 21: EF. D9
	vpor	xmm9, xmm9, xmm13			; 1682 _ C4 41 31: EB. CD
	vpxor	xmm14, xmm14, xmm9			; 1687 _ C4 41 09: EF. F1
	vpxor	xmm9, xmm9, xmm12			; 168C _ C4 41 31: EF. CC
	vpxor	xmm4, xmm13, xmm11			; 1691 _ C4 C1 11: EF. E3
	vpor	xmm8, xmm12, xmm14			; 1696 _ C4 41 19: EB. C6
	vpor	xmm11, xmm11, xmm9			; 169B _ C4 41 21: EB. D9
	vpxor	xmm10, xmm14, xmm4			; 16A0 _ C5 09: EF. D4
	vpand	xmm5, xmm11, xmm8			; 16A4 _ C4 C1 21: DB. E8
	vpxor	xmm13, xmm9, xmm10			; 16A9 _ C4 41 31: EF. EA
	vpslld	xmm14, xmm5, 13 			; 16AE _ C5 89: 72. F5, 0D
	vpsrld	xmm12, xmm5, 19 			; 16B3 _ C5 99: 72. D5, 13
	vpslld	xmm9, xmm4, 3				; 16B8 _ C5 B1: 72. F4, 03
	vpsrld	xmm4, xmm4, 29				; 16BD _ C5 D9: 72. D4, 1D
	vpor	xmm11, xmm14, xmm12			; 16C2 _ C4 41 09: EB. DC
	vpor	xmm12, xmm9, xmm4			; 16C7 _ C5 31: EB. E4
	vpand	xmm4, xmm13, xmm5			; 16CB _ C5 91: DB. E5
	vpand	xmm5, xmm10, xmm5			; 16CF _ C5 A9: DB. ED
	vpxor	xmm8, xmm8, xmm4			; 16D3 _ C5 39: EF. C4
	vpxor	xmm13, xmm5, xmm13			; 16D7 _ C4 41 51: EF. ED
	vpxor	xmm4, xmm8, xmm11			; 16DC _ C4 C1 39: EF. E3
	vpxor	xmm10, xmm13, xmm12			; 16E1 _ C4 41 11: EF. D4
	vpxor	xmm9, xmm4, xmm12			; 16E6 _ C4 41 59: EF. CC
	vpslld	xmm4, xmm11, 3				; 16EB _ C4 C1 59: 72. F3, 03
	vpxor	xmm13, xmm10, xmm4			; 16F1 _ C5 29: EF. EC
	vpslld	xmm14, xmm9, 1				; 16F5 _ C4 C1 09: 72. F1, 01
	vpsrld	xmm9, xmm9, 31				; 16FB _ C4 C1 31: 72. D1, 1F
	vpslld	xmm5, xmm13, 7				; 1701 _ C4 C1 51: 72. F5, 07
	vpor	xmm4, xmm14, xmm9			; 1707 _ C4 C1 09: EB. E1
	vpsrld	xmm8, xmm13, 25 			; 170C _ C4 C1 39: 72. D5, 19
	vpor	xmm14, xmm5, xmm8			; 1712 _ C4 41 51: EB. F0
	vpxor	xmm11, xmm11, xmm4			; 1717 _ C5 21: EF. DC
	vpxor	xmm9, xmm11, xmm14			; 171B _ C4 41 21: EF. CE
	vpxor	xmm12, xmm12, xmm14			; 1720 _ C4 41 19: EF. E6
	vpslld	xmm11, xmm4, 7				; 1725 _ C5 A1: 72. F4, 07
	vpsrld	xmm13, xmm9, 27 			; 172A _ C4 C1 11: 72. D1, 1B
	vmovd	xmm5, dword [r12+13A0H] 		; 1730 _ C4 C1 79: 6E. AC 24, 000013A0
	vpxor	xmm10, xmm12, xmm11			; 173A _ C4 41 19: EF. D3
	vpslld	xmm12, xmm9, 5				; 173F _ C4 C1 19: 72. F1, 05
	vmovd	xmm9, dword [r12+13A4H] 		; 1745 _ C4 41 79: 6E. 8C 24, 000013A4
	vpor	xmm8, xmm12, xmm13			; 174F _ C4 41 19: EB. C5
	vpshufd xmm11, xmm5, 0				; 1754 _ C5 79: 70. DD, 00
	vpshufd xmm12, xmm9, 0				; 1759 _ C4 41 79: 70. E1, 00
	vpxor	xmm5, xmm8, xmm11			; 175F _ C4 C1 39: EF. EB
	vmovd	xmm11, dword [r12+13A8H]		; 1764 _ C4 41 79: 6E. 9C 24, 000013A8
	vpxor	xmm8, xmm4, xmm12			; 176E _ C4 41 59: EF. C4
	vpslld	xmm4, xmm10, 22 			; 1773 _ C4 C1 59: 72. F2, 16
	vpsrld	xmm10, xmm10, 10			; 1779 _ C4 C1 29: 72. D2, 0A
	vmovd	xmm12, dword [r12+13ACH]		; 177F _ C4 41 79: 6E. A4 24, 000013AC
	vpor	xmm4, xmm4, xmm10			; 1789 _ C4 C1 59: EB. E2
	vpshufd xmm9, xmm11, 0				; 178E _ C4 41 79: 70. CB, 00
	vpshufd xmm13, xmm12, 0 			; 1794 _ C4 41 79: 70. EC, 00
	vpxor	xmm10, xmm4, xmm9			; 179A _ C4 41 59: EF. D1
	vpxor	xmm4, xmm14, xmm13			; 179F _ C4 C1 09: EF. E5
	vpand	xmm14, xmm5, xmm10			; 17A4 _ C4 41 51: DB. F2
	vpxor	xmm13, xmm14, xmm4			; 17A9 _ C5 09: EF. EC
	vpxor	xmm11, xmm10, xmm8			; 17AD _ C4 41 29: EF. D8
	vpxor	xmm12, xmm11, xmm13			; 17B2 _ C4 41 21: EF. E5
	vpor	xmm9, xmm4, xmm5			; 17B7 _ C5 59: EB. CD
	vpxor	xmm14, xmm9, xmm8			; 17BB _ C4 41 31: EF. F0
	vpxor	xmm8, xmm5, xmm12			; 17C0 _ C4 41 51: EF. C4
	vpor	xmm5, xmm14, xmm8			; 17C5 _ C4 C1 09: EB. E8
	vpand	xmm11, xmm13, xmm14			; 17CA _ C4 41 11: DB. DE
	vpxor	xmm9, xmm5, xmm13			; 17CF _ C4 41 51: EF. CD
	vpxor	xmm4, xmm8, xmm11			; 17D4 _ C4 C1 39: EF. E3
	vpxor	xmm5, xmm14, xmm9			; 17D9 _ C4 C1 09: EF. E9
	vpslld	xmm8, xmm12, 13 			; 17DE _ C4 C1 39: 72. F4, 0D
	vpxor	xmm10, xmm5, xmm4			; 17E4 _ C5 51: EF. D4
	vpsrld	xmm12, xmm12, 19			; 17E8 _ C4 C1 19: 72. D4, 13
	vpor	xmm14, xmm8, xmm12			; 17EE _ C4 41 39: EB. F4
	vpslld	xmm13, xmm10, 3 			; 17F3 _ C4 C1 11: 72. F2, 03
	vpsrld	xmm11, xmm10, 29			; 17F9 _ C4 C1 21: 72. D2, 1D
	vpxor	xmm9, xmm9, xmm14			; 17FF _ C4 41 31: EF. CE
	vpor	xmm11, xmm13, xmm11			; 1804 _ C4 41 11: EB. DB
	vpxor	xmm4, xmm4, xmm6			; 1809 _ C5 D9: EF. E6
	vpxor	xmm5, xmm9, xmm11			; 180D _ C4 C1 31: EF. EB
	vpxor	xmm4, xmm4, xmm11			; 1812 _ C4 C1 59: EF. E3
	vpslld	xmm9, xmm14, 3				; 1817 _ C4 C1 31: 72. F6, 03
	vpslld	xmm8, xmm5, 1				; 181D _ C5 B9: 72. F5, 01
	vpxor	xmm12, xmm4, xmm9			; 1822 _ C4 41 59: EF. E1
	vpsrld	xmm10, xmm5, 31 			; 1827 _ C5 A9: 72. D5, 1F
	vpor	xmm4, xmm8, xmm10			; 182C _ C4 C1 39: EB. E2
	vpslld	xmm13, xmm12, 7 			; 1831 _ C4 C1 11: 72. F4, 07
	vpsrld	xmm9, xmm12, 25 			; 1837 _ C4 C1 31: 72. D4, 19
	vpxor	xmm14, xmm14, xmm4			; 183D _ C5 09: EF. F4
	vpor	xmm9, xmm13, xmm9			; 1841 _ C4 41 11: EB. C9
	vpxor	xmm8, xmm14, xmm9			; 1846 _ C4 41 09: EF. C1
	vpxor	xmm5, xmm11, xmm9			; 184B _ C4 C1 21: EF. E9
	vpslld	xmm10, xmm4, 7				; 1850 _ C5 A9: 72. F4, 07
	vpslld	xmm14, xmm8, 5				; 1855 _ C4 C1 09: 72. F0, 05
	vpxor	xmm5, xmm5, xmm10			; 185B _ C4 C1 51: EF. EA
	vpsrld	xmm12, xmm8, 27 			; 1860 _ C4 C1 19: 72. D0, 1B
	vmovd	xmm8, dword [r12+13B0H] 		; 1866 _ C4 41 79: 6E. 84 24, 000013B0
	vpor	xmm13, xmm14, xmm12			; 1870 _ C4 41 09: EB. EC
	vmovd	xmm10, dword [r12+13B4H]		; 1875 _ C4 41 79: 6E. 94 24, 000013B4
	vpslld	xmm12, xmm5, 22 			; 187F _ C5 99: 72. F5, 16
	vpshufd xmm11, xmm8, 0				; 1884 _ C4 41 79: 70. D8, 00
	vpsrld	xmm5, xmm5, 10				; 188A _ C5 D1: 72. D5, 0A
	vmovd	xmm8, dword [r12+13B8H] 		; 188F _ C4 41 79: 6E. 84 24, 000013B8
	vpxor	xmm11, xmm13, xmm11			; 1899 _ C4 41 11: EF. DB
	vpshufd xmm14, xmm10, 0 			; 189E _ C4 41 79: 70. F2, 00
	vpor	xmm13, xmm12, xmm5			; 18A4 _ C5 19: EB. ED
	vmovd	xmm10, dword [r12+13BCH]		; 18A8 _ C4 41 79: 6E. 94 24, 000013BC
	vpxor	xmm4, xmm4, xmm14			; 18B2 _ C4 C1 59: EF. E6
	vpshufd xmm5, xmm8, 0				; 18B7 _ C4 C1 79: 70. E8, 00
	vpshufd xmm14, xmm10, 0 			; 18BD _ C4 41 79: 70. F2, 00
	vpxor	xmm8, xmm13, xmm5			; 18C3 _ C5 11: EF. C5
	vpxor	xmm9, xmm9, xmm14			; 18C7 _ C4 41 31: EF. CE
	vpor	xmm13, xmm11, xmm9			; 18CC _ C4 41 21: EB. E9
	vpxor	xmm12, xmm9, xmm4			; 18D1 _ C5 31: EF. E4
	vpand	xmm9, xmm4, xmm11			; 18D5 _ C4 41 59: DB. CB
	vpxor	xmm11, xmm11, xmm8			; 18DA _ C4 41 21: EF. D8
	vpxor	xmm4, xmm8, xmm12			; 18DF _ C4 C1 39: EF. E4
	vpor	xmm8, xmm11, xmm9			; 18E4 _ C4 41 21: EB. C1
	vpand	xmm11, xmm12, xmm13			; 18E9 _ C4 41 19: DB. DD
	vpxor	xmm12, xmm13, xmm9			; 18EE _ C4 41 11: EF. E1
	vpxor	xmm14, xmm11, xmm8			; 18F3 _ C4 41 21: EF. F0
	vpand	xmm8, xmm8, xmm12			; 18F8 _ C4 41 39: DB. C4
	vpxor	xmm9, xmm9, xmm14			; 18FD _ C4 41 31: EF. CE
	vpxor	xmm5, xmm12, xmm14			; 1902 _ C4 C1 19: EF. EE
	vpor	xmm13, xmm9, xmm12			; 1907 _ C4 41 31: EB. EC
	vpxor	xmm10, xmm13, xmm4			; 190C _ C5 11: EF. D4
	vpxor	xmm4, xmm8, xmm4			; 1910 _ C5 B9: EF. E4
	vpor	xmm9, xmm10, xmm14			; 1914 _ C4 41 29: EB. CE
	vpxor	xmm13, xmm5, xmm9			; 1919 _ C4 41 51: EF. E9
	vpslld	xmm11, xmm13, 13			; 191E _ C4 C1 21: 72. F5, 0D
	vpsrld	xmm5, xmm13, 19 			; 1924 _ C4 C1 51: 72. D5, 13
	vpor	xmm9, xmm11, xmm5			; 192A _ C5 21: EB. CD
	vpslld	xmm13, xmm14, 3 			; 192E _ C4 C1 11: 72. F6, 03
	vpsrld	xmm14, xmm14, 29			; 1934 _ C4 C1 09: 72. D6, 1D
	vpxor	xmm10, xmm10, xmm9			; 193A _ C4 41 29: EF. D1
	vpor	xmm5, xmm13, xmm14			; 193F _ C4 C1 11: EB. EE
	vpxor	xmm11, xmm10, xmm5			; 1944 _ C5 29: EF. DD
	vpxor	xmm8, xmm4, xmm5			; 1948 _ C5 59: EF. C5
	vpslld	xmm4, xmm9, 3				; 194C _ C4 C1 59: 72. F1, 03
	vpslld	xmm13, xmm11, 1 			; 1952 _ C4 C1 11: 72. F3, 01
	vpxor	xmm10, xmm8, xmm4			; 1958 _ C5 39: EF. D4
	vpsrld	xmm11, xmm11, 31			; 195C _ C4 C1 21: 72. D3, 1F
	vpor	xmm13, xmm13, xmm11			; 1962 _ C4 41 11: EB. EB
	vpslld	xmm14, xmm10, 7 			; 1967 _ C4 C1 09: 72. F2, 07
	vpsrld	xmm12, xmm10, 25			; 196D _ C4 C1 19: 72. D2, 19
	vpxor	xmm9, xmm9, xmm13			; 1973 _ C4 41 31: EF. CD
	vpor	xmm4, xmm14, xmm12			; 1978 _ C4 C1 09: EB. E4
	vpslld	xmm8, xmm13, 7				; 197D _ C4 C1 39: 72. F5, 07
	vpxor	xmm9, xmm9, xmm4			; 1983 _ C5 31: EF. CC
	vpxor	xmm5, xmm5, xmm4			; 1987 _ C5 D1: EF. EC
	vpxor	xmm8, xmm5, xmm8			; 198B _ C4 41 51: EF. C0
	vpslld	xmm11, xmm9, 5				; 1990 _ C4 C1 21: 72. F1, 05
	vpsrld	xmm5, xmm9, 27				; 1996 _ C4 C1 51: 72. D1, 1B
	vmovd	xmm9, dword [r12+13CCH] 		; 199C _ C4 41 79: 6E. 8C 24, 000013CC
	vpor	xmm14, xmm11, xmm5			; 19A6 _ C5 21: EB. F5
	vmovd	xmm10, dword [r12+13C0H]		; 19AA _ C4 41 79: 6E. 94 24, 000013C0
	vmovd	xmm5, dword [r12+13C4H] 		; 19B4 _ C4 C1 79: 6E. AC 24, 000013C4
	vpshufd xmm11, xmm9, 0				; 19BE _ C4 41 79: 70. D9, 00
	vpshufd xmm12, xmm10, 0 			; 19C4 _ C4 41 79: 70. E2, 00
	vpxor	xmm9, xmm4, xmm11			; 19CA _ C4 41 59: EF. CB
	vpshufd xmm4, xmm5, 0				; 19CF _ C5 F9: 70. E5, 00
	vpxor	xmm10, xmm14, xmm12			; 19D4 _ C4 41 09: EF. D4
	vmovd	xmm12, dword [r12+13C8H]		; 19D9 _ C4 41 79: 6E. A4 24, 000013C8
	vpxor	xmm13, xmm13, xmm4			; 19E3 _ C5 11: EF. EC
	vpslld	xmm4, xmm8, 22				; 19E7 _ C4 C1 59: 72. F0, 16
	vpsrld	xmm8, xmm8, 10				; 19ED _ C4 C1 39: 72. D0, 0A
	vpshufd xmm5, xmm12, 0				; 19F3 _ C4 C1 79: 70. EC, 00
	vpxor	xmm14, xmm13, xmm9			; 19F9 _ C4 41 11: EF. F1
	vpxor	xmm11, xmm9, xmm6			; 19FE _ C5 31: EF. DE
	vpor	xmm13, xmm4, xmm8			; 1A02 _ C4 41 59: EB. E8
	vpxor	xmm12, xmm13, xmm5			; 1A07 _ C5 11: EF. E5
	vpxor	xmm13, xmm11, xmm10			; 1A0B _ C4 41 21: EF. EA
	vpxor	xmm8, xmm12, xmm11			; 1A10 _ C4 41 19: EF. C3
	vpand	xmm11, xmm14, xmm13			; 1A15 _ C4 41 09: DB. DD
	vpxor	xmm12, xmm14, xmm13			; 1A1A _ C4 41 09: EF. E5
	vpxor	xmm11, xmm11, xmm8			; 1A1F _ C4 41 21: EF. D8
	vpxor	xmm14, xmm10, xmm12			; 1A24 _ C4 41 29: EF. F4
	vpand	xmm10, xmm8, xmm12			; 1A29 _ C4 41 39: DB. D4
	vpand	xmm8, xmm14, xmm11			; 1A2E _ C4 41 09: DB. C3
	vpxor	xmm5, xmm10, xmm14			; 1A33 _ C4 C1 29: EF. EE
	vpxor	xmm9, xmm13, xmm8			; 1A38 _ C4 41 11: EF. C8
	vpslld	xmm10, xmm11, 13			; 1A3D _ C4 C1 29: 72. F3, 0D
	vpor	xmm4, xmm8, xmm9			; 1A43 _ C4 C1 39: EB. E1
	vpsrld	xmm14, xmm11, 19			; 1A48 _ C4 C1 09: 72. D3, 13
	vpxor	xmm13, xmm4, xmm5			; 1A4E _ C5 59: EF. ED
	vpor	xmm11, xmm12, xmm11			; 1A52 _ C4 41 19: EB. DB
	vpxor	xmm4, xmm13, xmm6			; 1A57 _ C5 91: EF. E6
	vpxor	xmm8, xmm11, xmm8			; 1A5B _ C4 41 21: EF. C0
	vpand	xmm5, xmm5, xmm9			; 1A60 _ C4 C1 51: DB. E9
	vpor	xmm13, xmm10, xmm14			; 1A65 _ C4 41 29: EB. EE
	vpslld	xmm10, xmm4, 3				; 1A6A _ C5 A9: 72. F4, 03
	vpsrld	xmm4, xmm4, 29				; 1A6F _ C5 D9: 72. D4, 1D
	vpxor	xmm12, xmm8, xmm5			; 1A74 _ C5 39: EF. E5
	vpor	xmm14, xmm10, xmm4			; 1A78 _ C5 29: EB. F4
	vpxor	xmm8, xmm12, xmm13			; 1A7C _ C4 41 19: EF. C5
	vpxor	xmm9, xmm9, xmm14			; 1A81 _ C4 41 31: EF. CE
	vpxor	xmm4, xmm8, xmm14			; 1A86 _ C4 C1 39: EF. E6
	vpslld	xmm8, xmm13, 3				; 1A8B _ C4 C1 39: 72. F5, 03
	vpxor	xmm10, xmm9, xmm8			; 1A91 _ C4 41 31: EF. D0
	vpslld	xmm9, xmm4, 1				; 1A96 _ C5 B1: 72. F4, 01
	vpsrld	xmm11, xmm4, 31 			; 1A9B _ C5 A1: 72. D4, 1F
	vpslld	xmm12, xmm10, 7 			; 1AA0 _ C4 C1 19: 72. F2, 07
	vpor	xmm5, xmm9, xmm11			; 1AA6 _ C4 C1 31: EB. EB
	vpsrld	xmm8, xmm10, 25 			; 1AAB _ C4 C1 39: 72. D2, 19
	vmovd	xmm9, dword [r12+13D4H] 		; 1AB1 _ C4 41 79: 6E. 8C 24, 000013D4
	vpor	xmm10, xmm12, xmm8			; 1ABB _ C4 41 19: EB. D0
	vmovd	xmm12, dword [r12+13DCH]		; 1AC0 _ C4 41 79: 6E. A4 24, 000013DC
	vpxor	xmm13, xmm13, xmm5			; 1ACA _ C5 11: EF. ED
	vpxor	xmm8, xmm13, xmm10			; 1ACE _ C4 41 11: EF. C2
	vpxor	xmm14, xmm14, xmm10			; 1AD3 _ C4 41 09: EF. F2
	vpshufd xmm13, xmm9, 0				; 1AD8 _ C4 41 79: 70. E9, 00
	vpslld	xmm4, xmm5, 7				; 1ADE _ C5 D9: 72. F5, 07
	vpshufd xmm9, xmm12, 0				; 1AE3 _ C4 41 79: 70. CC, 00
	vpxor	xmm4, xmm14, xmm4			; 1AE9 _ C5 89: EF. E4
	vmovd	xmm11, dword [r12+13D0H]		; 1AED _ C4 41 79: 6E. 9C 24, 000013D0
	vpxor	xmm14, xmm5, xmm13			; 1AF7 _ C4 41 51: EF. F5
	vpxor	xmm13, xmm10, xmm9			; 1AFC _ C4 41 29: EF. E9
	vpslld	xmm10, xmm8, 5				; 1B01 _ C4 C1 29: 72. F0, 05
	vpsrld	xmm8, xmm8, 27				; 1B07 _ C4 C1 39: 72. D0, 1B
	vpxor	xmm12, xmm14, xmm13			; 1B0D _ C4 41 09: EF. E5
	vpshufd xmm11, xmm11, 0 			; 1B12 _ C4 41 79: 70. DB, 00
	vpor	xmm9, xmm10, xmm8			; 1B18 _ C4 41 29: EB. C8
	vpxor	xmm8, xmm9, xmm11			; 1B1D _ C4 41 31: EF. C3
	vpxor	xmm9, xmm13, xmm6			; 1B22 _ C5 11: EF. CE
	vmovd	xmm5, dword [r12+13D8H] 		; 1B26 _ C4 C1 79: 6E. AC 24, 000013D8
	vpxor	xmm11, xmm8, xmm14			; 1B30 _ C4 41 39: EF. DE
	vpslld	xmm14, xmm4, 22 			; 1B35 _ C5 89: 72. F4, 16
	vpsrld	xmm8, xmm4, 10				; 1B3A _ C5 B9: 72. D4, 0A
	vpshufd xmm5, xmm5, 0				; 1B3F _ C5 F9: 70. ED, 00
	vpor	xmm4, xmm14, xmm8			; 1B44 _ C4 C1 09: EB. E0
	vpxor	xmm8, xmm4, xmm5			; 1B49 _ C5 59: EF. C5
	vpand	xmm4, xmm12, xmm11			; 1B4D _ C4 C1 19: DB. E3
	vpxor	xmm13, xmm8, xmm9			; 1B52 _ C4 41 39: EF. E9
	vpxor	xmm10, xmm4, xmm13			; 1B57 _ C4 41 59: EF. D5
	vpor	xmm14, xmm13, xmm12			; 1B5C _ C4 41 11: EB. F4
	vpand	xmm5, xmm9, xmm10			; 1B61 _ C4 C1 31: DB. EA
	vpxor	xmm9, xmm12, xmm9			; 1B66 _ C4 41 19: EF. C9
	vpxor	xmm13, xmm5, xmm11			; 1B6B _ C4 41 51: EF. EB
	vpxor	xmm12, xmm9, xmm10			; 1B70 _ C4 41 31: EF. E2
	vpxor	xmm5, xmm12, xmm14			; 1B75 _ C4 C1 19: EF. EE
	vpand	xmm8, xmm11, xmm13			; 1B7A _ C4 41 21: DB. C5
	vpslld	xmm4, xmm10, 13 			; 1B7F _ C4 C1 59: 72. F2, 0D
	vpsrld	xmm10, xmm10, 19			; 1B85 _ C4 C1 29: 72. D2, 13
	vpxor	xmm8, xmm8, xmm5			; 1B8B _ C5 39: EF. C5
	vpor	xmm12, xmm4, xmm10			; 1B8F _ C4 41 59: EB. E2
	vpxor	xmm11, xmm14, xmm11			; 1B94 _ C4 41 09: EF. DB
	vpslld	xmm4, xmm8, 3				; 1B99 _ C4 C1 59: 72. F0, 03
	vpsrld	xmm8, xmm8, 29				; 1B9F _ C4 C1 39: 72. D0, 1D
	vpxor	xmm9, xmm13, xmm12			; 1BA5 _ C4 41 11: EF. CC
	vpor	xmm13, xmm5, xmm13			; 1BAA _ C4 41 51: EB. ED
	vpxor	xmm14, xmm11, xmm6			; 1BAF _ C5 21: EF. F6
	vpor	xmm4, xmm4, xmm8			; 1BB3 _ C4 C1 59: EB. E0
	vpxor	xmm8, xmm13, xmm14			; 1BB8 _ C4 41 11: EF. C6
	vpxor	xmm9, xmm9, xmm4			; 1BBD _ C5 31: EF. CC
	vpxor	xmm13, xmm8, xmm4			; 1BC1 _ C5 39: EF. EC
	vpslld	xmm11, xmm12, 3 			; 1BC5 _ C4 C1 21: 72. F4, 03
	vpslld	xmm5, xmm9, 1				; 1BCB _ C4 C1 51: 72. F1, 01
	vpxor	xmm10, xmm13, xmm11			; 1BD1 _ C4 41 11: EF. D3
	vpsrld	xmm9, xmm9, 31				; 1BD6 _ C4 C1 31: 72. D1, 1F
	vpor	xmm9, xmm5, xmm9			; 1BDC _ C4 41 51: EB. C9
	vpslld	xmm14, xmm10, 7 			; 1BE1 _ C4 C1 09: 72. F2, 07
	vpsrld	xmm8, xmm10, 25 			; 1BE7 _ C4 C1 39: 72. D2, 19
	vpxor	xmm12, xmm12, xmm9			; 1BED _ C4 41 19: EF. E1
	vpor	xmm8, xmm14, xmm8			; 1BF2 _ C4 41 09: EB. C0
	vpslld	xmm13, xmm9, 7				; 1BF7 _ C4 C1 11: 72. F1, 07
	vpxor	xmm11, xmm12, xmm8			; 1BFD _ C4 41 19: EF. D8
	vpxor	xmm4, xmm4, xmm8			; 1C02 _ C4 C1 59: EF. E0
	vmovd	xmm10, dword [r12+13E0H]		; 1C07 _ C4 41 79: 6E. 94 24, 000013E0
	vpxor	xmm13, xmm4, xmm13			; 1C11 _ C4 41 59: EF. ED
	vpslld	xmm4, xmm11, 5				; 1C16 _ C4 C1 59: 72. F3, 05
	vpsrld	xmm5, xmm11, 27 			; 1C1C _ C4 C1 51: 72. D3, 1B
	vpshufd xmm12, xmm10, 0 			; 1C22 _ C4 41 79: 70. E2, 00
	vpor	xmm14, xmm4, xmm5			; 1C28 _ C5 59: EB. F5
	vmovd	xmm11, dword [r12+13ECH]		; 1C2C _ C4 41 79: 6E. 9C 24, 000013EC
	vpxor	xmm4, xmm14, xmm12			; 1C36 _ C4 C1 09: EF. E4
	vmovd	xmm12, dword [r12+13E8H]		; 1C3B _ C4 41 79: 6E. A4 24, 000013E8
	vpslld	xmm14, xmm13, 22			; 1C45 _ C4 C1 09: 72. F5, 16
	vpsrld	xmm13, xmm13, 10			; 1C4B _ C4 C1 11: 72. D5, 0A
	vpshufd xmm5, xmm11, 0				; 1C51 _ C4 C1 79: 70. EB, 00
	vpor	xmm13, xmm14, xmm13			; 1C57 _ C4 41 09: EB. ED
	vpshufd xmm12, xmm12, 0 			; 1C5C _ C4 41 79: 70. E4, 00
	vpxor	xmm8, xmm8, xmm5			; 1C62 _ C5 39: EF. C5
	vmovd	xmm10, dword [r12+13E4H]		; 1C66 _ C4 41 79: 6E. 94 24, 000013E4
	vpxor	xmm13, xmm13, xmm12			; 1C70 _ C4 41 11: EF. EC
	vpshufd xmm10, xmm10, 0 			; 1C75 _ C4 41 79: 70. D2, 00
	vpxor	xmm5, xmm13, xmm6			; 1C7B _ C5 91: EF. EE
	vpxor	xmm13, xmm4, xmm8			; 1C7F _ C4 41 59: EF. E8
	vpand	xmm4, xmm8, xmm4			; 1C84 _ C5 B9: DB. E4
	vpxor	xmm12, xmm4, xmm5			; 1C88 _ C5 59: EF. E5
	vpxor	xmm9, xmm9, xmm10			; 1C8C _ C4 41 31: EF. CA
	vpxor	xmm4, xmm9, xmm12			; 1C91 _ C4 C1 31: EF. E4
	vpor	xmm9, xmm5, xmm8			; 1C96 _ C4 41 51: EB. C8
	vpor	xmm14, xmm13, xmm4			; 1C9B _ C5 11: EB. F4
	vpxor	xmm13, xmm9, xmm13			; 1C9F _ C4 41 31: EF. ED
	vpxor	xmm5, xmm13, xmm4			; 1CA4 _ C5 91: EF. EC
	vpor	xmm11, xmm14, xmm12			; 1CA8 _ C4 41 09: EB. DC
	vpxor	xmm8, xmm8, xmm14			; 1CAD _ C4 41 39: EF. C6
	vpxor	xmm10, xmm11, xmm5			; 1CB2 _ C5 21: EF. D5
	vpxor	xmm14, xmm8, xmm12			; 1CB6 _ C4 41 39: EF. F4
	vpslld	xmm9, xmm10, 13 			; 1CBB _ C4 C1 31: 72. F2, 0D
	vpxor	xmm8, xmm14, xmm10			; 1CC1 _ C4 41 09: EF. C2
	vpsrld	xmm10, xmm10, 19			; 1CC6 _ C4 C1 29: 72. D2, 13
	vpor	xmm11, xmm9, xmm10			; 1CCC _ C4 41 31: EB. DA
	vpslld	xmm9, xmm8, 3				; 1CD1 _ C4 C1 31: 72. F0, 03
	vpsrld	xmm13, xmm8, 29 			; 1CD7 _ C4 C1 11: 72. D0, 1D
	vpxor	xmm12, xmm12, xmm6			; 1CDD _ C5 19: EF. E6
	vpand	xmm8, xmm5, xmm8			; 1CE1 _ C4 41 51: DB. C0
	vpor	xmm13, xmm9, xmm13			; 1CE6 _ C4 41 31: EB. ED
	vpxor	xmm4, xmm4, xmm11			; 1CEB _ C4 C1 59: EF. E3
	vpxor	xmm5, xmm12, xmm8			; 1CF0 _ C4 C1 19: EF. E8
	vpxor	xmm10, xmm4, xmm13			; 1CF5 _ C4 41 59: EF. D5
	vpxor	xmm8, xmm5, xmm13			; 1CFA _ C4 41 51: EF. C5
	vpslld	xmm4, xmm11, 3				; 1CFF _ C4 C1 59: 72. F3, 03
	vpslld	xmm9, xmm10, 1				; 1D05 _ C4 C1 31: 72. F2, 01
	vpxor	xmm14, xmm8, xmm4			; 1D0B _ C5 39: EF. F4
	vpsrld	xmm5, xmm10, 31 			; 1D0F _ C4 C1 51: 72. D2, 1F
	vpor	xmm8, xmm9, xmm5			; 1D15 _ C5 31: EB. C5
	vpslld	xmm10, xmm14, 7 			; 1D19 _ C4 C1 29: 72. F6, 07
	vpsrld	xmm12, xmm14, 25			; 1D1F _ C4 C1 19: 72. D6, 19
	vpxor	xmm11, xmm11, xmm8			; 1D25 _ C4 41 21: EF. D8
	vpor	xmm4, xmm10, xmm12			; 1D2A _ C4 C1 29: EB. E4
	vpslld	xmm9, xmm8, 7				; 1D2F _ C4 C1 31: 72. F0, 07
	vpxor	xmm11, xmm11, xmm4			; 1D35 _ C5 21: EF. DC
	vpxor	xmm13, xmm13, xmm4			; 1D39 _ C5 11: EF. EC
	vpxor	xmm14, xmm13, xmm9			; 1D3D _ C4 41 11: EF. F1
	vpslld	xmm13, xmm11, 5 			; 1D42 _ C4 C1 11: 72. F3, 05
	vpsrld	xmm5, xmm11, 27 			; 1D48 _ C4 C1 51: 72. D3, 1B
	vmovd	xmm10, dword [r12+13F0H]		; 1D4E _ C4 41 79: 6E. 94 24, 000013F0
	vpor	xmm12, xmm13, xmm5			; 1D58 _ C5 11: EB. E5
	vmovd	xmm13, dword [r12+13F4H]		; 1D5C _ C4 41 79: 6E. AC 24, 000013F4
	vpshufd xmm9, xmm10, 0				; 1D66 _ C4 41 79: 70. CA, 00
	vpshufd xmm11, xmm13, 0 			; 1D6C _ C4 41 79: 70. DD, 00
	vpxor	xmm10, xmm12, xmm9			; 1D72 _ C4 41 19: EF. D1
	vpxor	xmm9, xmm8, xmm11			; 1D77 _ C4 41 39: EF. CB
	vpslld	xmm8, xmm14, 22 			; 1D7C _ C4 C1 39: 72. F6, 16
	vpsrld	xmm14, xmm14, 10			; 1D82 _ C4 C1 09: 72. D6, 0A
	vpor	xmm13, xmm8, xmm14			; 1D88 _ C4 41 39: EB. EE
	vmovd	xmm8, dword [r12+13F8H] 		; 1D8D _ C4 41 79: 6E. 84 24, 000013F8
	vmovd	xmm5, dword [r12+13FCH] 		; 1D97 _ C4 C1 79: 6E. AC 24, 000013FC
	vpshufd xmm11, xmm8, 0				; 1DA1 _ C4 41 79: 70. D8, 00
	vpshufd xmm14, xmm5, 0				; 1DA7 _ C5 79: 70. F5, 00
	vpxor	xmm8, xmm13, xmm11			; 1DAC _ C4 41 11: EF. C3
	vpxor	xmm14, xmm4, xmm14			; 1DB1 _ C4 41 59: EF. F6
	vpor	xmm4, xmm9, xmm8			; 1DB6 _ C4 C1 31: EB. E0
	vpxor	xmm11, xmm4, xmm14			; 1DBB _ C4 41 59: EF. DE
	vpxor	xmm9, xmm9, xmm8			; 1DC0 _ C4 41 31: EF. C8
	vpxor	xmm5, xmm8, xmm11			; 1DC5 _ C4 C1 39: EF. EB
	vpxor	xmm4, xmm9, xmm5			; 1DCA _ C5 B1: EF. E5
	vpor	xmm9, xmm14, xmm9			; 1DCE _ C4 41 09: EB. C9
	vpor	xmm12, xmm10, xmm4			; 1DD3 _ C5 29: EB. E4
	vpor	xmm8, xmm11, xmm4			; 1DD7 _ C5 21: EB. C4
	vpxor	xmm13, xmm12, xmm5			; 1DDB _ C5 19: EF. ED
	vpxor	xmm12, xmm8, xmm10			; 1DDF _ C4 41 39: EF. E2
	vpxor	xmm12, xmm12, xmm4			; 1DE4 _ C5 19: EF. E4
	vpand	xmm10, xmm9, xmm10			; 1DE8 _ C4 41 31: DB. D2
	vpxor	xmm5, xmm5, xmm12			; 1DED _ C4 C1 51: EF. EC
	vpand	xmm8, xmm12, xmm13			; 1DF2 _ C4 41 19: DB. C5
	vpxor	xmm5, xmm5, xmm6			; 1DF7 _ C5 D1: EF. EE
	vpxor	xmm8, xmm8, xmm4			; 1DFB _ C5 39: EF. C4
	vpor	xmm12, xmm5, xmm13			; 1DFF _ C4 41 51: EB. E5
	vpxor	xmm4, xmm4, xmm12			; 1E04 _ C4 C1 59: EF. E4
	vpslld	xmm12, xmm8, 3				; 1E09 _ C4 C1 19: 72. F0, 03
	vpslld	xmm5, xmm4, 13				; 1E0F _ C5 D1: 72. F4, 0D
	vpsrld	xmm4, xmm4, 19				; 1E14 _ C5 D9: 72. D4, 13
	vpsrld	xmm8, xmm8, 29				; 1E19 _ C4 C1 39: 72. D0, 1D
	vpor	xmm4, xmm5, xmm4			; 1E1F _ C5 D1: EB. E4
	vpor	xmm5, xmm12, xmm8			; 1E23 _ C4 C1 19: EB. E8
	vpxor	xmm8, xmm10, xmm11			; 1E28 _ C4 41 29: EF. C3
	vpxor	xmm9, xmm8, xmm4			; 1E2D _ C5 39: EF. CC
	vpxor	xmm13, xmm13, xmm5			; 1E31 _ C5 11: EF. ED
	vpxor	xmm10, xmm9, xmm5			; 1E35 _ C5 31: EF. D5
	vpslld	xmm11, xmm4, 3				; 1E39 _ C5 A1: 72. F4, 03
	vpxor	xmm8, xmm13, xmm11			; 1E3E _ C4 41 11: EF. C3
	vpslld	xmm14, xmm10, 1 			; 1E43 _ C4 C1 09: 72. F2, 01
	vpsrld	xmm12, xmm10, 31			; 1E49 _ C4 C1 19: 72. D2, 1F
	vpslld	xmm9, xmm8, 7				; 1E4F _ C4 C1 31: 72. F0, 07
	vpor	xmm11, xmm14, xmm12			; 1E55 _ C4 41 09: EB. DC
	vpsrld	xmm13, xmm8, 25 			; 1E5A _ C4 C1 11: 72. D0, 19
	vpor	xmm13, xmm9, xmm13			; 1E60 _ C4 41 31: EB. ED
	vpxor	xmm4, xmm4, xmm11			; 1E65 _ C4 C1 59: EF. E3
	vpxor	xmm14, xmm4, xmm13			; 1E6A _ C4 41 59: EF. F5
	vpxor	xmm5, xmm5, xmm13			; 1E6F _ C4 C1 51: EF. ED
	vpslld	xmm10, xmm11, 7 			; 1E74 _ C4 C1 29: 72. F3, 07
	vpslld	xmm8, xmm14, 5				; 1E7A _ C4 C1 39: 72. F6, 05
	vpsrld	xmm9, xmm14, 27 			; 1E80 _ C4 C1 31: 72. D6, 1B
	vpxor	xmm4, xmm5, xmm10			; 1E86 _ C4 C1 51: EF. E2
	vmovd	xmm5, dword [r12+1400H] 		; 1E8B _ C4 C1 79: 6E. AC 24, 00001400
	vpor	xmm12, xmm8, xmm9			; 1E95 _ C4 41 39: EB. E1
	vmovd	xmm9, dword [r12+1404H] 		; 1E9A _ C4 41 79: 6E. 8C 24, 00001404
	vpshufd xmm8, xmm5, 0				; 1EA4 _ C5 79: 70. C5, 00
	vpshufd xmm5, xmm9, 0				; 1EA9 _ C4 C1 79: 70. E9, 00
	vpxor	xmm8, xmm12, xmm8			; 1EAF _ C4 41 19: EF. C0
	vmovd	xmm14, dword [r12+140CH]		; 1EB4 _ C4 41 79: 6E. B4 24, 0000140C
	vpxor	xmm5, xmm11, xmm5			; 1EBE _ C5 A1: EF. ED
	vpslld	xmm11, xmm4, 22 			; 1EC2 _ C5 A1: 72. F4, 16
	vpsrld	xmm4, xmm4, 10				; 1EC7 _ C5 D9: 72. D4, 0A
	vmovd	xmm9, dword [r12+1408H] 		; 1ECC _ C4 41 79: 6E. 8C 24, 00001408
	vpor	xmm11, xmm11, xmm4			; 1ED6 _ C5 21: EB. DC
	vpshufd xmm4, xmm14, 0				; 1EDA _ C4 C1 79: 70. E6, 00
	vpshufd xmm10, xmm9, 0				; 1EE0 _ C4 41 79: 70. D1, 00
	vpxor	xmm13, xmm13, xmm4			; 1EE6 _ C5 11: EF. EC
	vpxor	xmm12, xmm11, xmm10			; 1EEA _ C4 41 21: EF. E2
	vpxor	xmm14, xmm13, xmm8			; 1EEF _ C4 41 11: EF. F0
	vpxor	xmm4, xmm5, xmm12			; 1EF4 _ C4 C1 51: EF. E4
	vpand	xmm5, xmm5, xmm14			; 1EF9 _ C4 C1 51: DB. EE
	vpxor	xmm9, xmm5, xmm8			; 1EFE _ C4 41 51: EF. C8
	vpxor	xmm13, xmm4, xmm14			; 1F03 _ C4 41 59: EF. EE
	vpor	xmm8, xmm8, xmm14			; 1F08 _ C4 41 39: EB. C6
	vpxor	xmm10, xmm14, xmm12			; 1F0D _ C4 41 09: EF. D4
	vpor	xmm12, xmm12, xmm9			; 1F12 _ C4 41 19: EB. E1
	vpxor	xmm14, xmm13, xmm6			; 1F17 _ C5 11: EF. F6
	vpxor	xmm5, xmm8, xmm4			; 1F1B _ C5 B9: EF. EC
	vpxor	xmm11, xmm12, xmm13			; 1F1F _ C4 41 19: EF. DD
	vpor	xmm13, xmm14, xmm9			; 1F24 _ C4 41 09: EB. E9
	vpxor	xmm9, xmm9, xmm10			; 1F29 _ C4 41 31: EF. CA
	vpor	xmm8, xmm10, xmm5			; 1F2E _ C5 29: EB. C5
	vpxor	xmm10, xmm9, xmm13			; 1F32 _ C4 41 31: EF. D5
	vpxor	xmm4, xmm10, xmm8			; 1F37 _ C4 C1 29: EF. E0
	vpslld	xmm12, xmm11, 3 			; 1F3C _ C4 C1 19: 72. F3, 03
	vpslld	xmm9, xmm4, 13				; 1F42 _ C5 B1: 72. F4, 0D
	vpsrld	xmm10, xmm4, 19 			; 1F47 _ C5 A9: 72. D4, 13
	vpsrld	xmm11, xmm11, 29			; 1F4C _ C4 C1 21: 72. D3, 1D
	vpor	xmm4, xmm9, xmm10			; 1F52 _ C4 C1 31: EB. E2
	vpor	xmm12, xmm12, xmm11			; 1F57 _ C4 41 19: EB. E3
	vpxor	xmm8, xmm13, xmm8			; 1F5C _ C4 41 11: EF. C0
	vpxor	xmm8, xmm8, xmm4			; 1F61 _ C5 39: EF. C4
	vpxor	xmm9, xmm5, xmm12			; 1F65 _ C4 41 51: EF. CC
	vpslld	xmm13, xmm4, 3				; 1F6A _ C5 91: 72. F4, 03
	vpxor	xmm11, xmm8, xmm12			; 1F6F _ C4 41 39: EF. DC
	vpxor	xmm14, xmm9, xmm13			; 1F74 _ C4 41 31: EF. F5
	vpslld	xmm5, xmm11, 1				; 1F79 _ C4 C1 51: 72. F3, 01
	vpsrld	xmm10, xmm11, 31			; 1F7F _ C4 C1 29: 72. D3, 1F
	vpslld	xmm8, xmm14, 7				; 1F85 _ C4 C1 39: 72. F6, 07
	vpsrld	xmm13, xmm14, 25			; 1F8B _ C4 C1 11: 72. D6, 19
	vpor	xmm9, xmm5, xmm10			; 1F91 _ C4 41 51: EB. CA
	vpor	xmm8, xmm8, xmm13			; 1F96 _ C4 41 39: EB. C5
	vpxor	xmm4, xmm4, xmm9			; 1F9B _ C4 C1 59: EF. E1
	vpxor	xmm11, xmm12, xmm8			; 1FA0 _ C4 41 19: EF. D8
	vpxor	xmm14, xmm4, xmm8			; 1FA5 _ C4 41 59: EF. F0
	vmovd	xmm12, dword [r12+1414H]		; 1FAA _ C4 41 79: 6E. A4 24, 00001414
	vpslld	xmm5, xmm9, 7				; 1FB4 _ C4 C1 51: 72. F1, 07
	vpshufd xmm4, xmm12, 0				; 1FBA _ C4 C1 79: 70. E4, 00
	vpslld	xmm13, xmm14, 5 			; 1FC0 _ C4 C1 11: 72. F6, 05
	vmovd	xmm10, dword [r12+1410H]		; 1FC6 _ C4 41 79: 6E. 94 24, 00001410
	vpxor	xmm4, xmm9, xmm4			; 1FD0 _ C5 B1: EF. E4
	vmovd	xmm9, dword [r12+141CH] 		; 1FD4 _ C4 41 79: 6E. 8C 24, 0000141C
	vpsrld	xmm12, xmm14, 27			; 1FDE _ C4 C1 19: 72. D6, 1B
	vpshufd xmm9, xmm9, 0				; 1FE4 _ C4 41 79: 70. C9, 00
	vpxor	xmm5, xmm11, xmm5			; 1FEA _ C5 A1: EF. ED
	vpshufd xmm10, xmm10, 0 			; 1FEE _ C4 41 79: 70. D2, 00
	vpor	xmm14, xmm13, xmm12			; 1FF4 _ C4 41 11: EB. F4
	vmovd	xmm11, dword [r12+1418H]		; 1FF9 _ C4 41 79: 6E. 9C 24, 00001418
	vpxor	xmm8, xmm8, xmm9			; 2003 _ C4 41 39: EF. C1
	vpxor	xmm9, xmm14, xmm10			; 2008 _ C4 41 09: EF. CA
	vpslld	xmm10, xmm5, 22 			; 200D _ C5 A9: 72. F5, 16
	vpsrld	xmm5, xmm5, 10				; 2012 _ C5 D1: 72. D5, 0A
	vpxor	xmm9, xmm9, xmm6			; 2017 _ C5 31: EF. CE
	vpshufd xmm11, xmm11, 0 			; 201B _ C4 41 79: 70. DB, 00
	vpor	xmm5, xmm10, xmm5			; 2021 _ C5 A9: EB. ED
	vpxor	xmm5, xmm5, xmm11			; 2025 _ C4 C1 51: EF. EB
	vpand	xmm13, xmm9, xmm4			; 202A _ C5 31: DB. EC
	vpxor	xmm10, xmm5, xmm6			; 202E _ C5 51: EF. D6
	vpxor	xmm5, xmm10, xmm13			; 2032 _ C4 C1 29: EF. ED
	vpor	xmm10, xmm13, xmm8			; 2037 _ C4 41 11: EB. D0
	vpxor	xmm14, xmm8, xmm5			; 203C _ C5 39: EF. F5
	vpxor	xmm4, xmm4, xmm10			; 2040 _ C4 C1 59: EF. E2
	vpxor	xmm11, xmm10, xmm9			; 2045 _ C4 41 29: EF. D9
	vpor	xmm13, xmm9, xmm4			; 204A _ C5 31: EB. EC
	vpxor	xmm10, xmm4, xmm14			; 204E _ C4 41 59: EF. D6
	vpor	xmm12, xmm5, xmm11			; 2053 _ C4 41 51: EB. E3
	vpand	xmm9, xmm12, xmm13			; 2058 _ C4 41 19: DB. CD
	vpxor	xmm5, xmm11, xmm10			; 205D _ C4 C1 21: EF. EA
	vpslld	xmm8, xmm9, 13				; 2062 _ C4 C1 39: 72. F1, 0D
	vpsrld	xmm4, xmm9, 19				; 2068 _ C4 C1 59: 72. D1, 13
	vpand	xmm11, xmm5, xmm9			; 206E _ C4 41 51: DB. D9
	vpor	xmm4, xmm8, xmm4			; 2073 _ C5 B9: EB. E4
	vpslld	xmm12, xmm14, 3 			; 2077 _ C4 C1 19: 72. F6, 03
	vpsrld	xmm14, xmm14, 29			; 207D _ C4 C1 09: 72. D6, 1D
	vpxor	xmm13, xmm13, xmm11			; 2083 _ C4 41 11: EF. EB
	vpand	xmm9, xmm10, xmm9			; 2088 _ C4 41 29: DB. C9
	vpor	xmm8, xmm12, xmm14			; 208D _ C4 41 19: EB. C6
	vpxor	xmm13, xmm13, xmm4			; 2092 _ C5 11: EF. EC
	vpxor	xmm5, xmm9, xmm5			; 2096 _ C5 B1: EF. ED
	vpxor	xmm11, xmm13, xmm8			; 209A _ C4 41 11: EF. D8
	vpxor	xmm10, xmm5, xmm8			; 209F _ C4 41 51: EF. D0
	vpslld	xmm5, xmm4, 3				; 20A4 _ C5 D1: 72. F4, 03
	vpxor	xmm13, xmm10, xmm5			; 20A9 _ C5 29: EF. ED
	vpslld	xmm9, xmm11, 1				; 20AD _ C4 C1 31: 72. F3, 01
	vpsrld	xmm10, xmm11, 31			; 20B3 _ C4 C1 29: 72. D3, 1F
	vpslld	xmm14, xmm13, 7 			; 20B9 _ C4 C1 09: 72. F5, 07
	vpor	xmm12, xmm9, xmm10			; 20BF _ C4 41 31: EB. E2
	vpsrld	xmm11, xmm13, 25			; 20C4 _ C4 C1 21: 72. D5, 19
	vpor	xmm14, xmm14, xmm11			; 20CA _ C4 41 09: EB. F3
	vpxor	xmm4, xmm4, xmm12			; 20CF _ C4 C1 59: EF. E4
	vpxor	xmm9, xmm4, xmm14			; 20D4 _ C4 41 59: EF. CE
	vpxor	xmm8, xmm8, xmm14			; 20D9 _ C4 41 39: EF. C6
	vmovd	xmm11, dword [r12+1420H]		; 20DE _ C4 41 79: 6E. 9C 24, 00001420
	vpslld	xmm5, xmm12, 7				; 20E8 _ C4 C1 51: 72. F4, 07
	vpslld	xmm10, xmm9, 5				; 20EE _ C4 C1 29: 72. F1, 05
	vpsrld	xmm13, xmm9, 27 			; 20F4 _ C4 C1 11: 72. D1, 1B
	vmovd	xmm9, dword [r12+1424H] 		; 20FA _ C4 41 79: 6E. 8C 24, 00001424
	vpxor	xmm4, xmm8, xmm5			; 2104 _ C5 B9: EF. E5
	vpshufd xmm5, xmm11, 0				; 2108 _ C4 C1 79: 70. EB, 00
	vpor	xmm8, xmm10, xmm13			; 210E _ C4 41 29: EB. C5
	vpshufd xmm10, xmm9, 0				; 2113 _ C4 41 79: 70. D1, 00
	vpxor	xmm13, xmm8, xmm5			; 2119 _ C5 39: EF. ED
	vmovd	xmm5, dword [r12+1428H] 		; 211D _ C4 C1 79: 6E. AC 24, 00001428
	vpxor	xmm10, xmm12, xmm10			; 2127 _ C4 41 19: EF. D2
	vpslld	xmm12, xmm4, 22 			; 212C _ C5 99: 72. F4, 16
	vpsrld	xmm4, xmm4, 10				; 2131 _ C5 D9: 72. D4, 0A
	vpor	xmm9, xmm12, xmm4			; 2136 _ C5 19: EB. CC
	vmovd	xmm12, dword [r12+142CH]		; 213A _ C4 41 79: 6E. A4 24, 0000142C
	vpshufd xmm4, xmm5, 0				; 2144 _ C5 F9: 70. E5, 00
	vpshufd xmm11, xmm12, 0 			; 2149 _ C4 41 79: 70. DC, 00
	vpxor	xmm8, xmm9, xmm4			; 214F _ C5 31: EF. C4
	vpxor	xmm4, xmm14, xmm11			; 2153 _ C4 C1 09: EF. E3
	vpand	xmm14, xmm13, xmm8			; 2158 _ C4 41 11: DB. F0
	vpxor	xmm9, xmm14, xmm4			; 215D _ C5 09: EF. CC
	vpxor	xmm5, xmm8, xmm10			; 2161 _ C4 C1 39: EF. EA
	vpxor	xmm5, xmm5, xmm9			; 2166 _ C4 C1 51: EF. E9
	vpor	xmm14, xmm4, xmm13			; 216B _ C4 41 59: EB. F5
	vpxor	xmm4, xmm14, xmm10			; 2170 _ C4 C1 09: EF. E2
	vpxor	xmm13, xmm13, xmm5			; 2175 _ C5 11: EF. ED
	vpor	xmm10, xmm4, xmm13			; 2179 _ C4 41 59: EB. D5
	vpxor	xmm8, xmm10, xmm9			; 217E _ C4 41 29: EF. C1
	vpand	xmm9, xmm9, xmm4			; 2183 _ C5 31: DB. CC
	vpxor	xmm11, xmm13, xmm9			; 2187 _ C4 41 11: EF. D9
	vpxor	xmm10, xmm4, xmm8			; 218C _ C4 41 59: EF. D0
	vpxor	xmm9, xmm10, xmm11			; 2191 _ C4 41 29: EF. CB
	vpslld	xmm13, xmm5, 13 			; 2196 _ C5 91: 72. F5, 0D
	vpsrld	xmm5, xmm5, 19				; 219B _ C5 D1: 72. D5, 13
	vpslld	xmm14, xmm9, 3				; 21A0 _ C4 C1 09: 72. F1, 03
	vpor	xmm12, xmm13, xmm5			; 21A6 _ C5 11: EB. E5
	vpsrld	xmm5, xmm9, 29				; 21AA _ C4 C1 51: 72. D1, 1D
	vpor	xmm14, xmm14, xmm5			; 21B0 _ C5 09: EB. F5
	vpxor	xmm8, xmm8, xmm12			; 21B4 _ C4 41 39: EF. C4
	vpxor	xmm11, xmm11, xmm6			; 21B9 _ C5 21: EF. DE
	vpxor	xmm10, xmm8, xmm14			; 21BD _ C4 41 39: EF. D6
	vpxor	xmm5, xmm11, xmm14			; 21C2 _ C4 C1 21: EF. EE
	vpslld	xmm9, xmm12, 3				; 21C7 _ C4 C1 31: 72. F4, 03
	vpxor	xmm11, xmm5, xmm9			; 21CD _ C4 41 51: EF. D9
	vpslld	xmm13, xmm10, 1 			; 21D2 _ C4 C1 11: 72. F2, 01
	vpsrld	xmm4, xmm10, 31 			; 21D8 _ C4 C1 59: 72. D2, 1F
	vpslld	xmm8, xmm11, 7				; 21DE _ C4 C1 39: 72. F3, 07
	vpor	xmm4, xmm13, xmm4			; 21E4 _ C5 91: EB. E4
	vpsrld	xmm5, xmm11, 25 			; 21E8 _ C4 C1 51: 72. D3, 19
	vpor	xmm13, xmm8, xmm5			; 21EE _ C5 39: EB. ED
	vpxor	xmm12, xmm12, xmm4			; 21F2 _ C5 19: EF. E4
	vpxor	xmm10, xmm12, xmm13			; 21F6 _ C4 41 19: EF. D5
	vpxor	xmm5, xmm14, xmm13			; 21FB _ C4 C1 09: EF. ED
	vpslld	xmm14, xmm10, 5 			; 2200 _ C4 C1 09: 72. F2, 05
	vpsrld	xmm12, xmm10, 27			; 2206 _ C4 C1 19: 72. D2, 1B
	vmovd	xmm11, dword [r12+1430H]		; 220C _ C4 41 79: 6E. 9C 24, 00001430
	vpslld	xmm9, xmm4, 7				; 2216 _ C5 B1: 72. F4, 07
	vpor	xmm8, xmm14, xmm12			; 221B _ C4 41 09: EB. C4
	vpxor	xmm9, xmm5, xmm9			; 2220 _ C4 41 51: EF. C9
	vmovd	xmm14, dword [r12+1434H]		; 2225 _ C4 41 79: 6E. B4 24, 00001434
	vpshufd xmm5, xmm11, 0				; 222F _ C4 C1 79: 70. EB, 00
	vpshufd xmm12, xmm14, 0 			; 2235 _ C4 41 79: 70. E6, 00
	vpxor	xmm10, xmm8, xmm5			; 223B _ C5 39: EF. D5
	vpxor	xmm5, xmm4, xmm12			; 223F _ C4 C1 59: EF. EC
	vpslld	xmm4, xmm9, 22				; 2244 _ C4 C1 59: 72. F1, 16
	vpsrld	xmm9, xmm9, 10				; 224A _ C4 C1 31: 72. D1, 0A
	vmovd	xmm11, dword [r12+143CH]		; 2250 _ C4 41 79: 6E. 9C 24, 0000143C
	vpor	xmm14, xmm4, xmm9			; 225A _ C4 41 59: EB. F1
	vmovd	xmm4, dword [r12+1438H] 		; 225F _ C4 C1 79: 6E. A4 24, 00001438
	vpshufd xmm12, xmm4, 0				; 2269 _ C5 79: 70. E4, 00
	vpshufd xmm9, xmm11, 0				; 226E _ C4 41 79: 70. CB, 00
	vpxor	xmm8, xmm14, xmm12			; 2274 _ C4 41 09: EF. C4
	vpxor	xmm13, xmm13, xmm9			; 2279 _ C4 41 11: EF. E9
	vpand	xmm9, xmm5, xmm10			; 227E _ C4 41 51: DB. CA
	vpor	xmm11, xmm10, xmm13			; 2283 _ C4 41 29: EB. DD
	vpxor	xmm13, xmm13, xmm5			; 2288 _ C5 11: EF. ED
	vpxor	xmm10, xmm10, xmm8			; 228C _ C4 41 29: EF. D0
	vpxor	xmm5, xmm8, xmm13			; 2291 _ C4 C1 39: EF. ED
	vpor	xmm8, xmm10, xmm9			; 2296 _ C4 41 29: EB. C1
	vpand	xmm4, xmm13, xmm11			; 229B _ C4 C1 11: DB. E3
	vpxor	xmm14, xmm4, xmm8			; 22A0 _ C4 41 59: EF. F0
	vpxor	xmm12, xmm11, xmm9			; 22A5 _ C4 41 21: EF. E1
	vpxor	xmm9, xmm9, xmm14			; 22AA _ C4 41 31: EF. CE
	vpand	xmm8, xmm8, xmm12			; 22AF _ C4 41 39: DB. C4
	vpor	xmm11, xmm9, xmm12			; 22B4 _ C4 41 31: EB. DC
	vpxor	xmm9, xmm12, xmm14			; 22B9 _ C4 41 19: EF. CE
	vpxor	xmm4, xmm11, xmm5			; 22BE _ C5 A1: EF. E5
	vpxor	xmm5, xmm8, xmm5			; 22C2 _ C5 B9: EF. ED
	vpor	xmm10, xmm4, xmm14			; 22C6 _ C4 41 59: EB. D6
	vpxor	xmm13, xmm9, xmm10			; 22CB _ C4 41 31: EF. EA
	vpslld	xmm10, xmm14, 3 			; 22D0 _ C4 C1 29: 72. F6, 03
	vpslld	xmm11, xmm13, 13			; 22D6 _ C4 C1 21: 72. F5, 0D
	vpsrld	xmm9, xmm13, 19 			; 22DC _ C4 C1 31: 72. D5, 13
	vpor	xmm9, xmm11, xmm9			; 22E2 _ C4 41 21: EB. C9
	vpsrld	xmm14, xmm14, 29			; 22E7 _ C4 C1 09: 72. D6, 1D
	vpor	xmm13, xmm10, xmm14			; 22ED _ C4 41 29: EB. EE
	vpxor	xmm10, xmm4, xmm9			; 22F2 _ C4 41 59: EF. D1
	vpxor	xmm10, xmm10, xmm13			; 22F7 _ C4 41 29: EF. D5
	vpxor	xmm5, xmm5, xmm13			; 22FC _ C4 C1 51: EF. ED
	vpslld	xmm4, xmm9, 3				; 2301 _ C4 C1 59: 72. F1, 03
	vpslld	xmm14, xmm10, 1 			; 2307 _ C4 C1 09: 72. F2, 01
	vpxor	xmm12, xmm5, xmm4			; 230D _ C5 51: EF. E4
	vpsrld	xmm10, xmm10, 31			; 2311 _ C4 C1 29: 72. D2, 1F
	vpor	xmm5, xmm14, xmm10			; 2317 _ C4 C1 09: EB. EA
	vpslld	xmm11, xmm12, 7 			; 231C _ C4 C1 21: 72. F4, 07
	vpsrld	xmm8, xmm12, 25 			; 2322 _ C4 C1 39: 72. D4, 19
	vpxor	xmm9, xmm9, xmm5			; 2328 _ C5 31: EF. CD
	vpor	xmm11, xmm11, xmm8			; 232C _ C4 41 21: EB. D8
	vpxor	xmm9, xmm9, xmm11			; 2331 _ C4 41 31: EF. CB
	vpxor	xmm10, xmm13, xmm11			; 2336 _ C4 41 11: EF. D3
	vpslld	xmm13, xmm5, 7				; 233B _ C5 91: 72. F5, 07
	vmovd	xmm4, dword [r12+1440H] 		; 2340 _ C4 C1 79: 6E. A4 24, 00001440
	vpxor	xmm12, xmm10, xmm13			; 234A _ C4 41 29: EF. E5
	vpslld	xmm10, xmm9, 5				; 234F _ C4 C1 29: 72. F1, 05
	vpsrld	xmm13, xmm9, 27 			; 2355 _ C4 C1 11: 72. D1, 1B
	vmovd	xmm9, dword [r12+144CH] 		; 235B _ C4 41 79: 6E. 8C 24, 0000144C
	vpor	xmm14, xmm10, xmm13			; 2365 _ C4 41 29: EB. F5
	vpshufd xmm8, xmm4, 0				; 236A _ C5 79: 70. C4, 00
	vmovd	xmm4, dword [r12+1444H] 		; 236F _ C4 C1 79: 6E. A4 24, 00001444
	vpxor	xmm10, xmm14, xmm8			; 2379 _ C4 41 09: EF. D0
	vpshufd xmm13, xmm9, 0				; 237E _ C4 41 79: 70. E9, 00
	vpxor	xmm9, xmm11, xmm13			; 2384 _ C4 41 21: EF. CD
	vpshufd xmm11, xmm4, 0				; 2389 _ C5 79: 70. DC, 00
	vpxor	xmm13, xmm9, xmm6			; 238E _ C5 31: EF. EE
	vpxor	xmm5, xmm5, xmm11			; 2392 _ C4 C1 51: EF. EB
	vmovd	xmm14, dword [r12+1448H]		; 2397 _ C4 41 79: 6E. B4 24, 00001448
	vpxor	xmm4, xmm5, xmm9			; 23A1 _ C4 C1 51: EF. E1
	vpslld	xmm5, xmm12, 22 			; 23A6 _ C4 C1 51: 72. F4, 16
	vpsrld	xmm12, xmm12, 10			; 23AC _ C4 C1 19: 72. D4, 0A
	vpshufd xmm9, xmm14, 0				; 23B2 _ C4 41 79: 70. CE, 00
	vpor	xmm5, xmm5, xmm12			; 23B8 _ C4 C1 51: EB. EC
	vpxor	xmm14, xmm5, xmm9			; 23BD _ C4 41 51: EF. F1
	vpxor	xmm9, xmm13, xmm10			; 23C2 _ C4 41 11: EF. CA
	vpxor	xmm8, xmm14, xmm13			; 23C7 _ C4 41 09: EF. C5
	vpand	xmm13, xmm4, xmm9			; 23CC _ C4 41 59: DB. E9
	vpxor	xmm12, xmm4, xmm9			; 23D1 _ C4 41 59: EF. E1
	vpxor	xmm11, xmm13, xmm8			; 23D6 _ C4 41 11: EF. D8
	vpxor	xmm13, xmm10, xmm12			; 23DB _ C4 41 29: EF. EC
	vpand	xmm10, xmm8, xmm12			; 23E0 _ C4 41 39: DB. D4
	vpand	xmm8, xmm13, xmm11			; 23E5 _ C4 41 11: DB. C3
	vpxor	xmm5, xmm10, xmm13			; 23EA _ C4 C1 29: EF. ED
	vpxor	xmm10, xmm9, xmm8			; 23EF _ C4 41 31: EF. D0
	vpor	xmm12, xmm12, xmm11			; 23F4 _ C4 41 19: EB. E3
	vpor	xmm9, xmm8, xmm10			; 23F9 _ C4 41 39: EB. CA
	vpsrld	xmm13, xmm11, 19			; 23FE _ C4 C1 11: 72. D3, 13
	vpxor	xmm4, xmm9, xmm5			; 2404 _ C5 B1: EF. E5
	vpslld	xmm9, xmm11, 13 			; 2408 _ C4 C1 31: 72. F3, 0D
	vpxor	xmm14, xmm4, xmm6			; 240E _ C5 59: EF. F6
	vpxor	xmm11, xmm12, xmm8			; 2412 _ C4 41 19: EF. D8
	vpand	xmm5, xmm5, xmm10			; 2417 _ C4 C1 51: DB. EA
	vpor	xmm13, xmm9, xmm13			; 241C _ C4 41 31: EB. ED
	vpslld	xmm4, xmm14, 3				; 2421 _ C4 C1 59: 72. F6, 03
	vpsrld	xmm14, xmm14, 29			; 2427 _ C4 C1 09: 72. D6, 1D
	vpxor	xmm9, xmm11, xmm5			; 242D _ C5 21: EF. CD
	vpor	xmm14, xmm4, xmm14			; 2431 _ C4 41 59: EB. F6
	vpxor	xmm4, xmm9, xmm13			; 2436 _ C4 C1 31: EF. E5
	vpxor	xmm10, xmm10, xmm14			; 243B _ C4 41 29: EF. D6
	vpxor	xmm11, xmm4, xmm14			; 2440 _ C4 41 59: EF. DE
	vpslld	xmm12, xmm13, 3 			; 2445 _ C4 C1 19: 72. F5, 03
	vpxor	xmm9, xmm10, xmm12			; 244B _ C4 41 29: EF. CC
	vpslld	xmm8, xmm11, 1				; 2450 _ C4 C1 39: 72. F3, 01
	vpsrld	xmm5, xmm11, 31 			; 2456 _ C4 C1 51: 72. D3, 1F
	vpslld	xmm10, xmm9, 7				; 245C _ C4 C1 29: 72. F1, 07
	vpor	xmm4, xmm8, xmm5			; 2462 _ C5 B9: EB. E5
	vpsrld	xmm12, xmm9, 25 			; 2466 _ C4 C1 19: 72. D1, 19
	vpor	xmm9, xmm10, xmm12			; 246C _ C4 41 29: EB. CC
	vpxor	xmm13, xmm13, xmm4			; 2471 _ C5 11: EF. EC
	vpxor	xmm5, xmm13, xmm9			; 2475 _ C4 C1 11: EF. E9
	vpslld	xmm10, xmm4, 7				; 247A _ C5 A9: 72. F4, 07
	vmovd	xmm13, dword [r12+1454H]		; 247F _ C4 41 79: 6E. AC 24, 00001454
	vpxor	xmm14, xmm14, xmm9			; 2489 _ C4 41 09: EF. F1
	vpshufd xmm12, xmm13, 0 			; 248E _ C4 41 79: 70. E5, 00
	vpxor	xmm14, xmm14, xmm10			; 2494 _ C4 41 09: EF. F2
	vpxor	xmm11, xmm4, xmm12			; 2499 _ C4 41 59: EF. DC
	vmovd	xmm4, dword [r12+145CH] 		; 249E _ C4 C1 79: 6E. A4 24, 0000145C
	vpshufd xmm8, xmm4, 0				; 24A8 _ C5 79: 70. C4, 00
	vmovd	xmm10, dword [r12+1450H]		; 24AD _ C4 41 79: 6E. 94 24, 00001450
	vpxor	xmm12, xmm9, xmm8			; 24B7 _ C4 41 31: EF. E0
	vpslld	xmm9, xmm5, 5				; 24BC _ C5 B1: 72. F5, 05
	vpsrld	xmm5, xmm5, 27				; 24C1 _ C5 D1: 72. D5, 1B
	vpshufd xmm10, xmm10, 0 			; 24C6 _ C4 41 79: 70. D2, 00
	vpor	xmm4, xmm9, xmm5			; 24CC _ C5 B1: EB. E5
	vpxor	xmm8, xmm4, xmm10			; 24D0 _ C4 41 59: EF. C2
	vpxor	xmm5, xmm11, xmm12			; 24D5 _ C4 C1 21: EF. EC
	vmovd	xmm13, dword [r12+1458H]		; 24DA _ C4 41 79: 6E. AC 24, 00001458
	vpxor	xmm10, xmm8, xmm11			; 24E4 _ C4 41 39: EF. D3
	vpslld	xmm11, xmm14, 22			; 24E9 _ C4 C1 21: 72. F6, 16
	vpsrld	xmm14, xmm14, 10			; 24EF _ C4 C1 09: 72. D6, 0A
	vpshufd xmm13, xmm13, 0 			; 24F5 _ C4 41 79: 70. ED, 00
	vpor	xmm9, xmm11, xmm14			; 24FB _ C4 41 21: EB. CE
	vpxor	xmm8, xmm12, xmm6			; 2500 _ C5 19: EF. C6
	vpxor	xmm4, xmm9, xmm13			; 2504 _ C4 C1 31: EF. E5
	vpxor	xmm12, xmm4, xmm8			; 2509 _ C4 41 59: EF. E0
	vpand	xmm14, xmm5, xmm10			; 250E _ C4 41 51: DB. F2
	vpxor	xmm9, xmm14, xmm12			; 2513 _ C4 41 09: EF. CC
	vpor	xmm11, xmm12, xmm5			; 2518 _ C5 19: EB. DD
	vpand	xmm13, xmm8, xmm9			; 251C _ C4 41 39: DB. E9
	vpxor	xmm5, xmm5, xmm8			; 2521 _ C4 C1 51: EF. E8
	vpxor	xmm12, xmm13, xmm10			; 2526 _ C4 41 11: EF. E2
	vpxor	xmm8, xmm5, xmm9			; 252B _ C4 41 51: EF. C1
	vpxor	xmm4, xmm8, xmm11			; 2530 _ C4 C1 39: EF. E3
	vpand	xmm5, xmm10, xmm12			; 2535 _ C4 C1 29: DB. EC
	vpxor	xmm13, xmm5, xmm4			; 253A _ C5 51: EF. EC
	vpslld	xmm14, xmm9, 13 			; 253E _ C4 C1 09: 72. F1, 0D
	vpsrld	xmm9, xmm9, 19				; 2544 _ C4 C1 31: 72. D1, 13
	vpxor	xmm10, xmm11, xmm10			; 254A _ C4 41 21: EF. D2
	vpor	xmm9, xmm14, xmm9			; 254F _ C4 41 09: EB. C9
	vpslld	xmm8, xmm13, 3				; 2554 _ C4 C1 39: 72. F5, 03
	vpsrld	xmm13, xmm13, 29			; 255A _ C4 C1 11: 72. D5, 1D
	vpor	xmm4, xmm4, xmm12			; 2560 _ C4 C1 59: EB. E4
	vpxor	xmm11, xmm10, xmm6			; 2565 _ C5 29: EF. DE
	vpor	xmm5, xmm8, xmm13			; 2569 _ C4 C1 39: EB. ED
	vpxor	xmm13, xmm12, xmm9			; 256E _ C4 41 19: EF. E9
	vpxor	xmm10, xmm4, xmm11			; 2573 _ C4 41 59: EF. D3
	vpxor	xmm8, xmm13, xmm5			; 2578 _ C5 11: EF. C5
	vpxor	xmm13, xmm10, xmm5			; 257C _ C5 29: EF. ED
	vpslld	xmm4, xmm9, 3				; 2580 _ C4 C1 59: 72. F1, 03
	vpslld	xmm14, xmm8, 1				; 2586 _ C4 C1 09: 72. F0, 01
	vpxor	xmm11, xmm13, xmm4			; 258C _ C5 11: EF. DC
	vpsrld	xmm12, xmm8, 31 			; 2590 _ C4 C1 19: 72. D0, 1F
	vpor	xmm8, xmm14, xmm12			; 2596 _ C4 41 09: EB. C4
	vpslld	xmm10, xmm11, 7 			; 259B _ C4 C1 29: 72. F3, 07
	vpsrld	xmm13, xmm11, 25			; 25A1 _ C4 C1 11: 72. D3, 19
	vpxor	xmm9, xmm9, xmm8			; 25A7 _ C4 41 31: EF. C8
	vpor	xmm4, xmm10, xmm13			; 25AC _ C4 C1 29: EB. E5
	vpxor	xmm13, xmm9, xmm4			; 25B1 _ C5 31: EF. EC
	vpxor	xmm5, xmm5, xmm4			; 25B5 _ C5 D1: EF. EC
	vpslld	xmm9, xmm8, 7				; 25B9 _ C4 C1 31: 72. F0, 07
	vpsrld	xmm14, xmm13, 27			; 25BF _ C4 C1 09: 72. D5, 1B
	vmovd	xmm12, dword [r12+1460H]		; 25C5 _ C4 41 79: 6E. A4 24, 00001460
	vpxor	xmm10, xmm5, xmm9			; 25CF _ C4 41 51: EF. D1
	vpslld	xmm5, xmm13, 5				; 25D4 _ C4 C1 51: 72. F5, 05
	vpor	xmm11, xmm5, xmm14			; 25DA _ C4 41 51: EB. DE
	vpshufd xmm5, xmm12, 0				; 25DF _ C4 C1 79: 70. EC, 00
	vmovd	xmm9, dword [r12+1468H] 		; 25E5 _ C4 41 79: 6E. 8C 24, 00001468
	vpxor	xmm14, xmm11, xmm5			; 25EF _ C5 21: EF. F5
	vmovd	xmm13, dword [r12+146CH]		; 25F3 _ C4 41 79: 6E. AC 24, 0000146C
	vpslld	xmm11, xmm10, 22			; 25FD _ C4 C1 21: 72. F2, 16
	vpsrld	xmm10, xmm10, 10			; 2603 _ C4 C1 29: 72. D2, 0A
	vpshufd xmm12, xmm13, 0 			; 2609 _ C4 41 79: 70. E5, 00
	vpor	xmm10, xmm11, xmm10			; 260F _ C4 41 21: EB. D2
	vpshufd xmm9, xmm9, 0				; 2614 _ C4 41 79: 70. C9, 00
	vpxor	xmm4, xmm4, xmm12			; 261A _ C4 C1 59: EF. E4
	vmovd	xmm5, dword [r12+1464H] 		; 261F _ C4 C1 79: 6E. AC 24, 00001464
	vpxor	xmm13, xmm10, xmm9			; 2629 _ C4 41 29: EF. E9
	vpshufd xmm5, xmm5, 0				; 262E _ C5 F9: 70. ED, 00
	vpxor	xmm10, xmm13, xmm6			; 2633 _ C5 11: EF. D6
	vpxor	xmm13, xmm14, xmm4			; 2637 _ C5 09: EF. EC
	vpand	xmm14, xmm4, xmm14			; 263B _ C4 41 59: DB. F6
	vpxor	xmm9, xmm14, xmm10			; 2640 _ C4 41 09: EF. CA
	vpxor	xmm8, xmm8, xmm5			; 2645 _ C5 39: EF. C5
	vpxor	xmm8, xmm8, xmm9			; 2649 _ C4 41 39: EF. C1
	vpor	xmm12, xmm10, xmm4			; 264E _ C5 29: EB. E4
	vpor	xmm11, xmm13, xmm8			; 2652 _ C4 41 11: EB. D8
	vpxor	xmm5, xmm12, xmm13			; 2657 _ C4 C1 19: EF. ED
	vpxor	xmm5, xmm5, xmm8			; 265C _ C4 C1 51: EF. E8
	vpor	xmm10, xmm11, xmm9			; 2661 _ C4 41 21: EB. D1
	vpxor	xmm4, xmm4, xmm11			; 2666 _ C4 C1 59: EF. E3
	vpxor	xmm12, xmm10, xmm5			; 266B _ C5 29: EF. E5
	vpxor	xmm10, xmm4, xmm9			; 266F _ C4 41 59: EF. D1
	vpslld	xmm13, xmm12, 13			; 2674 _ C4 C1 11: 72. F4, 0D
	vpxor	xmm14, xmm10, xmm12			; 267A _ C4 41 29: EF. F4
	vpsrld	xmm4, xmm12, 19 			; 267F _ C4 C1 59: 72. D4, 13
	vpor	xmm4, xmm13, xmm4			; 2685 _ C5 91: EB. E4
	vpslld	xmm11, xmm14, 3 			; 2689 _ C4 C1 21: 72. F6, 03
	vpsrld	xmm10, xmm14, 29			; 268F _ C4 C1 29: 72. D6, 1D
	vpxor	xmm9, xmm9, xmm6			; 2695 _ C5 31: EF. CE
	vpand	xmm14, xmm5, xmm14			; 2699 _ C4 41 51: DB. F6
	vpor	xmm13, xmm11, xmm10			; 269E _ C4 41 21: EB. EA
	vpxor	xmm8, xmm8, xmm4			; 26A3 _ C5 39: EF. C4
	vpxor	xmm5, xmm9, xmm14			; 26A7 _ C4 C1 31: EF. EE
	vpxor	xmm10, xmm8, xmm13			; 26AC _ C4 41 39: EF. D5
	vpxor	xmm9, xmm5, xmm13			; 26B1 _ C4 41 51: EF. CD
	vpslld	xmm14, xmm4, 3				; 26B6 _ C5 89: 72. F4, 03
	vpslld	xmm12, xmm10, 1 			; 26BB _ C4 C1 19: 72. F2, 01
	vpxor	xmm11, xmm9, xmm14			; 26C1 _ C4 41 31: EF. DE
	vpsrld	xmm10, xmm10, 31			; 26C6 _ C4 C1 29: 72. D2, 1F
	vpor	xmm5, xmm12, xmm10			; 26CC _ C4 C1 19: EB. EA
	vpslld	xmm8, xmm11, 7				; 26D1 _ C4 C1 39: 72. F3, 07
	vpsrld	xmm9, xmm11, 25 			; 26D7 _ C4 C1 31: 72. D3, 19
	vpxor	xmm4, xmm4, xmm5			; 26DD _ C5 D9: EF. E5
	vpor	xmm12, xmm8, xmm9			; 26E1 _ C4 41 39: EB. E1
	vpslld	xmm9, xmm5, 7				; 26E6 _ C5 B1: 72. F5, 07
	vpxor	xmm10, xmm4, xmm12			; 26EB _ C4 41 59: EF. D4
	vpxor	xmm13, xmm13, xmm12			; 26F0 _ C4 41 11: EF. EC
	vpxor	xmm4, xmm13, xmm9			; 26F5 _ C4 C1 11: EF. E1
	vpslld	xmm13, xmm10, 5 			; 26FA _ C4 C1 11: 72. F2, 05
	vpsrld	xmm14, xmm10, 27			; 2700 _ C4 C1 09: 72. D2, 1B
	vmovd	xmm11, dword [r12+1470H]		; 2706 _ C4 41 79: 6E. 9C 24, 00001470
	vpor	xmm8, xmm13, xmm14			; 2710 _ C4 41 11: EB. C6
	vmovd	xmm13, dword [r12+1474H]		; 2715 _ C4 41 79: 6E. AC 24, 00001474
	vpshufd xmm9, xmm11, 0				; 271F _ C4 41 79: 70. CB, 00
	vpshufd xmm14, xmm13, 0 			; 2725 _ C4 41 79: 70. F5, 00
	vpxor	xmm10, xmm8, xmm9			; 272B _ C4 41 39: EF. D1
	vpxor	xmm11, xmm5, xmm14			; 2730 _ C4 41 51: EF. DE
	vpslld	xmm5, xmm4, 22				; 2735 _ C5 D1: 72. F4, 16
	vpsrld	xmm9, xmm4, 10				; 273A _ C5 B1: 72. D4, 0A
	vpor	xmm13, xmm5, xmm9			; 273F _ C4 41 51: EB. E9
	vmovd	xmm5, dword [r12+1478H] 		; 2744 _ C4 C1 79: 6E. AC 24, 00001478
	vmovd	xmm8, dword [r12+147CH] 		; 274E _ C4 41 79: 6E. 84 24, 0000147C
	vpshufd xmm4, xmm5, 0				; 2758 _ C5 F9: 70. E5, 00
	vpshufd xmm5, xmm8, 0				; 275D _ C4 C1 79: 70. E8, 00
	vpxor	xmm14, xmm13, xmm4			; 2763 _ C5 11: EF. F4
	vpxor	xmm12, xmm12, xmm5			; 2767 _ C5 19: EF. E5
	vpor	xmm9, xmm11, xmm14			; 276B _ C4 41 21: EB. CE
	vpxor	xmm4, xmm9, xmm12			; 2770 _ C4 C1 31: EF. E4
	vpxor	xmm11, xmm11, xmm14			; 2775 _ C4 41 21: EF. DE
	vpxor	xmm9, xmm14, xmm4			; 277A _ C5 09: EF. CC
	vpor	xmm12, xmm12, xmm11			; 277E _ C4 41 19: EB. E3
	vpxor	xmm13, xmm11, xmm9			; 2783 _ C4 41 21: EF. E9
	vpor	xmm14, xmm10, xmm13			; 2788 _ C4 41 29: EB. F5
	vpor	xmm8, xmm4, xmm13			; 278D _ C4 41 59: EB. C5
	vpxor	xmm5, xmm14, xmm9			; 2792 _ C4 C1 09: EF. E9
	vpxor	xmm14, xmm8, xmm10			; 2797 _ C4 41 39: EF. F2
	vpxor	xmm14, xmm14, xmm13			; 279C _ C4 41 09: EF. F5
	vpand	xmm10, xmm12, xmm10			; 27A1 _ C4 41 19: DB. D2
	vpxor	xmm9, xmm9, xmm14			; 27A6 _ C4 41 31: EF. CE
	vpand	xmm12, xmm14, xmm5			; 27AB _ C5 09: DB. E5
	vpxor	xmm8, xmm9, xmm6			; 27AF _ C5 31: EF. C6
	vpxor	xmm14, xmm12, xmm13			; 27B3 _ C4 41 19: EF. F5
	vpor	xmm9, xmm8, xmm5			; 27B8 _ C5 39: EB. CD
	vpxor	xmm8, xmm13, xmm9			; 27BC _ C4 41 11: EF. C1
	vmovd	xmm9, dword [r12+1480H] 		; 27C1 _ C4 41 79: 6E. 8C 24, 00001480
	vpshufd xmm9, xmm9, 0				; 27CB _ C4 41 79: 70. C9, 00
	vpxor	xmm9, xmm8, xmm9			; 27D1 _ C4 41 39: EF. C9
	vpxor	xmm8, xmm10, xmm4			; 27D6 _ C5 29: EF. C4
	vmovd	xmm4, dword [r12+1484H] 		; 27DA _ C4 C1 79: 6E. A4 24, 00001484
	vmovd	xmm13, dword [r12+1488H]		; 27E4 _ C4 41 79: 6E. AC 24, 00001488
	vpshufd xmm11, xmm4, 0				; 27EE _ C5 79: 70. DC, 00
	vmovd	xmm4, dword [r12+148CH] 		; 27F3 _ C4 C1 79: 6E. A4 24, 0000148C
	vpxor	xmm10, xmm8, xmm11			; 27FD _ C4 41 39: EF. D3
	vpshufd xmm13, xmm13, 0 			; 2802 _ C4 41 79: 70. ED, 00
	vpshufd xmm8, xmm4, 0				; 2808 _ C5 79: 70. C4, 00
	vpxor	xmm13, xmm14, xmm13			; 280D _ C4 41 09: EF. ED
	vpxor	xmm11, xmm5, xmm8			; 2812 _ C4 41 51: EF. D8
	vpunpckldq xmm4, xmm9, xmm10			; 2817 _ C4 C1 31: 62. E2
	vpunpckldq xmm5, xmm13, xmm11			; 281C _ C4 C1 11: 62. EB
	vpunpckhdq xmm9, xmm9, xmm10			; 2821 _ C4 41 31: 6A. CA
	vpunpckhdq xmm8, xmm13, xmm11			; 2826 _ C4 41 11: 6A. C3
	vpunpcklqdq xmm10, xmm4, xmm5			; 282B _ C5 59: 6C. D5
	vpsllq	xmm14, xmm0, 1				; 282F _ C5 89: 73. F0, 01
	vpunpckhqdq xmm4, xmm4, xmm5			; 2834 _ C5 D9: 6D. E5
	vpxor	xmm3, xmm10, xmm3			; 2838 _ C5 A9: EF. DB
	vpslldq xmm12, xmm0, 8				; 283C _ C5 99: 73. F8, 08
	vpxor	xmm2, xmm4, xmm2			; 2841 _ C5 D9: EF. D2
	vpsrldq xmm13, xmm12, 7 			; 2845 _ C4 C1 11: 73. DC, 07
	inc	r10d					; 284B _ 41: FF. C2
	vmovdqu oword [rbp+10H], xmm2			; 284E _ C5 FA: 7F. 55, 10
	vmovdqu oword [rbp], xmm3			; 2853 _ C5 FA: 7F. 5D, 00
	vpsrlq	xmm2, xmm13, 7				; 2858 _ C4 C1 69: 73. D5, 07
	add	r13, 64 				; 285E _ 49: 83. C5, 40
	vpor	xmm3, xmm14, xmm2			; 2862 _ C5 89: EB. DA
	vpsraw	xmm2, xmm0, 8				; 2866 _ C5 E9: 71. E0, 08
	vpunpcklqdq xmm5, xmm9, xmm8			; 286B _ C4 C1 31: 6C. E8
	vpunpckhqdq xmm8, xmm9, xmm8			; 2870 _ C4 41 31: 6D. C0
	vpxor	xmm11, xmm5, xmm1			; 2875 _ C5 51: EF. D9
	vpsrldq xmm2, xmm2, 15				; 2879 _ C5 E9: 73. DA, 0F
	vpxor	xmm9, xmm8, xmm0			; 287E _ C5 39: EF. C8
	vpand	xmm2, xmm2, xmm7			; 2882 _ C5 E9: DB. D7
	vmovdqu oword [rbp+20H], xmm11			; 2886 _ C5 7A: 7F. 5D, 20
	vmovdqu oword [rbp+30H], xmm9			; 288B _ C5 7A: 7F. 4D, 30
	add	rbp, 64 				; 2890 _ 48: 83. C5, 40
	vpxor	xmm3, xmm3, xmm2			; 2894 _ C5 E1: EF. DA
	cmp	r10d, 8 				; 2898 _ 41: 83. FA, 08
	jl	?_003					; 289C _ 0F 8C, FFFFD831
	add	r14, -512				; 28A2 _ 49: 81. C6, FFFFFE00
	jne	?_001					; 28A9 _ 0F 85, FFFFD7EF
	vmovups xmm6, oword [rsp+0A0H]			; 28AF _ C5 F8: 10. B4 24, 000000A0
	vmovups xmm7, oword [rsp+90H]			; 28B8 _ C5 F8: 10. BC 24, 00000090
	vmovups xmm8, oword [rsp+80H]			; 28C1 _ C5 78: 10. 84 24, 00000080
	vmovups xmm9, oword [rsp+70H]			; 28CA _ C5 78: 10. 4C 24, 70
	vmovups xmm10, oword [rsp+60H]			; 28D0 _ C5 78: 10. 54 24, 60
	vmovups xmm11, oword [rsp+50H]			; 28D6 _ C5 78: 10. 5C 24, 50
	vmovups xmm12, oword [rsp+40H]			; 28DC _ C5 78: 10. 64 24, 40
	vmovups xmm13, oword [rsp+30H]			; 28E2 _ C5 78: 10. 6C 24, 30
	vmovups xmm14, oword [rsp+20H]			; 28E8 _ C5 78: 10. 74 24, 20
	add	rsp, 208				; 28EE _ 48: 81. C4, 000000D0
	pop	rbp					; 28F5 _ 5D
	pop	r15					; 28F6 _ 41: 5F
	pop	r14					; 28F8 _ 41: 5E
	pop	r13					; 28FA _ 41: 5D
	pop	r12					; 28FC _ 41: 5C
	ret						; 28FE _ C3
; xts_serpent_avx_encrypt End of function

ALIGN	16

xts_serpent_avx_decrypt:; Function begin
	push	r12					; 0000 _ 41: 54
	push	r13					; 0002 _ 41: 55
	push	r14					; 0004 _ 41: 56
	push	r15					; 0006 _ 41: 57
	push	rbp					; 0008 _ 55
	sub	rsp, 192				; 0009 _ 48: 81. EC, 000000C0
	mov	rbp, rdx				; 0010 _ 48: 89. D5
	mov	r10d, 135				; 0013 _ 41: BA, 00000087
	vpcmpeqd xmm0, xmm0, xmm0			; 0019 _ C5 F9: 76. C0
	mov	rax, qword [rsp+110H]			; 001D _ 48: 8B. 84 24, 00000110
	mov	r12, rax				; 0025 _ 49: 89. C4
	shr	r9, 9					; 0028 _ 49: C1. E9, 09
	mov	r13, rcx				; 002C _ 49: 89. CD
	mov	qword [rsp+0A0H], r9			; 002F _ 4C: 89. 8C 24, 000000A0
	mov	r14, r8 				; 0037 _ 4D: 89. C6
	vmovd	xmm1, r10d				; 003A _ C4 C1 79: 6E. CA
	lea	r9, [rax+2710H] 			; 003F _ 4C: 8D. 88, 00002710
	mov	qword [rsp+0A8H], 0			; 0046 _ 48: C7. 84 24, 000000A8, 00000000
	mov	r15, r9 				; 0052 _ 4D: 89. CF
	vmovups oword [rsp+90H], xmm6			; 0055 _ C5 F8: 11. B4 24, 00000090
	vmovdqa xmm6, xmm0				; 005E _ C5 F9: 6F. F0
	vmovups oword [rsp+80H], xmm7			; 0062 _ C5 F8: 11. BC 24, 00000080
	vmovdqa xmm7, xmm1				; 006B _ C5 F9: 6F. F9
	vmovups oword [rsp+70H], xmm8			; 006F _ C5 78: 11. 44 24, 70
	vmovups oword [rsp+60H], xmm9			; 0075 _ C5 78: 11. 4C 24, 60
	vmovups oword [rsp+50H], xmm10			; 007B _ C5 78: 11. 54 24, 50
	vmovups oword [rsp+40H], xmm11			; 0081 _ C5 78: 11. 5C 24, 40
	vmovups oword [rsp+30H], xmm12			; 0087 _ C5 78: 11. 64 24, 30
	vmovups oword [rsp+20H], xmm13			; 008D _ C5 78: 11. 6C 24, 20
	jmp	?_005					; 0093 _ EB, 09

?_004:	vmovdqu oword [rsp+0B0H], xmm3			; 0095 _ C5 FA: 7F. 9C 24, 000000B0
?_005:	mov	r8, r15 				; 009E _ 4D: 89. F8
	lea	rcx, [rsp+0A0H] 			; 00A1 _ 48: 8D. 8C 24, 000000A0
	inc	qword [rsp+0A0H]			; 00A9 _ 48: FF. 84 24, 000000A0
	lea	rdx, [rsp+0B0H] 			; 00B1 _ 48: 8D. 94 24, 000000B0
	call	serpent256_encrypt			; 00B9 _ E8, 00000000(rel)
	vmovdqu xmm3, oword [rsp+0B0H]			; 00BE _ C5 FA: 6F. 9C 24, 000000B0
	xor	r10d, r10d				; 00C7 _ 45: 33. D2
?_006:	vpslldq xmm2, xmm3, 8				; 00CA _ C5 E9: 73. FB, 08
	vpsllq	xmm4, xmm3, 1				; 00CF _ C5 D9: 73. F3, 01
	vpsrldq xmm2, xmm2, 7				; 00D4 _ C5 E9: 73. DA, 07
	vpsrlq	xmm2, xmm2, 7				; 00D9 _ C5 E9: 73. D2, 07
	vpor	xmm4, xmm4, xmm2			; 00DE _ C5 D9: EB. E2
	vpsraw	xmm2, xmm3, 8				; 00E2 _ C5 E9: 71. E3, 08
	vpsrldq xmm2, xmm2, 15				; 00E7 _ C5 E9: 73. DA, 0F
	vpand	xmm2, xmm2, xmm7			; 00EC _ C5 E9: DB. D7
	vpxor	xmm0, xmm4, xmm2			; 00F0 _ C5 D9: EF. C2
	vpslldq xmm2, xmm0, 8				; 00F4 _ C5 E9: 73. F8, 08
	vpsllq	xmm4, xmm0, 1				; 00F9 _ C5 D9: 73. F0, 01
	vpsrldq xmm2, xmm2, 7				; 00FE _ C5 E9: 73. DA, 07
	vpsrlq	xmm2, xmm2, 7				; 0103 _ C5 E9: 73. D2, 07
	vpor	xmm4, xmm4, xmm2			; 0108 _ C5 D9: EB. E2
	vpsraw	xmm2, xmm0, 8				; 010C _ C5 E9: 71. E0, 08
	vpsrldq xmm2, xmm2, 15				; 0111 _ C5 E9: 73. DA, 0F
	vpand	xmm2, xmm2, xmm7			; 0116 _ C5 E9: DB. D7
	vpxor	xmm2, xmm4, xmm2			; 011A _ C5 D9: EF. D2
	vpslldq xmm4, xmm2, 8				; 011E _ C5 D9: 73. FA, 08
	vpsllq	xmm5, xmm2, 1				; 0123 _ C5 D1: 73. F2, 01
	vpsrldq xmm4, xmm4, 7				; 0128 _ C5 D9: 73. DC, 07
	vpsrlq	xmm4, xmm4, 7				; 012D _ C5 D9: 73. D4, 07
	vpor	xmm5, xmm5, xmm4			; 0132 _ C5 D1: EB. EC
	vpsraw	xmm4, xmm2, 8				; 0136 _ C5 D9: 71. E2, 08
	vpsrldq xmm4, xmm4, 15				; 013B _ C5 D9: 73. DC, 0F
	vpand	xmm4, xmm4, xmm7			; 0140 _ C5 D9: DB. E7
	vpxor	xmm1, xmm5, xmm4			; 0144 _ C5 D1: EF. CC
	vpxor	xmm13, xmm3, oword [r13]		; 0148 _ C4 41 61: EF. 6D, 00
	vpxor	xmm9, xmm0, oword [r13+10H]		; 014E _ C4 41 79: EF. 4D, 10
	vpxor	xmm12, xmm2, oword [r13+20H]		; 0154 _ C4 41 69: EF. 65, 20
	vpxor	xmm4, xmm1, oword [r13+30H]		; 015A _ C4 C1 71: EF. 65, 30
	vpunpckldq xmm10, xmm13, xmm9			; 0160 _ C4 41 11: 62. D1
	vpunpckldq xmm8, xmm12, xmm4			; 0165 _ C5 19: 62. C4
	vpunpckhdq xmm11, xmm13, xmm9			; 0169 _ C4 41 11: 6A. D9
	vpunpcklqdq xmm9, xmm10, xmm8			; 016E _ C4 41 29: 6C. C8
	vmovd	xmm13, dword [r12+1480H]		; 0173 _ C4 41 79: 6E. AC 24, 00001480
	vpunpckhqdq xmm8, xmm10, xmm8			; 017D _ C4 41 29: 6D. C0
	vmovd	xmm10, dword [r12+1484H]		; 0182 _ C4 41 79: 6E. 94 24, 00001484
	vpunpckhdq xmm12, xmm12, xmm4			; 018C _ C5 19: 6A. E4
	vpshufd xmm5, xmm13, 0				; 0190 _ C4 C1 79: 70. ED, 00
	vpshufd xmm13, xmm10, 0 			; 0196 _ C4 41 79: 70. EA, 00
	vpxor	xmm9, xmm9, xmm5			; 019C _ C5 31: EF. CD
	vpunpcklqdq xmm4, xmm11, xmm12			; 01A0 _ C4 C1 21: 6C. E4
	vpxor	xmm10, xmm8, xmm13			; 01A5 _ C4 41 39: EF. D5
	vmovd	xmm8, dword [r12+1488H] 		; 01AA _ C4 41 79: 6E. 84 24, 00001488
	vpunpckhqdq xmm12, xmm11, xmm12 		; 01B4 _ C4 41 21: 6D. E4
	vmovd	xmm11, dword [r12+148CH]		; 01B9 _ C4 41 79: 6E. 9C 24, 0000148C
	vpshufd xmm5, xmm8, 0				; 01C3 _ C4 C1 79: 70. E8, 00
	vpshufd xmm11, xmm11, 0 			; 01C9 _ C4 41 79: 70. DB, 00
	vpxor	xmm8, xmm4, xmm5			; 01CF _ C5 59: EF. C5
	vpxor	xmm5, xmm12, xmm11			; 01D3 _ C4 C1 19: EF. EB
	vpand	xmm11, xmm9, xmm5			; 01D8 _ C5 31: DB. DD
	vpxor	xmm9, xmm8, xmm9			; 01DC _ C4 41 39: EF. C9
	vpor	xmm12, xmm8, xmm5			; 01E1 _ C5 39: EB. E5
	vpxor	xmm8, xmm9, xmm6			; 01E5 _ C5 31: EF. C6
	vpxor	xmm13, xmm11, xmm8			; 01E9 _ C4 41 21: EF. E8
	vpand	xmm8, xmm8, xmm12			; 01EE _ C4 41 39: DB. C4
	vpxor	xmm9, xmm5, xmm10			; 01F3 _ C4 41 51: EF. CA
	vpor	xmm10, xmm10, xmm11			; 01F8 _ C4 41 29: EB. D3
	vpxor	xmm5, xmm10, xmm8			; 01FD _ C4 C1 29: EF. E8
	vpxor	xmm10, xmm8, xmm13			; 0202 _ C4 41 39: EF. D5
	vpand	xmm4, xmm9, xmm12			; 0207 _ C4 C1 31: DB. E4
	vpxor	xmm8, xmm12, xmm5			; 020C _ C5 19: EF. C5
	vpor	xmm13, xmm13, xmm10			; 0210 _ C4 41 11: EB. EA
	vpxor	xmm12, xmm4, xmm8			; 0215 _ C4 41 59: EF. E0
	vpxor	xmm11, xmm13, xmm4			; 021A _ C5 11: EF. DC
	vpxor	xmm13, xmm12, xmm10			; 021E _ C4 41 19: EF. EA
	vmovd	xmm4, dword [r12+1470H] 		; 0223 _ C4 C1 79: 6E. A4 24, 00001470
	vpshufd xmm9, xmm4, 0				; 022D _ C5 79: 70. CC, 00
	vmovd	xmm4, dword [r12+1474H] 		; 0232 _ C4 C1 79: 6E. A4 24, 00001474
	vpxor	xmm12, xmm13, xmm9			; 023C _ C4 41 11: EF. E1
	vmovd	xmm9, dword [r12+1478H] 		; 0241 _ C4 41 79: 6E. 8C 24, 00001478
	vpshufd xmm13, xmm4, 0				; 024B _ C5 79: 70. EC, 00
	vpshufd xmm4, xmm9, 0				; 0250 _ C4 C1 79: 70. E1, 00
	vpxor	xmm13, xmm11, xmm13			; 0256 _ C4 41 21: EF. ED
	vpxor	xmm4, xmm5, xmm4			; 025B _ C5 D1: EF. E4
	vpor	xmm5, xmm8, xmm11			; 025F _ C4 C1 39: EB. EB
	vmovd	xmm8, dword [r12+147CH] 		; 0264 _ C4 41 79: 6E. 84 24, 0000147C
	vpxor	xmm11, xmm5, xmm10			; 026E _ C4 41 51: EF. DA
	vpshufd xmm10, xmm8, 0				; 0273 _ C4 41 79: 70. D0, 00
	vpslld	xmm8, xmm4, 10				; 0279 _ C5 B9: 72. F4, 0A
	vpsrld	xmm9, xmm4, 22				; 027E _ C5 B1: 72. D4, 16
	vpxor	xmm11, xmm11, xmm10			; 0283 _ C4 41 21: EF. DA
	vpor	xmm4, xmm8, xmm9			; 0288 _ C4 C1 39: EB. E1
	vpslld	xmm8, xmm12, 27 			; 028D _ C4 C1 39: 72. F4, 1B
	vpsrld	xmm12, xmm12, 5 			; 0293 _ C4 C1 19: 72. D4, 05
	vpxor	xmm5, xmm4, xmm11			; 0299 _ C4 C1 59: EF. EB
	vpor	xmm8, xmm8, xmm12			; 029E _ C4 41 39: EB. C4
	vpslld	xmm10, xmm13, 7 			; 02A3 _ C4 C1 29: 72. F5, 07
	vpxor	xmm12, xmm8, xmm13			; 02A9 _ C4 41 39: EF. E5
	vpslld	xmm4, xmm11, 25 			; 02AE _ C4 C1 59: 72. F3, 19
	vpxor	xmm8, xmm12, xmm11			; 02B4 _ C4 41 19: EF. C3
	vpsrld	xmm11, xmm11, 7 			; 02B9 _ C4 C1 21: 72. D3, 07
	vpxor	xmm9, xmm5, xmm10			; 02BF _ C4 41 51: EF. CA
	vpor	xmm11, xmm4, xmm11			; 02C4 _ C4 41 59: EB. DB
	vpslld	xmm5, xmm13, 31 			; 02C9 _ C4 C1 51: 72. F5, 1F
	vpsrld	xmm13, xmm13, 1 			; 02CF _ C4 C1 11: 72. D5, 01
	vpxor	xmm12, xmm11, xmm9			; 02D5 _ C4 41 21: EF. E1
	vpslld	xmm4, xmm8, 3				; 02DA _ C4 C1 59: 72. F0, 03
	vpor	xmm13, xmm5, xmm13			; 02E0 _ C4 41 51: EB. ED
	vpxor	xmm11, xmm12, xmm4			; 02E5 _ C5 19: EF. DC
	vpxor	xmm12, xmm13, xmm8			; 02E9 _ C4 41 11: EF. E0
	vpslld	xmm4, xmm9, 29				; 02EE _ C4 C1 59: 72. F1, 1D
	vpxor	xmm5, xmm12, xmm9			; 02F4 _ C4 C1 19: EF. E9
	vpsrld	xmm9, xmm9, 3				; 02F9 _ C4 C1 31: 72. D1, 03
	vpslld	xmm10, xmm8, 19 			; 02FF _ C4 C1 29: 72. F0, 13
	vpsrld	xmm8, xmm8, 13				; 0305 _ C4 C1 39: 72. D0, 0D
	vpor	xmm13, xmm4, xmm9			; 030B _ C4 41 59: EB. E9
	vpor	xmm9, xmm10, xmm8			; 0310 _ C4 41 29: EB. C8
	vpxor	xmm4, xmm9, xmm13			; 0315 _ C4 C1 31: EF. E5
	vpxor	xmm10, xmm11, xmm5			; 031A _ C5 21: EF. D5
	vpand	xmm12, xmm13, xmm4			; 031E _ C5 11: DB. E4
	vpxor	xmm11, xmm13, xmm11			; 0322 _ C4 41 11: EF. DB
	vpxor	xmm8, xmm12, xmm6			; 0327 _ C5 19: EF. C6
	vpor	xmm9, xmm11, xmm4			; 032B _ C5 21: EB. CC
	vpxor	xmm8, xmm8, xmm10			; 032F _ C4 41 39: EF. C2
	vpxor	xmm13, xmm10, xmm9			; 0334 _ C4 41 29: EF. E9
	vmovd	xmm10, dword [r12+1460H]		; 0339 _ C4 41 79: 6E. 94 24, 00001460
	vpxor	xmm11, xmm4, xmm8			; 0343 _ C4 41 59: EF. D8
	vpand	xmm4, xmm5, xmm13			; 0348 _ C4 C1 51: DB. E5
	vpxor	xmm9, xmm9, xmm5			; 034D _ C5 31: EF. CD
	vpxor	xmm12, xmm4, xmm11			; 0351 _ C4 41 59: EF. E3
	vpxor	xmm5, xmm11, xmm13			; 0356 _ C4 C1 21: EF. ED
	vpshufd xmm4, xmm10, 0				; 035B _ C4 C1 79: 70. E2, 00
	vpxor	xmm10, xmm12, xmm4			; 0361 _ C5 19: EF. D4
	vpxor	xmm12, xmm13, xmm12			; 0365 _ C4 41 11: EF. E4
	vmovd	xmm4, dword [r12+1464H] 		; 036A _ C4 C1 79: 6E. A4 24, 00001464
	vpshufd xmm4, xmm4, 0				; 0374 _ C5 F9: 70. E4, 00
	vmovd	xmm11, dword [r12+1468H]		; 0379 _ C4 41 79: 6E. 9C 24, 00001468
	vpxor	xmm4, xmm8, xmm4			; 0383 _ C5 B9: EF. E4
	vpor	xmm8, xmm5, xmm8			; 0387 _ C4 41 51: EB. C0
	vpshufd xmm5, xmm11, 0				; 038C _ C4 C1 79: 70. EB, 00
	vpxor	xmm9, xmm9, xmm8			; 0392 _ C4 41 31: EF. C8
	vmovd	xmm13, dword [r12+146CH]		; 0397 _ C4 41 79: 6E. AC 24, 0000146C
	vpxor	xmm8, xmm9, xmm5			; 03A1 _ C5 31: EF. C5
	vpshufd xmm11, xmm13, 0 			; 03A5 _ C4 41 79: 70. DD, 00
	vpslld	xmm13, xmm8, 10 			; 03AB _ C4 C1 11: 72. F0, 0A
	vpsrld	xmm8, xmm8, 22				; 03B1 _ C4 C1 39: 72. D0, 16
	vpxor	xmm11, xmm12, xmm11			; 03B7 _ C4 41 19: EF. DB
	vpor	xmm9, xmm13, xmm8			; 03BC _ C4 41 11: EB. C8
	vpslld	xmm8, xmm10, 27 			; 03C1 _ C4 C1 39: 72. F2, 1B
	vpsrld	xmm10, xmm10, 5 			; 03C7 _ C4 C1 29: 72. D2, 05
	vpxor	xmm12, xmm9, xmm11			; 03CD _ C4 41 31: EF. E3
	vpor	xmm8, xmm8, xmm10			; 03D2 _ C4 41 39: EB. C2
	vpslld	xmm5, xmm4, 7				; 03D7 _ C5 D1: 72. F4, 07
	vpxor	xmm9, xmm8, xmm4			; 03DC _ C5 39: EF. CC
	vpxor	xmm13, xmm12, xmm5			; 03E0 _ C5 19: EF. ED
	vpxor	xmm8, xmm9, xmm11			; 03E4 _ C4 41 31: EF. C3
	vpslld	xmm12, xmm11, 25			; 03E9 _ C4 C1 19: 72. F3, 19
	vpsrld	xmm11, xmm11, 7 			; 03EF _ C4 C1 21: 72. D3, 07
	vpslld	xmm9, xmm8, 19				; 03F5 _ C4 C1 31: 72. F0, 13
	vpor	xmm5, xmm12, xmm11			; 03FB _ C4 C1 19: EB. EB
	vpslld	xmm11, xmm8, 3				; 0400 _ C4 C1 21: 72. F0, 03
	vpxor	xmm10, xmm5, xmm13			; 0406 _ C4 41 51: EF. D5
	vpsrld	xmm12, xmm8, 13 			; 040B _ C4 C1 19: 72. D0, 0D
	vpslld	xmm5, xmm4, 31				; 0411 _ C5 D1: 72. F4, 1F
	vpsrld	xmm4, xmm4, 1				; 0416 _ C5 D9: 72. D4, 01
	vpxor	xmm11, xmm10, xmm11			; 041B _ C4 41 29: EF. DB
	vpor	xmm10, xmm9, xmm12			; 0420 _ C4 41 31: EB. D4
	vpor	xmm9, xmm5, xmm4			; 0425 _ C5 51: EB. CC
	vpslld	xmm4, xmm13, 29 			; 0429 _ C4 C1 59: 72. F5, 1D
	vpxor	xmm8, xmm9, xmm8			; 042F _ C4 41 31: EF. C0
	vpxor	xmm12, xmm8, xmm13			; 0434 _ C4 41 39: EF. E5
	vpsrld	xmm13, xmm13, 3 			; 0439 _ C4 C1 11: 72. D5, 03
	vpxor	xmm5, xmm12, xmm6			; 043F _ C5 99: EF. EE
	vpor	xmm8, xmm4, xmm13			; 0443 _ C4 41 59: EB. C5
	vpxor	xmm12, xmm8, xmm5			; 0448 _ C5 39: EF. E5
	vpor	xmm13, xmm11, xmm10			; 044C _ C4 41 21: EB. EA
	vpxor	xmm8, xmm13, xmm12			; 0451 _ C4 41 11: EF. C4
	vpxor	xmm9, xmm11, xmm8			; 0456 _ C4 41 21: EF. C8
	vpor	xmm11, xmm12, xmm5			; 045B _ C5 19: EB. DD
	vpand	xmm11, xmm11, xmm10			; 045F _ C4 41 21: DB. DA
	vpxor	xmm4, xmm11, xmm9			; 0464 _ C4 C1 21: EF. E1
	vpor	xmm9, xmm9, xmm10			; 0469 _ C4 41 31: EB. CA
	vpand	xmm13, xmm5, xmm4			; 046E _ C5 51: DB. EC
	vpxor	xmm5, xmm9, xmm5			; 0472 _ C5 B1: EF. ED
	vmovd	xmm12, dword [r12+1450H]		; 0476 _ C4 41 79: 6E. A4 24, 00001450
	vpxor	xmm13, xmm13, xmm8			; 0480 _ C4 41 11: EF. E8
	vpxor	xmm5, xmm5, xmm4			; 0485 _ C5 D1: EF. EC
	vpshufd xmm11, xmm12, 0 			; 0489 _ C4 41 79: 70. DC, 00
	vpxor	xmm9, xmm5, xmm13			; 048F _ C4 41 51: EF. CD
	vpand	xmm8, xmm8, xmm5			; 0494 _ C5 39: DB. C5
	vpxor	xmm13, xmm13, xmm11			; 0498 _ C4 41 11: EF. EB
	vmovd	xmm11, dword [r12+1454H]		; 049D _ C4 41 79: 6E. 9C 24, 00001454
	vpxor	xmm12, xmm9, xmm6			; 04A7 _ C5 31: EF. E6
	vpxor	xmm9, xmm8, xmm9			; 04AB _ C4 41 39: EF. C9
	vpshufd xmm11, xmm11, 0 			; 04B0 _ C4 41 79: 70. DB, 00
	vpxor	xmm8, xmm9, xmm10			; 04B6 _ C4 41 31: EF. C2
	vmovd	xmm10, dword [r12+1458H]		; 04BB _ C4 41 79: 6E. 94 24, 00001458
	vpxor	xmm12, xmm12, xmm11			; 04C5 _ C4 41 19: EF. E3
	vpshufd xmm11, xmm10, 0 			; 04CA _ C4 41 79: 70. DA, 00
	vpxor	xmm10, xmm8, xmm11			; 04D0 _ C4 41 39: EF. D3
	vmovd	xmm9, dword [r12+145CH] 		; 04D5 _ C4 41 79: 6E. 8C 24, 0000145C
	vpslld	xmm8, xmm10, 10 			; 04DF _ C4 C1 39: 72. F2, 0A
	vpsrld	xmm11, xmm10, 22			; 04E5 _ C4 C1 21: 72. D2, 16
	vpslld	xmm10, xmm12, 7 			; 04EB _ C4 C1 29: 72. F4, 07
	vpshufd xmm5, xmm9, 0				; 04F1 _ C4 C1 79: 70. E9, 00
	vpor	xmm9, xmm8, xmm11			; 04F7 _ C4 41 39: EB. CB
	vpslld	xmm8, xmm13, 27 			; 04FC _ C4 C1 39: 72. F5, 1B
	vpsrld	xmm13, xmm13, 5 			; 0502 _ C4 C1 11: 72. D5, 05
	vpxor	xmm4, xmm4, xmm5			; 0508 _ C5 D9: EF. E5
	vpor	xmm8, xmm8, xmm13			; 050C _ C4 41 39: EB. C5
	vpxor	xmm5, xmm9, xmm4			; 0511 _ C5 B1: EF. EC
	vpxor	xmm13, xmm8, xmm12			; 0515 _ C4 41 39: EF. EC
	vpxor	xmm11, xmm5, xmm10			; 051A _ C4 41 51: EF. DA
	vpxor	xmm5, xmm13, xmm4			; 051F _ C5 91: EF. EC
	vpslld	xmm9, xmm4, 25				; 0523 _ C5 B1: 72. F4, 19
	vpsrld	xmm4, xmm4, 7				; 0528 _ C5 D9: 72. D4, 07
	vpor	xmm8, xmm9, xmm4			; 052D _ C5 31: EB. C4
	vpslld	xmm10, xmm12, 31			; 0531 _ C4 C1 29: 72. F4, 1F
	vpsrld	xmm12, xmm12, 1 			; 0537 _ C4 C1 19: 72. D4, 01
	vpxor	xmm13, xmm8, xmm11			; 053D _ C4 41 39: EF. EB
	vpslld	xmm9, xmm5, 3				; 0542 _ C5 B1: 72. F5, 03
	vpor	xmm8, xmm10, xmm12			; 0547 _ C4 41 29: EB. C4
	vpxor	xmm4, xmm13, xmm9			; 054C _ C4 C1 11: EF. E1
	vpxor	xmm13, xmm8, xmm5			; 0551 _ C5 39: EF. ED
	vpxor	xmm8, xmm13, xmm11			; 0555 _ C4 41 11: EF. C3
	vpslld	xmm9, xmm11, 29 			; 055A _ C4 C1 31: 72. F3, 1D
	vpsrld	xmm11, xmm11, 3 			; 0560 _ C4 C1 21: 72. D3, 03
	vpslld	xmm12, xmm5, 19 			; 0566 _ C5 99: 72. F5, 13
	vpor	xmm11, xmm9, xmm11			; 056B _ C4 41 31: EB. DB
	vpsrld	xmm5, xmm5, 13				; 0570 _ C5 D1: 72. D5, 0D
	vpand	xmm10, xmm11, xmm4			; 0575 _ C5 21: DB. D4
	vpor	xmm9, xmm12, xmm5			; 0579 _ C5 19: EB. CD
	vpxor	xmm5, xmm10, xmm8			; 057D _ C4 C1 29: EF. E8
	vpor	xmm8, xmm8, xmm4			; 0582 _ C5 39: EB. C4
	vpand	xmm8, xmm8, xmm9			; 0586 _ C4 41 39: DB. C1
	vpxor	xmm11, xmm11, xmm5			; 058B _ C5 21: EF. DD
	vpxor	xmm11, xmm11, xmm8			; 058F _ C4 41 21: EF. D8
	vpxor	xmm12, xmm9, xmm6			; 0594 _ C5 31: EF. E6
	vpxor	xmm13, xmm4, xmm11			; 0598 _ C4 41 59: EF. EB
	vpand	xmm4, xmm8, xmm5			; 059D _ C5 B9: DB. E5
	vpxor	xmm9, xmm4, xmm13			; 05A1 _ C4 41 59: EF. CD
	vpxor	xmm8, xmm12, xmm9			; 05A6 _ C4 41 19: EF. C1
	vpand	xmm12, xmm13, xmm12			; 05AB _ C4 41 11: DB. E4
	vmovd	xmm13, dword [r12+1440H]		; 05B0 _ C4 41 79: 6E. AC 24, 00001440
	vpxor	xmm4, xmm12, xmm5			; 05BA _ C5 99: EF. E5
	vpshufd xmm12, xmm13, 0 			; 05BE _ C4 41 79: 70. E5, 00
	vpxor	xmm10, xmm4, xmm8			; 05C4 _ C4 41 59: EF. D0
	vpxor	xmm4, xmm8, xmm12			; 05C9 _ C4 C1 39: EF. E4
	vpand	xmm5, xmm5, xmm8			; 05CE _ C4 C1 51: DB. E8
	vmovd	xmm12, dword [r12+1444H]		; 05D3 _ C4 41 79: 6E. A4 24, 00001444
	vpxor	xmm13, xmm10, xmm8			; 05DD _ C4 41 29: EF. E8
	vpshufd xmm12, xmm12, 0 			; 05E2 _ C4 41 79: 70. E4, 00
	vpxor	xmm8, xmm5, xmm11			; 05E8 _ C4 41 51: EF. C3
	vpxor	xmm12, xmm13, xmm12			; 05ED _ C4 41 11: EF. E4
	vpor	xmm13, xmm8, xmm10			; 05F2 _ C4 41 39: EB. EA
	vpxor	xmm8, xmm13, xmm9			; 05F7 _ C4 41 11: EF. C1
	vmovd	xmm9, dword [r12+1448H] 		; 05FC _ C4 41 79: 6E. 8C 24, 00001448
	vpshufd xmm13, xmm9, 0				; 0606 _ C4 41 79: 70. E9, 00
	vpxor	xmm10, xmm8, xmm13			; 060C _ C4 41 39: EF. D5
	vmovd	xmm9, dword [r12+144CH] 		; 0611 _ C4 41 79: 6E. 8C 24, 0000144C
	vpslld	xmm8, xmm10, 10 			; 061B _ C4 C1 39: 72. F2, 0A
	vpsrld	xmm13, xmm10, 22			; 0621 _ C4 C1 11: 72. D2, 16
	vpslld	xmm10, xmm12, 7 			; 0627 _ C4 C1 29: 72. F4, 07
	vpshufd xmm5, xmm9, 0				; 062D _ C4 C1 79: 70. E9, 00
	vpor	xmm9, xmm8, xmm13			; 0633 _ C4 41 39: EB. CD
	vpslld	xmm8, xmm4, 27				; 0638 _ C5 B9: 72. F4, 1B
	vpsrld	xmm4, xmm4, 5				; 063D _ C5 D9: 72. D4, 05
	vpor	xmm8, xmm8, xmm4			; 0642 _ C5 39: EB. C4
	vpxor	xmm11, xmm11, xmm5			; 0646 _ C5 21: EF. DD
	vpxor	xmm13, xmm8, xmm12			; 064A _ C4 41 39: EF. EC
	vpxor	xmm5, xmm9, xmm11			; 064F _ C4 C1 31: EF. EB
	vpxor	xmm9, xmm13, xmm11			; 0654 _ C4 41 11: EF. CB
	vpslld	xmm4, xmm11, 25 			; 0659 _ C4 C1 59: 72. F3, 19
	vpsrld	xmm11, xmm11, 7 			; 065F _ C4 C1 21: 72. D3, 07
	vpxor	xmm10, xmm5, xmm10			; 0665 _ C4 41 51: EF. D2
	vpor	xmm8, xmm4, xmm11			; 066A _ C4 41 59: EB. C3
	vpslld	xmm4, xmm12, 31 			; 066F _ C4 C1 59: 72. F4, 1F
	vpsrld	xmm12, xmm12, 1 			; 0675 _ C4 C1 19: 72. D4, 01
	vpxor	xmm11, xmm8, xmm10			; 067B _ C4 41 39: EF. DA
	vpslld	xmm13, xmm9, 3				; 0680 _ C4 C1 11: 72. F1, 03
	vpor	xmm8, xmm4, xmm12			; 0686 _ C4 41 59: EB. C4
	vpxor	xmm5, xmm11, xmm13			; 068B _ C4 C1 21: EF. ED
	vpxor	xmm11, xmm8, xmm9			; 0690 _ C4 41 39: EF. D9
	vpslld	xmm13, xmm10, 29			; 0695 _ C4 C1 11: 72. F2, 1D
	vpsrld	xmm12, xmm10, 3 			; 069B _ C4 C1 19: 72. D2, 03
	vpxor	xmm11, xmm11, xmm10			; 06A1 _ C4 41 21: EF. DA
	vpor	xmm8, xmm13, xmm12			; 06A6 _ C4 41 11: EB. C4
	vpslld	xmm10, xmm9, 19 			; 06AB _ C4 C1 29: 72. F1, 13
	vpsrld	xmm9, xmm9, 13				; 06B1 _ C4 C1 31: 72. D1, 0D
	vpxor	xmm12, xmm8, xmm11			; 06B7 _ C4 41 39: EF. E3
	vpor	xmm13, xmm10, xmm9			; 06BC _ C4 41 29: EB. E9
	vpxor	xmm13, xmm13, xmm12			; 06C1 _ C4 41 11: EF. EC
	vpand	xmm8, xmm8, xmm12			; 06C6 _ C4 41 39: DB. C4
	vpxor	xmm9, xmm8, xmm13			; 06CB _ C4 41 39: EF. CD
	vpand	xmm4, xmm13, xmm11			; 06D0 _ C4 C1 11: DB. E3
	vpor	xmm10, xmm5, xmm9			; 06D5 _ C4 41 51: EB. D1
	vpxor	xmm5, xmm11, xmm5			; 06DA _ C5 A1: EF. ED
	vpxor	xmm8, xmm12, xmm10			; 06DE _ C4 41 19: EF. C2
	vpxor	xmm13, xmm4, xmm10			; 06E3 _ C4 41 59: EF. EA
	vpxor	xmm12, xmm5, xmm9			; 06E8 _ C4 41 51: EF. E1
	vpand	xmm11, xmm10, xmm8			; 06ED _ C4 41 29: DB. D8
	vpxor	xmm4, xmm12, xmm13			; 06F2 _ C4 C1 19: EF. E5
	vpxor	xmm11, xmm11, xmm12			; 06F7 _ C4 41 21: EF. DC
	vpor	xmm5, xmm4, xmm8			; 06FC _ C4 C1 59: EB. E8
	vpxor	xmm5, xmm5, xmm9			; 0701 _ C4 C1 51: EF. E9
	vmovd	xmm9, dword [r12+1430H] 		; 0706 _ C4 41 79: 6E. 8C 24, 00001430
	vpshufd xmm10, xmm9, 0				; 0710 _ C4 41 79: 70. D1, 00
	vmovd	xmm12, dword [r12+1438H]		; 0716 _ C4 41 79: 6E. A4 24, 00001438
	vpxor	xmm4, xmm8, xmm10			; 0720 _ C4 C1 39: EF. E2
	vmovd	xmm8, dword [r12+1434H] 		; 0725 _ C4 41 79: 6E. 84 24, 00001434
	vpshufd xmm9, xmm8, 0				; 072F _ C4 41 79: 70. C8, 00
	vpshufd xmm8, xmm12, 0				; 0735 _ C4 41 79: 70. C4, 00
	vpxor	xmm10, xmm5, xmm9			; 073B _ C4 41 51: EF. D1
	vpxor	xmm12, xmm11, xmm8			; 0740 _ C4 41 21: EF. E0
	vpxor	xmm11, xmm13, xmm11			; 0745 _ C4 41 11: EF. DB
	vmovd	xmm13, dword [r12+143CH]		; 074A _ C4 41 79: 6E. AC 24, 0000143C
	vpxor	xmm8, xmm11, xmm5			; 0754 _ C5 21: EF. C5
	vpshufd xmm11, xmm13, 0 			; 0758 _ C4 41 79: 70. DD, 00
	vpslld	xmm13, xmm12, 10			; 075E _ C4 C1 11: 72. F4, 0A
	vpsrld	xmm9, xmm12, 22 			; 0764 _ C4 C1 31: 72. D4, 16
	vpxor	xmm11, xmm8, xmm11			; 076A _ C4 41 39: EF. DB
	vpor	xmm12, xmm13, xmm9			; 076F _ C4 41 11: EB. E1
	vpslld	xmm8, xmm10, 7				; 0774 _ C4 C1 39: 72. F2, 07
	vpxor	xmm5, xmm12, xmm11			; 077A _ C4 C1 19: EF. EB
	vpslld	xmm9, xmm4, 27				; 077F _ C5 B1: 72. F4, 1B
	vpsrld	xmm4, xmm4, 5				; 0784 _ C5 D9: 72. D4, 05
	vpxor	xmm13, xmm5, xmm8			; 0789 _ C4 41 51: EF. E8
	vpor	xmm8, xmm9, xmm4			; 078E _ C5 31: EB. C4
	vpslld	xmm12, xmm11, 25			; 0792 _ C4 C1 19: 72. F3, 19
	vpxor	xmm9, xmm8, xmm10			; 0798 _ C4 41 39: EF. CA
	vpxor	xmm8, xmm9, xmm11			; 079D _ C4 41 31: EF. C3
	vpsrld	xmm11, xmm11, 7 			; 07A2 _ C4 C1 21: 72. D3, 07
	vpor	xmm4, xmm12, xmm11			; 07A8 _ C4 C1 19: EB. E3
	vpslld	xmm11, xmm8, 3				; 07AD _ C4 C1 21: 72. F0, 03
	vpxor	xmm5, xmm4, xmm13			; 07B3 _ C4 C1 59: EF. ED
	vpslld	xmm12, xmm10, 31			; 07B8 _ C4 C1 19: 72. F2, 1F
	vpsrld	xmm10, xmm10, 1 			; 07BE _ C4 C1 29: 72. D2, 01
	vpxor	xmm9, xmm5, xmm11			; 07C4 _ C4 41 51: EF. CB
	vpor	xmm11, xmm12, xmm10			; 07C9 _ C4 41 19: EB. DA
	vpslld	xmm4, xmm8, 19				; 07CE _ C4 C1 59: 72. F0, 13
	vpxor	xmm12, xmm11, xmm8			; 07D4 _ C4 41 21: EF. E0
	vpsrld	xmm8, xmm8, 13				; 07D9 _ C4 C1 39: 72. D0, 0D
	vpxor	xmm11, xmm12, xmm13			; 07DF _ C4 41 19: EF. DD
	vpslld	xmm5, xmm13, 29 			; 07E4 _ C4 C1 51: 72. F5, 1D
	vpsrld	xmm13, xmm13, 3 			; 07EA _ C4 C1 11: 72. D5, 03
	vpor	xmm8, xmm4, xmm8			; 07F0 _ C4 41 59: EB. C0
	vpor	xmm10, xmm5, xmm13			; 07F5 _ C4 41 51: EB. D5
	vpxor	xmm4, xmm9, xmm8			; 07FA _ C4 C1 31: EF. E0
	vpxor	xmm13, xmm10, xmm9			; 07FF _ C4 41 29: EF. E9
	vpand	xmm9, xmm4, xmm13			; 0804 _ C4 41 59: DB. CD
	vpxor	xmm12, xmm9, xmm11			; 0809 _ C4 41 31: EF. E3
	vpor	xmm11, xmm11, xmm13			; 080E _ C4 41 21: EB. DD
	vpxor	xmm10, xmm13, xmm12			; 0813 _ C4 41 11: EF. D4
	vpand	xmm13, xmm4, xmm12			; 0818 _ C4 41 59: DB. EC
	vpxor	xmm11, xmm11, xmm4			; 081D _ C5 21: EF. DC
	vpand	xmm4, xmm13, xmm8			; 0821 _ C4 C1 11: DB. E0
	vpxor	xmm9, xmm12, xmm6			; 0826 _ C5 19: EF. CE
	vpxor	xmm13, xmm4, xmm10			; 082A _ C4 41 59: EF. EA
	vmovd	xmm12, dword [r12+1420H]		; 082F _ C4 41 79: 6E. A4 24, 00001420
	vpand	xmm10, xmm10, xmm11			; 0839 _ C4 41 29: DB. D3
	vmovd	xmm4, dword [r12+1424H] 		; 083E _ C4 C1 79: 6E. A4 24, 00001424
	vpshufd xmm5, xmm12, 0				; 0848 _ C4 C1 79: 70. EC, 00
	vpxor	xmm12, xmm11, xmm5			; 084E _ C5 21: EF. E5
	vpshufd xmm5, xmm4, 0				; 0852 _ C5 F9: 70. EC, 00
	vpxor	xmm4, xmm13, xmm5			; 0857 _ C5 91: EF. E5
	vpor	xmm5, xmm10, xmm8			; 085B _ C4 C1 29: EB. E8
	vmovd	xmm10, dword [r12+1428H]		; 0860 _ C4 41 79: 6E. 94 24, 00001428
	vpxor	xmm5, xmm5, xmm9			; 086A _ C4 C1 51: EF. E9
	vpshufd xmm10, xmm10, 0 			; 086F _ C4 41 79: 70. D2, 00
	vpxor	xmm8, xmm8, xmm9			; 0875 _ C4 41 39: EF. C1
	vpxor	xmm5, xmm5, xmm10			; 087A _ C4 C1 51: EF. EA
	vpxor	xmm10, xmm9, xmm13			; 087F _ C4 41 31: EF. D5
	vpand	xmm13, xmm8, xmm11			; 0884 _ C4 41 39: DB. EB
	vpxor	xmm9, xmm10, xmm13			; 0889 _ C4 41 29: EF. CD
	vpslld	xmm13, xmm12, 27			; 088E _ C4 C1 11: 72. F4, 1B
	vmovd	xmm10, dword [r12+142CH]		; 0894 _ C4 41 79: 6E. 94 24, 0000142C
	vpsrld	xmm12, xmm12, 5 			; 089E _ C4 C1 19: 72. D4, 05
	vpshufd xmm8, xmm10, 0				; 08A4 _ C4 41 79: 70. C2, 00
	vpslld	xmm10, xmm5, 10 			; 08AA _ C5 A9: 72. F5, 0A
	vpsrld	xmm5, xmm5, 22				; 08AF _ C5 D1: 72. D5, 16
	vpxor	xmm11, xmm9, xmm8			; 08B4 _ C4 41 31: EF. D8
	vpor	xmm8, xmm10, xmm5			; 08B9 _ C5 29: EB. C5
	vpslld	xmm5, xmm4, 7				; 08BD _ C5 D1: 72. F4, 07
	vpxor	xmm10, xmm8, xmm11			; 08C2 _ C4 41 39: EF. D3
	vpxor	xmm8, xmm10, xmm5			; 08C7 _ C5 29: EF. C5
	vpor	xmm10, xmm13, xmm12			; 08CB _ C4 41 11: EB. D4
	vpxor	xmm5, xmm10, xmm4			; 08D0 _ C5 A9: EF. EC
	vpslld	xmm12, xmm11, 25			; 08D4 _ C4 C1 19: 72. F3, 19
	vpxor	xmm13, xmm5, xmm11			; 08DA _ C4 41 51: EF. EB
	vpsrld	xmm11, xmm11, 7 			; 08DF _ C4 C1 21: 72. D3, 07
	vpor	xmm9, xmm12, xmm11			; 08E5 _ C4 41 19: EB. CB
	vpslld	xmm5, xmm13, 3				; 08EA _ C4 C1 51: 72. F5, 03
	vpxor	xmm10, xmm9, xmm8			; 08F0 _ C4 41 31: EF. D0
	vpslld	xmm11, xmm4, 31 			; 08F5 _ C5 A1: 72. F4, 1F
	vpsrld	xmm4, xmm4, 1				; 08FA _ C5 D9: 72. D4, 01
	vpxor	xmm12, xmm10, xmm5			; 08FF _ C5 29: EF. E5
	vpor	xmm10, xmm11, xmm4			; 0903 _ C5 21: EB. D4
	vpslld	xmm9, xmm13, 19 			; 0907 _ C4 C1 31: 72. F5, 13
	vpxor	xmm5, xmm10, xmm13			; 090D _ C4 C1 29: EF. ED
	vpsrld	xmm13, xmm13, 13			; 0912 _ C4 C1 11: 72. D5, 0D
	vpxor	xmm11, xmm5, xmm8			; 0918 _ C4 41 51: EF. D8
	vpslld	xmm4, xmm8, 29				; 091D _ C4 C1 59: 72. F0, 1D
	vpxor	xmm10, xmm11, xmm12			; 0923 _ C4 41 21: EF. D4
	vpsrld	xmm8, xmm8, 3				; 0928 _ C4 C1 39: 72. D0, 03
	vpor	xmm9, xmm9, xmm13			; 092E _ C4 41 31: EB. CD
	vpand	xmm12, xmm12, xmm10			; 0933 _ C4 41 19: DB. E2
	vpor	xmm8, xmm4, xmm8			; 0938 _ C4 41 59: EB. C0
	vpxor	xmm4, xmm12, xmm9			; 093D _ C4 C1 19: EF. E1
	vpxor	xmm11, xmm11, xmm8			; 0942 _ C4 41 21: EF. D8
	vpxor	xmm5, xmm10, xmm4			; 0947 _ C5 A9: EF. EC
	vpor	xmm10, xmm9, xmm10			; 094B _ C4 41 31: EB. D2
	vpxor	xmm8, xmm8, xmm4			; 0950 _ C5 39: EF. C4
	vpxor	xmm13, xmm10, xmm11			; 0954 _ C4 41 29: EF. EB
	vpxor	xmm11, xmm11, xmm6			; 0959 _ C5 21: EF. DE
	vpor	xmm9, xmm13, xmm8			; 095D _ C4 41 11: EB. C8
	vpxor	xmm13, xmm9, xmm5			; 0962 _ C5 31: EF. ED
	vpor	xmm5, xmm5, xmm4			; 0966 _ C5 D1: EB. EC
	vpxor	xmm10, xmm5, xmm13			; 096A _ C4 41 51: EF. D5
	vmovd	xmm12, dword [r12+1410H]		; 096F _ C4 41 79: 6E. A4 24, 00001410
	vpxor	xmm9, xmm11, xmm10			; 0979 _ C4 41 21: EF. CA
	vmovd	xmm11, dword [r12+1414H]		; 097E _ C4 41 79: 6E. 9C 24, 00001414
	vpor	xmm10, xmm10, xmm13			; 0988 _ C4 41 29: EB. D5
	vpshufd xmm5, xmm12, 0				; 098D _ C4 C1 79: 70. EC, 00
	vpshufd xmm12, xmm11, 0 			; 0993 _ C4 41 79: 70. E3, 00
	vpxor	xmm5, xmm9, xmm5			; 0999 _ C5 B1: EF. ED
	vpxor	xmm11, xmm13, xmm12			; 099D _ C4 41 11: EF. DC
	vpxor	xmm13, xmm10, xmm13			; 09A2 _ C4 41 29: EF. ED
	vpor	xmm9, xmm13, xmm9			; 09A7 _ C4 41 11: EB. C9
	vpxor	xmm10, xmm4, xmm9			; 09AC _ C4 41 59: EF. D1
	vmovd	xmm4, dword [r12+1418H] 		; 09B1 _ C4 C1 79: 6E. A4 24, 00001418
	vmovd	xmm12, dword [r12+141CH]		; 09BB _ C4 41 79: 6E. A4 24, 0000141C
	vpshufd xmm13, xmm4, 0				; 09C5 _ C5 79: 70. EC, 00
	vpshufd xmm4, xmm12, 0				; 09CA _ C4 C1 79: 70. E4, 00
	vpxor	xmm9, xmm10, xmm13			; 09D0 _ C4 41 29: EF. CD
	vpxor	xmm13, xmm8, xmm4			; 09D5 _ C5 39: EF. EC
	vpslld	xmm8, xmm9, 10				; 09D9 _ C4 C1 39: 72. F1, 0A
	vpsrld	xmm10, xmm9, 22 			; 09DF _ C4 C1 29: 72. D1, 16
	vpslld	xmm9, xmm5, 27				; 09E5 _ C5 B1: 72. F5, 1B
	vpor	xmm8, xmm8, xmm10			; 09EA _ C4 41 39: EB. C2
	vpsrld	xmm5, xmm5, 5				; 09EF _ C5 D1: 72. D5, 05
	vpxor	xmm12, xmm8, xmm13			; 09F4 _ C4 41 39: EF. E5
	vpor	xmm8, xmm9, xmm5			; 09F9 _ C5 31: EB. C5
	vpxor	xmm10, xmm8, xmm11			; 09FD _ C4 41 39: EF. D3
	vpslld	xmm4, xmm11, 7				; 0A02 _ C4 C1 59: 72. F3, 07
	vpxor	xmm8, xmm10, xmm13			; 0A08 _ C4 41 29: EF. C5
	vpslld	xmm5, xmm13, 25 			; 0A0D _ C4 C1 51: 72. F5, 19
	vpsrld	xmm13, xmm13, 7 			; 0A13 _ C4 C1 11: 72. D5, 07
	vpxor	xmm12, xmm12, xmm4			; 0A19 _ C5 19: EF. E4
	vpor	xmm4, xmm5, xmm13			; 0A1D _ C4 C1 51: EB. E5
	vpslld	xmm10, xmm8, 3				; 0A22 _ C4 C1 29: 72. F0, 03
	vpxor	xmm9, xmm4, xmm12			; 0A28 _ C4 41 59: EF. CC
	vpslld	xmm5, xmm11, 31 			; 0A2D _ C4 C1 51: 72. F3, 1F
	vpsrld	xmm11, xmm11, 1 			; 0A33 _ C4 C1 21: 72. D3, 01
	vpxor	xmm13, xmm9, xmm10			; 0A39 _ C4 41 31: EF. EA
	vpor	xmm10, xmm5, xmm11			; 0A3E _ C4 41 51: EB. D3
	vpslld	xmm11, xmm8, 19 			; 0A43 _ C4 C1 21: 72. F0, 13
	vpxor	xmm5, xmm10, xmm8			; 0A49 _ C4 C1 29: EF. E8
	vpsrld	xmm8, xmm8, 13				; 0A4E _ C4 C1 39: 72. D0, 0D
	vpxor	xmm9, xmm5, xmm12			; 0A54 _ C4 41 51: EF. CC
	vpslld	xmm4, xmm12, 29 			; 0A59 _ C4 C1 59: 72. F4, 1D
	vpsrld	xmm12, xmm12, 3 			; 0A5F _ C4 C1 19: 72. D4, 03
	vpor	xmm5, xmm11, xmm8			; 0A65 _ C4 C1 21: EB. E8
	vpor	xmm8, xmm4, xmm12			; 0A6A _ C4 41 59: EB. C4
	vpxor	xmm10, xmm8, xmm6			; 0A6F _ C5 39: EF. D6
	vpxor	xmm8, xmm9, xmm6			; 0A73 _ C5 31: EF. C6
	vpor	xmm9, xmm9, xmm5			; 0A77 _ C5 31: EB. CD
	vpxor	xmm5, xmm5, xmm8			; 0A7B _ C4 C1 51: EF. E8
	vpxor	xmm11, xmm9, xmm10			; 0A80 _ C4 41 31: EF. DA
	vpor	xmm10, xmm10, xmm8			; 0A85 _ C4 41 29: EB. D0
	vpxor	xmm9, xmm11, xmm13			; 0A8A _ C4 41 21: EF. CD
	vpxor	xmm12, xmm10, xmm5			; 0A8F _ C5 29: EF. E5
	vpand	xmm10, xmm5, xmm13			; 0A93 _ C4 41 51: DB. D5
	vpxor	xmm4, xmm8, xmm10			; 0A98 _ C4 C1 39: EF. E2
	vpor	xmm8, xmm10, xmm9			; 0A9D _ C4 41 29: EB. C1
	vmovd	xmm10, dword [r12+1400H]		; 0AA2 _ C4 41 79: 6E. 94 24, 00001400
	vpxor	xmm11, xmm8, xmm12			; 0AAC _ C4 41 39: EF. DC
	vpxor	xmm13, xmm13, xmm4			; 0AB1 _ C5 11: EF. EC
	vpxor	xmm8, xmm13, xmm11			; 0AB5 _ C4 41 11: EF. C3
	vpshufd xmm13, xmm10, 0 			; 0ABA _ C4 41 79: 70. EA, 00
	vpxor	xmm5, xmm8, xmm9			; 0AC0 _ C4 C1 39: EF. E9
	vpxor	xmm8, xmm11, xmm13			; 0AC5 _ C4 41 21: EF. C5
	vpxor	xmm11, xmm12, xmm9			; 0ACA _ C4 41 19: EF. D9
	vpand	xmm12, xmm11, xmm5			; 0ACF _ C5 21: DB. E5
	vmovd	xmm11, dword [r12+1408H]		; 0AD3 _ C4 41 79: 6E. 9C 24, 00001408
	vpxor	xmm10, xmm4, xmm12			; 0ADD _ C4 41 59: EF. D4
	vmovd	xmm4, dword [r12+1404H] 		; 0AE2 _ C4 C1 79: 6E. A4 24, 00001404
	vpshufd xmm13, xmm4, 0				; 0AEC _ C5 79: 70. EC, 00
	vpshufd xmm4, xmm11, 0				; 0AF1 _ C4 C1 79: 70. E3, 00
	vpxor	xmm12, xmm10, xmm13			; 0AF7 _ C4 41 29: EF. E5
	vpxor	xmm13, xmm9, xmm4			; 0AFC _ C5 31: EF. EC
	vpslld	xmm4, xmm12, 7				; 0B00 _ C4 C1 59: 72. F4, 07
	vmovd	xmm9, dword [r12+140CH] 		; 0B06 _ C4 41 79: 6E. 8C 24, 0000140C
	vpshufd xmm10, xmm9, 0				; 0B10 _ C4 41 79: 70. D1, 00
	vpslld	xmm9, xmm8, 27				; 0B16 _ C4 C1 31: 72. F0, 1B
	vpxor	xmm10, xmm5, xmm10			; 0B1C _ C4 41 51: EF. D2
	vpslld	xmm5, xmm13, 10 			; 0B21 _ C4 C1 51: 72. F5, 0A
	vpsrld	xmm13, xmm13, 22			; 0B27 _ C4 C1 11: 72. D5, 16
	vpsrld	xmm8, xmm8, 5				; 0B2D _ C4 C1 39: 72. D0, 05
	vpor	xmm5, xmm5, xmm13			; 0B33 _ C4 C1 51: EB. ED
	vpor	xmm8, xmm9, xmm8			; 0B38 _ C4 41 31: EB. C0
	vpxor	xmm11, xmm5, xmm10			; 0B3D _ C4 41 51: EF. DA
	vpxor	xmm5, xmm8, xmm12			; 0B42 _ C4 C1 39: EF. EC
	vpxor	xmm5, xmm5, xmm10			; 0B47 _ C4 C1 51: EF. EA
	vpslld	xmm13, xmm10, 25			; 0B4C _ C4 C1 11: 72. F2, 19
	vpsrld	xmm10, xmm10, 7 			; 0B52 _ C4 C1 29: 72. D2, 07
	vpxor	xmm11, xmm11, xmm4			; 0B58 _ C5 21: EF. DC
	vpor	xmm4, xmm13, xmm10			; 0B5C _ C4 C1 11: EB. E2
	vpslld	xmm8, xmm5, 3				; 0B61 _ C5 B9: 72. F5, 03
	vpxor	xmm9, xmm4, xmm11			; 0B66 _ C4 41 59: EF. CB
	vpslld	xmm13, xmm12, 31			; 0B6B _ C4 C1 11: 72. F4, 1F
	vpsrld	xmm12, xmm12, 1 			; 0B71 _ C4 C1 19: 72. D4, 01
	vpxor	xmm10, xmm9, xmm8			; 0B77 _ C4 41 31: EF. D0
	vpor	xmm8, xmm13, xmm12			; 0B7C _ C4 41 11: EB. C4
	vpslld	xmm4, xmm11, 29 			; 0B81 _ C4 C1 59: 72. F3, 1D
	vpxor	xmm13, xmm8, xmm5			; 0B87 _ C5 39: EF. ED
	vpxor	xmm12, xmm13, xmm11			; 0B8B _ C4 41 11: EF. E3
	vpsrld	xmm11, xmm11, 3 			; 0B90 _ C4 C1 21: 72. D3, 03
	vpslld	xmm13, xmm5, 19 			; 0B96 _ C5 91: 72. F5, 13
	vpsrld	xmm5, xmm5, 13				; 0B9B _ C5 D1: 72. D5, 0D
	vpor	xmm8, xmm4, xmm11			; 0BA0 _ C4 41 59: EB. C3
	vpor	xmm11, xmm13, xmm5			; 0BA5 _ C5 11: EB. DD
	vpor	xmm5, xmm8, xmm10			; 0BA9 _ C4 C1 39: EB. EA
	vpxor	xmm8, xmm8, xmm11			; 0BAE _ C4 41 39: EF. C3
	vpand	xmm13, xmm11, xmm10			; 0BB3 _ C4 41 21: DB. EA
	vpxor	xmm9, xmm8, xmm6			; 0BB8 _ C5 39: EF. CE
	vpxor	xmm4, xmm13, xmm9			; 0BBC _ C4 C1 11: EF. E1
	vpand	xmm9, xmm9, xmm5			; 0BC1 _ C5 31: DB. CD
	vpxor	xmm10, xmm10, xmm12			; 0BC5 _ C4 41 29: EF. D4
	vpor	xmm12, xmm12, xmm13			; 0BCA _ C4 41 19: EB. E5
	vpand	xmm8, xmm10, xmm5			; 0BCF _ C5 29: DB. C5
	vpxor	xmm10, xmm12, xmm9			; 0BD3 _ C4 41 19: EF. D1
	vpxor	xmm13, xmm9, xmm4			; 0BD8 _ C5 31: EF. EC
	vpxor	xmm12, xmm5, xmm10			; 0BDC _ C4 41 51: EF. E2
	vpor	xmm4, xmm4, xmm13			; 0BE1 _ C4 C1 59: EB. E5
	vmovd	xmm11, dword [r12+13F0H]		; 0BE6 _ C4 41 79: 6E. 9C 24, 000013F0
	vpxor	xmm5, xmm4, xmm8			; 0BF0 _ C4 C1 59: EF. E8
	vpxor	xmm8, xmm8, xmm12			; 0BF5 _ C4 41 39: EF. C4
	vpshufd xmm9, xmm11, 0				; 0BFA _ C4 41 79: 70. CB, 00
	vpxor	xmm4, xmm8, xmm13			; 0C00 _ C4 C1 39: EF. E5
	vmovd	xmm8, dword [r12+13F4H] 		; 0C05 _ C4 41 79: 6E. 84 24, 000013F4
	vpxor	xmm9, xmm4, xmm9			; 0C0F _ C4 41 59: EF. C9
	vmovd	xmm4, dword [r12+13F8H] 		; 0C14 _ C4 C1 79: 6E. A4 24, 000013F8
	vpshufd xmm11, xmm8, 0				; 0C1E _ C4 41 79: 70. D8, 00
	vpshufd xmm8, xmm4, 0				; 0C24 _ C5 79: 70. C4, 00
	vpxor	xmm11, xmm5, xmm11			; 0C29 _ C4 41 51: EF. DB
	vpxor	xmm4, xmm10, xmm8			; 0C2E _ C4 C1 29: EF. E0
	vpor	xmm10, xmm12, xmm5			; 0C33 _ C5 19: EB. D5
	vmovd	xmm5, dword [r12+13FCH] 		; 0C37 _ C4 C1 79: 6E. AC 24, 000013FC
	vpxor	xmm13, xmm10, xmm13			; 0C41 _ C4 41 29: EF. ED
	vpshufd xmm12, xmm5, 0				; 0C46 _ C5 79: 70. E5, 00
	vpslld	xmm8, xmm4, 10				; 0C4B _ C5 B9: 72. F4, 0A
	vpsrld	xmm10, xmm4, 22 			; 0C50 _ C5 A9: 72. D4, 16
	vpxor	xmm5, xmm13, xmm12			; 0C55 _ C4 C1 11: EF. EC
	vpor	xmm13, xmm8, xmm10			; 0C5A _ C4 41 39: EB. EA
	vpslld	xmm8, xmm9, 27				; 0C5F _ C4 C1 39: 72. F1, 1B
	vpsrld	xmm9, xmm9, 5				; 0C65 _ C4 C1 31: 72. D1, 05
	vpxor	xmm12, xmm13, xmm5			; 0C6B _ C5 11: EF. E5
	vpor	xmm8, xmm8, xmm9			; 0C6F _ C4 41 39: EB. C1
	vpslld	xmm4, xmm11, 7				; 0C74 _ C4 C1 59: 72. F3, 07
	vpxor	xmm10, xmm8, xmm11			; 0C7A _ C4 41 39: EF. D3
	vpslld	xmm13, xmm5, 25 			; 0C7F _ C5 91: 72. F5, 19
	vpxor	xmm8, xmm10, xmm5			; 0C84 _ C5 29: EF. C5
	vpsrld	xmm5, xmm5, 7				; 0C88 _ C5 D1: 72. D5, 07
	vpxor	xmm12, xmm12, xmm4			; 0C8D _ C5 19: EF. E4
	vpor	xmm10, xmm13, xmm5			; 0C91 _ C5 11: EB. D5
	vpslld	xmm9, xmm11, 31 			; 0C95 _ C4 C1 31: 72. F3, 1F
	vpsrld	xmm11, xmm11, 1 			; 0C9B _ C4 C1 21: 72. D3, 01
	vpxor	xmm5, xmm10, xmm12			; 0CA1 _ C4 C1 29: EF. EC
	vpslld	xmm13, xmm8, 3				; 0CA6 _ C4 C1 11: 72. F0, 03
	vpor	xmm10, xmm9, xmm11			; 0CAC _ C4 41 31: EB. D3
	vpxor	xmm4, xmm5, xmm13			; 0CB1 _ C4 C1 51: EF. E5
	vpxor	xmm5, xmm10, xmm8			; 0CB6 _ C4 C1 29: EF. E8
	vpslld	xmm13, xmm12, 29			; 0CBB _ C4 C1 11: 72. F4, 1D
	vpxor	xmm5, xmm5, xmm12			; 0CC1 _ C4 C1 51: EF. EC
	vpsrld	xmm11, xmm12, 3 			; 0CC6 _ C4 C1 21: 72. D4, 03
	vpslld	xmm12, xmm8, 19 			; 0CCC _ C4 C1 19: 72. F0, 13
	vpsrld	xmm8, xmm8, 13				; 0CD2 _ C4 C1 39: 72. D0, 0D
	vpor	xmm11, xmm13, xmm11			; 0CD8 _ C4 41 11: EB. DB
	vpor	xmm10, xmm12, xmm8			; 0CDD _ C4 41 19: EB. D0
	vpxor	xmm10, xmm10, xmm11			; 0CE2 _ C4 41 29: EF. D3
	vpxor	xmm13, xmm4, xmm5			; 0CE7 _ C5 59: EF. ED
	vpand	xmm12, xmm11, xmm10			; 0CEB _ C4 41 21: DB. E2
	vpxor	xmm4, xmm11, xmm4			; 0CF0 _ C5 A1: EF. E4
	vpxor	xmm9, xmm12, xmm6			; 0CF4 _ C5 19: EF. CE
	vpor	xmm11, xmm4, xmm10			; 0CF8 _ C4 41 59: EB. DA
	vpxor	xmm8, xmm9, xmm13			; 0CFD _ C4 41 31: EF. C5
	vpxor	xmm13, xmm13, xmm11			; 0D02 _ C4 41 11: EF. EB
	vmovd	xmm4, dword [r12+13E0H] 		; 0D07 _ C4 C1 79: 6E. A4 24, 000013E0
	vpxor	xmm9, xmm10, xmm8			; 0D11 _ C4 41 29: EF. C8
	vpand	xmm10, xmm5, xmm13			; 0D16 _ C4 41 51: DB. D5
	vpxor	xmm5, xmm11, xmm5			; 0D1B _ C5 A1: EF. ED
	vpxor	xmm12, xmm10, xmm9			; 0D1F _ C4 41 29: EF. E1
	vpxor	xmm11, xmm9, xmm13			; 0D24 _ C4 41 31: EF. DD
	vpshufd xmm10, xmm4, 0				; 0D29 _ C5 79: 70. D4, 00
	vmovd	xmm4, dword [r12+13E4H] 		; 0D2E _ C4 C1 79: 6E. A4 24, 000013E4
	vpxor	xmm10, xmm12, xmm10			; 0D38 _ C4 41 19: EF. D2
	vpshufd xmm4, xmm4, 0				; 0D3D _ C5 F9: 70. E4, 00
	vpxor	xmm12, xmm13, xmm12			; 0D42 _ C4 41 11: EF. E4
	vpxor	xmm4, xmm8, xmm4			; 0D47 _ C5 B9: EF. E4
	vpor	xmm8, xmm11, xmm8			; 0D4B _ C4 41 21: EB. C0
	vpxor	xmm9, xmm5, xmm8			; 0D50 _ C4 41 51: EF. C8
	vmovd	xmm5, dword [r12+13E8H] 		; 0D55 _ C4 C1 79: 6E. AC 24, 000013E8
	vpshufd xmm8, xmm5, 0				; 0D5F _ C5 79: 70. C5, 00
	vmovd	xmm13, dword [r12+13ECH]		; 0D64 _ C4 41 79: 6E. AC 24, 000013EC
	vpxor	xmm8, xmm9, xmm8			; 0D6E _ C4 41 31: EF. C0
	vpshufd xmm5, xmm13, 0				; 0D73 _ C4 C1 79: 70. ED, 00
	vpslld	xmm11, xmm8, 10 			; 0D79 _ C4 C1 21: 72. F0, 0A
	vpsrld	xmm8, xmm8, 22				; 0D7F _ C4 C1 39: 72. D0, 16
	vpxor	xmm13, xmm12, xmm5			; 0D85 _ C5 19: EF. ED
	vpor	xmm12, xmm11, xmm8			; 0D89 _ C4 41 21: EB. E0
	vpslld	xmm8, xmm4, 7				; 0D8E _ C5 B9: 72. F4, 07
	vpxor	xmm9, xmm12, xmm13			; 0D93 _ C4 41 19: EF. CD
	vpslld	xmm5, xmm10, 27 			; 0D98 _ C4 C1 51: 72. F2, 1B
	vpsrld	xmm10, xmm10, 5 			; 0D9E _ C4 C1 29: 72. D2, 05
	vpxor	xmm9, xmm9, xmm8			; 0DA4 _ C4 41 31: EF. C8
	vpor	xmm8, xmm5, xmm10			; 0DA9 _ C4 41 51: EB. C2
	vpslld	xmm11, xmm13, 25			; 0DAE _ C4 C1 21: 72. F5, 19
	vpxor	xmm10, xmm8, xmm4			; 0DB4 _ C5 39: EF. D4
	vpxor	xmm5, xmm10, xmm13			; 0DB8 _ C4 C1 29: EF. ED
	vpsrld	xmm13, xmm13, 7 			; 0DBD _ C4 C1 11: 72. D5, 07
	vpor	xmm8, xmm11, xmm13			; 0DC3 _ C4 41 21: EB. C5
	vpslld	xmm13, xmm5, 3				; 0DC8 _ C5 91: 72. F5, 03
	vpxor	xmm10, xmm8, xmm9			; 0DCD _ C4 41 39: EF. D1
	vpslld	xmm11, xmm5, 19 			; 0DD2 _ C5 A1: 72. F5, 13
	vpxor	xmm8, xmm10, xmm13			; 0DD7 _ C4 41 29: EF. C5
	vpslld	xmm10, xmm4, 31 			; 0DDC _ C5 A9: 72. F4, 1F
	vpsrld	xmm4, xmm4, 1				; 0DE1 _ C5 D9: 72. D4, 01
	vpsrld	xmm12, xmm5, 13 			; 0DE6 _ C5 99: 72. D5, 0D
	vpor	xmm10, xmm10, xmm4			; 0DEB _ C5 29: EB. D4
	vpor	xmm13, xmm11, xmm12			; 0DEF _ C4 41 21: EB. EC
	vpxor	xmm5, xmm10, xmm5			; 0DF4 _ C5 A9: EF. ED
	vpslld	xmm10, xmm9, 29 			; 0DF8 _ C4 C1 29: 72. F1, 1D
	vpxor	xmm11, xmm5, xmm9			; 0DFE _ C4 41 51: EF. D9
	vpsrld	xmm9, xmm9, 3				; 0E03 _ C4 C1 31: 72. D1, 03
	vpxor	xmm4, xmm11, xmm6			; 0E09 _ C5 A1: EF. E6
	vpor	xmm10, xmm10, xmm9			; 0E0D _ C4 41 29: EB. D1
	vpxor	xmm11, xmm10, xmm4			; 0E12 _ C5 29: EF. DC
	vpor	xmm5, xmm8, xmm13			; 0E16 _ C4 C1 39: EB. ED
	vpxor	xmm5, xmm5, xmm11			; 0E1B _ C4 C1 51: EF. EB
	vpxor	xmm12, xmm8, xmm5			; 0E20 _ C5 39: EF. E5
	vpor	xmm8, xmm11, xmm4			; 0E24 _ C5 21: EB. C4
	vpand	xmm9, xmm8, xmm13			; 0E28 _ C4 41 39: DB. CD
	vpxor	xmm8, xmm9, xmm12			; 0E2D _ C4 41 31: EF. C4
	vpor	xmm12, xmm12, xmm13			; 0E32 _ C4 41 19: EB. E5
	vpand	xmm10, xmm4, xmm8			; 0E37 _ C4 41 59: DB. D0
	vpxor	xmm4, xmm12, xmm4			; 0E3C _ C5 99: EF. E4
	vmovd	xmm12, dword [r12+13D0H]		; 0E40 _ C4 41 79: 6E. A4 24, 000013D0
	vpxor	xmm11, xmm10, xmm5			; 0E4A _ C5 29: EF. DD
	vpxor	xmm4, xmm4, xmm8			; 0E4E _ C4 C1 59: EF. E0
	vpshufd xmm9, xmm12, 0				; 0E53 _ C4 41 79: 70. CC, 00
	vpxor	xmm10, xmm4, xmm11			; 0E59 _ C4 41 59: EF. D3
	vpand	xmm5, xmm5, xmm4			; 0E5E _ C5 D1: DB. EC
	vpxor	xmm12, xmm11, xmm9			; 0E62 _ C4 41 21: EF. E1
	vpxor	xmm9, xmm10, xmm6			; 0E67 _ C5 29: EF. CE
	vpxor	xmm10, xmm5, xmm10			; 0E6B _ C4 41 51: EF. D2
	vmovd	xmm11, dword [r12+13D4H]		; 0E70 _ C4 41 79: 6E. 9C 24, 000013D4
	vpxor	xmm10, xmm10, xmm13			; 0E7A _ C4 41 29: EF. D5
	vmovd	xmm13, dword [r12+13D8H]		; 0E7F _ C4 41 79: 6E. AC 24, 000013D8
	vpshufd xmm11, xmm11, 0 			; 0E89 _ C4 41 79: 70. DB, 00
	vpshufd xmm5, xmm13, 0				; 0E8F _ C4 C1 79: 70. ED, 00
	vpxor	xmm9, xmm9, xmm11			; 0E95 _ C4 41 31: EF. CB
	vmovd	xmm13, dword [r12+13DCH]		; 0E9A _ C4 41 79: 6E. AC 24, 000013DC
	vpxor	xmm4, xmm10, xmm5			; 0EA4 _ C5 A9: EF. E5
	vpshufd xmm11, xmm13, 0 			; 0EA8 _ C4 41 79: 70. DD, 00
	vpsrld	xmm10, xmm4, 22 			; 0EAE _ C5 A9: 72. D4, 16
	vpxor	xmm5, xmm8, xmm11			; 0EB3 _ C4 C1 39: EF. EB
	vpslld	xmm8, xmm4, 10				; 0EB8 _ C5 B9: 72. F4, 0A
	vpor	xmm8, xmm8, xmm10			; 0EBD _ C4 41 39: EB. C2
	vpslld	xmm4, xmm12, 27 			; 0EC2 _ C4 C1 59: 72. F4, 1B
	vpsrld	xmm12, xmm12, 5 			; 0EC8 _ C4 C1 19: 72. D4, 05
	vpxor	xmm13, xmm8, xmm5			; 0ECE _ C5 39: EF. ED
	vpslld	xmm11, xmm9, 7				; 0ED2 _ C4 C1 21: 72. F1, 07
	vpor	xmm8, xmm4, xmm12			; 0ED8 _ C4 41 59: EB. C4
	vpxor	xmm10, xmm13, xmm11			; 0EDD _ C4 41 11: EF. D3
	vpxor	xmm13, xmm8, xmm9			; 0EE2 _ C4 41 39: EF. E9
	vpxor	xmm4, xmm13, xmm5			; 0EE7 _ C5 91: EF. E5
	vpslld	xmm11, xmm5, 25 			; 0EEB _ C5 A1: 72. F5, 19
	vpsrld	xmm5, xmm5, 7				; 0EF0 _ C5 D1: 72. D5, 07
	vpslld	xmm13, xmm9, 31 			; 0EF5 _ C4 C1 11: 72. F1, 1F
	vpor	xmm12, xmm11, xmm5			; 0EFB _ C5 21: EB. E5
	vpslld	xmm5, xmm4, 3				; 0EFF _ C5 D1: 72. F4, 03
	vpxor	xmm8, xmm12, xmm10			; 0F04 _ C4 41 19: EF. C2
	vpsrld	xmm9, xmm9, 1				; 0F09 _ C4 C1 31: 72. D1, 01
	vpxor	xmm5, xmm8, xmm5			; 0F0F _ C5 B9: EF. ED
	vpor	xmm8, xmm13, xmm9			; 0F13 _ C4 41 11: EB. C1
	vpxor	xmm13, xmm8, xmm4			; 0F18 _ C5 39: EF. EC
	vpslld	xmm11, xmm10, 29			; 0F1C _ C4 C1 21: 72. F2, 1D
	vpxor	xmm8, xmm13, xmm10			; 0F22 _ C4 41 11: EF. C2
	vpsrld	xmm10, xmm10, 3 			; 0F27 _ C4 C1 29: 72. D2, 03
	vpor	xmm12, xmm11, xmm10			; 0F2D _ C4 41 21: EB. E2
	vpslld	xmm9, xmm4, 19				; 0F32 _ C5 B1: 72. F4, 13
	vpsrld	xmm4, xmm4, 13				; 0F37 _ C5 D9: 72. D4, 0D
	vpand	xmm10, xmm12, xmm5			; 0F3C _ C5 19: DB. D5
	vpor	xmm11, xmm9, xmm4			; 0F40 _ C5 31: EB. DC
	vpxor	xmm9, xmm10, xmm8			; 0F44 _ C4 41 29: EF. C8
	vpor	xmm8, xmm8, xmm5			; 0F49 _ C5 39: EB. C5
	vpxor	xmm13, xmm12, xmm9			; 0F4D _ C4 41 19: EF. E9
	vpand	xmm8, xmm8, xmm11			; 0F52 _ C4 41 39: DB. C3
	vpxor	xmm12, xmm13, xmm8			; 0F57 _ C4 41 11: EF. E0
	vpxor	xmm13, xmm11, xmm6			; 0F5C _ C5 21: EF. EE
	vpxor	xmm4, xmm5, xmm12			; 0F60 _ C4 C1 51: EF. E4
	vpand	xmm5, xmm8, xmm9			; 0F65 _ C4 C1 39: DB. E9
	vpxor	xmm8, xmm5, xmm4			; 0F6A _ C5 51: EF. C4
	vpand	xmm10, xmm4, xmm13			; 0F6E _ C4 41 59: DB. D5
	vpxor	xmm11, xmm13, xmm8			; 0F73 _ C4 41 11: EF. D8
	vpxor	xmm5, xmm10, xmm9			; 0F78 _ C4 C1 29: EF. E9
	vmovd	xmm13, dword [r12+13C0H]		; 0F7D _ C4 41 79: 6E. AC 24, 000013C0
	vpxor	xmm4, xmm5, xmm11			; 0F87 _ C4 C1 51: EF. E3
	vpshufd xmm10, xmm13, 0 			; 0F8C _ C4 41 79: 70. D5, 00
	vmovd	xmm13, dword [r12+13C4H]		; 0F92 _ C4 41 79: 6E. AC 24, 000013C4
	vpxor	xmm5, xmm11, xmm10			; 0F9C _ C4 C1 21: EF. EA
	vpxor	xmm10, xmm4, xmm11			; 0FA1 _ C4 41 59: EF. D3
	vpand	xmm11, xmm9, xmm11			; 0FA6 _ C4 41 31: DB. DB
	vpshufd xmm13, xmm13, 0 			; 0FAB _ C4 41 79: 70. ED, 00
	vpxor	xmm9, xmm11, xmm12			; 0FB1 _ C4 41 21: EF. CC
	vpxor	xmm13, xmm10, xmm13			; 0FB6 _ C4 41 29: EF. ED
	vpor	xmm10, xmm9, xmm4			; 0FBB _ C5 31: EB. D4
	vpxor	xmm11, xmm10, xmm8			; 0FBF _ C4 41 29: EF. D8
	vmovd	xmm8, dword [r12+13C8H] 		; 0FC4 _ C4 41 79: 6E. 84 24, 000013C8
	vmovd	xmm9, dword [r12+13CCH] 		; 0FCE _ C4 41 79: 6E. 8C 24, 000013CC
	vpshufd xmm4, xmm8, 0				; 0FD8 _ C4 C1 79: 70. E0, 00
	vpshufd xmm8, xmm9, 0				; 0FDE _ C4 41 79: 70. C1, 00
	vpxor	xmm10, xmm11, xmm4			; 0FE4 _ C5 21: EF. D4
	vpxor	xmm11, xmm12, xmm8			; 0FE8 _ C4 41 19: EF. D8
	vpslld	xmm12, xmm10, 10			; 0FED _ C4 C1 19: 72. F2, 0A
	vpsrld	xmm8, xmm10, 22 			; 0FF3 _ C4 C1 39: 72. D2, 16
	vpslld	xmm9, xmm13, 7				; 0FF9 _ C4 C1 31: 72. F5, 07
	vpor	xmm10, xmm12, xmm8			; 0FFF _ C4 41 19: EB. D0
	vpslld	xmm12, xmm5, 27 			; 1004 _ C5 99: 72. F5, 1B
	vpsrld	xmm5, xmm5, 5				; 1009 _ C5 D1: 72. D5, 05
	vpxor	xmm4, xmm10, xmm11			; 100E _ C4 C1 29: EF. E3
	vpor	xmm10, xmm12, xmm5			; 1013 _ C5 19: EB. D5
	vpxor	xmm8, xmm4, xmm9			; 1017 _ C4 41 59: EF. C1
	vpxor	xmm5, xmm10, xmm13			; 101C _ C4 C1 29: EF. ED
	vpslld	xmm4, xmm11, 25 			; 1021 _ C4 C1 59: 72. F3, 19
	vpxor	xmm10, xmm5, xmm11			; 1027 _ C4 41 51: EF. D3
	vpsrld	xmm11, xmm11, 7 			; 102C _ C4 C1 21: 72. D3, 07
	vpor	xmm5, xmm4, xmm11			; 1032 _ C4 C1 59: EB. EB
	vpslld	xmm9, xmm10, 3				; 1037 _ C4 C1 31: 72. F2, 03
	vpxor	xmm4, xmm5, xmm8			; 103D _ C4 C1 51: EF. E0
	vpslld	xmm11, xmm13, 31			; 1042 _ C4 C1 21: 72. F5, 1F
	vpsrld	xmm13, xmm13, 1 			; 1048 _ C4 C1 11: 72. D5, 01
	vpxor	xmm5, xmm4, xmm9			; 104E _ C4 C1 59: EF. E9
	vpor	xmm4, xmm11, xmm13			; 1053 _ C4 C1 21: EB. E5
	vpxor	xmm9, xmm4, xmm10			; 1058 _ C4 41 59: EF. CA
	vpxor	xmm4, xmm9, xmm8			; 105D _ C4 C1 31: EF. E0
	vpslld	xmm9, xmm8, 29				; 1062 _ C4 C1 31: 72. F0, 1D
	vpsrld	xmm13, xmm8, 3				; 1068 _ C4 C1 11: 72. D0, 03
	vpslld	xmm12, xmm10, 19			; 106E _ C4 C1 19: 72. F2, 13
	vpor	xmm11, xmm9, xmm13			; 1074 _ C4 41 31: EB. DD
	vpsrld	xmm10, xmm10, 13			; 1079 _ C4 C1 29: 72. D2, 0D
	vpxor	xmm9, xmm11, xmm4			; 107F _ C5 21: EF. CC
	vpor	xmm8, xmm12, xmm10			; 1083 _ C4 41 19: EB. C2
	vpxor	xmm8, xmm8, xmm9			; 1088 _ C4 41 39: EF. C1
	vpand	xmm11, xmm11, xmm9			; 108D _ C4 41 21: DB. D9
	vpxor	xmm10, xmm11, xmm8			; 1092 _ C4 41 21: EF. D0
	vpor	xmm13, xmm5, xmm10			; 1097 _ C4 41 51: EB. EA
	vpxor	xmm11, xmm9, xmm13			; 109C _ C4 41 31: EF. DD
	vpand	xmm9, xmm8, xmm4			; 10A1 _ C5 39: DB. CC
	vpxor	xmm4, xmm4, xmm5			; 10A5 _ C5 D9: EF. E5
	vpxor	xmm12, xmm9, xmm13			; 10A9 _ C4 41 31: EF. E5
	vpxor	xmm8, xmm4, xmm10			; 10AE _ C4 41 59: EF. C2
	vpand	xmm5, xmm13, xmm11			; 10B3 _ C4 C1 11: DB. EB
	vpxor	xmm4, xmm8, xmm12			; 10B8 _ C4 C1 39: EF. E4
	vpxor	xmm5, xmm5, xmm8			; 10BD _ C4 C1 51: EF. E8
	vmovd	xmm13, dword [r12+13B0H]		; 10C2 _ C4 41 79: 6E. AC 24, 000013B0
	vpor	xmm9, xmm4, xmm11			; 10CC _ C4 41 59: EB. CB
	vpxor	xmm4, xmm9, xmm10			; 10D1 _ C4 C1 31: EF. E2
	vpxor	xmm12, xmm12, xmm5			; 10D6 _ C5 19: EF. E5
	vpshufd xmm10, xmm13, 0 			; 10DA _ C4 41 79: 70. D5, 00
	vpxor	xmm8, xmm11, xmm10			; 10E0 _ C4 41 21: EF. C2
	vmovd	xmm11, dword [r12+13B4H]		; 10E5 _ C4 41 79: 6E. 9C 24, 000013B4
	vmovd	xmm10, dword [r12+13B8H]		; 10EF _ C4 41 79: 6E. 94 24, 000013B8
	vpshufd xmm9, xmm11, 0				; 10F9 _ C4 41 79: 70. CB, 00
	vpshufd xmm11, xmm10, 0 			; 10FF _ C4 41 79: 70. DA, 00
	vpxor	xmm13, xmm4, xmm9			; 1105 _ C4 41 59: EF. E9
	vpxor	xmm11, xmm5, xmm11			; 110A _ C4 41 51: EF. DB
	vpxor	xmm5, xmm12, xmm4			; 110F _ C5 99: EF. EC
	vmovd	xmm4, dword [r12+13BCH] 		; 1113 _ C4 C1 79: 6E. A4 24, 000013BC
	vpslld	xmm12, xmm11, 10			; 111D _ C4 C1 19: 72. F3, 0A
	vpshufd xmm9, xmm4, 0				; 1123 _ C5 79: 70. CC, 00
	vpsrld	xmm11, xmm11, 22			; 1128 _ C4 C1 21: 72. D3, 16
	vpxor	xmm10, xmm5, xmm9			; 112E _ C4 41 51: EF. D1
	vpor	xmm11, xmm12, xmm11			; 1133 _ C4 41 19: EB. DB
	vpxor	xmm4, xmm11, xmm10			; 1138 _ C4 C1 21: EF. E2
	vpslld	xmm11, xmm8, 27 			; 113D _ C4 C1 21: 72. F0, 1B
	vpsrld	xmm8, xmm8, 5				; 1143 _ C4 C1 39: 72. D0, 05
	vpslld	xmm5, xmm13, 7				; 1149 _ C4 C1 51: 72. F5, 07
	vpor	xmm11, xmm11, xmm8			; 114F _ C4 41 21: EB. D8
	vpxor	xmm12, xmm4, xmm5			; 1154 _ C5 59: EF. E5
	vpxor	xmm8, xmm11, xmm13			; 1158 _ C4 41 21: EF. C5
	vpslld	xmm4, xmm10, 25 			; 115D _ C4 C1 59: 72. F2, 19
	vpxor	xmm8, xmm8, xmm10			; 1163 _ C4 41 39: EF. C2
	vpsrld	xmm10, xmm10, 7 			; 1168 _ C4 C1 29: 72. D2, 07
	vpor	xmm11, xmm4, xmm10			; 116E _ C4 41 59: EB. DA
	vpslld	xmm10, xmm13, 31			; 1173 _ C4 C1 29: 72. F5, 1F
	vpsrld	xmm13, xmm13, 1 			; 1179 _ C4 C1 11: 72. D5, 01
	vpxor	xmm4, xmm11, xmm12			; 117F _ C4 C1 21: EF. E4
	vpslld	xmm5, xmm8, 3				; 1184 _ C4 C1 51: 72. F0, 03
	vpor	xmm11, xmm10, xmm13			; 118A _ C4 41 29: EB. DD
	vpxor	xmm9, xmm4, xmm5			; 118F _ C5 59: EF. CD
	vpxor	xmm4, xmm11, xmm8			; 1193 _ C4 C1 21: EF. E0
	vpxor	xmm5, xmm4, xmm12			; 1198 _ C4 C1 59: EF. EC
	vpslld	xmm13, xmm8, 19 			; 119D _ C4 C1 11: 72. F0, 13
	vpsrld	xmm8, xmm8, 13				; 11A3 _ C4 C1 39: 72. D0, 0D
	vpslld	xmm10, xmm12, 29			; 11A9 _ C4 C1 29: 72. F4, 1D
	vpsrld	xmm12, xmm12, 3 			; 11AF _ C4 C1 19: 72. D4, 03
	vpor	xmm8, xmm13, xmm8			; 11B5 _ C4 41 11: EB. C0
	vpor	xmm11, xmm10, xmm12			; 11BA _ C4 41 29: EB. DC
	vpxor	xmm4, xmm11, xmm9			; 11BF _ C4 C1 21: EF. E1
	vpxor	xmm11, xmm9, xmm8			; 11C4 _ C4 41 31: EF. D8
	vpand	xmm9, xmm11, xmm4			; 11C9 _ C5 21: DB. CC
	vpxor	xmm9, xmm9, xmm5			; 11CD _ C5 31: EF. CD
	vpor	xmm5, xmm5, xmm4			; 11D1 _ C5 D1: EB. EC
	vpxor	xmm12, xmm5, xmm11			; 11D5 _ C4 41 51: EF. E3
	vpand	xmm11, xmm11, xmm9			; 11DA _ C4 41 21: DB. D9
	vpxor	xmm4, xmm4, xmm9			; 11DF _ C4 C1 59: EF. E1
	vpand	xmm5, xmm11, xmm8			; 11E4 _ C4 C1 21: DB. E8
	vmovd	xmm11, dword [r12+13A4H]		; 11E9 _ C4 41 79: 6E. 9C 24, 000013A4
	vpxor	xmm5, xmm5, xmm4			; 11F3 _ C5 D1: EF. EC
	vpxor	xmm13, xmm9, xmm6			; 11F7 _ C5 31: EF. EE
	vpand	xmm4, xmm4, xmm12			; 11FB _ C4 C1 59: DB. E4
	vmovd	xmm9, dword [r12+13A0H] 		; 1200 _ C4 41 79: 6E. 8C 24, 000013A0
	vpshufd xmm10, xmm9, 0				; 120A _ C4 41 79: 70. D1, 00
	vpshufd xmm9, xmm11, 0				; 1210 _ C4 41 79: 70. CB, 00
	vpor	xmm11, xmm4, xmm8			; 1216 _ C4 41 59: EB. D8
	vmovd	xmm4, dword [r12+13A8H] 		; 121B _ C4 C1 79: 6E. A4 24, 000013A8
	vpxor	xmm11, xmm11, xmm13			; 1225 _ C4 41 21: EF. DD
	vpshufd xmm4, xmm4, 0				; 122A _ C5 F9: 70. E4, 00
	vpxor	xmm8, xmm8, xmm13			; 122F _ C4 41 39: EF. C5
	vpxor	xmm10, xmm12, xmm10			; 1234 _ C4 41 19: EF. D2
	vpxor	xmm11, xmm11, xmm4			; 1239 _ C5 21: EF. DC
	vpxor	xmm4, xmm13, xmm5			; 123D _ C5 91: EF. E5
	vpand	xmm13, xmm8, xmm12			; 1241 _ C4 41 39: DB. EC
	vmovd	xmm12, dword [r12+13ACH]		; 1246 _ C4 41 79: 6E. A4 24, 000013AC
	vpxor	xmm8, xmm4, xmm13			; 1250 _ C4 41 59: EF. C5
	vpshufd xmm4, xmm12, 0				; 1255 _ C4 C1 79: 70. E4, 00
	vpxor	xmm9, xmm5, xmm9			; 125B _ C4 41 51: EF. C9
	vpxor	xmm4, xmm8, xmm4			; 1260 _ C5 B9: EF. E4
	vpslld	xmm5, xmm11, 10 			; 1264 _ C4 C1 51: 72. F3, 0A
	vpsrld	xmm11, xmm11, 22			; 126A _ C4 C1 21: 72. D3, 16
	vpslld	xmm8, xmm10, 27 			; 1270 _ C4 C1 39: 72. F2, 1B
	vpsrld	xmm10, xmm10, 5 			; 1276 _ C4 C1 29: 72. D2, 05
	vpor	xmm13, xmm5, xmm11			; 127C _ C4 41 51: EB. EB
	vpor	xmm8, xmm8, xmm10			; 1281 _ C4 41 39: EB. C2
	vpxor	xmm12, xmm13, xmm4			; 1286 _ C5 11: EF. E4
	vpslld	xmm11, xmm9, 7				; 128A _ C4 C1 21: 72. F1, 07
	vpxor	xmm5, xmm8, xmm9			; 1290 _ C4 C1 39: EF. E9
	vpxor	xmm11, xmm12, xmm11			; 1295 _ C4 41 19: EF. DB
	vpxor	xmm13, xmm5, xmm4			; 129A _ C5 51: EF. EC
	vpslld	xmm12, xmm4, 25 			; 129E _ C5 99: 72. F4, 19
	vpsrld	xmm4, xmm4, 7				; 12A3 _ C5 D9: 72. D4, 07
	vpor	xmm10, xmm12, xmm4			; 12A8 _ C5 19: EB. D4
	vpslld	xmm4, xmm13, 3				; 12AC _ C4 C1 59: 72. F5, 03
	vpxor	xmm8, xmm10, xmm11			; 12B2 _ C4 41 29: EF. C3
	vpslld	xmm5, xmm9, 31				; 12B7 _ C4 C1 51: 72. F1, 1F
	vpsrld	xmm9, xmm9, 1				; 12BD _ C4 C1 31: 72. D1, 01
	vpxor	xmm4, xmm8, xmm4			; 12C3 _ C5 B9: EF. E4
	vpor	xmm8, xmm5, xmm9			; 12C7 _ C4 41 51: EB. C1
	vpslld	xmm9, xmm11, 29 			; 12CC _ C4 C1 31: 72. F3, 1D
	vpxor	xmm5, xmm8, xmm13			; 12D2 _ C4 C1 39: EF. ED
	vpslld	xmm12, xmm13, 19			; 12D7 _ C4 C1 19: 72. F5, 13
	vpxor	xmm10, xmm5, xmm11			; 12DD _ C4 41 51: EF. D3
	vpsrld	xmm11, xmm11, 3 			; 12E2 _ C4 C1 21: 72. D3, 03
	vpsrld	xmm13, xmm13, 13			; 12E8 _ C4 C1 11: 72. D5, 0D
	vpxor	xmm5, xmm10, xmm4			; 12EE _ C5 A9: EF. EC
	vpor	xmm11, xmm9, xmm11			; 12F2 _ C4 41 31: EB. DB
	vpor	xmm9, xmm12, xmm13			; 12F7 _ C4 41 19: EB. CD
	vpand	xmm4, xmm4, xmm5			; 12FC _ C5 D9: DB. E5
	vpxor	xmm8, xmm10, xmm11			; 1300 _ C4 41 29: EF. C3
	vpxor	xmm10, xmm4, xmm9			; 1305 _ C4 41 59: EF. D1
	vpor	xmm4, xmm9, xmm5			; 130A _ C5 B1: EB. E5
	vpxor	xmm11, xmm11, xmm10			; 130E _ C4 41 21: EF. DA
	vpxor	xmm12, xmm5, xmm10			; 1313 _ C4 41 51: EF. E2
	vpxor	xmm5, xmm4, xmm8			; 1318 _ C4 C1 59: EF. E8
	vpor	xmm13, xmm12, xmm10			; 131D _ C4 41 19: EB. EA
	vpor	xmm9, xmm5, xmm11			; 1322 _ C4 41 51: EB. CB
	vpxor	xmm8, xmm8, xmm6			; 1327 _ C5 39: EF. C6
	vpxor	xmm4, xmm9, xmm12			; 132B _ C4 C1 31: EF. E4
	vmovd	xmm12, dword [r12+1390H]		; 1330 _ C4 41 79: 6E. A4 24, 00001390
	vpxor	xmm5, xmm13, xmm4			; 133A _ C5 91: EF. EC
	vmovd	xmm13, dword [r12+1394H]		; 133E _ C4 41 79: 6E. AC 24, 00001394
	vpxor	xmm8, xmm8, xmm5			; 1348 _ C5 39: EF. C5
	vpshufd xmm9, xmm12, 0				; 134C _ C4 41 79: 70. CC, 00
	vpor	xmm5, xmm5, xmm4			; 1352 _ C5 D1: EB. EC
	vpshufd xmm12, xmm13, 0 			; 1356 _ C4 41 79: 70. E5, 00
	vpxor	xmm9, xmm8, xmm9			; 135C _ C4 41 39: EF. C9
	vpxor	xmm12, xmm4, xmm12			; 1361 _ C4 41 59: EF. E4
	vpxor	xmm4, xmm5, xmm4			; 1366 _ C5 D1: EF. E4
	vpor	xmm8, xmm4, xmm8			; 136A _ C4 41 59: EB. C0
	vpxor	xmm8, xmm10, xmm8			; 136F _ C4 41 29: EF. C0
	vmovd	xmm10, dword [r12+1398H]		; 1374 _ C4 41 79: 6E. 94 24, 00001398
	vmovd	xmm5, dword [r12+139CH] 		; 137E _ C4 C1 79: 6E. AC 24, 0000139C
	vpshufd xmm4, xmm10, 0				; 1388 _ C4 C1 79: 70. E2, 00
	vpshufd xmm13, xmm5, 0				; 138E _ C5 79: 70. ED, 00
	vpxor	xmm10, xmm8, xmm4			; 1393 _ C5 39: EF. D4
	vpxor	xmm13, xmm11, xmm13			; 1397 _ C4 41 21: EF. ED
	vpslld	xmm11, xmm10, 10			; 139C _ C4 C1 21: 72. F2, 0A
	vpsrld	xmm8, xmm10, 22 			; 13A2 _ C4 C1 39: 72. D2, 16
	vpslld	xmm10, xmm9, 27 			; 13A8 _ C4 C1 29: 72. F1, 1B
	vpor	xmm11, xmm11, xmm8			; 13AE _ C4 41 21: EB. D8
	vpsrld	xmm9, xmm9, 5				; 13B3 _ C4 C1 31: 72. D1, 05
	vpxor	xmm4, xmm11, xmm13			; 13B9 _ C4 C1 21: EF. E5
	vpslld	xmm5, xmm12, 7				; 13BE _ C4 C1 51: 72. F4, 07
	vpor	xmm11, xmm10, xmm9			; 13C4 _ C4 41 29: EB. D9
	vpxor	xmm8, xmm4, xmm5			; 13C9 _ C5 59: EF. C5
	vpxor	xmm4, xmm11, xmm12			; 13CD _ C4 C1 21: EF. E4
	vpslld	xmm5, xmm13, 25 			; 13D2 _ C4 C1 51: 72. F5, 19
	vpsrld	xmm9, xmm13, 7				; 13D8 _ C4 C1 31: 72. D5, 07
	vpxor	xmm11, xmm4, xmm13			; 13DE _ C4 41 59: EF. DD
	vpor	xmm13, xmm5, xmm9			; 13E3 _ C4 41 51: EB. E9
	vpslld	xmm4, xmm11, 3				; 13E8 _ C4 C1 59: 72. F3, 03
	vpxor	xmm10, xmm13, xmm8			; 13EE _ C4 41 11: EF. D0
	vpslld	xmm5, xmm12, 31 			; 13F3 _ C4 C1 51: 72. F4, 1F
	vpsrld	xmm12, xmm12, 1 			; 13F9 _ C4 C1 19: 72. D4, 01
	vpxor	xmm13, xmm10, xmm4			; 13FF _ C5 29: EF. EC
	vpor	xmm4, xmm5, xmm12			; 1403 _ C4 C1 51: EB. E4
	vpslld	xmm9, xmm11, 19 			; 1408 _ C4 C1 31: 72. F3, 13
	vpxor	xmm5, xmm4, xmm11			; 140E _ C4 C1 59: EF. EB
	vpsrld	xmm11, xmm11, 13			; 1413 _ C4 C1 21: 72. D3, 0D
	vpxor	xmm4, xmm5, xmm8			; 1419 _ C4 C1 51: EF. E0
	vpslld	xmm12, xmm8, 29 			; 141E _ C4 C1 19: 72. F0, 1D
	vpsrld	xmm8, xmm8, 3				; 1424 _ C4 C1 39: 72. D0, 03
	vpor	xmm11, xmm9, xmm11			; 142A _ C4 41 31: EB. DB
	vpor	xmm10, xmm12, xmm8			; 142F _ C4 41 19: EB. D0
	vpor	xmm8, xmm4, xmm11			; 1434 _ C4 41 59: EB. C3
	vpxor	xmm5, xmm10, xmm6			; 1439 _ C5 A9: EF. EE
	vpxor	xmm10, xmm4, xmm6			; 143D _ C5 59: EF. D6
	vpxor	xmm4, xmm8, xmm5			; 1441 _ C5 B9: EF. E5
	vpxor	xmm9, xmm11, xmm10			; 1445 _ C4 41 21: EF. CA
	vpxor	xmm4, xmm4, xmm13			; 144A _ C4 C1 59: EF. E5
	vpor	xmm11, xmm5, xmm10			; 144F _ C4 41 51: EB. DA
	vpand	xmm5, xmm9, xmm13			; 1454 _ C4 C1 31: DB. ED
	vpxor	xmm12, xmm11, xmm9			; 1459 _ C4 41 21: EF. E1
	vpxor	xmm9, xmm10, xmm5			; 145E _ C5 29: EF. CD
	vpor	xmm11, xmm5, xmm4			; 1462 _ C5 51: EB. DC
	vmovd	xmm5, dword [r12+1380H] 		; 1466 _ C4 C1 79: 6E. AC 24, 00001380
	vpxor	xmm8, xmm11, xmm12			; 1470 _ C4 41 21: EF. C4
	vpxor	xmm13, xmm13, xmm9			; 1475 _ C4 41 11: EF. E9
	vpxor	xmm11, xmm13, xmm8			; 147A _ C4 41 11: EF. D8
	vpshufd xmm13, xmm5, 0				; 147F _ C5 79: 70. ED, 00
	vpxor	xmm11, xmm11, xmm4			; 1484 _ C5 21: EF. DC
	vpxor	xmm5, xmm8, xmm13			; 1488 _ C4 C1 39: EF. ED
	vpxor	xmm8, xmm12, xmm4			; 148D _ C5 19: EF. C4
	vpand	xmm12, xmm8, xmm11			; 1491 _ C4 41 39: DB. E3
	vpxor	xmm10, xmm9, xmm12			; 1496 _ C4 41 31: EF. D4
	vmovd	xmm9, dword [r12+1384H] 		; 149B _ C4 41 79: 6E. 8C 24, 00001384
	vpshufd xmm8, xmm9, 0				; 14A5 _ C4 41 79: 70. C1, 00
	vmovd	xmm9, dword [r12+1388H] 		; 14AB _ C4 41 79: 6E. 8C 24, 00001388
	vpxor	xmm13, xmm10, xmm8			; 14B5 _ C4 41 29: EF. E8
	vpshufd xmm12, xmm9, 0				; 14BA _ C4 41 79: 70. E1, 00
	vpxor	xmm9, xmm4, xmm12			; 14C0 _ C4 41 59: EF. CC
	vpslld	xmm12, xmm13, 7 			; 14C5 _ C4 C1 19: 72. F5, 07
	vmovd	xmm4, dword [r12+138CH] 		; 14CB _ C4 C1 79: 6E. A4 24, 0000138C
	vpshufd xmm10, xmm4, 0				; 14D5 _ C5 79: 70. D4, 00
	vpsrld	xmm4, xmm9, 22				; 14DA _ C4 C1 59: 72. D1, 16
	vpxor	xmm8, xmm11, xmm10			; 14E0 _ C4 41 21: EF. C2
	vpslld	xmm11, xmm9, 10 			; 14E5 _ C4 C1 21: 72. F1, 0A
	vpslld	xmm10, xmm5, 27 			; 14EB _ C5 A9: 72. F5, 1B
	vpsrld	xmm5, xmm5, 5				; 14F0 _ C5 D1: 72. D5, 05
	vpor	xmm11, xmm11, xmm4			; 14F5 _ C5 21: EB. DC
	vpor	xmm4, xmm10, xmm5			; 14F9 _ C5 A9: EB. E5
	vpxor	xmm9, xmm11, xmm8			; 14FD _ C4 41 21: EF. C8
	vpxor	xmm5, xmm4, xmm13			; 1502 _ C4 C1 59: EF. ED
	vpxor	xmm11, xmm9, xmm12			; 1507 _ C4 41 31: EF. DC
	vpxor	xmm9, xmm5, xmm8			; 150C _ C4 41 51: EF. C8
	vpslld	xmm12, xmm8, 25 			; 1511 _ C4 C1 19: 72. F0, 19
	vpsrld	xmm8, xmm8, 7				; 1517 _ C4 C1 39: 72. D0, 07
	vpor	xmm10, xmm12, xmm8			; 151D _ C4 41 19: EB. D0
	vpslld	xmm4, xmm9, 3				; 1522 _ C4 C1 59: 72. F1, 03
	vpxor	xmm8, xmm10, xmm11			; 1528 _ C4 41 29: EF. C3
	vpslld	xmm12, xmm13, 31			; 152D _ C4 C1 19: 72. F5, 1F
	vpsrld	xmm13, xmm13, 1 			; 1533 _ C4 C1 11: 72. D5, 01
	vpxor	xmm5, xmm8, xmm4			; 1539 _ C5 B9: EF. EC
	vpor	xmm8, xmm12, xmm13			; 153D _ C4 41 19: EB. C5
	vpslld	xmm13, xmm11, 29			; 1542 _ C4 C1 11: 72. F3, 1D
	vpxor	xmm4, xmm8, xmm9			; 1548 _ C4 C1 39: EF. E1
	vpslld	xmm12, xmm9, 19 			; 154D _ C4 C1 19: 72. F1, 13
	vpxor	xmm8, xmm4, xmm11			; 1553 _ C4 41 59: EF. C3
	vpsrld	xmm11, xmm11, 3 			; 1558 _ C4 C1 21: 72. D3, 03
	vpsrld	xmm9, xmm9, 13				; 155E _ C4 C1 31: 72. D1, 0D
	vpor	xmm11, xmm13, xmm11			; 1564 _ C4 41 11: EB. DB
	vpor	xmm4, xmm12, xmm9			; 1569 _ C4 C1 19: EB. E1
	vpor	xmm13, xmm11, xmm5			; 156E _ C5 21: EB. ED
	vpxor	xmm11, xmm11, xmm4			; 1572 _ C5 21: EF. DC
	vpand	xmm10, xmm4, xmm5			; 1576 _ C5 59: DB. D5
	vpxor	xmm9, xmm11, xmm6			; 157A _ C5 21: EF. CE
	vpxor	xmm5, xmm5, xmm8			; 157E _ C4 C1 51: EF. E8
	vpand	xmm4, xmm9, xmm13			; 1583 _ C4 C1 31: DB. E5
	vpor	xmm8, xmm8, xmm10			; 1588 _ C4 41 39: EB. C2
	vpxor	xmm12, xmm10, xmm9			; 158D _ C4 41 29: EF. E1
	vpxor	xmm11, xmm8, xmm4			; 1592 _ C5 39: EF. DC
	vpand	xmm9, xmm5, xmm13			; 1596 _ C4 41 51: DB. CD
	vpxor	xmm5, xmm4, xmm12			; 159B _ C4 C1 59: EF. EC
	vpxor	xmm10, xmm13, xmm11			; 15A0 _ C4 41 11: EF. D3
	vpor	xmm13, xmm12, xmm5			; 15A5 _ C5 19: EB. ED
	vmovd	xmm4, dword [r12+1370H] 		; 15A9 _ C4 C1 79: 6E. A4 24, 00001370
	vpxor	xmm8, xmm9, xmm10			; 15B3 _ C4 41 31: EF. C2
	vpxor	xmm12, xmm13, xmm9			; 15B8 _ C4 41 11: EF. E1
	vpxor	xmm9, xmm8, xmm5			; 15BD _ C5 39: EF. CD
	vmovd	xmm8, dword [r12+1374H] 		; 15C1 _ C4 41 79: 6E. 84 24, 00001374
	vpshufd xmm13, xmm4, 0				; 15CB _ C5 79: 70. EC, 00
	vpxor	xmm4, xmm9, xmm13			; 15D0 _ C4 C1 31: EF. E5
	vmovd	xmm13, dword [r12+1378H]		; 15D5 _ C4 41 79: 6E. AC 24, 00001378
	vpshufd xmm9, xmm8, 0				; 15DF _ C4 41 79: 70. C8, 00
	vpxor	xmm8, xmm12, xmm9			; 15E5 _ C4 41 19: EF. C1
	vpor	xmm12, xmm10, xmm12			; 15EA _ C4 41 29: EB. E4
	vmovd	xmm10, dword [r12+137CH]		; 15EF _ C4 41 79: 6E. 94 24, 0000137C
	vpshufd xmm9, xmm13, 0				; 15F9 _ C4 41 79: 70. CD, 00
	vpxor	xmm13, xmm11, xmm9			; 15FF _ C4 41 21: EF. E9
	vpxor	xmm11, xmm12, xmm5			; 1604 _ C5 19: EF. DD
	vpshufd xmm5, xmm10, 0				; 1608 _ C4 C1 79: 70. EA, 00
	vpslld	xmm9, xmm13, 10 			; 160E _ C4 C1 31: 72. F5, 0A
	vpxor	xmm10, xmm11, xmm5			; 1614 _ C5 21: EF. D5
	vpsrld	xmm11, xmm13, 22			; 1618 _ C4 C1 21: 72. D5, 16
	vpor	xmm5, xmm9, xmm11			; 161E _ C4 C1 31: EB. EB
	vpslld	xmm11, xmm4, 27 			; 1623 _ C5 A1: 72. F4, 1B
	vpsrld	xmm4, xmm4, 5				; 1628 _ C5 D9: 72. D4, 05
	vpxor	xmm13, xmm5, xmm10			; 162D _ C4 41 51: EF. EA
	vpor	xmm11, xmm11, xmm4			; 1632 _ C5 21: EB. DC
	vpslld	xmm12, xmm8, 7				; 1636 _ C4 C1 19: 72. F0, 07
	vpxor	xmm4, xmm11, xmm8			; 163C _ C4 C1 21: EF. E0
	vpslld	xmm5, xmm10, 25 			; 1641 _ C4 C1 51: 72. F2, 19
	vpxor	xmm4, xmm4, xmm10			; 1647 _ C4 C1 59: EF. E2
	vpsrld	xmm10, xmm10, 7 			; 164C _ C4 C1 29: 72. D2, 07
	vpxor	xmm9, xmm13, xmm12			; 1652 _ C4 41 11: EF. CC
	vpor	xmm11, xmm5, xmm10			; 1657 _ C4 41 51: EB. DA
	vpslld	xmm12, xmm8, 31 			; 165C _ C4 C1 19: 72. F0, 1F
	vpsrld	xmm8, xmm8, 1				; 1662 _ C4 C1 39: 72. D0, 01
	vpxor	xmm5, xmm11, xmm9			; 1668 _ C4 C1 21: EF. E9
	vpor	xmm11, xmm12, xmm8			; 166D _ C4 41 19: EB. D8
	vpslld	xmm13, xmm4, 3				; 1672 _ C5 91: 72. F4, 03
	vpxor	xmm8, xmm11, xmm4			; 1677 _ C5 21: EF. C4
	vpxor	xmm10, xmm5, xmm13			; 167B _ C4 41 51: EF. D5
	vpxor	xmm5, xmm8, xmm9			; 1680 _ C4 C1 39: EF. E9
	vpslld	xmm13, xmm9, 29 			; 1685 _ C4 C1 11: 72. F1, 1D
	vpsrld	xmm9, xmm9, 3				; 168B _ C4 C1 31: 72. D1, 03
	vpslld	xmm12, xmm4, 19 			; 1691 _ C5 99: 72. F4, 13
	vpsrld	xmm4, xmm4, 13				; 1696 _ C5 D9: 72. D4, 0D
	vpor	xmm11, xmm13, xmm9			; 169B _ C4 41 11: EB. D9
	vpor	xmm8, xmm12, xmm4			; 16A0 _ C5 19: EB. C4
	vpxor	xmm4, xmm8, xmm11			; 16A4 _ C4 C1 39: EF. E3
	vpxor	xmm12, xmm10, xmm5			; 16A9 _ C5 29: EF. E5
	vpand	xmm9, xmm11, xmm4			; 16AD _ C5 21: DB. CC
	vpxor	xmm10, xmm11, xmm10			; 16B1 _ C4 41 21: EF. D2
	vpxor	xmm13, xmm9, xmm6			; 16B6 _ C5 31: EF. EE
	vpor	xmm11, xmm10, xmm4			; 16BA _ C5 29: EB. DC
	vpxor	xmm8, xmm13, xmm12			; 16BE _ C4 41 11: EF. C4
	vpxor	xmm13, xmm12, xmm11			; 16C3 _ C4 41 19: EF. EB
	vmovd	xmm10, dword [r12+1360H]		; 16C8 _ C4 41 79: 6E. 94 24, 00001360
	vpxor	xmm9, xmm4, xmm8			; 16D2 _ C4 41 59: EF. C8
	vpand	xmm4, xmm5, xmm13			; 16D7 _ C4 C1 51: DB. E5
	vpxor	xmm11, xmm11, xmm5			; 16DC _ C5 21: EF. DD
	vpxor	xmm12, xmm4, xmm9			; 16E0 _ C4 41 59: EF. E1
	vpxor	xmm5, xmm9, xmm13			; 16E5 _ C4 C1 31: EF. ED
	vpshufd xmm4, xmm10, 0				; 16EA _ C4 C1 79: 70. E2, 00
	vpxor	xmm13, xmm13, xmm12			; 16F0 _ C4 41 11: EF. EC
	vpxor	xmm10, xmm12, xmm4			; 16F5 _ C5 19: EF. D4
	vmovd	xmm4, dword [r12+1364H] 		; 16F9 _ C4 C1 79: 6E. A4 24, 00001364
	vpshufd xmm4, xmm4, 0				; 1703 _ C5 F9: 70. E4, 00
	vpxor	xmm4, xmm8, xmm4			; 1708 _ C5 B9: EF. E4
	vpor	xmm8, xmm5, xmm8			; 170C _ C4 41 51: EB. C0
	vpxor	xmm5, xmm11, xmm8			; 1711 _ C4 C1 21: EF. E8
	vmovd	xmm11, dword [r12+1368H]		; 1716 _ C4 41 79: 6E. 9C 24, 00001368
	vpshufd xmm9, xmm11, 0				; 1720 _ C4 41 79: 70. CB, 00
	vpxor	xmm8, xmm5, xmm9			; 1726 _ C4 41 51: EF. C1
	vpslld	xmm5, xmm8, 10				; 172B _ C4 C1 51: 72. F0, 0A
	vpsrld	xmm8, xmm8, 22				; 1731 _ C4 C1 39: 72. D0, 16
	vmovd	xmm11, dword [r12+136CH]		; 1737 _ C4 41 79: 6E. 9C 24, 0000136C
	vpor	xmm9, xmm5, xmm8			; 1741 _ C4 41 51: EB. C8
	vpslld	xmm8, xmm10, 27 			; 1746 _ C4 C1 39: 72. F2, 1B
	vpsrld	xmm10, xmm10, 5 			; 174C _ C4 C1 29: 72. D2, 05
	vpshufd xmm12, xmm11, 0 			; 1752 _ C4 41 79: 70. E3, 00
	vpor	xmm8, xmm8, xmm10			; 1758 _ C4 41 39: EB. C2
	vpxor	xmm11, xmm13, xmm12			; 175D _ C4 41 11: EF. DC
	vpxor	xmm5, xmm8, xmm4			; 1762 _ C5 B9: EF. EC
	vpxor	xmm13, xmm9, xmm11			; 1766 _ C4 41 31: EF. EB
	vpslld	xmm12, xmm4, 7				; 176B _ C5 99: 72. F4, 07
	vpxor	xmm5, xmm5, xmm11			; 1770 _ C4 C1 51: EF. EB
	vpslld	xmm9, xmm11, 25 			; 1775 _ C4 C1 31: 72. F3, 19
	vpsrld	xmm11, xmm11, 7 			; 177B _ C4 C1 21: 72. D3, 07
	vpxor	xmm13, xmm13, xmm12			; 1781 _ C4 41 11: EF. EC
	vpor	xmm11, xmm9, xmm11			; 1786 _ C4 41 31: EB. DB
	vpslld	xmm9, xmm5, 3				; 178B _ C5 B1: 72. F5, 03
	vpxor	xmm8, xmm11, xmm13			; 1790 _ C4 41 21: EF. C5
	vpslld	xmm12, xmm5, 19 			; 1795 _ C5 99: 72. F5, 13
	vpxor	xmm8, xmm8, xmm9			; 179A _ C4 41 39: EF. C1
	vpslld	xmm9, xmm4, 31				; 179F _ C5 B1: 72. F4, 1F
	vpsrld	xmm4, xmm4, 1				; 17A4 _ C5 D9: 72. D4, 01
	vpsrld	xmm10, xmm5, 13 			; 17A9 _ C5 A9: 72. D5, 0D
	vpor	xmm4, xmm9, xmm4			; 17AE _ C5 B1: EB. E4
	vpor	xmm11, xmm12, xmm10			; 17B2 _ C4 41 19: EB. DA
	vpxor	xmm5, xmm4, xmm5			; 17B7 _ C5 D9: EF. ED
	vpslld	xmm12, xmm13, 29			; 17BB _ C4 C1 19: 72. F5, 1D
	vpxor	xmm9, xmm5, xmm13			; 17C1 _ C4 41 51: EF. CD
	vpsrld	xmm13, xmm13, 3 			; 17C6 _ C4 C1 11: 72. D5, 03
	vpxor	xmm9, xmm9, xmm6			; 17CC _ C5 31: EF. CE
	vpor	xmm10, xmm12, xmm13			; 17D0 _ C4 41 19: EB. D5
	vpxor	xmm12, xmm10, xmm9			; 17D5 _ C4 41 29: EF. E1
	vpor	xmm4, xmm8, xmm11			; 17DA _ C4 C1 39: EB. E3
	vpxor	xmm13, xmm4, xmm12			; 17DF _ C4 41 59: EF. EC
	vpxor	xmm5, xmm8, xmm13			; 17E4 _ C4 C1 39: EF. ED
	vpor	xmm8, xmm12, xmm9			; 17E9 _ C4 41 19: EB. C1
	vpand	xmm8, xmm8, xmm11			; 17EE _ C4 41 39: DB. C3
	vpxor	xmm8, xmm8, xmm5			; 17F3 _ C5 39: EF. C5
	vpor	xmm5, xmm5, xmm11			; 17F7 _ C4 C1 51: EB. EB
	vpand	xmm10, xmm9, xmm8			; 17FC _ C4 41 31: DB. D0
	vpxor	xmm9, xmm5, xmm9			; 1801 _ C4 41 51: EF. C9
	vmovd	xmm5, dword [r12+1350H] 		; 1806 _ C4 C1 79: 6E. AC 24, 00001350
	vpxor	xmm4, xmm10, xmm13			; 1810 _ C4 C1 29: EF. E5
	vpshufd xmm10, xmm5, 0				; 1815 _ C5 79: 70. D5, 00
	vpxor	xmm12, xmm9, xmm8			; 181A _ C4 41 31: EF. E0
	vpxor	xmm9, xmm12, xmm4			; 181F _ C5 19: EF. CC
	vpxor	xmm4, xmm4, xmm10			; 1823 _ C4 C1 59: EF. E2
	vmovd	xmm10, dword [r12+1354H]		; 1828 _ C4 41 79: 6E. 94 24, 00001354
	vpand	xmm13, xmm13, xmm12			; 1832 _ C4 41 11: DB. EC
	vpshufd xmm10, xmm10, 0 			; 1837 _ C4 41 79: 70. D2, 00
	vpxor	xmm5, xmm9, xmm6			; 183D _ C5 B1: EF. EE
	vpxor	xmm9, xmm13, xmm9			; 1841 _ C4 41 11: EF. C9
	vpxor	xmm5, xmm5, xmm10			; 1846 _ C4 C1 51: EF. EA
	vpxor	xmm9, xmm9, xmm11			; 184B _ C4 41 31: EF. CB
	vmovd	xmm10, dword [r12+1358H]		; 1850 _ C4 41 79: 6E. 94 24, 00001358
	vmovd	xmm11, dword [r12+135CH]		; 185A _ C4 41 79: 6E. 9C 24, 0000135C
	vpshufd xmm12, xmm10, 0 			; 1864 _ C4 41 79: 70. E2, 00
	vpshufd xmm10, xmm11, 0 			; 186A _ C4 41 79: 70. D3, 00
	vpxor	xmm9, xmm9, xmm12			; 1870 _ C4 41 31: EF. CC
	vpxor	xmm13, xmm8, xmm10			; 1875 _ C4 41 39: EF. EA
	vpslld	xmm8, xmm9, 10				; 187A _ C4 C1 39: 72. F1, 0A
	vpsrld	xmm10, xmm9, 22 			; 1880 _ C4 C1 29: 72. D1, 16
	vpslld	xmm11, xmm4, 27 			; 1886 _ C5 A1: 72. F4, 1B
	vpsrld	xmm4, xmm4, 5				; 188B _ C5 D9: 72. D4, 05
	vpor	xmm9, xmm8, xmm10			; 1890 _ C4 41 39: EB. CA
	vpor	xmm10, xmm11, xmm4			; 1895 _ C5 21: EB. D4
	vpxor	xmm8, xmm9, xmm13			; 1899 _ C4 41 31: EF. C5
	vpslld	xmm12, xmm5, 7				; 189E _ C5 99: 72. F5, 07
	vpxor	xmm4, xmm10, xmm5			; 18A3 _ C5 A9: EF. E5
	vpxor	xmm9, xmm8, xmm12			; 18A7 _ C4 41 39: EF. CC
	vpxor	xmm11, xmm4, xmm13			; 18AC _ C4 41 59: EF. DD
	vpslld	xmm8, xmm13, 25 			; 18B1 _ C4 C1 39: 72. F5, 19
	vpsrld	xmm13, xmm13, 7 			; 18B7 _ C4 C1 11: 72. D5, 07
	vpor	xmm12, xmm8, xmm13			; 18BD _ C4 41 39: EB. E5
	vpslld	xmm4, xmm11, 3				; 18C2 _ C4 C1 59: 72. F3, 03
	vpxor	xmm10, xmm12, xmm9			; 18C8 _ C4 41 19: EF. D1
	vpslld	xmm13, xmm5, 31 			; 18CD _ C5 91: 72. F5, 1F
	vpsrld	xmm5, xmm5, 1				; 18D2 _ C5 D1: 72. D5, 01
	vpxor	xmm8, xmm10, xmm4			; 18D7 _ C5 29: EF. C4
	vpor	xmm10, xmm13, xmm5			; 18DB _ C5 11: EB. D5
	vpslld	xmm13, xmm9, 29 			; 18DF _ C4 C1 11: 72. F1, 1D
	vpxor	xmm5, xmm10, xmm11			; 18E5 _ C4 C1 29: EF. EB
	vpslld	xmm10, xmm11, 19			; 18EA _ C4 C1 29: 72. F3, 13
	vpxor	xmm4, xmm5, xmm9			; 18F0 _ C4 C1 51: EF. E1
	vpsrld	xmm9, xmm9, 3				; 18F5 _ C4 C1 31: 72. D1, 03
	vpor	xmm12, xmm13, xmm9			; 18FB _ C4 41 11: EB. E1
	vpsrld	xmm11, xmm11, 13			; 1900 _ C4 C1 21: 72. D3, 0D
	vpor	xmm5, xmm10, xmm11			; 1906 _ C4 C1 29: EB. EB
	vpand	xmm10, xmm12, xmm8			; 190B _ C4 41 19: DB. D0
	vpxor	xmm11, xmm10, xmm4			; 1910 _ C5 29: EF. DC
	vpor	xmm4, xmm4, xmm8			; 1914 _ C4 C1 59: EB. E0
	vpand	xmm9, xmm4, xmm5			; 1919 _ C5 59: DB. CD
	vpxor	xmm12, xmm12, xmm11			; 191D _ C4 41 19: EF. E3
	vpxor	xmm12, xmm12, xmm9			; 1922 _ C4 41 19: EF. E1
	vpxor	xmm13, xmm5, xmm6			; 1927 _ C5 51: EF. EE
	vpxor	xmm5, xmm8, xmm12			; 192B _ C4 C1 39: EF. EC
	vpand	xmm8, xmm9, xmm11			; 1930 _ C4 41 31: DB. C3
	vpxor	xmm9, xmm8, xmm5			; 1935 _ C5 39: EF. CD
	vpand	xmm4, xmm5, xmm13			; 1939 _ C4 C1 51: DB. E5
	vmovd	xmm5, dword [r12+1340H] 		; 193E _ C4 C1 79: 6E. AC 24, 00001340
	vpxor	xmm10, xmm13, xmm9			; 1948 _ C4 41 11: EF. D1
	vpxor	xmm13, xmm4, xmm11			; 194D _ C4 41 59: EF. EB
	vpand	xmm11, xmm11, xmm10			; 1952 _ C4 41 21: DB. DA
	vpshufd xmm4, xmm5, 0				; 1957 _ C5 F9: 70. E5, 00
	vpxor	xmm8, xmm13, xmm10			; 195C _ C4 41 11: EF. C2
	vpxor	xmm13, xmm10, xmm4			; 1961 _ C5 29: EF. EC
	vpxor	xmm4, xmm8, xmm10			; 1965 _ C4 C1 39: EF. E2
	vpxor	xmm10, xmm11, xmm12			; 196A _ C4 41 21: EF. D4
	vmovd	xmm5, dword [r12+1344H] 		; 196F _ C4 C1 79: 6E. AC 24, 00001344
	vpor	xmm8, xmm10, xmm8			; 1979 _ C4 41 29: EB. C0
	vpshufd xmm5, xmm5, 0				; 197E _ C5 F9: 70. ED, 00
	vpxor	xmm10, xmm8, xmm9			; 1983 _ C4 41 39: EF. D1
	vmovd	xmm9, dword [r12+1348H] 		; 1988 _ C4 41 79: 6E. 8C 24, 00001348
	vpxor	xmm4, xmm4, xmm5			; 1992 _ C5 D9: EF. E5
	vpshufd xmm5, xmm9, 0				; 1996 _ C4 C1 79: 70. E9, 00
	vmovd	xmm9, dword [r12+134CH] 		; 199C _ C4 41 79: 6E. 8C 24, 0000134C
	vpxor	xmm11, xmm10, xmm5			; 19A6 _ C5 29: EF. DD
	vpshufd xmm8, xmm9, 0				; 19AA _ C4 41 79: 70. C1, 00
	vpsrld	xmm10, xmm11, 22			; 19B0 _ C4 C1 29: 72. D3, 16
	vpxor	xmm5, xmm12, xmm8			; 19B6 _ C4 C1 19: EF. E8
	vpslld	xmm12, xmm11, 10			; 19BB _ C4 C1 19: 72. F3, 0A
	vpor	xmm9, xmm12, xmm10			; 19C1 _ C4 41 19: EB. CA
	vpslld	xmm11, xmm13, 27			; 19C6 _ C4 C1 21: 72. F5, 1B
	vpsrld	xmm13, xmm13, 5 			; 19CC _ C4 C1 11: 72. D5, 05
	vpxor	xmm8, xmm9, xmm5			; 19D2 _ C5 31: EF. C5
	vpor	xmm9, xmm11, xmm13			; 19D6 _ C4 41 21: EB. CD
	vpslld	xmm12, xmm4, 7				; 19DB _ C5 99: 72. F4, 07
	vpxor	xmm13, xmm9, xmm4			; 19E0 _ C5 31: EF. EC
	vpxor	xmm10, xmm8, xmm12			; 19E4 _ C4 41 39: EF. D4
	vpxor	xmm13, xmm13, xmm5			; 19E9 _ C5 11: EF. ED
	vpslld	xmm8, xmm5, 25				; 19ED _ C5 B9: 72. F5, 19
	vpsrld	xmm5, xmm5, 7				; 19F2 _ C5 D1: 72. D5, 07
	vpslld	xmm11, xmm4, 31 			; 19F7 _ C5 A1: 72. F4, 1F
	vpor	xmm5, xmm8, xmm5			; 19FC _ C5 B9: EB. ED
	vpsrld	xmm4, xmm4, 1				; 1A00 _ C5 D9: 72. D4, 01
	vpxor	xmm9, xmm5, xmm10			; 1A05 _ C4 41 51: EF. CA
	vpor	xmm5, xmm11, xmm4			; 1A0A _ C5 A1: EB. EC
	vpslld	xmm8, xmm13, 3				; 1A0E _ C4 C1 39: 72. F5, 03
	vpxor	xmm4, xmm5, xmm13			; 1A14 _ C4 C1 51: EF. E5
	vpxor	xmm12, xmm9, xmm8			; 1A19 _ C4 41 31: EF. E0
	vpxor	xmm5, xmm4, xmm10			; 1A1E _ C4 C1 59: EF. EA
	vpslld	xmm9, xmm10, 29 			; 1A23 _ C4 C1 31: 72. F2, 1D
	vpsrld	xmm10, xmm10, 3 			; 1A29 _ C4 C1 29: 72. D2, 03
	vpor	xmm9, xmm9, xmm10			; 1A2F _ C4 41 31: EB. CA
	vpslld	xmm11, xmm13, 19			; 1A34 _ C4 C1 21: 72. F5, 13
	vpsrld	xmm13, xmm13, 13			; 1A3A _ C4 C1 11: 72. D5, 0D
	vpxor	xmm8, xmm9, xmm5			; 1A40 _ C5 31: EF. C5
	vpor	xmm10, xmm11, xmm13			; 1A44 _ C4 41 21: EB. D5
	vpand	xmm9, xmm9, xmm8			; 1A49 _ C4 41 31: DB. C8
	vpxor	xmm4, xmm10, xmm8			; 1A4E _ C4 C1 29: EF. E0
	vpxor	xmm13, xmm9, xmm4			; 1A53 _ C5 31: EF. EC
	vpor	xmm10, xmm12, xmm13			; 1A57 _ C4 41 19: EB. D5
	vpxor	xmm12, xmm5, xmm12			; 1A5C _ C4 41 51: EF. E4
	vpxor	xmm9, xmm8, xmm10			; 1A61 _ C4 41 39: EF. CA
	vpand	xmm8, xmm4, xmm5			; 1A66 _ C5 59: DB. C5
	vpxor	xmm11, xmm8, xmm10			; 1A6A _ C4 41 39: EF. DA
	vpxor	xmm4, xmm12, xmm13			; 1A6F _ C4 C1 19: EF. E5
	vpand	xmm10, xmm10, xmm9			; 1A74 _ C4 41 29: DB. D1
	vpxor	xmm8, xmm4, xmm11			; 1A79 _ C4 41 59: EF. C3
	vpxor	xmm5, xmm10, xmm4			; 1A7E _ C5 A9: EF. EC
	vpor	xmm12, xmm8, xmm9			; 1A82 _ C4 41 39: EB. E1
	vmovd	xmm10, dword [r12+1330H]		; 1A87 _ C4 41 79: 6E. 94 24, 00001330
	vpxor	xmm13, xmm12, xmm13			; 1A91 _ C4 41 19: EF. ED
	vpshufd xmm4, xmm10, 0				; 1A96 _ C4 C1 79: 70. E2, 00
	vmovd	xmm8, dword [r12+1334H] 		; 1A9C _ C4 41 79: 6E. 84 24, 00001334
	vpxor	xmm9, xmm9, xmm4			; 1AA6 _ C5 31: EF. CC
	vmovd	xmm4, dword [r12+1338H] 		; 1AAA _ C4 C1 79: 6E. A4 24, 00001338
	vpshufd xmm12, xmm8, 0				; 1AB4 _ C4 41 79: 70. E0, 00
	vpshufd xmm8, xmm4, 0				; 1ABA _ C5 79: 70. C4, 00
	vpxor	xmm10, xmm13, xmm12			; 1ABF _ C4 41 11: EF. D4
	vpxor	xmm4, xmm5, xmm8			; 1AC4 _ C4 C1 51: EF. E0
	vpxor	xmm5, xmm11, xmm5			; 1AC9 _ C5 A1: EF. ED
	vpxor	xmm11, xmm5, xmm13			; 1ACD _ C4 41 51: EF. DD
	vmovd	xmm13, dword [r12+133CH]		; 1AD2 _ C4 41 79: 6E. AC 24, 0000133C
	vpshufd xmm5, xmm13, 0				; 1ADC _ C4 C1 79: 70. ED, 00
	vpslld	xmm13, xmm4, 10 			; 1AE2 _ C5 91: 72. F4, 0A
	vpsrld	xmm4, xmm4, 22				; 1AE7 _ C5 D9: 72. D4, 16
	vpxor	xmm11, xmm11, xmm5			; 1AEC _ C5 21: EF. DD
	vpor	xmm8, xmm13, xmm4			; 1AF0 _ C5 11: EB. C4
	vpslld	xmm5, xmm10, 7				; 1AF4 _ C4 C1 51: 72. F2, 07
	vpxor	xmm12, xmm8, xmm11			; 1AFA _ C4 41 39: EF. E3
	vpslld	xmm4, xmm9, 27				; 1AFF _ C4 C1 59: 72. F1, 1B
	vpsrld	xmm9, xmm9, 5				; 1B05 _ C4 C1 31: 72. D1, 05
	vpxor	xmm12, xmm12, xmm5			; 1B0B _ C5 19: EF. E5
	vpor	xmm5, xmm4, xmm9			; 1B0F _ C4 C1 59: EB. E9
	vpslld	xmm9, xmm11, 25 			; 1B14 _ C4 C1 31: 72. F3, 19
	vpxor	xmm4, xmm5, xmm10			; 1B1A _ C4 C1 51: EF. E2
	vpslld	xmm13, xmm10, 31			; 1B1F _ C4 C1 11: 72. F2, 1F
	vpxor	xmm8, xmm4, xmm11			; 1B25 _ C4 41 59: EF. C3
	vpsrld	xmm11, xmm11, 7 			; 1B2A _ C4 C1 21: 72. D3, 07
	vpsrld	xmm10, xmm10, 1 			; 1B30 _ C4 C1 29: 72. D2, 01
	vpor	xmm5, xmm9, xmm11			; 1B36 _ C4 C1 31: EB. EB
	vpor	xmm10, xmm13, xmm10			; 1B3B _ C4 41 11: EB. D2
	vpxor	xmm4, xmm5, xmm12			; 1B40 _ C4 C1 51: EF. E4
	vpslld	xmm9, xmm8, 3				; 1B45 _ C4 C1 31: 72. F0, 03
	vpxor	xmm5, xmm10, xmm8			; 1B4B _ C4 C1 29: EF. E8
	vpxor	xmm4, xmm4, xmm9			; 1B50 _ C4 C1 59: EF. E1
	vpxor	xmm9, xmm5, xmm12			; 1B55 _ C4 41 51: EF. CC
	vpslld	xmm13, xmm8, 19 			; 1B5A _ C4 C1 11: 72. F0, 13
	vpsrld	xmm8, xmm8, 13				; 1B60 _ C4 C1 39: 72. D0, 0D
	vpslld	xmm11, xmm12, 29			; 1B66 _ C4 C1 21: 72. F4, 1D
	vpsrld	xmm12, xmm12, 3 			; 1B6C _ C4 C1 19: 72. D4, 03
	vpor	xmm10, xmm13, xmm8			; 1B72 _ C4 41 11: EB. D0
	vpor	xmm5, xmm11, xmm12			; 1B77 _ C4 C1 21: EB. EC
	vpxor	xmm5, xmm5, xmm4			; 1B7C _ C5 D1: EF. EC
	vpxor	xmm13, xmm4, xmm10			; 1B80 _ C4 41 59: EF. EA
	vpand	xmm4, xmm13, xmm5			; 1B85 _ C5 91: DB. E5
	vpxor	xmm4, xmm4, xmm9			; 1B89 _ C4 C1 59: EF. E1
	vpor	xmm9, xmm9, xmm5			; 1B8E _ C5 31: EB. CD
	vpxor	xmm9, xmm9, xmm13			; 1B92 _ C4 41 31: EF. CD
	vpand	xmm13, xmm13, xmm4			; 1B97 _ C5 11: DB. EC
	vpxor	xmm8, xmm5, xmm4			; 1B9B _ C5 51: EF. C4
	vpand	xmm12, xmm13, xmm10			; 1B9F _ C4 41 11: DB. E2
	vpxor	xmm5, xmm12, xmm8			; 1BA4 _ C4 C1 19: EF. E8
	vpxor	xmm11, xmm4, xmm6			; 1BA9 _ C5 59: EF. DE
	vmovd	xmm4, dword [r12+1320H] 		; 1BAD _ C4 C1 79: 6E. A4 24, 00001320
	vpand	xmm8, xmm8, xmm9			; 1BB7 _ C4 41 39: DB. C1
	vmovd	xmm12, dword [r12+1324H]		; 1BBC _ C4 41 79: 6E. A4 24, 00001324
	vpor	xmm8, xmm8, xmm10			; 1BC6 _ C4 41 39: EB. C2
	vpshufd xmm13, xmm4, 0				; 1BCB _ C5 79: 70. EC, 00
	vpxor	xmm10, xmm10, xmm11			; 1BD0 _ C4 41 29: EF. D3
	vpshufd xmm4, xmm12, 0				; 1BD5 _ C4 C1 79: 70. E4, 00
	vpxor	xmm8, xmm8, xmm11			; 1BDB _ C4 41 39: EF. C3
	vmovd	xmm12, dword [r12+1328H]		; 1BE0 _ C4 41 79: 6E. A4 24, 00001328
	vpxor	xmm4, xmm5, xmm4			; 1BEA _ C5 D1: EF. E4
	vpshufd xmm12, xmm12, 0 			; 1BEE _ C4 41 79: 70. E4, 00
	vpxor	xmm5, xmm11, xmm5			; 1BF4 _ C5 A1: EF. ED
	vpand	xmm11, xmm10, xmm9			; 1BF8 _ C4 41 29: DB. D9
	vpxor	xmm13, xmm9, xmm13			; 1BFD _ C4 41 31: EF. ED
	vmovd	xmm10, dword [r12+132CH]		; 1C02 _ C4 41 79: 6E. 94 24, 0000132C
	vpxor	xmm8, xmm8, xmm12			; 1C0C _ C4 41 39: EF. C4
	vpshufd xmm9, xmm10, 0				; 1C11 _ C4 41 79: 70. CA, 00
	vpxor	xmm5, xmm5, xmm11			; 1C17 _ C4 C1 51: EF. EB
	vpxor	xmm12, xmm5, xmm9			; 1C1C _ C4 41 51: EF. E1
	vpslld	xmm10, xmm8, 10 			; 1C21 _ C4 C1 29: 72. F0, 0A
	vpsrld	xmm5, xmm8, 22				; 1C27 _ C4 C1 51: 72. D0, 16
	vpslld	xmm11, xmm4, 7				; 1C2D _ C5 A1: 72. F4, 07
	vpor	xmm9, xmm10, xmm5			; 1C32 _ C5 29: EB. CD
	vpslld	xmm10, xmm13, 27			; 1C36 _ C4 C1 29: 72. F5, 1B
	vpsrld	xmm13, xmm13, 5 			; 1C3C _ C4 C1 11: 72. D5, 05
	vpxor	xmm8, xmm9, xmm12			; 1C42 _ C4 41 31: EF. C4
	vpor	xmm10, xmm10, xmm13			; 1C47 _ C4 41 29: EB. D5
	vpxor	xmm9, xmm8, xmm11			; 1C4C _ C4 41 39: EF. CB
	vpxor	xmm5, xmm10, xmm4			; 1C51 _ C5 A9: EF. EC
	vpslld	xmm13, xmm12, 25			; 1C55 _ C4 C1 11: 72. F4, 19
	vpxor	xmm11, xmm5, xmm12			; 1C5B _ C4 41 51: EF. DC
	vpsrld	xmm12, xmm12, 7 			; 1C60 _ C4 C1 19: 72. D4, 07
	vpor	xmm10, xmm13, xmm12			; 1C66 _ C4 41 11: EB. D4
	vpslld	xmm8, xmm4, 31				; 1C6B _ C5 B9: 72. F4, 1F
	vpsrld	xmm4, xmm4, 1				; 1C70 _ C5 D9: 72. D4, 01
	vpxor	xmm5, xmm10, xmm9			; 1C75 _ C4 C1 29: EF. E9
	vpslld	xmm13, xmm11, 3 			; 1C7A _ C4 C1 11: 72. F3, 03
	vpor	xmm10, xmm8, xmm4			; 1C80 _ C5 39: EB. D4
	vpxor	xmm12, xmm5, xmm13			; 1C84 _ C4 41 51: EF. E5
	vpxor	xmm5, xmm10, xmm11			; 1C89 _ C4 C1 29: EF. EB
	vpxor	xmm8, xmm5, xmm9			; 1C8E _ C4 41 51: EF. C1
	vpslld	xmm4, xmm9, 29				; 1C93 _ C4 C1 59: 72. F1, 1D
	vpsrld	xmm9, xmm9, 3				; 1C99 _ C4 C1 31: 72. D1, 03
	vpslld	xmm13, xmm11, 19			; 1C9F _ C4 C1 11: 72. F3, 13
	vpsrld	xmm11, xmm11, 13			; 1CA5 _ C4 C1 21: 72. D3, 0D
	vpxor	xmm5, xmm8, xmm12			; 1CAB _ C4 C1 39: EF. EC
	vpor	xmm9, xmm4, xmm9			; 1CB0 _ C4 41 59: EB. C9
	vpor	xmm4, xmm13, xmm11			; 1CB5 _ C4 C1 11: EB. E3
	vpand	xmm12, xmm12, xmm5			; 1CBA _ C5 19: DB. E5
	vpxor	xmm13, xmm8, xmm9			; 1CBE _ C4 41 39: EF. E9
	vpxor	xmm8, xmm12, xmm4			; 1CC3 _ C5 19: EF. C4
	vpxor	xmm10, xmm5, xmm8			; 1CC7 _ C4 41 51: EF. D0
	vpor	xmm5, xmm4, xmm5			; 1CCC _ C5 D9: EB. ED
	vpxor	xmm9, xmm9, xmm8			; 1CD0 _ C4 41 31: EF. C8
	vpxor	xmm4, xmm5, xmm13			; 1CD5 _ C4 C1 51: EF. E5
	vpor	xmm12, xmm4, xmm9			; 1CDA _ C4 41 59: EB. E1
	vpxor	xmm13, xmm13, xmm6			; 1CDF _ C5 11: EF. EE
	vpxor	xmm4, xmm12, xmm10			; 1CE3 _ C4 C1 19: EF. E2
	vpor	xmm10, xmm10, xmm8			; 1CE8 _ C4 41 29: EB. D0
	vmovd	xmm11, dword [r12+1310H]		; 1CED _ C4 41 79: 6E. 9C 24, 00001310
	vpxor	xmm5, xmm10, xmm4			; 1CF7 _ C5 A9: EF. EC
	vpshufd xmm10, xmm11, 0 			; 1CFB _ C4 41 79: 70. D3, 00
	vpxor	xmm13, xmm13, xmm5			; 1D01 _ C5 11: EF. ED
	vmovd	xmm11, dword [r12+1314H]		; 1D05 _ C4 41 79: 6E. 9C 24, 00001314
	vpxor	xmm12, xmm13, xmm10			; 1D0F _ C4 41 11: EF. E2
	vpshufd xmm10, xmm11, 0 			; 1D14 _ C4 41 79: 70. D3, 00
	vpor	xmm5, xmm5, xmm4			; 1D1A _ C5 D1: EB. EC
	vpxor	xmm10, xmm4, xmm10			; 1D1E _ C4 41 59: EF. D2
	vpxor	xmm4, xmm5, xmm4			; 1D23 _ C5 D1: EF. E4
	vpor	xmm13, xmm4, xmm13			; 1D27 _ C4 41 59: EB. ED
	vpxor	xmm5, xmm8, xmm13			; 1D2C _ C4 C1 39: EF. ED
	vmovd	xmm8, dword [r12+1318H] 		; 1D31 _ C4 41 79: 6E. 84 24, 00001318
	vmovd	xmm13, dword [r12+131CH]		; 1D3B _ C4 41 79: 6E. AC 24, 0000131C
	vpshufd xmm4, xmm8, 0				; 1D45 _ C4 C1 79: 70. E0, 00
	vpshufd xmm8, xmm13, 0				; 1D4B _ C4 41 79: 70. C5, 00
	vpxor	xmm5, xmm5, xmm4			; 1D51 _ C5 D1: EF. EC
	vpxor	xmm11, xmm9, xmm8			; 1D55 _ C4 41 31: EF. D8
	vpslld	xmm9, xmm5, 10				; 1D5A _ C5 B1: 72. F5, 0A
	vpsrld	xmm5, xmm5, 22				; 1D5F _ C5 D1: 72. D5, 16
	vpslld	xmm8, xmm12, 27 			; 1D64 _ C4 C1 39: 72. F4, 1B
	vpor	xmm4, xmm9, xmm5			; 1D6A _ C5 B1: EB. E5
	vpsrld	xmm12, xmm12, 5 			; 1D6E _ C4 C1 19: 72. D4, 05
	vpxor	xmm9, xmm4, xmm11			; 1D74 _ C4 41 59: EF. CB
	vpslld	xmm13, xmm10, 7 			; 1D79 _ C4 C1 11: 72. F2, 07
	vpor	xmm4, xmm8, xmm12			; 1D7F _ C4 C1 39: EB. E4
	vpxor	xmm5, xmm9, xmm13			; 1D84 _ C4 C1 31: EF. ED
	vpxor	xmm9, xmm4, xmm10			; 1D89 _ C4 41 59: EF. CA
	vpslld	xmm13, xmm11, 25			; 1D8E _ C4 C1 11: 72. F3, 19
	vpxor	xmm9, xmm9, xmm11			; 1D94 _ C4 41 31: EF. CB
	vpsrld	xmm11, xmm11, 7 			; 1D99 _ C4 C1 21: 72. D3, 07
	vpslld	xmm12, xmm10, 31			; 1D9F _ C4 C1 19: 72. F2, 1F
	vpsrld	xmm10, xmm10, 1 			; 1DA5 _ C4 C1 29: 72. D2, 01
	vpor	xmm4, xmm13, xmm11			; 1DAB _ C4 C1 11: EB. E3
	vpor	xmm10, xmm12, xmm10			; 1DB0 _ C4 41 19: EB. D2
	vpxor	xmm13, xmm4, xmm5			; 1DB5 _ C5 59: EF. ED
	vpslld	xmm8, xmm9, 3				; 1DB9 _ C4 C1 39: 72. F1, 03
	vpxor	xmm4, xmm10, xmm9			; 1DBF _ C4 C1 29: EF. E1
	vpxor	xmm11, xmm13, xmm8			; 1DC4 _ C4 41 11: EF. D8
	vpxor	xmm4, xmm4, xmm5			; 1DC9 _ C5 D9: EF. E5
	vpslld	xmm13, xmm9, 19 			; 1DCD _ C4 C1 11: 72. F1, 13
	vpsrld	xmm9, xmm9, 13				; 1DD3 _ C4 C1 31: 72. D1, 0D
	vpslld	xmm8, xmm5, 29				; 1DD9 _ C5 B9: 72. F5, 1D
	vpsrld	xmm5, xmm5, 3				; 1DDE _ C5 D1: 72. D5, 03
	vpor	xmm13, xmm13, xmm9			; 1DE3 _ C4 41 11: EB. E9
	vpor	xmm12, xmm8, xmm5			; 1DE8 _ C5 39: EB. E5
	vpxor	xmm9, xmm4, xmm6			; 1DEC _ C5 59: EF. CE
	vpxor	xmm10, xmm12, xmm6			; 1DF0 _ C5 19: EF. D6
	vpor	xmm5, xmm4, xmm13			; 1DF4 _ C4 C1 59: EB. ED
	vpxor	xmm4, xmm5, xmm10			; 1DF9 _ C4 C1 51: EF. E2
	vpxor	xmm13, xmm13, xmm9			; 1DFE _ C4 41 11: EF. E9
	vpor	xmm10, xmm10, xmm9			; 1E03 _ C4 41 29: EB. D1
	vpxor	xmm4, xmm4, xmm11			; 1E08 _ C4 C1 59: EF. E3
	vpxor	xmm12, xmm10, xmm13			; 1E0D _ C4 41 29: EF. E5
	vpand	xmm10, xmm13, xmm11			; 1E12 _ C4 41 11: DB. D3
	vpxor	xmm8, xmm9, xmm10			; 1E17 _ C4 41 31: EF. C2
	vpor	xmm9, xmm10, xmm4			; 1E1C _ C5 29: EB. CC
	vmovd	xmm13, dword [r12+1300H]		; 1E20 _ C4 41 79: 6E. AC 24, 00001300
	vpxor	xmm5, xmm9, xmm12			; 1E2A _ C4 C1 31: EF. EC
	vpxor	xmm11, xmm11, xmm8			; 1E2F _ C4 41 21: EF. D8
	vpxor	xmm10, xmm11, xmm5			; 1E34 _ C5 21: EF. D5
	vpshufd xmm11, xmm13, 0 			; 1E38 _ C4 41 79: 70. DD, 00
	vpxor	xmm9, xmm10, xmm4			; 1E3E _ C5 29: EF. CC
	vpxor	xmm13, xmm5, xmm11			; 1E42 _ C4 41 51: EF. EB
	vpxor	xmm5, xmm12, xmm4			; 1E47 _ C5 99: EF. EC
	vpand	xmm12, xmm5, xmm9			; 1E4B _ C4 41 51: DB. E1
	vpxor	xmm10, xmm8, xmm12			; 1E50 _ C4 41 39: EF. D4
	vmovd	xmm8, dword [r12+1304H] 		; 1E55 _ C4 41 79: 6E. 84 24, 00001304
	vpshufd xmm5, xmm8, 0				; 1E5F _ C4 C1 79: 70. E8, 00
	vmovd	xmm8, dword [r12+1308H] 		; 1E65 _ C4 41 79: 6E. 84 24, 00001308
	vpxor	xmm11, xmm10, xmm5			; 1E6F _ C5 29: EF. DD
	vpshufd xmm12, xmm8, 0				; 1E73 _ C4 41 79: 70. E0, 00
	vpxor	xmm8, xmm4, xmm12			; 1E79 _ C4 41 59: EF. C4
	vpslld	xmm12, xmm13, 27			; 1E7E _ C4 C1 19: 72. F5, 1B
	vmovd	xmm4, dword [r12+130CH] 		; 1E84 _ C4 C1 79: 6E. A4 24, 0000130C
	vpsrld	xmm13, xmm13, 5 			; 1E8E _ C4 C1 11: 72. D5, 05
	vpshufd xmm10, xmm4, 0				; 1E94 _ C5 79: 70. D4, 00
	vpxor	xmm5, xmm9, xmm10			; 1E99 _ C4 C1 31: EF. EA
	vpslld	xmm9, xmm8, 10				; 1E9E _ C4 C1 31: 72. F0, 0A
	vpsrld	xmm10, xmm8, 22 			; 1EA4 _ C4 C1 29: 72. D0, 16
	vpslld	xmm8, xmm11, 7				; 1EAA _ C4 C1 39: 72. F3, 07
	vpor	xmm4, xmm9, xmm10			; 1EB0 _ C4 C1 31: EB. E2
	vpor	xmm10, xmm12, xmm13			; 1EB5 _ C4 41 19: EB. D5
	vpxor	xmm9, xmm4, xmm5			; 1EBA _ C5 59: EF. CD
	vpxor	xmm4, xmm10, xmm11			; 1EBE _ C4 C1 29: EF. E3
	vpxor	xmm4, xmm4, xmm5			; 1EC3 _ C5 D9: EF. E5
	vpslld	xmm13, xmm5, 25 			; 1EC7 _ C5 91: 72. F5, 19
	vpsrld	xmm5, xmm5, 7				; 1ECC _ C5 D1: 72. D5, 07
	vpxor	xmm9, xmm9, xmm8			; 1ED1 _ C4 41 31: EF. C8
	vpor	xmm8, xmm13, xmm5			; 1ED6 _ C5 11: EB. C5
	vpslld	xmm10, xmm4, 3				; 1EDA _ C5 A9: 72. F4, 03
	vpxor	xmm12, xmm8, xmm9			; 1EDF _ C4 41 39: EF. E1
	vpslld	xmm13, xmm11, 31			; 1EE4 _ C4 C1 11: 72. F3, 1F
	vpsrld	xmm11, xmm11, 1 			; 1EEA _ C4 C1 21: 72. D3, 01
	vpxor	xmm5, xmm12, xmm10			; 1EF0 _ C4 C1 19: EF. EA
	vpor	xmm10, xmm13, xmm11			; 1EF5 _ C4 41 11: EB. D3
	vpslld	xmm8, xmm9, 29				; 1EFA _ C4 C1 39: 72. F1, 1D
	vpxor	xmm13, xmm10, xmm4			; 1F00 _ C5 29: EF. EC
	vpslld	xmm10, xmm4, 19 			; 1F04 _ C5 A9: 72. F4, 13
	vpxor	xmm11, xmm13, xmm9			; 1F09 _ C4 41 11: EF. D9
	vpsrld	xmm9, xmm9, 3				; 1F0E _ C4 C1 31: 72. D1, 03
	vpsrld	xmm4, xmm4, 13				; 1F14 _ C5 D9: 72. D4, 0D
	vpor	xmm12, xmm8, xmm9			; 1F19 _ C4 41 39: EB. E1
	vpor	xmm9, xmm10, xmm4			; 1F1E _ C5 29: EB. CC
	vpor	xmm10, xmm12, xmm5			; 1F22 _ C5 19: EB. D5
	vpxor	xmm8, xmm12, xmm9			; 1F26 _ C4 41 19: EF. C1
	vpand	xmm13, xmm9, xmm5			; 1F2B _ C5 31: DB. ED
	vpxor	xmm4, xmm8, xmm6			; 1F2F _ C5 B9: EF. E6
	vpxor	xmm5, xmm5, xmm11			; 1F33 _ C4 C1 51: EF. EB
	vpxor	xmm12, xmm13, xmm4			; 1F38 _ C5 11: EF. E4
	vpand	xmm4, xmm4, xmm10			; 1F3C _ C4 C1 59: DB. E2
	vpor	xmm11, xmm11, xmm13			; 1F41 _ C4 41 21: EB. DD
	vpand	xmm9, xmm5, xmm10			; 1F46 _ C4 41 51: DB. CA
	vpxor	xmm8, xmm11, xmm4			; 1F4B _ C5 21: EF. C4
	vpxor	xmm4, xmm4, xmm12			; 1F4F _ C4 C1 59: EF. E4
	vpxor	xmm13, xmm10, xmm8			; 1F54 _ C4 41 29: EF. E8
	vpor	xmm12, xmm12, xmm4			; 1F59 _ C5 19: EB. E4
	vpxor	xmm10, xmm12, xmm9			; 1F5D _ C4 41 19: EF. D1
	vpxor	xmm5, xmm9, xmm13			; 1F62 _ C4 C1 31: EF. ED
	vmovd	xmm9, dword [r12+12F0H] 		; 1F67 _ C4 41 79: 6E. 8C 24, 000012F0
	vpxor	xmm11, xmm5, xmm4			; 1F71 _ C5 51: EF. DC
	vpshufd xmm12, xmm9, 0				; 1F75 _ C4 41 79: 70. E1, 00
	vmovd	xmm9, dword [r12+12F4H] 		; 1F7B _ C4 41 79: 6E. 8C 24, 000012F4
	vpxor	xmm5, xmm11, xmm12			; 1F85 _ C4 C1 21: EF. EC
	vmovd	xmm12, dword [r12+12F8H]		; 1F8A _ C4 41 79: 6E. A4 24, 000012F8
	vpshufd xmm11, xmm9, 0				; 1F94 _ C4 41 79: 70. D9, 00
	vpxor	xmm9, xmm10, xmm11			; 1F9A _ C4 41 29: EF. CB
	vpor	xmm10, xmm13, xmm10			; 1F9F _ C4 41 11: EB. D2
	vpshufd xmm11, xmm12, 0 			; 1FA4 _ C4 41 79: 70. DC, 00
	vpxor	xmm13, xmm10, xmm4			; 1FAA _ C5 29: EF. EC
	vmovd	xmm4, dword [r12+12FCH] 		; 1FAE _ C4 C1 79: 6E. A4 24, 000012FC
	vpxor	xmm8, xmm8, xmm11			; 1FB8 _ C4 41 39: EF. C3
	vpshufd xmm10, xmm4, 0				; 1FBD _ C5 79: 70. D4, 00
	vpslld	xmm4, xmm8, 10				; 1FC2 _ C4 C1 59: 72. F0, 0A
	vpsrld	xmm8, xmm8, 22				; 1FC8 _ C4 C1 39: 72. D0, 16
	vpslld	xmm11, xmm5, 27 			; 1FCE _ C5 A1: 72. F5, 1B
	vpsrld	xmm5, xmm5, 5				; 1FD3 _ C5 D1: 72. D5, 05
	vpxor	xmm13, xmm13, xmm10			; 1FD8 _ C4 41 11: EF. EA
	vpor	xmm10, xmm4, xmm8			; 1FDD _ C4 41 59: EB. D0
	vpor	xmm12, xmm11, xmm5			; 1FE2 _ C5 21: EB. E5
	vpxor	xmm4, xmm10, xmm13			; 1FE6 _ C4 C1 29: EF. E5
	vpslld	xmm8, xmm9, 7				; 1FEB _ C4 C1 39: 72. F1, 07
	vpxor	xmm5, xmm12, xmm9			; 1FF1 _ C4 C1 19: EF. E9
	vpxor	xmm10, xmm4, xmm8			; 1FF6 _ C4 41 59: EF. D0
	vpxor	xmm5, xmm5, xmm13			; 1FFB _ C4 C1 51: EF. ED
	vpslld	xmm4, xmm13, 25 			; 2000 _ C4 C1 59: 72. F5, 19
	vpsrld	xmm13, xmm13, 7 			; 2006 _ C4 C1 11: 72. D5, 07
	vpslld	xmm11, xmm5, 3				; 200C _ C5 A1: 72. F5, 03
	vpor	xmm4, xmm4, xmm13			; 2011 _ C4 C1 59: EB. E5
	vpslld	xmm12, xmm9, 31 			; 2016 _ C4 C1 19: 72. F1, 1F
	vpxor	xmm8, xmm4, xmm10			; 201C _ C4 41 59: EF. C2
	vpsrld	xmm9, xmm9, 1				; 2021 _ C4 C1 31: 72. D1, 01
	vpxor	xmm4, xmm8, xmm11			; 2027 _ C4 C1 39: EF. E3
	vpor	xmm8, xmm12, xmm9			; 202C _ C4 41 19: EB. C1
	vpxor	xmm8, xmm8, xmm5			; 2031 _ C5 39: EF. C5
	vpslld	xmm12, xmm10, 29			; 2035 _ C4 C1 19: 72. F2, 1D
	vpxor	xmm8, xmm8, xmm10			; 203B _ C4 41 39: EF. C2
	vpsrld	xmm10, xmm10, 3 			; 2040 _ C4 C1 29: 72. D2, 03
	vpslld	xmm9, xmm5, 19				; 2046 _ C5 B1: 72. F5, 13
	vpsrld	xmm5, xmm5, 13				; 204B _ C5 D1: 72. D5, 0D
	vpor	xmm11, xmm12, xmm10			; 2050 _ C4 41 19: EB. DA
	vpor	xmm5, xmm9, xmm5			; 2055 _ C5 B1: EB. ED
	vpxor	xmm5, xmm5, xmm11			; 2059 _ C4 C1 51: EF. EB
	vpxor	xmm13, xmm4, xmm8			; 205E _ C4 41 59: EF. E8
	vpand	xmm12, xmm11, xmm5			; 2063 _ C5 21: DB. E5
	vpxor	xmm4, xmm11, xmm4			; 2067 _ C5 A1: EF. E4
	vpxor	xmm10, xmm12, xmm6			; 206B _ C5 19: EF. D6
	vpxor	xmm9, xmm10, xmm13			; 206F _ C4 41 29: EF. CD
	vpor	xmm10, xmm4, xmm5			; 2074 _ C5 59: EB. D5
	vpxor	xmm12, xmm13, xmm10			; 2078 _ C4 41 11: EF. E2
	vpxor	xmm11, xmm5, xmm9			; 207D _ C4 41 51: EF. D9
	vmovd	xmm4, dword [r12+12E0H] 		; 2082 _ C4 C1 79: 6E. A4 24, 000012E0
	vpand	xmm5, xmm8, xmm12			; 208C _ C4 C1 39: DB. EC
	vpxor	xmm13, xmm5, xmm11			; 2091 _ C4 41 51: EF. EB
	vpxor	xmm10, xmm10, xmm8			; 2096 _ C4 41 29: EF. D0
	vpshufd xmm5, xmm4, 0				; 209B _ C5 F9: 70. EC, 00
	vpxor	xmm8, xmm11, xmm12			; 20A0 _ C4 41 21: EF. C4
	vpxor	xmm4, xmm13, xmm5			; 20A5 _ C5 91: EF. E5
	vpor	xmm11, xmm8, xmm9			; 20A9 _ C4 41 39: EB. D9
	vmovd	xmm5, dword [r12+12E4H] 		; 20AE _ C4 C1 79: 6E. AC 24, 000012E4
	vpxor	xmm8, xmm10, xmm11			; 20B8 _ C4 41 29: EF. C3
	vpshufd xmm5, xmm5, 0				; 20BD _ C5 F9: 70. ED, 00
	vpxor	xmm5, xmm9, xmm5			; 20C2 _ C5 B1: EF. ED
	vmovd	xmm9, dword [r12+12E8H] 		; 20C6 _ C4 41 79: 6E. 8C 24, 000012E8
	vpshufd xmm11, xmm9, 0				; 20D0 _ C4 41 79: 70. D9, 00
	vpxor	xmm9, xmm8, xmm11			; 20D6 _ C4 41 39: EF. CB
	vpxor	xmm8, xmm12, xmm13			; 20DB _ C4 41 19: EF. C5
	vmovd	xmm13, dword [r12+12ECH]		; 20E0 _ C4 41 79: 6E. AC 24, 000012EC
	vpslld	xmm12, xmm9, 10 			; 20EA _ C4 C1 19: 72. F1, 0A
	vpshufd xmm13, xmm13, 0 			; 20F0 _ C4 41 79: 70. ED, 00
	vpsrld	xmm10, xmm9, 22 			; 20F6 _ C4 C1 29: 72. D1, 16
	vpxor	xmm11, xmm8, xmm13			; 20FC _ C4 41 39: EF. DD
	vpor	xmm9, xmm12, xmm10			; 2101 _ C4 41 19: EB. CA
	vpslld	xmm12, xmm4, 27 			; 2106 _ C5 99: 72. F4, 1B
	vpsrld	xmm4, xmm4, 5				; 210B _ C5 D9: 72. D4, 05
	vpxor	xmm13, xmm9, xmm11			; 2110 _ C4 41 31: EF. EB
	vpslld	xmm8, xmm5, 7				; 2115 _ C5 B9: 72. F5, 07
	vpor	xmm10, xmm12, xmm4			; 211A _ C5 19: EB. D4
	vpxor	xmm8, xmm13, xmm8			; 211E _ C4 41 11: EF. C0
	vpxor	xmm13, xmm10, xmm5			; 2123 _ C5 29: EF. ED
	vpslld	xmm4, xmm11, 25 			; 2127 _ C4 C1 59: 72. F3, 19
	vpxor	xmm10, xmm13, xmm11			; 212D _ C4 41 11: EF. D3
	vpsrld	xmm11, xmm11, 7 			; 2132 _ C4 C1 21: 72. D3, 07
	vpor	xmm13, xmm4, xmm11			; 2138 _ C4 41 59: EB. EB
	vpslld	xmm11, xmm10, 3 			; 213D _ C4 C1 21: 72. F2, 03
	vpxor	xmm4, xmm13, xmm8			; 2143 _ C4 C1 11: EF. E0
	vpslld	xmm9, xmm10, 19 			; 2148 _ C4 C1 31: 72. F2, 13
	vpxor	xmm12, xmm4, xmm11			; 214E _ C4 41 59: EF. E3
	vpslld	xmm4, xmm5, 31				; 2153 _ C5 D9: 72. F5, 1F
	vpsrld	xmm5, xmm5, 1				; 2158 _ C5 D1: 72. D5, 01
	vpsrld	xmm13, xmm10, 13			; 215D _ C4 C1 11: 72. D2, 0D
	vpor	xmm5, xmm4, xmm5			; 2163 _ C5 D9: EB. ED
	vpor	xmm11, xmm9, xmm13			; 2167 _ C4 41 31: EB. DD
	vpxor	xmm13, xmm5, xmm10			; 216C _ C4 41 51: EF. EA
	vpslld	xmm10, xmm8, 29 			; 2171 _ C4 C1 29: 72. F0, 1D
	vpxor	xmm4, xmm13, xmm8			; 2177 _ C4 C1 11: EF. E0
	vpsrld	xmm8, xmm8, 3				; 217C _ C4 C1 39: 72. D0, 03
	vpxor	xmm4, xmm4, xmm6			; 2182 _ C5 D9: EF. E6
	vpor	xmm9, xmm10, xmm8			; 2186 _ C4 41 29: EB. C8
	vpxor	xmm8, xmm9, xmm4			; 218B _ C5 31: EF. C4
	vpor	xmm5, xmm12, xmm11			; 218F _ C4 C1 19: EB. EB
	vpxor	xmm5, xmm5, xmm8			; 2194 _ C4 C1 51: EF. E8
	vpxor	xmm13, xmm12, xmm5			; 2199 _ C5 19: EF. ED
	vpor	xmm12, xmm8, xmm4			; 219D _ C5 39: EB. E4
	vpand	xmm8, xmm12, xmm11			; 21A1 _ C4 41 19: DB. C3
	vpxor	xmm12, xmm8, xmm13			; 21A6 _ C4 41 39: EF. E5
	vpor	xmm13, xmm13, xmm11			; 21AB _ C4 41 11: EB. EB
	vpand	xmm10, xmm4, xmm12			; 21B0 _ C4 41 59: DB. D4
	vpxor	xmm4, xmm13, xmm4			; 21B5 _ C5 91: EF. E4
	vpxor	xmm8, xmm10, xmm5			; 21B9 _ C5 29: EF. C5
	vpxor	xmm13, xmm4, xmm12			; 21BD _ C4 41 59: EF. EC
	vmovd	xmm4, dword [r12+12D0H] 		; 21C2 _ C4 C1 79: 6E. A4 24, 000012D0
	vpxor	xmm9, xmm13, xmm8			; 21CC _ C4 41 11: EF. C8
	vpshufd xmm10, xmm4, 0				; 21D1 _ C5 79: 70. D4, 00
	vpand	xmm5, xmm5, xmm13			; 21D6 _ C4 C1 51: DB. ED
	vmovd	xmm4, dword [r12+12D4H] 		; 21DB _ C4 C1 79: 6E. A4 24, 000012D4
	vpxor	xmm8, xmm8, xmm10			; 21E5 _ C4 41 39: EF. C2
	vpxor	xmm10, xmm9, xmm6			; 21EA _ C5 31: EF. D6
	vpxor	xmm9, xmm5, xmm9			; 21EE _ C4 41 51: EF. C9
	vpshufd xmm4, xmm4, 0				; 21F3 _ C5 F9: 70. E4, 00
	vpxor	xmm5, xmm9, xmm11			; 21F8 _ C4 C1 31: EF. EB
	vmovd	xmm11, dword [r12+12D8H]		; 21FD _ C4 41 79: 6E. 9C 24, 000012D8
	vpxor	xmm10, xmm10, xmm4			; 2207 _ C5 29: EF. D4
	vmovd	xmm4, dword [r12+12DCH] 		; 220B _ C4 C1 79: 6E. A4 24, 000012DC
	vpshufd xmm13, xmm11, 0 			; 2215 _ C4 41 79: 70. EB, 00
	vpshufd xmm11, xmm4, 0				; 221B _ C5 79: 70. DC, 00
	vpxor	xmm9, xmm5, xmm13			; 2220 _ C4 41 51: EF. CD
	vpxor	xmm4, xmm12, xmm11			; 2225 _ C4 C1 19: EF. E3
	vpslld	xmm12, xmm9, 10 			; 222A _ C4 C1 19: 72. F1, 0A
	vpsrld	xmm5, xmm9, 22				; 2230 _ C4 C1 51: 72. D1, 16
	vpor	xmm13, xmm12, xmm5			; 2236 _ C5 19: EB. ED
	vpslld	xmm5, xmm8, 27				; 223A _ C4 C1 51: 72. F0, 1B
	vpsrld	xmm8, xmm8, 5				; 2240 _ C4 C1 39: 72. D0, 05
	vpxor	xmm11, xmm13, xmm4			; 2246 _ C5 11: EF. DC
	vpor	xmm5, xmm5, xmm8			; 224A _ C4 C1 51: EB. E8
	vpslld	xmm12, xmm10, 7 			; 224F _ C4 C1 19: 72. F2, 07
	vpxor	xmm13, xmm5, xmm10			; 2255 _ C4 41 51: EF. EA
	vpxor	xmm9, xmm11, xmm12			; 225A _ C4 41 21: EF. CC
	vpxor	xmm11, xmm13, xmm4			; 225F _ C5 11: EF. DC
	vpslld	xmm8, xmm4, 25				; 2263 _ C5 B9: 72. F4, 19
	vpsrld	xmm4, xmm4, 7				; 2268 _ C5 D9: 72. D4, 07
	vpslld	xmm12, xmm10, 31			; 226D _ C4 C1 19: 72. F2, 1F
	vpor	xmm5, xmm8, xmm4			; 2273 _ C5 B9: EB. EC
	vpsrld	xmm10, xmm10, 1 			; 2277 _ C4 C1 29: 72. D2, 01
	vpxor	xmm13, xmm5, xmm9			; 227D _ C4 41 51: EF. E9
	vpslld	xmm8, xmm11, 3				; 2282 _ C4 C1 39: 72. F3, 03
	vpor	xmm5, xmm12, xmm10			; 2288 _ C4 C1 19: EB. EA
	vpxor	xmm4, xmm13, xmm8			; 228D _ C4 C1 11: EF. E0
	vpxor	xmm13, xmm5, xmm11			; 2292 _ C4 41 51: EF. EB
	vpslld	xmm8, xmm9, 29				; 2297 _ C4 C1 39: 72. F1, 1D
	vpxor	xmm10, xmm13, xmm9			; 229D _ C4 41 11: EF. D1
	vpsrld	xmm9, xmm9, 3				; 22A2 _ C4 C1 31: 72. D1, 03
	vpor	xmm12, xmm8, xmm9			; 22A8 _ C4 41 39: EB. E1
	vpslld	xmm5, xmm11, 19 			; 22AD _ C4 C1 51: 72. F3, 13
	vpsrld	xmm11, xmm11, 13			; 22B3 _ C4 C1 21: 72. D3, 0D
	vpor	xmm13, xmm5, xmm11			; 22B9 _ C4 41 51: EB. EB
	vpand	xmm5, xmm12, xmm4			; 22BE _ C5 99: DB. EC
	vpxor	xmm11, xmm5, xmm10			; 22C2 _ C4 41 51: EF. DA
	vpor	xmm10, xmm10, xmm4			; 22C7 _ C5 29: EB. D4
	vpand	xmm9, xmm10, xmm13			; 22CB _ C4 41 29: DB. CD
	vpxor	xmm12, xmm12, xmm11			; 22D0 _ C4 41 19: EF. E3
	vpxor	xmm12, xmm12, xmm9			; 22D5 _ C4 41 19: EF. E1
	vpxor	xmm5, xmm13, xmm6			; 22DA _ C5 91: EF. EE
	vpxor	xmm4, xmm4, xmm12			; 22DE _ C4 C1 59: EF. E4
	vpand	xmm13, xmm9, xmm11			; 22E3 _ C4 41 31: DB. EB
	vpxor	xmm10, xmm13, xmm4			; 22E8 _ C5 11: EF. D4
	vpand	xmm9, xmm4, xmm5			; 22EC _ C5 59: DB. CD
	vmovd	xmm4, dword [r12+12C0H] 		; 22F0 _ C4 C1 79: 6E. A4 24, 000012C0
	vpxor	xmm8, xmm5, xmm10			; 22FA _ C4 41 51: EF. C2
	vpxor	xmm5, xmm9, xmm11			; 22FF _ C4 C1 31: EF. EB
	vpshufd xmm9, xmm4, 0				; 2304 _ C5 79: 70. CC, 00
	vpxor	xmm13, xmm5, xmm8			; 2309 _ C4 41 51: EF. E8
	vpxor	xmm9, xmm8, xmm9			; 230E _ C4 41 39: EF. C9
	vpxor	xmm5, xmm13, xmm8			; 2313 _ C4 C1 11: EF. E8
	vpand	xmm8, xmm11, xmm8			; 2318 _ C4 41 21: DB. C0
	vpxor	xmm11, xmm8, xmm12			; 231D _ C4 41 39: EF. DC
	vmovd	xmm4, dword [r12+12C4H] 		; 2322 _ C4 C1 79: 6E. A4 24, 000012C4
	vpor	xmm13, xmm11, xmm13			; 232C _ C4 41 21: EB. ED
	vpshufd xmm4, xmm4, 0				; 2331 _ C5 F9: 70. E4, 00
	vpxor	xmm13, xmm13, xmm10			; 2336 _ C4 41 11: EF. EA
	vmovd	xmm10, dword [r12+12C8H]		; 233B _ C4 41 79: 6E. 94 24, 000012C8
	vpxor	xmm5, xmm5, xmm4			; 2345 _ C5 D1: EF. EC
	vmovd	xmm4, dword [r12+12CCH] 		; 2349 _ C4 C1 79: 6E. A4 24, 000012CC
	vpshufd xmm8, xmm10, 0				; 2353 _ C4 41 79: 70. C2, 00
	vpshufd xmm11, xmm4, 0				; 2359 _ C5 79: 70. DC, 00
	vpxor	xmm10, xmm13, xmm8			; 235E _ C4 41 11: EF. D0
	vpxor	xmm13, xmm12, xmm11			; 2363 _ C4 41 19: EF. EB
	vpslld	xmm12, xmm10, 10			; 2368 _ C4 C1 19: 72. F2, 0A
	vpsrld	xmm8, xmm10, 22 			; 236E _ C4 C1 39: 72. D2, 16
	vpslld	xmm10, xmm9, 27 			; 2374 _ C4 C1 29: 72. F1, 1B
	vpsrld	xmm9, xmm9, 5				; 237A _ C4 C1 31: 72. D1, 05
	vpor	xmm4, xmm12, xmm8			; 2380 _ C4 C1 19: EB. E0
	vpor	xmm8, xmm10, xmm9			; 2385 _ C4 41 29: EB. C1
	vpxor	xmm11, xmm4, xmm13			; 238A _ C4 41 59: EF. DD
	vpslld	xmm12, xmm5, 7				; 238F _ C5 99: 72. F5, 07
	vpxor	xmm4, xmm8, xmm5			; 2394 _ C5 B9: EF. E5
	vpxor	xmm11, xmm11, xmm12			; 2398 _ C4 41 21: EF. DC
	vpxor	xmm4, xmm4, xmm13			; 239D _ C4 C1 59: EF. E5
	vpslld	xmm12, xmm13, 25			; 23A2 _ C4 C1 19: 72. F5, 19
	vpsrld	xmm13, xmm13, 7 			; 23A8 _ C4 C1 11: 72. D5, 07
	vpor	xmm10, xmm12, xmm13			; 23AE _ C4 41 19: EB. D5
	vpslld	xmm8, xmm5, 31				; 23B3 _ C5 B9: 72. F5, 1F
	vpsrld	xmm5, xmm5, 1				; 23B8 _ C5 D1: 72. D5, 01
	vpxor	xmm9, xmm10, xmm11			; 23BD _ C4 41 29: EF. CB
	vpslld	xmm13, xmm4, 3				; 23C2 _ C5 91: 72. F4, 03
	vpor	xmm5, xmm8, xmm5			; 23C7 _ C5 B9: EB. ED
	vpxor	xmm9, xmm9, xmm13			; 23CB _ C4 41 31: EF. CD
	vpxor	xmm13, xmm5, xmm4			; 23D0 _ C5 51: EF. EC
	vpxor	xmm13, xmm13, xmm11			; 23D4 _ C4 41 11: EF. EB
	vpslld	xmm8, xmm11, 29 			; 23D9 _ C4 C1 39: 72. F3, 1D
	vpsrld	xmm11, xmm11, 3 			; 23DF _ C4 C1 21: 72. D3, 03
	vpor	xmm8, xmm8, xmm11			; 23E5 _ C4 41 39: EB. C3
	vpslld	xmm11, xmm4, 19 			; 23EA _ C5 A1: 72. F4, 13
	vpsrld	xmm4, xmm4, 13				; 23EF _ C5 D9: 72. D4, 0D
	vpxor	xmm5, xmm8, xmm13			; 23F4 _ C4 C1 39: EF. ED
	vpor	xmm12, xmm11, xmm4			; 23F9 _ C5 21: EB. E4
	vpand	xmm8, xmm8, xmm5			; 23FD _ C5 39: DB. C5
	vpxor	xmm12, xmm12, xmm5			; 2401 _ C5 19: EF. E5
	vpxor	xmm4, xmm8, xmm12			; 2405 _ C4 C1 39: EF. E4
	vpand	xmm8, xmm12, xmm13			; 240A _ C4 41 19: DB. C5
	vpor	xmm10, xmm9, xmm4			; 240F _ C5 31: EB. D4
	vpxor	xmm13, xmm13, xmm9			; 2413 _ C4 41 11: EF. E9
	vpxor	xmm5, xmm5, xmm10			; 2418 _ C4 C1 51: EF. EA
	vpxor	xmm12, xmm8, xmm10			; 241D _ C4 41 39: EF. E2
	vpxor	xmm9, xmm13, xmm4			; 2422 _ C5 11: EF. CC
	vpand	xmm10, xmm10, xmm5			; 2426 _ C5 29: DB. D5
	vpxor	xmm11, xmm10, xmm9			; 242A _ C4 41 29: EF. D9
	vpxor	xmm13, xmm9, xmm12			; 242F _ C4 41 31: EF. EC
	vmovd	xmm10, dword [r12+12B0H]		; 2434 _ C4 41 79: 6E. 94 24, 000012B0
	vpor	xmm8, xmm13, xmm5			; 243E _ C5 11: EB. C5
	vpshufd xmm9, xmm10, 0				; 2442 _ C4 41 79: 70. CA, 00
	vpxor	xmm4, xmm8, xmm4			; 2448 _ C5 B9: EF. E4
	vmovd	xmm10, dword [r12+12B8H]		; 244C _ C4 41 79: 6E. 94 24, 000012B8
	vpxor	xmm5, xmm5, xmm9			; 2456 _ C4 C1 51: EF. E9
	vmovd	xmm13, dword [r12+12B4H]		; 245B _ C4 41 79: 6E. AC 24, 000012B4
	vpshufd xmm9, xmm10, 0				; 2465 _ C4 41 79: 70. CA, 00
	vpshufd xmm8, xmm13, 0				; 246B _ C4 41 79: 70. C5, 00
	vpxor	xmm9, xmm11, xmm9			; 2471 _ C4 41 21: EF. C9
	vpxor	xmm11, xmm12, xmm11			; 2476 _ C4 41 19: EF. DB
	vpxor	xmm13, xmm4, xmm8			; 247B _ C4 41 59: EF. E8
	vpxor	xmm12, xmm11, xmm4			; 2480 _ C5 21: EF. E4
	vpsrld	xmm11, xmm9, 22 			; 2484 _ C4 C1 21: 72. D1, 16
	vmovd	xmm4, dword [r12+12BCH] 		; 248A _ C4 C1 79: 6E. A4 24, 000012BC
	vpshufd xmm8, xmm4, 0				; 2494 _ C5 79: 70. C4, 00
	vpslld	xmm4, xmm9, 10				; 2499 _ C4 C1 59: 72. F1, 0A
	vpxor	xmm8, xmm12, xmm8			; 249F _ C4 41 19: EF. C0
	vpor	xmm12, xmm4, xmm11			; 24A4 _ C4 41 59: EB. E3
	vpslld	xmm4, xmm5, 27				; 24A9 _ C5 D9: 72. F5, 1B
	vpsrld	xmm5, xmm5, 5				; 24AE _ C5 D1: 72. D5, 05
	vpor	xmm5, xmm4, xmm5			; 24B3 _ C5 D9: EB. ED
	vpxor	xmm10, xmm12, xmm8			; 24B7 _ C4 41 19: EF. D0
	vpxor	xmm4, xmm5, xmm13			; 24BC _ C4 C1 51: EF. E5
	vpslld	xmm9, xmm13, 7				; 24C1 _ C4 C1 31: 72. F5, 07
	vpxor	xmm4, xmm4, xmm8			; 24C7 _ C4 C1 59: EF. E0
	vpslld	xmm11, xmm8, 25 			; 24CC _ C4 C1 21: 72. F0, 19
	vpsrld	xmm8, xmm8, 7				; 24D2 _ C4 C1 39: 72. D0, 07
	vpxor	xmm10, xmm10, xmm9			; 24D8 _ C4 41 29: EF. D1
	vpor	xmm12, xmm11, xmm8			; 24DD _ C4 41 21: EB. E0
	vpslld	xmm5, xmm4, 3				; 24E2 _ C5 D1: 72. F4, 03
	vpxor	xmm9, xmm12, xmm10			; 24E7 _ C4 41 19: EF. CA
	vpslld	xmm8, xmm13, 31 			; 24EC _ C4 C1 39: 72. F5, 1F
	vpsrld	xmm13, xmm13, 1 			; 24F2 _ C4 C1 11: 72. D5, 01
	vpxor	xmm12, xmm9, xmm5			; 24F8 _ C5 31: EF. E5
	vpor	xmm5, xmm8, xmm13			; 24FC _ C4 C1 39: EB. ED
	vpslld	xmm8, xmm4, 19				; 2501 _ C5 B9: 72. F4, 13
	vpxor	xmm13, xmm5, xmm4			; 2506 _ C5 51: EF. EC
	vpsrld	xmm4, xmm4, 13				; 250A _ C5 D9: 72. D4, 0D
	vpxor	xmm11, xmm13, xmm10			; 250F _ C4 41 11: EF. DA
	vpslld	xmm9, xmm10, 29 			; 2514 _ C4 C1 31: 72. F2, 1D
	vpsrld	xmm10, xmm10, 3 			; 251A _ C4 C1 29: 72. D2, 03
	vpor	xmm8, xmm8, xmm4			; 2520 _ C5 39: EB. C4
	vpor	xmm5, xmm9, xmm10			; 2524 _ C4 C1 31: EB. EA
	vpxor	xmm4, xmm5, xmm12			; 2529 _ C4 C1 51: EF. E4
	vpxor	xmm5, xmm12, xmm8			; 252E _ C4 C1 19: EF. E8
	vpand	xmm12, xmm5, xmm4			; 2533 _ C5 51: DB. E4
	vpxor	xmm13, xmm12, xmm11			; 2537 _ C4 41 19: EF. EB
	vpor	xmm11, xmm11, xmm4			; 253C _ C5 21: EB. DC
	vpxor	xmm12, xmm11, xmm5			; 2540 _ C5 21: EF. E5
	vpand	xmm5, xmm5, xmm13			; 2544 _ C4 C1 51: DB. ED
	vpxor	xmm10, xmm4, xmm13			; 2549 _ C4 41 59: EF. D5
	vpand	xmm4, xmm5, xmm8			; 254E _ C4 C1 51: DB. E0
	vmovd	xmm9, dword [r12+12A0H] 		; 2553 _ C4 41 79: 6E. 8C 24, 000012A0
	vpxor	xmm11, xmm4, xmm10			; 255D _ C4 41 59: EF. DA
	vmovd	xmm4, dword [r12+12A4H] 		; 2562 _ C4 C1 79: 6E. A4 24, 000012A4
	vpand	xmm10, xmm10, xmm12			; 256C _ C4 41 29: DB. D4
	vpshufd xmm5, xmm9, 0				; 2571 _ C4 C1 79: 70. E9, 00
	vpxor	xmm13, xmm13, xmm6			; 2577 _ C5 11: EF. EE
	vpshufd xmm9, xmm4, 0				; 257B _ C5 79: 70. CC, 00
	vpor	xmm10, xmm10, xmm8			; 2580 _ C4 41 29: EB. D0
	vpxor	xmm4, xmm11, xmm9			; 2585 _ C4 C1 21: EF. E1
	vpxor	xmm10, xmm10, xmm13			; 258A _ C4 41 29: EF. D5
	vpxor	xmm11, xmm13, xmm11			; 258F _ C4 41 11: EF. DB
	vpxor	xmm13, xmm8, xmm13			; 2594 _ C4 41 39: EF. ED
	vmovd	xmm9, dword [r12+12A8H] 		; 2599 _ C4 41 79: 6E. 8C 24, 000012A8
	vpand	xmm8, xmm13, xmm12			; 25A3 _ C4 41 11: DB. C4
	vpshufd xmm9, xmm9, 0				; 25A8 _ C4 41 79: 70. C9, 00
	vpxor	xmm5, xmm12, xmm5			; 25AE _ C5 99: EF. ED
	vpxor	xmm12, xmm11, xmm8			; 25B2 _ C4 41 21: EF. E0
	vpxor	xmm10, xmm10, xmm9			; 25B7 _ C4 41 29: EF. D1
	vmovd	xmm11, dword [r12+12ACH]		; 25BC _ C4 41 79: 6E. 9C 24, 000012AC
	vpshufd xmm13, xmm11, 0 			; 25C6 _ C4 41 79: 70. EB, 00
	vpslld	xmm11, xmm10, 10			; 25CC _ C4 C1 21: 72. F2, 0A
	vpxor	xmm8, xmm12, xmm13			; 25D2 _ C4 41 19: EF. C5
	vpsrld	xmm12, xmm10, 22			; 25D7 _ C4 C1 19: 72. D2, 16
	vpor	xmm10, xmm11, xmm12			; 25DD _ C4 41 21: EB. D4
	vpslld	xmm11, xmm5, 27 			; 25E2 _ C5 A1: 72. F5, 1B
	vpsrld	xmm5, xmm5, 5				; 25E7 _ C5 D1: 72. D5, 05
	vpxor	xmm9, xmm10, xmm8			; 25EC _ C4 41 29: EF. C8
	vpor	xmm5, xmm11, xmm5			; 25F1 _ C5 A1: EB. ED
	vpslld	xmm13, xmm4, 7				; 25F5 _ C5 91: 72. F4, 07
	vpxor	xmm11, xmm5, xmm4			; 25FA _ C5 51: EF. DC
	vpslld	xmm12, xmm8, 25 			; 25FE _ C4 C1 19: 72. F0, 19
	vpxor	xmm11, xmm11, xmm8			; 2604 _ C4 41 21: EF. D8
	vpsrld	xmm8, xmm8, 7				; 2609 _ C4 C1 39: 72. D0, 07
	vpxor	xmm13, xmm9, xmm13			; 260F _ C4 41 31: EF. ED
	vpor	xmm5, xmm12, xmm8			; 2614 _ C4 C1 19: EB. E8
	vpslld	xmm10, xmm4, 31 			; 2619 _ C5 A9: 72. F4, 1F
	vpsrld	xmm4, xmm4, 1				; 261E _ C5 D9: 72. D4, 01
	vpxor	xmm8, xmm5, xmm13			; 2623 _ C4 41 51: EF. C5
	vpor	xmm5, xmm10, xmm4			; 2628 _ C5 A9: EB. EC
	vpslld	xmm12, xmm11, 3 			; 262C _ C4 C1 19: 72. F3, 03
	vpxor	xmm4, xmm5, xmm11			; 2632 _ C4 C1 51: EF. E3
	vpxor	xmm8, xmm8, xmm12			; 2637 _ C4 41 39: EF. C4
	vpxor	xmm9, xmm4, xmm13			; 263C _ C4 41 59: EF. CD
	vpslld	xmm10, xmm11, 19			; 2641 _ C4 C1 29: 72. F3, 13
	vpsrld	xmm11, xmm11, 13			; 2647 _ C4 C1 21: 72. D3, 0D
	vpslld	xmm12, xmm13, 29			; 264D _ C4 C1 19: 72. F5, 1D
	vpsrld	xmm13, xmm13, 3 			; 2653 _ C4 C1 11: 72. D5, 03
	vpor	xmm4, xmm10, xmm11			; 2659 _ C4 C1 29: EB. E3
	vpxor	xmm11, xmm9, xmm8			; 265E _ C4 41 31: EF. D8
	vpor	xmm5, xmm12, xmm13			; 2663 _ C4 C1 19: EB. ED
	vpand	xmm13, xmm8, xmm11			; 2668 _ C4 41 39: DB. EB
	vpxor	xmm9, xmm9, xmm5			; 266D _ C5 31: EF. CD
	vpxor	xmm8, xmm13, xmm4			; 2671 _ C5 11: EF. C4
	vpor	xmm4, xmm4, xmm11			; 2675 _ C4 C1 59: EB. E3
	vpxor	xmm5, xmm5, xmm8			; 267A _ C4 C1 51: EF. E8
	vpxor	xmm10, xmm11, xmm8			; 267F _ C4 41 21: EF. D0
	vpxor	xmm11, xmm4, xmm9			; 2684 _ C4 41 59: EF. D9
	vpor	xmm12, xmm11, xmm5			; 2689 _ C5 21: EB. E5
	vpxor	xmm9, xmm9, xmm6			; 268D _ C5 31: EF. CE
	vpxor	xmm4, xmm12, xmm10			; 2691 _ C4 C1 19: EF. E2
	vpor	xmm10, xmm10, xmm8			; 2696 _ C4 41 29: EB. D0
	vmovd	xmm11, dword [r12+1290H]		; 269B _ C4 41 79: 6E. 9C 24, 00001290
	vpxor	xmm12, xmm10, xmm4			; 26A5 _ C5 29: EF. E4
	vpshufd xmm10, xmm11, 0 			; 26A9 _ C4 41 79: 70. D3, 00
	vpxor	xmm13, xmm9, xmm12			; 26AF _ C4 41 31: EF. EC
	vmovd	xmm11, dword [r12+1294H]		; 26B4 _ C4 41 79: 6E. 9C 24, 00001294
	vpxor	xmm9, xmm13, xmm10			; 26BE _ C4 41 11: EF. CA
	vpshufd xmm10, xmm11, 0 			; 26C3 _ C4 41 79: 70. D3, 00
	vpor	xmm12, xmm12, xmm4			; 26C9 _ C5 19: EB. E4
	vpxor	xmm11, xmm4, xmm10			; 26CD _ C4 41 59: EF. DA
	vpxor	xmm4, xmm12, xmm4			; 26D2 _ C5 99: EF. E4
	vpor	xmm13, xmm4, xmm13			; 26D6 _ C4 41 59: EB. ED
	vpxor	xmm13, xmm8, xmm13			; 26DB _ C4 41 39: EF. ED
	vmovd	xmm8, dword [r12+1298H] 		; 26E0 _ C4 41 79: 6E. 84 24, 00001298
	vmovd	xmm4, dword [r12+129CH] 		; 26EA _ C4 C1 79: 6E. A4 24, 0000129C
	vpshufd xmm8, xmm8, 0				; 26F4 _ C4 41 79: 70. C0, 00
	vpshufd xmm12, xmm4, 0				; 26FA _ C5 79: 70. E4, 00
	vpxor	xmm13, xmm13, xmm8			; 26FF _ C4 41 11: EF. E8
	vpxor	xmm10, xmm5, xmm12			; 2704 _ C4 41 51: EF. D4
	vpslld	xmm5, xmm13, 10 			; 2709 _ C4 C1 51: 72. F5, 0A
	vpsrld	xmm13, xmm13, 22			; 270F _ C4 C1 11: 72. D5, 16
	vpslld	xmm12, xmm9, 27 			; 2715 _ C4 C1 19: 72. F1, 1B
	vpor	xmm5, xmm5, xmm13			; 271B _ C4 C1 51: EB. ED
	vpsrld	xmm9, xmm9, 5				; 2720 _ C4 C1 31: 72. D1, 05
	vpxor	xmm8, xmm5, xmm10			; 2726 _ C4 41 51: EF. C2
	vpor	xmm5, xmm12, xmm9			; 272B _ C4 C1 19: EB. E9
	vpslld	xmm4, xmm11, 7				; 2730 _ C4 C1 59: 72. F3, 07
	vpxor	xmm13, xmm5, xmm11			; 2736 _ C4 41 51: EF. EB
	vpxor	xmm8, xmm8, xmm4			; 273B _ C5 39: EF. C4
	vpxor	xmm9, xmm13, xmm10			; 273F _ C4 41 11: EF. CA
	vpslld	xmm4, xmm10, 25 			; 2744 _ C4 C1 59: 72. F2, 19
	vpsrld	xmm10, xmm10, 7 			; 274A _ C4 C1 29: 72. D2, 07
	vpor	xmm5, xmm4, xmm10			; 2750 _ C4 C1 59: EB. EA
	vpslld	xmm12, xmm11, 31			; 2755 _ C4 C1 19: 72. F3, 1F
	vpsrld	xmm11, xmm11, 1 			; 275B _ C4 C1 21: 72. D3, 01
	vpxor	xmm13, xmm5, xmm8			; 2761 _ C4 41 51: EF. E8
	vpslld	xmm4, xmm9, 3				; 2766 _ C4 C1 59: 72. F1, 03
	vpor	xmm5, xmm12, xmm11			; 276C _ C4 C1 19: EB. EB
	vpxor	xmm10, xmm13, xmm4			; 2771 _ C5 11: EF. D4
	vpxor	xmm13, xmm5, xmm9			; 2775 _ C4 41 51: EF. E9
	vpxor	xmm5, xmm13, xmm8			; 277A _ C4 C1 11: EF. E8
	vpslld	xmm4, xmm9, 19				; 277F _ C4 C1 59: 72. F1, 13
	vpsrld	xmm9, xmm9, 13				; 2785 _ C4 C1 31: 72. D1, 0D
	vpslld	xmm13, xmm8, 29 			; 278B _ C4 C1 11: 72. F0, 1D
	vpsrld	xmm8, xmm8, 3				; 2791 _ C4 C1 39: 72. D0, 03
	vpor	xmm12, xmm4, xmm9			; 2797 _ C4 41 59: EB. E1
	vpor	xmm11, xmm13, xmm8			; 279C _ C4 41 11: EB. D8
	vpxor	xmm4, xmm5, xmm6			; 27A1 _ C5 D1: EF. E6
	vpxor	xmm13, xmm11, xmm6			; 27A5 _ C5 21: EF. EE
	vpor	xmm5, xmm5, xmm12			; 27A9 _ C4 C1 51: EB. EC
	vpxor	xmm8, xmm5, xmm13			; 27AE _ C4 41 51: EF. C5
	vpxor	xmm12, xmm12, xmm4			; 27B3 _ C5 19: EF. E4
	vpxor	xmm5, xmm8, xmm10			; 27B7 _ C4 C1 39: EF. EA
	vpor	xmm13, xmm13, xmm4			; 27BC _ C5 11: EB. EC
	vpand	xmm9, xmm12, xmm10			; 27C0 _ C4 41 19: DB. CA
	vpxor	xmm11, xmm13, xmm12			; 27C5 _ C4 41 11: EF. DC
	vpxor	xmm13, xmm4, xmm9			; 27CA _ C4 41 59: EF. E9
	vpor	xmm4, xmm9, xmm5			; 27CF _ C5 B1: EB. E5
	vpxor	xmm12, xmm4, xmm11			; 27D3 _ C4 41 59: EF. E3
	vpxor	xmm10, xmm10, xmm13			; 27D8 _ C4 41 29: EF. D5
	vpxor	xmm8, xmm10, xmm12			; 27DD _ C4 41 29: EF. C4
	vpxor	xmm10, xmm11, xmm5			; 27E2 _ C5 21: EF. D5
	vmovd	xmm4, dword [r12+1280H] 		; 27E6 _ C4 C1 79: 6E. A4 24, 00001280
	vpxor	xmm8, xmm8, xmm5			; 27F0 _ C5 39: EF. C5
	vpshufd xmm9, xmm4, 0				; 27F4 _ C5 79: 70. CC, 00
	vpand	xmm11, xmm10, xmm8			; 27F9 _ C4 41 29: DB. D8
	vpxor	xmm4, xmm12, xmm9			; 27FE _ C4 C1 19: EF. E1
	vpxor	xmm9, xmm13, xmm11			; 2803 _ C4 41 11: EF. CB
	vmovd	xmm12, dword [r12+1284H]		; 2808 _ C4 41 79: 6E. A4 24, 00001284
	inc	r10d					; 2812 _ 41: FF. C2
	vpshufd xmm13, xmm12, 0 			; 2815 _ C4 41 79: 70. EC, 00
	add	r13, 64 				; 281B _ 49: 83. C5, 40
	vpxor	xmm11, xmm9, xmm13			; 281F _ C4 41 31: EF. DD
	vmovd	xmm9, dword [r12+1288H] 		; 2824 _ C4 41 79: 6E. 8C 24, 00001288
	vpshufd xmm10, xmm9, 0				; 282E _ C4 41 79: 70. D1, 00
	vpxor	xmm10, xmm5, xmm10			; 2834 _ C4 41 51: EF. D2
	vmovd	xmm5, dword [r12+128CH] 		; 2839 _ C4 C1 79: 6E. AC 24, 0000128C
	vpshufd xmm12, xmm5, 0				; 2843 _ C5 79: 70. E5, 00
	vpxor	xmm13, xmm8, xmm12			; 2848 _ C4 41 39: EF. EC
	vpunpckldq xmm8, xmm4, xmm11			; 284D _ C4 41 59: 62. C3
	vpunpckldq xmm9, xmm10, xmm13			; 2852 _ C4 41 29: 62. CD
	vpunpckhdq xmm5, xmm4, xmm11			; 2857 _ C4 C1 59: 6A. EB
	vpunpcklqdq xmm11, xmm8, xmm9			; 285C _ C4 41 39: 6C. D9
	vpxor	xmm3, xmm11, xmm3			; 2861 _ C5 A1: EF. DB
	vmovdqu oword [rbp], xmm3			; 2865 _ C5 FA: 7F. 5D, 00
	vpunpckhqdq xmm3, xmm8, xmm9			; 286A _ C4 C1 39: 6D. D9
	vpunpckhdq xmm4, xmm10, xmm13			; 286F _ C4 C1 29: 6A. E5
	vpxor	xmm3, xmm3, xmm0			; 2874 _ C5 E1: EF. D8
	vmovdqu oword [rbp+10H], xmm3			; 2878 _ C5 FA: 7F. 5D, 10
	vpunpcklqdq xmm3, xmm5, xmm4			; 287D _ C5 D1: 6C. DC
	vpxor	xmm2, xmm3, xmm2			; 2881 _ C5 E1: EF. D2
	vpsllq	xmm3, xmm1, 1				; 2885 _ C5 E1: 73. F1, 01
	vmovdqu oword [rbp+20H], xmm2			; 288A _ C5 FA: 7F. 55, 20
	vpunpckhqdq xmm2, xmm5, xmm4			; 288F _ C5 D1: 6D. D4
	vpxor	xmm2, xmm2, xmm1			; 2893 _ C5 E9: EF. D1
	vmovdqu oword [rbp+30H], xmm2			; 2897 _ C5 FA: 7F. 55, 30
	vpslldq xmm2, xmm1, 8				; 289C _ C5 E9: 73. F9, 08
	add	rbp, 64 				; 28A1 _ 48: 83. C5, 40
	vpsrldq xmm2, xmm2, 7				; 28A5 _ C5 E9: 73. DA, 07
	cmp	r10d, 8 				; 28AA _ 41: 83. FA, 08
	vpsrlq	xmm2, xmm2, 7				; 28AE _ C5 E9: 73. D2, 07
	vpor	xmm3, xmm3, xmm2			; 28B3 _ C5 E1: EB. DA
	vpsraw	xmm2, xmm1, 8				; 28B7 _ C5 E9: 71. E1, 08
	vpsrldq xmm2, xmm2, 15				; 28BC _ C5 E9: 73. DA, 0F
	vpand	xmm2, xmm2, xmm7			; 28C1 _ C5 E9: DB. D7
	vpxor	xmm3, xmm3, xmm2			; 28C5 _ C5 E1: EF. DA
	jl	?_006					; 28C9 _ 0F 8C, FFFFD7FB
	add	r14, -512				; 28CF _ 49: 81. C6, FFFFFE00
	jne	?_004					; 28D6 _ 0F 85, FFFFD7B9
	vmovups xmm6, oword [rsp+90H]			; 28DC _ C5 F8: 10. B4 24, 00000090
	vmovups xmm7, oword [rsp+80H]			; 28E5 _ C5 F8: 10. BC 24, 00000080
	vmovups xmm8, oword [rsp+70H]			; 28EE _ C5 78: 10. 44 24, 70
	vmovups xmm9, oword [rsp+60H]			; 28F4 _ C5 78: 10. 4C 24, 60
	vmovups xmm10, oword [rsp+50H]			; 28FA _ C5 78: 10. 54 24, 50
	vmovups xmm11, oword [rsp+40H]			; 2900 _ C5 78: 10. 5C 24, 40
	vmovups xmm12, oword [rsp+30H]			; 2906 _ C5 78: 10. 64 24, 30
	vmovups xmm13, oword [rsp+20H]			; 290C _ C5 78: 10. 6C 24, 20
	add	rsp, 192				; 2912 _ 48: 81. C4, 000000C0
	pop	rbp					; 2919 _ 5D
	pop	r15					; 291A _ 41: 5F
	pop	r14					; 291C _ 41: 5E
	pop	r13					; 291E _ 41: 5D
	pop	r12					; 2920 _ 41: 5C
	ret						; 2922 _ C3
; xts_serpent_avx_decrypt End of function

; Filling space: 0DH
; Filler type: Multi-byte NOP
;       db 0FH, 1FH, 44H, 00H, 00H, 0FH, 1FH, 84H
;       db 00H, 00H, 00H, 00H, 00H

ALIGN	16

xts_serpent_avx_available:; Function begin
	push	rbx					; 0000 _ 53
	sub	rsp, 48 				; 0001 _ 48: 83. EC, 30
	mov	dword [rsp+20H], 0			; 0005 _ C7. 44 24, 20, 00000000
	mov	eax, 1					; 000D _ B8, 00000001
	cpuid						; 0012 _ 0F A2
	and	ecx, 18000000H				; 0014 _ 81. E1, 18000000
	cmp	ecx, 402653184				; 001A _ 81. F9, 18000000
	jnz	?_008					; 0020 _ 75, 15
	xor	ecx, ecx				; 0022 _ 33. C9
?_007:
; Error: Illegal opcode
; Error: Internal error in opcode table in opcodes.cpp
;       UNDEFINED                                       ; 0024 _ 0F 01. D0
	db 0FH, 01H, 0D0H
;       and     eax, 06H                                ; 0027 _ 83. E0, 06
	db 83H, 0E0H, 06H
;       cmp     eax, 6                                  ; 002A _ 83. F8, 06
	db 83H, 0F8H, 06H
;       jnz     ?_008                                   ; 002D _ 75, 08
	db 75H, 08H
;       mov     dword [rsp+20H], 1                      ; 002F _ C7. 44 24, 20, 00000001
	db 0C7H, 44H, 24H, 20H, 01H, 00H, 00H, 00H
?_008:	mov	eax, dword [rsp+20H]			; 0037 _ 8B. 44 24, 20
	add	rsp, 48 				; 003B _ 48: 83. C4, 30
	pop	rbx					; 003F _ 5B
	ret						; 0040 _ C3
; xts_serpent_avx_available End of function




