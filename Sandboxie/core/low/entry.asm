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
; Low Level DLL, entrypoint and boundary signatures
; The code section of this DLL is not executed as a typical "loaded" module
; in virtual address space.  Instead this dll is loaded from a resource in 
; SbieSrv.exe named Lowlevel.  The code section is copied from this resource
; and load directly in to memory and injected in to various targets.  Thus the
; code section of this dll is executed like shell code because of the manual 
; load techinque used by SbieSrv.exe and needs to execute in various memory locations
; since it is the injected code.  SbieSrv.exe will parse the PE Header of
; lowlevel.dll to locate the code section and copy it.  To simplify the work 
; needed to locate key locations with in the object code generated
; by entry.asm, lowlevel.dll needs to have the base address set to 0 in its PE Header.  
; There is a dependency on the linker setting that sets the base address of the dll to zero.  
; A non-zero address in the base address section of the PE header will result in a error from
; SbieSrv.exe.  There is also a dependency on the section names ".text" and "zzzz".
; SbieSrv.exe uses these section names to find the code section and critical symbol
; locations.  Currently these two sections need to be the first two sections defined
; in lowlevel.dll: ".text" must be the first section followed by "zzzz"
;----------------------------------------------------------------------------



ifndef _WIN64

.386p
.model flat

endif


;----------------------------------------------------------------------------
;Removed hard coded Head and Tail Signature dependency 
;Head Signature - deprecated
;----------------------------------------------------------------------------

		.code 
	start:	

;----------------------------------------------------------------------------
; Entrypoint at _Start 
;----------------------------------------------------------------------------

ifdef _WIN64	; 64-bit

EXTERN 		EntrypointC : PROC

_Start:
		sub	rsp, 28h		; standard stack frame
		mov	qword ptr [rsp+4*8], rcx
		mov	qword ptr [rsp+5*8], rdx
		mov	qword ptr [rsp+6*8], r8
		mov	qword ptr [rsp+7*8], r9

		call	$+5
_001:	pop     rcx
		; removed hard coded position dependency
        ; key symbols are now passed as arguments to EntrypointC
        ; 64 bit version takes 4 arguments
		; _EntrypointC(SbieLowData,_RtlFindActivationContextSectionString,_SystemService,_RtlFindActivationContextSectionString64)
		
		mov rbx,rcx
		add	rcx, offset SbieLowData - _001
		mov rdx,rbx  
		add rdx, offset _RtlFindActivationContextSectionString - _001
		mov r8,rbx
		add r8, offset _SystemService - _001
		mov r9,rbx
		add r9,  offset _RtlFindActivationContextSectionString64 - _001
        		
		call	EntrypointC
		
		mov	rcx, qword ptr [rsp+4*8]
		mov	rdx, qword ptr [rsp+5*8]
		mov	r8, qword ptr [rsp+6*8]
		mov	r9, qword ptr [rsp+7*8]
		add	rsp, 28h
		jmp	rax	; jump to LdrInitializeThunk trampoline

else 		; 32-bit

EXTERN 		_EntrypointC@12 : PROC
_Start:		
		call	$+5
_001:		pop	eax
		mov edx,eax
		; removed hard coded position dependency
		; key symbols are now passed as arguments to EntrypointC
		; 32 bit version takes 3 arguments
		;_EntrypointC(SbieLowData,_RtlFindActivationContextSectionString,_SystemService)
		add eax, offset _SystemService - _001 ;old + 96 offset  
		push eax
		mov eax,edx
		add eax, offset _RtlFindActivationContextSectionString - _001; old + 256 offset
		push eax
		mov eax, edx
		add	eax, offset SbieLowData - _001
		push eax
		call	_EntrypointC@12
		jmp	eax	; jump to LdrInitializeThunk trampoline
	
endif 		; 32-bit or 64-bit


;----------------------------------------------------------------------------
; SystemService
;----------------------------------------------------------------------------


_SystemService:

