
; ---------------------------------------------------------------------------
; Copyright (c) 1998-2007, Brian Gladman, Worcester, UK. All rights reserved.
; 
; LICENSE TERMS
; 
; The free distribution and use of this software is allowed (with or without
; changes) provided that:
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
; Issue 20/12/2007
;
; I am grateful to Dag Arne Osvik for many discussions of the techniques that
; can be used to optimise AES assembler code on AMD64/EM64T architectures.
; Some of the techniques used in this implementation are the result of
; suggestions made by him for which I am most grateful.

; An AES implementation for AMD64 processors using the YASM assembler.  This
; implementation provides only encryption, decryption and hence requires key
; scheduling support in C. It uses 8k bytes of tables but its encryption and
; decryption performance is very close to that obtained using large tables.
; It can use either Windows or Gnu/Linux calling conventions, which are as
; follows:
;               windows  gnu/linux
;
;   in_blk          rcx     rdi
;   out_blk         rdx     rsi
;   context (cx)     r8     rdx
;
;   preserved       rsi      -    + rbx, rbp, rsp, r12, r13, r14 & r15
;   registers       rdi      -      on both
;
;   destroyed        -      rsi   + rax, rcx, rdx, r8, r9, r10 & r11
;   registers        -      rdi     on both
;
; The default convention is that for windows, the gnu/linux convention being
; used if __GNUC__ is defined.
;
; Define _SEH_ to include support for Win64 structured exception handling
; (this requires YASM version 0.6 or later).
;
; This code provides the standard AES block size (128 bits, 16 bytes) and the
; three standard AES key sizes (128, 192 and 256 bits). It has the same call
; interface as my C implementation.  It uses the Microsoft C AMD64 calling
; conventions in which the three parameters are placed in  rcx, rdx and r8
; respectively.  The rbx, rsi, rdi, rbp and r12..r15 registers are preserved.
;
;     AES_RETURN aes_encrypt(const unsigned char in_blk[],
;                   unsigned char out_blk[], const aes_encrypt_ctx cx[1]);
;
;     AES_RETURN aes_decrypt(const unsigned char in_blk[],
;                   unsigned char out_blk[], const aes_decrypt_ctx cx[1]);
;
;     AES_RETURN aes_encrypt_key<NNN>(const unsigned char key[],
;                                            const aes_encrypt_ctx cx[1]);
;
;     AES_RETURN aes_decrypt_key<NNN>(const unsigned char key[],
;                                            const aes_decrypt_ctx cx[1]);
;
;     AES_RETURN aes_encrypt_key(const unsigned char key[],
;                           unsigned int len, const aes_decrypt_ctx cx[1]);
;
;     AES_RETURN aes_decrypt_key(const unsigned char key[],
;                           unsigned int len, const aes_decrypt_ctx cx[1]);
;
; where <NNN> is 128, 102 or 256.  In the last two calls the length can be in
; either bits or bytes.
;
; Comment in/out the following lines to obtain the desired subroutines. These
; selections MUST match those in the C header file aes.h

;%define AES_128                 ; define if AES with 128 bit keys is needed
;%define AES_192                 ; define if AES with 192 bit keys is needed
%define AES_256                 ; define if AES with 256 bit keys is needed
;%define AES_VAR                 ; define if a variable key size is needed
%define ENCRYPTION              ; define if encryption is needed
%define DECRYPTION              ; define if decryption is needed
%define AES_REV_DKS             ; define if key decryption schedule is reversed

%define LAST_ROUND_TABLES       ; define for the faster version using extra tables

; The encryption key schedule has the following in memory layout where N is the
; number of rounds (10, 12 or 14):
;
; lo: | input key (round 0)  |  ; each round is four 32-bit words
;     | encryption round 1   |
;     | encryption round 2   |
;     ....
;     | encryption round N-1 |
; hi: | encryption round N   |
;
; The decryption key schedule is normally set up so that it has the same
; layout as above by actually reversing the order of the encryption key
; schedule in memory (this happens when AES_REV_DKS is set):
;
; lo: | decryption round 0   | =              | encryption round N   |
;     | decryption round 1   | = INV_MIX_COL[ | encryption round N-1 | ]
;     | decryption round 2   | = INV_MIX_COL[ | encryption round N-2 | ]
;     ....                       ....
;     | decryption round N-1 | = INV_MIX_COL[ | encryption round 1   | ]
; hi: | decryption round N   | =              | input key (round 0)  |
;
; with rounds except the first and last modified using inv_mix_column()
; But if AES_REV_DKS is NOT set the order of keys is left as it is for
; encryption so that it has to be accessed in reverse when used for
; decryption (although the inverse mix column modifications are done)
;
; lo: | decryption round 0   | =              | input key (round 0)  |
;     | decryption round 1   | = INV_MIX_COL[ | encryption round 1   | ]
;     | decryption round 2   | = INV_MIX_COL[ | encryption round 2   | ]
;     ....                       ....
;     | decryption round N-1 | = INV_MIX_COL[ | encryption round N-1 | ]
; hi: | decryption round N   | =              | encryption round N   |
;
; This layout is faster when the assembler key scheduling provided here
; is used.
;
; The DLL interface must use the _stdcall convention in which the number
; of bytes of parameter space is added after an @ to the sutine's name.
; We must also remove our parameters from the stack before return (see
; the do_exit macro). Define DLL_EXPORT for the Dynamic Link Library version.

