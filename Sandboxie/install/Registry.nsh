;----------------------------------------------------------------------------
; Registry Access, 64-bit Enabled
;----------------------------------------------------------------------------

; !define REG_DEBUG

!verbose push
!verbose 0

;----------------------------------------------------------------------------
; Public Defines
;----------------------------------------------------------------------------

!define HKEY_CLASSES_ROOT        0x80000000
!define HKEY_CURRENT_USER        0x80000001
!define HKEY_LOCAL_MACHINE       0x80000002
!define HKEY_USERS               0x80000003
!define HKEY_PERFORMANCE_DATA    0x80000004
!define HKEY_PERFORMANCE_TEXT    0x80000050
!define HKEY_PERFORMANCE_NLSTEXT 0x80000060
!define HKEY_CURRENT_CONFIG      0x80000005
!define HKEY_DYN_DATA            0x80000006

!define KEY_QUERY_VALUE          0x0001
!define KEY_SET_VALUE            0x0002
!define KEY_CREATE_SUB_KEY       0x0004
!define KEY_ENUMERATE_SUB_KEYS   0x0008
!define DELETE                   0x10000
 
!define REG_NONE                 0
!define REG_SZ                   1
!define REG_EXPAND_SZ            2
!define REG_BINARY               3
!define REG_DWORD                4
!define REG_DWORD_LITTLE_ENDIAN  4
!define REG_DWORD_BIG_ENDIAN     5
!define REG_LINK                 6
!define REG_MULTI_SZ             7

;----------------------------------------------------------------------------
; Macro Reg_ReadString
;----------------------------------------------------------------------------

!macro Reg_ReadString UN ROOT SUBKEY VALUE

    Push ${VALUE}
    Push ${SUBKEY}
    Push ${ROOT}
    
    Call ${UN}Func_ReadRegStr_64

!macroend

;----------------------------------------------------------------------------
; Macro Reg_WriteString
;----------------------------------------------------------------------------

!macro Reg_WriteString UN ROOT SUBKEY VALUE DATA

    Push ${DATA}
    Push ${VALUE}
    Push ${SUBKEY}
    Push ${ROOT}
    
    Call ${UN}Func_WriteRegStr_64

!macroend

;----------------------------------------------------------------------------
; Macro Reg_TestKey
;----------------------------------------------------------------------------

!macro Reg_TestKey UN ROOT SUBKEY

    Push ${SUBKEY}
    Push ${ROOT}

    Call ${UN}Func_TestRegKey_64

!macroend

;----------------------------------------------------------------------------
; Macro Reg_CreateKey
;----------------------------------------------------------------------------

!macro Reg_CreateKey UN ROOT SUBKEY NEWKEY

    Push ${NEWKEY}
    Push ${SUBKEY}
    Push ${ROOT}
    
    Call ${UN}Func_CreateRegKey_64

!macroend

;----------------------------------------------------------------------------
; Macro Reg_DeleteKey
;----------------------------------------------------------------------------

!macro Reg_DeleteKey UN ROOT SUBKEY DELKEY

    Push ${DELKEY}
    Push ${SUBKEY}
    Push ${ROOT}
    
    Call ${UN}Func_DeleteRegKey_64

!macroend

;----------------------------------------------------------------------------
; Macro Reg_DeleteValue
;----------------------------------------------------------------------------

!macro Reg_DeleteValue UN ROOT SUBKEY VALUE

    Push ${VALUE}
    Push ${SUBKEY}
    Push ${ROOT}
    
    Call ${UN}Func_DeleteRegValue_64

!macroend

;----------------------------------------------------------------------------
; Private Defines
;----------------------------------------------------------------------------

!define RegOpenKeyEx     "Advapi32::RegOpenKeyExA(i, t, i, i, i) i"
!define RegCreateKeyEx   "Advapi32::RegCreateKeyExA(i, t, i, i, i, i, i, i, i) i"
!define RegQueryValueEx  "Advapi32::RegQueryValueExA(i, t, i, i, i, i) i"
!define RegSetValueEx    "Advapi32::RegSetValueExA(i, t, i, i, t, i) i"
!define RegDeleteKey     "Advapi32::RegDeleteKeyA(i, t) i"
!define RegDeleteKeyEx   "Advapi32::RegDeleteKeyExA(i, t, i, i) i"
!define RegDeleteValue   "Advapi32::RegDeleteValueA(i, t) i"
!define RegCloseKey      "Advapi32::RegCloseKey(i) i"

