;------------------------------------------------------------------------
; Copyright 2022-2023 David Xanatos, xanasoft.com
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

	AREA |.text|, CODE, READONLY

    IMPORT  EntrypointC

    IMPORT  DetourFunc

    ;EXPORT  ServiceDataPtr
    EXPORT  SystemServiceARM64

    EXPORT  NtDeviceIoControlFileEC
    EXPORT  DeviceIoControlSvc
    EXPORT  EcExitThunkPtr

    ;EXPORT  InjectDataPtr
    EXPORT  DetourCodeARM64

    EXPORT  SbieLowData

;----------------------------------------------------------------------------
; break
;----------------------------------------------------------------------------

break PROC

    brk #0xF000

 ENDP

;----------------------------------------------------------------------------
; Entrypoint at Start 
;----------------------------------------------------------------------------

Start PROC

    ;brk     #0xF000

    stp     x0, x1, [sp, #-0x10]!
    stp     x2, x3, [sp, #-0x10]!
    stp     x4, x5, [sp, #-0x10]!
    stp     x6, x7, [sp, #-0x10]!
    stp     fp, lr, [sp, #-0x10]!  

    ; EntrypointC()

    ;ldr     x8, =EntrypointC
    ;ldr     x9, =0x180001000    ; Base Address + Segment Offset
    ;sub     x8, x8, x9
    ;adrp    x9, 0
    ;add     x8, x8, x9
    ;blr     x8
    bl      EntrypointC

    mov     x8, x0

    ldp     fp, lr, [sp], #0x10
    ldp     x6, x7, [sp], #0x10
    ldp     x4, x5, [sp], #0x10
    ldp     x2, x3, [sp], #0x10
    ldp     x0, x1, [sp], #0x10

    br      x8                  ; jump to LdrInitializeThunk trampoline

 ENDP

;----------------------------------------------------------------------------
; SystemService
;----------------------------------------------------------------------------

ServiceDataPtr
    DCQ 0
SystemServiceARM64 Proc 

    ;brk     #0xF000

	;
	; spill parameters in registers onto the stack
	;

    stp     x6, x7, [sp, #-0x10]!
    stp     x4, x5, [sp, #-0x10]!
    stp     x2, x3, [sp, #-0x10]!
    stp     x0, x1, [sp, #-0x10]!

    mov     x8, sp                          ; x8 -> argument stack
    ldr     x9, ServiceDataPtr              ; x9 -> SbieLowData

    stp     fp, lr, [sp, #-0x10]!  
    ;stp    x19, x20, [sp, #-0x10]! 
    
	;
	; make room for several qwords on the stack:
	;     ULONG64 arg_9_arg_10_for_NtDeviceIoControlFile[2] (2)
	;     IO_STATUS_BLOCK IoStatusBlock64;	#2 on stack     (2)
	;     ULONG64 IoStatusBlock32;		#4 on stack         (1)
	;     ULONG64 parms[API_NUM_ARGS];	#5 on stack         (8)
    ;     ULONG64 padding                                   (1)
	;

    sub     sp, sp, (1 + 8 + 1 + 2 + 2) * 8 ; +1 must be 16 aligned !!!
    
	;
	; parms[1] = syscall_index
	; parms[2] = pointer to system service arguments on stack
	; parms[0] = API_INVOKE_SYSCALL (from SbieLowData)
	;

    add     x10, sp, (5*8)                  ; x10 -> parms

    str     x17, [x10, (1*8)]               ; syscall index
    str     x8, [x10, (2*8)]                ; stack address
    ldr     w8, [x9, (3*8+4)]               ; x8 -> data->api_invoke_syscall
    str     x8, [x10, (0*8)]                ; API_INVOKE_SYSCALL

	;
	; the 64-bit kernel function for NtDeviceIoControlFile
	; expects the 64-bit IoStatusBlock argument to contain a
	; 32-bit pointer to a 32-bit IoStatusBlock structure.
	; we can allocate the 32-bit structure on the stack
	; because the stack is 32-bits in a 32-bit program
	;

    add     x11, sp, (4*8)                  ; x11 -> IoStatusBlock32
    add     x12, sp, (2*8)                  ; x12 -> IoStatusBlock64
    str     x11, [x12]

	;
	; invoke SbieLow.NtDeviceIoControlFile(
	;		data->api_device_handle,
	;		NULL, NULL, NULL,
	;		&MyIoStatusBlock,
	;		API_SBIEDRV_CTLCODE,
	;		parms,
	;		API_NUM_ARGS * sizeof(ULONG64),
	;		NULL, 0);
	;

    ldr     x0, [x9, (2*8)]                 ; data->api_device_handle
    mov     x1, #0                          ; NULL
    mov     x2, #0                          ; NULL
    mov     x3, #0                          ; NULL
    mov     x4, x12                         ; MyIoStatusBlock64
    ldr     w5, [x9, (3*8+0)]               ; data->api_sbiedrv_ctlcode
    mov     x6, x10                         ; parms
    mov     x7, (8*8)                       ; API_NUM_ARGS * sizeof(ULONG64)
    mov     x8, #0
    add     x11, sp, 0
    str     x8, [x11]                       ; NULL
    add     x12, sp, 8
    str     x8, [x12]                       ; 0

    ;add     x8, x9, #0x80                   ; x8 -> SbieLow.NtDeviceIoControlFile_code
    add     x8, x9, #0xE8                   ; x8 -> SbieLow.NtDeviceIoControlFile
    ldr     x8, [x8]

    blr     x8

    ;
    ; cleanup
    ;

    add     sp, sp, (1 + 8 + 1 + 2 + 2) * 8

    ;ldp     x19, x20, [sp], #0x10
    ldp     fp, lr, [sp], #0x10
    add     sp, sp, #0x40                   ; spilled arguments

    ret

 ENDP


;----------------------------------------------------------------------------
; MyHandleStubHijack
;----------------------------------------------------------------------------


EcExitThunkPtr
    DCQ 0                           ; &__os_arm64x_dispatch_call_no_redirect
MyHandleStubHijack PROC

    stp  x29, x30, [sp, #-0x10]!
    mov  x29, sp
    sub  sp, sp, #0x20

    mov  x8, x0

    ldr  x16, EcExitThunkPtr
    ldr  x16, [x16]                 ; get value of __os_arm64x_dispatch_call_no_redirect
    blr  x16                        ; Call the emulator

    b    MyHandleStubHijackEnd

    DCD  0x00000000
    DCD  0x00000000
    DCD  0x00000000
    DCD  0x05000000

MyHandleStubHijackEnd
    nop 
    mov x0, x8                      ; Move return from RAX (x8) to x0

    add sp, sp, #0x20
    ldp x29, x30, [sp], #0x10
    ret 
    
 ENDP


;----------------------------------------------------------------------------
; NtDeviceIoControlFileEC
;----------------------------------------------------------------------------


NtDeviceIoControlFileEC PROC

    sub  sp, sp, #0x10

    adrl x16, MyHandleStubHijackEnd ; Put MyHandleStubHijackEnd in x16
    str  x16, [sp]                  ; Save address to stack

DeviceIoControlSvc
    svc  #0x07                      ; Issue the sys call

    ldr  x9, [sp]                   ; Load address from stack to x9
    adrl x16, MyHandleStubHijackEnd ; Again put MyHandleStubHijackEnd in x16
    subs x16, x16, x9               ; Compare the addresses

    add  sp, sp, #0x10

    b.ne MyHandleStubHijack         ; Jump to MyHandleStubHijack if the address changed

    movz x9, #0
    ret

 ENDP


;----------------------------------------------------------------------------
; detour code loading SbieDll.dll
;----------------------------------------------------------------------------


InjectDataPtr
    DCQ 0
DetourCodeARM64 PROC
    
    ;brk     #0xF000
    
    stp     fp, lr, [sp, #-0x10]!  
    stp     x19, x20, [sp, #-0x10]! 
    sub     sp, sp, #0x40

    stp     x0, x1, [sp, #0x00]
    stp     x2, x3, [sp, #0x10]
    stp     x4, x5, [sp, #0x20]
    stp     x6, x7, [sp, #0x30]

    ldr     x19, InjectDataPtr              ; x19 -> inject data area

	;
	; call DetourFunc
	;

    mov     x0, x19                         ; [x19].InjectData
    bl      DetourFunc

;    cmp	    x0, #0x00
;    bne	    DetourError

	;
	; resume execution or original function
	;

    ldp     x0, x1, [sp, #0x00]
    ldp     x2, x3, [sp, #0x10]
    ldp     x4, x5, [sp, #0x20]
    ldp     x6, x7, [sp, #0x30]

    ldr     x8, [x19, 0x08]                 ; [x19].InjectData.RtlFindActCtx

    add     sp, sp, #0x40
    ldp     x19, x20, [sp], #0x10
    ldp     fp, lr, [sp], #0x10

    br      x8

;DetourError
;
;    add     sp, sp, #0x40
;    ldp     x19, x20, [sp], #0x10
;    ldp     fp, lr, [sp], #0x10
;
;    ret

 ENDP


;;----------------------------------------------------------------------------
;; RtlFindActivationContextSectionString detour code
;;----------------------------------------------------------------------------
;
;
;InjectDataPtr
;    DCQ 0
;DetourCodeARM64 PROC
;    
;    ;brk     #0xF000
;    
;    stp     fp, lr, [sp, #-0x10]!  
;    stp     x19, x20, [sp, #-0x10]! 
;    sub     sp, sp, #0x40
;
;    stp     x0, x1, [sp, #0x00]
;    stp     x2, x3, [sp, #0x10]
;    stp     x4, x5, [sp, #0x20]
;    stp     x6, x7, [sp, #0x30]
;
;    ldr     x19, InjectDataPtr              ; x19 -> inject data area
;
;    ;
;    ; reatore RtlFindActCtx, copy 16 bytes	
;    ;
;
;    add     x8, x19, #0x20                  ; [x19].InjectData.RtlFindActCtx
;    ldr     x9, [x8]
;    ldp     w10, w11, [x19, #0x2C]          ; [x19].InjectData.RtlFindActCtx_Bytes
;    stp     w10, w11, [x9, #0x00]
;    ldp     w10, w11, [x19, #0x34]          ; [x19].InjectData.RtlFindActCtx_Bytes + 8
;    stp     w10, w11, [x9, #0x08]
;
;    ldr     x0, =0xFFFFFFFFFFFFFFFF         ; ProcessHandle
;    mov     x1, x9                          ; BaseAddress
;    mov     x2, #0x10                       ; NumberOfBytesToFlush
;
;    ;ldr     x8, [x19, 0x70]                 ; [x19].InjectData.NtFlushInstructionCache
;    ldr     x9, [x19]                       ; [x19].InjectData.SBIELOW_DATA
;    add     x8, x9, 0xA0                    ; SBIELOW_DATA.NtFlushInstructionCache_code
;    blr     x8
;
;	;
;	; call LdrLoadDll for kernel32
;	;
;
;    mov     x20, #0x10                      ; retry count
;
;LdrLoadRetry
;    mov     x0, #0x00                       ; PathToFile
;    mov     x1, #0x00                       ; Flags
;    add     x2, x19, 0x40                   ; [x19].InjectData.KernelDll_Unicode
;    add     x3, x19, 0x60                   ; [x19].InjectData.ModuleHandle
;
;    ldr     x8, [x19, 0x08]                 ; [x19].InjectData.LdrLoadDll
;    blr     x8
;
;    cmp	    x0, #0x00
;    beq     LdrLoadGood
;    sub     x20, x20, #0x01
;    cmp     x20, #0x00
;    bne     LdrLoadRetry
;    b       RtlFindActivationContextSectionStringError
;
;LdrLoadGood
;	;
;	; call LdrLoadDll for sbiedll
;	;
;
;    mov     x0, #0x00                       ; PathToFile
;    mov     x1, #0x00                       ; Flags
;    add     x2, x19, 0x50                   ; [x19].InjectData.SbieDll_Unicode
;    add     x3, x19, 0x60                   ; [x19].InjectData.ModuleHandle
;
;    ldr     x8, [x19, 0x08]                 ; [x19].InjectData.LdrLoadDll
;    blr     x8
;    cmp	    x0, #0x00
;    bne	    RtlFindActivationContextSectionStringError
;
;	;
;	; call the custom MyGetProcedureAddress implemented in c
;    ; which calls LdrGetProcedureAddress for sbiedll ordinal 1,
;	; this forces ntdll to initialize sbiedll and returns the address to call
;    ; 
;    ; in ARM64EC mode it returns the native function address instead of the FFS sequence
;	;
;
;    ldr     x0, [x19, 0x60]                 ; [x19].InjectData.ModuleHandle
;    mov     x1, #0x00                       ; FunctionName
;    mov     x2, #0x01                       ; Ordinal
;    add     x3, x19, 0x68                   ; [x19].InjectData.SbieDllOrdinal1
;    mov     x4, x19                         ; [x19].InjectData
;
;    ;ldr     x8, [x19, 0x10]                 ; [x19].InjectData.LdrGetProcAddr
;    ldr     x8, [x19, 0x70]                 ; [x19].InjectData.MyGetProcAddr
;    blr     x8
;    cmp	    x0, #0x00
;    bne	    RtlFindActivationContextSectionStringError
;
;	;
;	; pass control to ordinal 1, which will free the inject
;	; data area, and pass control to the original function
;	; RtlFindActivationContextSectionString
;	;
;	; note that we need to pass the address of the inject
;	; data area to ordinal 1, which we do by overwriting the
;	; first argument.  the original argument is saved in
;	; the inject data area
;	;
;
;    ldr     x8, [sp, #0x00]
;    str     x8, [x19, 0x08]                 ; [x19].InjectData.LdrLoadDll
;    mov     x0, x19
;    ldr     x1, [sp, #0x08]
;    ldp     x2, x3, [sp, #0x10]
;    ldp     x4, x5, [sp, #0x20]
;    ldp     x6, x7, [sp, #0x30]
;
;    ldr     x8, [x19, 0x68]                 ; [x19].InjectData.SbieDllOrdinal1
;
;    add     sp, sp, #0x40
;    ldp     x19, x20, [sp], #0x10
;    ldp     fp, lr, [sp], #0x10
;
;    br      x8
;
;RtlFindActivationContextSectionStringError
;
;    str     x0, [sp, #0x38]                 ; save ntstatus
;
;    add     x8, x19, 0x50                   ; [x19].InjectData.SbieDll_Unicode
;    str	    x8, [x19, 0x08]                 ; [x19].InjectData.LdrLoadDll
;
;    add     x5, x19, 0x10                   ; out_response - [x19].InjectData.LdrGetProcAddr
;    mov	    x4, #0x01                       ; response_buttons - ERROR_OK
;    mov	    x3, x8                          ; list_of_pointers_to_parameters
;    mov	    x2, #0x01                       ; mask_of_strings_in_list
;    mov	    x1, #0x01                       ; number_of_parameters_in_list
;    ldr	    x0, =0xD0000142                 ; ntstatus_message_code - (STATUS_DLL_INIT_FAILED or FORCE_ERROR_MESSAGE_BOX)
;
;    ldr     x8, [x19, 0x18]                 ; [x19].InjectData.LdrGetProcAddr
;    blr     x8
;
;    ldr     x0, [sp, #0x38]                 ; restore ntstatus
;
;    add     sp, sp, #0x40
;    ldp     x19, x20, [sp], #0x10
;    ldp     fp, lr, [sp], #0x10
;
;    ret
;
; ENDP

 
;----------------------------------------------------------------------------
; Parameters stored by SbieSvc
;----------------------------------------------------------------------------


    ALIGN 8

SbieLowData
    ; at least sizeof(SBIELOW_DATA)
    DCQ 0,0,0,0,0,0,0,0
    DCQ 0,0,0,0,0,0,0,0
    DCQ 0,0,0,0,0,0,0,0
    DCQ 0,0,0,0,0,0,0,0
    DCQ 0,0,0,0,0,0,0,0
    DCQ 0,0,0,0,0,0,0,0
    DCQ 0,0,0,0,0,0,0,0
    DCQ 0,0,0,0,0,0,0,0


;----------------------------------------------------------------------------
; Tail Signature
;----------------------------------------------------------------------------


	AREA |zzzzz|, DATA, READONLY

    DCQ Start                   ; entry point for the detour
    DCQ SbieLowData             ; data location
    DCQ DetourCodeARM64         ; detour code location


;----------------------------------------------------------------------------

 END