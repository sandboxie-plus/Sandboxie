; this code compiled with Intel® C++ Compiler Version 12.0.0.063
;
; Disassembly of file: xts_serpent_sse2.obj
; Thu May 19 19:04:44 2011
; Mode: 32 bits
; Syntax: YASM/NASM
; Instruction set: AVX


global _xts_serpent_avx_encrypt@24
global _xts_serpent_avx_decrypt@24
global _xts_serpent_avx_available@0

extern _serpent256_encrypt@12				; near

SECTION .text	align=16 execute			; section number 2, code
;  Communal section not supported by YASM

_xts_serpent_avx_encrypt@24:; Function begin
	push	ebp					; 0000 _ 55
	mov	ebp, esp				; 0001 _ 8B. EC
	and	esp, 0FFFFFFF0H 			; 0003 _ 83. E4, F0
	push	esi					; 0006 _ 56
	push	edi					; 0007 _ 57
	push	ebx					; 0008 _ 53
	sub	esp, 244				; 0009 _ 81. EC, 000000F4
	mov	esi, dword [ebp+18H]			; 000F _ 8B. 75, 18
	mov	ecx, esi				; 0012 _ 8B. CE
	mov	edi, dword [ebp+14H]			; 0014 _ 8B. 7D, 14
	shl	ecx, 23 				; 0017 _ C1. E1, 17
	shr	edi, 9					; 001A _ C1. EF, 09
	or	ecx, edi				; 001D _ 0B. CF
	mov	edi, 135				; 001F _ BF, 00000087
	mov	ebx, dword [ebp+1CH]			; 0024 _ 8B. 5D, 1C
	mov	edx, dword [ebp+8H]			; 0027 _ 8B. 55, 08
	mov	eax, dword [ebp+0CH]			; 002A _ 8B. 45, 0C
	vmovd	xmm1, edi				; 002D _ C5 F9: 6E. CF
	shr	esi, 9					; 0031 _ C1. EE, 09
	lea	ebx, [ebx+5710H]			; 0034 _ 8D. 9B, 00005710
	mov	dword [esp], ecx			; 003A _ 89. 0C 24
	xor	ecx, ecx				; 003D _ 33. C9
	mov	dword [esp+4H], esi			; 003F _ 89. 74 24, 04
	lea	edi, [esp+10H]				; 0043 _ 8D. 7C 24, 10
	mov	dword [esp+8H], ecx			; 0047 _ 89. 4C 24, 08
	mov	dword [esp+0CH], ecx			; 004B _ 89. 4C 24, 0C
	mov	esi, dword [ebp+10H]			; 004F _ 8B. 75, 10
	vmovdqu oword [esp+30H], xmm1			; 0052 _ C5 FA: 7F. 4C 24, 30
	mov	dword [esp+24H], eax			; 0058 _ 89. 44 24, 24
	mov	dword [esp+28H], edx			; 005C _ 89. 54 24, 28
	jmp	?_002					; 0060 _ EB, 06

?_001:	vmovdqu oword [esp+10H], xmm0			; 0062 _ C5 FA: 7F. 44 24, 10
?_002:	add	dword [esp], 1				; 0068 _ 83. 04 24, 01
	adc	dword [esp+4H], 0			; 006C _ 83. 54 24, 04, 00
	push	ebx					; 0071 _ 53
	push	edi					; 0072 _ 57
	lea	eax, [esp+8H]				; 0073 _ 8D. 44 24, 08
	push	eax					; 0077 _ 50
	call	_serpent256_encrypt@12			; 0078 _ E8, 00000000(rel)
	vmovdqu xmm0, oword [esp+10H]			; 007D _ C5 FA: 6F. 44 24, 10
	xor	eax, eax				; 0083 _ 33. C0
	mov	dword [esp+20H], esi			; 0085 _ 89. 74 24, 20
	vmovdqu oword [esp+70H], xmm0			; 0089 _ C5 FA: 7F. 44 24, 70
	mov	edx, dword [esp+24H]			; 008F _ 8B. 54 24, 24
	mov	esi, dword [esp+28H]			; 0093 _ 8B. 74 24, 28
	mov	ecx, dword [ebp+1CH]			; 0097 _ 8B. 4D, 1C
?_003:	vmovdqu xmm3, oword [esp+70H]			; 009A _ C5 FA: 6F. 5C 24, 70
	vpslldq xmm2, xmm3, 8				; 00A0 _ C5 E9: 73. FB, 08
	vpsllq	xmm0, xmm3, 1				; 00A5 _ C5 F9: 73. F3, 01
	vpsrldq xmm6, xmm2, 7				; 00AA _ C5 C9: 73. DA, 07
	vpsraw	xmm7, xmm3, 8				; 00AF _ C5 C1: 71. E3, 08
	vpsrlq	xmm1, xmm6, 7				; 00B4 _ C5 F1: 73. D6, 07
	vpsrldq xmm4, xmm7, 15				; 00B9 _ C5 D9: 73. DF, 0F
	vpor	xmm5, xmm0, xmm1			; 00BE _ C5 F9: EB. E9
	vmovdqu xmm0, oword [esp+30H]			; 00C2 _ C5 FA: 6F. 44 24, 30
	vpand	xmm2, xmm4, xmm0			; 00C8 _ C5 D9: DB. D0
	vpxor	xmm4, xmm5, xmm2			; 00CC _ C5 D1: EF. E2
	vpslldq xmm6, xmm4, 8				; 00D0 _ C5 C9: 73. FC, 08
	vpsraw	xmm2, xmm4, 8				; 00D5 _ C5 E9: 71. E4, 08
	vpsrldq xmm1, xmm6, 7				; 00DA _ C5 F1: 73. DE, 07
	vpsllq	xmm7, xmm4, 1				; 00DF _ C5 C1: 73. F4, 01
	vpsrldq xmm6, xmm2, 15				; 00E4 _ C5 C9: 73. DA, 0F
	vpsrlq	xmm5, xmm1, 7				; 00E9 _ C5 D1: 73. D1, 07
	vpor	xmm1, xmm7, xmm5			; 00EE _ C5 C1: EB. CD
	vpand	xmm7, xmm6, xmm0			; 00F2 _ C5 C9: DB. F8
	vpxor	xmm1, xmm1, xmm7			; 00F6 _ C5 F1: EF. CF
	vpslldq xmm5, xmm1, 8				; 00FA _ C5 D1: 73. F9, 08
	vpsllq	xmm6, xmm1, 1				; 00FF _ C5 C9: 73. F1, 01
	vpsrldq xmm2, xmm5, 7				; 0104 _ C5 E9: 73. DD, 07
	vpsraw	xmm5, xmm1, 8				; 0109 _ C5 D1: 71. E1, 08
	vpsrlq	xmm7, xmm2, 7				; 010E _ C5 C1: 73. D2, 07
	vpsrldq xmm2, xmm5, 15				; 0113 _ C5 E9: 73. DD, 0F
	vpor	xmm6, xmm6, xmm7			; 0118 _ C5 C9: EB. F7
	vpand	xmm0, xmm2, xmm0			; 011C _ C5 E9: DB. C0
	vpxor	xmm7, xmm6, xmm0			; 0120 _ C5 C9: EF. F8
	vpxor	xmm0, xmm3, oword [esi] 		; 0124 _ C5 E1: EF. 06
	vpxor	xmm3, xmm4, oword [esi+10H]		; 0128 _ C5 D9: EF. 5E, 10
	vmovdqu oword [esp+50H], xmm4			; 012D _ C5 FA: 7F. 64 24, 50
	vmovdqu oword [esp+40H], xmm1			; 0133 _ C5 FA: 7F. 4C 24, 40
	vpxor	xmm1, xmm1, oword [esi+20H]		; 0139 _ C5 F1: EF. 4E, 20
	vpxor	xmm4, xmm7, oword [esi+30H]		; 013E _ C5 C1: EF. 66, 30
	vmovdqu oword [esp+60H], xmm7			; 0143 _ C5 FA: 7F. 7C 24, 60
	vpunpckldq xmm6, xmm0, xmm3			; 0149 _ C5 F9: 62. F3
	vpunpckldq xmm7, xmm1, xmm4			; 014D _ C5 F1: 62. FC
	vpunpckhdq xmm5, xmm0, xmm3			; 0151 _ C5 F9: 6A. EB
	vpunpcklqdq xmm3, xmm6, xmm7			; 0155 _ C5 C9: 6C. DF
	vmovd	xmm0, dword [ecx+2A80H] 		; 0159 _ C5 F9: 6E. 81, 00002A80
	vpunpckhqdq xmm7, xmm6, xmm7			; 0161 _ C5 C9: 6D. FF
	vmovd	xmm6, dword [ecx+2A84H] 		; 0165 _ C5 F9: 6E. B1, 00002A84
	vpunpckhdq xmm1, xmm1, xmm4			; 016D _ C5 F1: 6A. CC
	vpshufd xmm2, xmm0, 0				; 0171 _ C5 F9: 70. D0, 00
	vpshufd xmm6, xmm6, 0				; 0176 _ C5 F9: 70. F6, 00
	vpxor	xmm0, xmm3, xmm2			; 017B _ C5 E1: EF. C2
	vpunpcklqdq xmm3, xmm5, xmm1			; 017F _ C5 D1: 6C. D9
	vpxor	xmm4, xmm7, xmm6			; 0183 _ C5 C1: EF. E6
	vmovd	xmm7, dword [ecx+2A88H] 		; 0187 _ C5 F9: 6E. B9, 00002A88
	vpunpckhqdq xmm1, xmm5, xmm1			; 018F _ C5 D1: 6D. C9
	vmovd	xmm5, dword [ecx+2A8CH] 		; 0193 _ C5 F9: 6E. A9, 00002A8C
	vpshufd xmm2, xmm7, 0				; 019B _ C5 F9: 70. D7, 00
	vpshufd xmm7, xmm5, 0				; 01A0 _ C5 F9: 70. FD, 00
	vpxor	xmm6, xmm3, xmm2			; 01A5 _ C5 E1: EF. F2
	vpxor	xmm5, xmm1, xmm7			; 01A9 _ C5 F1: EF. EF
	vpxor	xmm7, xmm4, xmm6			; 01AD _ C5 D9: EF. FE
	vpxor	xmm5, xmm5, xmm0			; 01B1 _ C5 D1: EF. E8
	vpand	xmm4, xmm4, xmm5			; 01B5 _ C5 D9: DB. E5
	vpxor	xmm1, xmm7, xmm5			; 01B9 _ C5 C1: EF. CD
	vpxor	xmm3, xmm4, xmm0			; 01BD _ C5 D9: EF. D8
	vpor	xmm0, xmm0, xmm5			; 01C1 _ C5 F9: EB. C5
	vpxor	xmm4, xmm0, xmm7			; 01C5 _ C5 F9: EF. E7
	vpxor	xmm5, xmm5, xmm6			; 01C9 _ C5 D1: EF. EE
	vpor	xmm6, xmm6, xmm3			; 01CD _ C5 C9: EB. F3
	vpcmpeqd xmm0, xmm0, xmm0			; 01D1 _ C5 F9: 76. C0
	vpxor	xmm7, xmm6, xmm1			; 01D5 _ C5 C9: EF. F9
	vpxor	xmm1, xmm1, xmm0			; 01D9 _ C5 F1: EF. C8
	vpor	xmm2, xmm1, xmm3			; 01DD _ C5 F1: EB. D3
	vpxor	xmm3, xmm3, xmm5			; 01E1 _ C5 E1: EF. DD
	vpor	xmm1, xmm5, xmm4			; 01E5 _ C5 D1: EB. CC
	vpxor	xmm6, xmm3, xmm2			; 01E9 _ C5 E1: EF. F2
	vpxor	xmm5, xmm6, xmm1			; 01ED _ C5 C9: EF. E9
	vpxor	xmm2, xmm2, xmm1			; 01F1 _ C5 E9: EF. D1
	vpslld	xmm3, xmm5, 13				; 01F5 _ C5 E1: 72. F5, 0D
	vpsrld	xmm6, xmm5, 19				; 01FA _ C5 C9: 72. D5, 13
	vpor	xmm5, xmm3, xmm6			; 01FF _ C5 E1: EB. EE
	vpslld	xmm3, xmm7, 3				; 0203 _ C5 E1: 72. F7, 03
	vpsrld	xmm7, xmm7, 29				; 0208 _ C5 C1: 72. D7, 1D
	vpxor	xmm1, xmm2, xmm5			; 020D _ C5 E9: EF. CD
	vpor	xmm6, xmm3, xmm7			; 0211 _ C5 E1: EB. F7
	vpxor	xmm2, xmm1, xmm6			; 0215 _ C5 F1: EF. D6
	vpxor	xmm4, xmm4, xmm6			; 0219 _ C5 D9: EF. E6
	vpslld	xmm1, xmm5, 3				; 021D _ C5 F1: 72. F5, 03
	vpslld	xmm7, xmm2, 1				; 0222 _ C5 C1: 72. F2, 01
	vpxor	xmm3, xmm4, xmm1			; 0227 _ C5 D9: EF. D9
	vpsrld	xmm4, xmm2, 31				; 022B _ C5 D9: 72. D2, 1F
	vpor	xmm4, xmm7, xmm4			; 0230 _ C5 C1: EB. E4
	vpslld	xmm2, xmm3, 7				; 0234 _ C5 E9: 72. F3, 07
	vpsrld	xmm3, xmm3, 25				; 0239 _ C5 E1: 72. D3, 19
	vpxor	xmm5, xmm5, xmm4			; 023E _ C5 D1: EF. EC
	vpor	xmm1, xmm2, xmm3			; 0242 _ C5 E9: EB. CB
	vpslld	xmm7, xmm4, 7				; 0246 _ C5 C1: 72. F4, 07
	vpxor	xmm2, xmm5, xmm1			; 024B _ C5 D1: EF. D1
	vpxor	xmm6, xmm6, xmm1			; 024F _ C5 C9: EF. F1
	vmovd	xmm5, dword [ecx+2A94H] 		; 0253 _ C5 F9: 6E. A9, 00002A94
	vpxor	xmm3, xmm6, xmm7			; 025B _ C5 C9: EF. DF
	vpshufd xmm6, xmm5, 0				; 025F _ C5 F9: 70. F5, 00
	vmovd	xmm5, dword [ecx+2A9CH] 		; 0264 _ C5 F9: 6E. A9, 00002A9C
	vpxor	xmm6, xmm4, xmm6			; 026C _ C5 D9: EF. F6
	vpshufd xmm5, xmm5, 0				; 0270 _ C5 F9: 70. ED, 00
	vmovd	xmm7, dword [ecx+2A90H] 		; 0275 _ C5 F9: 6E. B9, 00002A90
	vpxor	xmm5, xmm1, xmm5			; 027D _ C5 F1: EF. ED
	vpslld	xmm1, xmm2, 5				; 0281 _ C5 F1: 72. F2, 05
	vpsrld	xmm2, xmm2, 27				; 0286 _ C5 E9: 72. D2, 1B
	vpshufd xmm7, xmm7, 0				; 028B _ C5 F9: 70. FF, 00
	vpor	xmm1, xmm1, xmm2			; 0290 _ C5 F1: EB. CA
	vmovd	xmm4, dword [ecx+2A98H] 		; 0294 _ C5 F9: 6E. A1, 00002A98
	vpxor	xmm1, xmm1, xmm7			; 029C _ C5 F1: EF. CF
	vpslld	xmm7, xmm3, 22				; 02A0 _ C5 C1: 72. F3, 16
	vpsrld	xmm3, xmm3, 10				; 02A5 _ C5 E1: 72. D3, 0A
	vpshufd xmm4, xmm4, 0				; 02AA _ C5 F9: 70. E4, 00
	vpor	xmm7, xmm7, xmm3			; 02AF _ C5 C1: EB. FB
	vpxor	xmm1, xmm1, xmm0			; 02B3 _ C5 F1: EF. C8
	vpxor	xmm7, xmm7, xmm4			; 02B7 _ C5 C1: EF. FC
	vpand	xmm2, xmm1, xmm6			; 02BB _ C5 F1: DB. D6
	vpxor	xmm4, xmm7, xmm0			; 02BF _ C5 C1: EF. E0
	vpxor	xmm4, xmm4, xmm2			; 02C3 _ C5 D9: EF. E2
	vpor	xmm3, xmm2, xmm5			; 02C7 _ C5 E9: EB. DD
	vpxor	xmm7, xmm5, xmm4			; 02CB _ C5 D1: EF. FC
	vpxor	xmm6, xmm6, xmm3			; 02CF _ C5 C9: EF. F3
	vpxor	xmm5, xmm3, xmm1			; 02D3 _ C5 E1: EF. E9
	vpor	xmm3, xmm1, xmm6			; 02D7 _ C5 F1: EB. DE
	vpor	xmm1, xmm4, xmm5			; 02DB _ C5 D9: EB. CD
	vpxor	xmm2, xmm6, xmm7			; 02DF _ C5 C9: EF. D7
	vpand	xmm1, xmm1, xmm3			; 02E3 _ C5 F1: DB. CB
	vpxor	xmm4, xmm5, xmm2			; 02E7 _ C5 D1: EF. E2
	vpslld	xmm6, xmm1, 13				; 02EB _ C5 C9: 72. F1, 0D
	vpsrld	xmm5, xmm1, 19				; 02F0 _ C5 D1: 72. D1, 13
	vpor	xmm5, xmm6, xmm5			; 02F5 _ C5 C9: EB. ED
	vpslld	xmm6, xmm7, 3				; 02F9 _ C5 C9: 72. F7, 03
	vpsrld	xmm7, xmm7, 29				; 02FE _ C5 C1: 72. D7, 1D
	vpand	xmm2, xmm2, xmm1			; 0303 _ C5 E9: DB. D1
	vpor	xmm6, xmm6, xmm7			; 0307 _ C5 C9: EB. F7
	vpand	xmm7, xmm4, xmm1			; 030B _ C5 D9: DB. F9
	vpxor	xmm3, xmm3, xmm7			; 030F _ C5 E1: EF. DF
	vpxor	xmm1, xmm2, xmm4			; 0313 _ C5 E9: EF. CC
	vpxor	xmm7, xmm3, xmm5			; 0317 _ C5 E1: EF. FD
	vpslld	xmm4, xmm5, 3				; 031B _ C5 D9: 72. F5, 03
	vpxor	xmm3, xmm7, xmm6			; 0320 _ C5 C1: EF. DE
	vpxor	xmm7, xmm1, xmm6			; 0324 _ C5 F1: EF. FE
	vpxor	xmm2, xmm7, xmm4			; 0328 _ C5 C1: EF. D4
	vpslld	xmm1, xmm3, 1				; 032C _ C5 F1: 72. F3, 01
	vpsrld	xmm7, xmm3, 31				; 0331 _ C5 C1: 72. D3, 1F
	vpslld	xmm4, xmm2, 7				; 0336 _ C5 D9: 72. F2, 07
	vpor	xmm3, xmm1, xmm7			; 033B _ C5 F1: EB. DF
	vpsrld	xmm2, xmm2, 25				; 033F _ C5 E9: 72. D2, 19
	vpor	xmm4, xmm4, xmm2			; 0344 _ C5 D9: EB. E2
	vpxor	xmm5, xmm5, xmm3			; 0348 _ C5 D1: EF. EB
	vpxor	xmm5, xmm5, xmm4			; 034C _ C5 D1: EF. EC
	vpxor	xmm6, xmm6, xmm4			; 0350 _ C5 C9: EF. F4
	vpslld	xmm1, xmm3, 7				; 0354 _ C5 F1: 72. F3, 07
	vpsrld	xmm7, xmm5, 27				; 0359 _ C5 C1: 72. D5, 1B
	vpxor	xmm1, xmm6, xmm1			; 035E _ C5 C9: EF. C9
	vpslld	xmm6, xmm5, 5				; 0362 _ C5 C9: 72. F5, 05
	vmovd	xmm5, dword [ecx+2AA0H] 		; 0367 _ C5 F9: 6E. A9, 00002AA0
	vpor	xmm2, xmm6, xmm7			; 036F _ C5 C9: EB. D7
	vmovd	xmm7, dword [ecx+2AA4H] 		; 0373 _ C5 F9: 6E. B9, 00002AA4
	vpshufd xmm6, xmm5, 0				; 037B _ C5 F9: 70. F5, 00
	vpshufd xmm5, xmm7, 0				; 0380 _ C5 F9: 70. EF, 00
	vpxor	xmm2, xmm2, xmm6			; 0385 _ C5 E9: EF. D6
	vmovd	xmm7, dword [ecx+2AA8H] 		; 0389 _ C5 F9: 6E. B9, 00002AA8
	vpxor	xmm6, xmm3, xmm5			; 0391 _ C5 E1: EF. F5
	vpslld	xmm3, xmm1, 22				; 0395 _ C5 E1: 72. F1, 16
	vpsrld	xmm1, xmm1, 10				; 039A _ C5 F1: 72. D1, 0A
	vpor	xmm5, xmm3, xmm1			; 039F _ C5 E1: EB. E9
	vmovd	xmm1, dword [ecx+2AACH] 		; 03A3 _ C5 F9: 6E. 89, 00002AAC
	vpshufd xmm3, xmm7, 0				; 03AB _ C5 F9: 70. DF, 00
	vpxor	xmm7, xmm5, xmm3			; 03B0 _ C5 D1: EF. FB
	vpshufd xmm5, xmm1, 0				; 03B4 _ C5 F9: 70. E9, 00
	vpxor	xmm1, xmm4, xmm5			; 03B9 _ C5 D9: EF. CD
	vpand	xmm4, xmm2, xmm7			; 03BD _ C5 E9: DB. E7
	vpxor	xmm5, xmm4, xmm1			; 03C1 _ C5 D9: EF. E9
	vpxor	xmm7, xmm7, xmm6			; 03C5 _ C5 C1: EF. FE
	vpxor	xmm3, xmm7, xmm5			; 03C9 _ C5 C1: EF. DD
	vpor	xmm1, xmm1, xmm2			; 03CD _ C5 F1: EB. CA
	vpxor	xmm6, xmm1, xmm6			; 03D1 _ C5 F1: EF. F6
	vpxor	xmm2, xmm2, xmm3			; 03D5 _ C5 E9: EF. D3
	vpor	xmm4, xmm6, xmm2			; 03D9 _ C5 C9: EB. E2
	vpxor	xmm7, xmm4, xmm5			; 03DD _ C5 D9: EF. FD
	vpand	xmm5, xmm5, xmm6			; 03E1 _ C5 D1: DB. EE
	vpxor	xmm1, xmm2, xmm5			; 03E5 _ C5 E9: EF. CD
	vpxor	xmm6, xmm6, xmm7			; 03E9 _ C5 C9: EF. F7
	vpxor	xmm4, xmm6, xmm1			; 03ED _ C5 C9: EF. E1
	vpslld	xmm2, xmm3, 13				; 03F1 _ C5 E9: 72. F3, 0D
	vpsrld	xmm6, xmm3, 19				; 03F6 _ C5 C9: 72. D3, 13
	vpslld	xmm3, xmm4, 3				; 03FB _ C5 E1: 72. F4, 03
	vpor	xmm6, xmm2, xmm6			; 0400 _ C5 E9: EB. F6
	vpsrld	xmm4, xmm4, 29				; 0404 _ C5 D9: 72. D4, 1D
	vpor	xmm4, xmm3, xmm4			; 0409 _ C5 E1: EB. E4
	vpxor	xmm7, xmm7, xmm6			; 040D _ C5 C1: EF. FE
	vpxor	xmm1, xmm1, xmm0			; 0411 _ C5 F1: EF. C8
	vpxor	xmm5, xmm7, xmm4			; 0415 _ C5 C1: EF. EC
	vpxor	xmm1, xmm1, xmm4			; 0419 _ C5 F1: EF. CC
	vpslld	xmm7, xmm6, 3				; 041D _ C5 C1: 72. F6, 03
	vpxor	xmm3, xmm1, xmm7			; 0422 _ C5 F1: EF. DF
	vpslld	xmm2, xmm5, 1				; 0426 _ C5 E9: 72. F5, 01
	vpsrld	xmm5, xmm5, 31				; 042B _ C5 D1: 72. D5, 1F
	vpslld	xmm1, xmm3, 7				; 0430 _ C5 F1: 72. F3, 07
	vpor	xmm5, xmm2, xmm5			; 0435 _ C5 E9: EB. ED
	vpsrld	xmm3, xmm3, 25				; 0439 _ C5 E1: 72. D3, 19
	vpor	xmm3, xmm1, xmm3			; 043E _ C5 F1: EB. DB
	vpxor	xmm6, xmm6, xmm5			; 0442 _ C5 C9: EF. F5
	vpxor	xmm7, xmm6, xmm3			; 0446 _ C5 C9: EF. FB
	vpxor	xmm4, xmm4, xmm3			; 044A _ C5 D9: EF. E3
	vpslld	xmm6, xmm5, 7				; 044E _ C5 C9: 72. F5, 07
	vpslld	xmm1, xmm7, 5				; 0453 _ C5 F1: 72. F7, 05
	vpxor	xmm2, xmm4, xmm6			; 0458 _ C5 D9: EF. D6
	vpsrld	xmm7, xmm7, 27				; 045C _ C5 C1: 72. D7, 1B
	vmovd	xmm4, dword [ecx+2AB0H] 		; 0461 _ C5 F9: 6E. A1, 00002AB0
	vpor	xmm6, xmm1, xmm7			; 0469 _ C5 F1: EB. F7
	vpshufd xmm1, xmm4, 0				; 046D _ C5 F9: 70. CC, 00
	vmovd	xmm4, dword [ecx+2AB4H] 		; 0472 _ C5 F9: 6E. A1, 00002AB4
	vpxor	xmm7, xmm6, xmm1			; 047A _ C5 C9: EF. F9
	vpshufd xmm6, xmm4, 0				; 047E _ C5 F9: 70. F4, 00
	vmovd	xmm1, dword [ecx+2AB8H] 		; 0483 _ C5 F9: 6E. 89, 00002AB8
	vpxor	xmm6, xmm5, xmm6			; 048B _ C5 D1: EF. F6
	vpslld	xmm5, xmm2, 22				; 048F _ C5 D1: 72. F2, 16
	vpsrld	xmm2, xmm2, 10				; 0494 _ C5 E9: 72. D2, 0A
	vpor	xmm4, xmm5, xmm2			; 0499 _ C5 D1: EB. E2
	vpshufd xmm5, xmm1, 0				; 049D _ C5 F9: 70. E9, 00
	vmovd	xmm1, dword [ecx+2ABCH] 		; 04A2 _ C5 F9: 6E. 89, 00002ABC
	vpxor	xmm2, xmm4, xmm5			; 04AA _ C5 D9: EF. D5
	vpshufd xmm4, xmm1, 0				; 04AE _ C5 F9: 70. E1, 00
	vpxor	xmm3, xmm3, xmm4			; 04B3 _ C5 E1: EF. DC
	vpand	xmm4, xmm6, xmm7			; 04B7 _ C5 C9: DB. E7
	vpor	xmm1, xmm7, xmm3			; 04BB _ C5 C1: EB. CB
	vpxor	xmm5, xmm3, xmm6			; 04BF _ C5 E1: EF. EE
	vpxor	xmm7, xmm7, xmm2			; 04C3 _ C5 C1: EF. FA
	vpxor	xmm6, xmm2, xmm5			; 04C7 _ C5 E9: EF. F5
	vpor	xmm7, xmm7, xmm4			; 04CB _ C5 C1: EB. FC
	vpand	xmm5, xmm5, xmm1			; 04CF _ C5 D1: DB. E9
	vpxor	xmm2, xmm5, xmm7			; 04D3 _ C5 D1: EF. D7
	vpxor	xmm3, xmm1, xmm4			; 04D7 _ C5 F1: EF. DC
	vpxor	xmm1, xmm4, xmm2			; 04DB _ C5 D9: EF. CA
	vpxor	xmm5, xmm3, xmm2			; 04DF _ C5 E1: EF. EA
	vpor	xmm4, xmm1, xmm3			; 04E3 _ C5 F1: EB. E3
	vpand	xmm3, xmm7, xmm3			; 04E7 _ C5 C1: DB. DB
	vpxor	xmm1, xmm4, xmm6			; 04EB _ C5 D9: EF. CE
	vpxor	xmm6, xmm3, xmm6			; 04EF _ C5 E1: EF. F6
	vpor	xmm4, xmm1, xmm2			; 04F3 _ C5 F1: EB. E2
	vpxor	xmm4, xmm5, xmm4			; 04F7 _ C5 D1: EF. E4
	vpslld	xmm5, xmm4, 13				; 04FB _ C5 D1: 72. F4, 0D
	vpsrld	xmm4, xmm4, 19				; 0500 _ C5 D9: 72. D4, 13
	vpor	xmm5, xmm5, xmm4			; 0505 _ C5 D1: EB. EC
	vpslld	xmm4, xmm2, 3				; 0509 _ C5 D9: 72. F2, 03
	vpsrld	xmm2, xmm2, 29				; 050E _ C5 E9: 72. D2, 1D
	vpxor	xmm1, xmm1, xmm5			; 0513 _ C5 F1: EF. CD
	vpor	xmm4, xmm4, xmm2			; 0517 _ C5 D9: EB. E2
	vpslld	xmm7, xmm5, 3				; 051B _ C5 C1: 72. F5, 03
	vpxor	xmm1, xmm1, xmm4			; 0520 _ C5 F1: EF. CC
	vpxor	xmm6, xmm6, xmm4			; 0524 _ C5 C9: EF. F4
	vpxor	xmm6, xmm6, xmm7			; 0528 _ C5 C9: EF. F7
	vpslld	xmm3, xmm1, 1				; 052C _ C5 E1: 72. F1, 01
	vpsrld	xmm1, xmm1, 31				; 0531 _ C5 F1: 72. D1, 1F
	vpslld	xmm2, xmm6, 7				; 0536 _ C5 E9: 72. F6, 07
	vpor	xmm3, xmm3, xmm1			; 053B _ C5 E1: EB. D9
	vpsrld	xmm6, xmm6, 25				; 053F _ C5 C9: 72. D6, 19
	vpor	xmm7, xmm2, xmm6			; 0544 _ C5 E9: EB. FE
	vpxor	xmm5, xmm5, xmm3			; 0548 _ C5 D1: EF. EB
	vpxor	xmm6, xmm5, xmm7			; 054C _ C5 D1: EF. F7
	vpxor	xmm4, xmm4, xmm7			; 0550 _ C5 D9: EF. E7
	vpslld	xmm1, xmm3, 7				; 0554 _ C5 F1: 72. F3, 07
	vmovd	xmm2, dword [ecx+2AC0H] 		; 0559 _ C5 F9: 6E. 91, 00002AC0
	vpxor	xmm5, xmm4, xmm1			; 0561 _ C5 D9: EF. E9
	vpslld	xmm4, xmm6, 5				; 0565 _ C5 D9: 72. F6, 05
	vpsrld	xmm6, xmm6, 27				; 056A _ C5 C9: 72. D6, 1B
	vpshufd xmm1, xmm2, 0				; 056F _ C5 F9: 70. CA, 00
	vpor	xmm6, xmm4, xmm6			; 0574 _ C5 D9: EB. F6
	vpxor	xmm6, xmm6, xmm1			; 0578 _ C5 C9: EF. F1
	vmovd	xmm1, dword [ecx+2ACCH] 		; 057C _ C5 F9: 6E. 89, 00002ACC
	vmovd	xmm2, dword [ecx+2AC4H] 		; 0584 _ C5 F9: 6E. 91, 00002AC4
	vpshufd xmm1, xmm1, 0				; 058C _ C5 F9: 70. C9, 00
	vpxor	xmm1, xmm7, xmm1			; 0591 _ C5 C1: EF. C9
	vpshufd xmm7, xmm2, 0				; 0595 _ C5 F9: 70. FA, 00
	vpxor	xmm3, xmm3, xmm7			; 059A _ C5 E1: EF. DF
	vpsrld	xmm7, xmm5, 10				; 059E _ C5 C1: 72. D5, 0A
	vmovd	xmm4, dword [ecx+2AC8H] 		; 05A3 _ C5 F9: 6E. A1, 00002AC8
	vpxor	xmm2, xmm3, xmm1			; 05AB _ C5 E1: EF. D1
	vpxor	xmm3, xmm1, xmm0			; 05AF _ C5 F1: EF. D8
	vpslld	xmm1, xmm5, 22				; 05B3 _ C5 F1: 72. F5, 16
	vpshufd xmm4, xmm4, 0				; 05B8 _ C5 F9: 70. E4, 00
	vpor	xmm5, xmm1, xmm7			; 05BD _ C5 F1: EB. EF
	vpxor	xmm1, xmm5, xmm4			; 05C1 _ C5 D1: EF. CC
	vpxor	xmm7, xmm3, xmm6			; 05C5 _ C5 E1: EF. FE
	vpxor	xmm4, xmm1, xmm3			; 05C9 _ C5 F1: EF. E3
	vpand	xmm5, xmm2, xmm7			; 05CD _ C5 E9: DB. EF
	vpxor	xmm2, xmm2, xmm7			; 05D1 _ C5 E9: EF. D7
	vpxor	xmm1, xmm5, xmm4			; 05D5 _ C5 D1: EF. CC
	vpxor	xmm3, xmm6, xmm2			; 05D9 _ C5 C9: EF. DA
	vpand	xmm6, xmm4, xmm2			; 05DD _ C5 D9: DB. F2
	vpand	xmm4, xmm3, xmm1			; 05E1 _ C5 E1: DB. E1
	vpxor	xmm5, xmm6, xmm3			; 05E5 _ C5 C9: EF. EB
	vpxor	xmm6, xmm7, xmm4			; 05E9 _ C5 C1: EF. F4
	vpor	xmm2, xmm2, xmm1			; 05ED _ C5 E9: EB. D1
	vpor	xmm7, xmm4, xmm6			; 05F1 _ C5 D9: EB. FE
	vpxor	xmm3, xmm7, xmm5			; 05F5 _ C5 C1: EF. DD
	vpxor	xmm7, xmm3, xmm0			; 05F9 _ C5 E1: EF. F8
	vpslld	xmm0, xmm1, 13				; 05FD _ C5 F9: 72. F1, 0D
	vpsrld	xmm3, xmm1, 19				; 0602 _ C5 E1: 72. D1, 13
	vpxor	xmm1, xmm2, xmm4			; 0607 _ C5 E9: EF. CC
	vpor	xmm3, xmm0, xmm3			; 060B _ C5 F9: EB. DB
	vpslld	xmm0, xmm7, 3				; 060F _ C5 F9: 72. F7, 03
	vpsrld	xmm7, xmm7, 29				; 0614 _ C5 C1: 72. D7, 1D
	vpslld	xmm2, xmm3, 3				; 0619 _ C5 E9: 72. F3, 03
	vpor	xmm0, xmm0, xmm7			; 061E _ C5 F9: EB. C7
	vpand	xmm7, xmm5, xmm6			; 0622 _ C5 D1: DB. FE
	vpxor	xmm4, xmm1, xmm7			; 0626 _ C5 F1: EF. E7
	vpxor	xmm6, xmm6, xmm0			; 062A _ C5 C9: EF. F0
	vpxor	xmm5, xmm4, xmm3			; 062E _ C5 D9: EF. EB
	vpxor	xmm4, xmm6, xmm2			; 0632 _ C5 C9: EF. E2
	vpxor	xmm1, xmm5, xmm0			; 0636 _ C5 D1: EF. C8
	vpslld	xmm5, xmm4, 7				; 063A _ C5 D1: 72. F4, 07
	vpslld	xmm6, xmm1, 1				; 063F _ C5 C9: 72. F1, 01
	vpsrld	xmm7, xmm1, 31				; 0644 _ C5 C1: 72. D1, 1F
	vpsrld	xmm2, xmm4, 25				; 0649 _ C5 E9: 72. D4, 19
	vpor	xmm7, xmm6, xmm7			; 064E _ C5 C9: EB. FF
	vpor	xmm1, xmm5, xmm2			; 0652 _ C5 D1: EB. CA
	vpslld	xmm6, xmm7, 7				; 0656 _ C5 C9: 72. F7, 07
	vpxor	xmm0, xmm0, xmm1			; 065B _ C5 F9: EF. C1
	vpxor	xmm3, xmm3, xmm7			; 065F _ C5 E1: EF. DF
	vpxor	xmm6, xmm0, xmm6			; 0663 _ C5 F9: EF. F6
	vpxor	xmm4, xmm3, xmm1			; 0667 _ C5 E1: EF. E1
	vmovd	xmm0, dword [ecx+2AD4H] 		; 066B _ C5 F9: 6E. 81, 00002AD4
	vpshufd xmm3, xmm0, 0				; 0673 _ C5 F9: 70. D8, 00
	vpxor	xmm0, xmm7, xmm3			; 0678 _ C5 C1: EF. C3
	vmovd	xmm7, dword [ecx+2ADCH] 		; 067C _ C5 F9: 6E. B9, 00002ADC
	vpshufd xmm2, xmm7, 0				; 0684 _ C5 F9: 70. D7, 00
	vmovd	xmm5, dword [ecx+2AD0H] 		; 0689 _ C5 F9: 6E. A9, 00002AD0
	vpxor	xmm2, xmm1, xmm2			; 0691 _ C5 F1: EF. D2
	vpslld	xmm1, xmm4, 5				; 0695 _ C5 F1: 72. F4, 05
	vpsrld	xmm4, xmm4, 27				; 069A _ C5 D9: 72. D4, 1B
	vpshufd xmm7, xmm5, 0				; 069F _ C5 F9: 70. FD, 00
	vpor	xmm1, xmm1, xmm4			; 06A4 _ C5 F1: EB. CC
	vpxor	xmm4, xmm1, xmm7			; 06A8 _ C5 F1: EF. E7
	vpxor	xmm5, xmm4, xmm0			; 06AC _ C5 D9: EF. E8
	vpxor	xmm0, xmm0, xmm2			; 06B0 _ C5 F9: EF. C2
	vpcmpeqd xmm4, xmm4, xmm4			; 06B4 _ C5 D9: 76. E4
	vpand	xmm1, xmm0, xmm5			; 06B8 _ C5 F9: DB. CD
	vmovd	xmm3, dword [ecx+2AD8H] 		; 06BC _ C5 F9: 6E. 99, 00002AD8
	vpxor	xmm7, xmm2, xmm4			; 06C4 _ C5 E9: EF. FC
	vpslld	xmm2, xmm6, 22				; 06C8 _ C5 E9: 72. F6, 16
	vpsrld	xmm6, xmm6, 10				; 06CD _ C5 C9: 72. D6, 0A
	vpshufd xmm3, xmm3, 0				; 06D2 _ C5 F9: 70. DB, 00
	vpor	xmm6, xmm2, xmm6			; 06D7 _ C5 E9: EB. F6
	vpxor	xmm6, xmm6, xmm3			; 06DB _ C5 C9: EF. F3
	vpxor	xmm3, xmm6, xmm7			; 06DF _ C5 C9: EF. DF
	vpxor	xmm1, xmm1, xmm3			; 06E3 _ C5 F1: EF. CB
	vpor	xmm3, xmm3, xmm0			; 06E7 _ C5 E1: EB. D8
	vpand	xmm2, xmm7, xmm1			; 06EB _ C5 C1: DB. D1
	vpxor	xmm0, xmm0, xmm7			; 06EF _ C5 F9: EF. C7
	vpxor	xmm2, xmm2, xmm5			; 06F3 _ C5 E9: EF. D5
	vpxor	xmm6, xmm0, xmm1			; 06F7 _ C5 F9: EF. F1
	vpxor	xmm7, xmm6, xmm3			; 06FB _ C5 C9: EF. FB
	vpand	xmm0, xmm5, xmm2			; 06FF _ C5 D1: DB. C2
	vpxor	xmm0, xmm0, xmm7			; 0703 _ C5 F9: EF. C7
	vpslld	xmm6, xmm1, 13				; 0707 _ C5 C9: 72. F1, 0D
	vpsrld	xmm1, xmm1, 19				; 070C _ C5 F1: 72. D1, 13
	vpxor	xmm5, xmm3, xmm5			; 0711 _ C5 E1: EF. ED
	vpor	xmm6, xmm6, xmm1			; 0715 _ C5 C9: EB. F1
	vpslld	xmm1, xmm0, 3				; 0719 _ C5 F1: 72. F0, 03
	vpsrld	xmm0, xmm0, 29				; 071E _ C5 F9: 72. D0, 1D
	vpor	xmm7, xmm7, xmm2			; 0723 _ C5 C1: EB. FA
	vpor	xmm0, xmm1, xmm0			; 0727 _ C5 F1: EB. C0
	vpxor	xmm1, xmm2, xmm6			; 072B _ C5 E9: EF. CE
	vpxor	xmm2, xmm5, xmm4			; 072F _ C5 D1: EF. D4
	vpxor	xmm1, xmm1, xmm0			; 0733 _ C5 F1: EF. C8
	vpxor	xmm3, xmm7, xmm2			; 0737 _ C5 C1: EF. DA
	vpslld	xmm5, xmm6, 3				; 073B _ C5 D1: 72. F6, 03
	vpxor	xmm4, xmm3, xmm0			; 0740 _ C5 E1: EF. E0
	vpslld	xmm7, xmm1, 1				; 0744 _ C5 C1: 72. F1, 01
	vpsrld	xmm1, xmm1, 31				; 0749 _ C5 F1: 72. D1, 1F
	vpxor	xmm2, xmm4, xmm5			; 074E _ C5 D9: EF. D5
	vpor	xmm1, xmm7, xmm1			; 0752 _ C5 C1: EB. C9
	vpslld	xmm7, xmm2, 7				; 0756 _ C5 C1: 72. F2, 07
	vpsrld	xmm2, xmm2, 25				; 075B _ C5 E9: 72. D2, 19
	vpor	xmm4, xmm7, xmm2			; 0760 _ C5 C1: EB. E2
	vpxor	xmm6, xmm6, xmm1			; 0764 _ C5 C9: EF. F1
	vpxor	xmm2, xmm6, xmm4			; 0768 _ C5 C9: EF. D4
	vpxor	xmm0, xmm0, xmm4			; 076C _ C5 F9: EF. C4
	vpslld	xmm3, xmm1, 7				; 0770 _ C5 E1: 72. F1, 07
	vmovd	xmm7, dword [ecx+2AE0H] 		; 0775 _ C5 F9: 6E. B9, 00002AE0
	vpxor	xmm5, xmm0, xmm3			; 077D _ C5 F9: EF. EB
	vpslld	xmm0, xmm2, 5				; 0781 _ C5 F9: 72. F2, 05
	vpsrld	xmm3, xmm2, 27				; 0786 _ C5 E1: 72. D2, 1B
	vpor	xmm6, xmm0, xmm3			; 078B _ C5 F9: EB. F3
	vmovd	xmm3, dword [ecx+2AECH] 		; 078F _ C5 F9: 6E. 99, 00002AEC
	vpshufd xmm0, xmm7, 0				; 0797 _ C5 F9: 70. C7, 00
	vpshufd xmm7, xmm3, 0				; 079C _ C5 F9: 70. FB, 00
	vpxor	xmm2, xmm6, xmm0			; 07A1 _ C5 C9: EF. D0
	vmovd	xmm0, dword [ecx+2AE8H] 		; 07A5 _ C5 F9: 6E. 81, 00002AE8
	vpslld	xmm3, xmm5, 22				; 07AD _ C5 E1: 72. F5, 16
	vpsrld	xmm5, xmm5, 10				; 07B2 _ C5 D1: 72. D5, 0A
	vpxor	xmm4, xmm4, xmm7			; 07B7 _ C5 D9: EF. E7
	vpshufd xmm0, xmm0, 0				; 07BB _ C5 F9: 70. C0, 00
	vpor	xmm5, xmm3, xmm5			; 07C0 _ C5 E1: EB. ED
	vpxor	xmm3, xmm5, xmm0			; 07C4 _ C5 D1: EF. D8
	vmovd	xmm6, dword [ecx+2AE4H] 		; 07C8 _ C5 F9: 6E. B1, 00002AE4
	vpcmpeqd xmm0, xmm0, xmm0			; 07D0 _ C5 F9: 76. C0
	vpshufd xmm6, xmm6, 0				; 07D4 _ C5 F9: 70. F6, 00
	vpxor	xmm5, xmm3, xmm0			; 07D9 _ C5 E1: EF. E8
	vpxor	xmm3, xmm2, xmm4			; 07DD _ C5 E9: EF. DC
	vpand	xmm2, xmm4, xmm2			; 07E1 _ C5 D9: DB. D2
	vpxor	xmm2, xmm2, xmm5			; 07E5 _ C5 E9: EF. D5
	vpxor	xmm1, xmm1, xmm6			; 07E9 _ C5 F1: EF. CE
	vpxor	xmm1, xmm1, xmm2			; 07ED _ C5 F1: EF. CA
	vpor	xmm7, xmm5, xmm4			; 07F1 _ C5 D1: EB. FC
	vpor	xmm6, xmm3, xmm1			; 07F5 _ C5 E1: EB. F1
	vpxor	xmm5, xmm7, xmm3			; 07F9 _ C5 C1: EF. EB
	vpxor	xmm7, xmm5, xmm1			; 07FD _ C5 D1: EF. F9
	vpor	xmm3, xmm6, xmm2			; 0801 _ C5 C9: EB. DA
	vpxor	xmm4, xmm4, xmm6			; 0805 _ C5 D9: EF. E6
	vpxor	xmm3, xmm3, xmm7			; 0809 _ C5 E1: EF. DF
	vpxor	xmm4, xmm4, xmm2			; 080D _ C5 D9: EF. E2
	vpslld	xmm6, xmm3, 13				; 0811 _ C5 C9: 72. F3, 0D
	vpxor	xmm5, xmm4, xmm3			; 0816 _ C5 D9: EF. EB
	vpsrld	xmm3, xmm3, 19				; 081A _ C5 E1: 72. D3, 13
	vpor	xmm4, xmm6, xmm3			; 081F _ C5 C9: EB. E3
	vpslld	xmm6, xmm5, 3				; 0823 _ C5 C9: 72. F5, 03
	vpsrld	xmm3, xmm5, 29				; 0828 _ C5 E1: 72. D5, 1D
	vpxor	xmm1, xmm1, xmm4			; 082D _ C5 F1: EF. CC
	vpor	xmm6, xmm6, xmm3			; 0831 _ C5 C9: EB. F3
	vpxor	xmm2, xmm2, xmm0			; 0835 _ C5 E9: EF. D0
	vpxor	xmm3, xmm1, xmm6			; 0839 _ C5 F1: EF. DE
	vpand	xmm1, xmm7, xmm5			; 083D _ C5 C1: DB. CD
	vpxor	xmm2, xmm2, xmm1			; 0841 _ C5 E9: EF. D1
	vpslld	xmm7, xmm4, 3				; 0845 _ C5 C1: 72. F4, 03
	vpxor	xmm5, xmm2, xmm6			; 084A _ C5 E9: EF. EE
	vpslld	xmm1, xmm3, 1				; 084E _ C5 F1: 72. F3, 01
	vpxor	xmm7, xmm5, xmm7			; 0853 _ C5 D1: EF. FF
	vpsrld	xmm3, xmm3, 31				; 0857 _ C5 E1: 72. D3, 1F
	vpor	xmm1, xmm1, xmm3			; 085C _ C5 F1: EB. CB
	vpslld	xmm2, xmm7, 7				; 0860 _ C5 E9: 72. F7, 07
	vpsrld	xmm5, xmm7, 25				; 0865 _ C5 D1: 72. D7, 19
	vpxor	xmm4, xmm4, xmm1			; 086A _ C5 D9: EF. E1
	vpor	xmm7, xmm2, xmm5			; 086E _ C5 E9: EB. FD
	vpslld	xmm2, xmm1, 7				; 0872 _ C5 E9: 72. F1, 07
	vpxor	xmm4, xmm4, xmm7			; 0877 _ C5 D9: EF. E7
	vpxor	xmm6, xmm6, xmm7			; 087B _ C5 C9: EF. F7
	vmovd	xmm3, dword [ecx+2AF0H] 		; 087F _ C5 F9: 6E. 99, 00002AF0
	vpslld	xmm5, xmm4, 5				; 0887 _ C5 D1: 72. F4, 05
	vpsrld	xmm4, xmm4, 27				; 088C _ C5 D9: 72. D4, 1B
	vpxor	xmm2, xmm6, xmm2			; 0891 _ C5 C9: EF. D2
	vpor	xmm6, xmm5, xmm4			; 0895 _ C5 D1: EB. F4
	vpshufd xmm4, xmm3, 0				; 0899 _ C5 F9: 70. E3, 00
	vmovd	xmm5, dword [ecx+2AF4H] 		; 089E _ C5 F9: 6E. A9, 00002AF4
	vpxor	xmm4, xmm6, xmm4			; 08A6 _ C5 C9: EF. E4
	vpslld	xmm6, xmm2, 22				; 08AA _ C5 C9: 72. F2, 16
	vpsrld	xmm2, xmm2, 10				; 08AF _ C5 E9: 72. D2, 0A
	vpshufd xmm3, xmm5, 0				; 08B4 _ C5 F9: 70. DD, 00
	vpor	xmm5, xmm6, xmm2			; 08B9 _ C5 C9: EB. EA
	vmovd	xmm2, dword [ecx+2AF8H] 		; 08BD _ C5 F9: 6E. 91, 00002AF8
	vpxor	xmm1, xmm1, xmm3			; 08C5 _ C5 F1: EF. CB
	vmovd	xmm6, dword [ecx+2AFCH] 		; 08C9 _ C5 F9: 6E. B1, 00002AFC
	vpshufd xmm3, xmm2, 0				; 08D1 _ C5 F9: 70. DA, 00
	vpshufd xmm2, xmm6, 0				; 08D6 _ C5 F9: 70. D6, 00
	vpxor	xmm5, xmm5, xmm3			; 08DB _ C5 D1: EF. EB
	vpxor	xmm3, xmm7, xmm2			; 08DF _ C5 C1: EF. DA
	vpor	xmm7, xmm1, xmm5			; 08E3 _ C5 F1: EB. FD
	vpxor	xmm6, xmm7, xmm3			; 08E7 _ C5 C1: EF. F3
	vpxor	xmm2, xmm1, xmm5			; 08EB _ C5 F1: EF. D5
	vpxor	xmm7, xmm5, xmm6			; 08EF _ C5 D1: EF. FE
	vpxor	xmm5, xmm2, xmm7			; 08F3 _ C5 E9: EF. EF
	vpor	xmm2, xmm3, xmm2			; 08F7 _ C5 E1: EB. D2
	vmovdqu oword [esp+90H], xmm6			; 08FB _ C5 FA: 7F. B4 24, 00000090
	vpor	xmm6, xmm6, xmm5			; 0904 _ C5 C9: EB. F5
	vmovdqu oword [esp+80H], xmm4			; 0908 _ C5 FA: 7F. A4 24, 00000080
	vpor	xmm1, xmm4, xmm5			; 0911 _ C5 D9: EB. CD
	vpxor	xmm4, xmm6, xmm4			; 0915 _ C5 C9: EF. E4
	vpxor	xmm1, xmm1, xmm7			; 0919 _ C5 F1: EF. CF
	vpxor	xmm6, xmm4, xmm5			; 091D _ C5 D9: EF. F5
	vpxor	xmm7, xmm7, xmm6			; 0921 _ C5 C1: EF. FE
	vpand	xmm4, xmm6, xmm1			; 0925 _ C5 C9: DB. E1
	vpxor	xmm7, xmm7, xmm0			; 0929 _ C5 C1: EF. F8
	vpxor	xmm4, xmm4, xmm5			; 092D _ C5 D9: EF. E5
	vpor	xmm6, xmm7, xmm1			; 0931 _ C5 C1: EB. F1
	vpxor	xmm5, xmm5, xmm6			; 0935 _ C5 D1: EF. EE
	vpand	xmm3, xmm2, oword [esp+80H]		; 0939 _ C5 E9: DB. 9C 24, 00000080
	vpslld	xmm7, xmm5, 13				; 0942 _ C5 C1: 72. F5, 0D
	vpsrld	xmm5, xmm5, 19				; 0947 _ C5 D1: 72. D5, 13
	vpxor	xmm2, xmm3, oword [esp+90H]		; 094C _ C5 E1: EF. 94 24, 00000090
	vpor	xmm6, xmm7, xmm5			; 0955 _ C5 C1: EB. F5
	vpslld	xmm5, xmm4, 3				; 0959 _ C5 D1: 72. F4, 03
	vpsrld	xmm4, xmm4, 29				; 095E _ C5 D9: 72. D4, 1D
	vpor	xmm7, xmm5, xmm4			; 0963 _ C5 D1: EB. FC
	vpxor	xmm4, xmm2, xmm6			; 0967 _ C5 E9: EF. E6
	vpxor	xmm3, xmm4, xmm7			; 096B _ C5 D9: EF. DF
	vpxor	xmm1, xmm1, xmm7			; 096F _ C5 F1: EF. CF
	vpslld	xmm2, xmm6, 3				; 0973 _ C5 E9: 72. F6, 03
	vpsrld	xmm4, xmm3, 31				; 0978 _ C5 D9: 72. D3, 1F
	vpxor	xmm2, xmm1, xmm2			; 097D _ C5 F1: EF. D2
	vpslld	xmm1, xmm3, 1				; 0981 _ C5 F1: 72. F3, 01
	vpor	xmm3, xmm1, xmm4			; 0986 _ C5 F1: EB. DC
	vpslld	xmm5, xmm2, 7				; 098A _ C5 D1: 72. F2, 07
	vpsrld	xmm1, xmm2, 25				; 098F _ C5 F1: 72. D2, 19
	vpxor	xmm6, xmm6, xmm3			; 0994 _ C5 C9: EF. F3
	vpor	xmm2, xmm5, xmm1			; 0998 _ C5 D1: EB. D1
	vpslld	xmm1, xmm3, 7				; 099C _ C5 F1: 72. F3, 07
	vpxor	xmm4, xmm6, xmm2			; 09A1 _ C5 C9: EF. E2
	vpxor	xmm7, xmm7, xmm2			; 09A5 _ C5 C1: EF. FA
	vpslld	xmm5, xmm4, 5				; 09A9 _ C5 D1: 72. F4, 05
	vpsrld	xmm4, xmm4, 27				; 09AE _ C5 D9: 72. D4, 1B
	vmovd	xmm6, dword [ecx+2B00H] 		; 09B3 _ C5 F9: 6E. B1, 00002B00
	vpxor	xmm7, xmm7, xmm1			; 09BB _ C5 C1: EF. F9
	vpor	xmm1, xmm5, xmm4			; 09BF _ C5 D1: EB. CC
	vmovd	xmm5, dword [ecx+2B04H] 		; 09C3 _ C5 F9: 6E. A9, 00002B04
	vpshufd xmm4, xmm6, 0				; 09CB _ C5 F9: 70. E6, 00
	vpshufd xmm6, xmm5, 0				; 09D0 _ C5 F9: 70. F5, 00
	vpxor	xmm4, xmm1, xmm4			; 09D5 _ C5 F1: EF. E4
	vpxor	xmm1, xmm3, xmm6			; 09D9 _ C5 E1: EF. CE
	vpslld	xmm3, xmm7, 22				; 09DD _ C5 E1: 72. F7, 16
	vpsrld	xmm5, xmm7, 10				; 09E2 _ C5 D1: 72. D7, 0A
	vpor	xmm7, xmm3, xmm5			; 09E7 _ C5 E1: EB. FD
	vmovd	xmm3, dword [ecx+2B08H] 		; 09EB _ C5 F9: 6E. 99, 00002B08
	vmovd	xmm5, dword [ecx+2B0CH] 		; 09F3 _ C5 F9: 6E. A9, 00002B0C
	vpshufd xmm6, xmm3, 0				; 09FB _ C5 F9: 70. F3, 00
	vpshufd xmm3, xmm5, 0				; 0A00 _ C5 F9: 70. DD, 00
	vpxor	xmm7, xmm7, xmm6			; 0A05 _ C5 C1: EF. FE
	vpxor	xmm2, xmm2, xmm3			; 0A09 _ C5 E9: EF. D3
	vpxor	xmm5, xmm1, xmm7			; 0A0D _ C5 F1: EF. EF
	vpxor	xmm6, xmm2, xmm4			; 0A11 _ C5 E9: EF. F4
	vpand	xmm1, xmm1, xmm6			; 0A15 _ C5 F1: DB. CE
	vpxor	xmm2, xmm5, xmm6			; 0A19 _ C5 D1: EF. D6
	vpxor	xmm1, xmm1, xmm4			; 0A1D _ C5 F1: EF. CC
	vpor	xmm4, xmm4, xmm6			; 0A21 _ C5 D9: EB. E6
	vpxor	xmm4, xmm4, xmm5			; 0A25 _ C5 D9: EF. E5
	vpxor	xmm5, xmm6, xmm7			; 0A29 _ C5 C9: EF. EF
	vpor	xmm7, xmm7, xmm1			; 0A2D _ C5 C1: EB. F9
	vpxor	xmm7, xmm7, xmm2			; 0A31 _ C5 C1: EF. FA
	vpxor	xmm2, xmm2, xmm0			; 0A35 _ C5 E9: EF. D0
	vpor	xmm3, xmm2, xmm1			; 0A39 _ C5 E9: EB. D9
	vpxor	xmm1, xmm1, xmm5			; 0A3D _ C5 F1: EF. CD
	vpor	xmm2, xmm5, xmm4			; 0A41 _ C5 D1: EB. D4
	vpxor	xmm5, xmm1, xmm3			; 0A45 _ C5 F1: EF. EB
	vpxor	xmm6, xmm5, xmm2			; 0A49 _ C5 D1: EF. F2
	vpxor	xmm3, xmm3, xmm2			; 0A4D _ C5 E1: EF. DA
	vpslld	xmm1, xmm6, 13				; 0A51 _ C5 F1: 72. F6, 0D
	vpsrld	xmm5, xmm6, 19				; 0A56 _ C5 D1: 72. D6, 13
	vpor	xmm6, xmm1, xmm5			; 0A5B _ C5 F1: EB. F5
	vpslld	xmm1, xmm7, 3				; 0A5F _ C5 F1: 72. F7, 03
	vpsrld	xmm7, xmm7, 29				; 0A64 _ C5 C1: 72. D7, 1D
	vpslld	xmm2, xmm6, 3				; 0A69 _ C5 E9: 72. F6, 03
	vpor	xmm7, xmm1, xmm7			; 0A6E _ C5 F1: EB. FF
	vpxor	xmm1, xmm3, xmm6			; 0A72 _ C5 E1: EF. CE
	vpxor	xmm4, xmm4, xmm7			; 0A76 _ C5 D9: EF. E7
	vpxor	xmm1, xmm1, xmm7			; 0A7A _ C5 F1: EF. CF
	vpxor	xmm4, xmm4, xmm2			; 0A7E _ C5 D9: EF. E2
	vpslld	xmm5, xmm1, 1				; 0A82 _ C5 D1: 72. F1, 01
	vpsrld	xmm3, xmm1, 31				; 0A87 _ C5 E1: 72. D1, 1F
	vpslld	xmm1, xmm4, 7				; 0A8C _ C5 F1: 72. F4, 07
	vpsrld	xmm4, xmm4, 25				; 0A91 _ C5 D9: 72. D4, 19
	vpor	xmm2, xmm5, xmm3			; 0A96 _ C5 D1: EB. D3
	vpor	xmm3, xmm1, xmm4			; 0A9A _ C5 F1: EB. DC
	vpslld	xmm1, xmm2, 7				; 0A9E _ C5 F1: 72. F2, 07
	vpxor	xmm7, xmm7, xmm3			; 0AA3 _ C5 C1: EF. FB
	vpxor	xmm6, xmm6, xmm2			; 0AA7 _ C5 C9: EF. F2
	vpxor	xmm4, xmm7, xmm1			; 0AAB _ C5 C1: EF. E1
	vpxor	xmm5, xmm6, xmm3			; 0AAF _ C5 C9: EF. EB
	vmovd	xmm7, dword [ecx+2B14H] 		; 0AB3 _ C5 F9: 6E. B9, 00002B14
	vpshufd xmm6, xmm7, 0				; 0ABB _ C5 F9: 70. F7, 00
	vmovd	xmm7, dword [ecx+2B1CH] 		; 0AC0 _ C5 F9: 6E. B9, 00002B1C
	vpxor	xmm2, xmm2, xmm6			; 0AC8 _ C5 E9: EF. D6
	vpshufd xmm7, xmm7, 0				; 0ACC _ C5 F9: 70. FF, 00
	vmovd	xmm1, dword [ecx+2B10H] 		; 0AD1 _ C5 F9: 6E. 89, 00002B10
	vpxor	xmm7, xmm3, xmm7			; 0AD9 _ C5 E1: EF. FF
	vpslld	xmm3, xmm5, 5				; 0ADD _ C5 E1: 72. F5, 05
	vpsrld	xmm5, xmm5, 27				; 0AE2 _ C5 D1: 72. D5, 1B
	vpshufd xmm1, xmm1, 0				; 0AE7 _ C5 F9: 70. C9, 00
	vpor	xmm3, xmm3, xmm5			; 0AEC _ C5 E1: EB. DD
	vmovd	xmm6, dword [ecx+2B18H] 		; 0AF0 _ C5 F9: 6E. B1, 00002B18
	vpxor	xmm5, xmm3, xmm1			; 0AF8 _ C5 E1: EF. E9
	vpslld	xmm3, xmm4, 22				; 0AFC _ C5 E1: 72. F4, 16
	vpsrld	xmm4, xmm4, 10				; 0B01 _ C5 D9: 72. D4, 0A
	vpor	xmm4, xmm3, xmm4			; 0B06 _ C5 E1: EB. E4
	vpxor	xmm5, xmm5, xmm0			; 0B0A _ C5 D1: EF. E8
	vpshufd xmm3, xmm6, 0				; 0B0E _ C5 F9: 70. DE, 00
	vpand	xmm1, xmm5, xmm2			; 0B13 _ C5 D1: DB. CA
	vpxor	xmm6, xmm4, xmm3			; 0B17 _ C5 D9: EF. F3
	vpor	xmm3, xmm1, xmm7			; 0B1B _ C5 F1: EB. DF
	vpxor	xmm4, xmm6, xmm0			; 0B1F _ C5 C9: EF. E0
	vpxor	xmm4, xmm4, xmm1			; 0B23 _ C5 D9: EF. E1
	vpxor	xmm1, xmm7, xmm4			; 0B27 _ C5 C1: EF. CC
	vpxor	xmm7, xmm2, xmm3			; 0B2B _ C5 E9: EF. FB
	vpxor	xmm2, xmm3, xmm5			; 0B2F _ C5 E1: EF. D5
	vpor	xmm3, xmm5, xmm7			; 0B33 _ C5 D1: EB. DF
	vpor	xmm4, xmm4, xmm2			; 0B37 _ C5 D9: EB. E2
	vpxor	xmm5, xmm7, xmm1			; 0B3B _ C5 C1: EF. E9
	vpand	xmm4, xmm4, xmm3			; 0B3F _ C5 D9: DB. E3
	vpxor	xmm2, xmm2, xmm5			; 0B43 _ C5 E9: EF. D5
	vpslld	xmm6, xmm4, 13				; 0B47 _ C5 C9: 72. F4, 0D
	vpsrld	xmm7, xmm4, 19				; 0B4C _ C5 C1: 72. D4, 13
	vpor	xmm7, xmm6, xmm7			; 0B51 _ C5 C9: EB. FF
	vpslld	xmm6, xmm1, 3				; 0B55 _ C5 C9: 72. F1, 03
	vpsrld	xmm1, xmm1, 29				; 0B5A _ C5 F1: 72. D1, 1D
	vpand	xmm5, xmm5, xmm4			; 0B5F _ C5 D1: DB. EC
	vpor	xmm6, xmm6, xmm1			; 0B63 _ C5 C9: EB. F1
	vpand	xmm1, xmm2, xmm4			; 0B67 _ C5 E9: DB. CC
	vpxor	xmm3, xmm3, xmm1			; 0B6B _ C5 E1: EF. D9
	vpxor	xmm2, xmm5, xmm2			; 0B6F _ C5 D1: EF. D2
	vpxor	xmm1, xmm3, xmm7			; 0B73 _ C5 E1: EF. CF
	vpxor	xmm2, xmm2, xmm6			; 0B77 _ C5 E9: EF. D6
	vpxor	xmm1, xmm1, xmm6			; 0B7B _ C5 F1: EF. CE
	vpslld	xmm4, xmm7, 3				; 0B7F _ C5 D9: 72. F7, 03
	vpxor	xmm5, xmm2, xmm4			; 0B84 _ C5 E9: EF. EC
	vpslld	xmm3, xmm1, 1				; 0B88 _ C5 E1: 72. F1, 01
	vpsrld	xmm1, xmm1, 31				; 0B8D _ C5 F1: 72. D1, 1F
	vpor	xmm3, xmm3, xmm1			; 0B92 _ C5 E1: EB. D9
	vpslld	xmm1, xmm5, 7				; 0B96 _ C5 F1: 72. F5, 07
	vpsrld	xmm5, xmm5, 25				; 0B9B _ C5 D1: 72. D5, 19
	vpxor	xmm7, xmm7, xmm3			; 0BA0 _ C5 C1: EF. FB
	vpor	xmm1, xmm1, xmm5			; 0BA4 _ C5 F1: EB. CD
	vpslld	xmm2, xmm3, 7				; 0BA8 _ C5 E9: 72. F3, 07
	vpxor	xmm7, xmm7, xmm1			; 0BAD _ C5 C1: EF. F9
	vpxor	xmm6, xmm6, xmm1			; 0BB1 _ C5 C9: EF. F1
	vpslld	xmm4, xmm7, 5				; 0BB5 _ C5 D9: 72. F7, 05
	vpsrld	xmm5, xmm7, 27				; 0BBA _ C5 D1: 72. D7, 1B
	vmovd	xmm7, dword [ecx+2B20H] 		; 0BBF _ C5 F9: 6E. B9, 00002B20
	vpxor	xmm6, xmm6, xmm2			; 0BC7 _ C5 C9: EF. F2
	vpor	xmm2, xmm4, xmm5			; 0BCB _ C5 D9: EB. D5
	vmovd	xmm5, dword [ecx+2B24H] 		; 0BCF _ C5 F9: 6E. A9, 00002B24
	vpshufd xmm4, xmm7, 0				; 0BD7 _ C5 F9: 70. E7, 00
	vpxor	xmm7, xmm2, xmm4			; 0BDC _ C5 E9: EF. FC
	vpshufd xmm2, xmm5, 0				; 0BE0 _ C5 F9: 70. D5, 00
	vpxor	xmm4, xmm3, xmm2			; 0BE5 _ C5 E1: EF. E2
	vpslld	xmm3, xmm6, 22				; 0BE9 _ C5 E1: 72. F6, 16
	vmovd	xmm2, dword [ecx+2B28H] 		; 0BEE _ C5 F9: 6E. 91, 00002B28
	vpsrld	xmm6, xmm6, 10				; 0BF6 _ C5 C9: 72. D6, 0A
	vpor	xmm5, xmm3, xmm6			; 0BFB _ C5 E1: EB. EE
	vmovd	xmm6, dword [ecx+2B2CH] 		; 0BFF _ C5 F9: 6E. B1, 00002B2C
	vpshufd xmm3, xmm2, 0				; 0C07 _ C5 F9: 70. DA, 00
	vpxor	xmm2, xmm5, xmm3			; 0C0C _ C5 D1: EF. D3
	vpshufd xmm5, xmm6, 0				; 0C10 _ C5 F9: 70. EE, 00
	vpxor	xmm3, xmm1, xmm5			; 0C15 _ C5 F1: EF. DD
	vpand	xmm1, xmm7, xmm2			; 0C19 _ C5 C1: DB. CA
	vpxor	xmm6, xmm1, xmm3			; 0C1D _ C5 F1: EF. F3
	vpxor	xmm1, xmm2, xmm4			; 0C21 _ C5 E9: EF. CC
	vpxor	xmm5, xmm1, xmm6			; 0C25 _ C5 F1: EF. EE
	vpor	xmm2, xmm3, xmm7			; 0C29 _ C5 E1: EB. D7
	vpxor	xmm1, xmm2, xmm4			; 0C2D _ C5 E9: EF. CC
	vpxor	xmm7, xmm7, xmm5			; 0C31 _ C5 C1: EF. FD
	vpor	xmm4, xmm1, xmm7			; 0C35 _ C5 F1: EB. E7
	vpand	xmm2, xmm6, xmm1			; 0C39 _ C5 C9: DB. D1
	vpxor	xmm3, xmm4, xmm6			; 0C3D _ C5 D9: EF. DE
	vpxor	xmm2, xmm7, xmm2			; 0C41 _ C5 C1: EF. D2
	vpxor	xmm6, xmm1, xmm3			; 0C45 _ C5 F1: EF. F3
	vpslld	xmm1, xmm5, 13				; 0C49 _ C5 F1: 72. F5, 0D
	vpxor	xmm4, xmm6, xmm2			; 0C4E _ C5 C9: EF. E2
	vpsrld	xmm5, xmm5, 19				; 0C52 _ C5 D1: 72. D5, 13
	vpor	xmm1, xmm1, xmm5			; 0C57 _ C5 F1: EB. CD
	vpslld	xmm7, xmm4, 3				; 0C5B _ C5 C1: 72. F4, 03
	vpsrld	xmm4, xmm4, 29				; 0C60 _ C5 D9: 72. D4, 1D
	vpxor	xmm3, xmm3, xmm1			; 0C65 _ C5 E1: EF. D9
	vpor	xmm4, xmm7, xmm4			; 0C69 _ C5 C1: EB. E4
	vpxor	xmm2, xmm2, xmm0			; 0C6D _ C5 E9: EF. D0
	vpxor	xmm5, xmm3, xmm4			; 0C71 _ C5 E1: EF. EC
	vpxor	xmm3, xmm2, xmm4			; 0C75 _ C5 E9: EF. DC
	vpslld	xmm7, xmm1, 3				; 0C79 _ C5 C1: 72. F1, 03
	vpslld	xmm6, xmm5, 1				; 0C7E _ C5 C9: 72. F5, 01
	vpxor	xmm7, xmm3, xmm7			; 0C83 _ C5 E1: EF. FF
	vpsrld	xmm5, xmm5, 31				; 0C87 _ C5 D1: 72. D5, 1F
	vpor	xmm6, xmm6, xmm5			; 0C8C _ C5 C9: EB. F5
	vpslld	xmm2, xmm7, 7				; 0C90 _ C5 E9: 72. F7, 07
	vpsrld	xmm5, xmm7, 25				; 0C95 _ C5 D1: 72. D7, 19
	vpxor	xmm1, xmm1, xmm6			; 0C9A _ C5 F1: EF. CE
	vpor	xmm7, xmm2, xmm5			; 0C9E _ C5 E9: EB. FD
	vpxor	xmm2, xmm1, xmm7			; 0CA2 _ C5 F1: EF. D7
	vpxor	xmm4, xmm4, xmm7			; 0CA6 _ C5 D9: EF. E7
	vmovd	xmm3, dword [ecx+2B30H] 		; 0CAA _ C5 F9: 6E. 99, 00002B30
	vpslld	xmm1, xmm6, 7				; 0CB2 _ C5 F1: 72. F6, 07
	vpslld	xmm5, xmm2, 5				; 0CB7 _ C5 D1: 72. F2, 05
	vpsrld	xmm2, xmm2, 27				; 0CBC _ C5 E9: 72. D2, 1B
	vpxor	xmm4, xmm4, xmm1			; 0CC1 _ C5 D9: EF. E1
	vpor	xmm1, xmm5, xmm2			; 0CC5 _ C5 D1: EB. CA
	vmovd	xmm5, dword [ecx+2B34H] 		; 0CC9 _ C5 F9: 6E. A9, 00002B34
	vpshufd xmm2, xmm3, 0				; 0CD1 _ C5 F9: 70. D3, 00
	vpshufd xmm3, xmm5, 0				; 0CD6 _ C5 F9: 70. DD, 00
	vpxor	xmm1, xmm1, xmm2			; 0CDB _ C5 F1: EF. CA
	vpslld	xmm2, xmm4, 22				; 0CDF _ C5 E9: 72. F4, 16
	vpsrld	xmm4, xmm4, 10				; 0CE4 _ C5 D9: 72. D4, 0A
	vpxor	xmm6, xmm6, xmm3			; 0CE9 _ C5 C9: EF. F3
	vpor	xmm4, xmm2, xmm4			; 0CED _ C5 E9: EB. E4
	vmovd	xmm2, dword [ecx+2B38H] 		; 0CF1 _ C5 F9: 6E. 91, 00002B38
	vmovd	xmm3, dword [ecx+2B3CH] 		; 0CF9 _ C5 F9: 6E. 99, 00002B3C
	vpshufd xmm5, xmm2, 0				; 0D01 _ C5 F9: 70. EA, 00
	vpshufd xmm2, xmm3, 0				; 0D06 _ C5 F9: 70. D3, 00
	vpxor	xmm5, xmm4, xmm5			; 0D0B _ C5 D9: EF. ED
	vpxor	xmm7, xmm7, xmm2			; 0D0F _ C5 C1: EF. FA
	vpand	xmm2, xmm6, xmm1			; 0D13 _ C5 C9: DB. D1
	vpor	xmm4, xmm1, xmm7			; 0D17 _ C5 F1: EB. E7
	vpxor	xmm3, xmm7, xmm6			; 0D1B _ C5 C1: EF. DE
	vpxor	xmm1, xmm1, xmm5			; 0D1F _ C5 F1: EF. CD
	vpxor	xmm6, xmm5, xmm3			; 0D23 _ C5 D1: EF. F3
	vpor	xmm7, xmm1, xmm2			; 0D27 _ C5 F1: EB. FA
	vpand	xmm1, xmm3, xmm4			; 0D2B _ C5 E1: DB. CC
	vpxor	xmm3, xmm1, xmm7			; 0D2F _ C5 F1: EF. DF
	vpxor	xmm5, xmm4, xmm2			; 0D33 _ C5 D9: EF. EA
	vpxor	xmm2, xmm2, xmm3			; 0D37 _ C5 E9: EF. D3
	vpxor	xmm1, xmm5, xmm3			; 0D3B _ C5 D1: EF. CB
	vpor	xmm4, xmm2, xmm5			; 0D3F _ C5 E9: EB. E5
	vpand	xmm5, xmm7, xmm5			; 0D43 _ C5 C1: DB. ED
	vpxor	xmm2, xmm4, xmm6			; 0D47 _ C5 D9: EF. D6
	vpxor	xmm6, xmm5, xmm6			; 0D4B _ C5 D1: EF. F6
	vpor	xmm4, xmm2, xmm3			; 0D4F _ C5 E9: EB. E3
	vpxor	xmm4, xmm1, xmm4			; 0D53 _ C5 F1: EF. E4
	vpslld	xmm1, xmm4, 13				; 0D57 _ C5 F1: 72. F4, 0D
	vpsrld	xmm4, xmm4, 19				; 0D5C _ C5 D9: 72. D4, 13
	vpor	xmm4, xmm1, xmm4			; 0D61 _ C5 F1: EB. E4
	vpslld	xmm1, xmm3, 3				; 0D65 _ C5 F1: 72. F3, 03
	vpsrld	xmm3, xmm3, 29				; 0D6A _ C5 E1: 72. D3, 1D
	vpxor	xmm2, xmm2, xmm4			; 0D6F _ C5 E9: EF. D4
	vpor	xmm1, xmm1, xmm3			; 0D73 _ C5 F1: EB. CB
	vpslld	xmm5, xmm4, 3				; 0D77 _ C5 D1: 72. F4, 03
	vpxor	xmm3, xmm2, xmm1			; 0D7C _ C5 E9: EF. D9
	vpxor	xmm2, xmm6, xmm1			; 0D80 _ C5 C9: EF. D1
	vpxor	xmm5, xmm2, xmm5			; 0D84 _ C5 E9: EF. ED
	vpslld	xmm7, xmm3, 1				; 0D88 _ C5 C1: 72. F3, 01
	vpsrld	xmm3, xmm3, 31				; 0D8D _ C5 E1: 72. D3, 1F
	vpslld	xmm6, xmm5, 7				; 0D92 _ C5 C9: 72. F5, 07
	vpor	xmm7, xmm7, xmm3			; 0D97 _ C5 C1: EB. FB
	vpsrld	xmm2, xmm5, 25				; 0D9B _ C5 E9: 72. D5, 19
	vpor	xmm6, xmm6, xmm2			; 0DA0 _ C5 C9: EB. F2
	vpxor	xmm4, xmm4, xmm7			; 0DA4 _ C5 D9: EF. E7
	vpxor	xmm2, xmm4, xmm6			; 0DA8 _ C5 D9: EF. D6
	vpxor	xmm1, xmm1, xmm6			; 0DAC _ C5 F1: EF. CE
	vpslld	xmm4, xmm7, 7				; 0DB0 _ C5 D9: 72. F7, 07
	vmovd	xmm3, dword [ecx+2B40H] 		; 0DB5 _ C5 F9: 6E. 99, 00002B40
	vpxor	xmm5, xmm1, xmm4			; 0DBD _ C5 F1: EF. EC
	vpslld	xmm1, xmm2, 5				; 0DC1 _ C5 F1: 72. F2, 05
	vpsrld	xmm2, xmm2, 27				; 0DC6 _ C5 E9: 72. D2, 1B
	vpor	xmm1, xmm1, xmm2			; 0DCB _ C5 F1: EB. CA
	vpshufd xmm2, xmm3, 0				; 0DCF _ C5 F9: 70. D3, 00
	vmovd	xmm3, dword [ecx+2B4CH] 		; 0DD4 _ C5 F9: 6E. 99, 00002B4C
	vpxor	xmm1, xmm1, xmm2			; 0DDC _ C5 F1: EF. CA
	vmovd	xmm2, dword [ecx+2B44H] 		; 0DE0 _ C5 F9: 6E. 91, 00002B44
	vpshufd xmm3, xmm3, 0				; 0DE8 _ C5 F9: 70. DB, 00
	vpxor	xmm3, xmm6, xmm3			; 0DED _ C5 C9: EF. DB
	vpshufd xmm6, xmm2, 0				; 0DF1 _ C5 F9: 70. F2, 00
	vpxor	xmm0, xmm3, xmm0			; 0DF6 _ C5 E1: EF. C0
	vpxor	xmm7, xmm7, xmm6			; 0DFA _ C5 C1: EF. FE
	vmovd	xmm4, dword [ecx+2B48H] 		; 0DFE _ C5 F9: 6E. A1, 00002B48
	vpxor	xmm2, xmm7, xmm3			; 0E06 _ C5 C1: EF. D3
	vpslld	xmm3, xmm5, 22				; 0E0A _ C5 E1: 72. F5, 16
	vpsrld	xmm5, xmm5, 10				; 0E0F _ C5 D1: 72. D5, 0A
	vpshufd xmm4, xmm4, 0				; 0E14 _ C5 F9: 70. E4, 00
	vpor	xmm3, xmm3, xmm5			; 0E19 _ C5 E1: EB. DD
	vpxor	xmm5, xmm3, xmm4			; 0E1D _ C5 E1: EF. EC
	vpxor	xmm4, xmm5, xmm0			; 0E21 _ C5 D1: EF. E0
	vpxor	xmm5, xmm0, xmm1			; 0E25 _ C5 F9: EF. E9
	vpand	xmm0, xmm2, xmm5			; 0E29 _ C5 E9: DB. C5
	vpxor	xmm3, xmm2, xmm5			; 0E2D _ C5 E9: EF. DD
	vpxor	xmm0, xmm0, xmm4			; 0E31 _ C5 F9: EF. C4
	vpxor	xmm7, xmm1, xmm3			; 0E35 _ C5 F1: EF. FB
	vpand	xmm4, xmm4, xmm3			; 0E39 _ C5 D9: DB. E3
	vpand	xmm6, xmm7, xmm0			; 0E3D _ C5 C1: DB. F0
	vpxor	xmm4, xmm4, xmm7			; 0E41 _ C5 D9: EF. E7
	vpxor	xmm2, xmm5, xmm6			; 0E45 _ C5 D1: EF. D6
	vpor	xmm1, xmm6, xmm2			; 0E49 _ C5 C9: EB. CA
	vpcmpeqd xmm7, xmm7, xmm7			; 0E4D _ C5 C1: 76. FF
	vpxor	xmm5, xmm1, xmm4			; 0E51 _ C5 F1: EF. EC
	vpslld	xmm1, xmm0, 13				; 0E55 _ C5 F1: 72. F0, 0D
	vpxor	xmm5, xmm5, xmm7			; 0E5A _ C5 D1: EF. EF
	vpsrld	xmm7, xmm0, 19				; 0E5E _ C5 C1: 72. D0, 13
	vpor	xmm0, xmm3, xmm0			; 0E63 _ C5 E1: EB. C0
	vpor	xmm7, xmm1, xmm7			; 0E67 _ C5 F1: EB. FF
	vpslld	xmm1, xmm5, 3				; 0E6B _ C5 F1: 72. F5, 03
	vpsrld	xmm5, xmm5, 29				; 0E70 _ C5 D1: 72. D5, 1D
	vpxor	xmm3, xmm0, xmm6			; 0E75 _ C5 F9: EF. DE
	vpand	xmm4, xmm4, xmm2			; 0E79 _ C5 D9: DB. E2
	vpor	xmm1, xmm1, xmm5			; 0E7D _ C5 F1: EB. CD
	vpxor	xmm0, xmm3, xmm4			; 0E81 _ C5 E1: EF. C4
	vpxor	xmm5, xmm0, xmm7			; 0E85 _ C5 F9: EF. EF
	vpxor	xmm2, xmm2, xmm1			; 0E89 _ C5 E9: EF. D1
	vpslld	xmm6, xmm7, 3				; 0E8D _ C5 C9: 72. F7, 03
	vpxor	xmm4, xmm5, xmm1			; 0E92 _ C5 D1: EF. E1
	vpxor	xmm2, xmm2, xmm6			; 0E96 _ C5 E9: EF. D6
	vpslld	xmm3, xmm4, 1				; 0E9A _ C5 E1: 72. F4, 01
	vpsrld	xmm0, xmm4, 31				; 0E9F _ C5 F9: 72. D4, 1F
	vpslld	xmm5, xmm2, 7				; 0EA4 _ C5 D1: 72. F2, 07
	vpsrld	xmm4, xmm2, 25				; 0EA9 _ C5 D9: 72. D2, 19
	vpor	xmm6, xmm3, xmm0			; 0EAE _ C5 E1: EB. F0
	vpor	xmm3, xmm5, xmm4			; 0EB2 _ C5 D1: EB. DC
	vpslld	xmm4, xmm6, 7				; 0EB6 _ C5 D9: 72. F6, 07
	vpxor	xmm1, xmm1, xmm3			; 0EBB _ C5 F1: EF. CB
	vpxor	xmm7, xmm7, xmm6			; 0EBF _ C5 C1: EF. FE
	vpxor	xmm5, xmm1, xmm4			; 0EC3 _ C5 F1: EF. EC
	vpxor	xmm0, xmm7, xmm3			; 0EC7 _ C5 C1: EF. C3
	vmovd	xmm1, dword [ecx+2B54H] 		; 0ECB _ C5 F9: 6E. 89, 00002B54
	vpshufd xmm7, xmm1, 0				; 0ED3 _ C5 F9: 70. F9, 00
	vpxor	xmm1, xmm6, xmm7			; 0ED8 _ C5 C9: EF. CF
	vmovd	xmm6, dword [ecx+2B5CH] 		; 0EDC _ C5 F9: 6E. B1, 00002B5C
	vpshufd xmm4, xmm6, 0				; 0EE4 _ C5 F9: 70. E6, 00
	vmovd	xmm2, dword [ecx+2B50H] 		; 0EE9 _ C5 F9: 6E. 91, 00002B50
	vpxor	xmm4, xmm3, xmm4			; 0EF1 _ C5 E1: EF. E4
	vpslld	xmm3, xmm0, 5				; 0EF5 _ C5 E1: 72. F0, 05
	vpsrld	xmm0, xmm0, 27				; 0EFA _ C5 F9: 72. D0, 1B
	vpshufd xmm2, xmm2, 0				; 0EFF _ C5 F9: 70. D2, 00
	vpor	xmm3, xmm3, xmm0			; 0F04 _ C5 E1: EB. D8
	vpxor	xmm6, xmm3, xmm2			; 0F08 _ C5 E1: EF. F2
	vpxor	xmm2, xmm1, xmm4			; 0F0C _ C5 F1: EF. D4
	vpxor	xmm6, xmm6, xmm1			; 0F10 _ C5 C9: EF. F1
	vpcmpeqd xmm1, xmm1, xmm1			; 0F14 _ C5 F1: 76. C9
	vmovd	xmm7, dword [ecx+2B58H] 		; 0F18 _ C5 F9: 6E. B9, 00002B58
	vpxor	xmm3, xmm4, xmm1			; 0F20 _ C5 D9: EF. D9
	vpslld	xmm4, xmm5, 22				; 0F24 _ C5 D9: 72. F5, 16
	vpsrld	xmm5, xmm5, 10				; 0F29 _ C5 D1: 72. D5, 0A
	vpshufd xmm0, xmm7, 0				; 0F2E _ C5 F9: 70. C7, 00
	vpor	xmm4, xmm4, xmm5			; 0F33 _ C5 D9: EB. E5
	vpxor	xmm5, xmm4, xmm0			; 0F37 _ C5 D9: EF. E8
	vpand	xmm7, xmm2, xmm6			; 0F3B _ C5 E9: DB. FE
	vpxor	xmm0, xmm5, xmm3			; 0F3F _ C5 D1: EF. C3
	vpxor	xmm4, xmm7, xmm0			; 0F43 _ C5 C1: EF. E0
	vpor	xmm7, xmm0, xmm2			; 0F47 _ C5 F9: EB. FA
	vpand	xmm5, xmm3, xmm4			; 0F4B _ C5 E1: DB. EC
	vpxor	xmm3, xmm2, xmm3			; 0F4F _ C5 E9: EF. DB
	vpxor	xmm0, xmm5, xmm6			; 0F53 _ C5 D1: EF. C6
	vpxor	xmm2, xmm3, xmm4			; 0F57 _ C5 E1: EF. D4
	vpxor	xmm2, xmm2, xmm7			; 0F5B _ C5 E9: EF. D7
	vpand	xmm3, xmm6, xmm0			; 0F5F _ C5 C9: DB. D8
	vpxor	xmm5, xmm3, xmm2			; 0F63 _ C5 E1: EF. EA
	vpslld	xmm3, xmm4, 13				; 0F67 _ C5 E1: 72. F4, 0D
	vpsrld	xmm4, xmm4, 19				; 0F6C _ C5 D9: 72. D4, 13
	vpxor	xmm7, xmm7, xmm6			; 0F71 _ C5 C1: EF. FE
	vpor	xmm4, xmm3, xmm4			; 0F75 _ C5 E1: EB. E4
	vpslld	xmm3, xmm5, 3				; 0F79 _ C5 E1: 72. F5, 03
	vpsrld	xmm5, xmm5, 29				; 0F7E _ C5 D1: 72. D5, 1D
	vpxor	xmm6, xmm7, xmm1			; 0F83 _ C5 C1: EF. F1
	vpor	xmm5, xmm3, xmm5			; 0F87 _ C5 E1: EB. ED
	vpxor	xmm3, xmm0, xmm4			; 0F8B _ C5 F9: EF. DC
	vpor	xmm0, xmm2, xmm0			; 0F8F _ C5 E9: EB. C0
	vpxor	xmm3, xmm3, xmm5			; 0F93 _ C5 E1: EF. DD
	vpxor	xmm0, xmm0, xmm6			; 0F97 _ C5 F9: EF. C6
	vpslld	xmm7, xmm4, 3				; 0F9B _ C5 C1: 72. F4, 03
	vpxor	xmm2, xmm0, xmm5			; 0FA0 _ C5 F9: EF. D5
	vpslld	xmm6, xmm3, 1				; 0FA4 _ C5 C9: 72. F3, 01
	vpxor	xmm7, xmm2, xmm7			; 0FA9 _ C5 E9: EF. FF
	vpsrld	xmm3, xmm3, 31				; 0FAD _ C5 E1: 72. D3, 1F
	vpor	xmm0, xmm6, xmm3			; 0FB2 _ C5 C9: EB. C3
	vpslld	xmm3, xmm7, 7				; 0FB6 _ C5 E1: 72. F7, 07
	vpsrld	xmm2, xmm7, 25				; 0FBB _ C5 E9: 72. D7, 19
	vpxor	xmm4, xmm4, xmm0			; 0FC0 _ C5 D9: EF. E0
	vpor	xmm2, xmm3, xmm2			; 0FC4 _ C5 E1: EB. D2
	vpslld	xmm3, xmm0, 7				; 0FC8 _ C5 E1: 72. F0, 07
	vmovd	xmm7, dword [ecx+2B60H] 		; 0FCD _ C5 F9: 6E. B9, 00002B60
	vpxor	xmm4, xmm4, xmm2			; 0FD5 _ C5 D9: EF. E2
	vpxor	xmm5, xmm5, xmm2			; 0FD9 _ C5 D1: EF. EA
	vpxor	xmm6, xmm5, xmm3			; 0FDD _ C5 D1: EF. F3
	vpslld	xmm5, xmm4, 5				; 0FE1 _ C5 D1: 72. F4, 05
	vpshufd xmm3, xmm7, 0				; 0FE6 _ C5 F9: 70. DF, 00
	vpsrld	xmm4, xmm4, 27				; 0FEB _ C5 D9: 72. D4, 1B
	vmovd	xmm7, dword [ecx+2B6CH] 		; 0FF0 _ C5 F9: 6E. B9, 00002B6C
	vpor	xmm4, xmm5, xmm4			; 0FF8 _ C5 D1: EB. E4
	vpshufd xmm7, xmm7, 0				; 0FFC _ C5 F9: 70. FF, 00
	vpxor	xmm3, xmm4, xmm3			; 1001 _ C5 D9: EF. DB
	vmovd	xmm4, dword [ecx+2B68H] 		; 1005 _ C5 F9: 6E. A1, 00002B68
	vpxor	xmm7, xmm2, xmm7			; 100D _ C5 E9: EF. FF
	vpslld	xmm2, xmm6, 22				; 1011 _ C5 E9: 72. F6, 16
	vpsrld	xmm6, xmm6, 10				; 1016 _ C5 C9: 72. D6, 0A
	vpshufd xmm4, xmm4, 0				; 101B _ C5 F9: 70. E4, 00
	vpor	xmm2, xmm2, xmm6			; 1020 _ C5 E9: EB. D6
	vmovd	xmm5, dword [ecx+2B64H] 		; 1024 _ C5 F9: 6E. A9, 00002B64
	vpxor	xmm2, xmm2, xmm4			; 102C _ C5 E9: EF. D4
	vpshufd xmm5, xmm5, 0				; 1030 _ C5 F9: 70. ED, 00
	vpxor	xmm4, xmm2, xmm1			; 1035 _ C5 E9: EF. E1
	vpxor	xmm6, xmm3, xmm7			; 1039 _ C5 E1: EF. F7
	vpand	xmm3, xmm7, xmm3			; 103D _ C5 C1: DB. DB
	vpxor	xmm3, xmm3, xmm4			; 1041 _ C5 E1: EF. DC
	vpxor	xmm0, xmm0, xmm5			; 1045 _ C5 F9: EF. C5
	vpxor	xmm2, xmm0, xmm3			; 1049 _ C5 F9: EF. D3
	vpor	xmm4, xmm4, xmm7			; 104D _ C5 D9: EB. E7
	vpor	xmm0, xmm6, xmm2			; 1051 _ C5 C9: EB. C2
	vpxor	xmm6, xmm4, xmm6			; 1055 _ C5 D9: EF. F6
	vpxor	xmm5, xmm6, xmm2			; 1059 _ C5 C9: EF. EA
	vpor	xmm4, xmm0, xmm3			; 105D _ C5 F9: EB. E3
	vpxor	xmm0, xmm7, xmm0			; 1061 _ C5 C1: EF. C0
	vpxor	xmm4, xmm4, xmm5			; 1065 _ C5 D9: EF. E5
	vpxor	xmm7, xmm0, xmm3			; 1069 _ C5 F9: EF. FB
	vpslld	xmm0, xmm4, 13				; 106D _ C5 F9: 72. F4, 0D
	vpxor	xmm7, xmm7, xmm4			; 1072 _ C5 C1: EF. FC
	vpsrld	xmm4, xmm4, 19				; 1076 _ C5 D9: 72. D4, 13
	vpor	xmm0, xmm0, xmm4			; 107B _ C5 F9: EB. C4
	vpslld	xmm6, xmm7, 3				; 107F _ C5 C9: 72. F7, 03
	vpsrld	xmm4, xmm7, 29				; 1084 _ C5 D9: 72. D7, 1D
	vpxor	xmm3, xmm3, xmm1			; 1089 _ C5 E1: EF. D9
	vpand	xmm5, xmm5, xmm7			; 108D _ C5 D1: DB. EF
	vpor	xmm4, xmm6, xmm4			; 1091 _ C5 C9: EB. E4
	vpxor	xmm2, xmm2, xmm0			; 1095 _ C5 E9: EF. D0
	vpxor	xmm7, xmm3, xmm5			; 1099 _ C5 E1: EF. FD
	vpxor	xmm6, xmm2, xmm4			; 109D _ C5 E9: EF. F4
	vpxor	xmm3, xmm7, xmm4			; 10A1 _ C5 C1: EF. DC
	vpslld	xmm2, xmm0, 3				; 10A5 _ C5 E9: 72. F0, 03
	vpslld	xmm7, xmm6, 1				; 10AA _ C5 C1: 72. F6, 01
	vpxor	xmm5, xmm3, xmm2			; 10AF _ C5 E1: EF. EA
	vpsrld	xmm6, xmm6, 31				; 10B3 _ C5 C9: 72. D6, 1F
	vpor	xmm2, xmm7, xmm6			; 10B8 _ C5 C1: EB. D6
	vpslld	xmm3, xmm5, 7				; 10BC _ C5 E1: 72. F5, 07
	vpsrld	xmm5, xmm5, 25				; 10C1 _ C5 D1: 72. D5, 19
	vpxor	xmm0, xmm0, xmm2			; 10C6 _ C5 F9: EF. C2
	vpor	xmm5, xmm3, xmm5			; 10CA _ C5 E1: EB. ED
	vpslld	xmm3, xmm2, 7				; 10CE _ C5 E1: 72. F2, 07
	vpxor	xmm0, xmm0, xmm5			; 10D3 _ C5 F9: EF. C5
	vpxor	xmm4, xmm4, xmm5			; 10D7 _ C5 D9: EF. E5
	vmovd	xmm7, dword [ecx+2B70H] 		; 10DB _ C5 F9: 6E. B9, 00002B70
	vpxor	xmm3, xmm4, xmm3			; 10E3 _ C5 D9: EF. DB
	vpslld	xmm4, xmm0, 5				; 10E7 _ C5 D9: 72. F0, 05
	vpsrld	xmm0, xmm0, 27				; 10EC _ C5 F9: 72. D0, 1B
	vpor	xmm6, xmm4, xmm0			; 10F1 _ C5 D9: EB. F0
	vpshufd xmm4, xmm7, 0				; 10F5 _ C5 F9: 70. E7, 00
	vpxor	xmm0, xmm6, xmm4			; 10FA _ C5 C9: EF. C4
	vpslld	xmm4, xmm3, 22				; 10FE _ C5 D9: 72. F3, 16
	vpsrld	xmm3, xmm3, 10				; 1103 _ C5 E1: 72. D3, 0A
	vmovd	xmm7, dword [ecx+2B74H] 		; 1108 _ C5 F9: 6E. B9, 00002B74
	vpor	xmm3, xmm4, xmm3			; 1110 _ C5 D9: EB. DB
	vmovd	xmm4, dword [ecx+2B78H] 		; 1114 _ C5 F9: 6E. A1, 00002B78
	vpshufd xmm6, xmm7, 0				; 111C _ C5 F9: 70. F7, 00
	vpshufd xmm7, xmm4, 0				; 1121 _ C5 F9: 70. FC, 00
	vpxor	xmm2, xmm2, xmm6			; 1126 _ C5 E9: EF. D6
	vmovd	xmm4, dword [ecx+2B7CH] 		; 112A _ C5 F9: 6E. A1, 00002B7C
	vpxor	xmm6, xmm3, xmm7			; 1132 _ C5 E1: EF. F7
	vpshufd xmm3, xmm4, 0				; 1136 _ C5 F9: 70. DC, 00
	vpxor	xmm3, xmm5, xmm3			; 113B _ C5 D1: EF. DB
	vpor	xmm5, xmm2, xmm6			; 113F _ C5 E9: EB. EE
	vpxor	xmm4, xmm5, xmm3			; 1143 _ C5 D1: EF. E3
	vpxor	xmm5, xmm2, xmm6			; 1147 _ C5 E9: EF. EE
	vpxor	xmm2, xmm6, xmm4			; 114B _ C5 C9: EF. D4
	vpor	xmm3, xmm3, xmm5			; 114F _ C5 E1: EB. DD
	vpxor	xmm7, xmm5, xmm2			; 1153 _ C5 D1: EF. FA
	vmovdqu oword [esp+0B0H], xmm4			; 1157 _ C5 FA: 7F. A4 24, 000000B0
	vpor	xmm4, xmm4, xmm7			; 1160 _ C5 D9: EB. E7
	vmovdqu oword [esp+0A0H], xmm0			; 1164 _ C5 FA: 7F. 84 24, 000000A0
	vpor	xmm6, xmm0, xmm7			; 116D _ C5 F9: EB. F7
	vpxor	xmm0, xmm4, xmm0			; 1171 _ C5 D9: EF. C0
	vpxor	xmm6, xmm6, xmm2			; 1175 _ C5 C9: EF. F2
	vpxor	xmm0, xmm0, xmm7			; 1179 _ C5 F9: EF. C7
	vpxor	xmm2, xmm2, xmm0			; 117D _ C5 E9: EF. D0
	vpand	xmm4, xmm0, xmm6			; 1181 _ C5 F9: DB. E6
	vpxor	xmm0, xmm2, xmm1			; 1185 _ C5 E9: EF. C1
	vpxor	xmm4, xmm4, xmm7			; 1189 _ C5 D9: EF. E7
	vpor	xmm2, xmm0, xmm6			; 118D _ C5 F9: EB. D6
	vpxor	xmm7, xmm7, xmm2			; 1191 _ C5 C1: EF. FA
	vpand	xmm5, xmm3, oword [esp+0A0H]		; 1195 _ C5 E1: DB. AC 24, 000000A0
	vpslld	xmm0, xmm7, 13				; 119E _ C5 F9: 72. F7, 0D
	vpsrld	xmm2, xmm7, 19				; 11A3 _ C5 E9: 72. D7, 13
	vpslld	xmm7, xmm4, 3				; 11A8 _ C5 C1: 72. F4, 03
	vpsrld	xmm4, xmm4, 29				; 11AD _ C5 D9: 72. D4, 1D
	vpor	xmm0, xmm0, xmm2			; 11B2 _ C5 F9: EB. C2
	vpor	xmm2, xmm7, xmm4			; 11B6 _ C5 C1: EB. D4
	vpxor	xmm4, xmm5, oword [esp+0B0H]		; 11BA _ C5 D1: EF. A4 24, 000000B0
	vpxor	xmm5, xmm6, xmm2			; 11C3 _ C5 C9: EF. EA
	vpxor	xmm3, xmm4, xmm0			; 11C7 _ C5 D9: EF. D8
	vpslld	xmm6, xmm0, 3				; 11CB _ C5 C9: 72. F0, 03
	vpxor	xmm7, xmm3, xmm2			; 11D0 _ C5 E1: EF. FA
	vpxor	xmm5, xmm5, xmm6			; 11D4 _ C5 D1: EF. EE
	vpslld	xmm4, xmm7, 1				; 11D8 _ C5 D9: 72. F7, 01
	vpsrld	xmm3, xmm7, 31				; 11DD _ C5 E1: 72. D7, 1F
	vpor	xmm6, xmm4, xmm3			; 11E2 _ C5 D9: EB. F3
	vpslld	xmm7, xmm5, 7				; 11E6 _ C5 C1: 72. F5, 07
	vpsrld	xmm4, xmm5, 25				; 11EB _ C5 D9: 72. D5, 19
	vpxor	xmm0, xmm0, xmm6			; 11F0 _ C5 F9: EF. C6
	vpor	xmm7, xmm7, xmm4			; 11F4 _ C5 C1: EB. FC
	vpslld	xmm4, xmm6, 7				; 11F8 _ C5 D9: 72. F6, 07
	vpxor	xmm0, xmm0, xmm7			; 11FD _ C5 F9: EF. C7
	vpxor	xmm2, xmm2, xmm7			; 1201 _ C5 E9: EF. D7
	vpxor	xmm4, xmm2, xmm4			; 1205 _ C5 E9: EF. E4
	vpslld	xmm3, xmm0, 5				; 1209 _ C5 E1: 72. F0, 05
	vmovd	xmm2, dword [ecx+2B80H] 		; 120E _ C5 F9: 6E. 91, 00002B80
	vpsrld	xmm0, xmm0, 27				; 1216 _ C5 F9: 72. D0, 1B
	vpor	xmm5, xmm3, xmm0			; 121B _ C5 E1: EB. E8
	vpshufd xmm3, xmm2, 0				; 121F _ C5 F9: 70. DA, 00
	vmovd	xmm2, dword [ecx+2B84H] 		; 1224 _ C5 F9: 6E. 91, 00002B84
	vpxor	xmm0, xmm5, xmm3			; 122C _ C5 D1: EF. C3
	vpshufd xmm5, xmm2, 0				; 1230 _ C5 F9: 70. EA, 00
	vpxor	xmm5, xmm6, xmm5			; 1235 _ C5 C9: EF. ED
	vpslld	xmm6, xmm4, 22				; 1239 _ C5 C9: 72. F4, 16
	vpsrld	xmm4, xmm4, 10				; 123E _ C5 D9: 72. D4, 0A
	vpor	xmm2, xmm6, xmm4			; 1243 _ C5 C9: EB. D4
	vmovd	xmm4, dword [ecx+2B8CH] 		; 1247 _ C5 F9: 6E. A1, 00002B8C
	vpshufd xmm4, xmm4, 0				; 124F _ C5 F9: 70. E4, 00
	vmovd	xmm3, dword [ecx+2B88H] 		; 1254 _ C5 F9: 6E. 99, 00002B88
	vpxor	xmm7, xmm7, xmm4			; 125C _ C5 C1: EF. FC
	vpshufd xmm6, xmm3, 0				; 1260 _ C5 F9: 70. F3, 00
	vpxor	xmm3, xmm7, xmm0			; 1265 _ C5 C1: EF. D8
	vpxor	xmm6, xmm2, xmm6			; 1269 _ C5 E9: EF. F6
	vpand	xmm4, xmm5, xmm3			; 126D _ C5 D1: DB. E3
	vpxor	xmm2, xmm5, xmm6			; 1271 _ C5 D1: EF. D6
	vpxor	xmm5, xmm4, xmm0			; 1275 _ C5 D9: EF. E8
	vpor	xmm0, xmm0, xmm3			; 1279 _ C5 F9: EB. C3
	vpxor	xmm7, xmm2, xmm3			; 127D _ C5 E9: EF. FB
	vpxor	xmm4, xmm0, xmm2			; 1281 _ C5 F9: EF. E2
	vpxor	xmm0, xmm3, xmm6			; 1285 _ C5 E1: EF. C6
	vpor	xmm3, xmm6, xmm5			; 1289 _ C5 C9: EB. DD
	vpor	xmm6, xmm0, xmm4			; 128D _ C5 F9: EB. F4
	vpxor	xmm2, xmm3, xmm7			; 1291 _ C5 E1: EF. D7
	vpxor	xmm7, xmm7, xmm1			; 1295 _ C5 C1: EF. F9
	vpor	xmm3, xmm7, xmm5			; 1299 _ C5 C1: EB. DD
	vpxor	xmm5, xmm5, xmm0			; 129D _ C5 D1: EF. E8
	vpxor	xmm0, xmm5, xmm3			; 12A1 _ C5 D1: EF. C3
	vpxor	xmm5, xmm0, xmm6			; 12A5 _ C5 F9: EF. EE
	vpxor	xmm6, xmm3, xmm6			; 12A9 _ C5 E1: EF. F6
	vpslld	xmm7, xmm5, 13				; 12AD _ C5 C1: 72. F5, 0D
	vpsrld	xmm0, xmm5, 19				; 12B2 _ C5 F9: 72. D5, 13
	vpor	xmm0, xmm7, xmm0			; 12B7 _ C5 C1: EB. C0
	vpslld	xmm5, xmm2, 3				; 12BB _ C5 D1: 72. F2, 03
	vpsrld	xmm2, xmm2, 29				; 12C0 _ C5 E9: 72. D2, 1D
	vpxor	xmm3, xmm6, xmm0			; 12C5 _ C5 C9: EF. D8
	vpor	xmm5, xmm5, xmm2			; 12C9 _ C5 D1: EB. EA
	vpslld	xmm7, xmm0, 3				; 12CD _ C5 C1: 72. F0, 03
	vpxor	xmm2, xmm3, xmm5			; 12D2 _ C5 E1: EF. D5
	vpxor	xmm4, xmm4, xmm5			; 12D6 _ C5 D9: EF. E5
	vpxor	xmm6, xmm4, xmm7			; 12DA _ C5 D9: EF. F7
	vpslld	xmm4, xmm2, 1				; 12DE _ C5 D9: 72. F2, 01
	vpsrld	xmm2, xmm2, 31				; 12E3 _ C5 E9: 72. D2, 1F
	vpslld	xmm3, xmm6, 7				; 12E8 _ C5 E1: 72. F6, 07
	vpor	xmm4, xmm4, xmm2			; 12ED _ C5 D9: EB. E2
	vpsrld	xmm6, xmm6, 25				; 12F1 _ C5 C9: 72. D6, 19
	vpor	xmm3, xmm3, xmm6			; 12F6 _ C5 E1: EB. DE
	vpxor	xmm0, xmm0, xmm4			; 12FA _ C5 F9: EF. C4
	vmovd	xmm6, dword [ecx+2B94H] 		; 12FE _ C5 F9: 6E. B1, 00002B94
	vpxor	xmm2, xmm0, xmm3			; 1306 _ C5 F9: EF. D3
	vpxor	xmm5, xmm5, xmm3			; 130A _ C5 D1: EF. EB
	vpslld	xmm0, xmm4, 7				; 130E _ C5 F9: 72. F4, 07
	vpxor	xmm7, xmm5, xmm0			; 1313 _ C5 D1: EF. F8
	vpshufd xmm0, xmm6, 0				; 1317 _ C5 F9: 70. C6, 00
	vpxor	xmm0, xmm4, xmm0			; 131C _ C5 D9: EF. C0
	vmovd	xmm4, dword [ecx+2B9CH] 		; 1320 _ C5 F9: 6E. A1, 00002B9C
	vpshufd xmm4, xmm4, 0				; 1328 _ C5 F9: 70. E4, 00
	vmovd	xmm5, dword [ecx+2B90H] 		; 132D _ C5 F9: 6E. A9, 00002B90
	vpxor	xmm4, xmm3, xmm4			; 1335 _ C5 E1: EF. E4
	vpslld	xmm3, xmm2, 5				; 1339 _ C5 E1: 72. F2, 05
	vpsrld	xmm2, xmm2, 27				; 133E _ C5 E9: 72. D2, 1B
	vpshufd xmm5, xmm5, 0				; 1343 _ C5 F9: 70. ED, 00
	vpor	xmm3, xmm3, xmm2			; 1348 _ C5 E1: EB. DA
	vpxor	xmm3, xmm3, xmm5			; 134C _ C5 E1: EF. DD
	vmovd	xmm6, dword [ecx+2B98H] 		; 1350 _ C5 F9: 6E. B1, 00002B98
	vpxor	xmm2, xmm3, xmm1			; 1358 _ C5 E1: EF. D1
	vpslld	xmm3, xmm7, 22				; 135C _ C5 E1: 72. F7, 16
	vpsrld	xmm7, xmm7, 10				; 1361 _ C5 C1: 72. D7, 0A
	vpor	xmm3, xmm3, xmm7			; 1366 _ C5 E1: EB. DF
	vpand	xmm5, xmm2, xmm0			; 136A _ C5 E9: DB. E8
	vpshufd xmm7, xmm6, 0				; 136E _ C5 F9: 70. FE, 00
	vpxor	xmm6, xmm3, xmm7			; 1373 _ C5 E1: EF. F7
	vpxor	xmm3, xmm6, xmm1			; 1377 _ C5 C9: EF. D9
	vpor	xmm6, xmm5, xmm4			; 137B _ C5 D1: EB. F4
	vpxor	xmm7, xmm3, xmm5			; 137F _ C5 E1: EF. FD
	vpxor	xmm0, xmm0, xmm6			; 1383 _ C5 F9: EF. C6
	vpxor	xmm3, xmm6, xmm2			; 1387 _ C5 C9: EF. DA
	vpxor	xmm5, xmm4, xmm7			; 138B _ C5 D9: EF. EF
	vpor	xmm4, xmm2, xmm0			; 138F _ C5 E9: EB. E0
	vpor	xmm2, xmm7, xmm3			; 1393 _ C5 C1: EB. D3
	vpand	xmm7, xmm2, xmm4			; 1397 _ C5 E9: DB. FC
	vpxor	xmm6, xmm0, xmm5			; 139B _ C5 F9: EF. F5
	vpslld	xmm0, xmm7, 13				; 139F _ C5 F9: 72. F7, 0D
	vpsrld	xmm2, xmm7, 19				; 13A4 _ C5 E9: 72. D7, 13
	vpxor	xmm3, xmm3, xmm6			; 13A9 _ C5 E1: EF. DE
	vpor	xmm0, xmm0, xmm2			; 13AD _ C5 F9: EB. C2
	vpslld	xmm2, xmm5, 3				; 13B1 _ C5 E9: 72. F5, 03
	vpsrld	xmm5, xmm5, 29				; 13B6 _ C5 D1: 72. D5, 1D
	vpor	xmm5, xmm2, xmm5			; 13BB _ C5 E9: EB. ED
	vpand	xmm2, xmm3, xmm7			; 13BF _ C5 E1: DB. D7
	vpxor	xmm4, xmm4, xmm2			; 13C3 _ C5 D9: EF. E2
	vpand	xmm7, xmm6, xmm7			; 13C7 _ C5 C9: DB. FF
	vpxor	xmm4, xmm4, xmm0			; 13CB _ C5 D9: EF. E0
	vpxor	xmm3, xmm7, xmm3			; 13CF _ C5 C1: EF. DB
	vpxor	xmm4, xmm4, xmm5			; 13D3 _ C5 D9: EF. E5
	vpxor	xmm6, xmm3, xmm5			; 13D7 _ C5 E1: EF. F5
	vpslld	xmm3, xmm0, 3				; 13DB _ C5 E1: 72. F0, 03
	vpslld	xmm2, xmm4, 1				; 13E0 _ C5 E9: 72. F4, 01
	vpxor	xmm7, xmm6, xmm3			; 13E5 _ C5 C9: EF. FB
	vpsrld	xmm4, xmm4, 31				; 13E9 _ C5 D9: 72. D4, 1F
	vpor	xmm3, xmm2, xmm4			; 13EE _ C5 E9: EB. DC
	vpslld	xmm2, xmm7, 7				; 13F2 _ C5 E9: 72. F7, 07
	vpsrld	xmm7, xmm7, 25				; 13F7 _ C5 C1: 72. D7, 19
	vpxor	xmm0, xmm0, xmm3			; 13FC _ C5 F9: EF. C3
	vpor	xmm6, xmm2, xmm7			; 1400 _ C5 E9: EB. F7
	vpslld	xmm4, xmm3, 7				; 1404 _ C5 D9: 72. F3, 07
	vpxor	xmm0, xmm0, xmm6			; 1409 _ C5 F9: EF. C6
	vpxor	xmm5, xmm5, xmm6			; 140D _ C5 D1: EF. EE
	vmovd	xmm7, dword [ecx+2BA0H] 		; 1411 _ C5 F9: 6E. B9, 00002BA0
	vpxor	xmm2, xmm5, xmm4			; 1419 _ C5 D1: EF. D4
	vpslld	xmm5, xmm0, 5				; 141D _ C5 D1: 72. F0, 05
	vpsrld	xmm0, xmm0, 27				; 1422 _ C5 F9: 72. D0, 1B
	vpor	xmm4, xmm5, xmm0			; 1427 _ C5 D1: EB. E0
	vmovd	xmm5, dword [ecx+2BA4H] 		; 142B _ C5 F9: 6E. A9, 00002BA4
	vpshufd xmm0, xmm7, 0				; 1433 _ C5 F9: 70. C7, 00
	vpxor	xmm7, xmm4, xmm0			; 1438 _ C5 D9: EF. F8
	vpshufd xmm4, xmm5, 0				; 143C _ C5 F9: 70. E5, 00
	vpxor	xmm0, xmm3, xmm4			; 1441 _ C5 E1: EF. C4
	vpslld	xmm3, xmm2, 22				; 1445 _ C5 E1: 72. F2, 16
	vmovd	xmm4, dword [ecx+2BA8H] 		; 144A _ C5 F9: 6E. A1, 00002BA8
	vpsrld	xmm2, xmm2, 10				; 1452 _ C5 E9: 72. D2, 0A
	vmovd	xmm5, dword [ecx+2BACH] 		; 1457 _ C5 F9: 6E. A9, 00002BAC
	vpor	xmm3, xmm3, xmm2			; 145F _ C5 E1: EB. DA
	vpshufd xmm2, xmm4, 0				; 1463 _ C5 F9: 70. D4, 00
	vpxor	xmm4, xmm3, xmm2			; 1468 _ C5 E1: EF. E2
	vpshufd xmm3, xmm5, 0				; 146C _ C5 F9: 70. DD, 00
	vpand	xmm2, xmm7, xmm4			; 1471 _ C5 C1: DB. D4
	vpxor	xmm6, xmm6, xmm3			; 1475 _ C5 C9: EF. F3
	vpxor	xmm4, xmm4, xmm0			; 1479 _ C5 D9: EF. E0
	vpxor	xmm5, xmm2, xmm6			; 147D _ C5 E9: EF. EE
	vpor	xmm6, xmm6, xmm7			; 1481 _ C5 C9: EB. F7
	vpxor	xmm4, xmm4, xmm5			; 1485 _ C5 D9: EF. E5
	vpxor	xmm0, xmm6, xmm0			; 1489 _ C5 C9: EF. C0
	vpxor	xmm7, xmm7, xmm4			; 148D _ C5 C1: EF. FC
	vpslld	xmm6, xmm4, 13				; 1491 _ C5 C9: 72. F4, 0D
	vpor	xmm3, xmm0, xmm7			; 1496 _ C5 F9: EB. DF
	vpsrld	xmm4, xmm4, 19				; 149A _ C5 D9: 72. D4, 13
	vpxor	xmm2, xmm3, xmm5			; 149F _ C5 E1: EF. D5
	vpand	xmm5, xmm5, xmm0			; 14A3 _ C5 D1: DB. E8
	vpxor	xmm5, xmm7, xmm5			; 14A7 _ C5 C1: EF. ED
	vpxor	xmm0, xmm0, xmm2			; 14AB _ C5 F9: EF. C2
	vpxor	xmm7, xmm0, xmm5			; 14AF _ C5 F9: EF. FD
	vpor	xmm0, xmm6, xmm4			; 14B3 _ C5 C9: EB. C4
	vpslld	xmm4, xmm7, 3				; 14B7 _ C5 D9: 72. F7, 03
	vpsrld	xmm3, xmm7, 29				; 14BC _ C5 E1: 72. D7, 1D
	vpor	xmm4, xmm4, xmm3			; 14C1 _ C5 D9: EB. E3
	vpxor	xmm2, xmm2, xmm0			; 14C5 _ C5 E9: EF. D0
	vpxor	xmm1, xmm5, xmm1			; 14C9 _ C5 D1: EF. C9
	vpxor	xmm2, xmm2, xmm4			; 14CD _ C5 E9: EF. D4
	vpxor	xmm1, xmm1, xmm4			; 14D1 _ C5 F1: EF. CC
	vpslld	xmm3, xmm0, 3				; 14D5 _ C5 E1: 72. F0, 03
	vpxor	xmm6, xmm1, xmm3			; 14DA _ C5 F1: EF. F3
	vpslld	xmm5, xmm2, 1				; 14DE _ C5 D1: 72. F2, 01
	vpsrld	xmm2, xmm2, 31				; 14E3 _ C5 E9: 72. D2, 1F
	vpslld	xmm7, xmm6, 7				; 14E8 _ C5 C1: 72. F6, 07
	vpor	xmm1, xmm5, xmm2			; 14ED _ C5 D1: EB. CA
	vpsrld	xmm3, xmm6, 25				; 14F1 _ C5 E1: 72. D6, 19
	vpor	xmm3, xmm7, xmm3			; 14F6 _ C5 C1: EB. DB
	vpxor	xmm0, xmm0, xmm1			; 14FA _ C5 F9: EF. C1
	vpxor	xmm2, xmm0, xmm3			; 14FE _ C5 F9: EF. D3
	vpxor	xmm0, xmm4, xmm3			; 1502 _ C5 D9: EF. C3
	vpslld	xmm5, xmm1, 7				; 1506 _ C5 D1: 72. F1, 07
	vpxor	xmm6, xmm0, xmm5			; 150B _ C5 F9: EF. F5
	vpslld	xmm0, xmm2, 5				; 150F _ C5 F9: 72. F2, 05
	vpsrld	xmm5, xmm2, 27				; 1514 _ C5 D1: 72. D2, 1B
	vmovd	xmm7, dword [ecx+2BB0H] 		; 1519 _ C5 F9: 6E. B9, 00002BB0
	vpor	xmm2, xmm0, xmm5			; 1521 _ C5 F9: EB. D5
	vmovd	xmm0, dword [ecx+2BB4H] 		; 1525 _ C5 F9: 6E. 81, 00002BB4
	vpshufd xmm4, xmm7, 0				; 152D _ C5 F9: 70. E7, 00
	vpshufd xmm7, xmm0, 0				; 1532 _ C5 F9: 70. F8, 00
	vpxor	xmm5, xmm2, xmm4			; 1537 _ C5 E9: EF. EC
	vpxor	xmm4, xmm1, xmm7			; 153B _ C5 F1: EF. E7
	vpslld	xmm1, xmm6, 22				; 153F _ C5 F1: 72. F6, 16
	vpsrld	xmm6, xmm6, 10				; 1544 _ C5 C9: 72. D6, 0A
	vmovd	xmm0, dword [ecx+2BB8H] 		; 1549 _ C5 F9: 6E. 81, 00002BB8
	vpor	xmm7, xmm1, xmm6			; 1551 _ C5 F1: EB. FE
	vmovd	xmm1, dword [ecx+2BBCH] 		; 1555 _ C5 F9: 6E. 89, 00002BBC
	vpshufd xmm2, xmm0, 0				; 155D _ C5 F9: 70. D0, 00
	vpand	xmm0, xmm4, xmm5			; 1562 _ C5 D9: DB. C5
	vpshufd xmm6, xmm1, 0				; 1566 _ C5 F9: 70. F1, 00
	vpxor	xmm2, xmm7, xmm2			; 156B _ C5 C1: EF. D2
	vpxor	xmm3, xmm3, xmm6			; 156F _ C5 E1: EF. DE
	vpor	xmm6, xmm5, xmm3			; 1573 _ C5 D1: EB. F3
	vpxor	xmm1, xmm3, xmm4			; 1577 _ C5 E1: EF. CC
	vpxor	xmm5, xmm5, xmm2			; 157B _ C5 D1: EF. EA
	vpxor	xmm3, xmm2, xmm1			; 157F _ C5 E9: EF. D9
	vpor	xmm2, xmm5, xmm0			; 1583 _ C5 D1: EB. D0
	vpand	xmm4, xmm1, xmm6			; 1587 _ C5 F1: DB. E6
	vpxor	xmm7, xmm4, xmm2			; 158B _ C5 D9: EF. FA
	vpxor	xmm5, xmm6, xmm0			; 158F _ C5 C9: EF. E8
	vpxor	xmm0, xmm0, xmm7			; 1593 _ C5 F9: EF. C7
	vpxor	xmm4, xmm5, xmm7			; 1597 _ C5 D1: EF. E7
	vpor	xmm1, xmm0, xmm5			; 159B _ C5 F9: EB. CD
	vpand	xmm5, xmm2, xmm5			; 159F _ C5 E9: DB. ED
	vpxor	xmm0, xmm1, xmm3			; 15A3 _ C5 F1: EF. C3
	vpxor	xmm2, xmm5, xmm3			; 15A7 _ C5 D1: EF. D3
	vpor	xmm6, xmm0, xmm7			; 15AB _ C5 F9: EB. F7
	vpxor	xmm1, xmm4, xmm6			; 15AF _ C5 D9: EF. CE
	vpslld	xmm4, xmm1, 13				; 15B3 _ C5 D9: 72. F1, 0D
	vpsrld	xmm6, xmm1, 19				; 15B8 _ C5 C9: 72. D1, 13
	vpor	xmm1, xmm4, xmm6			; 15BD _ C5 D9: EB. CE
	vpslld	xmm4, xmm7, 3				; 15C1 _ C5 D9: 72. F7, 03
	vpsrld	xmm7, xmm7, 29				; 15C6 _ C5 C1: 72. D7, 1D
	vpxor	xmm0, xmm0, xmm1			; 15CB _ C5 F9: EF. C1
	vpor	xmm4, xmm4, xmm7			; 15CF _ C5 D9: EB. E7
	vpxor	xmm7, xmm0, xmm4			; 15D3 _ C5 F9: EF. FC
	vpxor	xmm3, xmm2, xmm4			; 15D7 _ C5 E9: EF. DC
	vpslld	xmm0, xmm1, 3				; 15DB _ C5 F9: 72. F1, 03
	vpslld	xmm5, xmm7, 1				; 15E0 _ C5 D1: 72. F7, 01
	vpxor	xmm2, xmm3, xmm0			; 15E5 _ C5 E1: EF. D0
	vpsrld	xmm7, xmm7, 31				; 15E9 _ C5 C1: 72. D7, 1F
	vpor	xmm7, xmm5, xmm7			; 15EE _ C5 D1: EB. FF
	vpslld	xmm6, xmm2, 7				; 15F2 _ C5 C9: 72. F2, 07
	vpsrld	xmm3, xmm2, 25				; 15F7 _ C5 E1: 72. D2, 19
	vpxor	xmm1, xmm1, xmm7			; 15FC _ C5 F1: EF. CF
	vpor	xmm5, xmm6, xmm3			; 1600 _ C5 C9: EB. EB
	vpslld	xmm0, xmm7, 7				; 1604 _ C5 F9: 72. F7, 07
	vpxor	xmm2, xmm1, xmm5			; 1609 _ C5 F1: EF. D5
	vpxor	xmm4, xmm4, xmm5			; 160D _ C5 D9: EF. E5
	vmovd	xmm3, dword [ecx+2BC0H] 		; 1611 _ C5 F9: 6E. 99, 00002BC0
	vpxor	xmm6, xmm4, xmm0			; 1619 _ C5 D9: EF. F0
	vpslld	xmm1, xmm2, 5				; 161D _ C5 F1: 72. F2, 05
	vpsrld	xmm4, xmm2, 27				; 1622 _ C5 D9: 72. D2, 1B
	vpshufd xmm2, xmm3, 0				; 1627 _ C5 F9: 70. D3, 00
	vpor	xmm0, xmm1, xmm4			; 162C _ C5 F1: EB. C4
	vmovd	xmm4, dword [ecx+2BCCH] 		; 1630 _ C5 F9: 6E. A1, 00002BCC
	vpxor	xmm1, xmm0, xmm2			; 1638 _ C5 F9: EF. CA
	vmovd	xmm3, dword [ecx+2BC4H] 		; 163C _ C5 F9: 6E. 99, 00002BC4
	vpshufd xmm2, xmm4, 0				; 1644 _ C5 F9: 70. D4, 00
	vpxor	xmm4, xmm5, xmm2			; 1649 _ C5 D1: EF. E2
	vpshufd xmm5, xmm3, 0				; 164D _ C5 F9: 70. EB, 00
	vpslld	xmm3, xmm6, 22				; 1652 _ C5 E1: 72. F6, 16
	vpxor	xmm7, xmm7, xmm5			; 1657 _ C5 C1: EF. FD
	vpsrld	xmm6, xmm6, 10				; 165B _ C5 C9: 72. D6, 0A
	vpxor	xmm5, xmm7, xmm4			; 1660 _ C5 C1: EF. EC
	vmovd	xmm0, dword [ecx+2BC8H] 		; 1664 _ C5 F9: 6E. 81, 00002BC8
	vpcmpeqd xmm7, xmm7, xmm7			; 166C _ C5 C1: 76. FF
	vpshufd xmm0, xmm0, 0				; 1670 _ C5 F9: 70. C0, 00
	vpxor	xmm2, xmm4, xmm7			; 1675 _ C5 D9: EF. D7
	vpor	xmm4, xmm3, xmm6			; 1679 _ C5 E1: EB. E6
	vpxor	xmm6, xmm4, xmm0			; 167D _ C5 D9: EF. F0
	vpxor	xmm0, xmm6, xmm2			; 1681 _ C5 C9: EF. C2
	vpxor	xmm6, xmm2, xmm1			; 1685 _ C5 E9: EF. F1
	vpand	xmm2, xmm5, xmm6			; 1689 _ C5 D1: DB. D6
	vpxor	xmm4, xmm5, xmm6			; 168D _ C5 D1: EF. E6
	vpxor	xmm3, xmm2, xmm0			; 1691 _ C5 E9: EF. D8
	vpxor	xmm1, xmm1, xmm4			; 1695 _ C5 F1: EF. CC
	vpand	xmm5, xmm0, xmm4			; 1699 _ C5 F9: DB. EC
	vpand	xmm0, xmm1, xmm3			; 169D _ C5 F1: DB. C3
	vpxor	xmm2, xmm5, xmm1			; 16A1 _ C5 D1: EF. D1
	vpxor	xmm5, xmm6, xmm0			; 16A5 _ C5 C9: EF. E8
	vpor	xmm1, xmm0, xmm5			; 16A9 _ C5 F9: EB. CD
	vpxor	xmm6, xmm1, xmm2			; 16AD _ C5 F1: EF. F2
	vpsrld	xmm1, xmm3, 19				; 16B1 _ C5 F1: 72. D3, 13
	vpxor	xmm6, xmm6, xmm7			; 16B6 _ C5 C9: EF. F7
	vpslld	xmm7, xmm3, 13				; 16BA _ C5 C1: 72. F3, 0D
	vpor	xmm7, xmm7, xmm1			; 16BF _ C5 C1: EB. F9
	vpslld	xmm1, xmm6, 3				; 16C3 _ C5 F1: 72. F6, 03
	vpsrld	xmm6, xmm6, 29				; 16C8 _ C5 C9: 72. D6, 1D
	vpor	xmm3, xmm4, xmm3			; 16CD _ C5 D9: EB. DB
	vpor	xmm1, xmm1, xmm6			; 16D1 _ C5 F1: EB. CE
	vpxor	xmm0, xmm3, xmm0			; 16D5 _ C5 E1: EF. C0
	vpand	xmm2, xmm2, xmm5			; 16D9 _ C5 E9: DB. D5
	vpxor	xmm5, xmm5, xmm1			; 16DD _ C5 D1: EF. E9
	vpxor	xmm4, xmm0, xmm2			; 16E1 _ C5 F9: EF. E2
	vpslld	xmm3, xmm7, 3				; 16E5 _ C5 E1: 72. F7, 03
	vpxor	xmm6, xmm4, xmm7			; 16EA _ C5 D9: EF. F7
	vpxor	xmm4, xmm5, xmm3			; 16EE _ C5 D1: EF. E3
	vpxor	xmm0, xmm6, xmm1			; 16F2 _ C5 C9: EF. C1
	vpslld	xmm6, xmm4, 7				; 16F6 _ C5 C9: 72. F4, 07
	vpsrld	xmm3, xmm4, 25				; 16FB _ C5 E1: 72. D4, 19
	vpslld	xmm5, xmm0, 1				; 1700 _ C5 D1: 72. F0, 01
	vpsrld	xmm2, xmm0, 31				; 1705 _ C5 E9: 72. D0, 1F
	vpor	xmm4, xmm6, xmm3			; 170A _ C5 C9: EB. E3
	vpor	xmm5, xmm5, xmm2			; 170E _ C5 D1: EB. EA
	vpxor	xmm0, xmm1, xmm4			; 1712 _ C5 F1: EF. C4
	vmovd	xmm1, dword [ecx+2BD4H] 		; 1716 _ C5 F9: 6E. 89, 00002BD4
	vpxor	xmm7, xmm7, xmm5			; 171E _ C5 C1: EF. FD
	vpshufd xmm2, xmm1, 0				; 1722 _ C5 F9: 70. D1, 00
	vpxor	xmm3, xmm7, xmm4			; 1727 _ C5 C1: EF. DC
	vpslld	xmm7, xmm5, 7				; 172B _ C5 C1: 72. F5, 07
	vpxor	xmm1, xmm5, xmm2			; 1730 _ C5 D1: EF. CA
	vmovd	xmm5, dword [ecx+2BDCH] 		; 1734 _ C5 F9: 6E. A9, 00002BDC
	vpxor	xmm7, xmm0, xmm7			; 173C _ C5 F9: EF. FF
	vmovd	xmm0, dword [ecx+2BD0H] 		; 1740 _ C5 F9: 6E. 81, 00002BD0
	vpshufd xmm6, xmm5, 0				; 1748 _ C5 F9: 70. F5, 00
	vpslld	xmm5, xmm3, 5				; 174D _ C5 D1: 72. F3, 05
	vpsrld	xmm3, xmm3, 27				; 1752 _ C5 E1: 72. D3, 1B
	vpxor	xmm4, xmm4, xmm6			; 1757 _ C5 D9: EF. E6
	vpshufd xmm0, xmm0, 0				; 175B _ C5 F9: 70. C0, 00
	vpor	xmm5, xmm5, xmm3			; 1760 _ C5 D1: EB. EB
	vpxor	xmm6, xmm5, xmm0			; 1764 _ C5 D1: EF. F0
	vpxor	xmm5, xmm1, xmm4			; 1768 _ C5 F1: EF. EC
	vpxor	xmm0, xmm6, xmm1			; 176C _ C5 C9: EF. C1
	vpcmpeqd xmm1, xmm1, xmm1			; 1770 _ C5 F1: 76. C9
	vmovd	xmm2, dword [ecx+2BD8H] 		; 1774 _ C5 F9: 6E. 91, 00002BD8
	vpxor	xmm6, xmm4, xmm1			; 177C _ C5 D9: EF. F1
	vpslld	xmm4, xmm7, 22				; 1780 _ C5 D9: 72. F7, 16
	vpsrld	xmm7, xmm7, 10				; 1785 _ C5 C1: 72. D7, 0A
	vpshufd xmm2, xmm2, 0				; 178A _ C5 F9: 70. D2, 00
	vpor	xmm4, xmm4, xmm7			; 178F _ C5 D9: EB. E7
	vpxor	xmm3, xmm4, xmm2			; 1793 _ C5 D9: EF. DA
	vpand	xmm7, xmm5, xmm0			; 1797 _ C5 D1: DB. F8
	vpxor	xmm2, xmm3, xmm6			; 179B _ C5 E1: EF. D6
	vpxor	xmm4, xmm7, xmm2			; 179F _ C5 C1: EF. E2
	vpor	xmm3, xmm2, xmm5			; 17A3 _ C5 E9: EB. DD
	vpand	xmm7, xmm6, xmm4			; 17A7 _ C5 C9: DB. FC
	vpxor	xmm5, xmm5, xmm6			; 17AB _ C5 D1: EF. EE
	vpxor	xmm2, xmm7, xmm0			; 17AF _ C5 C1: EF. D0
	vpxor	xmm6, xmm5, xmm4			; 17B3 _ C5 D1: EF. F4
	vpxor	xmm6, xmm6, xmm3			; 17B7 _ C5 C9: EF. F3
	vpand	xmm5, xmm0, xmm2			; 17BB _ C5 F9: DB. EA
	vpxor	xmm5, xmm5, xmm6			; 17BF _ C5 D1: EF. EE
	vpslld	xmm7, xmm4, 13				; 17C3 _ C5 C1: 72. F4, 0D
	vpsrld	xmm4, xmm4, 19				; 17C8 _ C5 D9: 72. D4, 13
	vpxor	xmm0, xmm3, xmm0			; 17CD _ C5 E1: EF. C0
	vpor	xmm7, xmm7, xmm4			; 17D1 _ C5 C1: EB. FC
	vpslld	xmm4, xmm5, 3				; 17D5 _ C5 D9: 72. F5, 03
	vpsrld	xmm5, xmm5, 29				; 17DA _ C5 D1: 72. D5, 1D
	vpor	xmm6, xmm6, xmm2			; 17DF _ C5 C9: EB. F2
	vpxor	xmm3, xmm0, xmm1			; 17E3 _ C5 F9: EF. D9
	vpor	xmm4, xmm4, xmm5			; 17E7 _ C5 D9: EB. E5
	vpxor	xmm5, xmm2, xmm7			; 17EB _ C5 E9: EF. EF
	vpxor	xmm0, xmm6, xmm3			; 17EF _ C5 C9: EF. C3
	vpxor	xmm5, xmm5, xmm4			; 17F3 _ C5 D1: EF. EC
	vpxor	xmm2, xmm0, xmm4			; 17F7 _ C5 F9: EF. D4
	vpslld	xmm6, xmm7, 3				; 17FB _ C5 C9: 72. F7, 03
	vpslld	xmm0, xmm5, 1				; 1800 _ C5 F9: 72. F5, 01
	vpxor	xmm3, xmm2, xmm6			; 1805 _ C5 E9: EF. DE
	vpsrld	xmm5, xmm5, 31				; 1809 _ C5 D1: 72. D5, 1F
	vpor	xmm0, xmm0, xmm5			; 180E _ C5 F9: EB. C5
	vpslld	xmm2, xmm3, 7				; 1812 _ C5 E9: 72. F3, 07
	vpsrld	xmm3, xmm3, 25				; 1817 _ C5 E1: 72. D3, 19
	vpxor	xmm7, xmm7, xmm0			; 181C _ C5 C1: EF. F8
	vpor	xmm2, xmm2, xmm3			; 1820 _ C5 E9: EB. D3
	vpxor	xmm5, xmm7, xmm2			; 1824 _ C5 C1: EF. EA
	vpxor	xmm7, xmm4, xmm2			; 1828 _ C5 D9: EF. FA
	vpslld	xmm4, xmm0, 7				; 182C _ C5 D9: 72. F0, 07
	vmovd	xmm3, dword [ecx+2BE0H] 		; 1831 _ C5 F9: 6E. 99, 00002BE0
	vpxor	xmm6, xmm7, xmm4			; 1839 _ C5 C1: EF. F4
	vmovd	xmm4, dword [ecx+2BECH] 		; 183D _ C5 F9: 6E. A1, 00002BEC
	vpslld	xmm7, xmm5, 5				; 1845 _ C5 C1: 72. F5, 05
	vpsrld	xmm5, xmm5, 27				; 184A _ C5 D1: 72. D5, 1B
	vpshufd xmm4, xmm4, 0				; 184F _ C5 F9: 70. E4, 00
	vpor	xmm5, xmm7, xmm5			; 1854 _ C5 C1: EB. ED
	vpshufd xmm7, xmm3, 0				; 1858 _ C5 F9: 70. FB, 00
	vpxor	xmm4, xmm2, xmm4			; 185D _ C5 E9: EF. E4
	vpxor	xmm3, xmm5, xmm7			; 1861 _ C5 D1: EF. DF
	vpslld	xmm2, xmm6, 22				; 1865 _ C5 E9: 72. F6, 16
	vmovd	xmm5, dword [ecx+2BE8H] 		; 186A _ C5 F9: 6E. A9, 00002BE8
	vpsrld	xmm6, xmm6, 10				; 1872 _ C5 C9: 72. D6, 0A
	vpshufd xmm5, xmm5, 0				; 1877 _ C5 F9: 70. ED, 00
	vpor	xmm2, xmm2, xmm6			; 187C _ C5 E9: EB. D6
	vmovd	xmm7, dword [ecx+2BE4H] 		; 1880 _ C5 F9: 6E. B9, 00002BE4
	vpxor	xmm6, xmm2, xmm5			; 1888 _ C5 E9: EF. F5
	vpshufd xmm7, xmm7, 0				; 188C _ C5 F9: 70. FF, 00
	vpxor	xmm5, xmm6, xmm1			; 1891 _ C5 C9: EF. E9
	vpand	xmm2, xmm4, xmm3			; 1895 _ C5 D9: DB. D3
	vpxor	xmm6, xmm3, xmm4			; 1899 _ C5 E1: EF. F4
	vpxor	xmm3, xmm2, xmm5			; 189D _ C5 E9: EF. DD
	vpxor	xmm0, xmm0, xmm7			; 18A1 _ C5 F9: EF. C7
	vpxor	xmm0, xmm0, xmm3			; 18A5 _ C5 F9: EF. C3
	vpor	xmm5, xmm5, xmm4			; 18A9 _ C5 D1: EB. EC
	vpor	xmm2, xmm6, xmm0			; 18AD _ C5 C9: EB. D0
	vpxor	xmm7, xmm5, xmm6			; 18B1 _ C5 D1: EF. FE
	vpxor	xmm5, xmm7, xmm0			; 18B5 _ C5 C1: EF. E8
	vpor	xmm6, xmm2, xmm3			; 18B9 _ C5 E9: EB. F3
	vpxor	xmm4, xmm4, xmm2			; 18BD _ C5 D9: EF. E2
	vpxor	xmm6, xmm6, xmm5			; 18C1 _ C5 C9: EF. F5
	vpxor	xmm7, xmm4, xmm3			; 18C5 _ C5 D9: EF. FB
	vpslld	xmm4, xmm6, 13				; 18C9 _ C5 D9: 72. F6, 0D
	vpxor	xmm2, xmm7, xmm6			; 18CE _ C5 C1: EF. D6
	vpsrld	xmm6, xmm6, 19				; 18D2 _ C5 C9: 72. D6, 13
	vpor	xmm7, xmm4, xmm6			; 18D7 _ C5 D9: EB. FE
	vpslld	xmm4, xmm2, 3				; 18DB _ C5 D9: 72. F2, 03
	vpsrld	xmm6, xmm2, 29				; 18E0 _ C5 C9: 72. D2, 1D
	vpxor	xmm3, xmm3, xmm1			; 18E5 _ C5 E1: EF. D9
	vpand	xmm5, xmm5, xmm2			; 18E9 _ C5 D1: DB. EA
	vpor	xmm6, xmm4, xmm6			; 18ED _ C5 D9: EB. F6
	vpxor	xmm0, xmm0, xmm7			; 18F1 _ C5 F9: EF. C7
	vpxor	xmm2, xmm3, xmm5			; 18F5 _ C5 E1: EF. D5
	vpxor	xmm0, xmm0, xmm6			; 18F9 _ C5 F9: EF. C6
	vpxor	xmm4, xmm2, xmm6			; 18FD _ C5 E9: EF. E6
	vpslld	xmm3, xmm7, 3				; 1901 _ C5 E1: 72. F7, 03
	vpslld	xmm2, xmm0, 1				; 1906 _ C5 E9: 72. F0, 01
	vpxor	xmm5, xmm4, xmm3			; 190B _ C5 D9: EF. EB
	vpsrld	xmm0, xmm0, 31				; 190F _ C5 F9: 72. D0, 1F
	vpor	xmm0, xmm2, xmm0			; 1914 _ C5 E9: EB. C0
	vpslld	xmm4, xmm5, 7				; 1918 _ C5 D9: 72. F5, 07
	vpsrld	xmm5, xmm5, 25				; 191D _ C5 D1: 72. D5, 19
	vpxor	xmm7, xmm7, xmm0			; 1922 _ C5 C1: EF. F8
	vpor	xmm5, xmm4, xmm5			; 1926 _ C5 D9: EB. ED
	vpxor	xmm3, xmm7, xmm5			; 192A _ C5 C1: EF. DD
	vpxor	xmm6, xmm6, xmm5			; 192E _ C5 C9: EF. F5
	vpslld	xmm7, xmm0, 7				; 1932 _ C5 C1: 72. F0, 07
	vpslld	xmm2, xmm3, 5				; 1937 _ C5 E9: 72. F3, 05
	vpsrld	xmm4, xmm3, 27				; 193C _ C5 D9: 72. D3, 1B
	vpxor	xmm7, xmm6, xmm7			; 1941 _ C5 C9: EF. FF
	vmovd	xmm6, dword [ecx+2BF0H] 		; 1945 _ C5 F9: 6E. B1, 00002BF0
	vpor	xmm3, xmm2, xmm4			; 194D _ C5 E9: EB. DC
	vmovd	xmm4, dword [ecx+2BF4H] 		; 1951 _ C5 F9: 6E. A1, 00002BF4
	vpshufd xmm2, xmm6, 0				; 1959 _ C5 F9: 70. D6, 00
	vpshufd xmm6, xmm4, 0				; 195E _ C5 F9: 70. F4, 00
	vpxor	xmm3, xmm3, xmm2			; 1963 _ C5 E1: EF. DA
	vpxor	xmm2, xmm0, xmm6			; 1967 _ C5 F9: EF. D6
	vpslld	xmm0, xmm7, 22				; 196B _ C5 F9: 72. F7, 16
	vpsrld	xmm7, xmm7, 10				; 1970 _ C5 C1: 72. D7, 0A
	vpor	xmm4, xmm0, xmm7			; 1975 _ C5 F9: EB. E7
	vmovd	xmm0, dword [ecx+2BF8H] 		; 1979 _ C5 F9: 6E. 81, 00002BF8
	vpshufd xmm6, xmm0, 0				; 1981 _ C5 F9: 70. F0, 00
	vmovd	xmm0, dword [ecx+2BFCH] 		; 1986 _ C5 F9: 6E. 81, 00002BFC
	vpxor	xmm4, xmm4, xmm6			; 198E _ C5 D9: EF. E6
	vpshufd xmm7, xmm0, 0				; 1992 _ C5 F9: 70. F8, 00
	vpxor	xmm0, xmm5, xmm7			; 1997 _ C5 D1: EF. C7
	vpor	xmm5, xmm2, xmm4			; 199B _ C5 E9: EB. EC
	vpxor	xmm7, xmm5, xmm0			; 199F _ C5 D1: EF. F8
	vpxor	xmm2, xmm2, xmm4			; 19A3 _ C5 E9: EF. D4
	vpxor	xmm6, xmm4, xmm7			; 19A7 _ C5 D9: EF. F7
	vpxor	xmm5, xmm2, xmm6			; 19AB _ C5 E9: EF. EE
	vpor	xmm2, xmm0, xmm2			; 19AF _ C5 F9: EB. D2
	vmovdqu oword [esp+0D0H], xmm7			; 19B3 _ C5 FA: 7F. BC 24, 000000D0
	vpor	xmm7, xmm7, xmm5			; 19BC _ C5 C1: EB. FD
	vmovdqu oword [esp+0C0H], xmm3			; 19C0 _ C5 FA: 7F. 9C 24, 000000C0
	vpor	xmm4, xmm3, xmm5			; 19C9 _ C5 E1: EB. E5
	vpxor	xmm3, xmm7, xmm3			; 19CD _ C5 C1: EF. DB
	vpxor	xmm4, xmm4, xmm6			; 19D1 _ C5 D9: EF. E6
	vpxor	xmm7, xmm3, xmm5			; 19D5 _ C5 E1: EF. FD
	vpxor	xmm6, xmm6, xmm7			; 19D9 _ C5 C9: EF. F7
	vpand	xmm3, xmm7, xmm4			; 19DD _ C5 C1: DB. DC
	vpxor	xmm7, xmm6, xmm1			; 19E1 _ C5 C9: EF. F9
	vpxor	xmm3, xmm3, xmm5			; 19E5 _ C5 E1: EF. DD
	vpor	xmm6, xmm7, xmm4			; 19E9 _ C5 C1: EB. F4
	vpxor	xmm5, xmm5, xmm6			; 19ED _ C5 D1: EF. EE
	vpslld	xmm6, xmm3, 3				; 19F1 _ C5 C9: 72. F3, 03
	vpand	xmm0, xmm2, oword [esp+0C0H]		; 19F6 _ C5 E9: DB. 84 24, 000000C0
	vpslld	xmm7, xmm5, 13				; 19FF _ C5 C1: 72. F5, 0D
	vpsrld	xmm5, xmm5, 19				; 1A04 _ C5 D1: 72. D5, 13
	vpsrld	xmm3, xmm3, 29				; 1A09 _ C5 E1: 72. D3, 1D
	vpxor	xmm2, xmm0, oword [esp+0D0H]		; 1A0E _ C5 F9: EF. 94 24, 000000D0
	vpor	xmm7, xmm7, xmm5			; 1A17 _ C5 C1: EB. FD
	vpor	xmm5, xmm6, xmm3			; 1A1B _ C5 C9: EB. EB
	vpxor	xmm6, xmm2, xmm7			; 1A1F _ C5 E9: EF. F7
	vpxor	xmm6, xmm6, xmm5			; 1A23 _ C5 C9: EF. F5
	vpxor	xmm4, xmm4, xmm5			; 1A27 _ C5 D9: EF. E5
	vpslld	xmm3, xmm7, 3				; 1A2B _ C5 E1: 72. F7, 03
	vpslld	xmm0, xmm6, 1				; 1A30 _ C5 F9: 72. F6, 01
	vpxor	xmm2, xmm4, xmm3			; 1A35 _ C5 D9: EF. D3
	vpsrld	xmm4, xmm6, 31				; 1A39 _ C5 D9: 72. D6, 1F
	vpor	xmm4, xmm0, xmm4			; 1A3E _ C5 F9: EB. E4
	vpslld	xmm0, xmm2, 7				; 1A42 _ C5 F9: 72. F2, 07
	vpsrld	xmm2, xmm2, 25				; 1A47 _ C5 E9: 72. D2, 19
	vpxor	xmm7, xmm7, xmm4			; 1A4C _ C5 C1: EF. FC
	vpor	xmm6, xmm0, xmm2			; 1A50 _ C5 F9: EB. F2
	vpxor	xmm0, xmm7, xmm6			; 1A54 _ C5 C1: EF. C6
	vpxor	xmm5, xmm5, xmm6			; 1A58 _ C5 D1: EF. EE
	vpslld	xmm7, xmm4, 7				; 1A5C _ C5 C1: 72. F4, 07
	vmovd	xmm2, dword [ecx+2C00H] 		; 1A61 _ C5 F9: 6E. 91, 00002C00
	vpxor	xmm3, xmm5, xmm7			; 1A69 _ C5 D1: EF. DF
	vmovd	xmm7, dword [ecx+2C04H] 		; 1A6D _ C5 F9: 6E. B9, 00002C04
	vpslld	xmm5, xmm0, 5				; 1A75 _ C5 D1: 72. F0, 05
	vpsrld	xmm0, xmm0, 27				; 1A7A _ C5 F9: 72. D0, 1B
	vpor	xmm0, xmm5, xmm0			; 1A7F _ C5 D1: EB. C0
	vpshufd xmm5, xmm2, 0				; 1A83 _ C5 F9: 70. EA, 00
	vpshufd xmm2, xmm7, 0				; 1A88 _ C5 F9: 70. D7, 00
	vpxor	xmm0, xmm0, xmm5			; 1A8D _ C5 F9: EF. C5
	vmovd	xmm5, dword [ecx+2C08H] 		; 1A91 _ C5 F9: 6E. A9, 00002C08
	vpxor	xmm2, xmm4, xmm2			; 1A99 _ C5 D9: EF. D2
	vpslld	xmm4, xmm3, 22				; 1A9D _ C5 D9: 72. F3, 16
	vpsrld	xmm3, xmm3, 10				; 1AA2 _ C5 E1: 72. D3, 0A
	vpor	xmm7, xmm4, xmm3			; 1AA7 _ C5 D9: EB. FB
	vmovd	xmm3, dword [ecx+2C0CH] 		; 1AAB _ C5 F9: 6E. 99, 00002C0C
	vpshufd xmm4, xmm5, 0				; 1AB3 _ C5 F9: 70. E5, 00
	vpxor	xmm5, xmm7, xmm4			; 1AB8 _ C5 C1: EF. EC
	vpshufd xmm7, xmm3, 0				; 1ABC _ C5 F9: 70. FB, 00
	vpxor	xmm6, xmm6, xmm7			; 1AC1 _ C5 C9: EF. F7
	vpxor	xmm7, xmm6, xmm0			; 1AC5 _ C5 C9: EF. F8
	vpxor	xmm6, xmm2, xmm5			; 1AC9 _ C5 E9: EF. F5
	vpand	xmm2, xmm2, xmm7			; 1ACD _ C5 E9: DB. D7
	vpxor	xmm4, xmm6, xmm7			; 1AD1 _ C5 C9: EF. E7
	vpxor	xmm2, xmm2, xmm0			; 1AD5 _ C5 E9: EF. D0
	vpor	xmm0, xmm0, xmm7			; 1AD9 _ C5 F9: EB. C7
	vpxor	xmm3, xmm0, xmm6			; 1ADD _ C5 F9: EF. DE
	vpxor	xmm7, xmm7, xmm5			; 1AE1 _ C5 C1: EF. FD
	vpor	xmm5, xmm5, xmm2			; 1AE5 _ C5 D1: EB. EA
	vpxor	xmm0, xmm4, xmm1			; 1AE9 _ C5 D9: EF. C1
	vpxor	xmm6, xmm5, xmm4			; 1AED _ C5 D1: EF. F4
	vpor	xmm0, xmm0, xmm2			; 1AF1 _ C5 F9: EB. C2
	vpor	xmm4, xmm7, xmm3			; 1AF5 _ C5 C1: EB. E3
	vpxor	xmm7, xmm2, xmm7			; 1AF9 _ C5 E9: EF. FF
	vpxor	xmm2, xmm7, xmm0			; 1AFD _ C5 C1: EF. D0
	vpxor	xmm5, xmm2, xmm4			; 1B01 _ C5 E9: EF. EC
	vpxor	xmm4, xmm0, xmm4			; 1B05 _ C5 F9: EF. E4
	vpslld	xmm7, xmm5, 13				; 1B09 _ C5 C1: 72. F5, 0D
	vpsrld	xmm2, xmm5, 19				; 1B0E _ C5 E9: 72. D5, 13
	vpslld	xmm5, xmm6, 3				; 1B13 _ C5 D1: 72. F6, 03
	vpsrld	xmm6, xmm6, 29				; 1B18 _ C5 C9: 72. D6, 1D
	vpor	xmm2, xmm7, xmm2			; 1B1D _ C5 C1: EB. D2
	vpor	xmm5, xmm5, xmm6			; 1B21 _ C5 D1: EB. EE
	vpxor	xmm0, xmm4, xmm2			; 1B25 _ C5 D9: EF. C2
	vpxor	xmm3, xmm3, xmm5			; 1B29 _ C5 E1: EF. DD
	vpslld	xmm7, xmm2, 3				; 1B2D _ C5 C1: 72. F2, 03
	vpxor	xmm0, xmm0, xmm5			; 1B32 _ C5 F9: EF. C5
	vpxor	xmm4, xmm3, xmm7			; 1B36 _ C5 E1: EF. E7
	vpslld	xmm6, xmm0, 1				; 1B3A _ C5 C9: 72. F0, 01
	vpsrld	xmm0, xmm0, 31				; 1B3F _ C5 F9: 72. D0, 1F
	vpslld	xmm3, xmm4, 7				; 1B44 _ C5 E1: 72. F4, 07
	vpsrld	xmm4, xmm4, 25				; 1B49 _ C5 D9: 72. D4, 19
	vpor	xmm0, xmm6, xmm0			; 1B4E _ C5 C9: EB. C0
	vpor	xmm6, xmm3, xmm4			; 1B52 _ C5 E1: EB. F4
	vpslld	xmm7, xmm0, 7				; 1B56 _ C5 C1: 72. F0, 07
	vpxor	xmm5, xmm5, xmm6			; 1B5B _ C5 D1: EF. EE
	vpxor	xmm2, xmm2, xmm0			; 1B5F _ C5 E9: EF. D0
	vpxor	xmm3, xmm5, xmm7			; 1B63 _ C5 D1: EF. DF
	vpxor	xmm2, xmm2, xmm6			; 1B67 _ C5 E9: EF. D6
	vmovd	xmm5, dword [ecx+2C14H] 		; 1B6B _ C5 F9: 6E. A9, 00002C14
	vpshufd xmm4, xmm5, 0				; 1B73 _ C5 F9: 70. E5, 00
	vmovd	xmm5, dword [ecx+2C1CH] 		; 1B78 _ C5 F9: 6E. A9, 00002C1C
	vpxor	xmm0, xmm0, xmm4			; 1B80 _ C5 F9: EF. C4
	vpshufd xmm5, xmm5, 0				; 1B84 _ C5 F9: 70. ED, 00
	vmovd	xmm7, dword [ecx+2C10H] 		; 1B89 _ C5 F9: 6E. B9, 00002C10
	vpxor	xmm5, xmm6, xmm5			; 1B91 _ C5 C9: EF. ED
	vpslld	xmm6, xmm2, 5				; 1B95 _ C5 C9: 72. F2, 05
	vpsrld	xmm2, xmm2, 27				; 1B9A _ C5 E9: 72. D2, 1B
	vpor	xmm6, xmm6, xmm2			; 1B9F _ C5 C9: EB. F2
	vpshufd xmm2, xmm7, 0				; 1BA3 _ C5 F9: 70. D7, 00
	vmovd	xmm4, dword [ecx+2C18H] 		; 1BA8 _ C5 F9: 6E. A1, 00002C18
	vpxor	xmm7, xmm6, xmm2			; 1BB0 _ C5 C9: EF. FA
	vpslld	xmm6, xmm3, 22				; 1BB4 _ C5 C9: 72. F3, 16
	vpsrld	xmm3, xmm3, 10				; 1BB9 _ C5 E1: 72. D3, 0A
	vpshufd xmm4, xmm4, 0				; 1BBE _ C5 F9: 70. E4, 00
	vpor	xmm3, xmm6, xmm3			; 1BC3 _ C5 C9: EB. DB
	vpxor	xmm2, xmm7, xmm1			; 1BC7 _ C5 C1: EF. D1
	vpxor	xmm6, xmm3, xmm4			; 1BCB _ C5 E1: EF. F4
	vpand	xmm7, xmm2, xmm0			; 1BCF _ C5 E9: DB. F8
	vpxor	xmm1, xmm6, xmm1			; 1BD3 _ C5 C9: EF. C9
	vpxor	xmm1, xmm1, xmm7			; 1BD7 _ C5 F1: EF. CF
	vpor	xmm4, xmm7, xmm5			; 1BDB _ C5 C1: EB. E5
	vpxor	xmm5, xmm5, xmm1			; 1BDF _ C5 D1: EF. E9
	vpxor	xmm3, xmm0, xmm4			; 1BE3 _ C5 F9: EF. DC
	vpxor	xmm6, xmm4, xmm2			; 1BE7 _ C5 D9: EF. F2
	vpor	xmm4, xmm2, xmm3			; 1BEB _ C5 E9: EB. E3
	vpxor	xmm0, xmm3, xmm5			; 1BEF _ C5 E1: EF. C5
	vpor	xmm2, xmm1, xmm6			; 1BF3 _ C5 F1: EB. D6
	vpand	xmm3, xmm2, xmm4			; 1BF7 _ C5 E9: DB. DC
	vpxor	xmm1, xmm6, xmm0			; 1BFB _ C5 C9: EF. C8
	vpslld	xmm2, xmm5, 3				; 1BFF _ C5 E9: 72. F5, 03
	vpsrld	xmm5, xmm5, 29				; 1C04 _ C5 D1: 72. D5, 1D
	vpslld	xmm7, xmm3, 13				; 1C09 _ C5 C1: 72. F3, 0D
	vpsrld	xmm6, xmm3, 19				; 1C0E _ C5 C9: 72. D3, 13
	vpor	xmm2, xmm2, xmm5			; 1C13 _ C5 E9: EB. D5
	vpand	xmm5, xmm1, xmm3			; 1C17 _ C5 F1: DB. EB
	vpor	xmm6, xmm7, xmm6			; 1C1B _ C5 C1: EB. F6
	vpxor	xmm4, xmm4, xmm5			; 1C1F _ C5 D9: EF. E5
	vpand	xmm0, xmm0, xmm3			; 1C23 _ C5 F9: DB. C3
	vpxor	xmm4, xmm4, xmm6			; 1C27 _ C5 D9: EF. E6
	vpxor	xmm1, xmm0, xmm1			; 1C2B _ C5 F9: EF. C9
	vpxor	xmm4, xmm4, xmm2			; 1C2F _ C5 D9: EF. E2
	vpxor	xmm3, xmm1, xmm2			; 1C33 _ C5 F1: EF. DA
	vpslld	xmm0, xmm6, 3				; 1C37 _ C5 F9: 72. F6, 03
	vpxor	xmm7, xmm3, xmm0			; 1C3C _ C5 E1: EF. F8
	vpslld	xmm5, xmm4, 1				; 1C40 _ C5 D1: 72. F4, 01
	vpsrld	xmm4, xmm4, 31				; 1C45 _ C5 D9: 72. D4, 1F
	vpslld	xmm1, xmm7, 7				; 1C4A _ C5 F1: 72. F7, 07
	vpor	xmm3, xmm5, xmm4			; 1C4F _ C5 D1: EB. DC
	vpsrld	xmm4, xmm7, 25				; 1C53 _ C5 D9: 72. D7, 19
	vpor	xmm7, xmm1, xmm4			; 1C58 _ C5 F1: EB. FC
	vpxor	xmm6, xmm6, xmm3			; 1C5C _ C5 C9: EF. F3
	vpxor	xmm6, xmm6, xmm7			; 1C60 _ C5 C9: EF. F7
	vpxor	xmm2, xmm2, xmm7			; 1C64 _ C5 E9: EF. D7
	vpslld	xmm1, xmm3, 7				; 1C68 _ C5 F1: 72. F3, 07
	vpslld	xmm4, xmm6, 5				; 1C6D _ C5 D9: 72. F6, 05
	vpxor	xmm5, xmm2, xmm1			; 1C72 _ C5 E9: EF. E9
	vpsrld	xmm2, xmm6, 27				; 1C76 _ C5 E9: 72. D6, 1B
	vmovd	xmm0, dword [ecx+2C20H] 		; 1C7B _ C5 F9: 6E. 81, 00002C20
	vpor	xmm1, xmm4, xmm2			; 1C83 _ C5 D9: EB. CA
	vmovd	xmm2, dword [ecx+2C24H] 		; 1C87 _ C5 F9: 6E. 91, 00002C24
	vpshufd xmm6, xmm0, 0				; 1C8F _ C5 F9: 70. F0, 00
	vpshufd xmm0, xmm2, 0				; 1C94 _ C5 F9: 70. C2, 00
	vpxor	xmm4, xmm1, xmm6			; 1C99 _ C5 F1: EF. E6
	vmovd	xmm1, dword [ecx+2C28H] 		; 1C9D _ C5 F9: 6E. 89, 00002C28
	vpxor	xmm2, xmm3, xmm0			; 1CA5 _ C5 E1: EF. D0
	vpslld	xmm3, xmm5, 22				; 1CA9 _ C5 E1: 72. F5, 16
	vpsrld	xmm5, xmm5, 10				; 1CAE _ C5 D1: 72. D5, 0A
	vmovd	xmm0, dword [ecx+2C2CH] 		; 1CB3 _ C5 F9: 6E. 81, 00002C2C
	vpor	xmm6, xmm3, xmm5			; 1CBB _ C5 E1: EB. F5
	vpshufd xmm3, xmm1, 0				; 1CBF _ C5 F9: 70. D9, 00
	vpshufd xmm5, xmm0, 0				; 1CC4 _ C5 F9: 70. E8, 00
	vpxor	xmm6, xmm6, xmm3			; 1CC9 _ C5 C9: EF. F3
	vpxor	xmm3, xmm7, xmm5			; 1CCD _ C5 C1: EF. DD
	vpand	xmm7, xmm4, xmm6			; 1CD1 _ C5 D9: DB. FE
	vpxor	xmm0, xmm7, xmm3			; 1CD5 _ C5 C1: EF. C3
	vpxor	xmm1, xmm6, xmm2			; 1CD9 _ C5 C9: EF. CA
	vpxor	xmm1, xmm1, xmm0			; 1CDD _ C5 F1: EF. C8
	vpor	xmm6, xmm3, xmm4			; 1CE1 _ C5 E1: EB. F4
	vpxor	xmm7, xmm6, xmm2			; 1CE5 _ C5 C9: EF. FA
	vpxor	xmm2, xmm4, xmm1			; 1CE9 _ C5 D9: EF. D1
	vpor	xmm4, xmm7, xmm2			; 1CED _ C5 C1: EB. E2
	vpand	xmm6, xmm0, xmm7			; 1CF1 _ C5 F9: DB. F7
	vpxor	xmm4, xmm4, xmm0			; 1CF5 _ C5 D9: EF. E0
	vpxor	xmm3, xmm2, xmm6			; 1CF9 _ C5 E9: EF. DE
	vpxor	xmm0, xmm7, xmm4			; 1CFD _ C5 C1: EF. C4
	vpslld	xmm5, xmm1, 13				; 1D01 _ C5 D1: 72. F1, 0D
	vpxor	xmm2, xmm0, xmm3			; 1D06 _ C5 F9: EF. D3
	vpsrld	xmm7, xmm1, 19				; 1D0A _ C5 C1: 72. D1, 13
	vpor	xmm7, xmm5, xmm7			; 1D0F _ C5 D1: EB. FF
	vpslld	xmm1, xmm2, 3				; 1D13 _ C5 F1: 72. F2, 03
	vpsrld	xmm6, xmm2, 29				; 1D18 _ C5 C9: 72. D2, 1D
	vpxor	xmm4, xmm4, xmm7			; 1D1D _ C5 D9: EF. E7
	vpor	xmm1, xmm1, xmm6			; 1D21 _ C5 F1: EB. CE
	vpslld	xmm2, xmm7, 3				; 1D25 _ C5 E9: 72. F7, 03
	vpxor	xmm0, xmm4, xmm1			; 1D2A _ C5 D9: EF. C1
	vpcmpeqd xmm4, xmm4, xmm4			; 1D2E _ C5 D9: 76. E4
	vpxor	xmm3, xmm3, xmm4			; 1D32 _ C5 E1: EF. DC
	vpxor	xmm6, xmm3, xmm1			; 1D36 _ C5 E1: EF. F1
	vpslld	xmm3, xmm0, 1				; 1D3A _ C5 E1: 72. F0, 01
	vpxor	xmm6, xmm6, xmm2			; 1D3F _ C5 C9: EF. F2
	vpsrld	xmm0, xmm0, 31				; 1D43 _ C5 F9: 72. D0, 1F
	vpor	xmm3, xmm3, xmm0			; 1D48 _ C5 E1: EB. D8
	vpslld	xmm5, xmm6, 7				; 1D4C _ C5 D1: 72. F6, 07
	vpsrld	xmm6, xmm6, 25				; 1D51 _ C5 C9: 72. D6, 19
	vpxor	xmm7, xmm7, xmm3			; 1D56 _ C5 C1: EF. FB
	vpor	xmm0, xmm5, xmm6			; 1D5A _ C5 D1: EB. C6
	vpslld	xmm6, xmm3, 7				; 1D5E _ C5 C9: 72. F3, 07
	vpxor	xmm2, xmm7, xmm0			; 1D63 _ C5 C1: EF. D0
	vpxor	xmm1, xmm1, xmm0			; 1D67 _ C5 F1: EF. C8
	vpslld	xmm7, xmm2, 5				; 1D6B _ C5 C1: 72. F2, 05
	vpsrld	xmm2, xmm2, 27				; 1D70 _ C5 E9: 72. D2, 1B
	vpxor	xmm5, xmm1, xmm6			; 1D75 _ C5 F1: EF. EE
	vpor	xmm6, xmm7, xmm2			; 1D79 _ C5 C1: EB. F2
	vmovd	xmm1, dword [ecx+2C30H] 		; 1D7D _ C5 F9: 6E. 89, 00002C30
	vmovd	xmm7, dword [ecx+2C34H] 		; 1D85 _ C5 F9: 6E. B9, 00002C34
	vpshufd xmm2, xmm1, 0				; 1D8D _ C5 F9: 70. D1, 00
	vpshufd xmm1, xmm7, 0				; 1D92 _ C5 F9: 70. CF, 00
	vpxor	xmm2, xmm6, xmm2			; 1D97 _ C5 C9: EF. D2
	vmovd	xmm7, dword [ecx+2C3CH] 		; 1D9B _ C5 F9: 6E. B9, 00002C3C
	vpxor	xmm3, xmm3, xmm1			; 1DA3 _ C5 E1: EF. D9
	vmovd	xmm1, dword [ecx+2C38H] 		; 1DA7 _ C5 F9: 6E. 89, 00002C38
	vpslld	xmm6, xmm5, 22				; 1DAF _ C5 C9: 72. F5, 16
	vpsrld	xmm5, xmm5, 10				; 1DB4 _ C5 D1: 72. D5, 0A
	vpor	xmm6, xmm6, xmm5			; 1DB9 _ C5 C9: EB. F5
	vpshufd xmm5, xmm1, 0				; 1DBD _ C5 F9: 70. E9, 00
	vpshufd xmm1, xmm7, 0				; 1DC2 _ C5 F9: 70. CF, 00
	vpxor	xmm5, xmm6, xmm5			; 1DC7 _ C5 C9: EF. ED
	vpxor	xmm0, xmm0, xmm1			; 1DCB _ C5 F9: EF. C1
	vpand	xmm1, xmm3, xmm2			; 1DCF _ C5 E1: DB. CA
	vpor	xmm7, xmm2, xmm0			; 1DD3 _ C5 E9: EB. F8
	vpxor	xmm6, xmm0, xmm3			; 1DD7 _ C5 F9: EF. F3
	vpxor	xmm2, xmm2, xmm5			; 1DDB _ C5 E9: EF. D5
	vpxor	xmm3, xmm5, xmm6			; 1DDF _ C5 D1: EF. DE
	vpor	xmm2, xmm2, xmm1			; 1DE3 _ C5 E9: EB. D1
	vpand	xmm6, xmm6, xmm7			; 1DE7 _ C5 C9: DB. F7
	vpxor	xmm6, xmm6, xmm2			; 1DEB _ C5 C9: EF. F2
	vpxor	xmm5, xmm7, xmm1			; 1DEF _ C5 C1: EF. E9
	vpxor	xmm1, xmm1, xmm6			; 1DF3 _ C5 F1: EF. CE
	vpand	xmm2, xmm2, xmm5			; 1DF7 _ C5 E9: DB. D5
	vpor	xmm0, xmm1, xmm5			; 1DFB _ C5 F1: EB. C5
	vpxor	xmm1, xmm5, xmm6			; 1DFF _ C5 D1: EF. CE
	vpxor	xmm7, xmm0, xmm3			; 1E03 _ C5 F9: EF. FB
	vpxor	xmm3, xmm2, xmm3			; 1E07 _ C5 E9: EF. DB
	vpor	xmm0, xmm7, xmm6			; 1E0B _ C5 C1: EB. C6
	vpxor	xmm1, xmm1, xmm0			; 1E0F _ C5 F1: EF. C8
	vpslld	xmm0, xmm1, 13				; 1E13 _ C5 F9: 72. F1, 0D
	vpsrld	xmm1, xmm1, 19				; 1E18 _ C5 F1: 72. D1, 13
	vpor	xmm0, xmm0, xmm1			; 1E1D _ C5 F9: EB. C1
	vpslld	xmm1, xmm6, 3				; 1E21 _ C5 F1: 72. F6, 03
	vpsrld	xmm6, xmm6, 29				; 1E26 _ C5 C9: 72. D6, 1D
	vpxor	xmm7, xmm7, xmm0			; 1E2B _ C5 C1: EF. F8
	vpor	xmm1, xmm1, xmm6			; 1E2F _ C5 F1: EB. CE
	vpxor	xmm6, xmm7, xmm1			; 1E33 _ C5 C1: EF. F1
	vpxor	xmm2, xmm3, xmm1			; 1E37 _ C5 E1: EF. D1
	vpslld	xmm3, xmm0, 3				; 1E3B _ C5 E1: 72. F0, 03
	vpslld	xmm5, xmm6, 1				; 1E40 _ C5 D1: 72. F6, 01
	vpxor	xmm2, xmm2, xmm3			; 1E45 _ C5 E9: EF. D3
	vpsrld	xmm6, xmm6, 31				; 1E49 _ C5 C9: 72. D6, 1F
	vpor	xmm3, xmm5, xmm6			; 1E4E _ C5 D1: EB. DE
	vpslld	xmm5, xmm2, 7				; 1E52 _ C5 D1: 72. F2, 07
	vpsrld	xmm2, xmm2, 25				; 1E57 _ C5 E9: 72. D2, 19
	vpxor	xmm0, xmm0, xmm3			; 1E5C _ C5 F9: EF. C3
	vpor	xmm2, xmm5, xmm2			; 1E60 _ C5 D1: EB. D2
	vpslld	xmm6, xmm3, 7				; 1E64 _ C5 C9: 72. F3, 07
	vpxor	xmm7, xmm0, xmm2			; 1E69 _ C5 F9: EF. FA
	vpxor	xmm1, xmm1, xmm2			; 1E6D _ C5 F1: EF. CA
	vpxor	xmm0, xmm1, xmm6			; 1E71 _ C5 F1: EF. C6
	vpslld	xmm1, xmm7, 5				; 1E75 _ C5 F1: 72. F7, 05
	vpsrld	xmm7, xmm7, 27				; 1E7A _ C5 C1: 72. D7, 1B
	vpor	xmm6, xmm1, xmm7			; 1E7F _ C5 F1: EB. F7
	vmovd	xmm1, dword [ecx+2C40H] 		; 1E83 _ C5 F9: 6E. 89, 00002C40
	vpshufd xmm5, xmm1, 0				; 1E8B _ C5 F9: 70. E9, 00
	vpxor	xmm7, xmm6, xmm5			; 1E90 _ C5 C9: EF. FD
	vmovd	xmm5, dword [ecx+2C4CH] 		; 1E94 _ C5 F9: 6E. A9, 00002C4C
	vmovd	xmm6, dword [ecx+2C44H] 		; 1E9C _ C5 F9: 6E. B1, 00002C44
	vpshufd xmm5, xmm5, 0				; 1EA4 _ C5 F9: 70. ED, 00
	vpxor	xmm5, xmm2, xmm5			; 1EA9 _ C5 E9: EF. ED
	vpshufd xmm2, xmm6, 0				; 1EAD _ C5 F9: 70. D6, 00
	vpxor	xmm6, xmm5, xmm4			; 1EB2 _ C5 D1: EF. F4
	vpxor	xmm3, xmm3, xmm2			; 1EB6 _ C5 E1: EF. DA
	vmovd	xmm1, dword [ecx+2C48H] 		; 1EBA _ C5 F9: 6E. 89, 00002C48
	vpxor	xmm2, xmm3, xmm5			; 1EC2 _ C5 E1: EF. D5
	vpslld	xmm3, xmm0, 22				; 1EC6 _ C5 E1: 72. F0, 16
	vpsrld	xmm0, xmm0, 10				; 1ECB _ C5 F9: 72. D0, 0A
	vpshufd xmm1, xmm1, 0				; 1ED0 _ C5 F9: 70. C9, 00
	vpor	xmm3, xmm3, xmm0			; 1ED5 _ C5 E1: EB. D8
	vpxor	xmm0, xmm3, xmm1			; 1ED9 _ C5 E1: EF. C1
	vpxor	xmm1, xmm6, xmm7			; 1EDD _ C5 C9: EF. CF
	vpxor	xmm0, xmm0, xmm6			; 1EE1 _ C5 F9: EF. C6
	vpand	xmm6, xmm2, xmm1			; 1EE5 _ C5 E9: DB. F1
	vpxor	xmm5, xmm2, xmm1			; 1EE9 _ C5 E9: EF. E9
	vpxor	xmm6, xmm6, xmm0			; 1EED _ C5 C9: EF. F0
	vpxor	xmm3, xmm7, xmm5			; 1EF1 _ C5 C1: EF. DD
	vpand	xmm7, xmm0, xmm5			; 1EF5 _ C5 F9: DB. FD
	vpand	xmm0, xmm3, xmm6			; 1EF9 _ C5 E1: DB. C6
	vpxor	xmm2, xmm7, xmm3			; 1EFD _ C5 C1: EF. D3
	vpxor	xmm7, xmm1, xmm0			; 1F01 _ C5 F1: EF. F8
	vpor	xmm5, xmm5, xmm6			; 1F05 _ C5 D1: EB. EE
	vpor	xmm1, xmm0, xmm7			; 1F09 _ C5 F9: EB. CF
	vpxor	xmm1, xmm1, xmm2			; 1F0D _ C5 F1: EF. CA
	vpand	xmm2, xmm2, xmm7			; 1F11 _ C5 E9: DB. D7
	vpxor	xmm3, xmm1, xmm4			; 1F15 _ C5 F1: EF. DC
	vpslld	xmm4, xmm6, 13				; 1F19 _ C5 D9: 72. F6, 0D
	vpsrld	xmm1, xmm6, 19				; 1F1E _ C5 F1: 72. D6, 13
	vpxor	xmm6, xmm5, xmm0			; 1F23 _ C5 D1: EF. F0
	vpor	xmm4, xmm4, xmm1			; 1F27 _ C5 D9: EB. E1
	vpslld	xmm1, xmm3, 3				; 1F2B _ C5 F1: 72. F3, 03
	vpsrld	xmm3, xmm3, 29				; 1F30 _ C5 E1: 72. D3, 1D
	vpxor	xmm6, xmm6, xmm2			; 1F35 _ C5 C9: EF. F2
	vpor	xmm1, xmm1, xmm3			; 1F39 _ C5 F1: EB. CB
	vpxor	xmm2, xmm6, xmm4			; 1F3D _ C5 C9: EF. D4
	vpxor	xmm3, xmm2, xmm1			; 1F41 _ C5 E9: EF. D9
	vpxor	xmm0, xmm7, xmm1			; 1F45 _ C5 C1: EF. C1
	vpslld	xmm7, xmm4, 3				; 1F49 _ C5 C1: 72. F4, 03
	vpslld	xmm6, xmm3, 1				; 1F4E _ C5 C9: 72. F3, 01
	vpxor	xmm0, xmm0, xmm7			; 1F53 _ C5 F9: EF. C7
	vpsrld	xmm2, xmm3, 31				; 1F57 _ C5 E9: 72. D3, 1F
	vpor	xmm5, xmm6, xmm2			; 1F5C _ C5 C9: EB. EA
	vpslld	xmm7, xmm0, 7				; 1F60 _ C5 C1: 72. F0, 07
	vpsrld	xmm6, xmm0, 25				; 1F65 _ C5 C9: 72. D0, 19
	vpxor	xmm4, xmm4, xmm5			; 1F6A _ C5 D9: EF. E5
	vpor	xmm0, xmm7, xmm6			; 1F6E _ C5 C1: EB. C6
	vpslld	xmm6, xmm5, 7				; 1F72 _ C5 C9: 72. F5, 07
	vpxor	xmm1, xmm1, xmm0			; 1F77 _ C5 F1: EF. C8
	vpxor	xmm4, xmm4, xmm0			; 1F7B _ C5 D9: EF. E0
	vpxor	xmm7, xmm1, xmm6			; 1F7F _ C5 F1: EF. FE
	vmovd	xmm1, dword [ecx+2C54H] 		; 1F83 _ C5 F9: 6E. 89, 00002C54
	vpshufd xmm2, xmm1, 0				; 1F8B _ C5 F9: 70. D1, 00
	vpxor	xmm1, xmm5, xmm2			; 1F90 _ C5 D1: EF. CA
	vmovd	xmm5, dword [ecx+2C5CH] 		; 1F94 _ C5 F9: 6E. A9, 00002C5C
	vpshufd xmm2, xmm5, 0				; 1F9C _ C5 F9: 70. D5, 00
	vmovd	xmm6, dword [ecx+2C50H] 		; 1FA1 _ C5 F9: 6E. B1, 00002C50
	vpxor	xmm2, xmm0, xmm2			; 1FA9 _ C5 F9: EF. D2
	vpslld	xmm0, xmm4, 5				; 1FAD _ C5 F9: 72. F4, 05
	vpsrld	xmm4, xmm4, 27				; 1FB2 _ C5 D9: 72. D4, 1B
	vpshufd xmm6, xmm6, 0				; 1FB7 _ C5 F9: 70. F6, 00
	vpor	xmm4, xmm0, xmm4			; 1FBC _ C5 F9: EB. E4
	vpxor	xmm0, xmm4, xmm6			; 1FC0 _ C5 D9: EF. C6
	vpxor	xmm5, xmm1, xmm2			; 1FC4 _ C5 F1: EF. EA
	vpxor	xmm0, xmm0, xmm1			; 1FC8 _ C5 F9: EF. C1
	vpcmpeqd xmm1, xmm1, xmm1			; 1FCC _ C5 F1: 76. C9
	vmovd	xmm3, dword [ecx+2C58H] 		; 1FD0 _ C5 F9: 6E. 99, 00002C58
	vpxor	xmm6, xmm2, xmm1			; 1FD8 _ C5 E9: EF. F1
	vpslld	xmm2, xmm7, 22				; 1FDC _ C5 E9: 72. F7, 16
	vpsrld	xmm7, xmm7, 10				; 1FE1 _ C5 C1: 72. D7, 0A
	vpshufd xmm3, xmm3, 0				; 1FE6 _ C5 F9: 70. DB, 00
	vpor	xmm4, xmm2, xmm7			; 1FEB _ C5 E9: EB. E7
	vpxor	xmm4, xmm4, xmm3			; 1FEF _ C5 D9: EF. E3
	vpand	xmm2, xmm5, xmm0			; 1FF3 _ C5 D1: DB. D0
	vpxor	xmm3, xmm4, xmm6			; 1FF7 _ C5 D9: EF. DE
	vpxor	xmm2, xmm2, xmm3			; 1FFB _ C5 E9: EF. D3
	vpor	xmm4, xmm3, xmm5			; 1FFF _ C5 E1: EB. E5
	vpand	xmm7, xmm6, xmm2			; 2003 _ C5 C9: DB. FA
	vpxor	xmm5, xmm5, xmm6			; 2007 _ C5 D1: EF. EE
	vpxor	xmm3, xmm7, xmm0			; 200B _ C5 C1: EF. D8
	vpxor	xmm6, xmm5, xmm2			; 200F _ C5 D1: EF. F2
	vpxor	xmm6, xmm6, xmm4			; 2013 _ C5 C9: EF. F4
	vpand	xmm5, xmm0, xmm3			; 2017 _ C5 F9: DB. EB
	vpxor	xmm5, xmm5, xmm6			; 201B _ C5 D1: EF. EE
	vpslld	xmm7, xmm2, 13				; 201F _ C5 C1: 72. F2, 0D
	vpsrld	xmm2, xmm2, 19				; 2024 _ C5 E9: 72. D2, 13
	vpxor	xmm0, xmm4, xmm0			; 2029 _ C5 D9: EF. C0
	vpor	xmm7, xmm7, xmm2			; 202D _ C5 C1: EB. FA
	vpslld	xmm2, xmm5, 3				; 2031 _ C5 E9: 72. F5, 03
	vpsrld	xmm5, xmm5, 29				; 2036 _ C5 D1: 72. D5, 1D
	vpor	xmm5, xmm2, xmm5			; 203B _ C5 E9: EB. ED
	vpxor	xmm2, xmm3, xmm7			; 203F _ C5 E1: EF. D7
	vpor	xmm3, xmm6, xmm3			; 2043 _ C5 C9: EB. DB
	vpxor	xmm6, xmm0, xmm1			; 2047 _ C5 F9: EF. F1
	vpxor	xmm4, xmm3, xmm6			; 204B _ C5 E1: EF. E6
	vpxor	xmm2, xmm2, xmm5			; 204F _ C5 E9: EF. D5
	vpxor	xmm3, xmm4, xmm5			; 2053 _ C5 D9: EF. DD
	vpslld	xmm0, xmm7, 3				; 2057 _ C5 F9: 72. F7, 03
	vpxor	xmm6, xmm3, xmm0			; 205C _ C5 E1: EF. F0
	vpslld	xmm4, xmm2, 1				; 2060 _ C5 D9: 72. F2, 01
	vpsrld	xmm2, xmm2, 31				; 2065 _ C5 E9: 72. D2, 1F
	vpslld	xmm3, xmm6, 7				; 206A _ C5 E1: 72. F6, 07
	vpor	xmm0, xmm4, xmm2			; 206F _ C5 D9: EB. C2
	vpsrld	xmm6, xmm6, 25				; 2073 _ C5 C9: 72. D6, 19
	vpor	xmm3, xmm3, xmm6			; 2078 _ C5 E1: EB. DE
	vpxor	xmm7, xmm7, xmm0			; 207C _ C5 C1: EF. F8
	vpxor	xmm7, xmm7, xmm3			; 2080 _ C5 C1: EF. FB
	vpxor	xmm5, xmm5, xmm3			; 2084 _ C5 D1: EF. EB
	vpslld	xmm6, xmm0, 7				; 2088 _ C5 C9: 72. F0, 07
	vpslld	xmm4, xmm7, 5				; 208D _ C5 D9: 72. F7, 05
	vpxor	xmm6, xmm5, xmm6			; 2092 _ C5 D1: EF. F6
	vpsrld	xmm2, xmm7, 27				; 2096 _ C5 E9: 72. D7, 1B
	vmovd	xmm7, dword [ecx+2C60H] 		; 209B _ C5 F9: 6E. B9, 00002C60
	vpor	xmm4, xmm4, xmm2			; 20A3 _ C5 D9: EB. E2
	vmovd	xmm5, dword [ecx+2C6CH] 		; 20A7 _ C5 F9: 6E. A9, 00002C6C
	vpshufd xmm2, xmm7, 0				; 20AF _ C5 F9: 70. D7, 00
	vpshufd xmm5, xmm5, 0				; 20B4 _ C5 F9: 70. ED, 00
	vpxor	xmm4, xmm4, xmm2			; 20B9 _ C5 D9: EF. E2
	vmovd	xmm2, dword [ecx+2C68H] 		; 20BD _ C5 F9: 6E. 91, 00002C68
	vpxor	xmm5, xmm3, xmm5			; 20C5 _ C5 E1: EF. ED
	vpslld	xmm3, xmm6, 22				; 20C9 _ C5 E1: 72. F6, 16
	vpsrld	xmm6, xmm6, 10				; 20CE _ C5 C9: 72. D6, 0A
	vpor	xmm3, xmm3, xmm6			; 20D3 _ C5 E1: EB. DE
	vpshufd xmm6, xmm2, 0				; 20D7 _ C5 F9: 70. F2, 00
	vmovd	xmm7, dword [ecx+2C64H] 		; 20DC _ C5 F9: 6E. B9, 00002C64
	vpxor	xmm2, xmm3, xmm6			; 20E4 _ C5 E1: EF. D6
	vpshufd xmm3, xmm7, 0				; 20E8 _ C5 F9: 70. DF, 00
	vpxor	xmm6, xmm2, xmm1			; 20ED _ C5 E9: EF. F1
	vpxor	xmm2, xmm4, xmm5			; 20F1 _ C5 D9: EF. D5
	vpand	xmm4, xmm5, xmm4			; 20F5 _ C5 D1: DB. E4
	vpxor	xmm4, xmm4, xmm6			; 20F9 _ C5 D9: EF. E6
	vpxor	xmm0, xmm0, xmm3			; 20FD _ C5 F9: EF. C3
	vpxor	xmm0, xmm0, xmm4			; 2101 _ C5 F9: EF. C4
	vpor	xmm7, xmm6, xmm5			; 2105 _ C5 C9: EB. FD
	vpor	xmm3, xmm2, xmm0			; 2109 _ C5 E9: EB. D8
	vpxor	xmm6, xmm7, xmm2			; 210D _ C5 C1: EF. F2
	vpxor	xmm7, xmm6, xmm0			; 2111 _ C5 C9: EF. F8
	vpor	xmm2, xmm3, xmm4			; 2115 _ C5 E1: EB. D4
	vpxor	xmm5, xmm5, xmm3			; 2119 _ C5 D1: EF. EB
	vpxor	xmm2, xmm2, xmm7			; 211D _ C5 E9: EF. D7
	vpxor	xmm6, xmm5, xmm4			; 2121 _ C5 D1: EF. F4
	vpslld	xmm3, xmm2, 13				; 2125 _ C5 E1: 72. F2, 0D
	vpxor	xmm5, xmm6, xmm2			; 212A _ C5 C9: EF. EA
	vpsrld	xmm6, xmm2, 19				; 212E _ C5 C9: 72. D2, 13
	vpor	xmm2, xmm3, xmm6			; 2133 _ C5 E1: EB. D6
	vpslld	xmm3, xmm5, 3				; 2137 _ C5 E1: 72. F5, 03
	vpsrld	xmm6, xmm5, 29				; 213C _ C5 C9: 72. D5, 1D
	vpxor	xmm4, xmm4, xmm1			; 2141 _ C5 D9: EF. E1
	vpand	xmm5, xmm7, xmm5			; 2145 _ C5 C1: DB. ED
	vpor	xmm6, xmm3, xmm6			; 2149 _ C5 E1: EB. F6
	vpxor	xmm0, xmm0, xmm2			; 214D _ C5 F9: EF. C2
	vpxor	xmm7, xmm4, xmm5			; 2151 _ C5 D9: EF. FD
	vpxor	xmm0, xmm0, xmm6			; 2155 _ C5 F9: EF. C6
	vpxor	xmm4, xmm7, xmm6			; 2159 _ C5 C1: EF. E6
	vpslld	xmm3, xmm2, 3				; 215D _ C5 E1: 72. F2, 03
	vpslld	xmm5, xmm0, 1				; 2162 _ C5 D1: 72. F0, 01
	vpxor	xmm4, xmm4, xmm3			; 2167 _ C5 D9: EF. E3
	vpsrld	xmm0, xmm0, 31				; 216B _ C5 F9: 72. D0, 1F
	vpor	xmm7, xmm5, xmm0			; 2170 _ C5 D1: EB. F8
	vpslld	xmm3, xmm4, 7				; 2174 _ C5 E1: 72. F4, 07
	vpsrld	xmm4, xmm4, 25				; 2179 _ C5 D9: 72. D4, 19
	vpxor	xmm2, xmm2, xmm7			; 217E _ C5 E9: EF. D7
	vpor	xmm0, xmm3, xmm4			; 2182 _ C5 E1: EB. C4
	vpslld	xmm4, xmm7, 7				; 2186 _ C5 D9: 72. F7, 07
	vpxor	xmm5, xmm2, xmm0			; 218B _ C5 E9: EF. E8
	vpxor	xmm6, xmm6, xmm0			; 218F _ C5 C9: EF. F0
	vpslld	xmm2, xmm5, 5				; 2193 _ C5 E9: 72. F5, 05
	vpsrld	xmm3, xmm5, 27				; 2198 _ C5 E1: 72. D5, 1B
	vmovd	xmm5, dword [ecx+2C70H] 		; 219D _ C5 F9: 6E. A9, 00002C70
	vpxor	xmm6, xmm6, xmm4			; 21A5 _ C5 C9: EF. F4
	vpor	xmm4, xmm2, xmm3			; 21A9 _ C5 E9: EB. E3
	vmovd	xmm3, dword [ecx+2C74H] 		; 21AD _ C5 F9: 6E. 99, 00002C74
	vpshufd xmm2, xmm5, 0				; 21B5 _ C5 F9: 70. D5, 00
	vpxor	xmm5, xmm4, xmm2			; 21BA _ C5 D9: EF. EA
	vpslld	xmm2, xmm6, 22				; 21BE _ C5 E9: 72. F6, 16
	vpshufd xmm4, xmm3, 0				; 21C3 _ C5 F9: 70. E3, 00
	vpsrld	xmm6, xmm6, 10				; 21C8 _ C5 C9: 72. D6, 0A
	vpxor	xmm7, xmm7, xmm4			; 21CD _ C5 C1: EF. FC
	vpor	xmm4, xmm2, xmm6			; 21D1 _ C5 E9: EB. E6
	vmovd	xmm6, dword [ecx+2C78H] 		; 21D5 _ C5 F9: 6E. B1, 00002C78
	vmovd	xmm3, dword [ecx+2C7CH] 		; 21DD _ C5 F9: 6E. 99, 00002C7C
	vpshufd xmm2, xmm6, 0				; 21E5 _ C5 F9: 70. D6, 00
	vpshufd xmm6, xmm3, 0				; 21EA _ C5 F9: 70. F3, 00
	vpxor	xmm4, xmm4, xmm2			; 21EF _ C5 D9: EF. E2
	vpxor	xmm6, xmm0, xmm6			; 21F3 _ C5 F9: EF. F6
	vpor	xmm0, xmm7, xmm4			; 21F7 _ C5 C1: EB. C4
	vpxor	xmm2, xmm0, xmm6			; 21FB _ C5 F9: EF. D6
	vpxor	xmm7, xmm7, xmm4			; 21FF _ C5 C1: EF. FC
	vpxor	xmm3, xmm4, xmm2			; 2203 _ C5 D9: EF. DA
	vpxor	xmm4, xmm7, xmm3			; 2207 _ C5 C1: EF. E3
	vpor	xmm7, xmm6, xmm7			; 220B _ C5 C9: EB. FF
	vmovdqu oword [esp+0E0H], xmm2			; 220F _ C5 FA: 7F. 94 24, 000000E0
	vpor	xmm2, xmm2, xmm4			; 2218 _ C5 E9: EB. D4
	vpxor	xmm2, xmm2, xmm5			; 221C _ C5 E9: EF. D5
	vpor	xmm0, xmm5, xmm4			; 2220 _ C5 D1: EB. C4
	vpxor	xmm2, xmm2, xmm4			; 2224 _ C5 E9: EF. D4
	vpxor	xmm0, xmm0, xmm3			; 2228 _ C5 F9: EF. C3
	vpxor	xmm3, xmm3, xmm2			; 222C _ C5 E1: EF. DA
	vpand	xmm6, xmm7, xmm5			; 2230 _ C5 C1: DB. F5
	vpxor	xmm1, xmm3, xmm1			; 2234 _ C5 E1: EF. C9
	vpand	xmm2, xmm2, xmm0			; 2238 _ C5 E9: DB. D0
	vpor	xmm1, xmm1, xmm0			; 223C _ C5 F1: EB. C8
	vpxor	xmm2, xmm2, xmm4			; 2240 _ C5 E9: EF. D4
	vpxor	xmm3, xmm4, xmm1			; 2244 _ C5 D9: EF. D9
	vmovd	xmm1, dword [ecx+2C80H] 		; 2248 _ C5 F9: 6E. 89, 00002C80
	vpshufd xmm1, xmm1, 0				; 2250 _ C5 F9: 70. C9, 00
	vpxor	xmm1, xmm3, xmm1			; 2255 _ C5 E1: EF. C9
	vmovd	xmm3, dword [ecx+2C84H] 		; 2259 _ C5 F9: 6E. 99, 00002C84
	vmovd	xmm4, dword [ecx+2C88H] 		; 2261 _ C5 F9: 6E. A1, 00002C88
	vpxor	xmm5, xmm6, oword [esp+0E0H]		; 2269 _ C5 C9: EF. AC 24, 000000E0
	vpshufd xmm7, xmm3, 0				; 2272 _ C5 F9: 70. FB, 00
	vmovd	xmm3, dword [ecx+2C8CH] 		; 2277 _ C5 F9: 6E. 99, 00002C8C
	vpxor	xmm6, xmm5, xmm7			; 227F _ C5 D1: EF. F7
	vpshufd xmm4, xmm4, 0				; 2283 _ C5 F9: 70. E4, 00
	vpshufd xmm5, xmm3, 0				; 2288 _ C5 F9: 70. EB, 00
	vpxor	xmm4, xmm2, xmm4			; 228D _ C5 E9: EF. E4
	vpxor	xmm0, xmm0, xmm5			; 2291 _ C5 F9: EF. C5
	vpunpckldq xmm2, xmm1, xmm6			; 2295 _ C5 F1: 62. D6
	vpunpckldq xmm3, xmm4, xmm0			; 2299 _ C5 D9: 62. D8
	vpunpckhdq xmm1, xmm1, xmm6			; 229D _ C5 F1: 6A. CE
	vpunpckhdq xmm0, xmm4, xmm0			; 22A1 _ C5 D9: 6A. C0
	vpunpcklqdq xmm4, xmm2, xmm3			; 22A5 _ C5 E9: 6C. E3
	inc	eax					; 22A9 _ 40
	vpxor	xmm5, xmm4, oword [esp+70H]		; 22AA _ C5 D9: EF. 6C 24, 70
	add	esi, 64 				; 22B0 _ 83. C6, 40
	vmovdqu xmm4, oword [esp+60H]			; 22B3 _ C5 FA: 6F. 64 24, 60
	vpunpcklqdq xmm6, xmm1, xmm0			; 22B9 _ C5 F1: 6C. F0
	vpunpckhqdq xmm0, xmm1, xmm0			; 22BD _ C5 F1: 6D. C0
	vpunpckhqdq xmm2, xmm2, xmm3			; 22C1 _ C5 E9: 6D. D3
	vpxor	xmm1, xmm0, xmm4			; 22C5 _ C5 F9: EF. CC
	vpslldq xmm0, xmm4, 8				; 22C9 _ C5 F9: 73. FC, 08
	vpxor	xmm3, xmm2, oword [esp+50H]		; 22CE _ C5 E9: EF. 5C 24, 50
	vpsllq	xmm2, xmm4, 1				; 22D4 _ C5 E9: 73. F4, 01
	vmovdqu oword [edx], xmm5			; 22D9 _ C5 FA: 7F. 2A
	vmovdqu oword [edx+30H], xmm1			; 22DD _ C5 FA: 7F. 4A, 30
	vmovdqu oword [edx+10H], xmm3			; 22E2 _ C5 FA: 7F. 5A, 10
	vpsrldq xmm1, xmm0, 7				; 22E7 _ C5 F1: 73. D8, 07
	vpsraw	xmm5, xmm4, 8				; 22EC _ C5 D1: 71. E4, 08
	vpxor	xmm7, xmm6, oword [esp+40H]		; 22F1 _ C5 C9: EF. 7C 24, 40
	vpsrlq	xmm3, xmm1, 7				; 22F7 _ C5 E1: 73. D1, 07
	vpsrldq xmm6, xmm5, 15				; 22FC _ C5 C9: 73. DD, 0F
	vpand	xmm0, xmm6, oword [esp+30H]		; 2301 _ C5 C9: DB. 44 24, 30
	vmovdqu oword [edx+20H], xmm7			; 2307 _ C5 FA: 7F. 7A, 20
	vpor	xmm7, xmm2, xmm3			; 230C _ C5 E9: EB. FB
	add	edx, 64 				; 2310 _ 83. C2, 40
	vpxor	xmm1, xmm7, xmm0			; 2313 _ C5 C1: EF. C8
	cmp	eax, 8					; 2317 _ 83. F8, 08
	vmovdqu oword [esp+70H], xmm1			; 231A _ C5 FA: 7F. 4C 24, 70
	jl	?_003					; 2320 _ 0F 8C, FFFFDD74
	mov	dword [esp+28H], esi			; 2326 _ 89. 74 24, 28
	mov	esi, dword [esp+20H]			; 232A _ 8B. 74 24, 20
	vmovdqu xmm0, oword [esp+70H]			; 232E _ C5 FA: 6F. 44 24, 70
	add	esi, -512				; 2334 _ 81. C6, FFFFFE00
	mov	dword [esp+24H], edx			; 233A _ 89. 54 24, 24
	jne	?_001					; 233E _ 0F 85, FFFFDD1E
	add	esp, 244				; 2344 _ 81. C4, 000000F4
	pop	ebx					; 234A _ 5B
	pop	edi					; 234B _ 5F
	pop	esi					; 234C _ 5E
	mov	esp, ebp				; 234D _ 8B. E5
	pop	ebp					; 234F _ 5D
	ret	24					; 2350 _ C2, 0018
; _xts_serpent_avx_encrypt@24 End of function

; Filling space: 0DH
; Filler type: Multi-byte NOP
;       db 0FH, 1FH, 44H, 00H, 00H, 0FH, 1FH, 84H
;       db 00H, 00H, 00H, 00H, 00H

ALIGN	16

_xts_serpent_avx_decrypt@24:; Function begin
	push	ebp					; 0000 _ 55
	mov	ebp, esp				; 0001 _ 8B. EC
	and	esp, 0FFFFFFF0H 			; 0003 _ 83. E4, F0
	push	esi					; 0006 _ 56
	push	edi					; 0007 _ 57
	push	ebx					; 0008 _ 53
	sub	esp, 132				; 0009 _ 81. EC, 00000084
	mov	esi, dword [ebp+18H]			; 000F _ 8B. 75, 18
	mov	ecx, esi				; 0012 _ 8B. CE
	mov	edi, dword [ebp+14H]			; 0014 _ 8B. 7D, 14
	shl	ecx, 23 				; 0017 _ C1. E1, 17
	shr	edi, 9					; 001A _ C1. EF, 09
	or	ecx, edi				; 001D _ 0B. CF
	mov	edi, 135				; 001F _ BF, 00000087
	mov	ebx, dword [ebp+1CH]			; 0024 _ 8B. 5D, 1C
	mov	edx, dword [ebp+8H]			; 0027 _ 8B. 55, 08
	mov	eax, dword [ebp+0CH]			; 002A _ 8B. 45, 0C
	vmovd	xmm1, edi				; 002D _ C5 F9: 6E. CF
	shr	esi, 9					; 0031 _ C1. EE, 09
	lea	ebx, [ebx+5710H]			; 0034 _ 8D. 9B, 00005710
	mov	dword [esp], ecx			; 003A _ 89. 0C 24
	xor	ecx, ecx				; 003D _ 33. C9
	mov	dword [esp+4H], esi			; 003F _ 89. 74 24, 04
	lea	edi, [esp+10H]				; 0043 _ 8D. 7C 24, 10
	mov	dword [esp+8H], ecx			; 0047 _ 89. 4C 24, 08
	mov	dword [esp+0CH], ecx			; 004B _ 89. 4C 24, 0C
	mov	esi, dword [ebp+10H]			; 004F _ 8B. 75, 10
	vmovdqu oword [esp+30H], xmm1			; 0052 _ C5 FA: 7F. 4C 24, 30
	mov	dword [esp+24H], eax			; 0058 _ 89. 44 24, 24
	mov	dword [esp+28H], edx			; 005C _ 89. 54 24, 28
	jmp	?_005					; 0060 _ EB, 06

?_004:	vmovdqu oword [esp+10H], xmm0			; 0062 _ C5 FA: 7F. 44 24, 10
?_005:	add	dword [esp], 1				; 0068 _ 83. 04 24, 01
	adc	dword [esp+4H], 0			; 006C _ 83. 54 24, 04, 00
	push	ebx					; 0071 _ 53
	push	edi					; 0072 _ 57
	lea	eax, [esp+8H]				; 0073 _ 8D. 44 24, 08
	push	eax					; 0077 _ 50
	call	_serpent256_encrypt@12			; 0078 _ E8, 00000000(rel)
	vmovdqu xmm0, oword [esp+10H]			; 007D _ C5 FA: 6F. 44 24, 10
	xor	eax, eax				; 0083 _ 33. C0
	mov	dword [esp+20H], esi			; 0085 _ 89. 74 24, 20
	vmovdqu oword [esp+70H], xmm0			; 0089 _ C5 FA: 7F. 44 24, 70
	mov	edx, dword [esp+24H]			; 008F _ 8B. 54 24, 24
	mov	esi, dword [esp+28H]			; 0093 _ 8B. 74 24, 28
	mov	ecx, dword [ebp+1CH]			; 0097 _ 8B. 4D, 1C
?_006:	vmovdqu xmm1, oword [esp+70H]			; 009A _ C5 FA: 6F. 4C 24, 70
	vpslldq xmm4, xmm1, 8				; 00A0 _ C5 D9: 73. F9, 08
	vpsraw	xmm7, xmm1, 8				; 00A5 _ C5 C1: 71. E1, 08
	vpsrldq xmm5, xmm4, 7				; 00AA _ C5 D1: 73. DC, 07
	vpsllq	xmm2, xmm1, 1				; 00AF _ C5 E9: 73. F1, 01
	vmovdqu xmm3, oword [esp+30H]			; 00B4 _ C5 FA: 6F. 5C 24, 30
	vpsrlq	xmm6, xmm5, 7				; 00BA _ C5 C9: 73. D5, 07
	vpsrldq xmm0, xmm7, 15				; 00BF _ C5 F9: 73. DF, 0F
	vpor	xmm4, xmm2, xmm6			; 00C4 _ C5 E9: EB. E6
	vpand	xmm5, xmm0, xmm3			; 00C8 _ C5 F9: DB. EB
	vpxor	xmm2, xmm4, xmm5			; 00CC _ C5 D9: EF. D5
	vpslldq xmm6, xmm2, 8				; 00D0 _ C5 C9: 73. FA, 08
	vpsraw	xmm5, xmm2, 8				; 00D5 _ C5 D1: 71. E2, 08
	vpsrldq xmm7, xmm6, 7				; 00DA _ C5 C1: 73. DE, 07
	vpsllq	xmm0, xmm2, 1				; 00DF _ C5 F9: 73. F2, 01
	vpsrldq xmm6, xmm5, 15				; 00E4 _ C5 C9: 73. DD, 0F
	vpsrlq	xmm4, xmm7, 7				; 00E9 _ C5 D9: 73. D7, 07
	vpor	xmm7, xmm0, xmm4			; 00EE _ C5 F9: EB. FC
	vpand	xmm0, xmm6, xmm3			; 00F2 _ C5 C9: DB. C3
	vpxor	xmm5, xmm7, xmm0			; 00F6 _ C5 C1: EF. E8
	vpslldq xmm4, xmm5, 8				; 00FA _ C5 D9: 73. FD, 08
	vpsllq	xmm7, xmm5, 1				; 00FF _ C5 C1: 73. F5, 01
	vpsrldq xmm6, xmm4, 7				; 0104 _ C5 C9: 73. DC, 07
	vpsraw	xmm4, xmm5, 8				; 0109 _ C5 D9: 71. E5, 08
	vpsrlq	xmm0, xmm6, 7				; 010E _ C5 F9: 73. D6, 07
	vpsrldq xmm6, xmm4, 15				; 0113 _ C5 C9: 73. DC, 0F
	vpor	xmm7, xmm7, xmm0			; 0118 _ C5 C1: EB. F8
	vpand	xmm3, xmm6, xmm3			; 011C _ C5 C9: DB. DB
	vpxor	xmm0, xmm7, xmm3			; 0120 _ C5 C1: EF. C3
	vpxor	xmm3, xmm1, oword [esi] 		; 0124 _ C5 F1: EF. 1E
	vpxor	xmm1, xmm2, oword [esi+10H]		; 0128 _ C5 E9: EF. 4E, 10
	vmovdqu oword [esp+50H], xmm2			; 012D _ C5 FA: 7F. 54 24, 50
	vmovdqu oword [esp+40H], xmm5			; 0133 _ C5 FA: 7F. 6C 24, 40
	vpxor	xmm5, xmm5, oword [esi+20H]		; 0139 _ C5 D1: EF. 6E, 20
	vpxor	xmm2, xmm0, oword [esi+30H]		; 013E _ C5 F9: EF. 56, 30
	vmovdqu oword [esp+60H], xmm0			; 0143 _ C5 FA: 7F. 44 24, 60
	vpunpckldq xmm6, xmm3, xmm1			; 0149 _ C5 E1: 62. F1
	vpunpckldq xmm7, xmm5, xmm2			; 014D _ C5 D1: 62. FA
	vpunpckhdq xmm0, xmm3, xmm1			; 0151 _ C5 E1: 6A. C1
	vmovd	xmm3, dword [ecx+2C80H] 		; 0155 _ C5 F9: 6E. 99, 00002C80
	vpunpckhdq xmm5, xmm5, xmm2			; 015D _ C5 D1: 6A. EA
	vpunpcklqdq xmm1, xmm6, xmm7			; 0161 _ C5 C9: 6C. CF
	vpshufd xmm4, xmm3, 0				; 0165 _ C5 F9: 70. E3, 00
	vpunpckhqdq xmm7, xmm6, xmm7			; 016A _ C5 C9: 6D. FF
	vpxor	xmm3, xmm1, xmm4			; 016E _ C5 F1: EF. DC
	vmovd	xmm6, dword [ecx+2C84H] 		; 0172 _ C5 F9: 6E. B1, 00002C84
	vpshufd xmm2, xmm6, 0				; 017A _ C5 F9: 70. D6, 00
	vmovd	xmm6, dword [ecx+2C88H] 		; 017F _ C5 F9: 6E. B1, 00002C88
	vpxor	xmm7, xmm7, xmm2			; 0187 _ C5 C1: EF. FA
	vpunpcklqdq xmm1, xmm0, xmm5			; 018B _ C5 F9: 6C. CD
	vpunpckhqdq xmm0, xmm0, xmm5			; 018F _ C5 F9: 6D. C5
	vmovd	xmm5, dword [ecx+2C8CH] 		; 0193 _ C5 F9: 6E. A9, 00002C8C
	vpshufd xmm4, xmm6, 0				; 019B _ C5 F9: 70. E6, 00
	vpshufd xmm5, xmm5, 0				; 01A0 _ C5 F9: 70. ED, 00
	vpxor	xmm1, xmm1, xmm4			; 01A5 _ C5 F1: EF. CC
	vpxor	xmm2, xmm0, xmm5			; 01A9 _ C5 F9: EF. D5
	vpxor	xmm6, xmm1, xmm3			; 01AD _ C5 F1: EF. F3
	vpand	xmm4, xmm3, xmm2			; 01B1 _ C5 E1: DB. E2
	vpor	xmm0, xmm1, xmm2			; 01B5 _ C5 F1: EB. C2
	vpcmpeqd xmm3, xmm3, xmm3			; 01B9 _ C5 E1: 76. DB
	vpxor	xmm2, xmm2, xmm7			; 01BD _ C5 E9: EF. D7
	vpxor	xmm5, xmm6, xmm3			; 01C1 _ C5 C9: EF. EB
	vpor	xmm7, xmm7, xmm4			; 01C5 _ C5 C1: EB. FC
	vpxor	xmm1, xmm4, xmm5			; 01C9 _ C5 D9: EF. CD
	vpand	xmm6, xmm5, xmm0			; 01CD _ C5 D1: DB. F0
	vpxor	xmm5, xmm7, xmm6			; 01D1 _ C5 C1: EF. EE
	vpxor	xmm6, xmm6, xmm1			; 01D5 _ C5 C9: EF. F1
	vpand	xmm2, xmm2, xmm0			; 01D9 _ C5 E9: DB. D0
	vpxor	xmm0, xmm0, xmm5			; 01DD _ C5 F9: EF. C5
	vpor	xmm7, xmm1, xmm6			; 01E1 _ C5 F1: EB. FE
	vmovd	xmm4, dword [ecx+2C70H] 		; 01E5 _ C5 F9: 6E. A1, 00002C70
	vpxor	xmm1, xmm7, xmm2			; 01ED _ C5 C1: EF. CA
	vpxor	xmm2, xmm2, xmm0			; 01F1 _ C5 E9: EF. D0
	vpor	xmm0, xmm0, xmm1			; 01F5 _ C5 F9: EB. C1
	vpshufd xmm7, xmm4, 0				; 01F9 _ C5 F9: 70. FC, 00
	vpxor	xmm2, xmm2, xmm6			; 01FE _ C5 E9: EF. D6
	vmovd	xmm4, dword [ecx+2C74H] 		; 0202 _ C5 F9: 6E. A1, 00002C74
	vpxor	xmm7, xmm2, xmm7			; 020A _ C5 E9: EF. FF
	vpshufd xmm2, xmm4, 0				; 020E _ C5 F9: 70. D4, 00
	vpxor	xmm4, xmm1, xmm2			; 0213 _ C5 F1: EF. E2
	vmovd	xmm2, dword [ecx+2C78H] 		; 0217 _ C5 F9: 6E. 91, 00002C78
	vpshufd xmm2, xmm2, 0				; 021F _ C5 F9: 70. D2, 00
	vmovd	xmm1, dword [ecx+2C7CH] 		; 0224 _ C5 F9: 6E. 89, 00002C7C
	vpxor	xmm2, xmm5, xmm2			; 022C _ C5 D1: EF. D2
	vpxor	xmm5, xmm0, xmm6			; 0230 _ C5 F9: EF. EE
	vpshufd xmm6, xmm1, 0				; 0234 _ C5 F9: 70. F1, 00
	vpslld	xmm1, xmm2, 10				; 0239 _ C5 F1: 72. F2, 0A
	vpsrld	xmm2, xmm2, 22				; 023E _ C5 E9: 72. D2, 16
	vpxor	xmm0, xmm5, xmm6			; 0243 _ C5 D1: EF. C6
	vpor	xmm5, xmm1, xmm2			; 0247 _ C5 F1: EB. EA
	vpslld	xmm6, xmm4, 7				; 024B _ C5 C9: 72. F4, 07
	vpxor	xmm2, xmm5, xmm0			; 0250 _ C5 D1: EF. D0
	vpslld	xmm1, xmm4, 31				; 0254 _ C5 F1: 72. F4, 1F
	vpxor	xmm5, xmm2, xmm6			; 0259 _ C5 E9: EF. EE
	vpslld	xmm2, xmm7, 27				; 025D _ C5 E9: 72. F7, 1B
	vpsrld	xmm7, xmm7, 5				; 0262 _ C5 C1: 72. D7, 05
	vpor	xmm2, xmm2, xmm7			; 0267 _ C5 E9: EB. D7
	vpslld	xmm7, xmm0, 25				; 026B _ C5 C1: 72. F0, 19
	vpxor	xmm6, xmm2, xmm4			; 0270 _ C5 E9: EF. F4
	vpsrld	xmm4, xmm4, 1				; 0274 _ C5 D9: 72. D4, 01
	vpxor	xmm6, xmm6, xmm0			; 0279 _ C5 C9: EF. F0
	vpsrld	xmm0, xmm0, 7				; 027D _ C5 F9: 72. D0, 07
	vpor	xmm2, xmm7, xmm0			; 0282 _ C5 C1: EB. D0
	vpslld	xmm0, xmm6, 3				; 0286 _ C5 F9: 72. F6, 03
	vpxor	xmm7, xmm2, xmm5			; 028B _ C5 E9: EF. FD
	vpor	xmm2, xmm1, xmm4			; 028F _ C5 F1: EB. D4
	vpxor	xmm0, xmm7, xmm0			; 0293 _ C5 C1: EF. C0
	vpxor	xmm7, xmm2, xmm6			; 0297 _ C5 E9: EF. FE
	vpxor	xmm1, xmm7, xmm5			; 029B _ C5 C1: EF. CD
	vpslld	xmm4, xmm5, 29				; 029F _ C5 D9: 72. F5, 1D
	vpsrld	xmm5, xmm5, 3				; 02A4 _ C5 D1: 72. D5, 03
	vpslld	xmm2, xmm6, 19				; 02A9 _ C5 E9: 72. F6, 13
	vpsrld	xmm6, xmm6, 13				; 02AE _ C5 C9: 72. D6, 0D
	vpor	xmm5, xmm4, xmm5			; 02B3 _ C5 D9: EB. ED
	vpor	xmm2, xmm2, xmm6			; 02B7 _ C5 E9: EB. D6
	vpxor	xmm6, xmm0, xmm1			; 02BB _ C5 F9: EF. F1
	vpxor	xmm4, xmm2, xmm5			; 02BF _ C5 E9: EF. E5
	vpand	xmm7, xmm5, xmm4			; 02C3 _ C5 D1: DB. FC
	vpxor	xmm5, xmm5, xmm0			; 02C7 _ C5 D1: EF. E8
	vpxor	xmm3, xmm7, xmm3			; 02CB _ C5 C1: EF. DB
	vpor	xmm7, xmm5, xmm4			; 02CF _ C5 D1: EB. FC
	vpxor	xmm2, xmm3, xmm6			; 02D3 _ C5 E1: EF. D6
	vpxor	xmm3, xmm6, xmm7			; 02D7 _ C5 C9: EF. DF
	vmovd	xmm6, dword [ecx+2C60H] 		; 02DB _ C5 F9: 6E. B1, 00002C60
	vpxor	xmm5, xmm4, xmm2			; 02E3 _ C5 D9: EF. EA
	vpand	xmm0, xmm1, xmm3			; 02E7 _ C5 F1: DB. C3
	vpxor	xmm7, xmm7, xmm1			; 02EB _ C5 C1: EF. F9
	vpshufd xmm4, xmm6, 0				; 02EF _ C5 F9: 70. E6, 00
	vpxor	xmm0, xmm0, xmm5			; 02F4 _ C5 F9: EF. C5
	vpxor	xmm6, xmm0, xmm4			; 02F8 _ C5 F9: EF. F4
	vpxor	xmm1, xmm5, xmm3			; 02FC _ C5 D1: EF. CB
	vmovd	xmm4, dword [ecx+2C64H] 		; 0300 _ C5 F9: 6E. A1, 00002C64
	vpor	xmm5, xmm1, xmm2			; 0308 _ C5 F1: EB. EA
	vpshufd xmm4, xmm4, 0				; 030C _ C5 F9: 70. E4, 00
	vpxor	xmm7, xmm7, xmm5			; 0311 _ C5 C1: EF. FD
	vpxor	xmm4, xmm2, xmm4			; 0315 _ C5 E9: EF. E4
	vpxor	xmm5, xmm3, xmm0			; 0319 _ C5 E1: EF. E8
	vmovd	xmm2, dword [ecx+2C68H] 		; 031D _ C5 F9: 6E. 91, 00002C68
	vpshufd xmm1, xmm2, 0				; 0325 _ C5 F9: 70. CA, 00
	vmovd	xmm3, dword [ecx+2C6CH] 		; 032A _ C5 F9: 6E. 99, 00002C6C
	vpxor	xmm1, xmm7, xmm1			; 0332 _ C5 C1: EF. C9
	vpshufd xmm2, xmm3, 0				; 0336 _ C5 F9: 70. D3, 00
	vpslld	xmm3, xmm1, 10				; 033B _ C5 E1: 72. F1, 0A
	vpsrld	xmm0, xmm1, 22				; 0340 _ C5 F9: 72. D1, 16
	vpxor	xmm7, xmm5, xmm2			; 0345 _ C5 D1: EF. FA
	vpor	xmm1, xmm3, xmm0			; 0349 _ C5 E1: EB. C8
	vpslld	xmm2, xmm4, 7				; 034D _ C5 E9: 72. F4, 07
	vpxor	xmm5, xmm1, xmm7			; 0352 _ C5 F1: EF. EF
	vpslld	xmm3, xmm6, 27				; 0356 _ C5 E1: 72. F6, 1B
	vpsrld	xmm6, xmm6, 5				; 035B _ C5 C9: 72. D6, 05
	vpxor	xmm5, xmm5, xmm2			; 0360 _ C5 D1: EF. EA
	vpor	xmm2, xmm3, xmm6			; 0364 _ C5 E1: EB. D6
	vpslld	xmm3, xmm7, 25				; 0368 _ C5 E1: 72. F7, 19
	vpxor	xmm6, xmm2, xmm4			; 036D _ C5 E9: EF. F4
	vpxor	xmm2, xmm6, xmm7			; 0371 _ C5 C9: EF. D7
	vpsrld	xmm7, xmm7, 7				; 0375 _ C5 C1: 72. D7, 07
	vpor	xmm6, xmm3, xmm7			; 037A _ C5 E1: EB. F7
	vpslld	xmm3, xmm2, 3				; 037E _ C5 E1: 72. F2, 03
	vpxor	xmm7, xmm6, xmm5			; 0383 _ C5 C9: EF. FD
	vpslld	xmm0, xmm2, 19				; 0387 _ C5 F9: 72. F2, 13
	vpxor	xmm6, xmm7, xmm3			; 038C _ C5 C1: EF. F3
	vpslld	xmm7, xmm4, 31				; 0390 _ C5 C1: 72. F4, 1F
	vpsrld	xmm4, xmm4, 1				; 0395 _ C5 D9: 72. D4, 01
	vpsrld	xmm1, xmm2, 13				; 039A _ C5 F1: 72. D2, 0D
	vpor	xmm7, xmm7, xmm4			; 039F _ C5 C1: EB. FC
	vpor	xmm3, xmm0, xmm1			; 03A3 _ C5 F9: EB. D9
	vpxor	xmm2, xmm7, xmm2			; 03A7 _ C5 C1: EF. D2
	vpslld	xmm4, xmm5, 29				; 03AB _ C5 D9: 72. F5, 1D
	vpxor	xmm1, xmm2, xmm5			; 03B0 _ C5 E9: EF. CD
	vpcmpeqd xmm0, xmm0, xmm0			; 03B4 _ C5 F9: 76. C0
	vpsrld	xmm5, xmm5, 3				; 03B8 _ C5 D1: 72. D5, 03
	vpxor	xmm2, xmm1, xmm0			; 03BD _ C5 F1: EF. D0
	vpor	xmm5, xmm4, xmm5			; 03C1 _ C5 D9: EB. ED
	vpor	xmm7, xmm6, xmm3			; 03C5 _ C5 C9: EB. FB
	vpxor	xmm1, xmm5, xmm2			; 03C9 _ C5 D1: EF. CA
	vpxor	xmm7, xmm7, xmm1			; 03CD _ C5 C1: EF. F9
	vpxor	xmm5, xmm6, xmm7			; 03D1 _ C5 C9: EF. EF
	vpor	xmm6, xmm1, xmm2			; 03D5 _ C5 F1: EB. F2
	vpand	xmm6, xmm6, xmm3			; 03D9 _ C5 C9: DB. F3
	vpxor	xmm6, xmm6, xmm5			; 03DD _ C5 C9: EF. F5
	vpor	xmm5, xmm5, xmm3			; 03E1 _ C5 D1: EB. EB
	vpand	xmm4, xmm2, xmm6			; 03E5 _ C5 E9: DB. E6
	vpxor	xmm2, xmm5, xmm2			; 03E9 _ C5 D1: EF. D2
	vpxor	xmm1, xmm4, xmm7			; 03ED _ C5 D9: EF. CF
	vpxor	xmm4, xmm2, xmm6			; 03F1 _ C5 E9: EF. E6
	vmovd	xmm2, dword [ecx+2C50H] 		; 03F5 _ C5 F9: 6E. 91, 00002C50
	vpxor	xmm5, xmm4, xmm1			; 03FD _ C5 D9: EF. E9
	vpshufd xmm2, xmm2, 0				; 0401 _ C5 F9: 70. D2, 00
	vpand	xmm7, xmm7, xmm4			; 0406 _ C5 C1: DB. FC
	vpxor	xmm1, xmm1, xmm2			; 040A _ C5 F1: EF. CA
	vpxor	xmm0, xmm5, xmm0			; 040E _ C5 D1: EF. C0
	vmovd	xmm2, dword [ecx+2C54H] 		; 0412 _ C5 F9: 6E. 91, 00002C54
	vpxor	xmm5, xmm7, xmm5			; 041A _ C5 C1: EF. ED
	vpshufd xmm2, xmm2, 0				; 041E _ C5 F9: 70. D2, 00
	vpxor	xmm5, xmm5, xmm3			; 0423 _ C5 D1: EF. EB
	vmovd	xmm3, dword [ecx+2C58H] 		; 0427 _ C5 F9: 6E. 99, 00002C58
	vpxor	xmm0, xmm0, xmm2			; 042F _ C5 F9: EF. C2
	vmovd	xmm7, dword [ecx+2C5CH] 		; 0433 _ C5 F9: 6E. B9, 00002C5C
	vpshufd xmm2, xmm3, 0				; 043B _ C5 F9: 70. D3, 00
	vpshufd xmm3, xmm7, 0				; 0440 _ C5 F9: 70. DF, 00
	vpxor	xmm4, xmm5, xmm2			; 0445 _ C5 D1: EF. E2
	vpxor	xmm2, xmm6, xmm3			; 0449 _ C5 C9: EF. D3
	vpslld	xmm6, xmm4, 10				; 044D _ C5 C9: 72. F4, 0A
	vpsrld	xmm5, xmm4, 22				; 0452 _ C5 D1: 72. D4, 16
	vpslld	xmm3, xmm0, 7				; 0457 _ C5 E1: 72. F0, 07
	vpor	xmm6, xmm6, xmm5			; 045C _ C5 C9: EB. F5
	vpslld	xmm5, xmm1, 27				; 0460 _ C5 D1: 72. F1, 1B
	vpsrld	xmm1, xmm1, 5				; 0465 _ C5 F1: 72. D1, 05
	vpxor	xmm7, xmm6, xmm2			; 046A _ C5 C9: EF. FA
	vpor	xmm5, xmm5, xmm1			; 046E _ C5 D1: EB. E9
	vpxor	xmm4, xmm7, xmm3			; 0472 _ C5 C1: EF. E3
	vpxor	xmm6, xmm5, xmm0			; 0476 _ C5 D1: EF. F0
	vpslld	xmm3, xmm2, 25				; 047A _ C5 E1: 72. F2, 19
	vpxor	xmm7, xmm6, xmm2			; 047F _ C5 C9: EF. FA
	vpsrld	xmm2, xmm2, 7				; 0483 _ C5 E9: 72. D2, 07
	vpor	xmm1, xmm3, xmm2			; 0488 _ C5 E1: EB. CA
	vpslld	xmm2, xmm7, 3				; 048C _ C5 E9: 72. F7, 03
	vpxor	xmm5, xmm1, xmm4			; 0491 _ C5 F1: EF. EC
	vpslld	xmm3, xmm0, 31				; 0495 _ C5 E1: 72. F0, 1F
	vpsrld	xmm0, xmm0, 1				; 049A _ C5 F9: 72. D0, 01
	vpxor	xmm6, xmm5, xmm2			; 049F _ C5 D1: EF. F2
	vpor	xmm5, xmm3, xmm0			; 04A3 _ C5 E1: EB. E8
	vpslld	xmm3, xmm4, 29				; 04A7 _ C5 E1: 72. F4, 1D
	vpsrld	xmm0, xmm4, 3				; 04AC _ C5 F9: 72. D4, 03
	vpxor	xmm2, xmm5, xmm7			; 04B1 _ C5 D1: EF. D7
	vpor	xmm0, xmm3, xmm0			; 04B5 _ C5 E1: EB. C0
	vpxor	xmm1, xmm2, xmm4			; 04B9 _ C5 E9: EF. CC
	vpslld	xmm4, xmm7, 19				; 04BD _ C5 D9: 72. F7, 13
	vpsrld	xmm5, xmm7, 13				; 04C2 _ C5 D1: 72. D7, 0D
	vpand	xmm2, xmm0, xmm6			; 04C7 _ C5 F9: DB. D6
	vpor	xmm3, xmm4, xmm5			; 04CB _ C5 D9: EB. DD
	vpxor	xmm7, xmm2, xmm1			; 04CF _ C5 E9: EF. F9
	vpor	xmm1, xmm1, xmm6			; 04D3 _ C5 F1: EB. CE
	vpand	xmm5, xmm1, xmm3			; 04D7 _ C5 F1: DB. EB
	vpxor	xmm2, xmm0, xmm7			; 04DB _ C5 F9: EF. D7
	vpxor	xmm4, xmm2, xmm5			; 04DF _ C5 E9: EF. E5
	vpcmpeqd xmm0, xmm0, xmm0			; 04E3 _ C5 F9: 76. C0
	vpxor	xmm2, xmm3, xmm0			; 04E7 _ C5 E1: EF. D0
	vpxor	xmm0, xmm6, xmm4			; 04EB _ C5 C9: EF. C4
	vpand	xmm6, xmm5, xmm7			; 04EF _ C5 D1: DB. F7
	vpxor	xmm3, xmm6, xmm0			; 04F3 _ C5 C9: EF. D8
	vpxor	xmm5, xmm2, xmm3			; 04F7 _ C5 E9: EF. EB
	vpand	xmm2, xmm0, xmm2			; 04FB _ C5 F9: DB. D2
	vmovd	xmm1, dword [ecx+2C40H] 		; 04FF _ C5 F9: 6E. 89, 00002C40
	vpxor	xmm6, xmm2, xmm7			; 0507 _ C5 E9: EF. F7
	vpshufd xmm2, xmm1, 0				; 050B _ C5 F9: 70. D1, 00
	vpxor	xmm6, xmm6, xmm5			; 0510 _ C5 C9: EF. F5
	vpand	xmm7, xmm7, xmm5			; 0514 _ C5 C1: DB. FD
	vpxor	xmm2, xmm5, xmm2			; 0518 _ C5 D1: EF. D2
	vmovd	xmm1, dword [ecx+2C44H] 		; 051C _ C5 F9: 6E. 89, 00002C44
	vpxor	xmm0, xmm6, xmm5			; 0524 _ C5 C9: EF. C5
	vpxor	xmm5, xmm7, xmm4			; 0528 _ C5 C1: EF. EC
	vmovd	xmm7, dword [ecx+2C48H] 		; 052C _ C5 F9: 6E. B9, 00002C48
	vpor	xmm6, xmm5, xmm6			; 0534 _ C5 D1: EB. F6
	vpshufd xmm1, xmm1, 0				; 0538 _ C5 F9: 70. C9, 00
	vpxor	xmm3, xmm6, xmm3			; 053D _ C5 C9: EF. DB
	vmovd	xmm5, dword [ecx+2C4CH] 		; 0541 _ C5 F9: 6E. A9, 00002C4C
	vpxor	xmm0, xmm0, xmm1			; 0549 _ C5 F9: EF. C1
	vpshufd xmm1, xmm7, 0				; 054D _ C5 F9: 70. CF, 00
	vpshufd xmm6, xmm5, 0				; 0552 _ C5 F9: 70. F5, 00
	vpxor	xmm7, xmm3, xmm1			; 0557 _ C5 E1: EF. F9
	vpxor	xmm3, xmm4, xmm6			; 055B _ C5 D9: EF. DE
	vpslld	xmm4, xmm7, 10				; 055F _ C5 D9: 72. F7, 0A
	vpsrld	xmm5, xmm7, 22				; 0564 _ C5 D1: 72. D7, 16
	vpslld	xmm1, xmm0, 7				; 0569 _ C5 F1: 72. F0, 07
	vpor	xmm6, xmm4, xmm5			; 056E _ C5 D9: EB. F5
	vpslld	xmm5, xmm2, 27				; 0572 _ C5 D1: 72. F2, 1B
	vpsrld	xmm2, xmm2, 5				; 0577 _ C5 E9: 72. D2, 05
	vpxor	xmm7, xmm6, xmm3			; 057C _ C5 C9: EF. FB
	vpor	xmm5, xmm5, xmm2			; 0580 _ C5 D1: EB. EA
	vpxor	xmm4, xmm7, xmm1			; 0584 _ C5 C1: EF. E1
	vpxor	xmm2, xmm5, xmm0			; 0588 _ C5 D1: EF. D0
	vpslld	xmm6, xmm3, 25				; 058C _ C5 C9: 72. F3, 19
	vpsrld	xmm7, xmm3, 7				; 0591 _ C5 C1: 72. D3, 07
	vpxor	xmm1, xmm2, xmm3			; 0596 _ C5 E9: EF. CB
	vpor	xmm3, xmm6, xmm7			; 059A _ C5 C9: EB. DF
	vpslld	xmm2, xmm1, 3				; 059E _ C5 E9: 72. F1, 03
	vpxor	xmm5, xmm3, xmm4			; 05A3 _ C5 E1: EF. EC
	vpslld	xmm6, xmm0, 31				; 05A7 _ C5 C9: 72. F0, 1F
	vpsrld	xmm0, xmm0, 1				; 05AC _ C5 F9: 72. D0, 01
	vpxor	xmm3, xmm5, xmm2			; 05B1 _ C5 D1: EF. DA
	vpor	xmm5, xmm6, xmm0			; 05B5 _ C5 C9: EB. E8
	vpslld	xmm6, xmm4, 29				; 05B9 _ C5 C9: 72. F4, 1D
	vpxor	xmm2, xmm5, xmm1			; 05BE _ C5 D1: EF. D1
	vpsrld	xmm7, xmm4, 3				; 05C2 _ C5 C1: 72. D4, 03
	vpxor	xmm2, xmm2, xmm4			; 05C7 _ C5 E9: EF. D4
	vpor	xmm6, xmm6, xmm7			; 05CB _ C5 C9: EB. F7
	vpslld	xmm4, xmm1, 19				; 05CF _ C5 D9: 72. F1, 13
	vpsrld	xmm5, xmm1, 13				; 05D4 _ C5 D1: 72. D1, 0D
	vpxor	xmm7, xmm6, xmm2			; 05D9 _ C5 C9: EF. FA
	vpor	xmm0, xmm4, xmm5			; 05DD _ C5 D9: EB. C5
	vpxor	xmm5, xmm0, xmm7			; 05E1 _ C5 F9: EF. EF
	vpand	xmm6, xmm6, xmm7			; 05E5 _ C5 C9: DB. F7
	vpxor	xmm4, xmm6, xmm5			; 05E9 _ C5 C9: EF. E5
	vpand	xmm1, xmm5, xmm2			; 05ED _ C5 D1: DB. CA
	vpor	xmm6, xmm3, xmm4			; 05F1 _ C5 E1: EB. F4
	vpxor	xmm3, xmm2, xmm3			; 05F5 _ C5 E9: EF. DB
	vpxor	xmm7, xmm7, xmm6			; 05F9 _ C5 C1: EF. FE
	vpxor	xmm5, xmm1, xmm6			; 05FD _ C5 F1: EF. EE
	vpxor	xmm3, xmm3, xmm4			; 0601 _ C5 E1: EF. DC
	vpand	xmm2, xmm6, xmm7			; 0605 _ C5 C9: DB. D7
	vmovd	xmm1, dword [ecx+2C30H] 		; 0609 _ C5 F9: 6E. 89, 00002C30
	vpxor	xmm6, xmm3, xmm5			; 0611 _ C5 E1: EF. F5
	vpor	xmm0, xmm6, xmm7			; 0615 _ C5 C9: EB. C7
	vpxor	xmm2, xmm2, xmm3			; 0619 _ C5 E9: EF. D3
	vpshufd xmm6, xmm1, 0				; 061D _ C5 F9: 70. F1, 00
	vpxor	xmm4, xmm0, xmm4			; 0622 _ C5 F9: EF. E4
	vpxor	xmm1, xmm7, xmm6			; 0626 _ C5 C1: EF. CE
	vpxor	xmm5, xmm5, xmm2			; 062A _ C5 D1: EF. EA
	vmovd	xmm7, dword [ecx+2C34H] 		; 062E _ C5 F9: 6E. B9, 00002C34
	vpxor	xmm5, xmm5, xmm4			; 0636 _ C5 D1: EF. EC
	vpshufd xmm6, xmm7, 0				; 063A _ C5 F9: 70. F7, 00
	vmovd	xmm7, dword [ecx+2C38H] 		; 063F _ C5 F9: 6E. B9, 00002C38
	vpxor	xmm6, xmm4, xmm6			; 0647 _ C5 D9: EF. F6
	vpshufd xmm3, xmm7, 0				; 064B _ C5 F9: 70. DF, 00
	vmovd	xmm4, dword [ecx+2C3CH] 		; 0650 _ C5 F9: 6E. A1, 00002C3C
	vpxor	xmm0, xmm2, xmm3			; 0658 _ C5 E9: EF. C3
	vpshufd xmm2, xmm4, 0				; 065C _ C5 F9: 70. D4, 00
	vpslld	xmm7, xmm0, 10				; 0661 _ C5 C1: 72. F0, 0A
	vpsrld	xmm3, xmm0, 22				; 0666 _ C5 E1: 72. D0, 16
	vpxor	xmm2, xmm5, xmm2			; 066B _ C5 D1: EF. D2
	vpor	xmm0, xmm7, xmm3			; 066F _ C5 C1: EB. C3
	vpslld	xmm5, xmm6, 7				; 0673 _ C5 D1: 72. F6, 07
	vpxor	xmm4, xmm0, xmm2			; 0678 _ C5 F9: EF. E2
	vpslld	xmm7, xmm1, 27				; 067C _ C5 C1: 72. F1, 1B
	vpsrld	xmm1, xmm1, 5				; 0681 _ C5 F1: 72. D1, 05
	vpxor	xmm3, xmm4, xmm5			; 0686 _ C5 D9: EF. DD
	vpor	xmm5, xmm7, xmm1			; 068A _ C5 C1: EB. E9
	vpslld	xmm0, xmm2, 25				; 068E _ C5 F9: 72. F2, 19
	vpxor	xmm7, xmm5, xmm6			; 0693 _ C5 D1: EF. FE
	vpxor	xmm5, xmm7, xmm2			; 0697 _ C5 C1: EF. EA
	vpsrld	xmm2, xmm2, 7				; 069B _ C5 E9: 72. D2, 07
	vpor	xmm1, xmm0, xmm2			; 06A0 _ C5 F9: EB. CA
	vpslld	xmm2, xmm5, 3				; 06A4 _ C5 E9: 72. F5, 03
	vpxor	xmm4, xmm1, xmm3			; 06A9 _ C5 F1: EF. E3
	vpslld	xmm0, xmm6, 31				; 06AD _ C5 F9: 72. F6, 1F
	vpsrld	xmm6, xmm6, 1				; 06B2 _ C5 C9: 72. D6, 01
	vpxor	xmm7, xmm4, xmm2			; 06B7 _ C5 D9: EF. FA
	vpor	xmm2, xmm0, xmm6			; 06BB _ C5 F9: EB. D6
	vpslld	xmm0, xmm5, 19				; 06BF _ C5 F9: 72. F5, 13
	vpxor	xmm6, xmm2, xmm5			; 06C4 _ C5 E9: EF. F5
	vpsrld	xmm5, xmm5, 13				; 06C8 _ C5 D1: 72. D5, 0D
	vpxor	xmm2, xmm6, xmm3			; 06CD _ C5 C9: EF. D3
	vpslld	xmm1, xmm3, 29				; 06D1 _ C5 F1: 72. F3, 1D
	vpsrld	xmm3, xmm3, 3				; 06D6 _ C5 E1: 72. D3, 03
	vpor	xmm5, xmm0, xmm5			; 06DB _ C5 F9: EB. ED
	vpor	xmm4, xmm1, xmm3			; 06DF _ C5 F1: EB. E3
	vpxor	xmm0, xmm7, xmm5			; 06E3 _ C5 C1: EF. C5
	vpxor	xmm4, xmm4, xmm7			; 06E7 _ C5 D9: EF. E7
	vpand	xmm6, xmm0, xmm4			; 06EB _ C5 F9: DB. F4
	vpor	xmm7, xmm2, xmm4			; 06EF _ C5 E9: EB. FC
	vpxor	xmm3, xmm6, xmm2			; 06F3 _ C5 C9: EF. DA
	vpxor	xmm2, xmm7, xmm0			; 06F7 _ C5 C1: EF. D0
	vpand	xmm6, xmm0, xmm3			; 06FB _ C5 F9: DB. F3
	vpxor	xmm1, xmm4, xmm3			; 06FF _ C5 D9: EF. CB
	vpand	xmm7, xmm6, xmm5			; 0703 _ C5 C9: DB. FD
	vmovd	xmm4, dword [ecx+2C24H] 		; 0707 _ C5 F9: 6E. A1, 00002C24
	vpxor	xmm6, xmm7, xmm1			; 070F _ C5 C1: EF. F1
	vpcmpeqd xmm0, xmm0, xmm0			; 0713 _ C5 F9: 76. C0
	vpand	xmm1, xmm1, xmm2			; 0717 _ C5 F1: DB. CA
	vpxor	xmm7, xmm3, xmm0			; 071B _ C5 E1: EF. F8
	vpor	xmm1, xmm1, xmm5			; 071F _ C5 F1: EB. CD
	vmovd	xmm3, dword [ecx+2C20H] 		; 0723 _ C5 F9: 6E. 99, 00002C20
	vpxor	xmm1, xmm1, xmm7			; 072B _ C5 F1: EF. CF
	vpshufd xmm0, xmm4, 0				; 072F _ C5 F9: 70. C4, 00
	vpxor	xmm5, xmm5, xmm7			; 0734 _ C5 D1: EF. EF
	vmovd	xmm4, dword [ecx+2C28H] 		; 0738 _ C5 F9: 6E. A1, 00002C28
	vpxor	xmm0, xmm6, xmm0			; 0740 _ C5 C9: EF. C0
	vpshufd xmm3, xmm3, 0				; 0744 _ C5 F9: 70. DB, 00
	vpshufd xmm4, xmm4, 0				; 0749 _ C5 F9: 70. E4, 00
	vpxor	xmm3, xmm2, xmm3			; 074E _ C5 E9: EF. DB
	vpxor	xmm1, xmm1, xmm4			; 0752 _ C5 F1: EF. CC
	vpxor	xmm4, xmm7, xmm6			; 0756 _ C5 C1: EF. E6
	vpand	xmm5, xmm5, xmm2			; 075A _ C5 D1: DB. EA
	vpxor	xmm7, xmm4, xmm5			; 075E _ C5 D9: EF. FD
	vpslld	xmm5, xmm3, 27				; 0762 _ C5 D1: 72. F3, 1B
	vmovd	xmm4, dword [ecx+2C2CH] 		; 0767 _ C5 F9: 6E. A1, 00002C2C
	vpsrld	xmm3, xmm3, 5				; 076F _ C5 E1: 72. D3, 05
	vpshufd xmm2, xmm4, 0				; 0774 _ C5 F9: 70. D4, 00
	vpslld	xmm4, xmm1, 10				; 0779 _ C5 D9: 72. F1, 0A
	vpsrld	xmm1, xmm1, 22				; 077E _ C5 F1: 72. D1, 16
	vpxor	xmm6, xmm7, xmm2			; 0783 _ C5 C1: EF. F2
	vpor	xmm2, xmm4, xmm1			; 0787 _ C5 D9: EB. D1
	vpslld	xmm4, xmm0, 7				; 078B _ C5 D9: 72. F0, 07
	vpxor	xmm1, xmm2, xmm6			; 0790 _ C5 E9: EF. CE
	vpor	xmm3, xmm5, xmm3			; 0794 _ C5 D1: EB. DB
	vpxor	xmm2, xmm1, xmm4			; 0798 _ C5 F1: EF. D4
	vpxor	xmm1, xmm3, xmm0			; 079C _ C5 E1: EF. C8
	vpxor	xmm7, xmm1, xmm6			; 07A0 _ C5 F1: EF. FE
	vpslld	xmm4, xmm6, 25				; 07A4 _ C5 D9: 72. F6, 19
	vpsrld	xmm6, xmm6, 7				; 07A9 _ C5 C9: 72. D6, 07
	vpslld	xmm1, xmm7, 3				; 07AE _ C5 F1: 72. F7, 03
	vpor	xmm5, xmm4, xmm6			; 07B3 _ C5 D9: EB. EE
	vpslld	xmm6, xmm0, 31				; 07B7 _ C5 C9: 72. F0, 1F
	vpxor	xmm3, xmm5, xmm2			; 07BC _ C5 D1: EF. DA
	vpsrld	xmm0, xmm0, 1				; 07C0 _ C5 F9: 72. D0, 01
	vpxor	xmm1, xmm3, xmm1			; 07C5 _ C5 E1: EF. C9
	vpor	xmm3, xmm6, xmm0			; 07C9 _ C5 C9: EB. D8
	vpxor	xmm0, xmm3, xmm7			; 07CD _ C5 E1: EF. C7
	vpslld	xmm6, xmm2, 29				; 07D1 _ C5 C9: 72. F2, 1D
	vpxor	xmm4, xmm0, xmm2			; 07D6 _ C5 F9: EF. E2
	vpsrld	xmm2, xmm2, 3				; 07DA _ C5 E9: 72. D2, 03
	vpor	xmm6, xmm6, xmm2			; 07DF _ C5 C9: EB. F2
	vpslld	xmm5, xmm7, 19				; 07E3 _ C5 D1: 72. F7, 13
	vpsrld	xmm7, xmm7, 13				; 07E8 _ C5 C1: 72. D7, 0D
	vpxor	xmm2, xmm4, xmm1			; 07ED _ C5 D9: EF. D1
	vpor	xmm0, xmm5, xmm7			; 07F1 _ C5 D1: EB. C7
	vpand	xmm3, xmm1, xmm2			; 07F5 _ C5 F1: DB. DA
	vpxor	xmm4, xmm4, xmm6			; 07F9 _ C5 D9: EF. E6
	vpxor	xmm7, xmm3, xmm0			; 07FD _ C5 E1: EF. F8
	vpor	xmm1, xmm0, xmm2			; 0801 _ C5 F9: EB. CA
	vpxor	xmm6, xmm6, xmm7			; 0805 _ C5 C9: EF. F7
	vpxor	xmm5, xmm2, xmm7			; 0809 _ C5 E9: EF. EF
	vpxor	xmm2, xmm1, xmm4			; 080D _ C5 F1: EF. D4
	vpor	xmm3, xmm2, xmm6			; 0811 _ C5 E9: EB. DE
	vpor	xmm0, xmm5, xmm7			; 0815 _ C5 D1: EB. C7
	vpxor	xmm1, xmm3, xmm5			; 0819 _ C5 E1: EF. CD
	vpxor	xmm3, xmm0, xmm1			; 081D _ C5 F9: EF. D9
	vpcmpeqd xmm0, xmm0, xmm0			; 0821 _ C5 F9: 76. C0
	vpxor	xmm4, xmm4, xmm0			; 0825 _ C5 D9: EF. E0
	vpxor	xmm2, xmm4, xmm3			; 0829 _ C5 D9: EF. D3
	vpor	xmm3, xmm3, xmm1			; 082D _ C5 E1: EB. D9
	vmovd	xmm4, dword [ecx+2C10H] 		; 0831 _ C5 F9: 6E. A1, 00002C10
	vpshufd xmm5, xmm4, 0				; 0839 _ C5 F9: 70. EC, 00
	vpxor	xmm4, xmm2, xmm5			; 083E _ C5 E9: EF. E5
	vmovd	xmm5, dword [ecx+2C14H] 		; 0842 _ C5 F9: 6E. A9, 00002C14
	vpshufd xmm5, xmm5, 0				; 084A _ C5 F9: 70. ED, 00
	vpxor	xmm5, xmm1, xmm5			; 084F _ C5 F1: EF. ED
	vpxor	xmm1, xmm3, xmm1			; 0853 _ C5 E1: EF. C9
	vpor	xmm2, xmm1, xmm2			; 0857 _ C5 F1: EB. D2
	vpxor	xmm2, xmm7, xmm2			; 085B _ C5 C1: EF. D2
	vmovd	xmm7, dword [ecx+2C18H] 		; 085F _ C5 F9: 6E. B9, 00002C18
	vmovd	xmm1, dword [ecx+2C1CH] 		; 0867 _ C5 F9: 6E. 89, 00002C1C
	vpshufd xmm3, xmm7, 0				; 086F _ C5 F9: 70. DF, 00
	vpshufd xmm7, xmm1, 0				; 0874 _ C5 F9: 70. F9, 00
	vpxor	xmm2, xmm2, xmm3			; 0879 _ C5 E9: EF. D3
	vpxor	xmm1, xmm6, xmm7			; 087D _ C5 C9: EF. CF
	vpslld	xmm6, xmm2, 10				; 0881 _ C5 C9: 72. F2, 0A
	vpsrld	xmm2, xmm2, 22				; 0886 _ C5 E9: 72. D2, 16
	vpslld	xmm7, xmm5, 7				; 088B _ C5 C1: 72. F5, 07
	vpor	xmm3, xmm6, xmm2			; 0890 _ C5 C9: EB. DA
	vpslld	xmm2, xmm4, 27				; 0894 _ C5 E9: 72. F4, 1B
	vpsrld	xmm4, xmm4, 5				; 0899 _ C5 D9: 72. D4, 05
	vpxor	xmm6, xmm3, xmm1			; 089E _ C5 E1: EF. F1
	vpor	xmm2, xmm2, xmm4			; 08A2 _ C5 E9: EB. D4
	vpxor	xmm7, xmm6, xmm7			; 08A6 _ C5 C9: EF. FF
	vpxor	xmm3, xmm2, xmm5			; 08AA _ C5 E9: EF. DD
	vpslld	xmm6, xmm1, 25				; 08AE _ C5 C9: 72. F1, 19
	vpxor	xmm3, xmm3, xmm1			; 08B3 _ C5 E1: EF. D9
	vpsrld	xmm1, xmm1, 7				; 08B7 _ C5 F1: 72. D1, 07
	vpor	xmm2, xmm6, xmm1			; 08BC _ C5 C9: EB. D1
	vpslld	xmm4, xmm5, 31				; 08C0 _ C5 D9: 72. F5, 1F
	vpsrld	xmm5, xmm5, 1				; 08C5 _ C5 D1: 72. D5, 01
	vpxor	xmm1, xmm2, xmm7			; 08CA _ C5 E9: EF. CF
	vpslld	xmm6, xmm3, 3				; 08CE _ C5 C9: 72. F3, 03
	vpor	xmm2, xmm4, xmm5			; 08D3 _ C5 D9: EB. D5
	vpxor	xmm1, xmm1, xmm6			; 08D7 _ C5 F1: EF. CE
	vpxor	xmm6, xmm2, xmm3			; 08DB _ C5 E9: EF. F3
	vpxor	xmm4, xmm6, xmm7			; 08DF _ C5 C9: EF. E7
	vpslld	xmm5, xmm3, 19				; 08E3 _ C5 D1: 72. F3, 13
	vpsrld	xmm3, xmm3, 13				; 08E8 _ C5 E1: 72. D3, 0D
	vpslld	xmm2, xmm7, 29				; 08ED _ C5 E9: 72. F7, 1D
	vpsrld	xmm7, xmm7, 3				; 08F2 _ C5 C1: 72. D7, 03
	vpor	xmm5, xmm5, xmm3			; 08F7 _ C5 D1: EB. EB
	vpor	xmm2, xmm2, xmm7			; 08FB _ C5 E9: EB. D7
	vpxor	xmm6, xmm4, xmm0			; 08FF _ C5 D9: EF. F0
	vpxor	xmm2, xmm2, xmm0			; 0903 _ C5 E9: EF. D0
	vpor	xmm3, xmm4, xmm5			; 0907 _ C5 D9: EB. DD
	vpxor	xmm4, xmm3, xmm2			; 090B _ C5 E1: EF. E2
	vpxor	xmm7, xmm5, xmm6			; 090F _ C5 D1: EF. FE
	vpxor	xmm4, xmm4, xmm1			; 0913 _ C5 D9: EF. E1
	vpor	xmm5, xmm2, xmm6			; 0917 _ C5 E9: EB. EE
	vpand	xmm2, xmm7, xmm1			; 091B _ C5 C1: DB. D1
	vpxor	xmm5, xmm5, xmm7			; 091F _ C5 D1: EF. EF
	vpxor	xmm7, xmm6, xmm2			; 0923 _ C5 C9: EF. FA
	vpor	xmm6, xmm2, xmm4			; 0927 _ C5 E9: EB. F4
	vpxor	xmm3, xmm6, xmm5			; 092B _ C5 C9: EF. DD
	vpxor	xmm1, xmm1, xmm7			; 092F _ C5 F1: EF. CF
	vpxor	xmm2, xmm1, xmm3			; 0933 _ C5 F1: EF. D3
	vmovd	xmm1, dword [ecx+2C00H] 		; 0937 _ C5 F9: 6E. 89, 00002C00
	vpxor	xmm6, xmm2, xmm4			; 093F _ C5 E9: EF. F4
	vpshufd xmm2, xmm1, 0				; 0943 _ C5 F9: 70. D1, 00
	vpxor	xmm2, xmm3, xmm2			; 0948 _ C5 E1: EF. D2
	vpxor	xmm3, xmm5, xmm4			; 094C _ C5 D1: EF. DC
	vpand	xmm5, xmm3, xmm6			; 0950 _ C5 E1: DB. EE
	vpxor	xmm3, xmm7, xmm5			; 0954 _ C5 C1: EF. DD
	vmovd	xmm7, dword [ecx+2C04H] 		; 0958 _ C5 F9: 6E. B9, 00002C04
	vmovd	xmm5, dword [ecx+2C08H] 		; 0960 _ C5 F9: 6E. A9, 00002C08
	vpshufd xmm1, xmm7, 0				; 0968 _ C5 F9: 70. CF, 00
	vpshufd xmm7, xmm5, 0				; 096D _ C5 F9: 70. FD, 00
	vpxor	xmm3, xmm3, xmm1			; 0972 _ C5 E1: EF. D9
	vpxor	xmm7, xmm4, xmm7			; 0976 _ C5 D9: EF. FF
	vmovd	xmm4, dword [ecx+2C0CH] 		; 097A _ C5 F9: 6E. A1, 00002C0C
	vpshufd xmm1, xmm4, 0				; 0982 _ C5 F9: 70. CC, 00
	vpxor	xmm5, xmm6, xmm1			; 0987 _ C5 C9: EF. E9
	vpslld	xmm6, xmm7, 10				; 098B _ C5 C9: 72. F7, 0A
	vpsrld	xmm1, xmm7, 22				; 0990 _ C5 F1: 72. D7, 16
	vpslld	xmm7, xmm3, 7				; 0995 _ C5 C1: 72. F3, 07
	vpor	xmm6, xmm6, xmm1			; 099A _ C5 C9: EB. F1
	vpslld	xmm1, xmm2, 27				; 099E _ C5 F1: 72. F2, 1B
	vpsrld	xmm2, xmm2, 5				; 09A3 _ C5 E9: 72. D2, 05
	vpxor	xmm4, xmm6, xmm5			; 09A8 _ C5 C9: EF. E5
	vpor	xmm2, xmm1, xmm2			; 09AC _ C5 F1: EB. D2
	vpxor	xmm6, xmm4, xmm7			; 09B0 _ C5 D9: EF. F7
	vpxor	xmm1, xmm2, xmm3			; 09B4 _ C5 E9: EF. CB
	vpslld	xmm4, xmm5, 25				; 09B8 _ C5 D9: 72. F5, 19
	vpxor	xmm2, xmm1, xmm5			; 09BD _ C5 F1: EF. D5
	vpsrld	xmm5, xmm5, 7				; 09C1 _ C5 D1: 72. D5, 07
	vpslld	xmm7, xmm3, 31				; 09C6 _ C5 C1: 72. F3, 1F
	vpsrld	xmm3, xmm3, 1				; 09CB _ C5 E1: 72. D3, 01
	vpor	xmm1, xmm4, xmm5			; 09D0 _ C5 D9: EB. CD
	vpor	xmm3, xmm7, xmm3			; 09D4 _ C5 C1: EB. DB
	vpxor	xmm4, xmm1, xmm6			; 09D8 _ C5 F1: EF. E6
	vpslld	xmm5, xmm2, 3				; 09DC _ C5 D1: 72. F2, 03
	vpxor	xmm1, xmm3, xmm2			; 09E1 _ C5 E1: EF. CA
	vpxor	xmm5, xmm4, xmm5			; 09E5 _ C5 D9: EF. ED
	vpxor	xmm7, xmm1, xmm6			; 09E9 _ C5 F1: EF. FE
	vpslld	xmm4, xmm6, 29				; 09ED _ C5 D9: 72. F6, 1D
	vpsrld	xmm6, xmm6, 3				; 09F2 _ C5 C9: 72. D6, 03
	vpslld	xmm3, xmm2, 19				; 09F7 _ C5 E1: 72. F2, 13
	vpsrld	xmm2, xmm2, 13				; 09FC _ C5 E9: 72. D2, 0D
	vpor	xmm1, xmm4, xmm6			; 0A01 _ C5 D9: EB. CE
	vpor	xmm6, xmm3, xmm2			; 0A05 _ C5 E1: EB. F2
	vpor	xmm2, xmm1, xmm5			; 0A09 _ C5 F1: EB. D5
	vpxor	xmm1, xmm1, xmm6			; 0A0D _ C5 F1: EF. CE
	vpand	xmm4, xmm6, xmm5			; 0A11 _ C5 C9: DB. E5
	vpxor	xmm3, xmm1, xmm0			; 0A15 _ C5 F1: EF. D8
	vpxor	xmm5, xmm5, xmm7			; 0A19 _ C5 D1: EF. EF
	vpxor	xmm1, xmm4, xmm3			; 0A1D _ C5 D9: EF. CB
	vpand	xmm6, xmm3, xmm2			; 0A21 _ C5 E1: DB. F2
	vpor	xmm7, xmm7, xmm4			; 0A25 _ C5 C1: EB. FC
	vpand	xmm3, xmm5, xmm2			; 0A29 _ C5 D1: DB. DA
	vpxor	xmm4, xmm7, xmm6			; 0A2D _ C5 C1: EF. E6
	vpxor	xmm7, xmm6, xmm1			; 0A31 _ C5 C9: EF. F9
	vpxor	xmm5, xmm2, xmm4			; 0A35 _ C5 E9: EF. EC
	vpor	xmm2, xmm1, xmm7			; 0A39 _ C5 F1: EB. D7
	vpxor	xmm1, xmm2, xmm3			; 0A3D _ C5 E9: EF. CB
	vpxor	xmm2, xmm3, xmm5			; 0A41 _ C5 E1: EF. D5
	vmovd	xmm3, dword [ecx+2BF0H] 		; 0A45 _ C5 F9: 6E. 99, 00002BF0
	vpxor	xmm6, xmm2, xmm7			; 0A4D _ C5 E9: EF. F7
	vpshufd xmm2, xmm3, 0				; 0A51 _ C5 F9: 70. D3, 00
	vmovd	xmm3, dword [ecx+2BF4H] 		; 0A56 _ C5 F9: 6E. 99, 00002BF4
	vpxor	xmm6, xmm6, xmm2			; 0A5E _ C5 C9: EF. F2
	vpshufd xmm2, xmm3, 0				; 0A62 _ C5 F9: 70. D3, 00
	vmovd	xmm3, dword [ecx+2BF8H] 		; 0A67 _ C5 F9: 6E. 99, 00002BF8
	vpxor	xmm2, xmm1, xmm2			; 0A6F _ C5 F1: EF. D2
	vpshufd xmm3, xmm3, 0				; 0A73 _ C5 F9: 70. DB, 00
	vpor	xmm1, xmm5, xmm1			; 0A78 _ C5 D1: EB. C9
	vmovd	xmm5, dword [ecx+2BFCH] 		; 0A7C _ C5 F9: 6E. A9, 00002BFC
	vpxor	xmm4, xmm4, xmm3			; 0A84 _ C5 D9: EF. E3
	vpshufd xmm3, xmm5, 0				; 0A88 _ C5 F9: 70. DD, 00
	vpxor	xmm7, xmm1, xmm7			; 0A8D _ C5 F1: EF. FF
	vpslld	xmm1, xmm4, 10				; 0A91 _ C5 F1: 72. F4, 0A
	vpsrld	xmm4, xmm4, 22				; 0A96 _ C5 D9: 72. D4, 16
	vpxor	xmm5, xmm7, xmm3			; 0A9B _ C5 C1: EF. EB
	vpor	xmm3, xmm1, xmm4			; 0A9F _ C5 F1: EB. DC
	vpxor	xmm1, xmm3, xmm5			; 0AA3 _ C5 E1: EF. CD
	vpslld	xmm4, xmm2, 7				; 0AA7 _ C5 D9: 72. F2, 07
	vpslld	xmm7, xmm6, 27				; 0AAC _ C5 C1: 72. F6, 1B
	vpsrld	xmm6, xmm6, 5				; 0AB1 _ C5 C9: 72. D6, 05
	vpxor	xmm3, xmm1, xmm4			; 0AB6 _ C5 F1: EF. DC
	vpor	xmm1, xmm7, xmm6			; 0ABA _ C5 C1: EB. CE
	vpxor	xmm6, xmm1, xmm2			; 0ABE _ C5 F1: EF. F2
	vpslld	xmm4, xmm5, 25				; 0AC2 _ C5 D9: 72. F5, 19
	vpxor	xmm7, xmm6, xmm5			; 0AC7 _ C5 C9: EF. FD
	vpsrld	xmm5, xmm5, 7				; 0ACB _ C5 D1: 72. D5, 07
	vpor	xmm1, xmm4, xmm5			; 0AD0 _ C5 D9: EB. CD
	vpslld	xmm4, xmm7, 3				; 0AD4 _ C5 D9: 72. F7, 03
	vpxor	xmm6, xmm1, xmm3			; 0AD9 _ C5 F1: EF. F3
	vpslld	xmm1, xmm2, 31				; 0ADD _ C5 F1: 72. F2, 1F
	vpsrld	xmm2, xmm2, 1				; 0AE2 _ C5 E9: 72. D2, 01
	vpxor	xmm5, xmm6, xmm4			; 0AE7 _ C5 C9: EF. EC
	vpor	xmm2, xmm1, xmm2			; 0AEB _ C5 F1: EB. D2
	vpslld	xmm6, xmm3, 29				; 0AEF _ C5 C9: 72. F3, 1D
	vpxor	xmm1, xmm2, xmm7			; 0AF4 _ C5 E9: EF. CF
	vpslld	xmm2, xmm7, 19				; 0AF8 _ C5 E9: 72. F7, 13
	vpxor	xmm1, xmm1, xmm3			; 0AFD _ C5 F1: EF. CB
	vpsrld	xmm3, xmm3, 3				; 0B01 _ C5 E1: 72. D3, 03
	vpsrld	xmm7, xmm7, 13				; 0B06 _ C5 C1: 72. D7, 0D
	vpor	xmm4, xmm6, xmm3			; 0B0B _ C5 C9: EB. E3
	vpor	xmm2, xmm2, xmm7			; 0B0F _ C5 E9: EB. D7
	vpxor	xmm7, xmm5, xmm1			; 0B13 _ C5 D1: EF. F9
	vpxor	xmm2, xmm2, xmm4			; 0B17 _ C5 E9: EF. D4
	vpand	xmm3, xmm4, xmm2			; 0B1B _ C5 D9: DB. DA
	vpxor	xmm0, xmm3, xmm0			; 0B1F _ C5 E1: EF. C0
	vpxor	xmm3, xmm0, xmm7			; 0B23 _ C5 F9: EF. DF
	vpxor	xmm0, xmm4, xmm5			; 0B27 _ C5 D9: EF. C5
	vpor	xmm6, xmm0, xmm2			; 0B2B _ C5 F9: EB. F2
	vpxor	xmm4, xmm2, xmm3			; 0B2F _ C5 E9: EF. E3
	vpxor	xmm0, xmm7, xmm6			; 0B33 _ C5 C1: EF. C6
	vpxor	xmm6, xmm6, xmm1			; 0B37 _ C5 C9: EF. F1
	vmovd	xmm7, dword [ecx+2BE0H] 		; 0B3B _ C5 F9: 6E. B9, 00002BE0
	vpand	xmm5, xmm1, xmm0			; 0B43 _ C5 F1: DB. E8
	vpshufd xmm2, xmm7, 0				; 0B47 _ C5 F9: 70. D7, 00
	vpxor	xmm5, xmm5, xmm4			; 0B4C _ C5 D1: EF. EC
	vpxor	xmm7, xmm5, xmm2			; 0B50 _ C5 D1: EF. FA
	vpxor	xmm4, xmm4, xmm0			; 0B54 _ C5 D9: EF. E0
	vmovd	xmm2, dword [ecx+2BE4H] 		; 0B58 _ C5 F9: 6E. 91, 00002BE4
	vpor	xmm1, xmm4, xmm3			; 0B60 _ C5 D9: EB. CB
	vpshufd xmm2, xmm2, 0				; 0B64 _ C5 F9: 70. D2, 00
	vpxor	xmm0, xmm0, xmm5			; 0B69 _ C5 F9: EF. C5
	vpxor	xmm2, xmm3, xmm2			; 0B6D _ C5 E1: EF. D2
	vpxor	xmm3, xmm6, xmm1			; 0B71 _ C5 C9: EF. D9
	vmovd	xmm6, dword [ecx+2BE8H] 		; 0B75 _ C5 F9: 6E. B1, 00002BE8
	vpslld	xmm4, xmm2, 7				; 0B7D _ C5 D9: 72. F2, 07
	vpshufd xmm1, xmm6, 0				; 0B82 _ C5 F9: 70. CE, 00
	vpxor	xmm1, xmm3, xmm1			; 0B87 _ C5 E1: EF. C9
	vmovd	xmm3, dword [ecx+2BECH] 		; 0B8B _ C5 F9: 6E. 99, 00002BEC
	vpslld	xmm5, xmm1, 10				; 0B93 _ C5 D1: 72. F1, 0A
	vpshufd xmm6, xmm3, 0				; 0B98 _ C5 F9: 70. F3, 00
	vpsrld	xmm3, xmm1, 22				; 0B9D _ C5 E1: 72. D1, 16
	vpxor	xmm0, xmm0, xmm6			; 0BA2 _ C5 F9: EF. C6
	vpor	xmm1, xmm5, xmm3			; 0BA6 _ C5 D1: EB. CB
	vpslld	xmm5, xmm7, 27				; 0BAA _ C5 D1: 72. F7, 1B
	vpsrld	xmm7, xmm7, 5				; 0BAF _ C5 C1: 72. D7, 05
	vpxor	xmm6, xmm1, xmm0			; 0BB4 _ C5 F1: EF. F0
	vpor	xmm3, xmm5, xmm7			; 0BB8 _ C5 D1: EB. DF
	vpxor	xmm1, xmm6, xmm4			; 0BBC _ C5 C9: EF. CC
	vpxor	xmm6, xmm3, xmm2			; 0BC0 _ C5 E1: EF. F2
	vpxor	xmm3, xmm6, xmm0			; 0BC4 _ C5 C9: EF. D8
	vpslld	xmm4, xmm0, 25				; 0BC8 _ C5 D9: 72. F0, 19
	vpsrld	xmm0, xmm0, 7				; 0BCD _ C5 F9: 72. D0, 07
	vpslld	xmm6, xmm3, 19				; 0BD2 _ C5 C9: 72. F3, 13
	vpor	xmm5, xmm4, xmm0			; 0BD7 _ C5 D9: EB. E8
	vpslld	xmm0, xmm3, 3				; 0BDB _ C5 F9: 72. F3, 03
	vpxor	xmm7, xmm5, xmm1			; 0BE0 _ C5 D1: EF. F9
	vpslld	xmm5, xmm2, 31				; 0BE4 _ C5 D1: 72. F2, 1F
	vpsrld	xmm2, xmm2, 1				; 0BE9 _ C5 E9: 72. D2, 01
	vpsrld	xmm4, xmm3, 13				; 0BEE _ C5 D9: 72. D3, 0D
	vpor	xmm2, xmm5, xmm2			; 0BF3 _ C5 D1: EB. D2
	vpxor	xmm0, xmm7, xmm0			; 0BF7 _ C5 C1: EF. C0
	vpxor	xmm3, xmm2, xmm3			; 0BFB _ C5 E9: EF. DB
	vpor	xmm6, xmm6, xmm4			; 0BFF _ C5 C9: EB. F4
	vpxor	xmm4, xmm3, xmm1			; 0C03 _ C5 E1: EF. E1
	vpcmpeqd xmm5, xmm5, xmm5			; 0C07 _ C5 D1: 76. ED
	vpslld	xmm7, xmm1, 29				; 0C0B _ C5 C1: 72. F1, 1D
	vpsrld	xmm1, xmm1, 3				; 0C10 _ C5 F1: 72. D1, 03
	vpxor	xmm2, xmm4, xmm5			; 0C15 _ C5 D9: EF. D5
	vpor	xmm3, xmm7, xmm1			; 0C19 _ C5 C1: EB. D9
	vpxor	xmm4, xmm3, xmm2			; 0C1D _ C5 E1: EF. E2
	vpor	xmm1, xmm0, xmm6			; 0C21 _ C5 F9: EB. CE
	vpxor	xmm3, xmm1, xmm4			; 0C25 _ C5 F1: EF. DC
	vpxor	xmm1, xmm0, xmm3			; 0C29 _ C5 F9: EF. CB
	vpor	xmm0, xmm4, xmm2			; 0C2D _ C5 D9: EB. C2
	vpand	xmm7, xmm0, xmm6			; 0C31 _ C5 F9: DB. FE
	vpxor	xmm4, xmm7, xmm1			; 0C35 _ C5 C1: EF. E1
	vpor	xmm1, xmm1, xmm6			; 0C39 _ C5 F1: EB. CE
	vpand	xmm0, xmm2, xmm4			; 0C3D _ C5 E9: DB. C4
	vpxor	xmm2, xmm1, xmm2			; 0C41 _ C5 F1: EF. D2
	vpxor	xmm7, xmm0, xmm3			; 0C45 _ C5 F9: EF. FB
	vpxor	xmm1, xmm2, xmm4			; 0C49 _ C5 E9: EF. CC
	vmovd	xmm0, dword [ecx+2BD0H] 		; 0C4D _ C5 F9: 6E. 81, 00002BD0
	vpxor	xmm2, xmm1, xmm7			; 0C55 _ C5 F1: EF. D7
	vpand	xmm3, xmm3, xmm1			; 0C59 _ C5 E1: DB. D9
	vpxor	xmm5, xmm2, xmm5			; 0C5D _ C5 E9: EF. ED
	vpshufd xmm0, xmm0, 0				; 0C61 _ C5 F9: 70. C0, 00
	vpxor	xmm2, xmm3, xmm2			; 0C66 _ C5 E1: EF. D2
	vpxor	xmm0, xmm7, xmm0			; 0C6A _ C5 C1: EF. C0
	vpxor	xmm2, xmm2, xmm6			; 0C6E _ C5 E9: EF. D6
	vmovd	xmm7, dword [ecx+2BD4H] 		; 0C72 _ C5 F9: 6E. B9, 00002BD4
	vmovd	xmm6, dword [ecx+2BD8H] 		; 0C7A _ C5 F9: 6E. B1, 00002BD8
	vpshufd xmm7, xmm7, 0				; 0C82 _ C5 F9: 70. FF, 00
	vpshufd xmm3, xmm6, 0				; 0C87 _ C5 F9: 70. DE, 00
	vpxor	xmm5, xmm5, xmm7			; 0C8C _ C5 D1: EF. EF
	vpxor	xmm7, xmm2, xmm3			; 0C90 _ C5 E9: EF. FB
	vmovd	xmm1, dword [ecx+2BDCH] 		; 0C94 _ C5 F9: 6E. 89, 00002BDC
	vpslld	xmm2, xmm7, 10				; 0C9C _ C5 E9: 72. F7, 0A
	vpsrld	xmm3, xmm7, 22				; 0CA1 _ C5 E1: 72. D7, 16
	vpslld	xmm7, xmm5, 7				; 0CA6 _ C5 C1: 72. F5, 07
	vpshufd xmm6, xmm1, 0				; 0CAB _ C5 F9: 70. F1, 00
	vpor	xmm1, xmm2, xmm3			; 0CB0 _ C5 E9: EB. CB
	vpslld	xmm2, xmm0, 27				; 0CB4 _ C5 E9: 72. F0, 1B
	vpsrld	xmm0, xmm0, 5				; 0CB9 _ C5 F9: 72. D0, 05
	vpor	xmm2, xmm2, xmm0			; 0CBE _ C5 E9: EB. D0
	vpxor	xmm4, xmm4, xmm6			; 0CC2 _ C5 D9: EF. E6
	vpxor	xmm3, xmm2, xmm5			; 0CC6 _ C5 E9: EF. DD
	vpxor	xmm6, xmm1, xmm4			; 0CCA _ C5 F1: EF. F4
	vpxor	xmm1, xmm3, xmm4			; 0CCE _ C5 E1: EF. CC
	vpslld	xmm0, xmm4, 25				; 0CD2 _ C5 F9: 72. F4, 19
	vpsrld	xmm4, xmm4, 7				; 0CD7 _ C5 D9: 72. D4, 07
	vpxor	xmm7, xmm6, xmm7			; 0CDC _ C5 C9: EF. FF
	vpor	xmm2, xmm0, xmm4			; 0CE0 _ C5 F9: EB. D4
	vpslld	xmm0, xmm1, 3				; 0CE4 _ C5 F9: 72. F1, 03
	vpxor	xmm3, xmm2, xmm7			; 0CE9 _ C5 E9: EF. DF
	vpslld	xmm6, xmm5, 31				; 0CED _ C5 C9: 72. F5, 1F
	vpsrld	xmm5, xmm5, 1				; 0CF2 _ C5 D1: 72. D5, 01
	vpxor	xmm4, xmm3, xmm0			; 0CF7 _ C5 E1: EF. E0
	vpor	xmm2, xmm6, xmm5			; 0CFB _ C5 C9: EB. D5
	vpslld	xmm0, xmm7, 29				; 0CFF _ C5 F9: 72. F7, 1D
	vpsrld	xmm5, xmm7, 3				; 0D04 _ C5 D1: 72. D7, 03
	vpxor	xmm3, xmm2, xmm1			; 0D09 _ C5 E9: EF. D9
	vpor	xmm0, xmm0, xmm5			; 0D0D _ C5 F9: EB. C5
	vpxor	xmm6, xmm3, xmm7			; 0D11 _ C5 E1: EF. F7
	vpslld	xmm7, xmm1, 19				; 0D15 _ C5 C1: 72. F1, 13
	vpsrld	xmm1, xmm1, 13				; 0D1A _ C5 F1: 72. D1, 0D
	vpand	xmm3, xmm0, xmm4			; 0D1F _ C5 F9: DB. DC
	vpor	xmm2, xmm7, xmm1			; 0D23 _ C5 C1: EB. D1
	vpxor	xmm1, xmm3, xmm6			; 0D27 _ C5 E1: EF. CE
	vpor	xmm6, xmm6, xmm4			; 0D2B _ C5 C9: EB. F4
	vpand	xmm3, xmm6, xmm2			; 0D2F _ C5 C9: DB. DA
	vpxor	xmm0, xmm0, xmm1			; 0D33 _ C5 F9: EF. C1
	vpxor	xmm5, xmm0, xmm3			; 0D37 _ C5 F9: EF. EB
	vpcmpeqd xmm7, xmm7, xmm7			; 0D3B _ C5 C1: 76. FF
	vpxor	xmm0, xmm4, xmm5			; 0D3F _ C5 D9: EF. C5
	vpand	xmm4, xmm3, xmm1			; 0D43 _ C5 E1: DB. E1
	vpxor	xmm2, xmm2, xmm7			; 0D47 _ C5 E9: EF. D7
	vpxor	xmm6, xmm4, xmm0			; 0D4B _ C5 D9: EF. F0
	vpxor	xmm3, xmm2, xmm6			; 0D4F _ C5 E9: EF. DE
	vpand	xmm2, xmm0, xmm2			; 0D53 _ C5 F9: DB. D2
	vpxor	xmm4, xmm2, xmm1			; 0D57 _ C5 E9: EF. E1
	vpand	xmm1, xmm1, xmm3			; 0D5B _ C5 F1: DB. CB
	vmovd	xmm2, dword [ecx+2BC0H] 		; 0D5F _ C5 F9: 6E. 91, 00002BC0
	vpxor	xmm7, xmm4, xmm3			; 0D67 _ C5 D9: EF. FB
	vpshufd xmm0, xmm2, 0				; 0D6B _ C5 F9: 70. C2, 00
	vpxor	xmm2, xmm7, xmm3			; 0D70 _ C5 C1: EF. D3
	vpxor	xmm4, xmm3, xmm0			; 0D74 _ C5 E1: EF. E0
	vmovd	xmm0, dword [ecx+2BC4H] 		; 0D78 _ C5 F9: 6E. 81, 00002BC4
	vpshufd xmm0, xmm0, 0				; 0D80 _ C5 F9: 70. C0, 00
	vpxor	xmm0, xmm2, xmm0			; 0D85 _ C5 E9: EF. C0
	vpxor	xmm2, xmm1, xmm5			; 0D89 _ C5 F1: EF. D5
	vmovd	xmm1, dword [ecx+2BC8H] 		; 0D8D _ C5 F9: 6E. 89, 00002BC8
	vpor	xmm3, xmm2, xmm7			; 0D95 _ C5 E9: EB. DF
	vmovd	xmm2, dword [ecx+2BCCH] 		; 0D99 _ C5 F9: 6E. 91, 00002BCC
	vpxor	xmm6, xmm3, xmm6			; 0DA1 _ C5 E1: EF. F6
	vpshufd xmm7, xmm1, 0				; 0DA5 _ C5 F9: 70. F9, 00
	vpshufd xmm3, xmm2, 0				; 0DAA _ C5 F9: 70. DA, 00
	vpxor	xmm1, xmm6, xmm7			; 0DAF _ C5 C9: EF. CF
	vpxor	xmm6, xmm5, xmm3			; 0DB3 _ C5 D1: EF. F3
	vpslld	xmm5, xmm1, 10				; 0DB7 _ C5 D1: 72. F1, 0A
	vpsrld	xmm2, xmm1, 22				; 0DBC _ C5 E9: 72. D1, 16
	vpslld	xmm7, xmm4, 27				; 0DC1 _ C5 C1: 72. F4, 1B
	vpor	xmm3, xmm5, xmm2			; 0DC6 _ C5 D1: EB. DA
	vpsrld	xmm4, xmm4, 5				; 0DCA _ C5 D9: 72. D4, 05
	vpxor	xmm1, xmm3, xmm6			; 0DCF _ C5 E1: EF. CE
	vpslld	xmm5, xmm0, 7				; 0DD3 _ C5 D1: 72. F0, 07
	vpor	xmm3, xmm7, xmm4			; 0DD8 _ C5 C1: EB. DC
	vpxor	xmm2, xmm1, xmm5			; 0DDC _ C5 F1: EF. D5
	vpxor	xmm1, xmm3, xmm0			; 0DE0 _ C5 E1: EF. C8
	vpslld	xmm4, xmm6, 25				; 0DE4 _ C5 D9: 72. F6, 19
	vpxor	xmm3, xmm1, xmm6			; 0DE9 _ C5 F1: EF. DE
	vpsrld	xmm6, xmm6, 7				; 0DED _ C5 C9: 72. D6, 07
	vpor	xmm1, xmm4, xmm6			; 0DF2 _ C5 D9: EB. CE
	vpslld	xmm6, xmm0, 31				; 0DF6 _ C5 C9: 72. F0, 1F
	vpsrld	xmm0, xmm0, 1				; 0DFB _ C5 F9: 72. D0, 01
	vpxor	xmm4, xmm1, xmm2			; 0E00 _ C5 F1: EF. E2
	vpslld	xmm5, xmm3, 3				; 0E04 _ C5 D1: 72. F3, 03
	vpor	xmm0, xmm6, xmm0			; 0E09 _ C5 C9: EB. C0
	vpxor	xmm1, xmm4, xmm5			; 0E0D _ C5 D9: EF. CD
	vpxor	xmm4, xmm0, xmm3			; 0E11 _ C5 F9: EF. E3
	vpxor	xmm0, xmm4, xmm2			; 0E15 _ C5 D9: EF. C2
	vpslld	xmm4, xmm2, 29				; 0E19 _ C5 D9: 72. F2, 1D
	vpsrld	xmm5, xmm2, 3				; 0E1E _ C5 D1: 72. D2, 03
	vpslld	xmm2, xmm3, 19				; 0E23 _ C5 E9: 72. F3, 13
	vpor	xmm6, xmm4, xmm5			; 0E28 _ C5 D9: EB. F5
	vpsrld	xmm3, xmm3, 13				; 0E2C _ C5 E1: 72. D3, 0D
	vpxor	xmm4, xmm6, xmm0			; 0E31 _ C5 C9: EF. E0
	vpor	xmm7, xmm2, xmm3			; 0E35 _ C5 E9: EB. FB
	vpxor	xmm2, xmm7, xmm4			; 0E39 _ C5 C1: EF. D4
	vpand	xmm6, xmm6, xmm4			; 0E3D _ C5 C9: DB. F4
	vpxor	xmm6, xmm6, xmm2			; 0E41 _ C5 C9: EF. F2
	vpand	xmm5, xmm2, xmm0			; 0E45 _ C5 E9: DB. E8
	vpor	xmm7, xmm1, xmm6			; 0E49 _ C5 F1: EB. FE
	vpxor	xmm0, xmm0, xmm1			; 0E4D _ C5 F9: EF. C1
	vpxor	xmm2, xmm5, xmm7			; 0E51 _ C5 D1: EF. D7
	vpxor	xmm5, xmm0, xmm6			; 0E55 _ C5 F9: EF. EE
	vpxor	xmm4, xmm4, xmm7			; 0E59 _ C5 D9: EF. E7
	vpxor	xmm3, xmm5, xmm2			; 0E5D _ C5 D1: EF. DA
	vpand	xmm0, xmm7, xmm4			; 0E61 _ C5 C1: DB. C4
	vpor	xmm7, xmm3, xmm4			; 0E65 _ C5 E1: EB. FC
	vpxor	xmm3, xmm7, xmm6			; 0E69 _ C5 C1: EF. DE
	vpxor	xmm1, xmm0, xmm5			; 0E6D _ C5 F9: EF. CD
	vmovd	xmm6, dword [ecx+2BB0H] 		; 0E71 _ C5 F9: 6E. B1, 00002BB0
	vpshufd xmm6, xmm6, 0				; 0E79 _ C5 F9: 70. F6, 00
	vpxor	xmm5, xmm4, xmm6			; 0E7E _ C5 D9: EF. EE
	vmovd	xmm4, dword [ecx+2BB4H] 		; 0E82 _ C5 F9: 6E. A1, 00002BB4
	vpshufd xmm6, xmm4, 0				; 0E8A _ C5 F9: 70. F4, 00
	vmovd	xmm4, dword [ecx+2BB8H] 		; 0E8F _ C5 F9: 6E. A1, 00002BB8
	vpxor	xmm0, xmm3, xmm6			; 0E97 _ C5 E1: EF. C6
	vpshufd xmm7, xmm4, 0				; 0E9B _ C5 F9: 70. FC, 00
	vmovd	xmm6, dword [ecx+2BBCH] 		; 0EA0 _ C5 F9: 6E. B1, 00002BBC
	vpxor	xmm4, xmm1, xmm7			; 0EA8 _ C5 F1: EF. E7
	vpxor	xmm1, xmm2, xmm1			; 0EAC _ C5 E9: EF. C9
	vpxor	xmm2, xmm1, xmm3			; 0EB0 _ C5 F1: EF. D3
	vpslld	xmm1, xmm4, 10				; 0EB4 _ C5 F1: 72. F4, 0A
	vpshufd xmm3, xmm6, 0				; 0EB9 _ C5 F9: 70. DE, 00
	vpsrld	xmm4, xmm4, 22				; 0EBE _ C5 D9: 72. D4, 16
	vpxor	xmm6, xmm2, xmm3			; 0EC3 _ C5 E9: EF. F3
	vpor	xmm7, xmm1, xmm4			; 0EC7 _ C5 F1: EB. FC
	vpxor	xmm4, xmm7, xmm6			; 0ECB _ C5 C1: EF. E6
	vpslld	xmm2, xmm0, 7				; 0ECF _ C5 E9: 72. F0, 07
	vpslld	xmm3, xmm5, 27				; 0ED4 _ C5 E1: 72. F5, 1B
	vpsrld	xmm5, xmm5, 5				; 0ED9 _ C5 D1: 72. D5, 05
	vpxor	xmm7, xmm4, xmm2			; 0EDE _ C5 D9: EF. FA
	vpor	xmm4, xmm3, xmm5			; 0EE2 _ C5 E1: EB. E5
	vpxor	xmm5, xmm4, xmm0			; 0EE6 _ C5 D9: EF. E8
	vpslld	xmm1, xmm6, 25				; 0EEA _ C5 F1: 72. F6, 19
	vpxor	xmm5, xmm5, xmm6			; 0EEF _ C5 D1: EF. EE
	vpsrld	xmm6, xmm6, 7				; 0EF3 _ C5 C9: 72. D6, 07
	vpor	xmm6, xmm1, xmm6			; 0EF8 _ C5 F1: EB. F6
	vpslld	xmm3, xmm0, 31				; 0EFC _ C5 E1: 72. F0, 1F
	vpsrld	xmm0, xmm0, 1				; 0F01 _ C5 F9: 72. D0, 01
	vpxor	xmm4, xmm6, xmm7			; 0F06 _ C5 C9: EF. E7
	vpslld	xmm2, xmm5, 3				; 0F0A _ C5 E9: 72. F5, 03
	vpor	xmm6, xmm3, xmm0			; 0F0F _ C5 E1: EB. F0
	vpxor	xmm1, xmm4, xmm2			; 0F13 _ C5 D9: EF. CA
	vpxor	xmm4, xmm6, xmm5			; 0F17 _ C5 C9: EF. E5
	vpslld	xmm0, xmm5, 19				; 0F1B _ C5 F9: 72. F5, 13
	vpsrld	xmm5, xmm5, 13				; 0F20 _ C5 D1: 72. D5, 0D
	vpslld	xmm2, xmm7, 29				; 0F25 _ C5 E9: 72. F7, 1D
	vpsrld	xmm3, xmm7, 3				; 0F2A _ C5 E1: 72. D7, 03
	vpxor	xmm6, xmm4, xmm7			; 0F2F _ C5 D9: EF. F7
	vpor	xmm5, xmm0, xmm5			; 0F33 _ C5 F9: EB. ED
	vpor	xmm7, xmm2, xmm3			; 0F37 _ C5 E9: EB. FB
	vpxor	xmm0, xmm1, xmm5			; 0F3B _ C5 F1: EF. C5
	vpxor	xmm2, xmm7, xmm1			; 0F3F _ C5 C1: EF. D1
	vpand	xmm4, xmm0, xmm2			; 0F43 _ C5 F9: DB. E2
	vpxor	xmm4, xmm4, xmm6			; 0F47 _ C5 D9: EF. E6
	vpor	xmm6, xmm6, xmm2			; 0F4B _ C5 C9: EB. F2
	vpxor	xmm3, xmm2, xmm4			; 0F4F _ C5 E9: EF. DC
	vpand	xmm1, xmm0, xmm4			; 0F53 _ C5 F9: DB. CC
	vpcmpeqd xmm2, xmm2, xmm2			; 0F57 _ C5 E9: 76. D2
	vpxor	xmm6, xmm6, xmm0			; 0F5B _ C5 C9: EF. F0
	vpand	xmm0, xmm1, xmm5			; 0F5F _ C5 F1: DB. C5
	vpxor	xmm7, xmm4, xmm2			; 0F63 _ C5 D9: EF. FA
	vmovd	xmm4, dword [ecx+2BA0H] 		; 0F67 _ C5 F9: 6E. A1, 00002BA0
	vpxor	xmm1, xmm0, xmm3			; 0F6F _ C5 F9: EF. CB
	vpshufd xmm0, xmm4, 0				; 0F73 _ C5 F9: 70. C4, 00
	vpand	xmm3, xmm3, xmm6			; 0F78 _ C5 E1: DB. DE
	vmovd	xmm4, dword [ecx+2BA4H] 		; 0F7C _ C5 F9: 6E. A1, 00002BA4
	vpxor	xmm2, xmm6, xmm0			; 0F84 _ C5 C9: EF. D0
	vpshufd xmm0, xmm4, 0				; 0F88 _ C5 F9: 70. C4, 00
	vpor	xmm4, xmm3, xmm5			; 0F8D _ C5 E1: EB. E5
	vmovd	xmm3, dword [ecx+2BA8H] 		; 0F91 _ C5 F9: 6E. 99, 00002BA8
	vpxor	xmm4, xmm4, xmm7			; 0F99 _ C5 D9: EF. E7
	vpshufd xmm3, xmm3, 0				; 0F9D _ C5 F9: 70. DB, 00
	vpxor	xmm5, xmm5, xmm7			; 0FA2 _ C5 D1: EF. EF
	vpxor	xmm4, xmm4, xmm3			; 0FA6 _ C5 D9: EF. E3
	vpxor	xmm3, xmm7, xmm1			; 0FAA _ C5 C1: EF. D9
	vpand	xmm7, xmm5, xmm6			; 0FAE _ C5 D1: DB. FE
	vpxor	xmm0, xmm1, xmm0			; 0FB2 _ C5 F1: EF. C0
	vmovd	xmm6, dword [ecx+2BACH] 		; 0FB6 _ C5 F9: 6E. B1, 00002BAC
	vpxor	xmm5, xmm3, xmm7			; 0FBE _ C5 E1: EF. EF
	vpshufd xmm3, xmm6, 0				; 0FC2 _ C5 F9: 70. DE, 00
	vpslld	xmm1, xmm4, 10				; 0FC7 _ C5 F1: 72. F4, 0A
	vpsrld	xmm4, xmm4, 22				; 0FCC _ C5 D9: 72. D4, 16
	vpxor	xmm6, xmm5, xmm3			; 0FD1 _ C5 D1: EF. F3
	vpor	xmm7, xmm1, xmm4			; 0FD5 _ C5 F1: EB. FC
	vpslld	xmm5, xmm0, 7				; 0FD9 _ C5 D1: 72. F0, 07
	vpxor	xmm4, xmm7, xmm6			; 0FDE _ C5 C1: EF. E6
	vpslld	xmm3, xmm2, 27				; 0FE2 _ C5 E1: 72. F2, 1B
	vpsrld	xmm2, xmm2, 5				; 0FE7 _ C5 E9: 72. D2, 05
	vpxor	xmm4, xmm4, xmm5			; 0FEC _ C5 D9: EF. E5
	vpor	xmm5, xmm3, xmm2			; 0FF0 _ C5 E1: EB. EA
	vpslld	xmm3, xmm6, 25				; 0FF4 _ C5 E1: 72. F6, 19
	vpxor	xmm2, xmm5, xmm0			; 0FF9 _ C5 D1: EF. D0
	vpxor	xmm1, xmm2, xmm6			; 0FFD _ C5 E9: EF. CE
	vpsrld	xmm6, xmm6, 7				; 1001 _ C5 C9: 72. D6, 07
	vpor	xmm7, xmm3, xmm6			; 1006 _ C5 E1: EB. FE
	vpslld	xmm5, xmm1, 3				; 100A _ C5 D1: 72. F1, 03
	vpxor	xmm6, xmm7, xmm4			; 100F _ C5 C1: EF. F4
	vpslld	xmm2, xmm0, 31				; 1013 _ C5 E9: 72. F0, 1F
	vpsrld	xmm0, xmm0, 1				; 1018 _ C5 F9: 72. D0, 01
	vpxor	xmm3, xmm6, xmm5			; 101D _ C5 C9: EF. DD
	vpor	xmm6, xmm2, xmm0			; 1021 _ C5 E9: EB. F0
	vpslld	xmm5, xmm4, 29				; 1025 _ C5 D1: 72. F4, 1D
	vpxor	xmm0, xmm6, xmm1			; 102A _ C5 C9: EF. C1
	vpslld	xmm2, xmm1, 19				; 102E _ C5 E9: 72. F1, 13
	vpxor	xmm6, xmm0, xmm4			; 1033 _ C5 F9: EF. F4
	vpsrld	xmm4, xmm4, 3				; 1037 _ C5 D9: 72. D4, 03
	vpsrld	xmm1, xmm1, 13				; 103C _ C5 F1: 72. D1, 0D
	vpor	xmm7, xmm5, xmm4			; 1041 _ C5 D1: EB. FC
	vpor	xmm0, xmm2, xmm1			; 1045 _ C5 E9: EB. C1
	vpxor	xmm1, xmm6, xmm3			; 1049 _ C5 C9: EF. CB
	vpxor	xmm5, xmm6, xmm7			; 104D _ C5 C9: EF. EF
	vpand	xmm6, xmm3, xmm1			; 1051 _ C5 E1: DB. F1
	vpxor	xmm4, xmm6, xmm0			; 1055 _ C5 C9: EF. E0
	vpor	xmm0, xmm0, xmm1			; 1059 _ C5 F9: EB. C1
	vpxor	xmm7, xmm7, xmm4			; 105D _ C5 C1: EF. FC
	vpxor	xmm2, xmm0, xmm5			; 1061 _ C5 F9: EF. D5
	vpxor	xmm6, xmm1, xmm4			; 1065 _ C5 F1: EF. F4
	vpor	xmm3, xmm2, xmm7			; 1069 _ C5 E9: EB. DF
	vpxor	xmm2, xmm3, xmm6			; 106D _ C5 E1: EF. D6
	vpor	xmm1, xmm6, xmm4			; 1071 _ C5 C9: EB. CC
	vpcmpeqd xmm0, xmm0, xmm0			; 1075 _ C5 F9: 76. C0
	vpxor	xmm6, xmm1, xmm2			; 1079 _ C5 F1: EF. F2
	vpxor	xmm5, xmm5, xmm0			; 107D _ C5 D1: EF. E8
	vpxor	xmm3, xmm5, xmm6			; 1081 _ C5 D1: EF. DE
	vpor	xmm6, xmm6, xmm2			; 1085 _ C5 C9: EB. F2
	vmovd	xmm5, dword [ecx+2B90H] 		; 1089 _ C5 F9: 6E. A9, 00002B90
	vpshufd xmm1, xmm5, 0				; 1091 _ C5 F9: 70. CD, 00
	vpxor	xmm5, xmm3, xmm1			; 1096 _ C5 E1: EF. E9
	vmovd	xmm1, dword [ecx+2B94H] 		; 109A _ C5 F9: 6E. 89, 00002B94
	vpshufd xmm1, xmm1, 0				; 10A2 _ C5 F9: 70. C9, 00
	vpxor	xmm1, xmm2, xmm1			; 10A7 _ C5 E9: EF. C9
	vpxor	xmm2, xmm6, xmm2			; 10AB _ C5 C9: EF. D2
	vpor	xmm3, xmm2, xmm3			; 10AF _ C5 E9: EB. DB
	vpxor	xmm6, xmm4, xmm3			; 10B3 _ C5 D9: EF. F3
	vmovd	xmm4, dword [ecx+2B98H] 		; 10B7 _ C5 F9: 6E. A1, 00002B98
	vmovd	xmm2, dword [ecx+2B9CH] 		; 10BF _ C5 F9: 6E. 91, 00002B9C
	vpshufd xmm4, xmm4, 0				; 10C7 _ C5 F9: 70. E4, 00
	vpxor	xmm3, xmm6, xmm4			; 10CC _ C5 C9: EF. DC
	vpshufd xmm6, xmm2, 0				; 10D0 _ C5 F9: 70. F2, 00
	vpxor	xmm4, xmm7, xmm6			; 10D5 _ C5 C1: EF. E6
	vpslld	xmm7, xmm3, 10				; 10D9 _ C5 C1: 72. F3, 0A
	vpsrld	xmm6, xmm3, 22				; 10DE _ C5 C9: 72. D3, 16
	vpor	xmm2, xmm7, xmm6			; 10E3 _ C5 C1: EB. D6
	vpslld	xmm6, xmm5, 27				; 10E7 _ C5 C9: 72. F5, 1B
	vpsrld	xmm5, xmm5, 5				; 10EC _ C5 D1: 72. D5, 05
	vpxor	xmm3, xmm2, xmm4			; 10F1 _ C5 E9: EF. DC
	vpor	xmm6, xmm6, xmm5			; 10F5 _ C5 C9: EB. F5
	vpslld	xmm7, xmm1, 7				; 10F9 _ C5 C1: 72. F1, 07
	vpxor	xmm5, xmm6, xmm1			; 10FE _ C5 C9: EF. E9
	vpxor	xmm7, xmm3, xmm7			; 1102 _ C5 E1: EF. FF
	vpxor	xmm2, xmm5, xmm4			; 1106 _ C5 D1: EF. D4
	vpslld	xmm3, xmm4, 25				; 110A _ C5 E1: 72. F4, 19
	vpsrld	xmm4, xmm4, 7				; 110F _ C5 D9: 72. D4, 07
	vpslld	xmm5, xmm2, 3				; 1114 _ C5 D1: 72. F2, 03
	vpor	xmm6, xmm3, xmm4			; 1119 _ C5 E1: EB. F4
	vpslld	xmm3, xmm1, 31				; 111D _ C5 E1: 72. F1, 1F
	vpsrld	xmm1, xmm1, 1				; 1122 _ C5 F1: 72. D1, 01
	vpxor	xmm4, xmm6, xmm7			; 1127 _ C5 C9: EF. E7
	vpor	xmm6, xmm3, xmm1			; 112B _ C5 E1: EB. F1
	vpxor	xmm4, xmm4, xmm5			; 112F _ C5 D9: EF. E5
	vpxor	xmm5, xmm6, xmm2			; 1133 _ C5 C9: EF. EA
	vpslld	xmm3, xmm2, 19				; 1137 _ C5 E1: 72. F2, 13
	vpxor	xmm5, xmm5, xmm7			; 113C _ C5 D1: EF. EF
	vpsrld	xmm2, xmm2, 13				; 1140 _ C5 E9: 72. D2, 0D
	vpslld	xmm1, xmm7, 29				; 1145 _ C5 F1: 72. F7, 1D
	vpsrld	xmm7, xmm7, 3				; 114A _ C5 C1: 72. D7, 03
	vpor	xmm6, xmm3, xmm2			; 114F _ C5 E1: EB. F2
	vpor	xmm2, xmm1, xmm7			; 1153 _ C5 F1: EB. D7
	vpxor	xmm3, xmm2, xmm0			; 1157 _ C5 E9: EF. D8
	vpxor	xmm1, xmm5, xmm0			; 115B _ C5 D1: EF. C8
	vpor	xmm5, xmm5, xmm6			; 115F _ C5 D1: EB. EE
	vpxor	xmm7, xmm5, xmm3			; 1163 _ C5 D1: EF. FB
	vpxor	xmm5, xmm6, xmm1			; 1167 _ C5 C9: EF. E9
	vpxor	xmm2, xmm7, xmm4			; 116B _ C5 C1: EF. D4
	vpor	xmm6, xmm3, xmm1			; 116F _ C5 E1: EB. F1
	vpand	xmm3, xmm5, xmm4			; 1173 _ C5 D1: DB. DC
	vpxor	xmm6, xmm6, xmm5			; 1177 _ C5 C9: EF. F5
	vpxor	xmm7, xmm1, xmm3			; 117B _ C5 F1: EF. FB
	vpor	xmm1, xmm3, xmm2			; 117F _ C5 E1: EB. CA
	vpxor	xmm5, xmm1, xmm6			; 1183 _ C5 F1: EF. EE
	vpxor	xmm4, xmm4, xmm7			; 1187 _ C5 D9: EF. E7
	vmovd	xmm1, dword [ecx+2B80H] 		; 118B _ C5 F9: 6E. 89, 00002B80
	vpxor	xmm4, xmm4, xmm5			; 1193 _ C5 D9: EF. E5
	vpxor	xmm3, xmm4, xmm2			; 1197 _ C5 D9: EF. DA
	vpxor	xmm6, xmm6, xmm2			; 119B _ C5 C9: EF. F2
	vpshufd xmm4, xmm1, 0				; 119F _ C5 F9: 70. E1, 00
	vpxor	xmm1, xmm5, xmm4			; 11A4 _ C5 D1: EF. CC
	vpand	xmm5, xmm6, xmm3			; 11A8 _ C5 C9: DB. EB
	vpxor	xmm6, xmm7, xmm5			; 11AC _ C5 C1: EF. F5
	vmovd	xmm7, dword [ecx+2B84H] 		; 11B0 _ C5 F9: 6E. B9, 00002B84
	vpshufd xmm4, xmm7, 0				; 11B8 _ C5 F9: 70. E7, 00
	vmovd	xmm7, dword [ecx+2B88H] 		; 11BD _ C5 F9: 6E. B9, 00002B88
	vpxor	xmm5, xmm6, xmm4			; 11C5 _ C5 C9: EF. EC
	vmovd	xmm4, dword [ecx+2B8CH] 		; 11C9 _ C5 F9: 6E. A1, 00002B8C
	vpshufd xmm6, xmm7, 0				; 11D1 _ C5 F9: 70. F7, 00
	vpshufd xmm7, xmm4, 0				; 11D6 _ C5 F9: 70. FC, 00
	vpxor	xmm2, xmm2, xmm6			; 11DB _ C5 E9: EF. D6
	vpxor	xmm4, xmm3, xmm7			; 11DF _ C5 E1: EF. E7
	vpslld	xmm3, xmm2, 10				; 11E3 _ C5 E1: 72. F2, 0A
	vpsrld	xmm2, xmm2, 22				; 11E8 _ C5 E9: 72. D2, 16
	vpslld	xmm7, xmm1, 27				; 11ED _ C5 C1: 72. F1, 1B
	vpor	xmm6, xmm3, xmm2			; 11F2 _ C5 E1: EB. F2
	vpslld	xmm3, xmm5, 7				; 11F6 _ C5 E1: 72. F5, 07
	vpxor	xmm2, xmm6, xmm4			; 11FB _ C5 C9: EF. D4
	vpsrld	xmm1, xmm1, 5				; 11FF _ C5 F1: 72. D1, 05
	vpxor	xmm6, xmm2, xmm3			; 1204 _ C5 E9: EF. F3
	vpor	xmm2, xmm7, xmm1			; 1208 _ C5 C1: EB. D1
	vpxor	xmm3, xmm2, xmm5			; 120C _ C5 E9: EF. DD
	vpslld	xmm1, xmm4, 25				; 1210 _ C5 F1: 72. F4, 19
	vpxor	xmm7, xmm3, xmm4			; 1215 _ C5 E1: EF. FC
	vpsrld	xmm4, xmm4, 7				; 1219 _ C5 D9: 72. D4, 07
	vpor	xmm4, xmm1, xmm4			; 121E _ C5 F1: EB. E4
	vpslld	xmm1, xmm5, 31				; 1222 _ C5 F1: 72. F5, 1F
	vpsrld	xmm5, xmm5, 1				; 1227 _ C5 D1: 72. D5, 01
	vpxor	xmm2, xmm4, xmm6			; 122C _ C5 D9: EF. D6
	vpslld	xmm3, xmm7, 3				; 1230 _ C5 E1: 72. F7, 03
	vpor	xmm5, xmm1, xmm5			; 1235 _ C5 F1: EB. ED
	vpxor	xmm4, xmm2, xmm3			; 1239 _ C5 E9: EF. E3
	vpxor	xmm2, xmm5, xmm7			; 123D _ C5 D1: EF. D7
	vpxor	xmm1, xmm2, xmm6			; 1241 _ C5 E9: EF. CE
	vpslld	xmm3, xmm6, 29				; 1245 _ C5 E1: 72. F6, 1D
	vpsrld	xmm6, xmm6, 3				; 124A _ C5 C9: 72. D6, 03
	vpor	xmm2, xmm3, xmm6			; 124F _ C5 E1: EB. D6
	vpslld	xmm6, xmm7, 19				; 1253 _ C5 C9: 72. F7, 13
	vpsrld	xmm7, xmm7, 13				; 1258 _ C5 C1: 72. D7, 0D
	vpor	xmm3, xmm6, xmm7			; 125D _ C5 C9: EB. DF
	vpor	xmm6, xmm2, xmm4			; 1261 _ C5 E9: EB. F4
	vpxor	xmm2, xmm2, xmm3			; 1265 _ C5 E9: EF. D3
	vpand	xmm5, xmm3, xmm4			; 1269 _ C5 E1: DB. EC
	vpxor	xmm7, xmm2, xmm0			; 126D _ C5 E9: EF. F8
	vpxor	xmm4, xmm4, xmm1			; 1271 _ C5 D9: EF. E1
	vpxor	xmm3, xmm5, xmm7			; 1275 _ C5 D1: EF. DF
	vpand	xmm7, xmm7, xmm6			; 1279 _ C5 C1: DB. FE
	vpor	xmm1, xmm1, xmm5			; 127D _ C5 F1: EB. CD
	vpand	xmm2, xmm4, xmm6			; 1281 _ C5 D9: DB. D6
	vpxor	xmm5, xmm1, xmm7			; 1285 _ C5 F1: EF. EF
	vpxor	xmm4, xmm7, xmm3			; 1289 _ C5 C1: EF. E3
	vpxor	xmm1, xmm6, xmm5			; 128D _ C5 C9: EF. CD
	vpor	xmm6, xmm3, xmm4			; 1291 _ C5 E1: EB. F4
	vmovd	xmm3, dword [ecx+2B70H] 		; 1295 _ C5 F9: 6E. 99, 00002B70
	vpxor	xmm7, xmm6, xmm2			; 129D _ C5 C9: EF. FA
	vpxor	xmm2, xmm2, xmm1			; 12A1 _ C5 E9: EF. D1
	vpxor	xmm6, xmm2, xmm4			; 12A5 _ C5 E9: EF. F4
	vpshufd xmm2, xmm3, 0				; 12A9 _ C5 F9: 70. D3, 00
	vmovd	xmm3, dword [ecx+2B74H] 		; 12AE _ C5 F9: 6E. 99, 00002B74
	vpxor	xmm6, xmm6, xmm2			; 12B6 _ C5 C9: EF. F2
	vpshufd xmm2, xmm3, 0				; 12BA _ C5 F9: 70. D3, 00
	vmovd	xmm3, dword [ecx+2B78H] 		; 12BF _ C5 F9: 6E. 99, 00002B78
	vpxor	xmm2, xmm7, xmm2			; 12C7 _ C5 C1: EF. D2
	vpshufd xmm3, xmm3, 0				; 12CB _ C5 F9: 70. DB, 00
	vpxor	xmm3, xmm5, xmm3			; 12D0 _ C5 D1: EF. DB
	vpor	xmm5, xmm1, xmm7			; 12D4 _ C5 F1: EB. EF
	vpxor	xmm1, xmm5, xmm4			; 12D8 _ C5 D1: EF. CC
	vpslld	xmm5, xmm3, 10				; 12DC _ C5 D1: 72. F3, 0A
	vmovd	xmm4, dword [ecx+2B7CH] 		; 12E1 _ C5 F9: 6E. A1, 00002B7C
	vpsrld	xmm3, xmm3, 22				; 12E9 _ C5 E1: 72. D3, 16
	vpshufd xmm7, xmm4, 0				; 12EE _ C5 F9: 70. FC, 00
	vpxor	xmm4, xmm1, xmm7			; 12F3 _ C5 F1: EF. E7
	vpor	xmm1, xmm5, xmm3			; 12F7 _ C5 D1: EB. CB
	vpslld	xmm3, xmm6, 27				; 12FB _ C5 E1: 72. F6, 1B
	vpsrld	xmm6, xmm6, 5				; 1300 _ C5 C9: 72. D6, 05
	vpxor	xmm7, xmm1, xmm4			; 1305 _ C5 F1: EF. FC
	vpslld	xmm5, xmm2, 7				; 1309 _ C5 D1: 72. F2, 07
	vpor	xmm6, xmm3, xmm6			; 130E _ C5 E1: EB. F6
	vpxor	xmm1, xmm7, xmm5			; 1312 _ C5 C1: EF. CD
	vpxor	xmm5, xmm6, xmm2			; 1316 _ C5 C9: EF. EA
	vpslld	xmm3, xmm4, 25				; 131A _ C5 E1: 72. F4, 19
	vpxor	xmm5, xmm5, xmm4			; 131F _ C5 D1: EF. EC
	vpsrld	xmm4, xmm4, 7				; 1323 _ C5 D9: 72. D4, 07
	vpor	xmm6, xmm3, xmm4			; 1328 _ C5 E1: EB. F4
	vpslld	xmm7, xmm2, 31				; 132C _ C5 C1: 72. F2, 1F
	vpsrld	xmm2, xmm2, 1				; 1331 _ C5 E9: 72. D2, 01
	vpxor	xmm4, xmm6, xmm1			; 1336 _ C5 C9: EF. E1
	vpor	xmm6, xmm7, xmm2			; 133A _ C5 C1: EB. F2
	vpslld	xmm3, xmm5, 3				; 133E _ C5 E1: 72. F5, 03
	vpxor	xmm2, xmm6, xmm5			; 1343 _ C5 C9: EF. D5
	vpxor	xmm4, xmm4, xmm3			; 1347 _ C5 D9: EF. E3
	vpxor	xmm3, xmm2, xmm1			; 134B _ C5 E9: EF. D9
	vpslld	xmm7, xmm1, 29				; 134F _ C5 C1: 72. F1, 1D
	vpsrld	xmm1, xmm1, 3				; 1354 _ C5 F1: 72. D1, 03
	vpslld	xmm2, xmm5, 19				; 1359 _ C5 E9: 72. F5, 13
	vpsrld	xmm5, xmm5, 13				; 135E _ C5 D1: 72. D5, 0D
	vpor	xmm6, xmm7, xmm1			; 1363 _ C5 C1: EB. F1
	vpor	xmm5, xmm2, xmm5			; 1367 _ C5 E9: EB. ED
	vpxor	xmm1, xmm4, xmm3			; 136B _ C5 D9: EF. CB
	vpxor	xmm7, xmm5, xmm6			; 136F _ C5 D1: EF. FE
	vpand	xmm2, xmm6, xmm7			; 1373 _ C5 C9: DB. D7
	vpxor	xmm6, xmm6, xmm4			; 1377 _ C5 C9: EF. F4
	vpxor	xmm0, xmm2, xmm0			; 137B _ C5 E9: EF. C0
	vpor	xmm6, xmm6, xmm7			; 137F _ C5 C9: EB. F7
	vpxor	xmm2, xmm0, xmm1			; 1383 _ C5 F9: EF. D1
	vpxor	xmm4, xmm1, xmm6			; 1387 _ C5 F1: EF. E6
	vmovd	xmm5, dword [ecx+2B60H] 		; 138B _ C5 F9: 6E. A9, 00002B60
	vpxor	xmm7, xmm7, xmm2			; 1393 _ C5 C1: EF. FA
	vpand	xmm0, xmm3, xmm4			; 1397 _ C5 E1: DB. C4
	vpxor	xmm6, xmm6, xmm3			; 139B _ C5 C9: EF. F3
	vpxor	xmm1, xmm0, xmm7			; 139F _ C5 F9: EF. CF
	vpxor	xmm3, xmm7, xmm4			; 13A3 _ C5 C1: EF. DC
	vpshufd xmm0, xmm5, 0				; 13A7 _ C5 F9: 70. C5, 00
	vpxor	xmm5, xmm1, xmm0			; 13AC _ C5 F1: EF. E8
	vmovd	xmm0, dword [ecx+2B64H] 		; 13B0 _ C5 F9: 6E. 81, 00002B64
	vpshufd xmm0, xmm0, 0				; 13B8 _ C5 F9: 70. C0, 00
	vpxor	xmm0, xmm2, xmm0			; 13BD _ C5 E9: EF. C0
	vpor	xmm2, xmm3, xmm2			; 13C1 _ C5 E1: EB. D2
	vpxor	xmm3, xmm6, xmm2			; 13C5 _ C5 C9: EF. DA
	vmovd	xmm6, dword [ecx+2B68H] 		; 13C9 _ C5 F9: 6E. B1, 00002B68
	vpshufd xmm7, xmm6, 0				; 13D1 _ C5 F9: 70. FE, 00
	vpxor	xmm6, xmm4, xmm1			; 13D6 _ C5 D9: EF. F1
	vmovd	xmm4, dword [ecx+2B6CH] 		; 13DA _ C5 F9: 6E. A1, 00002B6C
	vpxor	xmm3, xmm3, xmm7			; 13E2 _ C5 E1: EF. DF
	vpshufd xmm4, xmm4, 0				; 13E6 _ C5 F9: 70. E4, 00
	vpslld	xmm2, xmm3, 10				; 13EB _ C5 E9: 72. F3, 0A
	vpsrld	xmm3, xmm3, 22				; 13F0 _ C5 E1: 72. D3, 16
	vpxor	xmm6, xmm6, xmm4			; 13F5 _ C5 C9: EF. F4
	vpor	xmm1, xmm2, xmm3			; 13F9 _ C5 E9: EB. CB
	vpslld	xmm4, xmm0, 7				; 13FD _ C5 D9: 72. F0, 07
	vpxor	xmm7, xmm1, xmm6			; 1402 _ C5 F1: EF. FE
	vpslld	xmm2, xmm5, 27				; 1406 _ C5 E9: 72. F5, 1B
	vpsrld	xmm5, xmm5, 5				; 140B _ C5 D1: 72. D5, 05
	vpxor	xmm3, xmm7, xmm4			; 1410 _ C5 C1: EF. DC
	vpor	xmm4, xmm2, xmm5			; 1414 _ C5 E9: EB. E5
	vpslld	xmm1, xmm6, 25				; 1418 _ C5 F1: 72. F6, 19
	vpxor	xmm5, xmm4, xmm0			; 141D _ C5 D9: EF. E8
	vpxor	xmm5, xmm5, xmm6			; 1421 _ C5 D1: EF. EE
	vpsrld	xmm6, xmm6, 7				; 1425 _ C5 C9: 72. D6, 07
	vpor	xmm7, xmm1, xmm6			; 142A _ C5 F1: EB. FE
	vpslld	xmm4, xmm5, 3				; 142E _ C5 D9: 72. F5, 03
	vpxor	xmm6, xmm7, xmm3			; 1433 _ C5 C1: EF. F3
	vpslld	xmm2, xmm5, 19				; 1437 _ C5 E9: 72. F5, 13
	vpxor	xmm1, xmm6, xmm4			; 143C _ C5 C9: EF. CC
	vpslld	xmm4, xmm0, 31				; 1440 _ C5 D9: 72. F0, 1F
	vpsrld	xmm0, xmm0, 1				; 1445 _ C5 F9: 72. D0, 01
	vpsrld	xmm7, xmm5, 13				; 144A _ C5 C1: 72. D5, 0D
	vpor	xmm4, xmm4, xmm0			; 144F _ C5 D9: EB. E0
	vpor	xmm6, xmm2, xmm7			; 1453 _ C5 E9: EB. F7
	vpxor	xmm0, xmm4, xmm5			; 1457 _ C5 D9: EF. C5
	vpslld	xmm2, xmm3, 29				; 145B _ C5 E9: 72. F3, 1D
	vpxor	xmm5, xmm0, xmm3			; 1460 _ C5 F9: EF. EB
	vpcmpeqd xmm4, xmm4, xmm4			; 1464 _ C5 D9: 76. E4
	vpsrld	xmm3, xmm3, 3				; 1468 _ C5 E1: 72. D3, 03
	vpxor	xmm5, xmm5, xmm4			; 146D _ C5 D1: EF. EC
	vpor	xmm7, xmm2, xmm3			; 1471 _ C5 E9: EB. FB
	vpor	xmm0, xmm1, xmm6			; 1475 _ C5 F1: EB. C6
	vpxor	xmm7, xmm7, xmm5			; 1479 _ C5 C1: EF. FD
	vpxor	xmm3, xmm0, xmm7			; 147D _ C5 F9: EF. DF
	vpxor	xmm2, xmm1, xmm3			; 1481 _ C5 F1: EF. D3
	vpor	xmm1, xmm7, xmm5			; 1485 _ C5 C1: EB. CD
	vpand	xmm0, xmm1, xmm6			; 1489 _ C5 F1: DB. C6
	vpxor	xmm0, xmm0, xmm2			; 148D _ C5 F9: EF. C2
	vpor	xmm2, xmm2, xmm6			; 1491 _ C5 E9: EB. D6
	vpand	xmm1, xmm5, xmm0			; 1495 _ C5 D1: DB. C8
	vpxor	xmm5, xmm2, xmm5			; 1499 _ C5 E9: EF. ED
	vpxor	xmm7, xmm1, xmm3			; 149D _ C5 F1: EF. FB
	vpxor	xmm1, xmm5, xmm0			; 14A1 _ C5 D1: EF. C8
	vmovd	xmm5, dword [ecx+2B50H] 		; 14A5 _ C5 F9: 6E. A9, 00002B50
	vpxor	xmm2, xmm1, xmm7			; 14AD _ C5 F1: EF. D7
	vpshufd xmm5, xmm5, 0				; 14B1 _ C5 F9: 70. ED, 00
	vpand	xmm3, xmm3, xmm1			; 14B6 _ C5 E1: DB. D9
	vpxor	xmm5, xmm7, xmm5			; 14BA _ C5 C1: EF. ED
	vpxor	xmm4, xmm2, xmm4			; 14BE _ C5 E9: EF. E4
	vmovd	xmm7, dword [ecx+2B54H] 		; 14C2 _ C5 F9: 6E. B9, 00002B54
	vpxor	xmm1, xmm3, xmm2			; 14CA _ C5 E1: EF. CA
	vpshufd xmm7, xmm7, 0				; 14CE _ C5 F9: 70. FF, 00
	vpxor	xmm1, xmm1, xmm6			; 14D3 _ C5 F1: EF. CE
	vpxor	xmm4, xmm4, xmm7			; 14D7 _ C5 D9: EF. E7
	vmovd	xmm2, dword [ecx+2B58H] 		; 14DB _ C5 F9: 6E. 91, 00002B58
	vpshufd xmm3, xmm2, 0				; 14E3 _ C5 F9: 70. DA, 00
	vpxor	xmm7, xmm1, xmm3			; 14E8 _ C5 F1: EF. FB
	vmovd	xmm1, dword [ecx+2B5CH] 		; 14EC _ C5 F9: 6E. 89, 00002B5C
	vpslld	xmm2, xmm7, 10				; 14F4 _ C5 E9: 72. F7, 0A
	vpsrld	xmm3, xmm7, 22				; 14F9 _ C5 E1: 72. D7, 16
	vpslld	xmm7, xmm4, 7				; 14FE _ C5 C1: 72. F4, 07
	vpshufd xmm6, xmm1, 0				; 1503 _ C5 F9: 70. F1, 00
	vpor	xmm1, xmm2, xmm3			; 1508 _ C5 E9: EB. CB
	vpslld	xmm2, xmm5, 27				; 150C _ C5 E9: 72. F5, 1B
	vpsrld	xmm5, xmm5, 5				; 1511 _ C5 D1: 72. D5, 05
	vpxor	xmm0, xmm0, xmm6			; 1516 _ C5 F9: EF. C6
	vpor	xmm2, xmm2, xmm5			; 151A _ C5 E9: EB. D5
	vpxor	xmm6, xmm1, xmm0			; 151E _ C5 F1: EF. F0
	vpxor	xmm3, xmm2, xmm4			; 1522 _ C5 E9: EF. DC
	vpxor	xmm7, xmm6, xmm7			; 1526 _ C5 C9: EF. FF
	vpxor	xmm6, xmm3, xmm0			; 152A _ C5 E1: EF. F0
	vpslld	xmm1, xmm0, 25				; 152E _ C5 F1: 72. F0, 19
	vpsrld	xmm0, xmm0, 7				; 1533 _ C5 F9: 72. D0, 07
	vpor	xmm5, xmm1, xmm0			; 1538 _ C5 F1: EB. E8
	vpslld	xmm0, xmm6, 3				; 153C _ C5 F9: 72. F6, 03
	vpxor	xmm2, xmm5, xmm7			; 1541 _ C5 D1: EF. D7
	vpslld	xmm3, xmm4, 31				; 1545 _ C5 E1: 72. F4, 1F
	vpsrld	xmm4, xmm4, 1				; 154A _ C5 D9: 72. D4, 01
	vpxor	xmm1, xmm2, xmm0			; 154F _ C5 E9: EF. C8
	vpor	xmm2, xmm3, xmm4			; 1553 _ C5 E1: EB. D4
	vpslld	xmm0, xmm7, 29				; 1557 _ C5 F9: 72. F7, 1D
	vpsrld	xmm5, xmm7, 3				; 155C _ C5 D1: 72. D7, 03
	vpxor	xmm4, xmm2, xmm6			; 1561 _ C5 E9: EF. E6
	vpor	xmm0, xmm0, xmm5			; 1565 _ C5 F9: EB. C5
	vpxor	xmm3, xmm4, xmm7			; 1569 _ C5 D9: EF. DF
	vpslld	xmm7, xmm6, 19				; 156D _ C5 C1: 72. F6, 13
	vpsrld	xmm6, xmm6, 13				; 1572 _ C5 C9: 72. D6, 0D
	vpand	xmm4, xmm0, xmm1			; 1577 _ C5 F9: DB. E1
	vpor	xmm2, xmm7, xmm6			; 157B _ C5 C1: EB. D6
	vpxor	xmm6, xmm4, xmm3			; 157F _ C5 D9: EF. F3
	vpor	xmm3, xmm3, xmm1			; 1583 _ C5 E1: EB. D9
	vpand	xmm3, xmm3, xmm2			; 1587 _ C5 E1: DB. DA
	vpxor	xmm0, xmm0, xmm6			; 158B _ C5 F9: EF. C6
	vpxor	xmm4, xmm0, xmm3			; 158F _ C5 F9: EF. E3
	vpcmpeqd xmm5, xmm5, xmm5			; 1593 _ C5 D1: 76. ED
	vpxor	xmm1, xmm1, xmm4			; 1597 _ C5 F1: EF. CC
	vpxor	xmm0, xmm2, xmm5			; 159B _ C5 E9: EF. C5
	vpand	xmm2, xmm3, xmm6			; 159F _ C5 E1: DB. D6
	vpxor	xmm3, xmm2, xmm1			; 15A3 _ C5 E9: EF. D9
	vpand	xmm7, xmm1, xmm0			; 15A7 _ C5 F1: DB. F8
	vmovd	xmm5, dword [ecx+2B40H] 		; 15AB _ C5 F9: 6E. A9, 00002B40
	vpxor	xmm2, xmm0, xmm3			; 15B3 _ C5 F9: EF. D3
	vpxor	xmm0, xmm7, xmm6			; 15B7 _ C5 C1: EF. C6
	vpand	xmm6, xmm6, xmm2			; 15BB _ C5 C9: DB. F2
	vpshufd xmm7, xmm5, 0				; 15BF _ C5 F9: 70. FD, 00
	vpxor	xmm1, xmm0, xmm2			; 15C4 _ C5 F9: EF. CA
	vpxor	xmm5, xmm2, xmm7			; 15C8 _ C5 E9: EF. EF
	vpxor	xmm7, xmm1, xmm2			; 15CC _ C5 F1: EF. FA
	vpxor	xmm2, xmm6, xmm4			; 15D0 _ C5 C9: EF. D4
	vmovd	xmm0, dword [ecx+2B44H] 		; 15D4 _ C5 F9: 6E. 81, 00002B44
	vpor	xmm1, xmm2, xmm1			; 15DC _ C5 E9: EB. C9
	vpshufd xmm0, xmm0, 0				; 15E0 _ C5 F9: 70. C0, 00
	vpxor	xmm2, xmm1, xmm3			; 15E5 _ C5 F1: EF. D3
	vmovd	xmm3, dword [ecx+2B48H] 		; 15E9 _ C5 F9: 6E. 99, 00002B48
	vpxor	xmm7, xmm7, xmm0			; 15F1 _ C5 C1: EF. F8
	vpshufd xmm0, xmm3, 0				; 15F5 _ C5 F9: 70. C3, 00
	vmovd	xmm3, dword [ecx+2B4CH] 		; 15FA _ C5 F9: 6E. 99, 00002B4C
	vpxor	xmm6, xmm2, xmm0			; 1602 _ C5 E9: EF. F0
	vpshufd xmm1, xmm3, 0				; 1606 _ C5 F9: 70. CB, 00
	vpsrld	xmm2, xmm6, 22				; 160B _ C5 E9: 72. D6, 16
	vpxor	xmm0, xmm4, xmm1			; 1610 _ C5 D9: EF. C1
	vpslld	xmm4, xmm6, 10				; 1614 _ C5 D9: 72. F6, 0A
	vpor	xmm4, xmm4, xmm2			; 1619 _ C5 D9: EB. E2
	vpslld	xmm6, xmm5, 27				; 161D _ C5 C9: 72. F5, 1B
	vpsrld	xmm5, xmm5, 5				; 1622 _ C5 D1: 72. D5, 05
	vpxor	xmm3, xmm4, xmm0			; 1627 _ C5 D9: EF. D8
	vpslld	xmm1, xmm7, 7				; 162B _ C5 F1: 72. F7, 07
	vpor	xmm2, xmm6, xmm5			; 1630 _ C5 C9: EB. D5
	vpxor	xmm4, xmm3, xmm1			; 1634 _ C5 E1: EF. E1
	vpxor	xmm3, xmm2, xmm7			; 1638 _ C5 E9: EF. DF
	vpxor	xmm5, xmm3, xmm0			; 163C _ C5 E1: EF. E8
	vpslld	xmm1, xmm0, 25				; 1640 _ C5 F1: 72. F0, 19
	vpsrld	xmm0, xmm0, 7				; 1645 _ C5 F9: 72. D0, 07
	vpslld	xmm3, xmm5, 3				; 164A _ C5 E1: 72. F5, 03
	vpor	xmm2, xmm1, xmm0			; 164F _ C5 F1: EB. D0
	vpslld	xmm1, xmm7, 31				; 1653 _ C5 F1: 72. F7, 1F
	vpsrld	xmm7, xmm7, 1				; 1658 _ C5 C1: 72. D7, 01
	vpxor	xmm0, xmm2, xmm4			; 165D _ C5 E9: EF. C4
	vpor	xmm2, xmm1, xmm7			; 1661 _ C5 F1: EB. D7
	vpxor	xmm3, xmm0, xmm3			; 1665 _ C5 F9: EF. DB
	vpxor	xmm0, xmm2, xmm5			; 1669 _ C5 E9: EF. C5
	vpslld	xmm1, xmm4, 29				; 166D _ C5 F1: 72. F4, 1D
	vpxor	xmm0, xmm0, xmm4			; 1672 _ C5 F9: EF. C4
	vpsrld	xmm4, xmm4, 3				; 1676 _ C5 D9: 72. D4, 03
	vpor	xmm1, xmm1, xmm4			; 167B _ C5 F1: EB. CC
	vpslld	xmm6, xmm5, 19				; 167F _ C5 C9: 72. F5, 13
	vpsrld	xmm5, xmm5, 13				; 1684 _ C5 D1: 72. D5, 0D
	vpxor	xmm2, xmm1, xmm0			; 1689 _ C5 F1: EF. D0
	vpor	xmm7, xmm6, xmm5			; 168D _ C5 C9: EB. FD
	vpand	xmm4, xmm1, xmm2			; 1691 _ C5 F1: DB. E2
	vpxor	xmm6, xmm7, xmm2			; 1695 _ C5 C1: EF. F2
	vpxor	xmm1, xmm4, xmm6			; 1699 _ C5 D9: EF. CE
	vpand	xmm6, xmm6, xmm0			; 169D _ C5 C9: DB. F0
	vpor	xmm4, xmm3, xmm1			; 16A1 _ C5 E1: EB. E1
	vpxor	xmm3, xmm0, xmm3			; 16A5 _ C5 F9: EF. DB
	vpxor	xmm6, xmm6, xmm4			; 16A9 _ C5 C9: EF. F4
	vpxor	xmm0, xmm3, xmm1			; 16AD _ C5 E1: EF. C1
	vpxor	xmm2, xmm2, xmm4			; 16B1 _ C5 E9: EF. D4
	vpxor	xmm3, xmm0, xmm6			; 16B5 _ C5 F9: EF. DE
	vpor	xmm7, xmm3, xmm2			; 16B9 _ C5 E1: EB. FA
	vpand	xmm4, xmm4, xmm2			; 16BD _ C5 D9: DB. E2
	vpxor	xmm3, xmm7, xmm1			; 16C1 _ C5 C1: EF. D9
	vpxor	xmm5, xmm4, xmm0			; 16C5 _ C5 D9: EF. E8
	vmovd	xmm1, dword [ecx+2B30H] 		; 16C9 _ C5 F9: 6E. 89, 00002B30
	vpshufd xmm4, xmm1, 0				; 16D1 _ C5 F9: 70. E1, 00
	vmovd	xmm1, dword [ecx+2B38H] 		; 16D6 _ C5 F9: 6E. 89, 00002B38
	vpxor	xmm0, xmm2, xmm4			; 16DE _ C5 E9: EF. C4
	vmovd	xmm2, dword [ecx+2B34H] 		; 16E2 _ C5 F9: 6E. 91, 00002B34
	vpshufd xmm2, xmm2, 0				; 16EA _ C5 F9: 70. D2, 00
	vpshufd xmm7, xmm1, 0				; 16EF _ C5 F9: 70. F9, 00
	vpxor	xmm4, xmm3, xmm2			; 16F4 _ C5 E1: EF. E2
	vmovd	xmm2, dword [ecx+2B3CH] 		; 16F8 _ C5 F9: 6E. 91, 00002B3C
	vpxor	xmm7, xmm5, xmm7			; 1700 _ C5 D1: EF. FF
	vpxor	xmm5, xmm6, xmm5			; 1704 _ C5 C9: EF. ED
	vpslld	xmm6, xmm7, 10				; 1708 _ C5 C9: 72. F7, 0A
	vpshufd xmm1, xmm2, 0				; 170D _ C5 F9: 70. CA, 00
	vpxor	xmm3, xmm5, xmm3			; 1712 _ C5 D1: EF. DB
	vpsrld	xmm2, xmm7, 22				; 1716 _ C5 E9: 72. D7, 16
	vpslld	xmm7, xmm0, 27				; 171B _ C5 C1: 72. F0, 1B
	vpsrld	xmm0, xmm0, 5				; 1720 _ C5 F9: 72. D0, 05
	vpxor	xmm5, xmm3, xmm1			; 1725 _ C5 E1: EF. E9
	vpor	xmm3, xmm6, xmm2			; 1729 _ C5 C9: EB. DA
	vpor	xmm2, xmm7, xmm0			; 172D _ C5 C1: EB. D0
	vpxor	xmm1, xmm3, xmm5			; 1731 _ C5 E1: EF. CD
	vpslld	xmm6, xmm4, 7				; 1735 _ C5 C9: 72. F4, 07
	vpxor	xmm0, xmm2, xmm4			; 173A _ C5 E9: EF. C4
	vpxor	xmm6, xmm1, xmm6			; 173E _ C5 F1: EF. F6
	vpxor	xmm1, xmm0, xmm5			; 1742 _ C5 F9: EF. CD
	vpslld	xmm3, xmm5, 25				; 1746 _ C5 E1: 72. F5, 19
	vpsrld	xmm5, xmm5, 7				; 174B _ C5 D1: 72. D5, 07
	vpslld	xmm7, xmm6, 29				; 1750 _ C5 C1: 72. F6, 1D
	vpor	xmm2, xmm3, xmm5			; 1755 _ C5 E1: EB. D5
	vpslld	xmm5, xmm4, 31				; 1759 _ C5 D1: 72. F4, 1F
	vpsrld	xmm4, xmm4, 1				; 175E _ C5 D9: 72. D4, 01
	vpxor	xmm0, xmm2, xmm6			; 1763 _ C5 E9: EF. C6
	vpslld	xmm3, xmm1, 3				; 1767 _ C5 E1: 72. F1, 03
	vpor	xmm4, xmm5, xmm4			; 176C _ C5 D1: EB. E4
	vpxor	xmm2, xmm0, xmm3			; 1770 _ C5 F9: EF. D3
	vpxor	xmm0, xmm4, xmm1			; 1774 _ C5 D9: EF. C1
	vpxor	xmm3, xmm0, xmm6			; 1778 _ C5 F9: EF. DE
	vpslld	xmm5, xmm1, 19				; 177C _ C5 D1: 72. F1, 13
	vpsrld	xmm1, xmm1, 13				; 1781 _ C5 F1: 72. D1, 0D
	vpsrld	xmm6, xmm6, 3				; 1786 _ C5 C9: 72. D6, 03
	vpor	xmm1, xmm5, xmm1			; 178B _ C5 D1: EB. C9
	vpor	xmm4, xmm7, xmm6			; 178F _ C5 C1: EB. E6
	vpxor	xmm0, xmm4, xmm2			; 1793 _ C5 D9: EF. C2
	vpxor	xmm4, xmm2, xmm1			; 1797 _ C5 E9: EF. E1
	vpand	xmm2, xmm4, xmm0			; 179B _ C5 D9: DB. D0
	vpxor	xmm7, xmm2, xmm3			; 179F _ C5 E9: EF. FB
	vpor	xmm3, xmm3, xmm0			; 17A3 _ C5 E1: EB. D8
	vpxor	xmm5, xmm0, xmm7			; 17A7 _ C5 F9: EF. EF
	vpand	xmm0, xmm4, xmm7			; 17AB _ C5 D9: DB. C7
	vpxor	xmm2, xmm3, xmm4			; 17AF _ C5 E1: EF. D4
	vpand	xmm6, xmm0, xmm1			; 17B3 _ C5 F9: DB. F1
	vpcmpeqd xmm4, xmm4, xmm4			; 17B7 _ C5 D9: 76. E4
	vpxor	xmm0, xmm6, xmm5			; 17BB _ C5 C9: EF. C5
	vmovd	xmm6, dword [ecx+2B24H] 		; 17BF _ C5 F9: 6E. B1, 00002B24
	vpxor	xmm3, xmm7, xmm4			; 17C7 _ C5 C1: EF. DC
	vmovd	xmm7, dword [ecx+2B20H] 		; 17CB _ C5 F9: 6E. B9, 00002B20
	vpand	xmm5, xmm5, xmm2			; 17D3 _ C5 D1: DB. EA
	vpshufd xmm4, xmm7, 0				; 17D7 _ C5 F9: 70. E7, 00
	vpshufd xmm7, xmm6, 0				; 17DC _ C5 F9: 70. FE, 00
	vpor	xmm6, xmm5, xmm1			; 17E1 _ C5 D1: EB. F1
	vmovd	xmm5, dword [ecx+2B28H] 		; 17E5 _ C5 F9: 6E. A9, 00002B28
	vpxor	xmm6, xmm6, xmm3			; 17ED _ C5 C9: EF. F3
	vpshufd xmm5, xmm5, 0				; 17F1 _ C5 F9: 70. ED, 00
	vpxor	xmm1, xmm1, xmm3			; 17F6 _ C5 F1: EF. CB
	vpxor	xmm4, xmm2, xmm4			; 17FA _ C5 E9: EF. E4
	vpxor	xmm7, xmm0, xmm7			; 17FE _ C5 F9: EF. FF
	vpxor	xmm6, xmm6, xmm5			; 1802 _ C5 C9: EF. F5
	vpxor	xmm5, xmm3, xmm0			; 1806 _ C5 E1: EF. E8
	vmovd	xmm0, dword [ecx+2B2CH] 		; 180A _ C5 F9: 6E. 81, 00002B2C
	vpand	xmm2, xmm1, xmm2			; 1812 _ C5 F1: DB. D2
	vpshufd xmm1, xmm0, 0				; 1816 _ C5 F9: 70. C8, 00
	vpxor	xmm3, xmm5, xmm2			; 181B _ C5 D1: EF. DA
	vpslld	xmm5, xmm6, 10				; 181F _ C5 D1: 72. F6, 0A
	vpsrld	xmm6, xmm6, 22				; 1824 _ C5 C9: 72. D6, 16
	vpxor	xmm0, xmm3, xmm1			; 1829 _ C5 E1: EF. C1
	vpor	xmm2, xmm5, xmm6			; 182D _ C5 D1: EB. D6
	vpslld	xmm5, xmm4, 27				; 1831 _ C5 D1: 72. F4, 1B
	vpsrld	xmm4, xmm4, 5				; 1836 _ C5 D9: 72. D4, 05
	vpxor	xmm3, xmm2, xmm0			; 183B _ C5 E9: EF. D8
	vpor	xmm2, xmm5, xmm4			; 183F _ C5 D1: EB. D4
	vpslld	xmm1, xmm7, 7				; 1843 _ C5 F1: 72. F7, 07
	vpxor	xmm4, xmm2, xmm7			; 1848 _ C5 E9: EF. E7
	vpxor	xmm6, xmm3, xmm1			; 184C _ C5 E1: EF. F1
	vpxor	xmm1, xmm4, xmm0			; 1850 _ C5 D9: EF. C8
	vpslld	xmm3, xmm0, 25				; 1854 _ C5 E1: 72. F0, 19
	vpsrld	xmm0, xmm0, 7				; 1859 _ C5 F9: 72. D0, 07
	vpor	xmm2, xmm3, xmm0			; 185E _ C5 E1: EB. D0
	vpslld	xmm5, xmm7, 31				; 1862 _ C5 D1: 72. F7, 1F
	vpsrld	xmm7, xmm7, 1				; 1867 _ C5 C1: 72. D7, 01
	vpxor	xmm4, xmm2, xmm6			; 186C _ C5 E9: EF. E6
	vpslld	xmm0, xmm1, 3				; 1870 _ C5 F9: 72. F1, 03
	vpor	xmm2, xmm5, xmm7			; 1875 _ C5 D1: EB. D7
	vpxor	xmm0, xmm4, xmm0			; 1879 _ C5 D9: EF. C0
	vpxor	xmm4, xmm2, xmm1			; 187D _ C5 E9: EF. E1
	vpxor	xmm2, xmm4, xmm6			; 1881 _ C5 D9: EF. D6
	vpslld	xmm3, xmm6, 29				; 1885 _ C5 E1: 72. F6, 1D
	vpsrld	xmm6, xmm6, 3				; 188A _ C5 C9: 72. D6, 03
	vpslld	xmm5, xmm1, 19				; 188F _ C5 D1: 72. F1, 13
	vpsrld	xmm1, xmm1, 13				; 1894 _ C5 F1: 72. D1, 0D
	vpor	xmm3, xmm3, xmm6			; 1899 _ C5 E1: EB. DE
	vpor	xmm7, xmm5, xmm1			; 189D _ C5 D1: EB. F9
	vpxor	xmm1, xmm2, xmm0			; 18A1 _ C5 E9: EF. C8
	vpxor	xmm5, xmm2, xmm3			; 18A5 _ C5 E9: EF. EB
	vpand	xmm2, xmm0, xmm1			; 18A9 _ C5 F9: DB. D1
	vpxor	xmm4, xmm2, xmm7			; 18AD _ C5 E9: EF. E7
	vpor	xmm0, xmm7, xmm1			; 18B1 _ C5 C1: EB. C1
	vpxor	xmm6, xmm3, xmm4			; 18B5 _ C5 E1: EF. F4
	vpxor	xmm7, xmm0, xmm5			; 18B9 _ C5 F9: EF. FD
	vpxor	xmm3, xmm1, xmm4			; 18BD _ C5 F1: EF. DC
	vpor	xmm2, xmm7, xmm6			; 18C1 _ C5 C1: EB. D6
	vpxor	xmm7, xmm2, xmm3			; 18C5 _ C5 E9: EF. FB
	vpor	xmm3, xmm3, xmm4			; 18C9 _ C5 E1: EB. DC
	vpxor	xmm0, xmm3, xmm7			; 18CD _ C5 E1: EF. C7
	vpcmpeqd xmm3, xmm3, xmm3			; 18D1 _ C5 E1: 76. DB
	vpxor	xmm1, xmm5, xmm3			; 18D5 _ C5 D1: EF. CB
	vmovd	xmm5, dword [ecx+2B10H] 		; 18D9 _ C5 F9: 6E. A9, 00002B10
	vpxor	xmm1, xmm1, xmm0			; 18E1 _ C5 F1: EF. C8
	vpshufd xmm2, xmm5, 0				; 18E5 _ C5 F9: 70. D5, 00
	vpor	xmm0, xmm0, xmm7			; 18EA _ C5 F9: EB. C7
	vpxor	xmm5, xmm1, xmm2			; 18EE _ C5 F1: EF. EA
	vmovd	xmm2, dword [ecx+2B14H] 		; 18F2 _ C5 F9: 6E. 91, 00002B14
	vpshufd xmm2, xmm2, 0				; 18FA _ C5 F9: 70. D2, 00
	vpxor	xmm2, xmm7, xmm2			; 18FF _ C5 C1: EF. D2
	vpxor	xmm7, xmm0, xmm7			; 1903 _ C5 F9: EF. FF
	vpor	xmm0, xmm7, xmm1			; 1907 _ C5 C1: EB. C1
	vpxor	xmm0, xmm4, xmm0			; 190B _ C5 D9: EF. C0
	vmovd	xmm4, dword [ecx+2B18H] 		; 190F _ C5 F9: 6E. A1, 00002B18
	vmovd	xmm1, dword [ecx+2B1CH] 		; 1917 _ C5 F9: 6E. 89, 00002B1C
	vpshufd xmm4, xmm4, 0				; 191F _ C5 F9: 70. E4, 00
	vpshufd xmm7, xmm1, 0				; 1924 _ C5 F9: 70. F9, 00
	vpxor	xmm0, xmm0, xmm4			; 1929 _ C5 F9: EF. C4
	vpxor	xmm4, xmm6, xmm7			; 192D _ C5 C9: EF. E7
	vpslld	xmm6, xmm0, 10				; 1931 _ C5 C9: 72. F0, 0A
	vpsrld	xmm0, xmm0, 22				; 1936 _ C5 F9: 72. D0, 16
	vpslld	xmm7, xmm2, 7				; 193B _ C5 C1: 72. F2, 07
	vpor	xmm1, xmm6, xmm0			; 1940 _ C5 C9: EB. C8
	vpslld	xmm0, xmm5, 27				; 1944 _ C5 F9: 72. F5, 1B
	vpsrld	xmm5, xmm5, 5				; 1949 _ C5 D1: 72. D5, 05
	vpxor	xmm6, xmm1, xmm4			; 194E _ C5 F1: EF. F4
	vpor	xmm0, xmm0, xmm5			; 1952 _ C5 F9: EB. C5
	vpxor	xmm6, xmm6, xmm7			; 1956 _ C5 C9: EF. F7
	vpxor	xmm1, xmm0, xmm2			; 195A _ C5 F9: EF. CA
	vpslld	xmm7, xmm4, 25				; 195E _ C5 C1: 72. F4, 19
	vpxor	xmm5, xmm1, xmm4			; 1963 _ C5 F1: EF. EC
	vpsrld	xmm4, xmm4, 7				; 1967 _ C5 D9: 72. D4, 07
	vpor	xmm4, xmm7, xmm4			; 196C _ C5 C1: EB. E4
	vpslld	xmm7, xmm2, 31				; 1970 _ C5 C1: 72. F2, 1F
	vpsrld	xmm2, xmm2, 1				; 1975 _ C5 E9: 72. D2, 01
	vpxor	xmm0, xmm4, xmm6			; 197A _ C5 D9: EF. C6
	vpslld	xmm1, xmm5, 3				; 197E _ C5 F1: 72. F5, 03
	vpor	xmm2, xmm7, xmm2			; 1983 _ C5 C1: EB. D2
	vpxor	xmm4, xmm0, xmm1			; 1987 _ C5 F9: EF. E1
	vpxor	xmm0, xmm2, xmm5			; 198B _ C5 E9: EF. C5
	vpslld	xmm1, xmm5, 19				; 198F _ C5 F1: 72. F5, 13
	vpsrld	xmm5, xmm5, 13				; 1994 _ C5 D1: 72. D5, 0D
	vpxor	xmm0, xmm0, xmm6			; 1999 _ C5 F9: EF. C6
	vpor	xmm2, xmm1, xmm5			; 199D _ C5 F1: EB. D5
	vpslld	xmm1, xmm6, 29				; 19A1 _ C5 F1: 72. F6, 1D
	vpsrld	xmm6, xmm6, 3				; 19A6 _ C5 C9: 72. D6, 03
	vpor	xmm1, xmm1, xmm6			; 19AB _ C5 F1: EB. CE
	vpxor	xmm6, xmm0, xmm3			; 19AF _ C5 F9: EF. F3
	vpxor	xmm1, xmm1, xmm3			; 19B3 _ C5 F1: EF. CB
	vpor	xmm0, xmm0, xmm2			; 19B7 _ C5 F9: EB. C2
	vpxor	xmm5, xmm0, xmm1			; 19BB _ C5 F9: EF. E9
	vpor	xmm7, xmm1, xmm6			; 19BF _ C5 F1: EB. FE
	vpxor	xmm0, xmm5, xmm4			; 19C3 _ C5 D1: EF. C4
	vpxor	xmm5, xmm2, xmm6			; 19C7 _ C5 E9: EF. EE
	vpand	xmm1, xmm5, xmm4			; 19CB _ C5 D1: DB. CC
	vpxor	xmm2, xmm7, xmm5			; 19CF _ C5 C1: EF. D5
	vpxor	xmm7, xmm6, xmm1			; 19D3 _ C5 C9: EF. F9
	vpor	xmm6, xmm1, xmm0			; 19D7 _ C5 F1: EB. F0
	vpxor	xmm5, xmm6, xmm2			; 19DB _ C5 C9: EF. EA
	vpxor	xmm4, xmm4, xmm7			; 19DF _ C5 D9: EF. E7
	vpxor	xmm4, xmm4, xmm5			; 19E3 _ C5 D9: EF. E5
	vpxor	xmm2, xmm2, xmm0			; 19E7 _ C5 E9: EF. D0
	vpxor	xmm1, xmm4, xmm0			; 19EB _ C5 D9: EF. C8
	vmovd	xmm6, dword [ecx+2B00H] 		; 19EF _ C5 F9: 6E. B1, 00002B00
	vpand	xmm2, xmm2, xmm1			; 19F7 _ C5 E9: DB. D1
	vpshufd xmm4, xmm6, 0				; 19FB _ C5 F9: 70. E6, 00
	vpxor	xmm2, xmm7, xmm2			; 1A00 _ C5 C1: EF. D2
	vmovd	xmm7, dword [ecx+2B04H] 		; 1A04 _ C5 F9: 6E. B9, 00002B04
	vpxor	xmm4, xmm5, xmm4			; 1A0C _ C5 D1: EF. E4
	vmovd	xmm5, dword [ecx+2B08H] 		; 1A10 _ C5 F9: 6E. A9, 00002B08
	vpshufd xmm6, xmm7, 0				; 1A18 _ C5 F9: 70. F7, 00
	vpxor	xmm2, xmm2, xmm6			; 1A1D _ C5 E9: EF. D6
	vpshufd xmm6, xmm5, 0				; 1A21 _ C5 F9: 70. F5, 00
	vmovd	xmm5, dword [ecx+2B0CH] 		; 1A26 _ C5 F9: 6E. A9, 00002B0C
	vpxor	xmm0, xmm0, xmm6			; 1A2E _ C5 F9: EF. C6
	vpshufd xmm7, xmm5, 0				; 1A32 _ C5 F9: 70. FD, 00
	vpslld	xmm5, xmm2, 7				; 1A37 _ C5 D1: 72. F2, 07
	vpxor	xmm6, xmm1, xmm7			; 1A3C _ C5 F1: EF. F7
	vpslld	xmm1, xmm0, 10				; 1A40 _ C5 F1: 72. F0, 0A
	vpsrld	xmm0, xmm0, 22				; 1A45 _ C5 F9: 72. D0, 16
	vpslld	xmm7, xmm4, 27				; 1A4A _ C5 C1: 72. F4, 1B
	vpsrld	xmm4, xmm4, 5				; 1A4F _ C5 D9: 72. D4, 05
	vpor	xmm0, xmm1, xmm0			; 1A54 _ C5 F1: EB. C0
	vpor	xmm4, xmm7, xmm4			; 1A58 _ C5 C1: EB. E4
	vpxor	xmm1, xmm0, xmm6			; 1A5C _ C5 F9: EF. CE
	vpxor	xmm0, xmm4, xmm2			; 1A60 _ C5 D9: EF. C2
	vpxor	xmm1, xmm1, xmm5			; 1A64 _ C5 F1: EF. CD
	vpxor	xmm0, xmm0, xmm6			; 1A68 _ C5 F9: EF. C6
	vpslld	xmm5, xmm6, 25				; 1A6C _ C5 D1: 72. F6, 19
	vpsrld	xmm6, xmm6, 7				; 1A71 _ C5 C9: 72. D6, 07
	vpor	xmm7, xmm5, xmm6			; 1A76 _ C5 D1: EB. FE
	vpslld	xmm5, xmm2, 31				; 1A7A _ C5 D1: 72. F2, 1F
	vpsrld	xmm2, xmm2, 1				; 1A7F _ C5 E9: 72. D2, 01
	vpxor	xmm4, xmm7, xmm1			; 1A84 _ C5 C1: EF. E1
	vpslld	xmm6, xmm0, 3				; 1A88 _ C5 C9: 72. F0, 03
	vpor	xmm2, xmm5, xmm2			; 1A8D _ C5 D1: EB. D2
	vpxor	xmm6, xmm4, xmm6			; 1A91 _ C5 D9: EF. F6
	vpxor	xmm4, xmm2, xmm0			; 1A95 _ C5 E9: EF. E0
	vpxor	xmm5, xmm4, xmm1			; 1A99 _ C5 D9: EF. E9
	vpslld	xmm7, xmm1, 29				; 1A9D _ C5 C1: 72. F1, 1D
	vpsrld	xmm1, xmm1, 3				; 1AA2 _ C5 F1: 72. D1, 03
	vpslld	xmm2, xmm0, 19				; 1AA7 _ C5 E9: 72. F0, 13
	vpsrld	xmm0, xmm0, 13				; 1AAC _ C5 F9: 72. D0, 0D
	vpor	xmm1, xmm7, xmm1			; 1AB1 _ C5 C1: EB. C9
	vpor	xmm4, xmm2, xmm0			; 1AB5 _ C5 E9: EB. E0
	vpor	xmm0, xmm1, xmm6			; 1AB9 _ C5 F1: EB. C6
	vpxor	xmm1, xmm1, xmm4			; 1ABD _ C5 F1: EF. CC
	vpand	xmm7, xmm4, xmm6			; 1AC1 _ C5 D9: DB. FE
	vpxor	xmm3, xmm1, xmm3			; 1AC5 _ C5 F1: EF. DB
	vpxor	xmm4, xmm6, xmm5			; 1AC9 _ C5 C9: EF. E5
	vpxor	xmm1, xmm7, xmm3			; 1ACD _ C5 C1: EF. CB
	vpand	xmm2, xmm3, xmm0			; 1AD1 _ C5 E1: DB. D0
	vpor	xmm6, xmm5, xmm7			; 1AD5 _ C5 D1: EB. F7
	vpand	xmm3, xmm4, xmm0			; 1AD9 _ C5 D9: DB. D8
	vpxor	xmm5, xmm6, xmm2			; 1ADD _ C5 C9: EF. EA
	vpxor	xmm6, xmm2, xmm1			; 1AE1 _ C5 E9: EF. F1
	vpxor	xmm7, xmm0, xmm5			; 1AE5 _ C5 F9: EF. FD
	vpor	xmm2, xmm1, xmm6			; 1AE9 _ C5 F1: EB. D6
	vpxor	xmm4, xmm2, xmm3			; 1AED _ C5 E9: EF. E3
	vpxor	xmm0, xmm3, xmm7			; 1AF1 _ C5 E1: EF. C7
	vmovd	xmm3, dword [ecx+2AF0H] 		; 1AF5 _ C5 F9: 6E. 99, 00002AF0
	vpxor	xmm1, xmm0, xmm6			; 1AFD _ C5 F9: EF. CE
	vpshufd xmm2, xmm3, 0				; 1B01 _ C5 F9: 70. D3, 00
	vmovd	xmm3, dword [ecx+2AF4H] 		; 1B06 _ C5 F9: 6E. 99, 00002AF4
	vpxor	xmm0, xmm1, xmm2			; 1B0E _ C5 F1: EF. C2
	vmovd	xmm2, dword [ecx+2AF8H] 		; 1B12 _ C5 F9: 6E. 91, 00002AF8
	vpshufd xmm1, xmm3, 0				; 1B1A _ C5 F9: 70. CB, 00
	vpxor	xmm3, xmm4, xmm1			; 1B1F _ C5 D9: EF. D9
	vpshufd xmm1, xmm2, 0				; 1B23 _ C5 F9: 70. CA, 00
	vpxor	xmm2, xmm5, xmm1			; 1B28 _ C5 D1: EF. D1
	vpor	xmm5, xmm7, xmm4			; 1B2C _ C5 C1: EB. EC
	vmovd	xmm7, dword [ecx+2AFCH] 		; 1B30 _ C5 F9: 6E. B9, 00002AFC
	vpxor	xmm4, xmm5, xmm6			; 1B38 _ C5 D1: EF. E6
	vpshufd xmm1, xmm7, 0				; 1B3C _ C5 F9: 70. CF, 00
	vpslld	xmm5, xmm2, 10				; 1B41 _ C5 D1: 72. F2, 0A
	vpsrld	xmm2, xmm2, 22				; 1B46 _ C5 E9: 72. D2, 16
	vpxor	xmm1, xmm4, xmm1			; 1B4B _ C5 D9: EF. C9
	vpor	xmm6, xmm5, xmm2			; 1B4F _ C5 D1: EB. F2
	vpslld	xmm4, xmm3, 7				; 1B53 _ C5 D9: 72. F3, 07
	vpxor	xmm2, xmm6, xmm1			; 1B58 _ C5 C9: EF. D1
	vpslld	xmm7, xmm0, 27				; 1B5C _ C5 C1: 72. F0, 1B
	vpsrld	xmm0, xmm0, 5				; 1B61 _ C5 F9: 72. D0, 05
	vpxor	xmm2, xmm2, xmm4			; 1B66 _ C5 E9: EF. D4
	vpor	xmm4, xmm7, xmm0			; 1B6A _ C5 C1: EB. E0
	vpslld	xmm5, xmm1, 25				; 1B6E _ C5 D1: 72. F1, 19
	vpxor	xmm0, xmm4, xmm3			; 1B73 _ C5 D9: EF. C3
	vpxor	xmm4, xmm0, xmm1			; 1B77 _ C5 F9: EF. E1
	vpsrld	xmm1, xmm1, 7				; 1B7B _ C5 F1: 72. D1, 07
	vpor	xmm6, xmm5, xmm1			; 1B80 _ C5 D1: EB. F1
	vpslld	xmm0, xmm4, 3				; 1B84 _ C5 F9: 72. F4, 03
	vpxor	xmm7, xmm6, xmm2			; 1B89 _ C5 C9: EF. FA
	vpslld	xmm1, xmm3, 31				; 1B8D _ C5 F1: 72. F3, 1F
	vpsrld	xmm3, xmm3, 1				; 1B92 _ C5 E1: 72. D3, 01
	vpxor	xmm0, xmm7, xmm0			; 1B97 _ C5 C1: EF. C0
	vpor	xmm1, xmm1, xmm3			; 1B9B _ C5 F1: EB. CB
	vpxor	xmm5, xmm1, xmm4			; 1B9F _ C5 F1: EF. EC
	vpslld	xmm1, xmm2, 29				; 1BA3 _ C5 F1: 72. F2, 1D
	vpxor	xmm7, xmm5, xmm2			; 1BA8 _ C5 D1: EF. FA
	vpsrld	xmm2, xmm2, 3				; 1BAC _ C5 E9: 72. D2, 03
	vpslld	xmm3, xmm4, 19				; 1BB1 _ C5 E1: 72. F4, 13
	vpsrld	xmm4, xmm4, 13				; 1BB6 _ C5 D9: 72. D4, 0D
	vpor	xmm6, xmm1, xmm2			; 1BBB _ C5 F1: EB. F2
	vpor	xmm5, xmm3, xmm4			; 1BBF _ C5 E1: EB. EC
	vpxor	xmm3, xmm5, xmm6			; 1BC3 _ C5 D1: EF. DE
	vpxor	xmm4, xmm0, xmm7			; 1BC7 _ C5 F9: EF. E7
	vpand	xmm1, xmm6, xmm3			; 1BCB _ C5 C9: DB. CB
	vpcmpeqd xmm2, xmm2, xmm2			; 1BCF _ C5 E9: 76. D2
	vpxor	xmm5, xmm1, xmm2			; 1BD3 _ C5 F1: EF. EA
	vpxor	xmm6, xmm6, xmm0			; 1BD7 _ C5 C9: EF. F0
	vpxor	xmm5, xmm5, xmm4			; 1BDB _ C5 D1: EF. EC
	vpor	xmm0, xmm6, xmm3			; 1BDF _ C5 C9: EB. C3
	vpxor	xmm1, xmm3, xmm5			; 1BE3 _ C5 E1: EF. CD
	vpxor	xmm2, xmm4, xmm0			; 1BE7 _ C5 D9: EF. D0
	vpand	xmm6, xmm7, xmm2			; 1BEB _ C5 C1: DB. F2
	vpxor	xmm0, xmm0, xmm7			; 1BEF _ C5 F9: EF. C7
	vpxor	xmm7, xmm1, xmm2			; 1BF3 _ C5 F1: EF. FA
	vpxor	xmm4, xmm6, xmm1			; 1BF7 _ C5 C9: EF. E1
	vmovd	xmm3, dword [ecx+2AE0H] 		; 1BFB _ C5 F9: 6E. 99, 00002AE0
	vpor	xmm1, xmm7, xmm5			; 1C03 _ C5 C1: EB. CD
	vpshufd xmm6, xmm3, 0				; 1C07 _ C5 F9: 70. F3, 00
	vpxor	xmm1, xmm0, xmm1			; 1C0C _ C5 F9: EF. C9
	vmovd	xmm0, dword [ecx+2AE8H] 		; 1C10 _ C5 F9: 6E. 81, 00002AE8
	vpxor	xmm3, xmm4, xmm6			; 1C18 _ C5 D9: EF. DE
	vmovd	xmm6, dword [ecx+2AE4H] 		; 1C1C _ C5 F9: 6E. B1, 00002AE4
	vpxor	xmm4, xmm2, xmm4			; 1C24 _ C5 E9: EF. E4
	vpshufd xmm0, xmm0, 0				; 1C28 _ C5 F9: 70. C0, 00
	vmovd	xmm2, dword [ecx+2AECH] 		; 1C2D _ C5 F9: 6E. 91, 00002AEC
	vpxor	xmm7, xmm1, xmm0			; 1C35 _ C5 F1: EF. F8
	vpshufd xmm6, xmm6, 0				; 1C39 _ C5 F9: 70. F6, 00
	vpslld	xmm1, xmm7, 10				; 1C3E _ C5 F1: 72. F7, 0A
	vpxor	xmm6, xmm5, xmm6			; 1C43 _ C5 D1: EF. F6
	vpsrld	xmm7, xmm7, 22				; 1C47 _ C5 C1: 72. D7, 16
	vpshufd xmm5, xmm2, 0				; 1C4C _ C5 F9: 70. EA, 00
	vpor	xmm1, xmm1, xmm7			; 1C51 _ C5 F1: EB. CF
	vpxor	xmm0, xmm4, xmm5			; 1C55 _ C5 D9: EF. C5
	vpslld	xmm4, xmm3, 27				; 1C59 _ C5 D9: 72. F3, 1B
	vpsrld	xmm3, xmm3, 5				; 1C5E _ C5 E1: 72. D3, 05
	vpxor	xmm2, xmm1, xmm0			; 1C63 _ C5 F1: EF. D0
	vpslld	xmm7, xmm6, 7				; 1C67 _ C5 C1: 72. F6, 07
	vpor	xmm1, xmm4, xmm3			; 1C6C _ C5 D9: EB. CB
	vpxor	xmm5, xmm2, xmm7			; 1C70 _ C5 E9: EF. EF
	vpxor	xmm2, xmm1, xmm6			; 1C74 _ C5 F1: EF. D6
	vpxor	xmm2, xmm2, xmm0			; 1C78 _ C5 E9: EF. D0
	vpslld	xmm3, xmm0, 25				; 1C7C _ C5 E1: 72. F0, 19
	vpsrld	xmm0, xmm0, 7				; 1C81 _ C5 F9: 72. D0, 07
	vpslld	xmm1, xmm2, 3				; 1C86 _ C5 F1: 72. F2, 03
	vpor	xmm7, xmm3, xmm0			; 1C8B _ C5 E1: EB. F8
	vpslld	xmm0, xmm2, 19				; 1C8F _ C5 F9: 72. F2, 13
	vpxor	xmm4, xmm7, xmm5			; 1C94 _ C5 C1: EF. E5
	vpslld	xmm7, xmm6, 31				; 1C98 _ C5 C1: 72. F6, 1F
	vpsrld	xmm6, xmm6, 1				; 1C9D _ C5 C9: 72. D6, 01
	vpsrld	xmm3, xmm2, 13				; 1CA2 _ C5 E1: 72. D2, 0D
	vpor	xmm6, xmm7, xmm6			; 1CA7 _ C5 C1: EB. F6
	vpxor	xmm1, xmm4, xmm1			; 1CAB _ C5 D9: EF. C9
	vpxor	xmm2, xmm6, xmm2			; 1CAF _ C5 C9: EF. D2
	vpor	xmm4, xmm0, xmm3			; 1CB3 _ C5 F9: EB. E3
	vpxor	xmm0, xmm2, xmm5			; 1CB7 _ C5 E9: EF. C5
	vpslld	xmm7, xmm5, 29				; 1CBB _ C5 C1: 72. F5, 1D
	vpcmpeqd xmm2, xmm2, xmm2			; 1CC0 _ C5 E9: 76. D2
	vpsrld	xmm5, xmm5, 3				; 1CC4 _ C5 D1: 72. D5, 03
	vpxor	xmm3, xmm0, xmm2			; 1CC9 _ C5 F9: EF. DA
	vpor	xmm6, xmm7, xmm5			; 1CCD _ C5 C1: EB. F5
	vpxor	xmm7, xmm6, xmm3			; 1CD1 _ C5 C9: EF. FB
	vpor	xmm0, xmm1, xmm4			; 1CD5 _ C5 F1: EB. C4
	vpxor	xmm0, xmm0, xmm7			; 1CD9 _ C5 F9: EF. C7
	vpxor	xmm6, xmm1, xmm0			; 1CDD _ C5 F1: EF. F0
	vpor	xmm1, xmm7, xmm3			; 1CE1 _ C5 C1: EB. CB
	vpand	xmm5, xmm1, xmm4			; 1CE5 _ C5 F1: DB. EC
	vpxor	xmm1, xmm5, xmm6			; 1CE9 _ C5 D1: EF. CE
	vpor	xmm6, xmm6, xmm4			; 1CED _ C5 C9: EB. F4
	vpand	xmm7, xmm3, xmm1			; 1CF1 _ C5 E1: DB. F9
	vpxor	xmm3, xmm6, xmm3			; 1CF5 _ C5 C9: EF. DB
	vpxor	xmm5, xmm7, xmm0			; 1CF9 _ C5 C1: EF. E8
	vpxor	xmm6, xmm3, xmm1			; 1CFD _ C5 E1: EF. F1
	vmovd	xmm3, dword [ecx+2AD0H] 		; 1D01 _ C5 F9: 6E. 99, 00002AD0
	vpxor	xmm7, xmm6, xmm5			; 1D09 _ C5 C9: EF. FD
	vpshufd xmm3, xmm3, 0				; 1D0D _ C5 F9: 70. DB, 00
	vpand	xmm0, xmm0, xmm6			; 1D12 _ C5 F9: DB. C6
	vpxor	xmm3, xmm5, xmm3			; 1D16 _ C5 D1: EF. DB
	vpxor	xmm5, xmm7, xmm2			; 1D1A _ C5 C1: EF. EA
	vmovd	xmm2, dword [ecx+2AD4H] 		; 1D1E _ C5 F9: 6E. 91, 00002AD4
	vpshufd xmm2, xmm2, 0				; 1D26 _ C5 F9: 70. D2, 00
	vpxor	xmm2, xmm5, xmm2			; 1D2B _ C5 D1: EF. D2
	vpxor	xmm5, xmm0, xmm7			; 1D2F _ C5 F9: EF. EF
	vpxor	xmm6, xmm5, xmm4			; 1D33 _ C5 D1: EF. F4
	vmovd	xmm4, dword [ecx+2AD8H] 		; 1D37 _ C5 F9: 6E. A1, 00002AD8
	vmovd	xmm7, dword [ecx+2ADCH] 		; 1D3F _ C5 F9: 6E. B9, 00002ADC
	vpshufd xmm0, xmm4, 0				; 1D47 _ C5 F9: 70. C4, 00
	vpshufd xmm4, xmm7, 0				; 1D4C _ C5 F9: 70. E7, 00
	vpxor	xmm5, xmm6, xmm0			; 1D51 _ C5 C9: EF. E8
	vpxor	xmm7, xmm1, xmm4			; 1D55 _ C5 F1: EF. FC
	vpslld	xmm1, xmm5, 10				; 1D59 _ C5 F1: 72. F5, 0A
	vpsrld	xmm6, xmm5, 22				; 1D5E _ C5 C9: 72. D5, 16
	vpslld	xmm5, xmm3, 27				; 1D63 _ C5 D1: 72. F3, 1B
	vpor	xmm1, xmm1, xmm6			; 1D68 _ C5 F1: EB. CE
	vpsrld	xmm3, xmm3, 5				; 1D6C _ C5 E1: 72. D3, 05
	vpxor	xmm0, xmm1, xmm7			; 1D71 _ C5 F1: EF. C7
	vpor	xmm1, xmm5, xmm3			; 1D75 _ C5 D1: EB. CB
	vpslld	xmm4, xmm2, 7				; 1D79 _ C5 D9: 72. F2, 07
	vpxor	xmm6, xmm1, xmm2			; 1D7E _ C5 F1: EF. F2
	vpxor	xmm4, xmm0, xmm4			; 1D82 _ C5 F9: EF. E4
	vpxor	xmm1, xmm6, xmm7			; 1D86 _ C5 C9: EF. CF
	vpslld	xmm0, xmm7, 25				; 1D8A _ C5 F9: 72. F7, 19
	vpsrld	xmm7, xmm7, 7				; 1D8F _ C5 C1: 72. D7, 07
	vpslld	xmm5, xmm2, 31				; 1D94 _ C5 D1: 72. F2, 1F
	vpsrld	xmm2, xmm2, 1				; 1D99 _ C5 E9: 72. D2, 01
	vpor	xmm6, xmm0, xmm7			; 1D9E _ C5 F9: EB. F7
	vpor	xmm2, xmm5, xmm2			; 1DA2 _ C5 D1: EB. D2
	vpxor	xmm0, xmm6, xmm4			; 1DA6 _ C5 C9: EF. C4
	vpslld	xmm3, xmm1, 3				; 1DAA _ C5 E1: 72. F1, 03
	vpxor	xmm6, xmm2, xmm1			; 1DAF _ C5 E9: EF. F1
	vpxor	xmm7, xmm0, xmm3			; 1DB3 _ C5 F9: EF. FB
	vpxor	xmm3, xmm6, xmm4			; 1DB7 _ C5 C9: EF. DC
	vpslld	xmm0, xmm4, 29				; 1DBB _ C5 F9: 72. F4, 1D
	vpsrld	xmm4, xmm4, 3				; 1DC0 _ C5 D9: 72. D4, 03
	vpslld	xmm6, xmm1, 19				; 1DC5 _ C5 C9: 72. F1, 13
	vpor	xmm2, xmm0, xmm4			; 1DCA _ C5 F9: EB. D4
	vpsrld	xmm1, xmm1, 13				; 1DCE _ C5 F1: 72. D1, 0D
	vpand	xmm4, xmm2, xmm7			; 1DD3 _ C5 E9: DB. E7
	vpor	xmm0, xmm6, xmm1			; 1DD7 _ C5 C9: EB. C1
	vpxor	xmm1, xmm4, xmm3			; 1DDB _ C5 D9: EF. CB
	vpor	xmm3, xmm3, xmm7			; 1DDF _ C5 E1: EB. DF
	vpand	xmm6, xmm3, xmm0			; 1DE3 _ C5 E1: DB. F0
	vpxor	xmm5, xmm2, xmm1			; 1DE7 _ C5 E9: EF. E9
	vpxor	xmm2, xmm5, xmm6			; 1DEB _ C5 D1: EF. D6
	vpcmpeqd xmm3, xmm3, xmm3			; 1DEF _ C5 E1: 76. DB
	vpxor	xmm4, xmm0, xmm3			; 1DF3 _ C5 F9: EF. E3
	vpxor	xmm3, xmm7, xmm2			; 1DF7 _ C5 C1: EF. DA
	vpand	xmm7, xmm6, xmm1			; 1DFB _ C5 C9: DB. F9
	vpxor	xmm6, xmm7, xmm3			; 1DFF _ C5 C1: EF. F3
	vpand	xmm7, xmm3, xmm4			; 1E03 _ C5 E1: DB. FC
	vmovd	xmm3, dword [ecx+2AC0H] 		; 1E07 _ C5 F9: 6E. 99, 00002AC0
	vpxor	xmm0, xmm4, xmm6			; 1E0F _ C5 D9: EF. C6
	vpxor	xmm4, xmm7, xmm1			; 1E13 _ C5 C1: EF. E1
	vpand	xmm1, xmm1, xmm0			; 1E17 _ C5 F1: DB. C8
	vpshufd xmm7, xmm3, 0				; 1E1B _ C5 F9: 70. FB, 00
	vpxor	xmm5, xmm4, xmm0			; 1E20 _ C5 D9: EF. E8
	vpxor	xmm4, xmm0, xmm7			; 1E24 _ C5 F9: EF. E7
	vpxor	xmm3, xmm5, xmm0			; 1E28 _ C5 D1: EF. D8
	vpxor	xmm0, xmm1, xmm2			; 1E2C _ C5 F1: EF. C2
	vpor	xmm5, xmm0, xmm5			; 1E30 _ C5 F9: EB. ED
	vmovd	xmm7, dword [ecx+2AC4H] 		; 1E34 _ C5 F9: 6E. B9, 00002AC4
	vpxor	xmm1, xmm5, xmm6			; 1E3C _ C5 D1: EF. CE
	vmovd	xmm6, dword [ecx+2AC8H] 		; 1E40 _ C5 F9: 6E. B1, 00002AC8
	vmovd	xmm0, dword [ecx+2ACCH] 		; 1E48 _ C5 F9: 6E. 81, 00002ACC
	vpshufd xmm7, xmm7, 0				; 1E50 _ C5 F9: 70. FF, 00
	vpshufd xmm6, xmm6, 0				; 1E55 _ C5 F9: 70. F6, 00
	vpxor	xmm3, xmm3, xmm7			; 1E5A _ C5 E1: EF. DF
	vpshufd xmm7, xmm0, 0				; 1E5E _ C5 F9: 70. F8, 00
	vpxor	xmm5, xmm1, xmm6			; 1E63 _ C5 F1: EF. EE
	vpxor	xmm7, xmm2, xmm7			; 1E67 _ C5 E9: EF. FF
	vpslld	xmm2, xmm5, 10				; 1E6B _ C5 E9: 72. F5, 0A
	vpsrld	xmm1, xmm5, 22				; 1E70 _ C5 F1: 72. D5, 16
	vpslld	xmm5, xmm4, 27				; 1E75 _ C5 D1: 72. F4, 1B
	vpor	xmm2, xmm2, xmm1			; 1E7A _ C5 E9: EB. D1
	vpsrld	xmm4, xmm4, 5				; 1E7E _ C5 D9: 72. D4, 05
	vpxor	xmm6, xmm2, xmm7			; 1E83 _ C5 E9: EF. F7
	vpslld	xmm0, xmm3, 7				; 1E87 _ C5 F9: 72. F3, 07
	vpor	xmm1, xmm5, xmm4			; 1E8C _ C5 D1: EB. CC
	vpxor	xmm2, xmm6, xmm0			; 1E90 _ C5 C9: EF. D0
	vpxor	xmm6, xmm1, xmm3			; 1E94 _ C5 F1: EF. F3
	vpslld	xmm0, xmm7, 25				; 1E98 _ C5 F9: 72. F7, 19
	vpxor	xmm1, xmm6, xmm7			; 1E9D _ C5 C9: EF. CF
	vpsrld	xmm7, xmm7, 7				; 1EA1 _ C5 C1: 72. D7, 07
	vpor	xmm4, xmm0, xmm7			; 1EA6 _ C5 F9: EB. E7
	vpslld	xmm6, xmm1, 3				; 1EAA _ C5 C9: 72. F1, 03
	vpxor	xmm5, xmm4, xmm2			; 1EAF _ C5 D9: EF. EA
	vpslld	xmm0, xmm3, 31				; 1EB3 _ C5 F9: 72. F3, 1F
	vpsrld	xmm3, xmm3, 1				; 1EB8 _ C5 E1: 72. D3, 01
	vpxor	xmm7, xmm5, xmm6			; 1EBD _ C5 D1: EF. FE
	vpor	xmm6, xmm0, xmm3			; 1EC1 _ C5 F9: EB. F3
	vpslld	xmm3, xmm2, 29				; 1EC5 _ C5 E1: 72. F2, 1D
	vpxor	xmm0, xmm6, xmm1			; 1ECA _ C5 C9: EF. C1
	vpslld	xmm4, xmm1, 19				; 1ECE _ C5 D9: 72. F1, 13
	vpxor	xmm5, xmm0, xmm2			; 1ED3 _ C5 F9: EF. EA
	vpsrld	xmm2, xmm2, 3				; 1ED7 _ C5 E9: 72. D2, 03
	vpor	xmm6, xmm3, xmm2			; 1EDC _ C5 E1: EB. F2
	vpsrld	xmm1, xmm1, 13				; 1EE0 _ C5 F1: 72. D1, 0D
	vpxor	xmm0, xmm6, xmm5			; 1EE5 _ C5 C9: EF. C5
	vpor	xmm1, xmm4, xmm1			; 1EE9 _ C5 D9: EB. C9
	vpxor	xmm1, xmm1, xmm0			; 1EED _ C5 F1: EF. C8
	vpand	xmm2, xmm6, xmm0			; 1EF1 _ C5 C9: DB. D0
	vpxor	xmm6, xmm2, xmm1			; 1EF5 _ C5 E9: EF. F1
	vpand	xmm3, xmm1, xmm5			; 1EF9 _ C5 F1: DB. DD
	vpor	xmm2, xmm7, xmm6			; 1EFD _ C5 C1: EB. D6
	vpxor	xmm7, xmm5, xmm7			; 1F01 _ C5 D1: EF. FF
	vpxor	xmm0, xmm0, xmm2			; 1F05 _ C5 F9: EF. C2
	vpxor	xmm1, xmm3, xmm2			; 1F09 _ C5 E1: EF. CA
	vpxor	xmm3, xmm7, xmm6			; 1F0D _ C5 C1: EF. DE
	vpand	xmm5, xmm2, xmm0			; 1F11 _ C5 E9: DB. E8
	vpxor	xmm2, xmm3, xmm1			; 1F15 _ C5 E1: EF. D1
	vpxor	xmm5, xmm5, xmm3			; 1F19 _ C5 D1: EF. EB
	vpor	xmm7, xmm2, xmm0			; 1F1D _ C5 E9: EB. F8
	vpxor	xmm1, xmm1, xmm5			; 1F21 _ C5 F1: EF. CD
	vpxor	xmm4, xmm7, xmm6			; 1F25 _ C5 C1: EF. E6
	vmovd	xmm6, dword [ecx+2AB0H] 		; 1F29 _ C5 F9: 6E. B1, 00002AB0
	vpshufd xmm2, xmm6, 0				; 1F31 _ C5 F9: 70. D6, 00
	vmovd	xmm6, dword [ecx+2AB8H] 		; 1F36 _ C5 F9: 6E. B1, 00002AB8
	vpxor	xmm7, xmm0, xmm2			; 1F3E _ C5 F9: EF. FA
	vmovd	xmm0, dword [ecx+2AB4H] 		; 1F42 _ C5 F9: 6E. 81, 00002AB4
	vpshufd xmm2, xmm0, 0				; 1F4A _ C5 F9: 70. D0, 00
	vpshufd xmm0, xmm6, 0				; 1F4F _ C5 F9: 70. C6, 00
	vpxor	xmm2, xmm4, xmm2			; 1F54 _ C5 D9: EF. D2
	vpxor	xmm0, xmm5, xmm0			; 1F58 _ C5 D1: EF. C0
	vpxor	xmm5, xmm1, xmm4			; 1F5C _ C5 F1: EF. EC
	vpslld	xmm6, xmm0, 10				; 1F60 _ C5 C9: 72. F0, 0A
	vpsrld	xmm0, xmm0, 22				; 1F65 _ C5 F9: 72. D0, 16
	vmovd	xmm4, dword [ecx+2ABCH] 		; 1F6A _ C5 F9: 6E. A1, 00002ABC
	vpor	xmm3, xmm6, xmm0			; 1F72 _ C5 C9: EB. D8
	vpslld	xmm6, xmm7, 27				; 1F76 _ C5 C9: 72. F7, 1B
	vpsrld	xmm7, xmm7, 5				; 1F7B _ C5 C1: 72. D7, 05
	vpshufd xmm1, xmm4, 0				; 1F80 _ C5 F9: 70. CC, 00
	vpor	xmm6, xmm6, xmm7			; 1F85 _ C5 C9: EB. F7
	vpxor	xmm1, xmm5, xmm1			; 1F89 _ C5 D1: EF. C9
	vpxor	xmm0, xmm6, xmm2			; 1F8D _ C5 C9: EF. C2
	vpxor	xmm4, xmm3, xmm1			; 1F91 _ C5 E1: EF. E1
	vpslld	xmm5, xmm2, 7				; 1F95 _ C5 D1: 72. F2, 07
	vpxor	xmm6, xmm0, xmm1			; 1F9A _ C5 F9: EF. F1
	vpslld	xmm7, xmm1, 25				; 1F9E _ C5 C1: 72. F1, 19
	vpsrld	xmm1, xmm1, 7				; 1FA3 _ C5 F1: 72. D1, 07
	vpxor	xmm3, xmm4, xmm5			; 1FA8 _ C5 D9: EF. DD
	vpor	xmm4, xmm7, xmm1			; 1FAC _ C5 C1: EB. E1
	vpslld	xmm1, xmm6, 3				; 1FB0 _ C5 F1: 72. F6, 03
	vpxor	xmm5, xmm4, xmm3			; 1FB5 _ C5 D9: EF. EB
	vpslld	xmm7, xmm2, 31				; 1FB9 _ C5 C1: 72. F2, 1F
	vpsrld	xmm2, xmm2, 1				; 1FBE _ C5 E9: 72. D2, 01
	vpxor	xmm0, xmm5, xmm1			; 1FC3 _ C5 D1: EF. C1
	vpor	xmm1, xmm7, xmm2			; 1FC7 _ C5 C1: EB. CA
	vpslld	xmm4, xmm6, 19				; 1FCB _ C5 D9: 72. F6, 13
	vpxor	xmm2, xmm1, xmm6			; 1FD0 _ C5 F1: EF. D6
	vpsrld	xmm6, xmm6, 13				; 1FD4 _ C5 C9: 72. D6, 0D
	vpxor	xmm2, xmm2, xmm3			; 1FD9 _ C5 E9: EF. D3
	vpslld	xmm5, xmm3, 29				; 1FDD _ C5 D1: 72. F3, 1D
	vpsrld	xmm3, xmm3, 3				; 1FE2 _ C5 E1: 72. D3, 03
	vpor	xmm6, xmm4, xmm6			; 1FE7 _ C5 D9: EB. F6
	vpor	xmm1, xmm5, xmm3			; 1FEB _ C5 D1: EB. CB
	vpxor	xmm7, xmm0, xmm6			; 1FEF _ C5 F9: EF. FE
	vpxor	xmm1, xmm1, xmm0			; 1FF3 _ C5 F1: EF. C8
	vpand	xmm0, xmm7, xmm1			; 1FF7 _ C5 C1: DB. C1
	vpxor	xmm5, xmm0, xmm2			; 1FFB _ C5 F9: EF. EA
	vpor	xmm2, xmm2, xmm1			; 1FFF _ C5 E9: EB. D1
	vpxor	xmm0, xmm2, xmm7			; 2003 _ C5 E9: EF. C7
	vpand	xmm7, xmm7, xmm5			; 2007 _ C5 C1: DB. FD
	vpcmpeqd xmm2, xmm2, xmm2			; 200B _ C5 E9: 76. D2
	vpxor	xmm4, xmm1, xmm5			; 200F _ C5 F1: EF. E5
	vpand	xmm1, xmm7, xmm6			; 2013 _ C5 C1: DB. CE
	vpxor	xmm7, xmm5, xmm2			; 2017 _ C5 D1: EF. FA
	vmovd	xmm5, dword [ecx+2AA0H] 		; 201B _ C5 F9: 6E. A9, 00002AA0
	vpxor	xmm3, xmm1, xmm4			; 2023 _ C5 F1: EF. DC
	vpshufd xmm1, xmm5, 0				; 2027 _ C5 F9: 70. CD, 00
	vpand	xmm4, xmm4, xmm0			; 202C _ C5 D9: DB. E0
	vmovd	xmm5, dword [ecx+2AA4H] 		; 2030 _ C5 F9: 6E. A9, 00002AA4
	vpxor	xmm2, xmm0, xmm1			; 2038 _ C5 F9: EF. D1
	vpshufd xmm1, xmm5, 0				; 203C _ C5 F9: 70. CD, 00
	vpxor	xmm5, xmm3, xmm1			; 2041 _ C5 E1: EF. E9
	vpor	xmm1, xmm4, xmm6			; 2045 _ C5 D9: EB. CE
	vmovd	xmm4, dword [ecx+2AA8H] 		; 2049 _ C5 F9: 6E. A1, 00002AA8
	vpxor	xmm6, xmm6, xmm7			; 2051 _ C5 C9: EF. F7
	vpshufd xmm4, xmm4, 0				; 2055 _ C5 F9: 70. E4, 00
	vpxor	xmm1, xmm1, xmm7			; 205A _ C5 F1: EF. CF
	vpxor	xmm3, xmm7, xmm3			; 205E _ C5 C1: EF. DB
	vpand	xmm0, xmm6, xmm0			; 2062 _ C5 C9: DB. C0
	vpxor	xmm1, xmm1, xmm4			; 2066 _ C5 F1: EF. CC
	vpxor	xmm6, xmm3, xmm0			; 206A _ C5 E1: EF. F0
	vmovd	xmm3, dword [ecx+2AACH] 		; 206E _ C5 F9: 6E. 99, 00002AAC
	vpshufd xmm0, xmm3, 0				; 2076 _ C5 F9: 70. C3, 00
	vpslld	xmm3, xmm1, 10				; 207B _ C5 E1: 72. F1, 0A
	vpsrld	xmm1, xmm1, 22				; 2080 _ C5 F1: 72. D1, 16
	vpxor	xmm4, xmm6, xmm0			; 2085 _ C5 C9: EF. E0
	vpor	xmm7, xmm3, xmm1			; 2089 _ C5 E1: EB. F9
	vpslld	xmm0, xmm2, 27				; 208D _ C5 F9: 72. F2, 1B
	vpsrld	xmm2, xmm2, 5				; 2092 _ C5 E9: 72. D2, 05
	vpxor	xmm1, xmm7, xmm4			; 2097 _ C5 C1: EF. CC
	vpslld	xmm6, xmm5, 7				; 209B _ C5 C9: 72. F5, 07
	vpor	xmm7, xmm0, xmm2			; 20A0 _ C5 F9: EB. FA
	vpxor	xmm3, xmm1, xmm6			; 20A4 _ C5 F1: EF. DE
	vpxor	xmm1, xmm7, xmm5			; 20A8 _ C5 C1: EF. CD
	vpxor	xmm0, xmm1, xmm4			; 20AC _ C5 F1: EF. C4
	vpslld	xmm2, xmm4, 25				; 20B0 _ C5 E9: 72. F4, 19
	vpsrld	xmm4, xmm4, 7				; 20B5 _ C5 D9: 72. D4, 07
	vpslld	xmm7, xmm5, 31				; 20BA _ C5 C1: 72. F5, 1F
	vpor	xmm1, xmm2, xmm4			; 20BF _ C5 E9: EB. CC
	vpsrld	xmm5, xmm5, 1				; 20C3 _ C5 D1: 72. D5, 01
	vpxor	xmm2, xmm1, xmm3			; 20C8 _ C5 F1: EF. D3
	vpslld	xmm6, xmm0, 3				; 20CC _ C5 C9: 72. F0, 03
	vpor	xmm1, xmm7, xmm5			; 20D1 _ C5 C1: EB. CD
	vpxor	xmm4, xmm2, xmm6			; 20D5 _ C5 E9: EF. E6
	vpxor	xmm2, xmm1, xmm0			; 20D9 _ C5 F1: EF. D0
	vpslld	xmm7, xmm0, 19				; 20DD _ C5 C1: 72. F0, 13
	vpxor	xmm2, xmm2, xmm3			; 20E2 _ C5 E9: EF. D3
	vpsrld	xmm0, xmm0, 13				; 20E6 _ C5 F9: 72. D0, 0D
	vpslld	xmm6, xmm3, 29				; 20EB _ C5 C9: 72. F3, 1D
	vpsrld	xmm3, xmm3, 3				; 20F0 _ C5 E1: 72. D3, 03
	vpor	xmm5, xmm7, xmm0			; 20F5 _ C5 C1: EB. E8
	vpxor	xmm0, xmm2, xmm4			; 20F9 _ C5 E9: EF. C4
	vpor	xmm1, xmm6, xmm3			; 20FD _ C5 C9: EB. CB
	vpand	xmm4, xmm4, xmm0			; 2101 _ C5 D9: DB. E0
	vpxor	xmm3, xmm2, xmm1			; 2105 _ C5 E9: EF. D9
	vpxor	xmm4, xmm4, xmm5			; 2109 _ C5 D9: EF. E5
	vpor	xmm5, xmm5, xmm0			; 210D _ C5 D1: EB. E8
	vpxor	xmm2, xmm1, xmm4			; 2111 _ C5 F1: EF. D4
	vpxor	xmm6, xmm5, xmm3			; 2115 _ C5 D1: EF. F3
	vpxor	xmm1, xmm0, xmm4			; 2119 _ C5 F9: EF. CC
	vpor	xmm0, xmm6, xmm2			; 211D _ C5 C9: EB. C2
	vpxor	xmm6, xmm0, xmm1			; 2121 _ C5 F9: EF. F1
	vpor	xmm1, xmm1, xmm4			; 2125 _ C5 F1: EB. CC
	vpxor	xmm5, xmm1, xmm6			; 2129 _ C5 F1: EF. EE
	vpcmpeqd xmm1, xmm1, xmm1			; 212D _ C5 F1: 76. C9
	vmovd	xmm0, dword [ecx+2A90H] 		; 2131 _ C5 F9: 6E. 81, 00002A90
	vpxor	xmm3, xmm3, xmm1			; 2139 _ C5 E1: EF. D9
	vpxor	xmm7, xmm3, xmm5			; 213D _ C5 E1: EF. FD
	vpor	xmm5, xmm5, xmm6			; 2141 _ C5 D1: EB. EE
	vpshufd xmm3, xmm0, 0				; 2145 _ C5 F9: 70. D8, 00
	vmovd	xmm0, dword [ecx+2A94H] 		; 214A _ C5 F9: 6E. 81, 00002A94
	vpxor	xmm3, xmm7, xmm3			; 2152 _ C5 C1: EF. DB
	vpshufd xmm0, xmm0, 0				; 2156 _ C5 F9: 70. C0, 00
	vpxor	xmm0, xmm6, xmm0			; 215B _ C5 C9: EF. C0
	vpxor	xmm6, xmm5, xmm6			; 215F _ C5 D1: EF. F6
	vpor	xmm7, xmm6, xmm7			; 2163 _ C5 C9: EB. FF
	vpxor	xmm6, xmm4, xmm7			; 2167 _ C5 D9: EF. F7
	vmovd	xmm4, dword [ecx+2A98H] 		; 216B _ C5 F9: 6E. A1, 00002A98
	vpshufd xmm7, xmm4, 0				; 2173 _ C5 F9: 70. FC, 00
	vmovd	xmm4, dword [ecx+2A9CH] 		; 2178 _ C5 F9: 6E. A1, 00002A9C
	vpxor	xmm7, xmm6, xmm7			; 2180 _ C5 C9: EF. FF
	vpshufd xmm5, xmm4, 0				; 2184 _ C5 F9: 70. EC, 00
	vpsrld	xmm6, xmm7, 22				; 2189 _ C5 C9: 72. D7, 16
	vpxor	xmm4, xmm2, xmm5			; 218E _ C5 E9: EF. E5
	vpslld	xmm2, xmm7, 10				; 2192 _ C5 E9: 72. F7, 0A
	vpor	xmm2, xmm2, xmm6			; 2197 _ C5 E9: EB. D6
	vpslld	xmm6, xmm3, 27				; 219B _ C5 C9: 72. F3, 1B
	vpsrld	xmm3, xmm3, 5				; 21A0 _ C5 E1: 72. D3, 05
	vpxor	xmm7, xmm2, xmm4			; 21A5 _ C5 E9: EF. FC
	vpor	xmm6, xmm6, xmm3			; 21A9 _ C5 C9: EB. F3
	vpslld	xmm5, xmm0, 7				; 21AD _ C5 D1: 72. F0, 07
	vpxor	xmm3, xmm6, xmm0			; 21B2 _ C5 C9: EF. D8
	vpxor	xmm2, xmm7, xmm5			; 21B6 _ C5 C1: EF. D5
	vpxor	xmm3, xmm3, xmm4			; 21BA _ C5 E1: EF. DC
	vpslld	xmm7, xmm4, 25				; 21BE _ C5 C1: 72. F4, 19
	vpsrld	xmm4, xmm4, 7				; 21C3 _ C5 D9: 72. D4, 07
	vpslld	xmm5, xmm0, 31				; 21C8 _ C5 D1: 72. F0, 1F
	vpor	xmm6, xmm7, xmm4			; 21CD _ C5 C1: EB. F4
	vpsrld	xmm0, xmm0, 1				; 21D1 _ C5 F9: 72. D0, 01
	vpxor	xmm7, xmm6, xmm2			; 21D6 _ C5 C9: EF. FA
	vpor	xmm6, xmm5, xmm0			; 21DA _ C5 D1: EB. F0
	vpslld	xmm4, xmm3, 3				; 21DE _ C5 D9: 72. F3, 03
	vpxor	xmm0, xmm6, xmm3			; 21E3 _ C5 C9: EF. C3
	vpxor	xmm4, xmm7, xmm4			; 21E7 _ C5 C1: EF. E4
	vpxor	xmm5, xmm0, xmm2			; 21EB _ C5 F9: EF. EA
	vpslld	xmm7, xmm3, 19				; 21EF _ C5 C1: 72. F3, 13
	vpsrld	xmm3, xmm3, 13				; 21F4 _ C5 E1: 72. D3, 0D
	vpslld	xmm0, xmm2, 29				; 21F9 _ C5 F9: 72. F2, 1D
	vpsrld	xmm2, xmm2, 3				; 21FE _ C5 E9: 72. D2, 03
	vpor	xmm6, xmm7, xmm3			; 2203 _ C5 C1: EB. F3
	vpor	xmm2, xmm0, xmm2			; 2207 _ C5 F9: EB. D2
	vpxor	xmm7, xmm2, xmm1			; 220B _ C5 E9: EF. F9
	vpxor	xmm0, xmm5, xmm1			; 220F _ C5 D1: EF. C1
	vpor	xmm1, xmm5, xmm6			; 2213 _ C5 D1: EB. CE
	vpxor	xmm5, xmm6, xmm0			; 2217 _ C5 C9: EF. E8
	vpxor	xmm1, xmm1, xmm7			; 221B _ C5 F1: EF. CF
	vpor	xmm6, xmm7, xmm0			; 221F _ C5 C1: EB. F0
	vpxor	xmm1, xmm1, xmm4			; 2223 _ C5 F1: EF. CC
	vpxor	xmm3, xmm6, xmm5			; 2227 _ C5 C9: EF. DD
	vpand	xmm6, xmm5, xmm4			; 222B _ C5 D1: DB. F4
	vpxor	xmm2, xmm0, xmm6			; 222F _ C5 F9: EF. D6
	vpor	xmm0, xmm6, xmm1			; 2233 _ C5 C9: EB. C1
	vpxor	xmm7, xmm0, xmm3			; 2237 _ C5 F9: EF. FB
	vpxor	xmm4, xmm4, xmm2			; 223B _ C5 D9: EF. E2
	vpxor	xmm6, xmm4, xmm7			; 223F _ C5 D9: EF. F7
	vpxor	xmm3, xmm3, xmm1			; 2243 _ C5 E1: EF. D9
	vmovd	xmm0, dword [ecx+2A80H] 		; 2247 _ C5 F9: 6E. 81, 00002A80
	vpxor	xmm6, xmm6, xmm1			; 224F _ C5 C9: EF. F1
	vpshufd xmm4, xmm0, 0				; 2253 _ C5 F9: 70. E0, 00
	vpand	xmm5, xmm3, xmm6			; 2258 _ C5 E1: DB. EE
	vpxor	xmm0, xmm7, xmm4			; 225C _ C5 C1: EF. C4
	vpxor	xmm3, xmm2, xmm5			; 2260 _ C5 E9: EF. DD
	vmovd	xmm7, dword [ecx+2A84H] 		; 2264 _ C5 F9: 6E. B9, 00002A84
	inc	eax					; 226C _ 40
	vpshufd xmm2, xmm7, 0				; 226D _ C5 F9: 70. D7, 00
	add	esi, 64 				; 2272 _ 83. C6, 40
	vpxor	xmm5, xmm3, xmm2			; 2275 _ C5 E1: EF. EA
	vmovd	xmm3, dword [ecx+2A88H] 		; 2279 _ C5 F9: 6E. 99, 00002A88
	vpshufd xmm4, xmm3, 0				; 2281 _ C5 F9: 70. E3, 00
	vpxor	xmm4, xmm1, xmm4			; 2286 _ C5 F1: EF. E4
	vmovd	xmm1, dword [ecx+2A8CH] 		; 228A _ C5 F9: 6E. 89, 00002A8C
	vpshufd xmm7, xmm1, 0				; 2292 _ C5 F9: 70. F9, 00
	vpxor	xmm6, xmm6, xmm7			; 2297 _ C5 C9: EF. F7
	vpunpckldq xmm2, xmm0, xmm5			; 229B _ C5 F9: 62. D5
	vpunpckldq xmm3, xmm4, xmm6			; 229F _ C5 D9: 62. DE
	vpunpckhdq xmm1, xmm0, xmm5			; 22A3 _ C5 F9: 6A. CD
	vpunpckhdq xmm0, xmm4, xmm6			; 22A7 _ C5 D9: 6A. C6
	vpunpcklqdq xmm4, xmm2, xmm3			; 22AB _ C5 E9: 6C. E3
	vpxor	xmm5, xmm4, oword [esp+70H]		; 22AF _ C5 D9: EF. 6C 24, 70
	vmovdqu xmm4, oword [esp+60H]			; 22B5 _ C5 FA: 6F. 64 24, 60
	vpunpcklqdq xmm6, xmm1, xmm0			; 22BB _ C5 F1: 6C. F0
	vpunpckhqdq xmm0, xmm1, xmm0			; 22BF _ C5 F1: 6D. C0
	vpunpckhqdq xmm2, xmm2, xmm3			; 22C3 _ C5 E9: 6D. D3
	vpxor	xmm1, xmm0, xmm4			; 22C7 _ C5 F9: EF. CC
	vpslldq xmm0, xmm4, 8				; 22CB _ C5 F9: 73. FC, 08
	vpxor	xmm3, xmm2, oword [esp+50H]		; 22D0 _ C5 E9: EF. 5C 24, 50
	vpsllq	xmm2, xmm4, 1				; 22D6 _ C5 E9: 73. F4, 01
	vmovdqu oword [edx], xmm5			; 22DB _ C5 FA: 7F. 2A
	vmovdqu oword [edx+30H], xmm1			; 22DF _ C5 FA: 7F. 4A, 30
	vmovdqu oword [edx+10H], xmm3			; 22E4 _ C5 FA: 7F. 5A, 10
	vpsrldq xmm1, xmm0, 7				; 22E9 _ C5 F1: 73. D8, 07
	vpsraw	xmm5, xmm4, 8				; 22EE _ C5 D1: 71. E4, 08
	vpxor	xmm7, xmm6, oword [esp+40H]		; 22F3 _ C5 C9: EF. 7C 24, 40
	vpsrlq	xmm3, xmm1, 7				; 22F9 _ C5 E1: 73. D1, 07
	vpsrldq xmm6, xmm5, 15				; 22FE _ C5 C9: 73. DD, 0F
	vpand	xmm0, xmm6, oword [esp+30H]		; 2303 _ C5 C9: DB. 44 24, 30
	vmovdqu oword [edx+20H], xmm7			; 2309 _ C5 FA: 7F. 7A, 20
	vpor	xmm7, xmm2, xmm3			; 230E _ C5 E9: EB. FB
	add	edx, 64 				; 2312 _ 83. C2, 40
	vpxor	xmm1, xmm7, xmm0			; 2315 _ C5 C1: EF. C8
	cmp	eax, 8					; 2319 _ 83. F8, 08
	vmovdqu oword [esp+70H], xmm1			; 231C _ C5 FA: 7F. 4C 24, 70
	jl	?_006					; 2322 _ 0F 8C, FFFFDD72
	mov	dword [esp+28H], esi			; 2328 _ 89. 74 24, 28
	mov	esi, dword [esp+20H]			; 232C _ 8B. 74 24, 20
	vmovdqu xmm0, oword [esp+70H]			; 2330 _ C5 FA: 6F. 44 24, 70
	add	esi, -512				; 2336 _ 81. C6, FFFFFE00
	mov	dword [esp+24H], edx			; 233C _ 89. 54 24, 24
	jne	?_004					; 2340 _ 0F 85, FFFFDD1C
	add	esp, 132				; 2346 _ 81. C4, 00000084
	pop	ebx					; 234C _ 5B
	pop	edi					; 234D _ 5F
	pop	esi					; 234E _ 5E
	mov	esp, ebp				; 234F _ 8B. E5
	pop	ebp					; 2351 _ 5D
	ret	24					; 2352 _ C2, 0018
; _xts_serpent_avx_decrypt@24 End of function

; Filling space: 0BH
; Filler type: Multi-byte NOP
;       db 0FH, 1FH, 40H, 00H, 0FH, 1FH, 80H, 00H
;       db 00H, 00H, 00H

ALIGN	16


_xts_serpent_avx_available@0:; Function begin
	push	ebx					; 0000 _ 53
	push	esi					; 0001 _ 56
	mov	dword [esp], 0				; 0002 _ C7. 04 24, 00000000
	mov	eax, 1					; 0009 _ B8, 00000001
	cpuid						; 000E _ 0F A2
	and	ecx, 18000000H				; 0010 _ 81. E1, 18000000
	cmp	ecx, 402653184				; 0016 _ 81. F9, 18000000
	jnz	?_008					; 001C _ 75, 14
	xor	ecx, ecx				; 001E _ 33. C9
?_007:
; Error: Illegal opcode
; Error: Internal error in opcode table in opcodes.cpp
;       UNDEFINED                                       ; 0020 _ 0F 01. D0
	db 0FH, 01H, 0D0H
;       and     eax, 06H                                ; 0023 _ 83. E0, 06
	db 83H, 0E0H, 06H
;       cmp     eax, 6                                  ; 0026 _ 83. F8, 06
	db 83H, 0F8H, 06H
;       jnz     ?_008                                   ; 0029 _ 75, 07
	db 75H, 07H
;       mov     dword [esp], 1                          ; 002B _ C7. 04 24, 00000001
	db 0C7H, 04H, 24H, 01H, 00H, 00H, 00H
?_008:	mov	eax, dword [esp]			; 0032 _ 8B. 04 24
	pop	ecx					; 0035 _ 59
	pop	ebx					; 0036 _ 5B
	ret						; 0037 _ C3
; _xts_serpent_avx_available@0 End of function

