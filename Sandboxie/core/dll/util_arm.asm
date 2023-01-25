;------------------------------------------------------------------------
; Copyright 2022 David Xanatos, xanasoft.com
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
; Assembler Utilities -- ARM64
;----------------------------------------------------------------------------


    AREA my_arm, CODE, READONLY
    
    EXPORT ProtectCall2
    EXPORT ProtectCall3
    EXPORT ProtectCall4
    
    EXPORT Gui_FixupCallbackPointers

    EXPORT Ldr_Inject_Entry64
    IMPORT Ldr_Inject_Entry

    EXPORT RpcRt_NdrClientCall2
    IMPORT RpcRt_NdrClientCall2ARM64
    IMPORT __sys_NdrClientCall2
    
    EXPORT RpcRt_NdrClientCall3
    IMPORT RpcRt_NdrClientCall3ARM64
    IMPORT __sys_NdrClientCall3
    
    EXPORT RpcRt_NdrAsyncClientCall
    IMPORT RpcRt_NdrAsyncClientCallARM64
    IMPORT __sys_NdrAsyncClientCall

    EXPORT RpcRt_Ndr64AsyncClientCall
    IMPORT RpcRt_Ndr64AsyncClientCallARM64
    IMPORT Secure_HandleElevation
    IMPORT __sys_Ndr64AsyncClientCall

    EXPORT Dll_JumpStubData

    EXPORT InstrumentationCallbackAsm
    IMPORT __sys_RtlCaptureContext
    IMPORT InstrumentationCallback


;----------------------------------------------------------------------------
; ProtectCall2
;----------------------------------------------------------------------------


