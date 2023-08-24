;
;   *
;   * Copyright (c) 2010
;   * ntldr <ntldr@diskcryptor.net> PGP key ID - 0x1B6A24550F33E44A
;   *
;   This program is free software: you can redistribute it and/or modify
;   it under the terms of the GNU General Public License version 3 as
;   published by the Free Software Foundation.
;
;   This program is distributed in the hope that it will be useful,
;   but WITHOUT ANY WARRANTY; without even the implied warranty of
;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;   GNU General Public License for more details.
;
;   You should have received a copy of the GNU General Public License
;   along with this program.  If not, see <http://www.gnu.org/licenses/>.
;

%macro aesxor_4 6 ; B0, B1, B2, B3, key, round
  movdqa     tt, [%5+(%6*10h)]
  pxor	     %1, tt
  pxor	     %2, tt
  pxor	     %3, tt
  pxor	     %4, tt
%endmacro

%macro aesenc_4 6 ; B0, B1, B2, B3, key, round
  movdqa     tt, [%5+(%6*10h)]
  aesenc     %1, tt
  aesenc     %2, tt
  aesenc     %3, tt
  aesenc     %4, tt
%endmacro

%macro aesdec_4 6 ; B0, B1, B2, B3, key, round
  movdqa     tt, [%5+(%6*10h)]
  aesdec     %1, tt
  aesdec     %2, tt
  aesdec     %3, tt
  aesdec     %4, tt
%endmacro

%macro aesenclast_4 6 ; B0, B1, B2, B3, key, round
  movdqa     tt, [%5+(%6*10h)]
  aesenclast %1, tt
  aesenclast %2, tt
  aesenclast %3, tt
  aesenclast %4, tt
%endmacro

%macro aesdeclast_4 6 ; B0, B1, B2, B3, key, round
  movdqa     tt, [%5+(%6*10h)]
  aesdeclast %1, tt
  aesdeclast %2, tt
  aesdeclast %3, tt
  aesdeclast %4, tt
%endmacro

%macro aes_encrypt_1 2	; XMMn, key
 pxor	    %1, [%2]
 aesenc     %1, [%2+010h]
 aesenc     %1, [%2+020h]
 aesenc     %1, [%2+030h]
 aesenc     %1, [%2+040h]
 aesenc     %1, [%2+050h]
 aesenc     %1, [%2+060h]
 aesenc     %1, [%2+070h]
 aesenc     %1, [%2+080h]
 aesenc     %1, [%2+090h]
 aesenc     %1, [%2+0A0h]
 aesenc     %1, [%2+0B0h]
 aesenc     %1, [%2+0C0h]
 aesenc     %1, [%2+0D0h]
 aesenclast %1, [%2+0E0h]
%endmacro

%macro aes_encrypt_4 5 ; B0, B1, B2, B3, key
 aesxor_4     %1, %2, %3, %4, %5, 0
 aesenc_4     %1, %2, %3, %4, %5, 1
 aesenc_4     %1, %2, %3, %4, %5, 2
 aesenc_4     %1, %2, %3, %4, %5, 3
 aesenc_4     %1, %2, %3, %4, %5, 4
 aesenc_4     %1, %2, %3, %4, %5, 5
 aesenc_4     %1, %2, %3, %4, %5, 6
 aesenc_4     %1, %2, %3, %4, %5, 7
 aesenc_4     %1, %2, %3, %4, %5, 8
 aesenc_4     %1, %2, %3, %4, %5, 9
 aesenc_4     %1, %2, %3, %4, %5, 10
 aesenc_4     %1, %2, %3, %4, %5, 11
 aesenc_4     %1, %2, %3, %4, %5, 12
 aesenc_4     %1, %2, %3, %4, %5, 13
 aesenclast_4 %1, %2, %3, %4, %5, 14
%endmacro

%macro aes_decrypt_4 5 ; B0, B1, B2, B3, key
 aesxor_4     %1, %2, %3, %4, %5, 0
 aesdec_4     %1, %2, %3, %4, %5, 1
 aesdec_4     %1, %2, %3, %4, %5, 2
 aesdec_4     %1, %2, %3, %4, %5, 3
 aesdec_4     %1, %2, %3, %4, %5, 4
 aesdec_4     %1, %2, %3, %4, %5, 5
 aesdec_4     %1, %2, %3, %4, %5, 6
 aesdec_4     %1, %2, %3, %4, %5, 7
 aesdec_4     %1, %2, %3, %4, %5, 8
 aesdec_4     %1, %2, %3, %4, %5, 9
 aesdec_4     %1, %2, %3, %4, %5, 10
 aesdec_4     %1, %2, %3, %4, %5, 11
 aesdec_4     %1, %2, %3, %4, %5, 12
 aesdec_4     %1, %2, %3, %4, %5, 13
 aesdeclast_4 %1, %2, %3, %4, %5, 14