;----------------------------------------------------------------------------
; Function Reg_Sam64Key
;----------------------------------------------------------------------------

!macro Reg_Sam64Key_Common UN
Function ${UN}Reg_Sam64Key

    Push $0
    
    Call ${UN}IsWow64
    Pop $0
    StrCmp $0 'N' pushzero

    Pop $0
    Push 0x100		; KEY_WOW64_64KEY
    Return
    
pushzero:

    Pop $0
    Push 0
    Return

FunctionEnd
!macroend


!insertmacro Reg_Sam64Key_Common ""
; !insertmacro Reg_Sam64Key_Common "un."

;----------------------------------------------------------------------------
; Function Func_ReadRegStr_64
;----------------------------------------------------------------------------

!macro Func_ReadRegStr_64_Common UN
Function ${UN}Func_ReadRegStr_64

    ; Pop parameters pushed to the stack by the macro

    Pop $7 ; ROOT
    Pop $8 ; SUBKEY
    Pop $9 ; VALUE

    ; Push empty string as a default return value
    
    Push ""
   
    ; Initialize work registers:
    ;   Set $0 = 0
    ;   Set $1 = 0
    ;   Set $3 = ""
    
    StrCpy $0 ""
    StrCpy $1 ""
    StrCpy $3 ""
    System::Call "*(i) i (0) .r0"
    System::Call "*(i) i (0) .r1"
    
    ; Set $6 = KEY_WOW64_64KEY or zero

    Call ${UN}Reg_Sam64Key
    Pop $6
    
    ; Call $3 = RegOpenKeyEx(ROOT, SUBKEY, 0, (sam | $6), $0)

    System::Call "${RegOpenKeyEx}( \
        $7, '$8', 0, ${KEY_QUERY_VALUE}|$6, r0) .r3"

    StrCmp $3 0 open_ok
!ifdef REG_DEBUG
    MessageBox MB_OK|MB_ICONSTOP \
        "Can't open registry key for reading $7 $8 ($3)"
!endif
    DetailPrint "Can't open registry key for reading $7 $8 ($3)"
    SetErrors
    Goto done1

open_ok:

    ; Set $4 = *(int *)$0

    System::Call "*$0(&i4 .r4)"

    ; Set $2 = ${NSIS_MAX_STRLEN}
    
    StrCpy $2 ""
    System::Call "*(i) i (${NSIS_MAX_STRLEN}) .r2"

    System::Call "*$2(&i4 .r6)"
    
    ; Set $6 --> buffer, size $2 bytes
    
    System::Alloc ${NSIS_MAX_STRLEN}
    Pop $6
    
    ; Call $3 = RegQueryValueEx($4, VALUE, 0, $1, $6, $2) .r3"
    
    System::Call "${RegQueryValueEx}(r4, '$9', 0, r1, r6, r2) .r3"

    StrCmp $3 0 read_ok
!ifdef REG_DEBUG
    MessageBox MB_OK|MB_ICONSTOP \
        "Can't read registry value $9 in key $7 $8 ($3)"
!endif
    DetailPrint "Can't read registry value $9 in key $7 $8 ($3)"
    SetErrors
    Goto done2

read_ok:

    ; Set $3 = *(int *)$1

    System::Call "*$1(&i4 .r3)"
    
    StrCmp $3 ${REG_SZ} type_ok
    StrCmp $3 ${REG_EXPAND_SZ} type_ok
!ifdef REG_DEBUG
    MessageBox MB_OK|MB_ICONSTOP \
        "Registry value $9 in key $7 $8 is not a string"
!endif
    DetailPrint "Registry value $9 in key $7 $8 is not a string"
    SetErrors
    Goto done2

type_ok:

    ; Pop the default return value and push the value queried

    Pop $3

    System::Call "*$6(&t${NSIS_MAX_STRLEN} .r3)"
    Push $3

    ; Indicate Success
    
    ClearErrors

