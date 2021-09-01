/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
 * Copyright 2020-2021 David Xanatos, xanasoft.com
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

#include <windows.h>
#include <stdio.h>
#include "common/list.h"
#include "common/list.c"


//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


struct MSG {

    LIST_ELEM list_elem;
    ULONG code;
    ULONG ver;
    WCHAR *text;
};


struct LANG {

    LIST_ELEM list_elem;
    WCHAR name[128];
    ULONG code;
    LIST msgs;
};


//---------------------------------------------------------------------------
// Alloc
//---------------------------------------------------------------------------


void *Alloc(ULONG Size)
{
    void *Buffer = HeapAlloc(GetProcessHeap(), 0, Size);
    if (! Buffer) {
        fprintf(stderr, "memory allocation error\n");
        exit(EXIT_FAILURE);
    }
    if (! HeapValidate(GetProcessHeap(), 0, NULL)) {
        fprintf(stderr, "memory allocation error\n");
        exit(EXIT_FAILURE);
    }
    return Buffer;
}


//---------------------------------------------------------------------------
// AddTextEntry
//---------------------------------------------------------------------------


ULONG AddTextEntry(
                const UCHAR *path, WCHAR *BufPtr, LIST *msgs, ULONG LineNum)
{
    ULONG code, ver, why;
    WCHAR *ptr, *cr, *lf, *dot;
    struct MSG *msg;

    why = 0;

    if (BufPtr[4] != L';') {
        why = 1;
        goto err;
    }

    code = _wtoi(BufPtr);
    if (! code) {
        why = 2;
        goto err;
    }

    ptr = BufPtr + 5;
    if (_wcsnicmp(ptr, L"evt;pop;", 8) == 0) {
        code |= 0x103 << 16;    // Facility=EventPopup
        ptr += 8;
    } else if (_wcsnicmp(ptr, L"evt;", 4) == 0) {
        code |= 0x101 << 16;    // Facility=Event
        ptr += 4;
    } else if (_wcsnicmp(ptr, L"pop;", 4) == 0) {
        code |= 0x102 << 16;    // Facility=Popup
        ptr += 4;
    } else if (_wcsnicmp(ptr, L"txt;", 4) == 0) {
        code |= 0x000 << 16;    // Facility=Text
        ptr += 4;
    } else if (_wcsnicmp(ptr, L"ins;", 4) == 0) {
        code |= 0xFFF << 16;    // NSIS text
        ptr += 4;
    } else {
        why = 3;
        goto err;
    }

    if ((code & 0xFFF0000) == 0x1010000 ||
        (code & 0xFFF0000) == 0x1020000 ||
        (code & 0xFFF0000) == 0x1030000) {

        // parse Severity only for Event/Popup/EventPopup

        if (_wcsnicmp(ptr, L"inf;", 4) == 0) {
            code |= 1 << 30;    // Severity=Informational
            ptr += 4;
        } else if (_wcsnicmp(ptr, L"wrn;", 4) == 0) {
            code |= 2 << 30;    // Severity=Warning
            ptr += 4;
        } else if (_wcsnicmp(ptr, L"err;", 4) == 0) {
            code |= 3 << 30;    // Severity=Error
            ptr += 4;
        } else {
            why = 4;
            goto err;
        }
    }

    ver = _wtoi(ptr);
    if (! ver) {
        why = 5;
        goto err;
    }

    cr = wcschr(ptr, L'\r');
    lf = wcschr(ptr, L'\n');
    if (cr && cr > lf)
        ptr = cr + 1;
    else if (lf)
        ptr = lf + 1;
    else {
        why = 6;
        goto err;
    }

    dot = ptr;
    while (1) {
        dot = wcschr(dot, L'.');
        if (! dot) {
            why = 7;
            goto err;
        }
        ++dot;
        if ((*dot == L'\r' || *dot == L'\n') &&
            (dot[-2] == L'\r' || dot[-2] == L'\n'))
        {
            *dot = L'\0';
            break;
        }
    }

    msg = Alloc(sizeof(struct MSG));
    msg->code = code;
    msg->ver = ver;
    msg->text = Alloc((wcslen(ptr) + 1) * sizeof(WCHAR));
    wcscpy(msg->text, ptr);

    List_Insert_After(msgs, NULL, msg);

    if ((code & 0xFFF0000) == 0xFFF0000) {

        // eliminate terminating dot for NSIS text

        ULONG len = wcslen(msg->text);
        if (len && msg->text[len - 1] == L'.') {

            while (len) {
                --len;
                if (! len)
                    break;
                if (msg->text[len - 1] != L'\n' &&
                    msg->text[len - 1] != L'\r')
                    break;
            }

            msg->text[len] = L'\0';
        }
    }

    // printf("Added code %08X/%02X text <%S>\n", msg->code, msg->ver, msg->text);

    *dot = L'\n';
    return dot - BufPtr;

err:

    cr = wcschr(BufPtr, L'\r');
    if (cr)
        *cr = L'\0';
    lf = wcschr(BufPtr, L'\n');
    if (lf)
        *lf = L'\0';

    fprintf(stderr, "*==========\n* Syntax Error in %s\n*==========\n\n", path);
    fprintf(stderr, "syntax error (%d) in file %s - line %d - %S\n", why, path, LineNum, BufPtr);
    fprintf(stderr, "\nA B O R T I N G\n\n\n");
    exit(EXIT_FAILURE);
    return 0;
}