;%define DLL_EXPORT

; End of user defines

%ifdef AES_VAR
%ifndef AES_128
%define AES_128
%endif
%ifndef AES_192
%define AES_192
%endif
%ifndef AES_256
%define AES_256
%endif
%endif

%ifdef AES_VAR
%define KS_LENGTH       60
%elifdef AES_256
%define KS_LENGTH       60
%elifdef AES_192
%define KS_LENGTH       52
%else
%define KS_LENGTH       44
%endif

%define     r0  rax
%define     r1  rdx
%define     r2  rcx
%define     r3  rbx
%define     r4  rsi
%define     r5  rdi
%define     r6  rbp
%define     r7  rsp

%define     raxd    eax
%define     rdxd    edx
%define     rcxd    ecx
%define     rbxd    ebx
%define     rsid    esi
%define     rdid    edi
%define     rbpd    ebp
%define     rspd    esp

%define     raxb    al
%define     rdxb    dl
%define     rcxb    cl
%define     rbxb    bl
%define     rsib    sil
%define     rdib    dil
%define     rbpb    bpl
%define     rspb    spl

%define     r0h ah
%define     r1h dh
%define     r2h ch
%define     r3h bh

%define     r0d eax
%define     r1d edx
%define     r2d ecx
%define     r3d ebx

; finite field multiplies by {02}, {04} and {08}

%define f2(x)   ((x<<1)^(((x>>7)&1)*0x11b))
%define f4(x)   ((x<<2)^(((x>>6)&1)*0x11b)^(((x>>6)&2)*0x11b))
%define f8(x)   ((x<<3)^(((x>>5)&1)*0x11b)^(((x>>5)&2)*0x11b)^(((x>>5)&4)*0x11b))

; finite field multiplies required in table generation

%define f3(x)   (f2(x) ^ x)
%define f9(x)   (f8(x) ^ x)
%define fb(x)   (f8(x) ^ f2(x) ^ x)
%define fd(x)   (f8(x) ^ f4(x) ^ x)
%define fe(x)   (f8(x) ^ f4(x) ^ f2(x))

; macro for expanding S-box data