done2:

    System::Free $6
    System::Free $2

    ; call RegCloseKey($4)

    System::Call "${RegCloseKey}(r4)"

done1:

    System::Free $1
    System::Free $0

FunctionEnd
!macroend


!insertmacro Func_ReadRegStr_64_Common ""
; !insertmacro Func_ReadRegStr_64_Common "un."

;----------------------------------------------------------------------------
; Function Func_WriteRegStr_64
;----------------------------------------------------------------------------

!macro Func_WriteRegStr_64_Common UN
Function ${UN}Func_WriteRegStr_64

    ; Pop parameters pushed to the stack by the macro

    Pop $7 ; ROOT
    Pop $8 ; SUBKEY
    Pop $9 ; VALUE
    Pop $5 ; DATA

    ; Initialize work registers:
    ;   Set $0 = 0
    ;   Set $1 = 0
    ;   Set $3 = ""
    
    StrCpy $0 ""
    StrCpy $1 ""
    StrCpy $3 ""
    System::Call "*(i) i (0) .r0"
    System::Call "*(i) i (0) .r1"
    
    ; Set $6 = KEY_WOW64_64KEY or zero

    Call ${UN}Reg_Sam64Key
    Pop $6
    
    ; Call $3 = RegOpenKeyEx(ROOT, SUBKEY, 0, (sam | $6), $0)

    System::Call "${RegOpenKeyEx}( \
        $7, '$8', 0, ${KEY_SET_VALUE}|$6, r0) .r3"

    StrCmp $3 0 open_ok
    MessageBox MB_OK|MB_ICONSTOP \
        "Can't open registry key for writing $7 $8 ($3)"
    DetailPrint "Can't open registry key for writing $7 $8 ($3)"
    SetErrors
    Goto done1

open_ok:

    ; Set $4 = *(int *)$0

    System::Call "*$0(&i4 .r4)"
    
    ; Set $2 = StrLen(DATA) + 1
    
    StrLen $2 '$5'
    
    IntOp $2 $2 + 1

    ; Call $3 = RegSetValueEx($4, VALUE, 0, REG_SZ, DATA, $2) .r3"
    
    System::Call "${RegSetValueEx}( \
        r4, '$9', 0, ${REG_SZ}, '$5', $2) .r3"

    StrCmp $3 0 write_ok
    MessageBox MB_OK|MB_ICONSTOP \
        "Can't write registry value $9 in key $7 $8 ($3)"
    DetailPrint "Can't write registry value $9 in key $7 $8 ($3)"
    SetErrors
    Goto done2

write_ok:

    ; Indicate Success
    
    DetailPrint "Created registry value $9 in key $8"
    
    ClearErrors

done2:

    ; call RegCloseKey($4)

    System::Call "${RegCloseKey}(r4)"

done1:

    System::Free $1
    System::Free $0

FunctionEnd
!macroend


!insertmacro Func_WriteRegStr_64_Common ""
; !insertmacro Func_WriteRegStr_64_Common "un."

;----------------------------------------------------------------------------
; Function Func_TestRegKey_64
;----------------------------------------------------------------------------

!macro Func_TestRegKey_64_Common UN
Function ${UN}Func_TestRegKey_64

    ; Pop parameters pushed to the stack by the macro

    Pop $7 ; ROOT
    Pop $8 ; SUBKEY
    
    ; Initialize work registers:
    ;   Set $0 = 0
    ;   Set $3 = ""
    
    StrCpy $0 ""
    StrCpy $3 ""
    System::Call "*(i) i (0) .r0"
    
    ; Set $6 = KEY_WOW64_64KEY or zero

    Call ${UN}Reg_Sam64Key
    Pop $6
    
    ; Call $3 = RegOpenKeyEx(ROOT, SUBKEY, 0, (sam | $6), $0)

    System::Call "${RegOpenKeyEx}( \
        $7, '$8', 0, ${KEY_CREATE_SUB_KEY}|$6, r0) .r3"

    StrCmp $3 0 open_ok

    ; Push error return value
    
    Push "0"
    
    Goto done

open_ok:

    ; Set $4 = *(int *)$0

    System::Call "*$0(&i4 .r4)"
    
    ; RegCloseKey($4)
    
    System::Call "${RegCloseKey}(r4)"
    
    ; Push success return value
    
    Push "1"