//---------------------------------------------------------------------------
// ReadTextFile
//---------------------------------------------------------------------------


void ReadTextFile(const UCHAR *path, LIST *msgs)
{
    HANDLE hFile;
    ULONG ByteSize, ReadSize;
    UCHAR* Buffer;
    WCHAR *BufPtr;
    ULONG LineNum;
    ULONG i;
    BOOLEAN isUTF8 = TRUE;

    //
    // read entire contents of text file
    //

    hFile = CreateFileA(path, FILE_READ_DATA, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "can\'t open input file - %s\n", path);
        exit(EXIT_FAILURE);
    }

    ByteSize = GetFileSize(hFile, NULL);
    if (ByteSize)
        Buffer = Alloc(ByteSize + 16);
    else
        Buffer = NULL;

    if (! ReadFile(hFile, Buffer, ByteSize, &ReadSize, NULL))
        ReadSize = 0;
    if (ReadSize == 0 || ReadSize != ByteSize) {
        fprintf(stderr, "can\'t read input file - %s\n", path);
        exit(EXIT_FAILURE);
    }

    CloseHandle(hFile);

    memset(&Buffer[ReadSize], 0, 16);

    //
    // build list of messages
    //

    List_Init(msgs);

    LineNum = 1;


    if (Buffer[0] == 0xFF && Buffer[1] == 0xFE) // Unicode BOM
    {
        BufPtr = &Buffer[2];
        ReadSize -= 2;
        isUTF8 = FALSE;
    }
    else if (Buffer[0] == 0xEF && Buffer[1] == 0xBB && Buffer[2] == 0xBF)  // UTF8 BOM
    {
        BufPtr = &Buffer[3];
        ReadSize -= 3;
        isUTF8 = TRUE;
    }
    else
        BufPtr = Buffer;

    if (isUTF8)
    {
        char* utf8 = BufPtr;

        ByteSize = MultiByteToWideChar(CP_UTF8, 0, utf8, ReadSize, NULL, 0) + 1;
        Buffer = Alloc(ByteSize * sizeof(wchar_t));
        BufPtr = Buffer;

        MultiByteToWideChar(CP_UTF8, 0, utf8, ReadSize, BufPtr, ByteSize);
    }

    while (1) {

        WCHAR *Cr = wcschr(BufPtr, L'\r');
        WCHAR *Lf = wcschr(BufPtr, L'\n');
        ULONG Len;

        if (BufPtr[0] == L'\r' && BufPtr[1] == L'\n')
            ++LineNum;

        if (Cr == BufPtr || Lf == BufPtr) {
            ++BufPtr;
            continue;
        }

        if (Cr && Cr < Lf)
            Len = Cr - BufPtr;
        else if (Lf)
            Len = Lf - BufPtr;
        else
            break;

        if (Lf != Cr + 1) {
            fprintf(stderr, "\ncrlf error - %s - line %d - %*.*S\n", path, LineNum, Len, Len, BufPtr);
            exit(EXIT_FAILURE);
        }

        if (*BufPtr != L'#') {

            Len = AddTextEntry(path, BufPtr, msgs, LineNum);
            for (i = 0; i < Len; ++i)
                if (BufPtr[i] == L'\r' && BufPtr[i + 1] == L'\n') {
                    ++LineNum;
                    ++i;
                }

            ++LineNum;
        }

        BufPtr += Len;
    }
}