%macro enc_vals 1
    db  %1(0x63),%1(0x7c),%1(0x77),%1(0x7b),%1(0xf2),%1(0x6b),%1(0x6f),%1(0xc5)
    db  %1(0x30),%1(0x01),%1(0x67),%1(0x2b),%1(0xfe),%1(0xd7),%1(0xab),%1(0x76)
    db  %1(0xca),%1(0x82),%1(0xc9),%1(0x7d),%1(0xfa),%1(0x59),%1(0x47),%1(0xf0)
    db  %1(0xad),%1(0xd4),%1(0xa2),%1(0xaf),%1(0x9c),%1(0xa4),%1(0x72),%1(0xc0)
    db  %1(0xb7),%1(0xfd),%1(0x93),%1(0x26),%1(0x36),%1(0x3f),%1(0xf7),%1(0xcc)
    db  %1(0x34),%1(0xa5),%1(0xe5),%1(0xf1),%1(0x71),%1(0xd8),%1(0x31),%1(0x15)
    db  %1(0x04),%1(0xc7),%1(0x23),%1(0xc3),%1(0x18),%1(0x96),%1(0x05),%1(0x9a)
    db  %1(0x07),%1(0x12),%1(0x80),%1(0xe2),%1(0xeb),%1(0x27),%1(0xb2),%1(0x75)
    db  %1(0x09),%1(0x83),%1(0x2c),%1(0x1a),%1(0x1b),%1(0x6e),%1(0x5a),%1(0xa0)
    db  %1(0x52),%1(0x3b),%1(0xd6),%1(0xb3),%1(0x29),%1(0xe3),%1(0x2f),%1(0x84)
    db  %1(0x53),%1(0xd1),%1(0x00),%1(0xed),%1(0x20),%1(0xfc),%1(0xb1),%1(0x5b)
    db  %1(0x6a),%1(0xcb),%1(0xbe),%1(0x39),%1(0x4a),%1(0x4c),%1(0x58),%1(0xcf)
    db  %1(0xd0),%1(0xef),%1(0xaa),%1(0xfb),%1(0x43),%1(0x4d),%1(0x33),%1(0x85)
    db  %1(0x45),%1(0xf9),%1(0x02),%1(0x7f),%1(0x50),%1(0x3c),%1(0x9f),%1(0xa8)
    db  %1(0x51),%1(0xa3),%1(0x40),%1(0x8f),%1(0x92),%1(0x9d),%1(0x38),%1(0xf5)
    db  %1(0xbc),%1(0xb6),%1(0xda),%1(0x21),%1(0x10),%1(0xff),%1(0xf3),%1(0xd2)
    db  %1(0xcd),%1(0x0c),%1(0x13),%1(0xec),%1(0x5f),%1(0x97),%1(0x44),%1(0x17)
    db  %1(0xc4),%1(0xa7),%1(0x7e),%1(0x3d),%1(0x64),%1(0x5d),%1(0x19),%1(0x73)
    db  %1(0x60),%1(0x81),%1(0x4f),%1(0xdc),%1(0x22),%1(0x2a),%1(0x90),%1(0x88)
    db  %1(0x46),%1(0xee),%1(0xb8),%1(0x14),%1(0xde),%1(0x5e),%1(0x0b),%1(0xdb)
    db  %1(0xe0),%1(0x32),%1(0x3a),%1(0x0a),%1(0x49),%1(0x06),%1(0x24),%1(0x5c)
    db  %1(0xc2),%1(0xd3),%1(0xac),%1(0x62),%1(0x91),%1(0x95),%1(0xe4),%1(0x79)
    db  %1(0xe7),%1(0xc8),%1(0x37),%1(0x6d),%1(0x8d),%1(0xd5),%1(0x4e),%1(0xa9)
    db  %1(0x6c),%1(0x56),%1(0xf4),%1(0xea),%1(0x65),%1(0x7a),%1(0xae),%1(0x08)
    db  %1(0xba),%1(0x78),%1(0x25),%1(0x2e),%1(0x1c),%1(0xa6),%1(0xb4),%1(0xc6)
    db  %1(0xe8),%1(0xdd),%1(0x74),%1(0x1f),%1(0x4b),%1(0xbd),%1(0x8b),%1(0x8a)
    db  %1(0x70),%1(0x3e),%1(0xb5),%1(0x66),%1(0x48),%1(0x03),%1(0xf6),%1(0x0e)
    db  %1(0x61),%1(0x35),%1(0x57),%1(0xb9),%1(0x86),%1(0xc1),%1(0x1d),%1(0x9e)
    db  %1(0xe1),%1(0xf8),%1(0x98),%1(0x11),%1(0x69),%1(0xd9),%1(0x8e),%1(0x94)
    db  %1(0x9b),%1(0x1e),%1(0x87),%1(0xe9),%1(0xce),%1(0x55),%1(0x28),%1(0xdf)
    db  %1(0x8c),%1(0xa1),%1(0x89),%1(0x0d),%1(0xbf),%1(0xe6),%1(0x42),%1(0x68)
    db  %1(0x41),%1(0x99),%1(0x2d),%1(0x0f),%1(0xb0),%1(0x54),%1(0xbb),%1(0x16)
%endmacro

