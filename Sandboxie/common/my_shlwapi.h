/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
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

#ifndef _MY_SHLWAPI_H
#define _MY_SHLWAPI_H

//---------------------------------------------------------------------------
// Imports from shlwapi.h
//---------------------------------------------------------------------------

#define LWSTDAPI          EXTERN_C DECLSPEC_IMPORT HRESULT STDAPICALLTYPE

enum {
    ASSOCF_INIT_NOREMAPCLSID           = 0x00000001,  //  do not remap clsids to progids
    ASSOCF_INIT_BYEXENAME              = 0x00000002,  //  executable is being passed in
    ASSOCF_OPEN_BYEXENAME              = 0x00000002,  //  executable is being passed in
    ASSOCF_INIT_DEFAULTTOSTAR          = 0x00000004,  //  treat "*" as the BaseClass
    ASSOCF_INIT_DEFAULTTOFOLDER        = 0x00000008,  //  treat "Folder" as the BaseClass
    ASSOCF_NOUSERSETTINGS              = 0x00000010,  //  don't use HKCU
    ASSOCF_NOTRUNCATE                  = 0x00000020,  //  don't truncate the return string
    ASSOCF_VERIFY                      = 0x00000040,  //  verify data is accurate (DISK HITS)
    ASSOCF_REMAPRUNDLL                 = 0x00000080,  //  actually gets info about rundlls target if applicable
    ASSOCF_NOFIXUPS                    = 0x00000100,  //  attempt to fix errors if found
    ASSOCF_IGNOREBASECLASS             = 0x00000200,  //  don't recurse into the baseclass
};

typedef DWORD ASSOCF;

typedef enum {
    ASSOCSTR_COMMAND      = 1,  //  shell\verb\command string
    ASSOCSTR_EXECUTABLE,        //  the executable part of command string
    ASSOCSTR_FRIENDLYDOCNAME,   //  friendly name of the document type
    ASSOCSTR_FRIENDLYAPPNAME,   //  friendly name of executable
    ASSOCSTR_NOOPEN,            //  noopen value
    ASSOCSTR_SHELLNEWVALUE,     //  query values under the shellnew key
    ASSOCSTR_DDECOMMAND,        //  template for DDE commands
    ASSOCSTR_DDEIFEXEC,         //  DDECOMMAND to use if just create a process
    ASSOCSTR_DDEAPPLICATION,    //  Application name in DDE broadcast
    ASSOCSTR_DDETOPIC,          //  Topic Name in DDE broadcast
    ASSOCSTR_INFOTIP,           //  info tip for an item, or list of properties to create info tip from
    ASSOCSTR_QUICKTIP,          //  same as ASSOCSTR_INFOTIP, except, this list contains only quickly retrievable properties
    ASSOCSTR_TILEINFO,          //  similar to ASSOCSTR_INFOTIP - lists important properties for tileview
    ASSOCSTR_CONTENTTYPE,       //  MIME Content type
    ASSOCSTR_DEFAULTICON,       //  Default icon source
    ASSOCSTR_SHELLEXTENSION,    //  Guid string pointing to the Shellex\Shellextensionhandler value.
    ASSOCSTR_MAX                //  last item in enum...
} ASSOCSTR;

LWSTDAPI AssocQueryStringW(ASSOCF flags, ASSOCSTR str, LPCWSTR pszAssoc, LPCWSTR pszExtra, LPWSTR pszOut, DWORD *pcchOut);
#define AssocQueryString  AssocQueryStringW

typedef HRESULT (*P_AssocQueryStringW)(
    ASSOCF flags, ASSOCSTR str, LPCTSTR pszAssoc,
    LPCTSTR pszExtra, LPTSTR pszOut, DWORD *pcchOut);


#define SHACF_DEFAULT                   0x00000000
#define SHACF_FILESYSTEM                0x00000001
#define SHACF_URLALL                    (SHACF_URLHISTORY | SHACF_URLMRU)
#define SHACF_URLHISTORY                0x00000002
#define SHACF_URLMRU                    0x00000004
#define SHACF_USETAB                    0x00000008
#define SHACF_FILESYS_ONLY              0x00000010

#define SHACF_AUTOSUGGEST_FORCE_ON      0x10000000
#define SHACF_AUTOSUGGEST_FORCE_OFF     0x20000000
#define SHACF_AUTOAPPEND_FORCE_ON       0x40000000
#define SHACF_AUTOAPPEND_FORCE_OFF      0x80000000


LWSTDAPI SHAutoComplete(HWND hwndEdit, DWORD dwFlags);


#endif /* _MY_SHLWAPI_H */
