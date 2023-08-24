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

%macro aesxor_2 4 ; B0, B1, key, round
  movdqa     tt, [%3+(%4*10h)]
  pxor	     %1, tt
  pxor	     %2, tt
%endmacro

%macro aesenc_2 4 ; B0, B1, key, round
  movdqa     tt, [%3+(%4*10h)]
  aesenc     %1, tt
  aesenc     %2, tt
%endmacro

%macro aesdec_2 4 ; B0, B1, key, round
  movdqa     tt, [%3+(%4*10h)]
  aesdec     %1, tt
  aesdec     %2, tt
%endmacro

%macro aesenclast_2 4 ; B0, B1, key, round
  movdqa     tt, [%3+(%4*10h)]
  aesenclast %1, tt
  aesenclast %2, tt
%endmacro

%macro aesdeclast_2 4 ; B0, B1, key, round
  movdqa     tt, [%3+(%4*10h)]
  aesdeclast %1, tt
  aesdeclast %2, tt
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

%macro aes_encrypt_2 3 ; B0, B1, key
 aesxor_2     %1, %2, %3, 0
 aesenc_2     %1, %2, %3, 1
 aesenc_2     %1, %2, %3, 2
 aesenc_2     %1, %2, %3, 3
 aesenc_2     %1, %2, %3, 4
 aesenc_2     %1, %2, %3, 5
 aesenc_2     %1, %2, %3, 6
 aesenc_2     %1, %2, %3, 7
 aesenc_2     %1, %2, %3, 8
 aesenc_2     %1, %2, %3, 9
 aesenc_2     %1, %2, %3, 10
 aesenc_2     %1, %2, %3, 11
 aesenc_2     %1, %2, %3, 12
 aesenc_2     %1, %2, %3, 13
 aesenclast_2 %1, %2, %3, 14
%endmacro

%macro aes_decrypt_2 3 ; B0, B1, key
 aesxor_2     %1, %2, %3, 0
 aesdec_2     %1, %2, %3, 1
 aesdec_2     %1, %2, %3, 2
 aesdec_2     %1, %2, %3, 3
 aesdec_2     %1, %2, %3, 4
 aesdec_2     %1, %2, %3, 5
 aesdec_2     %1, %2, %3, 6
 aesdec_2     %1, %2, %3, 7
 aesdec_2     %1, %2, %3, 8
 aesdec_2     %1, %2, %3, 9
 aesdec_2     %1, %2, %3, 10
 aesdec_2     %1, %2, %3, 11
 aesdec_2     %1, %2, %3, 12
 aesdec_2     %1, %2, %3, 13
 aesdeclast_2 %1, %2, %3, 14
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

%macro aes_xts_process 2
 push	       esi
 push	       edi
 ; load XTS tweak polynomial
 mov	       eax, 135
 movd	       POLY, eax
 mov	       eax, [esp+1Ch]	  ;
 shrd	       [esp+18h], eax, 9  ; idx.a = offset / XTS_SECTOR_SIZE
 shr	       eax, 9		  ;
 mov	       [esp+1Ch], eax	  ;
 mov	       esi, [esp+0Ch]	  ; esi = in
 mov	       edi, [esp+10h]	  ; edi = out
 mov	       eax, [esp+20h]	  ; eax = crypt key
 lea	       edx, [eax+tweak_k] ; edx = tweak key
%if %2 != 0
 add	       eax, %2		  ; eax = decryption key
%endif
%%xts_loop:
 add	       dword [esp+18h], 1 ; idx.a++
 adc	       dword [esp+1Ch], 0 ;
 movq	       T0,  [esp+18h]
 aes_encrypt_1 T0, edx
 mov	       ecx, 16 ; ecx = XTS_BLOCKS_IN_SECTOR
%%blocks_loop:
 next_tweak    T1, T0
 ; load two blocks
 movdqu        B0, [esi+00h]
 movdqu        B1, [esi+10h]
 ; input tweak
 pxor	       B0, T0
 pxor	       B1, T1
 ; encrypt / decrypt
 %1	       B0, B1, eax
 ; output tweak
 pxor	       B0, T0
 pxor	       B1, T1
 ; save two blocks
 movdqu        [edi+00h], B0
 movdqu        [edi+10h], B1
 add	       esi, 32 ; in += XTS_BLOCK_SIZE*2
 add	       edi, 32 ; out += XTS_BLOCK_SIZE*2
 dec	       ecx
 jz	       %%block_done
 next_tweak    T0, T1
 jmp	       %%blocks_loop
%%block_done:
 sub	       dword [esp+14h], 512 ; len -= XTS_SECTOR_SIZE
 jnz	       %%xts_loop
 pop	       edi
 pop	       esi
 retn	       18h
%endmacro

; =========================================

%define B0   xmm0
%define B1   xmm1

%define T0   xmm2
%define T1   xmm3

%define tt   xmm4
%define t2   xmm5
%define POLY xmm6

%define tweak_k 11408
%define enc_key 0
%define dec_key 4*15*4

; =========================================

global _xts_aes_ni_encrypt@24
global _xts_aes_ni_decrypt@24


align 16
_xts_aes_ni_encrypt@24:
 aes_xts_process aes_encrypt_2, enc_key

align 16
_xts_aes_ni_decrypt@24:
 aes_xts_process aes_decrypt_2, dec_key







