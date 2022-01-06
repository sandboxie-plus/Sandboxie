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
; Assembler Utilities
;----------------------------------------------------------------------------

ifdef _WIN64

;

else

.386p
.model flat

endif

;----------------------------------------------------------------------------

.code

;----------------------------------------------------------------------------

mov_eax_cr0 MACRO
ifdef _WIN64
        mov rax, cr0
else
        mov eax, cr0
endif
                ENDM

mov_cr0_eax MACRO
ifdef _WIN64
        mov cr0, rax
else
        mov cr0, eax
endif
                ENDM

;----------------------------------------------------------------------------

ifdef _WIN64
DisableWriteProtect PROC
else
_DisableWriteProtect@0 PROC
endif

        mov_eax_cr0
        and eax, not 10000h      ; clear WP bit
        mov_cr0_eax

        ret

ifdef _WIN64
DisableWriteProtect ENDP
else
_DisableWriteProtect@0 ENDP
PUBLIC _DisableWriteProtect@0
endif

;----------------------------------------------------------------------------

ifdef _WIN64
EnableWriteProtect PROC
else
_EnableWriteProtect@0 PROC
endif

        mov_eax_cr0
        or  eax, 10000h          ; set WP bit
        mov_cr0_eax

        ret

ifdef _WIN64
EnableWriteProtect ENDP
else
_EnableWriteProtect@0 ENDP
PUBLIC _EnableWriteProtect@0
endif

;----------------------------------------------------------------------------

ifdef _WIN64

EXTERN Process_FindSandboxed : proc

        align 16

Process_FindSandboxed64 PROC FRAME

        ; prolog
        sub    rsp,50h
        .allocstack(50h)
        .endprolog
        ; save caller's registers.
        mov    [rsp+28h+00h],rcx
        mov    [rsp+28h+08h],rdx
        mov    [rsp+28h+10h],r8
        mov    [rsp+28h+18h],r9
        ; call Process_FindSandboxed(NULL, NULL)
        xor    rcx,rcx
        xor    rdx,rdx
        call   Process_FindSandboxed
        mov    r10,rax
        ; restore caller's registers
        mov    rcx,[rsp+28h+00h]
        mov    rdx,[rsp+28h+08h]
        mov    r8, [rsp+28h+10h]
        mov    r9, [rsp+28h+18h]
        ; epilog
        add    rsp,50h
        ret

Process_FindSandboxed64 ENDP

endif

;----------------------------------------------------------------------------

ifdef _WIN64
Process_GetCurrent PROC

        align 16

        ; 64-bit:  Process_FindSandboxed64 stores current process in R10

        mov rax, r10
        ret

Process_GetCurrent ENDP

else

        align 4

_Process_GetCurrent@0 PROC

        ; 32-bit:  current process is passed in EAX

        ret

_Process_GetCurrent@0 ENDP
PUBLIC _Process_GetCurrent@0

endif

;----------------------------------------------------------------------------

ifdef _WIN64

InvokeCPUID PROC

    mov r11, rbx

        xor rax, rax
    mov eax, ecx
    mov r10, rdx
    db 0Fh, 0A2h ; cpuid

    mov dword ptr [r10+0],  eax
    mov dword ptr [r10+4],  ebx
    mov dword ptr [r10+8],  ecx
    mov dword ptr [r10+12], edx

    mov rbx, r11
    ret
    
InvokeCPUID ENDP

else

_InvokeCPUID@8 PROC

    push ebx
    push esi

    mov esi, dword ptr [esp+8+8] ; +8 for push ebx,esi

    pushfd
    pop ebx          ; ebx = flags
    xor ebx, 200000h ; toggle cpuid flag
    push ebx
    popfd            ; flags = ebx
    pushfd
    pop ecx          ; ecx = flags
    and ebx, 200000h
    and ecx, 200000h
    cmp ebx, ecx
    je cont
    
    xor eax, eax
    xor ebx, ebx
    xor ecx, ecx
    xor edx, edx
    jmp done

cont:   mov eax, dword ptr [esp+8+4] ; +8 for push ebx,esi
    db 0Fh, 0A2h ; cpuid
       
done:   mov dword ptr [esi+0],  eax
    mov dword ptr [esi+4],  ebx
    mov dword ptr [esi+8],  ecx
    mov dword ptr [esi+12], edx
    pop esi
    pop ebx
    ret 8
    
_InvokeCPUID@8 ENDP
PUBLIC _InvokeCPUID@8

endif

;----------------------------------------------------------------------------