%macro dec_vals 1
    db  %1(0x52),%1(0x09),%1(0x6a),%1(0xd5),%1(0x30),%1(0x36),%1(0xa5),%1(0x38)
    db  %1(0xbf),%1(0x40),%1(0xa3),%1(0x9e),%1(0x81),%1(0xf3),%1(0xd7),%1(0xfb)
    db  %1(0x7c),%1(0xe3),%1(0x39),%1(0x82),%1(0x9b),%1(0x2f),%1(0xff),%1(0x87)
    db  %1(0x34),%1(0x8e),%1(0x43),%1(0x44),%1(0xc4),%1(0xde),%1(0xe9),%1(0xcb)
    db  %1(0x54),%1(0x7b),%1(0x94),%1(0x32),%1(0xa6),%1(0xc2),%1(0x23),%1(0x3d)
    db  %1(0xee),%1(0x4c),%1(0x95),%1(0x0b),%1(0x42),%1(0xfa),%1(0xc3),%1(0x4e)
    db  %1(0x08),%1(0x2e),%1(0xa1),%1(0x66),%1(0x28),%1(0xd9),%1(0x24),%1(0xb2)
    db  %1(0x76),%1(0x5b),%1(0xa2),%1(0x49),%1(0x6d),%1(0x8b),%1(0xd1),%1(0x25)
    db  %1(0x72),%1(0xf8),%1(0xf6),%1(0x64),%1(0x86),%1(0x68),%1(0x98),%1(0x16)
    db  %1(0xd4),%1(0xa4),%1(0x5c),%1(0xcc),%1(0x5d),%1(0x65),%1(0xb6),%1(0x92)
    db  %1(0x6c),%1(0x70),%1(0x48),%1(0x50),%1(0xfd),%1(0xed),%1(0xb9),%1(0xda)
    db  %1(0x5e),%1(0x15),%1(0x46),%1(0x57),%1(0xa7),%1(0x8d),%1(0x9d),%1(0x84)
    db  %1(0x90),%1(0xd8),%1(0xab),%1(0x00),%1(0x8c),%1(0xbc),%1(0xd3),%1(0x0a)
    db  %1(0xf7),%1(0xe4),%1(0x58),%1(0x05),%1(0xb8),%1(0xb3),%1(0x45),%1(0x06)
    db  %1(0xd0),%1(0x2c),%1(0x1e),%1(0x8f),%1(0xca),%1(0x3f),%1(0x0f),%1(0x02)
    db  %1(0xc1),%1(0xaf),%1(0xbd),%1(0x03),%1(0x01),%1(0x13),%1(0x8a),%1(0x6b)
    db  %1(0x3a),%1(0x91),%1(0x11),%1(0x41),%1(0x4f),%1(0x67),%1(0xdc),%1(0xea)
    db  %1(0x97),%1(0xf2),%1(0xcf),%1(0xce),%1(0xf0),%1(0xb4),%1(0xe6),%1(0x73)
    db  %1(0x96),%1(0xac),%1(0x74),%1(0x22),%1(0xe7),%1(0xad),%1(0x35),%1(0x85)
    db  %1(0xe2),%1(0xf9),%1(0x37),%1(0xe8),%1(0x1c),%1(0x75),%1(0xdf),%1(0x6e)
    db  %1(0x47),%1(0xf1),%1(0x1a),%1(0x71),%1(0x1d),%1(0x29),%1(0xc5),%1(0x89)
    db  %1(0x6f),%1(0xb7),%1(0x62),%1(0x0e),%1(0xaa),%1(0x18),%1(0xbe),%1(0x1b)
    db  %1(0xfc),%1(0x56),%1(0x3e),%1(0x4b),%1(0xc6),%1(0xd2),%1(0x79),%1(0x20)
    db  %1(0x9a),%1(0xdb),%1(0xc0),%1(0xfe),%1(0x78),%1(0xcd),%1(0x5a),%1(0xf4)
    db  %1(0x1f),%1(0xdd),%1(0xa8),%1(0x33),%1(0x88),%1(0x07),%1(0xc7),%1(0x31)
    db  %1(0xb1),%1(0x12),%1(0x10),%1(0x59),%1(0x27),%1(0x80),%1(0xec),%1(0x5f)
    db  %1(0x60),%1(0x51),%1(0x7f),%1(0xa9),%1(0x19),%1(0xb5),%1(0x4a),%1(0x0d)
    db  %1(0x2d),%1(0xe5),%1(0x7a),%1(0x9f),%1(0x93),%1(0xc9),%1(0x9c),%1(0xef)
    db  %1(0xa0),%1(0xe0),%1(0x3b),%1(0x4d),%1(0xae),%1(0x2a),%1(0xf5),%1(0xb0)
    db  %1(0xc8),%1(0xeb),%1(0xbb),%1(0x3c),%1(0x83),%1(0x53),%1(0x99),%1(0x61)
    db  %1(0x17),%1(0x2b),%1(0x04),%1(0x7e),%1(0xba),%1(0x77),%1(0xd6),%1(0x26)
    db  %1(0xe1),%1(0x69),%1(0x14),%1(0x63),%1(0x55),%1(0x21),%1(0x0c),%1(0x7d)
%endmacro

%define u8(x)   f2(x), x, x, f3(x), f2(x), x, x, f3(x)
%define v8(x)   fe(x), f9(x), fd(x), fb(x), fe(x), f9(x), fd(x), x
%define w8(x)   x, 0, 0, 0, x, 0, 0, 0

%define tptr    rbp     ; table pointer
%define kptr    r8      ; key schedule pointer
%define fofs    128     ; adjust offset in key schedule to keep |disp| < 128
%define fk_ref(x,y) [kptr-16*x+fofs+4*y]
%ifdef  AES_REV_DKS
%define rofs    128
%define ik_ref(x,y) [kptr-16*x+rofs+4*y]
%else
%define rofs    -128
%define ik_ref(x,y) [kptr+16*x+rofs+4*y]
%endif

