
; ---------------------------------------------------------------------------
; Copyright (c) 1998-2008, Brian Gladman, Worcester, UK. All rights reserved.
; Copyright (c) 2010, ntldr <ntldr@diskcryptor.net> PGP key ID - 0x1B6A24550F33E44A
;
; LICENSE TERMS
;
; The redistribution and use of this software (with or without changes)
; is allowed without the payment of fees or royalties provided that:
;
;  1. source code distributions include the above copyright notice, this
;     list of conditions and the following disclaimer;
;
;  2. binary distributions include the above copyright notice, this list
;     of conditions and the following disclaimer in their documentation;
;
;  3. the name of the copyright holder is not used to endorse products
;     built using this software without specific written permission.
;
; DISCLAIMER
;
; This software is provided 'as is' with no explicit or implied warranties
; in respect of its properties, including, but not limited to, correctness
; and/or fitness for purpose.
; ---------------------------------------------------------------------------
; offsets to parameters
in_blk	equ	4   ; input byte array address parameter
out_blk equ	8   ; output byte array address parameter
stk_spc equ    16   ; stack space
parms	equ    12   ; parameter space on stack

extern	_Te0, _Te1, _Te2, _Te3, _Te4_0, _Te4_1, _Te4_2, _Te4_3
extern	_Td0, _Td1, _Td2, _Td3, _Td4_0, _Td4_1, _Td4_2, _Td4_3
extern _aes256_set_key@8

global _aes256_asm_set_key@8
global _aes256_asm_encrypt@12
global _aes256_asm_decrypt@12

; ROUND FUNCTION.  Build column[2] on ESI and column[3] on EDI that have the
; round keys pre-loaded. Build column[0] in EBP and column[1] in EBX.
;
; Input:
;
;   EAX     column[0]
;   EBX     column[1]
;   ECX     column[2]
;   EDX     column[3]
;   ESI     column key[round][2]
;   EDI     column key[round][3]
;   EBP     scratch
;
; Output:
;
;   EBP     column[0]   unkeyed
;   EBX     column[1]   unkeyed
;   ESI     column[2]   keyed
;   EDI     column[3]   keyed
;   EAX     scratch
;   ECX     scratch
;   EDX     scratch
%macro rnd_fun 2
    rol     ebx,16
    %1	    esi, cl, 0, ebp
    %1	    esi, dh, 1, ebp
    %1	    esi, bh, 3, ebp
    %1	    edi, dl, 0, ebp
    %1	    edi, ah, 1, ebp
    %1	    edi, bl, 2, ebp
    %2	    ebp, al, 0, ebp
    shr     ebx,16
    and     eax,0xffff0000
    or	    eax,ebx
    shr     edx,16   
    %1	    ebp, ah, 1, ebx
    %1	    ebp, dh, 3, ebx
    %2	    ebx, dl, 2, ebx
    %1	    ebx, ch, 1, edx
    %1	    ebx, al, 0, edx
    shr     eax,16
    shr     ecx,16
    %1	    ebp, cl, 2, edx
    %1	    edi, ch, 3, edx
    %1	    esi, al, 2, edx
    %1	    ebx, ah, 3, edx
%endmacro

; Basic MOV and XOR Operations for normal rounds
%macro	nr_xor	4
    movzx   %4, %2
    xor     %1, [_Te%3+4*%4]
%endmacro

%macro	nr_mov	4
    movzx   %4, %2
    mov     %1, [_Te%3+4*%4]
%endmacro

; Basic MOV and XOR Operations for last round
%macro	lr_xor	4
	movzx	%4, %2
	xor	%1, [_Te4_%3+4*%4]
%endmacro

%macro	lr_mov	4
	movzx	%4, %2
	mov	%1, [_Te4_%3+4*%4]
%endmacro

%macro enc_round 4
    mov     esi, %3
    mov     edi, %4

    rnd_fun nr_xor, nr_mov

    mov     eax, ebp
    mov     ecx, esi
    mov     edx, edi
    xor     eax, %1
    xor     ebx, %2
%endmacro

