;----------------------------------------------------------------------------
; 64-bit (WOW64) Detection
;----------------------------------------------------------------------------


!define IsWow64Process   "Kernel32::IsWow64Process(i, i) i"


!macro IsWow64_Common UN
Function ${UN}IsWow64

    ; Save $0, $1

    Push $0
    Push $1
    
    ; Set $0 --> Kernel32.IsWow64Process
    
    System::Get '${IsWow64Process}'
    Pop $0
    StrCmp $0 'error' pushzero
    
    ; Set $R1 --> DWORD
    
    StrCpy $1 ""
    System::Call "*(i) i (0) .r1"
    
    ; Call IsWow64Process(GetCurrentProcess(), $1)

    System::Call "kernel32::GetCurrentProcess() i .s"
    System::Call "${IsWow64Process}(s, r1)"
    
    ; Set $0 = *(int *)$1
    
    System::Call "*$1(&i4 .r0)"
    IntCmp $0 0 pushzero
    
    ; Restore $0, $1 and return with positive result

    Pop $1    
    Pop $0	
    Push "Y" 	
    Return
    
pushzero:

    ; Restore $0, $1 and return with negative result

    Pop $1
    Pop $0
    Push "N"
    Return

FunctionEnd
!macroend


!insertmacro IsWow64_Common ""
; !insertmacro IsWow64_Common "un."