%define tab_0(x)   [tptr+8*x]
%define tab_1(x)   [tptr+8*x+3]
%define tab_2(x)   [tptr+8*x+2]
%define tab_3(x)   [tptr+8*x+1]
%define tab_f(x)   byte [tptr+8*x+1]
%define tab_i(x)   byte [tptr+8*x+7]
%define t_ref(x,r) tab_ %+ x(r)

%macro ff_rnd 5                 ; normal forward round
    mov     %1d, fk_ref(%5,0)
    mov     %2d, fk_ref(%5,1)
    mov     %3d, fk_ref(%5,2)
    mov     %4d, fk_ref(%5,3)

    movzx   esi, al
    movzx   edi, ah
    shr     eax, 16
    xor     %1d, t_ref(0,rsi)
    xor     %4d, t_ref(1,rdi)
    movzx   esi, al
    movzx   edi, ah
    xor     %3d, t_ref(2,rsi)
    xor     %2d, t_ref(3,rdi)

    movzx   esi, bl
    movzx   edi, bh
    shr     ebx, 16
    xor     %2d, t_ref(0,rsi)
    xor     %1d, t_ref(1,rdi)
    movzx   esi, bl
    movzx   edi, bh
    xor     %4d, t_ref(2,rsi)
    xor     %3d, t_ref(3,rdi)

    movzx   esi, cl
    movzx   edi, ch
    shr     ecx, 16
    xor     %3d, t_ref(0,rsi)
    xor     %2d, t_ref(1,rdi)
    movzx   esi, cl
    movzx   edi, ch
    xor     %1d, t_ref(2,rsi)
    xor     %4d, t_ref(3,rdi)

    movzx   esi, dl
    movzx   edi, dh
    shr     edx, 16
    xor     %4d, t_ref(0,rsi)
    xor     %3d, t_ref(1,rdi)
    movzx   esi, dl
    movzx   edi, dh
    xor     %2d, t_ref(2,rsi)
    xor     %1d, t_ref(3,rdi)

    mov     eax,%1d
    mov     ebx,%2d
    mov     ecx,%3d
    mov     edx,%4d
%endmacro

%ifdef LAST_ROUND_TABLES

%macro fl_rnd 5                 ; last forward round
    add     tptr, 2048
    mov     %1d, fk_ref(%5,0)
    mov     %2d, fk_ref(%5,1)
    mov     %3d, fk_ref(%5,2)
    mov     %4d, fk_ref(%5,3)

    movzx   esi, al
    movzx   edi, ah
    shr     eax, 16
    xor     %1d, t_ref(0,rsi)
    xor     %4d, t_ref(1,rdi)
    movzx   esi, al
    movzx   edi, ah
    xor     %3d, t_ref(2,rsi)
    xor     %2d, t_ref(3,rdi)

    movzx   esi, bl
    movzx   edi, bh
    shr     ebx, 16
    xor     %2d, t_ref(0,rsi)
    xor     %1d, t_ref(1,rdi)
    movzx   esi, bl
    movzx   edi, bh
    xor     %4d, t_ref(2,rsi)
    xor     %3d, t_ref(3,rdi)

    movzx   esi, cl
    movzx   edi, ch
    shr     ecx, 16
    xor     %3d, t_ref(0,rsi)
    xor     %2d, t_ref(1,rdi)
    movzx   esi, cl
    movzx   edi, ch
    xor     %1d, t_ref(2,rsi)
    xor     %4d, t_ref(3,rdi)

    movzx   esi, dl
    movzx   edi, dh
    shr     edx, 16
    xor     %4d, t_ref(0,rsi)
    xor     %3d, t_ref(1,rdi)
    movzx   esi, dl
    movzx   edi, dh
    xor     %2d, t_ref(2,rsi)
    xor     %1d, t_ref(3,rdi)
%endmacro

%else

