;***************************************************************************
;*   Copyright (C) 2006 by Joachim Fritschi, <jfritschi@freenet.de>        *
;*   adapted for DiskCryptor by ntldr <ntldr@diskcryptor.net>                  *
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

%define in_blk	4  ; input byte array address parameter
%define out_blk 8  ; output byte array address parameter
%define tfm	12 ; Twofish context structure

%define a_offset 0
%define b_offset 4
%define c_offset 8
%define d_offset 12

; Structure of the crypto context struct
%define s0     0       ; S0 Array 256 Words each
%define s1     1024    ; S1 Array
%define s2     2048    ; S2 Array
%define s3     3072    ; S3 Array
%define w      4096    ; 8 whitening keys (word)
%define k      4128    ; key 1-32 ( word )

; define a few register aliases to allow macro substitution
%define R0D eax
%define R0B al
%define R0H ah

%define R1D ebx
%define R1B bl
%define R1H bh

%define R2D ecx
%define R2B cl
%define R2H ch

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

;
; * a input register containing a (rotated 16)
; * b input register containing b
; * c input register containing c
; * d input register containing d (already rol $1)
; * operations on a and b are interleaved to increase performance

%macro encrypt_round 5
	push	%4D
	movzx	edi, %2B
	mov	%4D, [ebp+edi*4+s1]
	movzx	edi, %1B
	mov	esi, [ebp+edi*4+s2]
	movzx	edi, %2H
	ror	%2D, 16
	xor	%4D, [ebp+edi*4+s2]
	movzx	edi, %1H
	ror	%1D, 16
	xor	esi, [ebp+edi*4+s3]
	movzx	edi, %2B
	xor	%4D, [ebp+edi*4+s3]
	movzx	edi, %1B
	xor	esi, [ebp+edi*4]
	movzx	edi, %2H
	ror	%2D, 15
	xor	%4D, [ebp+edi*4]
	movzx	edi, %1H
	xor	esi, [ebp+edi*4+s1]
	pop	edi
	add	esi, %4D
	add	%4D, esi
	add	esi, [ebp+k+%5]
	xor	%3D, esi
	rol	%3D, 15
	add	%4D, [ebp+k+4+%5]
	xor	%4D, edi
%endmacro

; * a input register containing a (rotated 16)
; * b input register containing b
; * c input register containing c
; * d input register containing d (already rol $1)
; * operations on a and b are interleaved to increase performance
; * last round has different rotations for the output preparation
%macro encrypt_last_round 5
	push	%4D
	movzx	edi, %2B
	mov	%4D, [ebp+edi*4+s1]
	movzx	edi, %1B
	mov	esi, [ebp+edi*4+s2]
	movzx	edi, %2H
	ror	%2D, 16
	xor	%4D, [ebp+edi*4+s2]
	movzx	edi, %1H
	ror	%1D, 16
	xor	esi, [ebp+edi*4+s3]
	movzx	edi, %2B
	xor	%4D, [ebp+edi*4+s3]
	movzx	edi, %1B
	xor	esi, [ebp+edi*4]
	movzx	edi, %2H
	ror	%2D, 16
	xor	%4D, [ebp+edi*4]
	movzx	edi, %1H
	xor	esi, [ebp+edi*4+s1]
	pop	edi
	add	esi, %4D
	add	%4D, esi
	add	esi, [ebp+k+%5]
	xor	%3D, esi
	ror	%3D, 1
	add	%4D, [ebp+k+4+%5]
	xor	%4D, edi
%endmacro

; * a input register containing a
; * b input register containing b (rotated 16)
; * c input register containing c
; * d input register containing d (already rol $1)
; * operations on a and b are interleaved to increase performance
%macro decrypt_round 5
	push	%3D
	movzx	edi, %1B
	mov	%3D, [ebp+edi*4]
	movzx	edi, %2B
	mov	esi, [ebp+edi*4+s3]
	movzx	edi, %1H
	ror	%1D, 16
	xor	%3D, [ebp+edi*4+s1]
	movzx	edi, %2H
	ror	%2D, 16
	xor	esi, [ebp+edi*4]
	movzx	edi, %1B
	xor	%3D, [ebp+edi*4+s2]
	movzx	edi, %2B
	xor	esi, [ebp+edi*4+s1]
	movzx	edi, %1H
	ror	%1D, 15
	xor	%3D, [ebp+edi*4+s3]
	movzx	edi, %2H
	xor	esi, [ebp+edi*4+s2]
	pop	edi
	add	%3D, esi
	add	esi, %3D
	add	%3D, [ebp+k+%5]
	xor	%3D, edi
	add	esi, [ebp+k+4+%5]
	xor	%4D, esi
	rol	%4D, 15
