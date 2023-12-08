;***************************************************************************
;*   Copyright (C) 2006 by Joachim Fritschi, <jfritschi@freenet.de>        *
;*   adapted for DiskCryptor by ntldr <ntldr@diskcryptor.net>              *
;*       PGP key ID - 0x1B6A24550F33E44A                                   *
;*                                                                         *
;*   This program is free software; you can redistribute it and/or modify  *
;*   it under the terms of the GNU General Public License as published by  *
;*   the Free Software Foundation; either version 2 of the License, or     *
;*   (at your option) any later version.                                   *
;*                                                                         *
;*   This program is distributed in the hope that it will be useful,       *
;*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
;*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
;*   GNU General Public License for more details.                          *
;*                                                                         *
;*   You should have received a copy of the GNU General Public License     *
;*   along with this program; if not, write to the                         *
;*   Free Software Foundation, Inc.,                                       *
;*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
;***************************************************************************

%define a_offset 0
%define b_offset 4
%define c_offset 8
%define d_offset 12

; Structure of the crypto context struct
%define s0  0	    ; S0 Array 256 Words each
%define s1  1024    ; S1 Array
%define s2  2048    ; S2 Array
%define s3  3072    ; S3 Array
%define w   4096    ; 8 whitening keys (word)
%define k   4128    ; key 1-32 ( word )

; define a few register aliases to allow macro substitution
%define R0Q rax
%define R0D eax
%define R0B al
%define R0H ah

%define R1Q rbx
%define R1D ebx
%define R1B bl
%define R1H bh

%define R2Q rcx
%define R2D ecx
%define R2B cl
%define R2H ch

%define R3Q rdx
%define R3D edx
%define R3B dl
%define R3H dh


; performs input whitening
%macro input_whitening 3
  xor %1, [w+(%2)+%3]
%endmacro

; performs input whitening
%macro output_whitening 3
  xor %1, [w+16+(%2)+%3]
%endmacro


; * a input register containing a (rotated 16)
; * b input register containing b
; * c input register containing c
; * d input register containing d (already rol $1)
; * operations on a and b are interleaved to increase performance
%macro encrypt_round 5
	movzx	edi,  %2B
	mov	r11d, [r8+rdi*4+s1]
	movzx	edi,  %1B
	mov	r9d,  [r8+rdi*4+s2]
	movzx	edi,  %2H
	ror	%2D,  16
	xor	r11d, [r8+rdi*4+s2]
	movzx	edi,  %1H
	ror	%1D,  16
	xor	r9d,  [r8+rdi*4+s3]
	movzx	edi,  %2B
	xor	r11d, [r8+rdi*4+s3]
	movzx	edi,  %1B
	xor	r9d,  [r8+rdi*4]
	movzx	edi,  %2H
	ror	%2D,  15
	xor	r11d, [r8+rdi*4]
	movzx	edi,  %1H
	xor	r9d,  [r8+rdi*4+s1]
	add	r9d,  r11d
	add	r11d, r9d
	add	r9d,  [r8+k+%5]
	xor	%3D,  r9d
	rol	%3D,  15
	add	r11d, [r8+k+4+%5]
	xor	%4D,  r11d
%endmacro

; * a input register containing a(rotated 16)
; * b input register containing b
; * c input register containing c
; * d input register containing d (already rol $1)
; * operations on a and b are interleaved to increase performance
; * during the %5 a and b are prepared for the output whitening
%macro encrypt_last_round 5
	mov	r10d, %2D
	shl	r10,  32
	movzx	edi,  %2B
	mov	r11d, [r8+rdi*4+s1]
	movzx	edi,  %1B
	mov	r9d,  [r8+rdi*4+s2]
	movzx	edi,  %2H
	ror	%2D,  16
	xor	r11d, [r8+rdi*4+s2]
	movzx	edi,  %1H
	ror	%1D,  16
	xor	r9d,  [r8+rdi*4+s3]
	movzx	edi,  %2B
	xor	r11d, [r8+rdi*4+s3]
	movzx	edi,  %1B
	xor	r9d,  [r8+rdi*4]
	xor	r10,  %1Q
	movzx	edi,  %2H
	xor	r11d, [r8+rdi*4]
	movzx	edi,  %1H
	xor	r9d,  [r8+rdi*4+s1]
	add	r9d,  r11d
	add	r11d, r9d
	add	r9d,  [r8+k+%5]
	xor	%3D,  r9d
	ror	%3D,  1
	add	r11d, [r8+k+4+%5]
	xor	%4D,  r11d
%endmacro

; * a input register containing a
; * b input register containing b (rotated 16)
; * c input register containing c (already rol $1)
; * d input register containing d
; * operations on a and b are interleaved to increase performance
%macro decrypt_round 5
	movzx	edi,  %1B
	mov	r9d,  [r8+rdi*4]
	movzx	edi,  %2B
	mov	r11d, [r8+rdi*4+s3]
	movzx	edi,  %1H
	ror	%1D,  16
	xor	r9d,  [r8+rdi*4+s1]
	movzx	edi,  %2H
	ror	%2D,  16
	xor	r11d, [r8+rdi*4]
	movzx	edi,  %1B
	xor	r9d,  [r8+rdi*4+s2]
	movzx	edi,  %2B
	xor	r11d, [r8+rdi*4+s1]
	movzx	edi,  %1H
	ror	%1D,  15
	xor	r9d,  [r8+rdi*4+s3]
	movzx	edi,  %2H
	xor	r11d, [r8+rdi*4+s2]
	add	r9d,  r11d
	add	r11d, r9d
	add	r9d,  [r8+k+%5]
	xor	%3D,  r9d
	add	r11d, [r8+k+4+%5]
	xor	%4D,  r11d
	rol	%4D,  15