%macro fl_rnd 5                 ; last forward round
    mov     %1d, fk_ref(%5,0)
    mov     %2d, fk_ref(%5,1)
    mov     %3d, fk_ref(%5,2)
    mov     %4d, fk_ref(%5,3)

    movzx   esi, al
    movzx   edi, ah
    shr     eax, 16
    movzx   esi, t_ref(f,rsi)
    movzx   edi, t_ref(f,rdi)
    xor     %1d, esi
    rol     edi, 8
    xor     %4d, edi
    movzx   esi, al
    movzx   edi, ah
    movzx   esi, t_ref(f,rsi)
    movzx   edi, t_ref(f,rdi)
    rol     esi, 16
    rol     edi, 24
    xor     %3d, esi
    xor     %2d, edi

    movzx   esi, bl
    movzx   edi, bh
    shr     ebx, 16
    movzx   esi, t_ref(f,rsi)
    movzx   edi, t_ref(f,rdi)
    xor     %2d, esi
    rol     edi, 8
    xor     %1d, edi
    movzx   esi, bl
    movzx   edi, bh
    movzx   esi, t_ref(f,rsi)
    movzx   edi, t_ref(f,rdi)
    rol     esi, 16
    rol     edi, 24
    xor     %4d, esi
    xor     %3d, edi

    movzx   esi, cl
    movzx   edi, ch
    movzx   esi, t_ref(f,rsi)
    movzx   edi, t_ref(f,rdi)
    shr     ecx, 16
    xor     %3d, esi
    rol     edi, 8
    xor     %2d, edi
    movzx   esi, cl
    movzx   edi, ch
    movzx   esi, t_ref(f,rsi)
    movzx   edi, t_ref(f,rdi)
    rol     esi, 16
    rol     edi, 24
    xor     %1d, esi
    xor     %4d, edi

    movzx   esi, dl
    movzx   edi, dh
    movzx   esi, t_ref(f,rsi)
    movzx   edi, t_ref(f,rdi)
    shr     edx, 16
    xor     %4d, esi
    rol     edi, 8
    xor     %3d, edi
    movzx   esi, dl
    movzx   edi, dh
    movzx   esi, t_ref(f,rsi)
    movzx   edi, t_ref(f,rdi)
    rol     esi, 16
    rol     edi, 24
    xor     %2d, esi
    xor     %1d, edi
%endmacro

%endif

%macro ii_rnd 5                 ; normal inverse round
    mov     %1d, ik_ref(%5,0)
    mov     %2d, ik_ref(%5,1)
    mov     %3d, ik_ref(%5,2)
    mov     %4d, ik_ref(%5,3)

    movzx   esi, al
    movzx   edi, ah
    shr     eax, 16
    xor     %1d, t_ref(0,rsi)
    xor     %2d, t_ref(1,rdi)
    movzx   esi, al
    movzx   edi, ah
    xor     %3d, t_ref(2,rsi)
    xor     %4d, t_ref(3,rdi)

    movzx   esi, bl
    movzx   edi, bh
    shr     ebx, 16
    xor     %2d, t_ref(0,rsi)
    xor     %3d, t_ref(1,rdi)
    movzx   esi, bl
    movzx   edi, bh
    xor     %4d, t_ref(2,rsi)
    xor     %1d, t_ref(3,rdi)

    movzx   esi, cl
    movzx   edi, ch
    shr     ecx, 16
    xor     %3d, t_ref(0,rsi)
    xor     %4d, t_ref(1,rdi)
    movzx   esi, cl
    movzx   edi, ch
    xor     %1d, t_ref(2,rsi)
    xor     %2d, t_ref(3,rdi)

    movzx   esi, dl
    movzx   edi, dh
    shr     edx, 16
    xor     %4d, t_ref(0,rsi)
    xor     %1d, t_ref(1,rdi)
    movzx   esi, dl
    movzx   edi, dh
    xor     %2d, t_ref(2,rsi)
    xor     %3d, t_ref(3,rdi)

    mov     eax,%1d
    mov     ebx,%2d
    mov     ecx,%3d
    mov     edx,%4d
%endmacro

%ifdef LAST_ROUND_TABLES

%macro il_rnd 5                 ; last inverse round
    add     tptr, 2048
    mov     %1d, ik_ref(%5,0)
    mov     %2d, ik_ref(%5,1)
    mov     %3d, ik_ref(%5,2)
    mov     %4d, ik_ref(%5,3)

    movzx   esi, al
    movzx   edi, ah
    shr     eax, 16
    xor     %1d, t_ref(0,rsi)
    xor     %2d, t_ref(1,rdi)
    movzx   esi, al
    movzx   edi, ah
    xor     %3d, t_ref(2,rsi)
    xor     %4d, t_ref(3,rdi)

    movzx   esi, bl
    movzx   edi, bh
    shr     ebx, 16
    xor     %2d, t_ref(0,rsi)
    xor     %3d, t_ref(1,rdi)
    movzx   esi, bl
    movzx   edi, bh
    xor     %4d, t_ref(2,rsi)
    xor     %1d, t_ref(3,rdi)

    movzx   esi, cl
    movzx   edi, ch
    shr     ecx, 16
    xor     %3d, t_ref(0,rsi)
    xor     %4d, t_ref(1,rdi)
    movzx   esi, cl
    movzx   edi, ch
    xor     %1d, t_ref(2,rsi)
    xor     %2d, t_ref(3,rdi)

    movzx   esi, dl
    movzx   edi, dh
    shr     edx, 16
    xor     %4d, t_ref(0,rsi)
    xor     %1d, t_ref(1,rdi)
    movzx   esi, dl
    movzx   edi, dh
    xor     %2d, t_ref(2,rsi)
    xor     %3d, t_ref(3,rdi)
%endmacro

%else