%endmacro

%macro next_tweak 2 ; new, old
 movdqa  tt, %2
 psraw	 tt, 8
 psrldq  tt, 15
 pand	 tt, POLY
 movdqa  t2, %2
 pslldq  t2, 8
 psrldq  t2, 7
 psrlq	 t2, 7
 movdqa  %1, %2
 psllq	 %1, 1
 por	 %1, t2
 pxor	 %1, tt
%endmacro

%macro tweak_block_4 0
 pxor	       B0, T0
 pxor	       B1, T1
 pxor	       B2, T2
 pxor	       B3, T3
%endmacro

%macro load_block_4 1
 movdqu        B0, [%1+00h]
 movdqu        B1, [%1+10h]
 movdqu        B2, [%1+20h]
 movdqu        B3, [%1+30h]
%endmacro

%macro save_block_4 1
 movdqu        [%1+00h], B0
 movdqu        [%1+10h], B1
 movdqu        [%1+20h], B2
 movdqu        [%1+30h], B3
%endmacro

%macro aes_xts_process 2
 ; rcx = in, rdx = out, r8 = len, r9 = offset, [rsp+28h] = key
 sub	       rsp, 58h
 ; save nonvolatile XMM registers
 movaps        [rsp+40h], xmm6
 movaps        [rsp+30h], xmm7
 movaps        [rsp+20h], xmm8
 movaps        [rsp+10h], xmm9
 movaps        [rsp+00h], xmm10
 ; load XTS tweak polynomial
 mov	       eax, 135
 movd	       POLY, eax
 ; load pointers of keys
 mov	       rax, [rsp+28h+58h]
 lea	       r11, [rax+tweak_k] ; r11 - tweak key
%if %2 != 0
 add	       rax, %2 ; rax - encryption key
%endif
 shr	       r9, 9   ; idx = offset / XTS_SECTOR_SIZE
 shr	       r8, 9   ; len /= XTS_SECTOR_SIZE
%%xts_loop:
 inc	       r9      ; idx++
 movq	       T0, r9
 aes_encrypt_1 T0, r11
 mov	       r10d, 8 ; XTS_BLOCKS_IN_SECTOR / 4
%%blocks_loop:
 ; calc tweaks
 next_tweak    T1, T0
 next_tweak    T2, T1
 next_tweak    T3, T2
 ; load blocks
 load_block_4  rcx
 add	       rcx, 64 ; in += XTS_BLOCK_SIZE*4
align 16
 ; input tweak
 tweak_block_4
 ; encrypt / decrypt
 %1	       B0, B1, B2, B3, rax
 ; output tweak
 tweak_block_4
 ; save blocks
 save_block_4  rdx
 dec	       r10d
 jz	       %%block_done
 next_tweak    T0, T3
 add	       rdx, 64 ; out += XTS_BLOCK_SIZE*4
 jmp	       %%blocks_loop
%%block_done:
 add	       rdx, 64 ; out += XTS_BLOCK_SIZE*4
 dec	       r8
 jnz	       %%xts_loop
 ; restore nonvolatile XMM registers
 movaps        xmm6,  [rsp+40h]
 movaps        xmm7,  [rsp+30h]
 movaps        xmm8,  [rsp+20h]
 movaps        xmm9,  [rsp+10h]
 movaps        xmm10, [rsp+00h]
 add	       rsp, 58h
 ret
%endmacro

%define B0   xmm0
%define B1   xmm1
%define B2   xmm2
%define B3   xmm3

%define T0   xmm4
%define T1   xmm5
%define T2   xmm6
%define T3   xmm7

%define tt   xmm8
%define t2   xmm9
%define POLY xmm10

%define tweak_k 5264
%define enc_key 0
%define dec_key 4*15*4

global xts_aes_ni_encrypt
global xts_aes_ni_decrypt

xts_aes_ni_encrypt:
 aes_xts_process aes_encrypt_4, enc_key

align 16
xts_aes_ni_decrypt:
 aes_xts_process aes_decrypt_4, dec_key