//---------------------------------------------------------------------------
// GetMsgForError
//---------------------------------------------------------------------------


const WCHAR *GetMsgForError(struct MSG *msg)
{
    static WCHAR text[4096];
    WCHAR *iptr, *optr;
    ULONG fac, sev;

    swprintf(text, L"%04d;", msg->code & 0xFFFF);

    fac = (msg->code >> 16) & 0xFFF;
    sev = (msg->code >> 30) & 3;

    if (fac == 0)
        wcscat(text, L"txt;");
    else if (fac == 0xFFF)
        wcscat(text, L"ins;");
    else {

        if ((fac & 0x101) == 0x101)
            wcscat(text, L"evt;");
        if ((fac & 0x102) == 0x102)
            wcscat(text, L"pop;");

        if (sev == 1)
            wcscat(text, L"inf;");
        else if (sev == 2)
            wcscat(text, L"wrn;");
        else if (sev == 3)
            wcscat(text, L"err;");
    }

    swprintf(text + wcslen(text), L"%02d\n", msg->ver);

    iptr = msg->text;
    optr = text + wcslen(text);
    while (*iptr) {
        *optr = *iptr;
        if (*iptr != L'\r')
            ++optr;
        ++iptr;
    }
    *optr = L'\n';
    ++optr;
    *optr = L'\n';
    ++optr;
    *optr = L'\0';

    return text;
}


//---------------------------------------------------------------------------
// DiscardOldText
//---------------------------------------------------------------------------


void DiscardOldText(const UCHAR *Name, LIST *msgs)
{
    struct MSG *mi, *mj;

    mi = List_Head(msgs);
    while (mi) {

        mj = List_Head(msgs);
        while (mj) {

            if (mj != mi && mj->code == mi->code) {

                if (mj->ver == mi->ver) {

                    fprintf(stderr, "*==========\n* Duplicate Messages in %s\n*==========\n\n", Name);
                    fprintf(stderr, "%S", GetMsgForError(mi));
                    fprintf(stderr, "\nA B O R T I N G\n\n\n");
                    exit(EXIT_FAILURE);
                }

                if (mj->ver > mi->ver) {

                    mi->ver = mj->ver;
                    mi->text = mj->text;
                }

                List_Remove(msgs, mj);
                mi = NULL;
                break;
            }

            mj = List_Next(mj);
        }

        if (mi)
            mi = List_Next(mi);
        else
            mi = List_Head(msgs);
    }
}


//---------------------------------------------------------------------------
// FindTextEntry
//---------------------------------------------------------------------------


struct MSG *FindTextEntry(LIST *msgs, ULONG code, ULONG ver)
{
    struct MSG *msg = List_Head(msgs);
    while (msg) {
        if (msg->code == code && msg->ver == ver)
            break;
        msg = List_Next(msg);
    }
    return msg;
}


//---------------------------------------------------------------------------
// CompareText
//---------------------------------------------------------------------------


void CompareText(UCHAR *Name, LIST *Foreign, LIST *English)
{
    struct MSG *me, *mf;
    BOOLEAN AtLeastOne = FALSE;

    fprintf(stderr, "*==========\n* Missing Messages in %s\n*==========\n\n", Name);

    //
    // report English text missing in Foreign file
    //

    me = List_Head(English);
    while (me) {

        mf = FindTextEntry(Foreign, me->code, me->ver);
        if (! mf) {

            AtLeastOne = TRUE;
            fprintf(stderr, "%S", GetMsgForError(me));

            mf = Alloc(sizeof(struct MSG));

            mf->code = me->code;
            mf->ver = me->ver;

            mf->text = me->text;

            List_Insert_After(Foreign, NULL, mf);
        }

        me = List_Next(me);
    }

    if (! AtLeastOne)
        fprintf(stderr, "There are no missing messages.\n\n");

    //
    // report Foreign text missing in English file
    //

    AtLeastOne = FALSE;

    mf = List_Head(Foreign);
    while (mf) {

        me = FindTextEntry(English, mf->code, mf->ver);
        if (! me) {

            if (! AtLeastOne) {
                fprintf(stderr, "*==========\n* Extraneous Messages in %s\n*==========\n\n", Name);
                AtLeastOne = TRUE;
            }

            fprintf(stderr, "%S", GetMsgForError(mf));
        }

        mf = List_Next(mf);
    }

    if (AtLeastOne)
        fprintf(stderr, "\n\n");
}


