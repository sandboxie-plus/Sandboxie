;----------------------------------------------------------------------------
; __except_handler3 support routine is not longer available on ntdll.lib 
;
; this is a copy of __except_handler3 procedure and required procedures from 
; the c:\WinDDK\7600.16385.1\lib\wnet\i386\ntdll.lib 
;
;----------------------------------------------------------------------------

.386

_TEXT         segment use32 para public 'CODE'


public  __except_handler3

;__except_handler3        proc    near

  push        esi
  inc         ebx
  xor         dh,byte ptr [eax]
  pop         eax
  inc         ebx
  xor         byte ptr [eax],dh
__except_handler3:
  push        ebp
  mov         ebp,esp
  sub         esp,8
  push        ebx
  push        esi
  push        edi
  push        ebp
  cld
  mov         ebx,dword ptr [ebp+0Ch]
  mov         eax,dword ptr [ebp+8]
  test        dword ptr [eax+4],6
  nop ; 00
  jne         _lh_unwinding
  mov         dword ptr [ebp-8],eax
  mov         eax,dword ptr [ebp+10h]
  mov         dword ptr [ebp-4],eax
  lea         eax,[ebp-8]
  mov         dword ptr [ebx-4],eax
  mov         esi,dword ptr [ebx+0Ch]
  mov         edi,dword ptr [ebx+8]
  push        ebx
  call        __ValidateEH3RN
  add         esp,4
  or          eax,eax
  je          _lh_abort
_lh_top:
  cmp         esi,0FFFFFFFFh
  je          _lh_bagit
  lea         ecx,[esi+esi*2]
  mov         eax,dword ptr [edi+ecx*4+4]
  or          eax,eax
  je          _lh_continue
  push        esi
  push        ebp
  lea         ebp,[ebx+10h]
  xor         ebx,ebx
  xor         ecx,ecx
  xor         edx,edx
  xor         esi,esi
  xor         edi,edi
  call        eax
  pop         ebp
  pop         esi
  mov         ebx,dword ptr [ebp+0Ch]
  or          eax,eax
  je          _lh_continue
  js          _lh_dismiss
  mov         edi,dword ptr [ebx+8]
  push        ebx
  call        __global_unwind2
  add         esp,4
  lea         ebp,[ebx+10h]
  push        esi
  push        ebx
  call        __local_unwind2
  add         esp,8
  lea         ecx,[esi+esi*2]
  push        1
  mov         eax,dword ptr [edi+ecx*4+8]
  call        __NLG_Notify
  mov         eax,dword ptr [edi+ecx*4]
  mov         dword ptr [ebx+0Ch],eax
  mov         eax,dword ptr [edi+ecx*4+8]
  xor         ebx,ebx
  xor         ecx,ecx
  xor         edx,edx
  xor         esi,esi
  xor         edi,edi
  call        eax
_lh_continue:
  mov         edi,dword ptr [ebx+8]
  lea         ecx,[esi+esi*2]
  mov         esi,dword ptr [edi+ecx*4]
  jmp         _lh_top
_lh_dismiss:
  mov         eax,0
  jmp         _lh_return
_lh_abort:
  mov         eax,dword ptr [ebp+8]
  or          dword ptr [eax+4],8
_lh_bagit:
  mov         eax,1
  jmp         _lh_return
_lh_unwinding:
  push        ebp
  lea         ebp,[ebx+10h]
  push        0FFFFFFFFh
  push        ebx
  call        __local_unwind2
  add         esp,8
  pop         ebp
  mov         eax,1
_lh_return:
  pop         ebp
  pop         edi
  pop         esi
  pop         ebx
  mov         esp,ebp
  pop         ebp
  ret
__seh_longjmp_unwind@4:
  push        ebp
  mov         ecx,dword ptr [esp+8]
  mov         ebp,dword ptr [ecx]
  mov         eax,dword ptr [ecx+1Ch]
  push        eax
  mov         eax,dword ptr [ecx+18h]
  push        eax
  call        __local_unwind2
  add         esp,8
  pop         ebp
  ret         4