;----------------------------------------------------------------------------
ifdef _WIN64	; 64-bit
myService Proc  
		db	48h, 0B8h	; rax -> SbieLowData
		dq	?
		;
		; make room for several qwords on the stack:
		;     ULONG64 arg_space_for_NtDeviceIoControlFile[10]
		;     IO_STATUS_BLOCK IoStatusBlock64;	#10 on stack
		;     ULONG64 IoStatusBlock32;		#12 on stack
		;     ULONG64 parms[API_NUM_ARGS];	#13 on stack
		; note that stack pointer is known to be xxxx8 on entry,
		; because of x64 calling convention.
		;

		push rbp; target rsp
		push rbx; target rip

		mov r11,[rax + 0e0h] ; real NtDeviceIOControlFile
        add r11b,0fh

        mov rbx, r11 ; pass new rip in rbx
        mov r11, rsp ; restore stack in r11
		add	r11, 10h 

		mov rbp, r11 ; pass stack frame in rbp

		API_NUM_ARGS = 8
		sub	rsp, (API_NUM_ARGS + 1 + 2 + 10) * 8

		;
		; spill parameters in registers onto the stack
		;

		mov	qword ptr [r11+1*8], rcx
		mov	qword ptr [r11+2*8], rdx
		mov	qword ptr [r11+3*8], r8
		mov	qword ptr [r11+4*8], r9

		
		;
		; parms[1] = syscall_index
		; parms[2] = pointer to system service arguments on stack
		; parms[0] = API_INVOKE_SYSCALL (from SbieLowData)
		;

		lea	rcx, [rsp+13*8]			; rcx -> parms
		
		mov	qword ptr [rcx+1*8], r10	; syscall index
		mov	r10, rax			; r10 -> SbieLowData

		lea	r11, [r11+8]
		mov	qword ptr [rcx+2*8], r11	; stack address
		
		mov	eax, dword ptr [r10+3*8+4]	; API_INVOKE_SYSCALL
		mov	qword ptr [rcx+0*8], rax
		
		;
		; the 64-bit kernel function for NtDeviceIoControlFile
		; expects the 64-bit IoStatusBlock argument to contain a
		; 32-bit pointer to a 32-bit IoStatusBlock structure.
		; we can allocate the 32-bit structure on the stack
		; because the stack is 32-bits in a 32-bit program
		;
		
		lea	rax, [rsp+12*8]		; rax -> IoStatusBlock32
		lea	rdx, [rsp+10*8]		; rdx -> MyIoStatusBlock64
		mov	qword ptr [rdx], rax
		
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
		
		mov	qword ptr [rsp+4*8], rdx	; MyIoStatusBlock64

		mov	eax, [r10+3*8+0]		; API_SBIEDRV_CTLCODE
		mov	qword ptr [rsp+5*8], rax
		
		mov	qword ptr [rsp+6*8], rcx	; parms
		
		mov	eax, (API_NUM_ARGS * 8)
		mov	qword ptr [rsp+7*8], rax
		
		mov	rcx, qword ptr [r10+2*8]	; file handle

		xor	rdx, rdx
		mov	r8, rdx
		mov	r9, rdx
		
		mov	qword ptr [rsp+8*8], rdx
		mov	qword ptr [rsp+9*8], rdx

		lea	r10, [r10+80h] ; r10 -> SbieLow.NtDeviceIoControlFile

		call	r10
		add	rsp, (API_NUM_ARGS + 1 + 2 + 10) * 8

		pop rbx
		pop rbp
        ret
		
myService ENDP
;----------------------------------------------------------------------------

