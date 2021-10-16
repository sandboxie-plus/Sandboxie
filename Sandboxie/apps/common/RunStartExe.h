#ifndef _MY_RUNSTARTEXE_H
#define _MY_RUNSTARTEXE_H


//---------------------------------------------------------------------------
// Functions (c++)
//---------------------------------------------------------------------------


#ifdef __cplusplus


#include <afxcmn.h>


void Common_RunStartExe(const CString &cmd, const CString &box,
                        BOOL wait = FALSE, BOOL inherit = FALSE);


#endif __cplusplus

#endif //_MY_RUNSTARTEXE_H