%macro enc_last_round 4
    mov     esi, %3
    mov     edi, %4

    rnd_fun lr_xor, lr_mov

    mov     eax, ebp
    xor     eax, %1
    xor     ebx, %2
%endmacro

%macro irn_fun 2
    rol     eax,16
    %1	    esi, cl, 0, ebp
    %1	    esi, bh, 1, ebp
    %1	    esi, al, 2, ebp
    %1	    edi, dl, 0, ebp
    %1	    edi, ch, 1, ebp
    %1	    edi, ah, 3, ebp
    %2	    ebp, bl, 0, ebp
    shr     eax,16
    and     ebx,0xffff0000
    or	    ebx,eax
    shr     ecx,16
    %1	    ebp, bh, 1, eax
    %1	    ebp, ch, 3, eax
    %2	    eax, cl, 2, ecx
    %1	    eax, bl, 0, ecx
    %1	    eax, dh, 1, ecx
    shr     ebx,16
    shr     edx,16
    %1	    esi, dh, 3, ecx
    %1	    ebp, dl, 2, ecx
    %1	    eax, bh, 3, ecx
    %1	    edi, bl, 2, ecx
%endmacro

; Basic MOV and XOR Operations for normal rounds
%macro	ni_xor	4
    movzx   %4, %2
    xor     %1, [_Td%3+4*%4]
%endmacro

%macro	ni_mov	4
    movzx   %4, %2
    mov     %1, [_Td%3+4*%4]
%endmacro

; Basic MOV and XOR Operations for last round
%macro	li_xor	4
    movzx   %4, %2
    xor     %1, [_Td4_%3+4*%4]
%endmacro

%macro	li_mov	4
    movzx   %4, %2
    mov     %1, [_Td4_%3+4*%4]
%endmacro

%macro dec_round 4
    mov     esi, %3
    mov     edi, %4

    irn_fun ni_xor, ni_mov

    mov     ebx, ebp
    mov     ecx, esi
    mov     edx, edi
    xor     eax, %1
    xor     ebx, %2
%endmacro

%macro dec_last_round 4
    mov     esi, %3
    mov     edi, %4
    
    irn_fun li_xor, li_mov

    mov     ebx, ebp
    xor     eax, %1
    xor     ebx, %2
%endmacro

%assign i 0
%rep	60
   RK_ %+ i equ (12340000h | i)
%assign i i+1
%endrep


    section .text align=32

; AES Encryption Subroutine
aes256_encrypt_code:
    sub     esp, stk_spc
    mov     [esp+12], ebp
    mov     [esp+ 8], ebx
    mov     [esp+ 4], esi
    mov     [esp+ 0], edi

    mov     esi, [esp+in_blk+stk_spc] ; input pointer
    mov     eax, [esi+ 0]
    mov     ebx, [esi+ 4]
    mov     ecx, [esi+ 8]
    mov     edx, [esi+12]

    xor     eax, RK_0
    xor     ebx, RK_1
    xor     ecx, RK_2
    xor     edx, RK_3

    enc_round	   RK_4, RK_5, RK_6, RK_7
    enc_round	   RK_8, RK_9, RK_10, RK_11
    enc_round	   RK_12, RK_13, RK_14, RK_15
    enc_round	   RK_16, RK_17, RK_18, RK_19
    enc_round	   RK_20, RK_21, RK_22, RK_23
    enc_round	   RK_24, RK_25, RK_26, RK_27
    enc_round	   RK_28, RK_29, RK_30, RK_31
    enc_round	   RK_32, RK_33, RK_34, RK_35
    enc_round	   RK_36, RK_37, RK_38, RK_39
    enc_round	   RK_40, RK_41, RK_42, RK_43	 
    enc_round	   RK_44, RK_45, RK_46, RK_47
    enc_round	   RK_48, RK_49, RK_50, RK_51	 
    enc_round	   RK_52, RK_53, RK_54, RK_55
    enc_last_round RK_56, RK_57, RK_58, RK_59

    mov     edx, [esp+out_blk+stk_spc]
    mov     [edx+ 0], eax
    mov     [edx+ 4], ebx
    mov     [edx+ 8], esi
    mov     [edx+12], edi

    mov     ebp, [esp+12]
    mov     ebx, [esp+ 8]
    mov     esi, [esp+ 4]
    mov     edi, [esp+ 0]
    add     esp, stk_spc
    retn    0Ch