done:

    System::Free $0

FunctionEnd
!macroend


!insertmacro Func_TestRegKey_64_Common ""
; !insertmacro Func_TestRegKey_64_Common "un."

;----------------------------------------------------------------------------
; Function Func_CreateRegKey_64
;----------------------------------------------------------------------------

!macro Func_CreateRegKey_64_Common UN
Function ${UN}Func_CreateRegKey_64

    ; Pop parameters pushed to the stack by the macro

    Pop $7 ; ROOT
    Pop $8 ; SUBKEY
    Pop $9 ; NEWKEY
    
    ; Initialize work registers:
    ;   Set $0 = 0
    ;   Set $1 = 0
    ;   Set $3 = ""
    
    StrCpy $0 ""
    StrCpy $1 ""
    StrCpy $3 ""
    System::Call "*(i) i (0) .r0"
    System::Call "*(i) i (0) .r1"
    
    ; Set $6 = KEY_WOW64_64KEY or zero

    Call ${UN}Reg_Sam64Key
    Pop $6
    
    ; Call $3 = RegOpenKeyEx(ROOT, SUBKEY, 0, (sam | $6), $0)

    System::Call "${RegOpenKeyEx}( \
        $7, '$8', 0, ${KEY_CREATE_SUB_KEY}|$6, r0) .r3"

    StrCmp $3 0 open_ok
    MessageBox MB_OK|MB_ICONSTOP \
        "Can't open registry key for create $7 $8 ($3)"
    DetailPrint "Can't open registry key for create $7 $8 ($3)"
    SetErrors
    Goto done1

open_ok:

    ; Set $4 = *(int *)$0

    System::Call "*$0(&i4 .r4)"

    ; Call $3 = RegCreateKeyEx($4, NEWKEY, 0, 0, 0, (sam | $6), 0, $1, 0)
    
    System::Call "${RegCreateKeyEx}( \
        $4, '$9', 0, 0, 0, \
        ${KEY_SET_VALUE}|${KEY_CREATE_SUB_KEY}|$6, \
        0, r1, 0) .r3"
    
    StrCmp $3 0 create_ok
    MessageBox MB_OK|MB_ICONSTOP \
        "Can't create registry key $7 $8 $9 ($3)"
    DetailPrint "Can't create registry key $7 $8 $9 ($3)"
    SetErrors
    Goto done2

create_ok:

    ; Set $5 = *(int *)$1

    System::Call "*$1(&i4 .r5)"
    
    ; RegCloseKey($5)
    
    System::Call "${RegCloseKey}(r5)"
    
    ; Indicate Success

    DetailPrint "Created registry key $8\$9"
    
    ClearErrors

done2:

    ; RegCloseKey($4)
    
    System::Call "${RegCloseKey}(r4)"

done1:

    System::Free $1
    System::Free $0

FunctionEnd
!macroend


!insertmacro Func_CreateRegKey_64_Common ""
; !insertmacro Func_CreateRegKey_64_Common "un."

;----------------------------------------------------------------------------
; Function Func_DeleteRegKey_64
;----------------------------------------------------------------------------

!macro Func_DeleteRegKey_64_Common UN
Function ${UN}Func_DeleteRegKey_64

    ; Pop parameters pushed to the stack by the macro

    Pop $7 ; ROOT
    Pop $8 ; SUBKEY
    Pop $9 ; DELKEY

    ; Initialize work registers:
    ;   Set $0 = 0
    ;   Set $1 = 0
    ;   Set $3 = ""
    
    StrCpy $0 ""
    StrCpy $1 ""
    StrCpy $3 ""
    System::Call "*(i) i (0) .r0"
    System::Call "*(i) i (0) .r1"
    
    ; Set $6 = KEY_WOW64_64KEY or zero

    Call ${UN}Reg_Sam64Key
    Pop $6
    
    ; Call $3 = RegOpenKeyEx(ROOT, SUBKEY, 0, (sam | $6), $0)

    System::Call "${RegOpenKeyEx}( \
        $7, '$8', 0, ${DELETE}|$6, r0) .r3"

    StrCmp $3 0 open_ok
