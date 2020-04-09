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
; _aulldvrm support routine is not available on Windows 2000
;
; this is an exact copy of _aulldvrm procedure from ulldvrm.obj object file
; in the c:\work\tools\winddk\lib\crt\i386\libcmt.lib library
;
; it also the same as the copy of _aulldvrm in the Windows kernel
;----------------------------------------------------------------------------

ifdef _WIN64

;

else

.386p
.model flat

.code

__aulldvrm 	proc

		push    esi
		mov     eax,dword ptr [esp+14h]
		or      eax,eax
		jne     label01
		mov     ecx,dword ptr [esp+10h]
		mov     eax,dword ptr [esp+0Ch]
		xor     edx,edx
		div     ecx
		mov     ebx,eax
		mov     eax,dword ptr [esp+8]
		div     ecx
		mov     esi,eax
		mov     eax,ebx
		mul     dword ptr [esp+10h]
		mov     ecx,eax
		mov     eax,esi
		mul     dword ptr [esp+10h]
		add     edx,ecx
		jmp     label05
label01:	mov     ecx,eax
		mov     ebx,dword ptr [esp+10h]
		mov     edx,dword ptr [esp+0Ch]
		mov     eax,dword ptr [esp+8]
label02:	shr     ecx,1
		rcr     ebx,1
		shr     edx,1
		rcr     eax,1
		or      ecx,ecx
		jne     label02
		div     ebx
		mov     esi,eax
		mul     dword ptr [esp+14h]
		mov     ecx,eax
		mov     eax,dword ptr [esp+10h]
		mul     esi
		add     edx,ecx
		jb      label03
		cmp     edx,dword ptr [esp+0Ch]
		ja      label03
		jb      label04
		cmp     eax,dword ptr [esp+8]
		jbe     label04
label03:	dec     esi
		sub     eax,dword ptr [esp+10h]
		sbb     edx,dword ptr [esp+14h]
label04:	xor     ebx,ebx
label05:	sub     eax,dword ptr [esp+8]
		sbb     edx,dword ptr [esp+0Ch]
		neg     edx
		neg     eax
		sbb     edx,0
		mov     ecx,edx
		mov     edx,ebx
		mov     ebx,ecx
		mov     ecx,eax
		mov     eax,esi
		pop     esi
		ret     10h

__aulldvrm 	endp

public 		__aulldvrm

endif

end