%macro il_rnd 5                 ; last inverse round
    mov     %1d, ik_ref(%5,0)
    mov     %2d, ik_ref(%5,1)
    mov     %3d, ik_ref(%5,2)
    mov     %4d, ik_ref(%5,3)

    movzx   esi, al
    movzx   edi, ah
    movzx   esi, t_ref(i,rsi)
    movzx   edi, t_ref(i,rdi)
    shr     eax, 16
    xor     %1d, esi
    rol     edi, 8
    xor     %2d, edi
    movzx   esi, al
    movzx   edi, ah
    movzx   esi, t_ref(i,rsi)
    movzx   edi, t_ref(i,rdi)
    rol     esi, 16
    rol     edi, 24
    xor     %3d, esi
    xor     %4d, edi

    movzx   esi, bl
    movzx   edi, bh
    movzx   esi, t_ref(i,rsi)
    movzx   edi, t_ref(i,rdi)
    shr     ebx, 16
    xor     %2d, esi
    rol     edi, 8
    xor     %3d, edi
    movzx   esi, bl
    movzx   edi, bh
    movzx   esi, t_ref(i,rsi)
    movzx   edi, t_ref(i,rdi)
    rol     esi, 16
    rol     edi, 24
    xor     %4d, esi
    xor     %1d, edi

    movzx   esi, cl
    movzx   edi, ch
    movzx   esi, t_ref(i,rsi)
    movzx   edi, t_ref(i,rdi)
    shr     ecx, 16
    xor     %3d, esi
    rol     edi, 8
    xor     %4d, edi
    movzx   esi, cl
    movzx   edi, ch
    movzx   esi, t_ref(i,rsi)
    movzx   edi, t_ref(i,rdi)
    rol     esi, 16
    rol     edi, 24
    xor     %1d, esi
    xor     %2d, edi

    movzx   esi, dl
    movzx   edi, dh
    movzx   esi, t_ref(i,rsi)
    movzx   edi, t_ref(i,rdi)
    shr     edx, 16
    xor     %4d, esi
    rol     edi, 8
    xor     %1d, edi
    movzx   esi, dl
    movzx   edi, dh
    movzx   esi, t_ref(i,rsi)
    movzx   edi, t_ref(i,rdi)
    rol     esi, 16
    rol     edi, 24
    xor     %2d, esi
    xor     %3d, edi
%endmacro

%endif

%ifdef ENCRYPTION

    global  aes256_asm_encrypt
%ifdef DLL_EXPORT
    export  aes256_asm_encrypt
%endif

    section .data align=64
    align   64
enc_tab:
    enc_vals u8
%ifdef LAST_ROUND_TABLES
    enc_vals w8
%endif

    section .text align=16
    align   16

%ifdef _SEH_
proc_frame aes256_asm_encrypt
	alloc_stack	7*8			; 7 to align stack to 16 bytes
	save_reg	rsi,4*8
	save_reg	rdi,5*8
	save_reg	rbx,1*8
	save_reg	rbp,2*8
	save_reg	r12,3*8
end_prologue
    mov     rdi, rcx        ; input pointer
    mov     [rsp+0*8], rdx  ; output pointer
%else
	aes256_asm_encrypt:
	%ifdef __GNUC__
		sub     rsp, 4*8        ; gnu/linux binary interface
		mov     [rsp+0*8], rsi  ; output pointer
		mov     r8, rdx         ; context
	%else
		sub     rsp, 6*8        ; windows binary interface
		mov     [rsp+4*8], rsi
		mov     [rsp+5*8], rdi
		mov     rdi, rcx        ; input pointer
		mov     [rsp+0*8], rdx  ; output pointer
	%endif
		mov     [rsp+1*8], rbx  ; input pointer in rdi
		mov     [rsp+2*8], rbp  ; output pointer in [rsp]
		mov     [rsp+3*8], r12  ; context in r8
%endif
    lea     tptr,[enc_tab wrt rip]
    sub     kptr, fofs

    mov     eax, [rdi+0*4]
    mov     ebx, [rdi+1*4]
    mov     ecx, [rdi+2*4]
    mov     edx, [rdi+3*4]

    xor     eax, [kptr+fofs]
    xor     ebx, [kptr+fofs+4]
    xor     ecx, [kptr+fofs+8]
    xor     edx, [kptr+fofs+12]

    add     kptr, 14*16

    ff_rnd  r9, r10, r11, r12, 13
    ff_rnd  r9, r10, r11, r12, 12
    ff_rnd  r9, r10, r11, r12, 11
    ff_rnd  r9, r10, r11, r12, 10
    ff_rnd  r9, r10, r11, r12, 9
    ff_rnd  r9, r10, r11, r12, 8
    ff_rnd  r9, r10, r11, r12, 7
    ff_rnd  r9, r10, r11, r12, 6
    ff_rnd  r9, r10, r11, r12, 5
    ff_rnd  r9, r10, r11, r12, 4
    ff_rnd  r9, r10, r11, r12, 3
    ff_rnd  r9, r10, r11, r12, 2
    ff_rnd  r9, r10, r11, r12, 1
    fl_rnd  r9, r10, r11, r12, 0

    mov     rbx, [rsp]
    mov     [rbx], r9d
    mov     [rbx+4], r10d
    mov     [rbx+8], r11d
    mov     [rbx+12], r12d
    xor     rax, rax

    mov     rbx, [rsp+1*8]
    mov     rbp, [rsp+2*8]
    mov     r12, [rsp+3*8]
