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
; Assembler Utilities -- ARM64EC
;----------------------------------------------------------------------------


    AREA my_arm__ec, CODE, READONLY
    

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
    

;----------------------------------------------------------------------------
; RpcRt_NdrClientCall2
;----------------------------------------------------------------------------


RpcRt_NdrClientCall2 PROC

    stp     x2,x3,[sp,#-0x10]!
    ; x4 first argument on stack
    ; x5 arguments size on stack

    mov     x8, sp

    stp     x0, x1, [sp, #-0x10]!

    stp     fp, lr, [sp, #-0x10]!  
    ;stp     x19, x20, [sp, #-0x10]! 
    stp     x4, x5, [sp, #-0x10]! 
    
           ;x0      ; pStubDescriptor
           ;x1      ; pFormat
    mov     x2, x8  ; pStack
    mov     x3, lr  ; ReturnAddress
    bl      RpcRt_NdrClientCall2ARM64

    mov     x8, x0

    ldp     x4, x5, [sp], #0x10
    ;ldp     x19, x20, [sp], #0x10
    ldp     fp, lr, [sp], #0x10

    ldp     x0, x1, [sp], #0x10

    ldp     x2, x3, [sp], #0x10

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

    sub     sp,sp,#0x10  
    str     x3,[sp,#0x8]
    ; x4 first argument on stack
    ; x5 arguments size on stack

    add     x8, sp, #8

    str     x2,[sp,#0]
    stp     x0, x1, [sp, #-0x10]!

    stp     fp, lr, [sp, #-0x10]!  
    ;stp    x19, x20, [sp, #-0x10]! 
    stp     x4, x5, [sp, #-0x10]! 

           ;x0      ; pProxyInfo
           ;x1      ; nProcNum
           ;x2      ; pReturnValue
    mov     x3, x8  ; pStack
    bl      RpcRt_NdrClientCall3ARM64

    mov     x8, x0
    
    ldp     x4, x5, [sp], #0x10
    ;ldp     x19, x20, [sp], #0x10
    ldp     fp, lr, [sp], #0x10

    ldp     x0, x1, [sp], #0x10
    ldr     x2, [sp]

    ldr     x3,[sp,#0x8]
    add     sp,sp,#0x10  

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

    stp     x2,x3,[sp,#-0x10]!
    ; x4 first argument on stack
    ; x5 arguments size on stack

    mov     x8, sp

    stp     x0, x1, [sp, #-0x10]!

    stp     fp, lr, [sp, #-0x10]!  
    ;stp    x19, x20, [sp, #-0x10]! 
    stp     x4, x5, [sp, #-0x10]! 

           ;x0      ; pStubDescriptor
           ;x1      ; pFormat
    mov     x2, x8  ; pStack
    mov     x3, lr  ; ReturnAddress
    bl      RpcRt_NdrAsyncClientCallARM64

    mov     x8, x0

    ldp     x4, x5, [sp], #0x10
    ;ldp     x19, x20, [sp], #0x10
    ldp     fp, lr, [sp], #0x10

    ldp     x0, x1, [sp], #0x10

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

    sub     sp,sp,#0x10  
    str     x3,[sp,#0x8]
    ; x4 first argument on stack
    ; x5 arguments size on stack

    add     x8, sp, #8

    str     x2,[sp,#0]
    stp     x0, x1, [sp, #-0x10]!

    stp     fp, lr, [sp, #-0x10]!  
    stp     x19, x20, [sp, #-0x10]! 
    stp     x4, x5, [sp, #-0x10]! 

    mov     x19, x8

           ;x0      ; pProxyInfo
           ;x1      ; nProcNum
           ;x2      ; pReturnValue
    mov     x3, x8  ; pStack
    bl      RpcRt_Ndr64AsyncClientCallARM64

    cmp     x0, #0x00
    bne     WeHandleElevation

    ldp     x4, x5, [sp], #0x10
    ldp     x19, x20, [sp], #0x10
    ldp     fp, lr, [sp], #0x10

    ldp     x0, x1, [sp], #0x10
    ldr     x2, [sp]

    ldr     x3,[sp,#0x8]
    
    add     sp,sp,#0x10  

    ldr     x16, =__sys_Ndr64AsyncClientCall
    ldr     x16, [x16]
    br      x16

WeHandleElevation

    mov     x0, #0  ; pStubDescriptor
    mov     x1, #0  ; pFormat
    mov     x2, x19 ; Args
    bl      Secure_HandleElevation

    ldp     x4, x5, [sp], #0x10
    ldp     x19, x20, [sp], #0x10
    ldp     fp, lr, [sp], #0x10

    add     sp,sp,#0x20  

    ret

 ENDP
 

;----------------------------------------------------------------------------

 END