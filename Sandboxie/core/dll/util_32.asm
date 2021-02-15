;------------------------------------------------------------------------
; Copyright 2004-2020 Sandboxie Holdings, LLC 
;
; This program is free software: you can redistribute it and/or modify
;   it under the terms of the GNU General Public License as published by
;   the Free Software Foundation, either version 3 of the License, or
;   (at your option) any later version.
;
;   This program is distributed in the hope that it will be useful,
;   but WITHOUT ANY WARRANTY; without even the implied warranty of
;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;   GNU General Public License for more details.
;
;   You should have received a copy of the GNU General Public License
;   along with this program.  If not, see <https://www.gnu.org/licenses/>.
;------------------------------------------------------------------------

;----------------------------------------------------------------------------
; Assembler Utilities -- 32-bit
;----------------------------------------------------------------------------


;----------------------------------------------------------------------------
; ProtectCall2
;----------------------------------------------------------------------------


_ProtectCall2@12        PROC

        push ebp
        mov ebp,esp
        push edi
        mov edi,esp
        push [ebp+16]   ; push Arg2
        push [ebp+12]   ; push Arg1
        call [ebp+8]    ; call CallAddress
        mov esp,edi
        pop edi
        pop ebp
        ret 12

_ProtectCall2@12        ENDP

PUBLIC _ProtectCall2@12


;----------------------------------------------------------------------------
; ProtectCall3
;----------------------------------------------------------------------------


_ProtectCall3@16        PROC

        push ebp
        mov ebp,esp
        push edi
        mov edi,esp
        push [ebp+20]   ; push Arg3
        push [ebp+16]   ; push Arg2
        push [ebp+12]   ; push Arg1
        call [ebp+8]    ; call CallAddress
        mov esp,edi
        pop edi
        pop ebp
        ret 16

_ProtectCall3@16        ENDP

PUBLIC _ProtectCall3@16


;----------------------------------------------------------------------------
; ProtectCall4
;----------------------------------------------------------------------------


_ProtectCall4@20        PROC

        push ebp
        mov ebp,esp
        push edi
        mov edi,esp
        push [ebp+24]   ; push Arg4
        push [ebp+20]   ; push Arg3
        push [ebp+16]   ; push Arg2
        push [ebp+12]   ; push Arg1
        call [ebp+8]    ; call CallAddress
        mov esp,edi
        pop edi
        pop ebp
        ret 20

_ProtectCall4@20        ENDP

PUBLIC _ProtectCall4@20


;----------------------------------------------------------------------------
; Secure_NdrAsyncClientCall
;----------------------------------------------------------------------------


EXTERN C Secure_CheckElevation    : PROC
EXTERN C Secure_HandleElevation   : PROC
EXTERN C __sys_NdrAsyncClientCall : DWORD

Secure_NdrAsyncClientCall   PROC C PUBLIC

        call Secure_CheckElevation
        test al,al
        jnz WeHandleElevation
    
        jmp dword ptr [__sys_NdrAsyncClientCall]

WeHandleElevation:    

        jmp Secure_HandleElevation

Secure_NdrAsyncClientCall   ENDP

PUBLIC C Secure_NdrAsyncClientCall


;----------------------------------------------------------------------------
; Ldr_Inject_Entry32
;----------------------------------------------------------------------------


EXTERN _Ldr_Inject_Entry@4        : PROC

Ldr_Inject_Entry32@0        PROC C PUBLIC

    push esp            ; pRetAddr parameter
    call _Ldr_Inject_Entry@4
    
    ;
    ; clear the stack of any leftovers from Ldr_Inject_Entry.
    ; necessary because some injected code (e.g. F-Secure)
    ; assumes the stack is zero
    ;
    
    lea edi,[esp-200h]
    mov ecx,200h/4
    xor eax,eax
    cld
    rep stosd
    
    ret
    
Ldr_Inject_Entry32@0        ENDP

PUBLIC C Ldr_Inject_Entry32@0


;----------------------------------------------------------------------------
; Gui_FixupCallbackPointers
;----------------------------------------------------------------------------