else 		; 32-bit

		mov	edx, 0		; edx -> SbieLowData
		push	eax		; save syscall index and param count
		push	ebp
		mov	ebp, esp
		
		;
		; make room for several qwords on the stack:
		;     IO_STATUS_BLOCK MyIoStatusBlock;
		;     __declspec(align(8)) ULONG64 parms[API_NUM_ARGS];
		; align stack on 8-bytes for call to NtDeviceIoControlFile
		;
		; keep API_NUM_ARGS in sync with core/drv/api_defs.h

		API_NUM_ARGS = 8
		
		sub	esp, (API_NUM_ARGS + 1) * 8 + 7
		and	esp, 0FFFFFFF8h
		
		;
		; parms[1] = syscall_index
		; parms[2] = pointer to system service arguments on stack
		; parms[0] = API_INVOKE_SYSCALL (from SbieLowData)
		;

		and	eax, 0FFFh	; keep just service index number
		mov	dword ptr [esp+2*8], eax

		lea	eax, [ebp+4*3]	; eax -> orig arg 1 on stack
		mov	dword ptr [esp+3*8], eax
		
		mov	eax, [edx+3*8+4] ; eax = SbieLow.api_invoke_syscall
		mov	dword ptr [esp+1*8], eax

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
		
		mov	ecx, esp	; ecx = address of IoStatusBlock
		lea	eax, [esp+1*8]	; eax = address of parms
		
		push	0
		push	0
		push	(API_NUM_ARGS * 8)
		push	eax		; push address of parms
		push	[edx+3*8+0]	; push API_SBIEDRV_CTLCODE
		push	ecx		; push address of IoStatusBlock
		push	0
		push	0
		push	0
		push	[edx+2*8]	; push SbieLow.api_device_handle

		lea	eax, [edx+80h] ; eax -> SbieLow.NtDeviceIoControlFile
		call	eax
		
		;
		; restore stack and return to caller.  we can't do "ret n"
		; because n is not a known constant, so we manually pop the
		; arguments off the stack and jump to the return address
		;
		
		mov	esp, ebp
		pop	ebp
		
		pop	ecx		; restore original eax
		shr	ecx, 24		; keep just param count
		add	ecx, 4
		add	esp, ecx	; pop arguments for service call
		neg	ecx
		mov	ecx, [esp+ecx]
		jmp	ecx		; return to caller
endif 		; 32-bit or 64-bit


;----------------------------------------------------------------------------
; Inject Data Area for our RtlFindActivationContextSectionString
;----------------------------------------------------------------------------

		
InjectData		struct		; keep in sync with inject.c
			dq	?
LdrLoadDll		dq	?
LdrGetProcAddr		dq	?
NtRaiseHardError	dq	?
RtlFindActCtx		dq	?
RtlFindActCtx_Protect	dd	?
RtlFindActCtx_Bytes 	db	12 dup (?)
KernelDll_Unicode	dq	2 dup (?)
SbieDll_Unicode		dq	2 dup (?)
ModuleHandle		dq	?
SbieDllOrdinal1		dq	?
InjectData		ends


;----------------------------------------------------------------------------
; 32-bit RtlFindActivationContextSectionString detour code
;----------------------------------------------------------------------------