//---------------------------------------------------------------------------
// AddLanguage
//---------------------------------------------------------------------------


void AddLanguage(LIST *langs, LIST *msgs, const UCHAR *filename)
{
    struct LANG *lang;
    WCHAR *ptr;

    lang = Alloc(sizeof(struct LANG));

    swprintf(lang->name, L"%S", filename + 5);

    ptr = wcschr(lang->name, L'-');
    if (! ptr) {
        fprintf(stderr, "bad file name - %s\n", filename);
        return;
    }
    *ptr = L'\0';

    ++ptr;
    lang->code = _wtoi(ptr);
    if (! lang->code) {
        fprintf(stderr, "bad file name - %s\n", filename);
        return;
    }

    memcpy(&lang->msgs, msgs, sizeof(LIST));
    List_Insert_After(langs, NULL, lang);
}


//---------------------------------------------------------------------------
// Put
//---------------------------------------------------------------------------


void Put(HANDLE hFile, const WCHAR *Text)
{
    ULONG Written;
    WriteFile(hFile, Text, wcslen(Text) * sizeof(WCHAR), &Written, NULL);
}


//---------------------------------------------------------------------------
// WriteMessageFile
//---------------------------------------------------------------------------


void WriteMessageFile(LIST *langs)
{
    HANDLE hFile;
    struct LANG *lang;
    WCHAR text[128];
    LIST *English;
    struct MSG *me, *mf;

    //
    // create output file
    //

    hFile = CreateFile(L"msgs.mc", FILE_GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "can\'t create output file\n");
        exit(EXIT_FAILURE);
    }

    //
    // write FacilityNames
    //

    Put(hFile,  L"FacilityNames=(\r\n"
                L"    Text=0\r\n"
                L"    Event=0x101:MSG_FACILITY_EVENT\r\n"
                L"    Popup=0x102:MSG_FACILITY_POPUP\r\n"
                L"    EventPopup=0x103\r\n"
                L")\r\n\r\n");

    //
    // write LanguageNames
    //

    lang = List_Head(langs);
    English = &lang->msgs;
    lang = List_Next(lang);
    if (lang) {
        Put(hFile, L"LanguageNames=(\r\n");
        while (lang) {
            swprintf(text, L"    %s=0x%X:MSG%05X\r\n",
                     lang->name, lang->code, lang->code);
            Put(hFile, text);
            lang = List_Next(lang);
        }
        Put(hFile, L")\r\n\r\n");
    }

    //
    // write messages
    //

    me = List_Head(English);
    while (me) {

        ULONG fac = (me->code >> 16) & 0xFFF;
        ULONG sev = (me->code >> 30) & 3;

        if (fac != 0xFFF) {

            swprintf(text, L"MessageId=%d", me->code & 0xFFFF);

            wcscat(text, L" Facility=");
            if (fac == 0)
                wcscat(text, L"Text");
            else if (fac == 0x101)
                wcscat(text, L"Event");
            else if (fac == 0x102)
                wcscat(text, L"Popup");
            else if (fac == 0x103)
                wcscat(text, L"EventPopup");

            wcscat(text, L" Severity=");
            if (sev == 1)
                wcscat(text, L"Informational");
            else if (sev == 2)
                wcscat(text, L"Warning");
            else if (sev == 3)
                wcscat(text, L"Error");
            else
                wcscat(text, L"Success");

            swprintf(text + wcslen(text),
                     L" SymbolicName=MSG_%04d\r\n"
                     L"Language=English\r\n",
                     me->code & 0xFFFF);

            Put(hFile, text);

            Put(hFile, me->text);
            Put(hFile, L"\r\n");

            lang = List_Head(langs);
            lang = List_Next(lang);
            while (lang) {

                mf = FindTextEntry(&lang->msgs, me->code, me->ver);
                if (mf) {
                    swprintf(text, L"Language=%s\r\n", lang->name);
                    Put(hFile, text);
                    Put(hFile, mf->text);
                    Put(hFile, L"\r\n");
                }

                lang = List_Next(lang);
            }

            Put(hFile, L"\r\n");
        }

        me = List_Next(me);
    }

    CloseHandle(hFile);
}


