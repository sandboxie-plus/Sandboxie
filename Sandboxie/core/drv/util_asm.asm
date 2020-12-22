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

Sbie_SepFilterTokenHandler_asm PROC

     mov         qword ptr [rsp+20h],r9  
     mov         qword ptr [rsp+18h],r8  
     mov         qword ptr [rsp+10h],rdx  
     mov         qword ptr [rsp+8],rcx  
     sub         rsp,78h  
     mov         dword ptr [rsp+60h],0  
     mov         rax,qword ptr [rsp+00000000000000A0h]  
     mov         qword ptr [rsp+50h],rax  
     mov         rax,qword ptr [rsp+0000000000000098h]  
     mov         qword ptr [rsp+48h],rax  
     mov         rax,qword ptr [rsp+0000000000000090h]  
     mov         qword ptr [rsp+40h],rax  
     mov         rax,qword ptr [rsp+0000000000000088h]  
     mov         qword ptr [rsp+38h],rax  
     mov         qword ptr [rsp+30h],0  
     mov         qword ptr [rsp+28h],0  
     mov         qword ptr [rsp+20h],0  
     xor         r9d,r9d  
     xor         r8d,r8d  
     xor         edx,edx  
     mov         rcx,qword ptr [rsp+0000000000000080h]  
     call        Token_SepFilterToken
     mov         dword ptr [rsp+60h],eax  
     mov         eax,dword ptr [rsp+60h]  
     add         rsp,78h  
     ret  

Sbie_SepFilterTokenHandler_asm ENDP
endif

;----------------------------------------------------------------------------

end