;__except_handler3        endp

EXTERN _RtlUnwind@16 	  : PROC

;__global_unwind2        proc    near

__global_unwind2:
  push        ebp
  mov         ebp,esp
  push        ebx
  push        esi
  push        edi
  push        ebp
  push        0
  push        0
  push        offset _gu_return
  push        dword ptr [ebp+8]
  ;call        P_RtlUnwind
  call        _RtlUnwind@16 ; call it from ntdll.lib
_gu_return:
  pop         ebp
  pop         edi
  pop         esi
  pop         ebx
  mov         esp,ebp
  pop         ebp
  ret
__unwind_handler:
  mov         ecx,dword ptr [esp+4]
  test        dword ptr [ecx+4],6
  nop ; 00
  mov         eax,1
  je          _uh_return
  mov         eax,dword ptr [esp+14h]
  push        ebp
  mov         ebp,dword ptr [eax+10h]
  mov         edx,dword ptr [eax+28h]
  push        edx
  mov         edx,dword ptr [eax+24h]
  push        edx
  call        __local_unwind2
 add         esp,8
  pop         ebp
  mov         eax,dword ptr [esp+8]
  mov         edx,dword ptr [esp+10h]
  mov         dword ptr [edx],eax
  mov         eax,3
_uh_return:
  ret
__local_unwind2:
  push        ebx
  push        esi
  push        edi
  mov         eax,dword ptr [esp+10h]
  push        ebp
  push        eax
  push        0FFFFFFFEh
  push        offset __unwind_handler
  push        dword ptr fs:[0]
  nop ; 00
  mov         dword ptr fs:[0],esp
  nop ; 00
_lu_top:
  mov         eax,dword ptr [esp+24h]
  mov         ebx,dword ptr [eax+8]
  mov         esi,dword ptr [eax+0Ch]
  cmp         esi,0FFFFFFFFh
  je          _lu_done
  cmp         dword ptr [esp+28h],0FFFFFFFFh
  je          _continue_
  cmp         esi,dword ptr [esp+28h]
  jbe         _lu_done
_continue_:
  lea         esi,[esi+esi*2]
  mov         ecx,dword ptr [ebx+esi*4]
  mov         dword ptr [esp+8],ecx
  mov         dword ptr [eax+0Ch],ecx
  cmp         dword ptr [ebx+esi*4+4],0
  jne         __NLG_Return2
  push        101h
  mov         eax,dword ptr [ebx+esi*4+8]
  call        __NLG_Notify
  call        dword ptr [ebx+esi*4+8]
__NLG_Return2:
  jmp         _lu_top
_lu_done:
  pop         dword ptr fs:[0]
  nop ; 00
  add         esp,10h
  pop         edi
  pop         esi
  pop         ebx
  ret
__abnormal_termination:
  xor         eax,eax
  mov         ecx,dword ptr fs:[0]
  nop ; 00
  cmp         dword ptr [ecx+4],offset __unwind_handler
  nop ; 00
  jne         _at_done
  mov         edx,dword ptr [ecx+0Ch]
  mov         edx,dword ptr [edx+0Ch]
  cmp         dword ptr [ecx+8],edx
  jne         _at_done
  mov         eax,1
_at_done:
  ret
__NLG_Notify1:
  push        ebx
  push        ecx
  mov         ebx,offset __NLG_Destination
  jmp         __NLG_Go
__NLG_Notify:
  push        ebx
  push        ecx
  mov         ebx,offset __NLG_Destination
  mov         ecx,dword ptr [ebp+8]
__NLG_Go:
  mov         dword ptr [ebx+8],ecx
  mov         dword ptr [ebx+4],eax
  mov         dword ptr [ebx+0Ch],ebp
  push        ebp
  push        ecx
  push        eax
