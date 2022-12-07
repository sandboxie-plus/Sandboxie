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
; Assembler Utilities
;----------------------------------------------------------------------------

    
    AREA util_arm, CODE, READONLY
    
    EXPORT Sbie_InvokeSyscall_asm

    EXPORT Sbie_SepFilterTokenHandler_asm
    IMPORT Token_SepFilterToken

    EXPORT Sbie_CallZwServiceFunction_asm
    IMPORT Driver_KiServiceInternal


Sbie_InvokeSyscall_asm PROC
    
    ; prolog
    stp         fp,lr,[sp,#-0x10]! ; push
    mov         fp,sp  
    sub         sp,sp,#0x60  


    ; save our 3 relevant arguments to spare registers
    mov         x10,x0          ; func
    mov         w9,w1           ; count
    mov         x8,x2           ; args

    ; quick sanity check
	    ;if (count > 19)
    cmp         w9,#0x13        ; < 19
    ble         ok
            ;return 0xC000001CL;
    mov         x0, 0x001C
    movk        x0, 0xC000, lsl 16
    b           finish

ok
    ; check if we have higher arguments and if not skip 
    cmp         w9,#0x08        ; < 8
    ble         skip

	    ;return ((P_Test19)func)(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10], args[11], args[12], args[13], args[14], args[15], args[16], args[17], args[18]);

    ; copy arguments 9-19
    mov         x11,#0x00       ; offset

next
    cmp         w9,#8           ; < 8
    ble         skip

    add         x12,x8,x11      ; args + offset
    add         x12,x12,#0x40   ; args + offset + 0x40
    ldr         x13,[x12]       ; args...

    add         x14,sp,x11      ; stack + offset ... sp+0x00 -> sp+0x50
    str         x13,[x14]       ; args...

    add         x11,x11,#8      ; offset += 8

    sub         w9,w9,#1        ; -- w9
    b           next

    ; copy arguments 1-8
skip
    ldr         x7,[x8,#0x38]   ; args[7]
    ldr         x6,[x8,#0x30]   ; args[6]
    ldr         x5,[x8,#0x28]   ; args[5]
    ldr         x4,[x8,#0x20]   ; args[4]
    ldr         x3,[x8,#0x18]   ; args[3]
    ldr         x2,[x8,#0x10]   ; args[2]
    ldr         x1,[x8,#8]      ; args[1]
    ldr         x0,[x8]         ; args[0]

    ; call the function
    blr         x10  

    ; epilog
finish
    add         sp,sp,#0x60  
    ldp         fp,lr,[sp],#0x10 ; pop
    ret  

    ENDP


Sbie_SepFilterTokenHandler_asm PROC

    stp  x29, x30, [sp, #-0x10]!
    mov  x29, sp
    sub  sp, sp, #0x20
    
    ldr  x9, =Token_SepFilterToken
    ldr  x8, [x9]           ; SepFilterToken (func)
    mov  x0, x0             ; TokenObject
    stp  x3, x4, [sp, #8]   ; VariableLengthIncrease (sp+8) ; NewToken (sp+16)
    str  x2, [sp]           ; RestrictedSidPtr (sp+0)
    mov  x7, x1             ; RestrictedSidCount
    movz x6, #0             ; zero7
    movz x5, #0             ; zero6
    movz x4, #0             ; zero5
    movz x3, #0             ; zero4
    movz x2, #0             ; zero3
    movz x1, #0             ; zero2
    blr  x8

    add  sp, sp, #0x20
    ldp  x29, x30, [sp], #0x10
    ret  

    ENDP


Sbie_CallZwServiceFunction_asm PROC

    ldr  x16,[sp,#0x58]     ; prepare the sys call number
    ldr  x9, =Driver_KiServiceInternal
    ldr  x8, [x9]
    br   x8                 ; jump to KiServiceInternal

    ENDP

    
    END