%endmacro

; * a input register containing a
; * b input register containing b
; * c input register containing c (already rol $1)
; * d input register containing d
; * operations on a and b are interleaved to increase performance
; * during the %5 a and b are prepared for the output whitening
%macro decrypt_last_round 5
	movzx	edi,  %1B
	mov	r9d,  [r8+rdi*4]
	movzx	edi,  %2B
	mov	r11d, [r8+rdi*4+s3]
	movzx	edi,  %2H
	ror	%2D,  16
	xor	r11d, [r8+rdi*4]
	movzx	edi,  %1H
	mov	r10d, %2D
	shl	r10,  32
	xor	r10,  %1Q
	ror	%1D,  16
	xor	r9d,  [r8+rdi*4+s1]
	movzx	edi,  %2B
	xor	r11d, [r8+rdi*4+s1]
	movzx	edi,  %1B
	xor	r9d,  [r8+rdi*4+s2]
	movzx	edi,  %2H
	xor	r11d, [r8+rdi*4+s2]
	movzx	edi,  %1H
	xor	r9d,  [r8+rdi*4+s3]
	add	r9d,  r11d
	add	r11d, r9d
	add	r9d,  [r8+k+%5]
	xor	%3D,  r9d
	add	r11d, [r8+k+4+%5]
	xor	%4D,  r11d
	ror	%4D,  1
%endmacro

global twofish256_encrypt
global twofish256_decrypt

align 16
twofish256_encrypt:
	push	R1Q
	push	rsi
	push	rdi
	; r8 contains the crypto tfm address
	; rdx contains the output address
	; rcx contains the input address
	mov	rsi, rdx

	mov	R1Q, [rcx]
	mov	R3Q, [rcx+8]

	input_whitening R1Q, r8, a_offset
	input_whitening R3Q, r8, c_offset
	mov	R0D, R1D
	rol	R0D, 16
	shr	R1Q, 32
	mov	R2D, R3D
	shr	R3Q, 32
	rol	R3D, 1

	encrypt_round R0,R1,R2,R3,0
	encrypt_round R2,R3,R0,R1,8
	encrypt_round R0,R1,R2,R3,2*8
	encrypt_round R2,R3,R0,R1,3*8
	encrypt_round R0,R1,R2,R3,4*8
	encrypt_round R2,R3,R0,R1,5*8
	encrypt_round R0,R1,R2,R3,6*8
	encrypt_round R2,R3,R0,R1,7*8

	encrypt_round R0,R1,R2,R3,8*8
	encrypt_round R2,R3,R0,R1,9*8
	encrypt_round R0,R1,R2,R3,10*8
	encrypt_round R2,R3,R0,R1,11*8
	encrypt_round R0,R1,R2,R3,12*8
	encrypt_round R2,R3,R0,R1,13*8
	encrypt_round R0,R1,R2,R3,14*8
	encrypt_last_round R2,R3,R0,R1,15*8

	output_whitening r10, r8, a_offset
	mov	[rsi], r10

	shl	R1Q, 32
	xor	R1Q, R0Q

	output_whitening R1Q, r8, c_offset
	mov	[rsi+8], R1Q

	pop	rdi
	pop	rsi

	pop	R1Q
	ret

align 16
twofish256_decrypt:
	push	R1Q
	push	rsi
	push	rdi
	; r8 contains the crypto tfm address
	; rdx contains the output address
	; rcx contains the input address
	mov	rsi, rdx

	mov	R1Q, [rcx]
	mov	R3Q, [rcx+8]

	output_whitening R1Q, r8, a_offset
	output_whitening R3Q, r8, c_offset
	mov	R0D, R1D
	shr	R1Q, 32
	rol	R1D, 16
	mov	R2D, R3D
	shr	R3Q, 32
	rol	R2D, 1

	decrypt_round R0,R1,R2,R3,15*8
	decrypt_round R2,R3,R0,R1,14*8
	decrypt_round R0,R1,R2,R3,13*8
	decrypt_round R2,R3,R0,R1,12*8
	decrypt_round R0,R1,R2,R3,11*8
	decrypt_round R2,R3,R0,R1,10*8
	decrypt_round R0,R1,R2,R3,9*8
	decrypt_round R2,R3,R0,R1,8*8
	decrypt_round R0,R1,R2,R3,7*8
	decrypt_round R2,R3,R0,R1,6*8
	decrypt_round R0,R1,R2,R3,5*8
	decrypt_round R2,R3,R0,R1,4*8
	decrypt_round R0,R1,R2,R3,3*8
	decrypt_round R2,R3,R0,R1,2*8
	decrypt_round R0,R1,R2,R3,1*8
	decrypt_last_round R2,R3,R0,R1,0

	input_whitening r10, r8, a_offset
	mov	[rsi], r10

	shl	R1Q, 32
	xor	R1Q, R0Q

	input_whitening R1Q, r8, c_offset
	mov	[rsi+8], R1Q

	pop	rdi
	pop	rsi

	pop	R1Q
	ret