//---------------------------------------------------------------------------
// FindCopyTextForNsis
//---------------------------------------------------------------------------


WCHAR *FindCopyTextForNsis(struct MSG *mf, LIST *msgs)
{
    if (_wcsnicmp(mf->text, L"=copy:", 6) == 0) {

        ULONG code = _wtoi(mf->text + 6);
        struct MSG *msg = List_Head(msgs);
        while (msg) {
            if (msg->code == code) {

                // eliminate terminating dot for NSIS text

                ULONG len = wcslen(msg->text);
                WCHAR *text = Alloc((len + 1) * sizeof(WCHAR));
                memcpy(text, msg->text, (len + 1) * sizeof(WCHAR));
                if (len && text[len - 1] == L'.') {

                    while (len) {
                        --len;
                        if (! len)
                            break;
                        if (text[len - 1] != L'\n' &&
                            text[len - 1] != L'\r')
                            break;
                    }

                    text[len] = L'\0';
                }

                return text;
            }

            msg = List_Next(msg);
        }
    }

    return mf->text;
}


//---------------------------------------------------------------------------
// WriteNsisFiles
//---------------------------------------------------------------------------


void WriteNsisFiles(LIST *langs)
{
    struct LANG *lang;
    WCHAR text[4096];
    WCHAR uplang[64];
    HANDLE hFile;
    LIST *English;
    struct MSG *me, *mf;

    //
    // create output file
    //

    lang = List_Head(langs);
    English = &lang->msgs;
    while (lang) {

        swprintf(text, L"SbieRelease\\NsisText_%s.txt", lang->name);

        hFile = CreateFile(text, FILE_GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            fprintf(stderr, "can\'t create output file - %S\n", text);
            exit(EXIT_FAILURE);
        }

        text[0] = 0xFEFF;
        text[1] = L'\0';
        Put(hFile, text);

        wcscpy(uplang, lang->name);
        _wcsupr(uplang);

        me = List_Head(English);
        while (me) {

            ULONG fac = (me->code >> 16) & 0xFFF;
            if (fac == 0xFFF) {

                WCHAR *iptr, *optr;

                mf = FindTextEntry(&lang->msgs, me->code, me->ver);
                if (mf) {

                    swprintf(text, L"LangString MSG_%04d ${LANG_%s} \"",
                             mf->code & 0xFFFF, uplang);

                    iptr = FindCopyTextForNsis(mf, &lang->msgs);
                    optr = text + wcslen(text);
                    while (*iptr) {

                        if ((*iptr == L'\r') || (*iptr == L'\n')) {
                            ++iptr;
                            if ((*iptr == L'\r') || (*iptr == L'\n'))
                                ++iptr;
                            wcscpy(optr, L"$\\n");
                            optr += 3;

                        } else {
                            *optr = *iptr;
                            ++iptr;
                            ++optr;
                        }
                    }

                    wcscpy(optr, L"\"\r\n");

                    Put(hFile, text);
                }
            }

            me = List_Next(me);
        }

        CloseHandle(hFile);

        lang = List_Next(lang);
    }
}


//---------------------------------------------------------------------------
// main
//---------------------------------------------------------------------------


int __cdecl main(int argc, char *argv[])
{
    static const UCHAR *EnglishFileName = "Text-English-1033.txt";
    int i;
    LIST English;
    LIST Langs;

    ReadTextFile(EnglishFileName, &English);
    DiscardOldText(EnglishFileName, &English);

    List_Init(&Langs);
    AddLanguage(&Langs, &English, EnglishFileName);

    for (i = 1; i < argc; ++i) {

        UCHAR *dot = strrchr(argv[i], '.');
        if ((! dot) ||
            _stricmp(dot, ".txt") != 0 ||
            _strnicmp(argv[i], "Text-", 5) != 0)
        {
            fprintf(stderr, "ignoring file - %s\n", argv[i]);
            continue;
        }

        if (_stricmp(argv[i], EnglishFileName) != 0) {

            LIST Foreign;
            ReadTextFile(argv[i], &Foreign);
            DiscardOldText(argv[i], &Foreign);
            CompareText(argv[i], &Foreign, &English);

            AddLanguage(&Langs, &Foreign, argv[i]);
        }
    }

    WriteMessageFile(&Langs);
    WriteNsisFiles(&Langs);

    return 0;
}