__NLG_Dispatch:
  pop         eax
  pop         ecx
  pop         ebp
  pop         ecx
  pop         ebx
  ret         4

;__global_unwind2        endp


EXTERN _NtQueryVirtualMemory@24: PROC

;__ValidateEH3RN        proc    near

__ValidateEH3RN:
  mov         edi,edi
  push        ebp
  mov         ebp,esp
  sub         esp,20h
  push        ebx
  push        esi
  mov         esi,dword ptr [ebp+8]
  mov         ebx,dword ptr [esi+8]
  test        bl,3
  jne         label01
  mov         eax,dword ptr fs:[00000004h]
  mov         dword ptr [ebp+8],eax
  mov         eax,dword ptr fs:[00000008h]
  mov         dword ptr [ebp-4],eax
  cmp         ebx,dword ptr [ebp-4]
  jb          label02
  cmp         ebx,dword ptr [ebp+8]
  jae         label02
label01:
  xor         eax,eax
  jmp         label03
label02:
  push        edi
  mov         edi,dword ptr [esi+0Ch]
  cmp         edi,0FFFFFFFFh
  jne         label04
  xor         eax,eax
  inc         eax
  jmp         label05
label04:
  xor         edx,edx
  mov         dword ptr [ebp+8],edx
  mov         eax,ebx
label30:
  mov         ecx,dword ptr [eax]
  cmp         ecx,0FFFFFFFFh
  je          label06
  cmp         ecx,edx
  jae         label07
label06:
  cmp         dword ptr [eax+4],0
  je          label10
  inc         dword ptr [ebp+8]
label10:
  inc         edx
  add         eax,0Ch
  cmp         edx,edi
  jbe         label30
  cmp         dword ptr [ebp+8],0
  je          label11
  mov         eax,dword ptr [esi-8]
  cmp         eax,dword ptr [ebp-4]
  jb          label07
  cmp         eax,esi
  jae         label07
label11:
  mov         ecx,dword ptr [_nValidPages]
  mov         esi,ebx
  and         esi,0FFFFF000h
  xor         eax,eax
  test        ecx,ecx
  jle         label08
label12:
  cmp         dword ptr _rgValidPages[eax*4],esi
  nop ; 00
  je          label09
  inc         eax
  cmp         eax,ecx
  jl          label12
label08:
  lea         eax,[ebp+8]
  push        eax
  push        1Ch
  lea         eax,[ebp-20h]
  push        eax
  push        0
  push        ebx
  or          edi,0FFFFFFFFh
  push        edi
  ;call        _NtQueryVirtualMemory
  call        _NtQueryVirtualMemory@24 ; call it from ntdll.lib
  test        eax,eax
  jl          label13
  mov         eax,dword ptr [ebp+8]
  jmp         label14
  xor         eax,eax
label07:
  jmp         label05
  test        eax,eax
label09:
  jle         label15
  xor         edx,edx
  mov         ebx,offset _lModifying
  inc         edx
  mov         edi,ebx
  xchg        edx,dword ptr [edi]
  test        edx,edx
  jne         label15
  cmp         dword ptr _rgValidPages[eax*4],esi
  nop ; 00
  je          label15
  lea         eax,[ecx-1]
  test        eax,eax
  jl          label16
label18:
  cmp         dword ptr _rgValidPages[eax*4],esi
  nop ; 00
  je          label17
  dec         eax
  jns         label18
label17:
  test        eax,eax
  jge         label19
  cmp         ecx,10h
label16:
  jge         label20
  inc         ecx
  mov         dword ptr [_nValidPages],ecx
label20:
  lea         eax,[ecx-1]
  jmp         label15
label19:
  je          label21
label15:
  xor         edx,edx
  test        eax,eax
  jl          label21
label22:
  lea         ecx,_rgValidPages[edx*4]
  nop ; 00
  mov         edi,dword ptr [ecx]
  inc         edx
  cmp         edx,eax
  mov         dword ptr [ecx],esi
  mov         esi,edi
  jle         label22