Gui_FixupCallbackPointers@4 PROC C PUBLIC

    ;
    ; copy of USER32!FixupCallbackPointers
    ; with additional zeroing of the dword at [ecx+8] before returning
    ;

    push    ebp
    mov     ebp,esp
    mov     ecx,dword ptr [ebp+8]
    mov     edx,dword ptr [ecx+10h]
    push    esi
    xor     esi,esi
    add     edx,ecx
    cmp     dword ptr [ecx+8],esi
    jbe     l02
l01:    mov     eax,dword ptr [edx]
    add     eax,ecx
    add     dword ptr [eax],ecx
    inc     esi
    add     edx,4
    cmp     esi,dword ptr [ecx+8]
    jb      l01
    xor     edx,edx
    mov     dword ptr [ecx+8],edx
l02:    pop     esi
    pop     ebp
    ret     4

Gui_FixupCallbackPointers@4 ENDP

PUBLIC C Gui_FixupCallbackPointers@4


;----------------------------------------------------------------------------
; Gui_MsgWaitForMultipleObjects
;----------------------------------------------------------------------------


EXTERN C __sys_MsgWaitForMultipleObjects : DWORD

Gui_MsgWaitForMultipleObjects@20    PROC C PUBLIC
    
    test    esp,3
    jnz l03
    ; stack is dword aligned, jump directly to MsgWaitForMultipleObjects
    jmp __sys_MsgWaitForMultipleObjects
    ; align stack before calling MsgWaitForMultipleObjects
l03:    mov eax, esp
l04:    dec esp
    test    esp,3
    jnz l04
    push    eax  ; save unaligned esp
    push    [eax+20] ; arg 5
    push    [eax+16] ; arg 4
    push    [eax+12] ; arg 3
    push    [eax+8]  ; arg 2
    push    [eax+4]  ; arg 1
    call    __sys_MsgWaitForMultipleObjects
    pop esp  ; restore unaligned esp
    ret 20

Gui_MsgWaitForMultipleObjects@20    ENDP

PUBLIC C Gui_MsgWaitForMultipleObjects@20


;----------------------------------------------------------------------------
; RpcRt_NdrClientCall
;----------------------------------------------------------------------------


EXTERN C RpcRt_NdrClientCall_x86    : PROC
;EXTERN C RpcRt_NdrClientCall_...   : PROC
EXTERN C __sys_NdrClientCall : DWORD

RpcRt_NdrClientCall   PROC C PUBLIC

        call RpcRt_NdrClientCall_x86
        test al,al
        jnz CancelCall
    
        jmp dword ptr [__sys_NdrClientCall]

CancelCall:

;        jmp RpcRt_NdrClientCall_...
		ret

RpcRt_NdrClientCall   ENDP

PUBLIC C RpcRt_NdrClientCall


;----------------------------------------------------------------------------
; RpcRt_NdrClientCall2
;----------------------------------------------------------------------------


EXTERN C RpcRt_NdrClientCall2_x86    : PROC
;EXTERN C RpcRt_NdrClientCall2_...   : PROC
EXTERN C __sys_NdrClientCall2 : DWORD

RpcRt_NdrClientCall2   PROC C PUBLIC

        call RpcRt_NdrClientCall2_x86
        test al,al
        jnz CancelCall2
    
        jmp dword ptr [__sys_NdrClientCall2]

CancelCall2:

;        jmp RpcRt_NdrClientCall2_...
		ret

RpcRt_NdrClientCall2   ENDP

PUBLIC C RpcRt_NdrClientCall2


;----------------------------------------------------------------------------
; RpcRt_NdrClientCall4
;----------------------------------------------------------------------------


EXTERN C RpcRt_NdrClientCall4_x86    : PROC
;EXTERN C RpcRt_NdrClientCall4_...   : PROC
EXTERN C __sys_NdrClientCall4 : DWORD

RpcRt_NdrClientCall4   PROC C PUBLIC

        call RpcRt_NdrClientCall4_x86
        test al,al
        jnz CancelCall4
    
        jmp dword ptr [__sys_NdrClientCall4]

CancelCall4:

;        jmp RpcRt_NdrClientCall4_...
		ret

RpcRt_NdrClientCall4   ENDP

PUBLIC C RpcRt_NdrClientCall4