%ifdef __GNUC__
    add     rsp, 4*8
    ret
%else
	mov     rsi, [rsp+4*8]
	mov     rdi, [rsp+5*8]
	%ifdef _SEH_
		add     rsp, 7*8
		ret
	endproc_frame
	%else
		add     rsp, 6*8
		ret
	%endif
%endif

%endif

%ifdef DECRYPTION

    global  aes256_asm_decrypt
%ifdef DLL_EXPORT
    export  aes256_asm_decrypt
%endif

    section .data
    align   64
dec_tab:
    dec_vals v8
%ifdef LAST_ROUND_TABLES
    dec_vals w8
%endif

    section .text
    align   16

%ifdef _SEH_
proc_frame aes256_asm_decrypt
	alloc_stack	7*8			; 7 to align stack to 16 bytes
	save_reg	rsi,4*8
	save_reg	rdi,5*8
	save_reg	rbx,1*8
	save_reg	rbp,2*8
	save_reg	r12,3*8
end_prologue
    mov     rdi, rcx        ; input pointer
    mov     [rsp+0*8], rdx  ; output pointer
%else
	aes256_asm_decrypt:
	%ifdef __GNUC__
		sub     rsp, 4*8        ; gnu/linux binary interface
		mov     [rsp+0*8], rsi  ; output pointer
		mov     r8, rdx         ; context
	%else
		sub     rsp, 6*8        ; windows binary interface
		mov     [rsp+4*8], rsi
		mov     [rsp+5*8], rdi
		mov     rdi, rcx        ; input pointer
		mov     [rsp+0*8], rdx  ; output pointer
	%endif
		mov     [rsp+1*8], rbx  ; input pointer in rdi
		mov     [rsp+2*8], rbp  ; output pointer in [rsp]
		mov     [rsp+3*8], r12  ; context in r8
%endif
    add     kptr, 4*KS_LENGTH
    lea     tptr,[dec_tab wrt rip]
    sub     kptr, rofs

    mov     eax, [rdi+0*4]
    mov     ebx, [rdi+1*4]
    mov     ecx, [rdi+2*4]
    mov     edx, [rdi+3*4]

%ifdef      AES_REV_DKS
    mov     rdi, kptr
    add     kptr, 14*16
%else
    add     rdi,  14*16
%endif

    xor     eax, [rdi+rofs]
    xor     ebx, [rdi+rofs+4]
    xor     ecx, [rdi+rofs+8]
    xor     edx, [rdi+rofs+12]

 
    ii_rnd  r9, r10, r11, r12, 13
    ii_rnd  r9, r10, r11, r12, 12
    ii_rnd  r9, r10, r11, r12, 11
    ii_rnd  r9, r10, r11, r12, 10
    ii_rnd  r9, r10, r11, r12, 9
    ii_rnd  r9, r10, r11, r12, 8
    ii_rnd  r9, r10, r11, r12, 7
    ii_rnd  r9, r10, r11, r12, 6
    ii_rnd  r9, r10, r11, r12, 5
    ii_rnd  r9, r10, r11, r12, 4
    ii_rnd  r9, r10, r11, r12, 3
    ii_rnd  r9, r10, r11, r12, 2
    ii_rnd  r9, r10, r11, r12, 1
    il_rnd  r9, r10, r11, r12, 0

    mov     rbx, [rsp]
    mov     [rbx], r9d
    mov     [rbx+4], r10d
    mov     [rbx+8], r11d
    mov     [rbx+12], r12d
    xor     rax, rax
    mov     rbx, [rsp+1*8]
    mov     rbp, [rsp+2*8]
    mov     r12, [rsp+3*8]
%ifdef __GNUC__
    add     rsp, 4*8
    ret
%else
	mov     rsi, [rsp+4*8]
	mov     rdi, [rsp+5*8]
	%ifdef _SEH_
		add     rsp, 7*8
		ret
	endproc_frame
	%else
		add     rsp, 6*8
		ret
	%endif
%endif

%endif

    end