label21:
  xor         eax,eax
  xchg        eax,dword ptr [ebx]
  jmp         label15
label13:
  xor         eax,eax
label14:
  test        eax,eax
  je          label23
  cmp         dword ptr [ebp-8],1000000h
  nop ; 01
  jne         label23
  test        byte ptr [ebp-0Ch],0CCh
  je          label0
  mov         ecx,dword ptr [ebp-1Ch]
  cmp         word ptr [ecx],5A4Dh
  jne         label23
  mov         eax,dword ptr [ecx+3Ch]
  add         eax,ecx
  cmp         dword ptr [eax],4550h
  jne         label23
  cmp         word ptr [eax+18h],10Bh
  jne         label23
  sub         ebx,ecx
  cmp         word ptr [eax+6],0
  movzx       ecx,word ptr [eax+14h]
  lea         ecx,[ecx+eax+18h]
  jbe         label23
  mov         eax,dword ptr [ecx+0Ch]
  cmp         ebx,eax
  jb          label0
  mov         edx,dword ptr [ecx+8]
  add         edx,eax
  cmp         ebx,edx
  jae         label0
  test        byte ptr [ecx+27h],80h
  jne         label07
label0:
  xor         eax,eax
  inc         eax
  mov         ecx,eax
  mov         edx,offset _lModifying
  xchg        ecx,dword ptr [edx]
  test        ecx,ecx
  jne         label05
  mov         ecx,dword ptr [_nValidPages]
  test        ecx,ecx
  mov         edx,ecx
  jle         label24
  lea         eax,_rgValidPages[ecx*4+0FFFFFFFCh]
  nop ; FF
label25:
  cmp         dword ptr [eax],esi
  je          label24
  dec         edx
  sub         eax,4
  test        edx,edx
  jg          label25
label24:
  test        edx,edx
  jne         label26
  push        0Fh
  pop         ebx
  cmp         ecx,ebx
  jg          label27
  mov         ebx,ecx
label27:
  xor         edx,edx
  test        ebx,ebx
  jl          label28
label29:
  lea         eax,_rgValidPages[edx*4]
  nop ; 00
  mov         edi,dword ptr [eax]
  inc         edx
  cmp         edx,ebx
  mov         dword ptr [eax],esi
  mov         esi,edi
  jle         label29
label28:
  cmp         ecx,10h
  jge         label26
  inc         ecx
  mov         dword ptr [_nValidPages],ecx
label26:
  xor         eax,eax
  mov         ecx,offset _lModifying
  xchg        eax,dword ptr [ecx]
  jmp         label15
label23:
  mov         eax,edi
label05:
  pop         edi
label03:
  pop         esi
  pop         ebx
  leave
  ret

;__ValidateEH3RN        endp

_TEXT           ends


.model flat
.data

; Global variables for  __NLG_Destination
;typedef struct {
;	unsigned long dwSig;
;	unsigned long uoffDestination;
;	unsigned long dwCode;
;	unsigned long uoffFramePointer;
;} _NLG_INFO;
;_NLG_INFO _NLG_Destination = { 0x019930520, 0, 0, 0 };
__NLG_Destination LABEL DWORD
	dd	1 dup (019930520h) ; MAGIC_NUMBER1
	dd	1 dup (0)
	dd	1 dup (0)
	dd	1 dup (0)
	public  __NLG_Destination


; Global variables for  __ValidateEH3RN
;INT		_nValidPages = 0;
_nValidPages LABEL DWORD
	dd	1 dup (0)
	public  _nValidPages
;PVOID	_rgValidPages[0x10] = { NULL };
_rgValidPages LABEL DWORD
	dd	16 dup (0)
	public  _rgValidPages
;BOOL	_lModifying = FALSE;
_lModifying LABEL DWORD
	dd	1 dup (0)
	public  _lModifying


end