ProtectCall2 PROC
    stp         fp, lr, [sp, #-0x10]! ; push
    mov         x8, x0
    mov         x0, x1
    mov         x1, x2
    blr         x8
    ldp         fp, lr, [sp], #0x10 ; pop
    ret
 ENDP
    

;----------------------------------------------------------------------------
; ProtectCall3
;----------------------------------------------------------------------------


ProtectCall3 PROC
    stp         fp, lr, [sp, #-0x10]! ; push
    mov         x8, x0
    mov         x0, x1
    mov         x1, x2
    mov         x2, x3
    blr         x8
    ldp         fp, lr, [sp], #0x10 ; pop
    ret
 ENDP


;----------------------------------------------------------------------------
; ProtectCall4
;----------------------------------------------------------------------------


ProtectCall4 PROC
    stp         fp, lr, [sp, #-0x10]! ; push
    mov         x8, x0
    mov         x0, x1
    mov         x1, x2
    mov         x2, x3
    mov         x3, x4
    blr         x8
    ldp         fp, lr, [sp], #0x10 ; pop
    ret
 ENDP


;----------------------------------------------------------------------------
; Gui_FixupCallbackPointers
;----------------------------------------------------------------------------


Gui_FixupCallbackPointers PROC

    ;
    ; copy of USER32!FixupCallbackPointers
    ; with additional zeroing of the dword at [X0+8] before returning
    ;

    ldr         w8, [x0,#0x18]
    mov         w11, #0
    ldr         w9, [x0,#8]
    add         x12, x0, w8,uxtw
    cbz         w9, l02

l01
    ldr         w10, [x12],#4
    add         w11, w11, #1
    ldr         x8, [x0,w10,uxtw]
    add         x9, x0, x8
    str         x9, [x0,w10,uxtw]
    ldr         w8, [x0,#8]
    cmp         w11, w8
    b.cc        l01
    mov         w8, #0
    str         w8, [x0,#8]
l02
    ret

 ENDP


;----------------------------------------------------------------------------
; Ldr_Inject_Entry64
;----------------------------------------------------------------------------


Ldr_Inject_Entry64 PROC

    stp         fp, lr, [sp, #-0x10]! ; push
    ldr         x8, =Ldr_Inject_Entry
    blr         x8
    ldp         fp, lr, [sp], #0x10 ; pop
    br          x0

 ENDP
    

;----------------------------------------------------------------------------
; RpcRt_NdrClientCall2
;----------------------------------------------------------------------------


RpcRt_NdrClientCall2 PROC

    stp     x2,x3,[sp,#-0x30]!
    stp     x4,x5,[sp,#0x10]  
    stp     x6,x7,[sp,#0x20]  

    mov     x8, sp

    stp     x0, x1, [sp, #-0x10]!

    stp     fp, lr, [sp, #-0x10]!  
    ;stp    x19, x20, [sp, #-0x10]! 
    
           ;x0      ; pStubDescriptor
           ;x1      ; pFormat
    mov     x2, x8  ; pStack
    mov     x3, lr  ; ReturnAddress
    bl      RpcRt_NdrClientCall2ARM64

    mov     x8, x0

    ;ldp     x19, x20, [sp], #0x10
    ldp     fp, lr, [sp], #0x10

    ldp     x0, x1, [sp], #0x10

    ldp     x2, x3, [sp], #0x10
    ldp     x4, x5, [sp], #0x10
    ldp     x6, x7, [sp], #0x10

    cmp     x8, #0x00
    bne     CancelCall2

    ldr     x16, =__sys_NdrClientCall2
    ldr     x16, [x16]
    br      x16

CancelCall2
    ret

 ENDP
    

;----------------------------------------------------------------------------
; RpcRt_NdrClientCall3
;----------------------------------------------------------------------------


RpcRt_NdrClientCall3 PROC

    sub     sp,sp,#0x30  
    stp     x3,x4,[sp,#8]  
    stp     x5,x6,[sp,#0x18]  
    str     x7,[sp,#0x28]

    add     x8, sp, #8

    str     x2,[sp,#0]
    stp     x0, x1, [sp, #-0x10]!

    stp     fp, lr, [sp, #-0x10]!  
    ;stp    x19, x20, [sp, #-0x10]! 
    
           ;x0      ; pProxyInfo
           ;x1      ; nProcNum
           ;x2      ; pReturnValue
    mov     x3, x8  ; pStack
    mov     x4, lr  ; ReturnAddress
    bl      RpcRt_NdrClientCall3ARM64

    mov     x8, x0

    ;ldp     x19, x20, [sp], #0x10
    ldp     fp, lr, [sp], #0x10

    ldp     x0, x1, [sp], #0x10
    ldr     x2, [sp]

    ldr     x7,[sp,#0x28]
    ldp     x5,x6,[sp,#0x18]
    ldp     x3,x4,[sp,#8]
    add     sp,sp,#0x30  

    cmp     x8, #0x00
    bne     CancelCall3

    ldr     x16, =__sys_NdrClientCall3
    ldr     x16, [x16]
    br      x16

CancelCall3
    ret

 ENDP
    

;----------------------------------------------------------------------------
; RpcRt_NdrAsyncClientCall
;----------------------------------------------------------------------------


RpcRt_NdrAsyncClientCall PROC

    stp     x2,x3,[sp,#-0x30]!
    stp     x4,x5,[sp,#0x10]  
    stp     x6,x7,[sp,#0x20]  

    mov     x8, sp

    stp     x0, x1, [sp, #-0x10]!

    stp     fp, lr, [sp, #-0x10]!  
    ;stp    x19, x20, [sp, #-0x10]! 
    
           ;x0      ; pStubDescriptor
           ;x1      ; pFormat
    mov     x2, x8  ; pStack
    mov     x3, lr  ; ReturnAddress
    bl      RpcRt_NdrAsyncClientCallARM64

    mov     x8, x0

    ;ldp     x19, x20, [sp], #0x10
    ldp     fp, lr, [sp], #0x10

    ldp     x0, x1, [sp], #0x10

    ldp     x6, x7, [sp], #0x10
    ldp     x4, x5, [sp], #0x10
    ldp     x2, x3, [sp], #0x10

    cmp     x8, #0x00
    bne     CancelCallA

    ldr     x16, =__sys_NdrAsyncClientCall
    ldr     x16, [x16]
    br      x16

CancelCallA
    ret

 ENDP
    

;----------------------------------------------------------------------------
; RpcRt_Ndr64AsyncClientCall
;----------------------------------------------------------------------------


RpcRt_Ndr64AsyncClientCall PROC

    sub     sp,sp,#0x30  
    stp     x3,x4,[sp,#8]  
    stp     x5,x6,[sp,#0x18]  
    str     x7,[sp,#0x28]

    add     x8, sp, #8

    str     x2,[sp,#0]
    stp     x0, x1, [sp, #-0x10]!

    stp     fp, lr, [sp, #-0x10]!  
    stp     x19, x20, [sp, #-0x10]! 
    
    mov     x19, x8

           ;x0      ; pProxyInfo
           ;x1      ; nProcNum
           ;x2      ; pReturnValue
    mov     x3, x8  ; pStack
    mov     x4, lr  ; ReturnAddress
    bl      RpcRt_Ndr64AsyncClientCallARM64

    cmp     x0, #0x00
    bne     WeHandleElevation

    ldp     x19, x20, [sp], #0x10
    ldp     fp, lr, [sp], #0x10

    ldp     x0, x1, [sp], #0x10
    ldr     x2, [sp]

    ldr     x7,[sp,#0x28]
    ldp     x5,x6,[sp,#0x18]
    ldp     x3,x4,[sp,#8]
    add     sp,sp,#0x30  

    ldr     x16, =__sys_Ndr64AsyncClientCall
    ldr     x16, [x16]
    br      x16

WeHandleElevation

    mov     x0, #0  ; pStubDescriptor
    mov     x1, #0  ; pFormat
    mov     x2, x19 ; Args
    bl      Secure_HandleElevation

    ldp     x19, x20, [sp], #0x10
    ldp     fp, lr, [sp], #0x10

    add     sp,sp,#0x40  

    ret

 ENDP
 

;----------------------------------------------------------------------------
; Dll_JumpStubData
;----------------------------------------------------------------------------


Dll_JumpStubData PROC
    mov         x0, x17
    ret
 ENDP



;----------------------------------------------------------------------------
; InstrumentationCallbackAsm
;----------------------------------------------------------------------------


InstrumentationCallbackAsm PROC
    
    ;brk #0xF000
    ;br x16 ; IP0

    mov         x8,sp
    str         x8, [x18, #0x2e0]           ; Win10 TEB InstrumentationCallbackPreviousSp
    str         x16, [x18, #0x2d8]          ; Win10 TEB InstrumentationCallbackPreviousPc

    sub         sp, sp, #0x390              ; Alloc stack space for CONTEXT structure
    stp         x0, lr, [sp, #-0x10]!       ; Save original X0 and LR
    add         x0, sp, #0x10
    ldr         x16, =__sys_RtlCaptureContext
    ldr         x16, [x16]
    blr         x16                         ; Save the current register state. RtlCaptureContext does not require shadow space
    add         x0, sp, #0x10
    ldr         x2, [x18, #0x2d8]           ; Win10 TEB InstrumentationCallbackPreviousPc
    ldp         x3, x1, [sp], #0x10         ; Pass the return value and the original LR to the callback
    bl          InstrumentationCallback     ; Call main instrumentation routine
    brk         #0xF000                     ; it should not return

 ENDP

;----------------------------------------------------------------------------

 END