_RtlFindActivationContextSectionString:
ifdef _WIN64	; 64-bit

	;
	; we need the 32-bit version of RtlFindActivationContextSectionString
	; on both 32-bit and 64-bit versions of SbieLow, because of wow64
	;
	; it will not compile correctly as assembly on 64-bit, so we simply
	; dump the machine code bytes here
	;
	; the code bytes below are copied by InitInjectWow64 (see inject.c),
	; make sure to keep the length in that function in sync with the code
	;
		
	db	0BAh, 0, 0, 0, 0				; mov	edx, 0
	db	056h							; push 	esi
	db	08Bh, 0F2h						; mov	esi, edx
	;
	db	08Bh, 046h, 020h				; mov	eax,dword ptr [esi+20h]
	db	08Ah, 056h, 02Ch        		; mov	dl,byte ptr [esi+2Ch]
	db	088h, 010h						; mov	byte ptr [eax],dl
	db	08Bh, 056h, 02Dh				; mov	edx,dword ptr [esi+2Dh]
	db	089h, 050h, 001h				; mov	dword ptr [eax+1],edx
	;;;;;new code for retry loop
	db  0b9h, 010h, 0, 0, 0				; mov ecx, 10h
	;;;;;; 
	;for(i = 0; i < 0x10; i++) {
	    db  051h						; push ecx
	    db	08Dh, 046h, 058h			; lea	eax,[esi+58h]
	    db	050h						; push	eax
	    db	08Dh, 046h, 038h			; lea	eax,[esi+38h]
	    db	050h						; push	eax
	    db	06Ah, 000h					; push	0
	    db	06Ah, 000h					; push	0
	    db	0FFh, 056h, 008h			; call	dword ptr [esi+8]
	    ;;;;;
	    db  059h						; pop ecx
	    ;;;;;
	    db	085h, 0C0h					; test	eax,eax
	    db  074h, 004h					; jz good
	;}
	db  0e2h, 0e9h						; loop retry
	db  0ebh, 034h						; jmp error
	;

	db	08Dh, 046h, 058h				; lea	eax,[esi+58h]
	db	050h							; push	eax
	db	08Dh, 046h, 048h				; lea	eax,[esi+48h]
	db	050h							; push	eax
	db	06Ah, 000h						; push	0
	db	06Ah, 000h						; push	0
	db	0FFh, 056h, 008h				; call	dword ptr [esi+8]
	;
	db	085h, 0C0h						; test	eax,eax
	db	075h, 021h						; jnz	error
	;
	db	08Dh, 046h, 060h				; lea	eax,[esi+60h]
	db	050h							; push	eax
	db	06Ah, 001h						; push	1
	db	06Ah, 000h						; push	0
	db	0FFh, 076h, 058h				; push	dword ptr [esi+58h]
	db	0FFh, 056h, 010h				; call	dword ptr [esi+10h]
	;
	db	085h, 0C0h						; test	eax,eax
	db	075h, 00Fh						; jnz	error
	;
	db	08Bh, 0C6h						; mov	eax, esi
	db	087h, 044h, 024h, 008h			; xchg	eax, dword ptr [esp+8]
	db	089h, 046h, 008h				; mov	dword ptr [esi+8],eax
	db	08Bh, 0C6h						; mov	eax, esi
	db	05Eh							; pop	esi
	db	0FFh, 060h, 060h				; jmp	dword ptr [eax+60h]
	;
	db	050h							; push	eax
	db	08Dh, 056h, 048h				; lea	edx,[esi+48h]
	db	089h, 056h, 008h				; mov	dword ptr [esi+8],edx
	db	08dh, 056h, 010h				; lea	edx,[esi+10h]
	db	052h							; push	edx
	db	06Ah, 001h						; push	1
	db	08Dh, 056h, 008h				; lea	edx,[esi+8]
	db	052h							; push	edx
	db	06Ah, 001h						; push	1
	db	06Ah, 001h						; push	1
	db	068h, 042h, 001h, 000h, 0D0h	; push 0D0000142h
	db	0FFh, 056h, 018h				; call	dword ptr [esi+18h]
	db	058h							; pop	eax
	db	05Eh							; pop	esi
	db	0C2h, 14h, 00h					; ret	14h

else 		; 32-bit

		mov	edx, 0		; edx -> inject data area
		
		push	esi
		mov	esi, edx	; esi -> inject data area
		
		mov	eax, dword ptr [esi].InjectData.RtlFindActCtx
		mov	dl, byte ptr [esi].InjectData.RtlFindActCtx_Bytes
		mov	byte ptr [eax], dl
		mov	edx, dword ptr [esi].InjectData.RtlFindActCtx_Bytes+1
		mov	dword ptr [eax+1], edx
		
		;
		; call LdrLoadDll for kernel32
		;
		mov ecx, 10h ;number of retries
LdrLoadDll_Retry:	
			push ecx
			lea	eax, [esi].InjectData.ModuleHandle
			push	eax
			lea	eax, [esi].InjectData.KernelDll_Unicode
			push	eax
			push	0
			push	0
			call	dword ptr [esi].InjectData.LdrLoadDll
			pop ecx
			test	eax, eax
			jz    LdrLoadDll_Good	
		loop	 LdrLoadDll_Retry
		  ; retry failed 16 times: raise error
		jmp	RtlFindActivationContextSectionStringError
