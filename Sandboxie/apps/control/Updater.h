/*
 * Copyright 2020 David Xanatos, xanasoft.com
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
 // Updater
 //---------------------------------------------------------------------------


#ifndef _MY_UPDATER_H
#define _MY_UPDATER_H

//#define WM_UPDATERESULT     (WM_APP + 10)

typedef struct _UPDATER_DATA
{
	CString userMsg;
	CString infoUrl;

	CString version;
	//unsigned __int64 updated;
	CString updateMsg;
	CString updateUrl;
	CString downloadUrl;

	BOOLEAN Manual;
	ULONG ErrorCode;

} UPDATER_DATA, *PUPDATER_DATA;

extern const WCHAR *_UpdateCheckNotify;
extern const WCHAR *_NextUpdateCheck;

class CUpdater
{
	static CUpdater *m_instance;

	bool m_update_pending;

	OSVERSIONINFOW m_osvi;

	CUpdater();

	BOOLEAN DownloadUpdateData(const WCHAR* Host, const WCHAR* Path, PSTR* pData, ULONG* dataLength);
	BOOLEAN QueryUpdateData(UPDATER_DATA* Context);
	CString DownloadUpdate(const CString& downloadUrl);

	static ULONG UpdaterServiceThread(void *lpParameter);

public:

	~CUpdater();

	static CUpdater &GetInstance();

	bool CheckUpdates(CWnd *pParentWnd, bool bManual = true);


};

#endif // _MY_UPDATER_H