ifdef _WIN64

EXTERN Token_SepFilterToken : QWORD

Sbie_SepFilterTokenHandler_asm PROC FRAME

    mov         qword ptr [rsp+20h],r9  
    mov         qword ptr [rsp+18h],r8  
    mov         qword ptr [rsp+10h],rdx  
    mov         qword ptr [rsp+8],rcx  

    sub         rsp,78h  
    .allocstack 78h
    .endprolog

    mov         dword ptr [rsp+60h],0  
    mov         rax,qword ptr [rsp+0A0h] ; NewToken
    mov         qword ptr [rsp+50h],rax  
    mov         rax,qword ptr [rsp+098h] ; LengthIncrease
    mov         qword ptr [rsp+48h],rax  
    mov         rax,qword ptr [rsp+090h] ; SidPtr
    mov         qword ptr [rsp+40h],rax  
    mov         rax,qword ptr [rsp+088h] ; SidCount
    mov         qword ptr [rsp+38h],rax  
    mov         qword ptr [rsp+30h],0  
    mov         qword ptr [rsp+28h],0  
    mov         qword ptr [rsp+20h],0  
    mov         r9d,0
    mov         r8d,0
    mov         edx,0
    mov         rcx,qword ptr [rsp+080h] ; TokenObject 
    call        Token_SepFilterToken

    add         rsp,78h  

    ret  

Sbie_SepFilterTokenHandler_asm ENDP

endif

;----------------------------------------------------------------------------

ifdef _WIN64

; NTSTATUS Sbie_InvokeSyscall_asm(void* func, ULONG count, void* args);
Sbie_InvokeSyscall_asm PROC FRAME

    ; prolog
    push        rsi
    .pushreg    rsi
    push        rdi
    .pushreg    rdi
    sub         rsp, 98h ; 8 * 19 - prepare enough stack for up to 19 arguments
    .allocstack 98h
    .endprolog
     
    ; quick sanity check
    cmp         rdx, 13h ; if count > 19
    jle         arg_count_ok
    mov         rax, 0C000001Ch ; return STATUS_INVALID_SYSTEM_SERVICE
    jmp         func_return
arg_count_ok:

    ; save our 3 relevant arguments to spare registers
    mov         r11, r8  ; args
    mov         r10, rdx ; count
    mov         rax, rcx ; func

    ; check if we have higher arguments and if not skip 
    cmp         r10, 4
    jle         copy_reg_args
    ; copy arguments 5-19
    mov         rsi, r11 ; source
    add         rsi, 20h
    mov         rdi, rsp ; destination
    add         rdi, 20h
    mov         rcx, r10 ; arg count
    sub         rcx, 4   ; skip the register passed args
    rep movsq

copy_reg_args:
    ; copy arguments 1-4
    mov         r9,  qword ptr [r11+18h]
    mov         r8,  qword ptr [r11+10h]
    mov         rdx, qword ptr [r11+08h]
    mov         rcx, qword ptr [r11+00h]

    ; call the function
    call        rax

func_return:
    ; epilog
    add         rsp, 98h  
    pop         rdi
    pop         rsi

    ret  

Sbie_InvokeSyscall_asm ENDP

else

; NTSTATUS Sbie_InvokeSyscall_asm(void* func, ULONG count, void* args);
_Sbie_InvokeSyscall_asm@12 PROC

    ; prolog
    push        ebp  
    push        esi
    push        edi
    mov         ebp, esp  
    sub         esp, 4Ch ; 4 * 19 - prepare enough stack for up to 19 arguments

    ; quick sanity check
    cmp         dword ptr [ebp+10h+4h], 13h ; arg count @count
    jle         arg_count_ok
    mov         eax, 0C000001Ch ; return STATUS_INVALID_SYSTEM_SERVICE
    jmp         func_return
arg_count_ok:

    ; copy arguments 0-19
    mov         esi, dword ptr [ebp+10h+8h] ; source @args
    mov         edi, esp ; destination
    mov         ecx, dword ptr [ebp+10h+4h] ; arg count @count
    rep movsd

    ; call the function
    mov         eax, dword ptr [ebp+10h+0h] ; @func
    call        eax

func_return:
    ; epilog
    mov         esp,ebp  
    pop         edi
    pop         esi
    pop         ebp
    ret         0Ch

_Sbie_InvokeSyscall_asm@12 ENDP
PUBLIC _Sbie_InvokeSyscall_asm@12

endif

;----------------------------------------------------------------------------


end