LdrLoadDll_Good:
		;
		; call LdrLoadDll for sbiedll
		;

		lea	eax, [esi].InjectData.ModuleHandle
		push	eax
		lea	eax, [esi].InjectData.SbieDll_Unicode
		push	eax
		push	0
		push	0
		call	dword ptr [esi].InjectData.LdrLoadDll

		test	eax, eax
		jnz	RtlFindActivationContextSectionStringError

		;
		; call LdrGetProcedureAddress for sbiedll ordinal 1,
		; which forces ntdll to initialize sbiedll
		;
		
		lea	eax, [esi].InjectData.SbieDllOrdinal1
		push	eax
		push	1
		push	0
		push	dword ptr [esi].InjectData.ModuleHandle
		call	dword ptr [esi].InjectData.LdrGetProcAddr
		
		test	eax, eax
		jnz	RtlFindActivationContextSectionStringError

		;
		; pass control to ordinal 1, which will free the inject
		; data area, and pass control to the original function
		; RtlFindActivationContextSectionString
		;
		; note that we need to pass the address of the inject
		; data area to ordinal 1, which we do by overwriting the
		; first argument.  the original argument is saved in
		; the inject data area
		;
		
		mov	eax, esi
		xchg	eax, dword ptr [esp+4*2]
		mov	dword ptr [esi].InjectData.LdrLoadDll, eax
		mov	eax, esi
		pop	esi
		jmp	dword ptr [eax].InjectData.SbieDllOrdinal1
		
		;
		; display error message, invoke NtRaiseHardError(
		;	NTSTATUS   ntstatus_message_code,
		;	ULONG      number_of_parameters_in_list,
		;	ULONG      mask_of_strings_in_list,
		;	ULONG_PTR *list_of_pointers_to_parameters,
		;	ULONG      response_buttons,
		;	ULONG     *out_response)
		;
		
RtlFindActivationContextSectionStringError:
		
		STATUS_DLL_INIT_FAILED  = 0C0000142h
		FORCE_ERROR_MESSAGE_BOX = 010000000h
		
    	push	eax		; save ntstatus
    		
    	lea	edx, [esi].InjectData.SbieDll_Unicode
    	mov	dword ptr [esi].InjectData.LdrLoadDll, edx
		
    	lea	edx, [esi].InjectData.LdrGetProcAddr
    	push	edx		; out_response
    	push	1		; response_buttons - ERROR_OK
    	lea	edx, [esi].InjectData.LdrLoadDll
    	push	edx		; list_of_pointers_to_parameters
		push	1		; mask_of_strings_in_list
		push	1		; number_of_parameters_in_list
		push	(STATUS_DLL_INIT_FAILED or FORCE_ERROR_MESSAGE_BOX)
		call	dword ptr [esi].InjectData.NtRaiseHardError
		
		pop	eax		; pop error ntstatus to return
		pop	esi
		ret	14h		; return to caller with error

endif 		; 32-bit or 64-bit


;----------------------------------------------------------------------------
; 64-bit RtlFindActivationContextSectionString detour code
;----------------------------------------------------------------------------


ifdef _WIN64	; 64-bit
dq 0h ;inject data area address
 _RtlFindActivationContextSectionString64:
		mov	rax, qword ptr [$-8]		; rax -> inject data area
		
		push	rsi		; save rsi, and align stack
		sub	rsp, 8*8	; set up local stack
		
		mov	qword ptr [rsp+4*8], rcx
		mov	qword ptr [rsp+5*8], rdx
		mov	qword ptr [rsp+6*8], r8
		mov	qword ptr [rsp+7*8], r9

		mov  	rsi, rax  	; rsi -> inject data area
		
		mov	rax, qword ptr [rsi].InjectData.RtlFindActCtx

	;replace 12bytes	
		mov	rdx, qword ptr [rsi].InjectData.RtlFindActCtx_Bytes
		mov	qword ptr [rax], rdx
		mov	edx, dword ptr [rsi].InjectData.RtlFindActCtx_Bytes + 8
		mov	dword ptr [rax+8], edx
		;
		; call LdrLoadDll for kernel32
		;
	    ;; retry loop 
		 mov  qword ptr [rsi].InjectData.RtlFindActCtx_Bytes, rbx
         mov     rbx, 010h

LdrLoadRetry:	
			xor	rcx, rcx
			xor	rdx, rdx
			lea	r8, [rsi].InjectData.KernelDll_Unicode
			lea	r9, [rsi].InjectData.ModuleHandle
			;cmp rbx,1
			;jnz LdrTestLoop
			call	qword ptr [rsi].InjectData.LdrLoadDll
			test	eax, eax
			jz LdrLoadGood
