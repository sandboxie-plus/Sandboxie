/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2022 David Xanatos, xanasoft.com
 *
 * This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifdef _WIN64

//
// we need the 32-bit version of RtlFindActivationContextSectionString
// on both 32-bit and 64-bit versions of SbieLow, because of wow64
//
// it will not compile correctly as assembly on 64-bit, so we simply
// dump the machine code bytes here
//

UCHAR SbieDll_ShellCode_x86[] = 
{
	0xBA, 0, 0, 0, 0,				// mov	edx, 0						; edx -> inject data area

	//0xCC,							// int3

	0x56,							// push 	esi
	0x8B, 0xF2,						// mov	esi, edx					; esi -> inject data area

									//
									// restore bytes
									//

	0x8B, 0x46, 0x20,				// mov	eax,dword ptr [esi+20h]		; ... [esi].InjectData.RtlFindActCtx
	0x8A, 0x56, 0x2C,        		// mov	dl,byte ptr [esi+2Ch]		; ... [esi].InjectData.RtlFindActCtx_Bytes
	0x88, 0x10,						// mov	byte ptr [eax],dl
	0x8B, 0x56, 0x2D,				// mov	edx,dword ptr [esi+2Dh]		; ... [esi].InjectData.RtlFindActCtx_Bytes+1
	0x89, 0x50, 0x01,				// mov	dword ptr [eax+1],edx

									//
									// call LdrLoadDll for kernel32
									//
	
	0xb9, 0x10, 0, 0, 0,			// mov ecx, 10h
								// LdrLoadDll_Retry:
	//for(i = 0; i < 0x10; i++) {
	    0x51,						// push ecx
	    0x8D, 0x46, 0x60,			// lea	eax,[esi+60h]				; ... [esi].InjectData.ModuleHandle
	    0x50,						// push	eax
	    0x8D, 0x46, 0x40,			// lea	eax,[esi+40h]				; ... [esi].InjectData.KernelDll_Unicode
	    0x50,						// push	eax
	    0x6A, 0x00,					// push	0
	    0x6A, 0x00,					// push	0
	    0xFF, 0x56, 0x08,			// call	dword ptr [esi+8]			; ... [esi].InjectData.LdrLoadDll
	    0x59,						// pop ecx
	    0x85, 0xC0,					// test	eax,eax
	    0x74, 0x04,					// jz LdrLoadDll_Good
	//}
	0xE2, 0xE9,						// loop LdrLoadDll_Retry
	0xEB, 0x34,						// jmp error
								// LdrLoadDll_Good:
	
									//
									// call LdrLoadDll for sbiedll
									//

	0x8D, 0x46, 0x60,				// lea	eax,[esi+60h]			; ... [esi].InjectData.ModuleHandle
	0x50,							// push	eax
	0x8D, 0x46, 0x50,				// lea	eax,[esi+50h]			; ... [esi].InjectData.SbieDll_Unicode
	0x50,							// push	eax
	0x6A, 0x00,						// push	0
	0x6A, 0x00,						// push	0
	0xFF, 0x56, 0x08,				// call	dword ptr [esi+8]		; ... [esi].InjectData.LdrLoadDll
	
	0x85, 0xC0,						// test	eax,eax
	0x75, 0x21,						// jnz	RtlFindActivationContextSectionStringError
	
									//
									// call LdrGetProcedureAddress for sbiedll ordinal 1,
									// which forces ntdll to initialize sbiedll
									//

	0x8D, 0x46, 0x68,				// lea	eax,[esi+68h]		; ... [esi].InjectData.SbieDllOrdinal1
	0x50,							// push	eax
	0x6A, 0x01,						// push	1
	0x6A, 0x00,						// push	0
	0xFF, 0x76, 0x60,				// push	dword ptr [esi+60h]	; ... [esi].InjectData.ModuleHandle
	0xFF, 0x56, 0x10,				// call	dword ptr [esi+10h] ; ... [esi].InjectData.LdrGetProcAddr
	
	0x85, 0xC0,						// test	eax,eax
	0x75, 0x0F,						// jnz	RtlFindActivationContextSectionStringError

									//
									// pass control to ordinal 1 ...
									//

	0x8B, 0xC6,						// mov	eax, esi
	0x87, 0x44, 0x24, 0x08,			// xchg	eax, dword ptr [esp+8]
	0x89, 0x46, 0x08,				// mov	dword ptr [esi+8],eax	; ... [esi].InjectData.LdrLoadDll ...
	0x8B, 0xC6,						// mov	eax, esi
	0x5E,							// pop	esi
	0xFF, 0x60, 0x68,				// jmp	dword ptr [eax+68h]		; ... [eax].InjectData.SbieDllOrdinal1
	
									//
									// display error message ...
									//

								// RtlFindActivationContextSectionStringError:
	0x50,							// push	eax

	0x8D, 0x56, 0x50,				// lea	edx,[esi+50h]			; ... [esi].InjectData.SbieDll_Unicode
	0x89, 0x56, 0x08,				// mov	dword ptr [esi+8],edx	; ... [esi].InjectData.LdrLoadDll ... 

	0x8d, 0x56, 0x10,				// lea	edx,[esi+10h]			; ... [esi].InjectData.LdrGetProcAddr
	0x52,							// push	edx
	0x6A, 0x01,						// push	1
	0x8D, 0x56, 0x08,				// lea	edx,[esi+8]				; ... [esi].InjectData.LdrLoadDll
	0x52,							// push	edx
	0x6A, 0x01,						// push	1
	0x6A, 0x01,						// push	1
	0x68, 0x42, 0x01, 0x00, 0xD0,	// push 0D0000142h
	0xFF, 0x56, 0x18,				// call	dword ptr [esi+18h]		; ... [esi].InjectData.NtRaiseHardError

	0x58,							// pop	eax
	0x5E,							// pop	esi
	0xC2, 0x14, 0					// ret	14h
};