%endmacro

; * a input register containing a
; * b input register containing b (rotated 16)
; * c input register containing c
; * d input register containing d (already rol $1)
; * operations on a and b are interleaved to increase performance
; * last round has different rotations for the output preparation
%macro decrypt_last_round 5
	push	%3D
	movzx	edi, %1B
	mov	%3D, [ebp+edi*4]
	movzx	edi, %2B
	mov	esi, [ebp+edi*4+s3]
	movzx	edi, %1H
	ror	%1D, 16
	xor	%3D, [ebp+edi*4+s1]
	movzx	edi, %2H
	ror	%2D, 16
	xor	esi, [ebp+edi*4]
	movzx	edi, %1B
	xor	%3D, [ebp+edi*4+s2]
	movzx	edi, %2B
	xor	esi, [ebp+edi*4+s1]
	movzx	edi, %1H
	ror	%1D, 16
	xor	%3D, [ebp+edi*4+s3]
	movzx	edi, %2H
	xor	esi, [ebp+edi*4+s2]
	pop	edi
	add	%3D, esi
	add	esi, %3D
	add	%3D, [ebp+k+%5]
	xor	%3D, edi
	add	esi, [ebp+k+4+%5]
	xor	%4D, esi
	ror	%4D, 1
%endmacro

global _twofish256_encrypt@12
global _twofish256_decrypt@12

_twofish256_encrypt@12:
	push	ebp		       ; save registers according to calling convention
	push	ebx
	push	esi
	push	edi
	mov	ebp, [tfm + 16+esp]    ; abuse the base pointer: set new base pointer to the crypto tfm
	mov	edi, [in_blk+16+esp]   ; input address in edi

	mov	eax, [edi]
	mov	ebx, [b_offset+edi]
	mov	ecx, [c_offset+edi]
	mov	edx, [d_offset+edi]
	input_whitening eax, ebp, a_offset
	ror	eax, 16
	input_whitening ebx, ebp, b_offset
	input_whitening ecx, ebp, c_offset
	input_whitening edx, ebp, d_offset
	rol	edx, 1

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

	output_whitening eax, ebp, c_offset
	output_whitening ebx, ebp, d_offset
	output_whitening ecx, ebp, a_offset
	output_whitening edx, ebp, b_offset
	mov	edi, [out_blk+16+esp]
	mov	[c_offset+edi], eax
	mov	[d_offset+edi], ebx
	mov	[edi], ecx
	mov	[b_offset+edi], edx
	pop	edi
	pop	esi
	pop	ebx
	pop	ebp
	retn	0Ch


_twofish256_decrypt@12:
	push	ebp		       ; save registers according to calling convention*/
	push	ebx
	push	esi
	push	edi


	mov	ebp, [tfm + 16+esp]    ; abuse the base pointer: set new base pointer to the crypto tfm
	mov	edi, [in_blk + 16+esp] ; input address in edi

	mov	eax, [edi]
	mov	ebx, [b_offset+edi]
	mov	ecx, [c_offset+edi]
	mov	edx, [d_offset+edi]
	output_whitening eax, ebp, a_offset
	output_whitening ebx, ebp, b_offset
	ror	ebx, 16
	output_whitening ecx, ebp, c_offset
	output_whitening edx, ebp, d_offset
	rol	ecx, 1

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

	input_whitening eax, ebp, c_offset
	input_whitening ebx, ebp, d_offset
	input_whitening ecx, ebp, a_offset
	input_whitening edx, ebp, b_offset
	mov	edi, [out_blk + 16+esp]
	mov	[c_offset+edi], eax
	mov	[d_offset+edi], ebx
	mov	[edi], ecx
	mov	[b_offset+edi], edx

	pop	edi
	pop	esi
	pop	ebx
	pop	ebp
	retn	0Ch
