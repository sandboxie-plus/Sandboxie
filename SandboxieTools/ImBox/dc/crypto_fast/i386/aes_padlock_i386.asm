;
;   *
;   * Copyright (c) 2009-2010
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

%define NEH_LOAD_KEY 00000080h	    ; load schedule from memory
%define NEH_ENCRYPT  00000000h	    ; encryption
%define NEH_DECRYPT  00000200h	    ; decryption
%define NEH_KEY128   00000000h+0ah  ; 128 bit key
%define NEH_KEY192   00000400h+0ch  ; 192 bit key
%define NEH_KEY256   00000800h+0eh  ; 256 bit key
%define NEH_ENC_LOAD (NEH_ENCRYPT | NEH_LOAD_KEY)
%define NEH_DEC_LOAD (NEH_DECRYPT | NEH_LOAD_KEY)

align 16
enc_cwd dd (NEH_ENC_LOAD | NEH_KEY256), 0, 0
align 16
dec_cwd dd (NEH_DEC_LOAD | NEH_KEY256), 0, 0

global _aes256_padlock_available@0
global _aes256_padlock_encrypt@16
global _aes256_padlock_decrypt@16

_aes256_padlock_available@0:
 push	 ebx
 ; test for VIA CPU
 mov	 eax, 0C0000000h
 cpuid
 cmp	 eax, 0C0000001h
 jb	 no_ace
 ; read VIA flags
 mov	 eax, 0C0000001h
 cpuid
 and	 edx, 0C0h ; ACE_MASK,CPUID EDX code for ACE
 cmp	 edx, 0C0h ; ACE_MASK,CPUID EDX code for ACE
 jnz	 no_ace
 ; ACE present
 xor	 eax, eax
 inc	 eax
 jmp	 end_ace
no_ace:
 xor	 eax, eax
end_ace:
 pop	 ebx
 ret

align 16
_aes256_padlock_encrypt@16:
 push	 ebx
 push	 esi
 push	 edi
 mov	 esi, [esp+10h] ; in
 mov	 edi, [esp+14h] ; out
 mov	 ecx, [esp+18h] ; n_blocks
 mov	 ebx, [esp+1Ch] ; key
 mov	 edx, enc_cwd
 xcryptecb
 pop	 edi
 pop	 esi
 pop	 ebx
 retn	 10h

align 16
_aes256_padlock_decrypt@16
 push	 ebx
 push	 esi
 push	 edi
 mov	 esi, [esp+10h] ; in
 mov	 edi, [esp+14h] ; out
 mov	 ecx, [esp+18h] ; n_blocks
 mov	 ebx, [esp+1Ch] ; key
 add	 ebx, 4*15*4
 mov	 edx, dec_cwd
 xcryptecb
 pop	 edi
 pop	 esi
 pop	 ebx
 retn	 10h