#endif

#ifdef _M_ARM64

//
// we need the x64 version of RtlFindActivationContextSectionString
//
// it will not compile correctly as assembly on arm64, so we simply
// dump the machine code bytes here
//

//UCHAR SbieDll_ShellCode_x64[] = 
//{
//
//	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// inject data area address
//
//	0x48, 0x8B, 0x05, 0xF1, 0xFF, 0xFF, 0xFF,    // mov  rax, qword ptr [rip - 0xf]		; rax -> inject data area
//
//	// 0xCC, 						// int3
//
//	0x56,							// push rsi											; save rsi, and align stack
//	0x48, 0x83, 0xEC, 0x40,         // sub  rsp, 0x40									; set up local stack
//
//	0x48, 0x89, 0x4C, 0x24, 0x20,   // mov  qword ptr [rsp + 0x20], rcx
//	0x48, 0x89, 0x54, 0x24, 0x28,   // mov  qword ptr [rsp + 0x28], rdx
//	0x4C, 0x89, 0x44, 0x24, 0x30,   // mov  qword ptr [rsp + 0x30], r8
//	0x4C, 0x89, 0x4C, 0x24, 0x38,   // mov  qword ptr [rsp + 0x38], r9
//
//	0x48, 0x8B, 0xF0,               // mov  rsi, rax										; rsi -> inject data area
//
//	0x48, 0x8B, 0x46, 0x20,         // mov  rax, qword ptr [rsi + 0x20]					; ... [rsi].InjectData.RtlFindActCtx
//
//									// replace 12bytes
//	0x48, 0x8B, 0x56, 0x2C,         // mov  rdx, qword ptr [rsi + 0x2c]					; ... [rsi].InjectData.RtlFindActCtx_Bytes
//	0x48, 0x89, 0x10,               // mov  qword ptr [rax], rdx
//	0x8B, 0x56, 0x34,               // mov  edx, dword ptr [rsi + 0x34]					; ... [rsi].InjectData.RtlFindActCtx_Bytes + 8
//	0x89, 0x50, 0x08,               // mov  dword ptr [rax + 8], edx
//
//									//
//									// call LdrLoadDll for kernel32
//									//
//									//// retry loop 
//	0x48, 0x89, 0x5E, 0x2C,         // mov  qword ptr [rsi + 0x2c], rbx					; ... [rsi].InjectData.RtlFindActCtx_Bytes ...
//	0x48, 0xC7, 0xC3, 0x10, 0x00, 0x00, 0x00,    // mov  rbx, 0x10
//
//									// LdrLoadRetry:
//	0x48, 0x33, 0xC9,               // xor  rcx, rcx
//	0x48, 0x33, 0xD2,               // xor  rdx, rdx
//	0x4C, 0x8D, 0x46, 0x40,			// lea  r8, [rsi + 0x40]								; ... [rsi].InjectData.KernelDll_Unicode
//	0x4C, 0x8D, 0x4E, 0x60,         // lea  r9, [rsi + 0x60]								; ... [rsi].InjectData.ModuleHandle
//									//cmp rbx,1
//									//jnz LdrTestLoop
//	0xFF, 0x56, 0x08,               // call qword ptr [rsi + 8]							; ... [rsi].InjectData.LdrLoadDll
//	0x85, 0xC0,						// test eax, eax
//	0x74, 0x0A,						// je   0x5e											; LdrLoadGood
//									////LdrTestLoop:
//	0x48, 0xFF, 0xCB,               // dec  rbx
//	0x48, 0x85, 0xDB,               // test rbx, rbx
//	0x75, 0xE3,						// jne  0x3f											; ;loop LdrLoadRetry
//	0xEB, 0x54,						// jmp  0xb2											; RtlFindActivationContextSectionStringError
//
//									//
//									// call LdrLoadDll for sbiedll
//									//
//									// LdrLoadGood:
//	0x48, 0x8B, 0x5E, 0x2C,         // mov  rbx, qword ptr [rsi + 0x2c]					; ... [rsi].InjectData.RtlFindActCtx_Bytes
//	0x48, 0x33, 0xC9,               // xor  rcx, rcx
//	0x48, 0x33, 0xD2,               // xor  rdx, rdx
//	0x4C, 0x8D, 0x46, 0x50,         // lea  r8, [rsi + 0x50]								; ... [rsi].InjectData.SbieDll_Unicode
//	0x4C, 0x8D, 0x4E, 0x60,         // lea  r9, [rsi + 0x60]								; ... [rsi].InjectData.ModuleHandle
//	0xFF, 0x56, 0x08,               // call qword ptr [rsi + 8]							; ... [rsi].InjectData.LdrLoadDll
//
//	0x85, 0xC0,						// test eax, eax
//	0x75, 0x3B,						// jne  0xb2											; RtlFindActivationContextSectionStringError
//
//									//
//									// call LdrGetProcedureAddress for sbiedll ordinal 1,
//									// which forces ntdll to initialize sbiedll
//									//
//
//	0x48, 0x8B, 0x4E, 0x60,         // mov  rcx, qword ptr [rsi + 0x60]					; ... [rsi].InjectData.ModuleHandle
//	0x48, 0x33, 0xD2,               // xor  rdx, rdx
//	0x4D, 0x33, 0xC0,               // xor  r8, r8
//	0x49, 0xFF, 0xC0,               // inc  r8
//	0x4C, 0x8D, 0x4E, 0x68,         // lea  r9, [rsi + 0x68]								; ... [rsi].InjectData.SbieDllOrdinal1
//	0xFF, 0x56, 0x10,               // call qword ptr [rsi + 0x10]						; ... [rsi].InjectData.LdrGetProcAddr
//
//	0x85, 0xC0,						// test eax, eax
//	0x75, 0x23,						// jne  0xb2											; RtlFindActivationContextSectionStringError
//	
//									//
//									// pass control to ordinal 1, which will free the inject
//									// data area, and pass control to the original function
//									// RtlFindActivationContextSectionString
//									//
//									// note that we need to pass the address of the inject
//									// data area to ordinal 1, which we do by overwriting the
//									// first argument.  the original argument is saved in
//									// the inject data area
//									//
//
//	0x48, 0x8B, 0x44, 0x24, 0x20,   // mov  rax, qword ptr [rsp + 0x20]
//	0x48, 0x89, 0x46, 0x08,         // mov  qword ptr [rsi + 8], rax						; ... [rsi].InjectData.LdrLoadDll ...
//	0x48, 0x8B, 0xCE,               // mov  rcx, rsi
//	0x48, 0x8B, 0x54, 0x24, 0x28,   // mov  rdx, qword ptr [rsp + 0x28]
//	0x4C, 0x8B, 0x44, 0x24, 0x30,   // mov  r8, qword ptr [rsp + 0x30]
//	0x4C, 0x8B, 0x4C, 0x24, 0x38,   // mov  r9, qword ptr [rsp + 0x38]
//
//	0x48, 0x83, 0xC4, 0x40,			// add  rsp, 0x40
//	0x5E,							// pop  rsi
//	0xFF, 0x61, 0x68,               // jmp  qword ptr [rcx + 0x68]						; [rcx].InjectData.SbieDllOrdinal1
//
//									//
//									// display error message, invoke NtRaiseHardError(
//									//	NTSTATUS   ntstatus_message_code,
//									//	ULONG      number_of_parameters_in_list,
//									//	ULONG      mask_of_strings_in_list,
//									//	ULONG_PTR *list_of_pointers_to_parameters,
//									//	ULONG      response_buttons,
//									//	ULONG     *out_response)
//									//
//
//									//RtlFindActivationContextSectionStringError:
//
//	0x48, 0x89, 0x44, 0x24, 0x38,   // mov  qword ptr [rsp + 0x38], rax					; save ntstatus
//	0xB9, 0x42, 0x01, 0x00, 0xD0,   // mov  ecx, 0xd0000142								; ntstatus_message_code
//	0x48, 0x33, 0xD2,               // xor  rdx, rdx										; number_of_parameters_in_list
//	0x48, 0xFF, 0xC2,               // inc  rdx
//	0x4C, 0x8B, 0xC2,               // mov  r8, rdx										; mask_of_strings_in_list
//	0x67, 0x4C, 0x8D, 0x4E, 0x08,   // lea  r9, [esi + 8]								; ... [esi].InjectData.LdrLoadDll ; list_of_pointers_to_parameters
//	0x48, 0x8D, 0x46, 0x50,         // lea  rax, [rsi + 0x50]							; ... [rsi].InjectData.SbieDll_Unicode
//	0x49, 0x89, 0x01,               // mov  qword ptr [r9], rax
//	0x48, 0x89, 0x54, 0x24, 0x20,   // mov  qword ptr [rsp + 0x20], rdx					; response_buttons - ERROR_OK
//	0x48, 0x8D, 0x46, 0x10,         // lea  rax, [rsi + 0x10]							; ... [rsi].InjectData.LdrGetProcAddr
//	0x48, 0x89, 0x44, 0x24, 0x28,   // mov  qword ptr [rsp + 0x28], rax					; out_response
//	0xFF, 0x56, 0x18,               // call qword ptr [rsi + 0x18]						; ... [rsi].InjectData.NtRaiseHardError
//	0x48, 0x8B, 0x4C, 0x24, 0x38,   // mov  rcx, qword ptr [rsp + 0x38]					; restore ntstatus
//	0x48, 0x83, 0xC4, 0x40,			// add  rsp, 0x40
//	0x5E,							// pop  rsi
//	0xC3							// ret												; return to caller with error
//};

#endif