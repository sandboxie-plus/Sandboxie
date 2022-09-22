/*
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

//---------------------------------------------------------------------------
// File (Snapshot)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

#define FILE_MAX_SNAPSHOT_ID	17

#define FILE_INSNAPSHOT_FLAG    0x0004

//---------------------------------------------------------------------------
// Structures and Types
//---------------------------------------------------------------------------


typedef struct _FILE_SNAPSHOT {
	WCHAR					ID[FILE_MAX_SNAPSHOT_ID];
	ULONG					IDlen;
	ULONG					ScramKey;
	//WCHAR					Name[34];
	struct _FILE_SNAPSHOT*	Parent;
	LIST					PathRoot;
} FILE_SNAPSHOT, *PFILE_SNAPSHOT;


//---------------------------------------------------------------------------
// Variables
//---------------------------------------------------------------------------


static const WCHAR* File_Snapshot_Prefix = L"snapshot-";
static const ULONG  File_Snapshot_PrefixLen = 9; // wcslen(File_Snapshot_Prefix);


static FILE_SNAPSHOT *File_Snapshot = NULL;
static ULONG File_Snapshot_Count = 0;



//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

static void File_ScrambleShortName(WCHAR* ShortName, CCHAR* ShortNameLength, ULONG ScramKey);
static void File_UnScrambleShortName(WCHAR* ShortName, ULONG ScramKey);

static WCHAR* File_MakeSnapshotPath(FILE_SNAPSHOT* Cur_Snapshot, const WCHAR* CopyPath);
static WCHAR* File_FindSnapshotPath(WCHAR* CopyPath);
static WCHAR* File_ResolveTruePath(WCHAR* TruePath, WCHAR* CopyPath, ULONG* pFlags);
static ULONG File_IsDeletedEx(const WCHAR* TruePath, const WCHAR* CopyPath, FILE_SNAPSHOT* snapshot);


static void File_InitSnapshots(void);

//---------------------------------------------------------------------------
// File_Scramble_Char
//---------------------------------------------------------------------------


_FX WCHAR File_Scramble_Char(WCHAR wValue, int Key, BOOLEAN scram)
{
	//
	// This function allows to scramble file name charakters properly, 
	// i.e. no invalid cahacters can result fron this operation.
	// It does not scramble invalid charakters like: " * / : < > ? \ |
	// And it does not scramble ~
	// The entropy of the scrambler is 25,5bit (i.e. 52 million values)
	//

	char reserved_ch[] = { '\"', '*', '/', ':', '<', '>', '?', '\\', '|' };
	const int reserved_count = 9;
	const int max_ch = 0x7E - reserved_count - 0x20;

	int uValue = (wValue & 0x7F);
	if (uValue < 0x20 || uValue >= 0x7E) // < space || >= ~
		return wValue;
	for (int i = 0; i < reserved_count; i++)
		if (uValue == reserved_ch[i]) return wValue;

	Key &= 0x7f;
	while (Key >= max_ch)
		Key -= max_ch;
	if (!scram)
		Key = -Key;

	for (int i = 1; i <= reserved_count; i++)
		if (uValue > reserved_ch[reserved_count - i])	uValue -= 1;
	uValue -= 0x20;

	uValue += Key;

	if (uValue >= max_ch)
		uValue -= max_ch;
	else if (uValue < 0)
		uValue += max_ch;

	uValue += 0x20;
	for (int i = 0; i < reserved_count; i++)
		if (uValue >= reserved_ch[i])	uValue += 1;

	return uValue;
}


//---------------------------------------------------------------------------
// File_ScrambleShortName
//---------------------------------------------------------------------------


_FX void File_ScrambleShortName(WCHAR* ShortName, CCHAR* ShortNameBytes, ULONG ScramKey)
{
	CCHAR ShortNameLength = *ShortNameBytes / sizeof(WCHAR);

	CCHAR dot_pos;
	WCHAR *dot = wcsrchr(ShortName, L'.');
	if (dot == NULL) {
		dot_pos = ShortNameLength;
		if (ShortNameLength >= 12)
			return; // this should never not happen!
		ShortName[ShortNameLength++] = L'.';
	}
	else
		dot_pos = (CCHAR)(dot - ShortName);

	while (ShortNameLength - dot_pos < 4)
	{
		if (ShortNameLength >= 12)
			return; // this should never not happen!
		ShortName[ShortNameLength++] = L' ';
	}

	*ShortNameBytes = ShortNameLength * sizeof(WCHAR);

	if (dot_pos > 0)
		ShortName[dot_pos - 1] = File_Scramble_Char(ShortName[dot_pos - 1], ((char*)&ScramKey)[0], TRUE);
	for (int i = 1; i <= 3; i++)
		ShortName[dot_pos + i] = File_Scramble_Char(ShortName[dot_pos + i], ((char*)&ScramKey)[i], TRUE);
}


//---------------------------------------------------------------------------
// File_UnScrambleShortName
//---------------------------------------------------------------------------


_FX void File_UnScrambleShortName(WCHAR* ShortName, ULONG ScramKey)
{
	CCHAR ShortNameLength = (CCHAR)wcslen(ShortName);

	WCHAR *dot = wcsrchr(ShortName, L'.');
	if (dot == NULL)
		return; // not a scrambled short name.
	CCHAR dot_pos = (CCHAR)(dot - ShortName);

	if (dot_pos > 0)
		ShortName[dot_pos - 1] = File_Scramble_Char(ShortName[dot_pos - 1], ((char*)&ScramKey)[0], FALSE);
	for (int i = 1; i <= 3; i++)
		ShortName[dot_pos + i] = File_Scramble_Char(ShortName[dot_pos + i], ((char*)&ScramKey)[i], FALSE);

	while (ShortName[ShortNameLength - 1] == L' ')
		ShortName[ShortNameLength-- - 1] = 0;
	if (ShortName[ShortNameLength - 1] == L'.')
		ShortName[ShortNameLength-- - 1] = 0;
}


//---------------------------------------------------------------------------
// File_MakeSnapshotPath
//---------------------------------------------------------------------------


_FX WCHAR* File_MakeSnapshotPath(FILE_SNAPSHOT* Cur_Snapshot, const WCHAR* CopyPath)
{
	if (!Cur_Snapshot)
		return NULL;

	ULONG prefixLen = File_FindBoxPrefixLength(CopyPath);
	if (prefixLen == 0)
		return NULL;

	THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

	WCHAR* TmplName = Dll_GetTlsNameBuffer(TlsData, TMPL_NAME_BUFFER, (wcslen(CopyPath) + File_Snapshot_PrefixLen + FILE_MAX_SNAPSHOT_ID + 1) * sizeof(WCHAR));

	wcsncpy(TmplName, CopyPath, prefixLen + 1);
	wcscpy(TmplName + prefixLen + 1, File_Snapshot_Prefix);
	wcscpy(TmplName + prefixLen + 1 + File_Snapshot_PrefixLen, Cur_Snapshot->ID);
	wcscpy(TmplName + prefixLen + 1 + File_Snapshot_PrefixLen + Cur_Snapshot->IDlen, CopyPath + prefixLen);

	return TmplName;
}


//---------------------------------------------------------------------------
// File_FindSnapshotPath
//---------------------------------------------------------------------------


_FX WCHAR* File_FindSnapshotPath(WCHAR* CopyPath)
{
	NTSTATUS status;
	OBJECT_ATTRIBUTES objattrs;
	UNICODE_STRING objname;
	ULONG FileType;

	InitializeObjectAttributes(&objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

	//
	// When working with snapshots the actual "CopyFile" may be located in a snapshot directory.
	// To deal with that when the file is not in the active box directory we look through the snapshots,
	// When we find it we update the path to point to the snapshot containing the file.
	//
	
	RtlInitUnicodeString(&objname, CopyPath);
	status = File_GetFileType(&objattrs, FALSE, &FileType, NULL);
	if (!(status == STATUS_OBJECT_NAME_NOT_FOUND || status == STATUS_OBJECT_PATH_NOT_FOUND))
		return NULL; // file is present directly in copy path

	for (FILE_SNAPSHOT* Cur_Snapshot = File_Snapshot; Cur_Snapshot != NULL; Cur_Snapshot = Cur_Snapshot->Parent)
	{
		WCHAR* TmplName = File_MakeSnapshotPath(Cur_Snapshot, CopyPath);
		if (!TmplName)
			break;
		
		RtlInitUnicodeString(&objname, TmplName);
		status = File_GetFileType(&objattrs, FALSE, &FileType, NULL);
		if (!(status == STATUS_OBJECT_NAME_NOT_FOUND || status == STATUS_OBJECT_PATH_NOT_FOUND))
		{
			return TmplName;
		}
	}

	return NULL; // this file is not in any snapshot
}


//---------------------------------------------------------------------------
// File_GetPathFlagsEx
//---------------------------------------------------------------------------


_FX ULONG File_GetPathFlagsEx(const WCHAR *TruePath, const WCHAR *CopyPath, WCHAR** pRelocation, FILE_SNAPSHOT* lastSnapshot)
{
	ULONG Flags = 0;
    WCHAR* Relocation = NULL;

	THREAD_DATA *TlsData = Dll_GetTlsData(NULL);

	if (File_Delete_v2) 
	{
		File_RefreshPathTree();

		if(!pRelocation)
			Dll_PushTlsNameBuffer(TlsData);

		EnterCriticalSection(File_PathRoot_CritSec);

		//
		// check true path relocation and deleteion for the active state
		//

		Flags = File_GetPathFlags_internal(&File_PathRoot, TruePath, &Relocation, TRUE); // this requires a name buffer
		if (FILE_PATH_DELETED(Flags))
			goto finish;
	}

	if (!File_Snapshot) 
	{
		if (pRelocation) *pRelocation = Relocation; // return a MISC_NAME_BUFFER buffer valid at the current name buffer depth

		goto finish;
	}

	//
	// Handle snapshots
	//

	NTSTATUS status;
	OBJECT_ATTRIBUTES objattrs;
	UNICODE_STRING objname;
	ULONG FileType;

	InitializeObjectAttributes(&objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

	//
	// we need a few helper buffers here, to make it efficient we will exploit
	// an implementation artefact of the TlsNameBuffer mechanism, namely
	// the property that after a pop the buffers remain valid untill the next push
	// 
	// so we can pop out of the current frame request a buffer of the required size
	// and still read from the buffer that was filled in the previosue frame
	//

	Dll_PushTlsNameBuffer(TlsData);

	WCHAR* TmplRelocation = Relocation;

	for (FILE_SNAPSHOT* Cur_Snapshot = File_Snapshot; Cur_Snapshot != lastSnapshot; Cur_Snapshot = Cur_Snapshot->Parent)
	{
		if (TmplRelocation) 
		{
			//
			// update the true file name
			//

			TruePath = Dll_GetTlsNameBuffer(TlsData, TRUE_NAME_BUFFER, (wcslen(TmplRelocation) + 1) * sizeof(WCHAR));
			wcscpy((WCHAR*)TruePath, TmplRelocation);

			if (CopyPath) 
			{
				//
				// update the copy file name
				//

				Dll_PushTlsNameBuffer(TlsData);

				WCHAR* TruePath2, * CopyPath2;
				RtlInitUnicodeString(&objname, TmplRelocation);
				File_GetName(NULL, &objname, &TruePath2, &CopyPath2, NULL);

				Dll_PopTlsNameBuffer(TlsData);

				// note: pop leaves TruePath2 valid we can still use it

				CopyPath = Dll_GetTlsNameBuffer(TlsData, COPY_NAME_BUFFER, (wcslen(CopyPath2) + 1) * sizeof(WCHAR));
				wcscpy((WCHAR*)CopyPath, CopyPath2);
			}
		}

		if (CopyPath) 
		{
			//
			// check if the specified file is present in the current snapshot
			//

			WCHAR* TmplName = File_MakeSnapshotPath(Cur_Snapshot, CopyPath);
			if (!TmplName)
				break; // something went wrong

			RtlInitUnicodeString(&objname, TmplName);
			status = File_GetFileType(&objattrs, FALSE, &FileType, NULL);
			if (!(status == STATUS_OBJECT_NAME_NOT_FOUND || status == STATUS_OBJECT_PATH_NOT_FOUND)) 
			{
				Flags |= FILE_INSNAPSHOT_FLAG;
				Relocation = TmplName;
				goto complete;
			}
		}

		if (File_Delete_v2) 
		{
			//
			// check true path relocation and deleteion for the current snapshot
			//

			TmplRelocation = NULL;
			Flags = File_GetPathFlags_internal(&Cur_Snapshot->PathRoot, TruePath, &TmplRelocation, TRUE);
			if(TmplRelocation)
				Relocation = TmplRelocation;
			if (FILE_PATH_DELETED(Flags))
				goto complete;
		}
	}

complete:
	Dll_PopTlsNameBuffer(TlsData);

	// note: pop leaves the buffers valid we can still use them

	if (pRelocation && Relocation) // return a new TMPL_NAME_BUFFER buffer valid at the current name buffer depth
	{
		*pRelocation = Dll_GetTlsNameBuffer(TlsData, TMPL_NAME_BUFFER, (wcslen(Relocation) + 1) * sizeof(WCHAR));
		wcscpy(*pRelocation, Relocation);
	}

finish:

	if (File_Delete_v2) 
	{
		LeaveCriticalSection(File_PathRoot_CritSec);

		if(!pRelocation)
			Dll_PopTlsNameBuffer(TlsData);
	}

	return Flags;
}


//---------------------------------------------------------------------------
// File_ResolveTruePath
//---------------------------------------------------------------------------


_FX WCHAR* File_ResolveTruePath(WCHAR *TruePath, WCHAR *CopyPath, ULONG* pFlags)
{
	WCHAR* Relocation = NULL;
	ULONG Flags = File_GetPathFlagsEx(TruePath, CopyPath, &Relocation, NULL);

	if (pFlags) *pFlags = Flags;
	return Relocation;
}


//---------------------------------------------------------------------------
// File_IsDeletedEx
//---------------------------------------------------------------------------


_FX ULONG File_IsDeletedEx(const WCHAR* TruePath, const WCHAR* CopyPath, FILE_SNAPSHOT* snapshot)
{
	ULONG Flags = File_GetPathFlagsEx(TruePath, CopyPath, NULL, snapshot);

	return (Flags & FILE_DELETED_MASK);
}


//---------------------------------------------------------------------------
// File_InitSnapshots
//---------------------------------------------------------------------------

// CRC
#define CRC_WITH_ADLERTZUK64
#include "common/crc.c"

_FX void File_InitSnapshots(void)
{
	WCHAR ShapshotsIni[MAX_PATH] = { 0 };
	wcscpy(ShapshotsIni, Dll_BoxFilePath);
	wcscat(ShapshotsIni, L"\\Snapshots.ini");
	SbieDll_TranslateNtToDosPath(ShapshotsIni);

	WCHAR Shapshot[FILE_MAX_SNAPSHOT_ID] = { 0 };
	GetPrivateProfileStringW(L"Current", L"Snapshot", L"", Shapshot, FILE_MAX_SNAPSHOT_ID, ShapshotsIni);

	if (*Shapshot == 0)
		return; // not using snapshots

	File_Snapshot = Dll_Alloc(sizeof(FILE_SNAPSHOT));
	memzero(File_Snapshot, sizeof(FILE_SNAPSHOT));
	wcscpy(File_Snapshot->ID, Shapshot);
	File_Snapshot->IDlen = wcslen(Shapshot);
	FILE_SNAPSHOT* Cur_Snapshot = File_Snapshot;
	File_Snapshot_Count = 1;

	for (;;)
	{
		Cur_Snapshot->ScramKey = CRC32(Cur_Snapshot->ID, Cur_Snapshot->IDlen * sizeof(WCHAR));

		WCHAR ShapshotId[26] = L"Snapshot_";
		wcscat(ShapshotId, Shapshot);
		
		if (File_Delete_v2) 
		{
			WCHAR PathFile[MAX_PATH];
			wcscpy(PathFile, File_Snapshot_Prefix);
			wcscat(PathFile, Cur_Snapshot->ID);
			wcscat(PathFile, L"\\");
			wcscat(PathFile, FILE_PATH_FILE_NAME);

			File_LoadPathTree_internal(&Cur_Snapshot->PathRoot, PathFile);
		}

		//WCHAR ShapshotName[34] = { 0 };
		//GetPrivateProfileStringW(ShapshotId, L"Name", L"", ShapshotName, 34, ShapshotsIni);
		//wcscpy(Cur_Snapshot->Name, ShapshotName);

		GetPrivateProfileStringW(ShapshotId, L"Parent", L"", Shapshot, 16, ShapshotsIni);

		if (*Shapshot == 0)
			break; // no more snapshots

		Cur_Snapshot->Parent = Dll_Alloc(sizeof(FILE_SNAPSHOT));
		memzero(Cur_Snapshot->Parent, sizeof(FILE_SNAPSHOT));
		wcscpy(Cur_Snapshot->Parent->ID, Shapshot);
		Cur_Snapshot->Parent->IDlen = wcslen(Shapshot);
		Cur_Snapshot = Cur_Snapshot->Parent;
		File_Snapshot_Count++;
	}
}