;LdrTestLoop:
			dec rbx
            test rbx, rbx
			jnz LdrLoadRetry ;loop LdrLoadRetry
			jmp	RtlFindActivationContextSectionStringError
		;
		; call LdrLoadDll for sbiedll
		;
LdrLoadGood:
		mov rbx, qword ptr [rsi].InjectData.RtlFindActCtx_Bytes
		xor	rcx, rcx
		xor	rdx, rdx
		lea	r8, [rsi].InjectData.SbieDll_Unicode
		lea	r9, [rsi].InjectData.ModuleHandle
		call	qword ptr [rsi].InjectData.LdrLoadDll

		test	eax, eax
		jnz	RtlFindActivationContextSectionStringError
		
		;
		; call LdrGetProcedureAddress for sbiedll ordinal 1,
		; which forces ntdll to initialize sbiedll
		;
		
		mov	rcx, qword ptr [rsi].InjectData.ModuleHandle
		xor	rdx, rdx
		xor	r8, r8
		inc	r8
		lea	r9, [rsi].InjectData.SbieDllOrdinal1
		call	qword ptr [rsi].InjectData.LdrGetProcAddr

		test	eax, eax
		jnz	RtlFindActivationContextSectionStringError

		;
		; pass control to ordinal 1, which will free the inject
		; data area, and pass control to the original function
		; RtlFindActivationContextSectionString
		;
		; note that we need to pass the address of the inject
		; data area to ordinal 1, which we do by overwriting the
		; first argument.  the original argument is saved in
		; the inject data area
		;

		mov	rax, qword ptr [rsp+4*8]
		mov	qword ptr [rsi].InjectData.LdrLoadDll, rax
		mov	rcx, rsi
		mov	rdx, qword ptr [rsp+5*8]
		mov	r8, qword ptr [rsp+6*8]
		mov	r9, qword ptr [rsp+7*8]
		
		add	rsp, 8*8
		pop	rsi
		jmp	qword ptr [rcx].InjectData.SbieDllOrdinal1

		;
		; display error message, invoke NtRaiseHardError(
		;	NTSTATUS   ntstatus_message_code,
		;	ULONG      number_of_parameters_in_list,
		;	ULONG      mask_of_strings_in_list,
		;	ULONG_PTR *list_of_pointers_to_parameters,
		;	ULONG      response_buttons,
		;	ULONG     *out_response)
		;

RtlFindActivationContextSectionStringError:
		
		STATUS_DLL_INIT_FAILED  = 0C0000142h
		FORCE_ERROR_MESSAGE_BOX = 010000000h
		
    		mov	qword ptr [rsp+7*8], rax	; save ntstatus
    		
    		mov	ecx, \		; ntstatus_message_code
    			(STATUS_DLL_INIT_FAILED or FORCE_ERROR_MESSAGE_BOX)

		xor	rdx, rdx	; number_of_parameters_in_list
		inc	rdx

		mov	r8, rdx		; mask_of_strings_in_list

    		lea	r9, \		; list_of_pointers_to_parameters
    			[esi].InjectData.LdrLoadDll
    		lea	rax, [rsi].InjectData.SbieDll_Unicode
		mov	qword ptr [r9], rax
		
		mov	\		; response_buttons - ERROR_OK
			qword ptr [rsp+4*8], rdx
		
		lea	rax, [rsi].InjectData.LdrGetProcAddr
		mov	\		; out_response
			qword ptr [rsp+5*8], rax
		
		call	qword ptr [rsi].InjectData.NtRaiseHardError
		
		mov	rcx, qword ptr [rsp+7*8]	; restore ntstatus
		add	rsp, 8*8
		pop	rsi
		ret			; return to caller with error
		
endif 		; 64-bit


;----------------------------------------------------------------------------
; Parameters stored by SbieSvc
;----------------------------------------------------------------------------

SbieLowData	LABEL	QWORD

		dq	48 dup (0)	; at least sizeof(SBIELOW_DATA)


;----------------------------------------------------------------------------
; Tail Signature
;----------------------------------------------------------------------------
; Replaced Tail signature in zzzz section with the two symbol locations 
; _Start and SbiLowData used by SbieSvc

		.code	zzzz
	
		dq	_Start
		dq   SbieLowData
	
;----------------------------------------------------------------------------
end