aes256_encrypt_size equ $-aes256_encrypt_code

aes256_decrypt_code:
    sub     esp, stk_spc
    mov     [esp+12],ebp
    mov     [esp+ 8],ebx
    mov     [esp+ 4],esi
    mov     [esp+ 0],edi
    
    ; input four columns and xor in first round key
    mov     esi,[esp+in_blk+stk_spc] ; input pointer
    mov     eax,[esi   ]
    mov     ebx,[esi+ 4]
    mov     ecx,[esi+ 8]
    mov     edx,[esi+12]

    xor     eax, RK_0
    xor     ebx, RK_1
    xor     ecx, RK_2
    xor     edx, RK_3

    dec_round	   RK_4, RK_5, RK_6, RK_7
    dec_round	   RK_8, RK_9, RK_10, RK_11
    dec_round	   RK_12, RK_13, RK_14, RK_15
    dec_round	   RK_16, RK_17, RK_18, RK_19
    dec_round	   RK_20, RK_21, RK_22, RK_23
    dec_round	   RK_24, RK_25, RK_26, RK_27
    dec_round	   RK_28, RK_29, RK_30, RK_31
    dec_round	   RK_32, RK_33, RK_34, RK_35
    dec_round	   RK_36, RK_37, RK_38, RK_39
    dec_round	   RK_40, RK_41, RK_42, RK_43
    dec_round	   RK_44, RK_45, RK_46, RK_47
    dec_round	   RK_48, RK_49, RK_50, RK_51
    dec_round	   RK_52, RK_53, RK_54, RK_55
    dec_last_round RK_56, RK_57, RK_58, RK_59
    
    ; move final values to the output array.
    mov     ebp,[esp+out_blk+stk_spc]
    mov     [ebp],eax
    mov     [ebp+4],ebx
    mov     [ebp+8],esi
    mov     [ebp+12],edi
    
    mov     ebp,[esp+12]
    mov     ebx,[esp+ 8]
    mov     esi,[esp+ 4]
    mov     edi,[esp+ 0]
    add     esp,stk_spc
    retn    0Ch
aes256_decrypt_size equ $-aes256_decrypt_code

align 32
_aes256_asm_encrypt@12:
    mov     eax, [esp+12] ; key
    add     eax, 480	  ; ek_code
    jmp     eax

align 32
_aes256_asm_decrypt@12:
    mov     eax, [esp+12] ; key
    add     eax, 3552	  ; dk_code
    jmp     eax
    
aes256_patch_code: ; ebp - round keys, ebx - code buff, ecx - code size
    pushad
    sub     ecx, 4
patch_loop:
    mov     eax, [ebx]
    mov     edx, eax
    shr     edx, 16
    cmp     edx, 1234h
    jnz     no_patch
    movzx   edx, ax
    mov     eax, [ebp+edx*4]	
    mov     [ebx], eax
no_patch:
    inc     ebx
    loop    patch_loop
    popad
    retn

_aes256_asm_set_key@8:
    pushad
    mov     ebp, [esp+28h] ; skey
    mov     eax, [esp+24h] ; key
    push    ebp
    push    eax
    call    _aes256_set_key@8
    lea     ebx, [ebp+480] ; ek_code
    mov     esi, aes256_encrypt_code
    mov     edi, ebx
    mov     ecx, aes256_encrypt_size
    push    ecx
    rep movsb
    pop     ecx
    call    aes256_patch_code
    lea     ebx, [ebp+3552] ; dk_code
    add     ebp, 240	    ; dec_key
    mov     esi, aes256_decrypt_code
    mov     edi, ebx
    mov     ecx, aes256_decrypt_size
    push    ecx
    rep movsb
    pop     ecx
    call    aes256_patch_code
    popad
    retn    08h