!ifdef REG_DEBUG
    MessageBox MB_OK|MB_ICONSTOP \
        "Can't open registry key for delete $7 $8 ($3)"
!endif
    DetailPrint "Can't open registry key for delete $7 $8 ($3)"
    SetErrors
    Goto done1

open_ok:

    ; Set $4 = *(int *)$0

    System::Call "*$0(&i4 .r4)"

    ; Use RegDeleteKeyEx on amd64, RegDeleteKey on i386
    
    StrCmp $6 "0" delete_i386

    ; Call $3 = RegDeleteKeyEx($4, DELKEY, (sam | $6), 0)
    
    System::Call "${RegDeleteKeyEx}($4, '$9', $6, 0) .r3"
    Goto delete_cont

delete_i386:

    ; Call $3 = RegDeleteKey($4, DELKEY)
    
    System::Call "${RegDeleteKey}($4, '$9') .r3"
    
delete_cont:

    StrCmp $3 0 delete_ok
!ifdef REG_DEBUG
    MessageBox MB_OK|MB_ICONSTOP \
        "Can't delete registry key $7 $8 $9 ($3)"
!endif
    DetailPrint "Can't delete registry key $7 $8 $9 ($3)"
    SetErrors
    Goto done2

delete_ok:

    ; Indicate Success

    DetailPrint "Deleted registry key $8\$9"

    ClearErrors

done2:

    ; RegCloseKey($4)
    
    System::Call "${RegCloseKey}(r4)"

done1:

    System::Free $1
    System::Free $0

FunctionEnd
!macroend

    
!insertmacro Func_DeleteRegKey_64_Common ""
; !insertmacro Func_DeleteRegKey_64_Common "un."

;----------------------------------------------------------------------------
; Function Func_DeleteRegValue_64
;----------------------------------------------------------------------------

!macro Func_DeleteRegValue_64_Common UN
Function ${UN}Func_DeleteRegValue_64

    ; Pop parameters pushed to the stack by the macro

    Pop $7 ; ROOT
    Pop $8 ; SUBKEY
    Pop $9 ; VALUE

    ; Initialize work registers:
    ;   Set $0 = 0
    ;   Set $1 = 0
    ;   Set $3 = ""
    
    StrCpy $0 ""
    StrCpy $1 ""
    StrCpy $3 ""
    System::Call "*(i) i (0) .r0"
    System::Call "*(i) i (0) .r1"
    
    ; Set $6 = KEY_WOW64_64KEY or zero

    Call ${UN}Reg_Sam64Key
    Pop $6
    
    ; Call $3 = RegOpenKeyEx(ROOT, SUBKEY, 0, (sam | $6), $0)

    System::Call "${RegOpenKeyEx}( \
        $7, '$8', 0, ${KEY_SET_VALUE}|$6, r0) .r3"

    StrCmp $3 0 open_ok
!ifdef REG_DEBUG
    MessageBox MB_OK|MB_ICONSTOP \
        "Can't open registry key for value delete $7 $8 ($3)"
!endif
    DetailPrint "Can't open registry key for value delete $7 $8 ($3)"
    SetErrors
    Goto done1

open_ok:

    ; Set $4 = *(int *)$0

    System::Call "*$0(&i4 .r4)"

    ; Call $3 = RegDeleteValue($4, VALUE)
    
    System::Call "${RegDeleteValue}($4, '$9') .r3"
    
    StrCmp $3 0 delete_ok
!ifdef REG_DEBUG
    MessageBox MB_OK|MB_ICONSTOP \
        "Can't delete registry value $7 $8 $9 ($3)"
!endif
    DetailPrint "Can't delete registry value $7 $8 $9 ($3)"
    SetErrors
    Goto done2

delete_ok:

    ; Indicate Success

    DetailPrint "Deleted registry value $9 in key $8"

    ClearErrors

done2:

    ; RegCloseKey($4)
    
    System::Call "${RegCloseKey}(r4)"

done1:

    System::Free $1
    System::Free $0

FunctionEnd
!macroend

    
!insertmacro Func_DeleteRegValue_64_Common ""
; !insertmacro Func_DeleteRegValue_64_Common "un."

;----------------------------------------------------------------------------

!verbose pop
