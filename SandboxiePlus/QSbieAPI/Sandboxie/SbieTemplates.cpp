/*
 *
 * Copyright (c) 2020, David Xanatos
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "stdafx.h"
#include "SbieTemplates.h"
#include "../SbieAPI.h"

#include <ntstatus.h>
#define WIN32_NO_STATUS
typedef long NTSTATUS;

#include <windows.h>
#include "SbieDefs.h"
#include "..\..\Sandboxie\common\win32_ntddk.h"
#include "..\..\Sandboxie\core\drv\api_flags.h"


CSbieTemplates::CSbieTemplates(CSbieAPI* pAPI, QObject* paretn)
	: QObject(paretn)
{
	m_pAPI = pAPI;

	InitExpandPaths(false);
	InitExpandPaths(true);
}

bool CSbieTemplates::RunCheck()
{
	CollectObjects();
	CollectClasses();
	CollectServices();
	CollectProducts();
	CollectTemplates();

	for (QMap<QString, int>::iterator I = m_Templates.begin(); I != m_Templates.end(); ++I)
	{
		if ((I.value() & eRequired) != 0 && (I.value() & eConfigured) == 0)
			return true;
	}

	return false;
}

void CSbieTemplates::CollectObjects()
{
	m_Objects.clear();

	QStringList objdirs;
	objdirs.append("\\BaseNamedObjects");
	objdirs.append("\\Sessions");
	objdirs.append("\\RPC Control");
	objdirs.append("\\Device");

	static const WCHAR *WantedTypes[] = {
		L"Directory",
		L"Event", L"Mutant", L"Section", L"Semaphore",
		L"Port", L"ALPC Port",
		L"Device",
		NULL
	};

	ULONG info_len = 0x8000;
	OBJECT_DIRECTORY_INFORMATION *info = (OBJECT_DIRECTORY_INFORMATION *)malloc(info_len);

	foreach(const QString objdir, objdirs)
	{
		wstring wobjdir = objdir.toStdWString();
		if (wobjdir.substr(0,10) == L"\\Sessions\\" && wobjdir.length() <= 13)
			wobjdir += L"\\BaseNamedObjects";

		OBJECT_ATTRIBUTES objattrs;
		UNICODE_STRING objname;
		InitializeObjectAttributes(&objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);
		RtlInitUnicodeString(&objname, wobjdir.c_str());

		HANDLE handle;
		NTSTATUS status = NtOpenDirectoryObject(&handle, DIRECTORY_QUERY, &objattrs);
		if (!NT_SUCCESS(status))
			continue;

		for (int i = 0; i < 10; i++) 
		{
			ULONG context;
			ULONG len;
			status = NtQueryDirectoryObject(handle, info, info_len, FALSE, TRUE, &context, &len);

			if (status == STATUS_MORE_ENTRIES || status == STATUS_BUFFER_OVERFLOW || status == STATUS_INFO_LENGTH_MISMATCH || status == STATUS_BUFFER_TOO_SMALL) 
			{
				free(info);

				info_len += 0x8000;
				info = (OBJECT_DIRECTORY_INFORMATION *)malloc(info_len);
				continue;
			}
			break;
		}

		NtClose(handle);
		if (!NT_SUCCESS(status))
			continue;

		for (OBJECT_DIRECTORY_INFORMATION* info_ptr = info; info_ptr->Name.Buffer; info_ptr++)
		{
			int i;
			for (i = 0; WantedTypes[i]; ++i) {
				if (_wcsicmp(info_ptr->TypeName.Buffer, WantedTypes[i]) == 0)
					break;
			}
			if (!WantedTypes[i])
				continue;

			QString objpath = objdir + "\\" + QString::fromWCharArray(info_ptr->Name.Buffer);
			if (i == 0)
				objdirs.append(objpath);
			else
				m_Objects.push_back(objpath.toLower().toStdWString());
		}
	}

	free(info);
}

void CSbieTemplates::CollectClasses()
{
	m_Classes.clear();

	EnumWindows([](HWND hwnd, LPARAM lparam) 
	{ 
		WCHAR clsnm[256];
		GetClassName(hwnd, clsnm, 250);
		clsnm[250] = L'\0';

		if (clsnm[0] && wcsncmp(clsnm, L"Sandbox:", 8) != 0)
		{
			_wcslwr(clsnm);
			((CSbieTemplates*)lparam)->m_Classes.push_back(clsnm);
		}

		return TRUE;
	}, (LPARAM)this);
}

void CSbieTemplates::CollectServices()
{
	m_Services.clear();

	SC_HANDLE hManager = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
	if (!hManager)
		return;

	ULONG info_len = 10240;
	ENUM_SERVICE_STATUS* info = (ENUM_SERVICE_STATUS *)malloc(info_len);

	ULONG ResumeHandle = 0;
	for(;;)
	{
		ULONG len;
		ULONG num;
		BOOL ret = EnumServicesStatus(hManager, SERVICE_TYPE_ALL, SERVICE_STATE_ALL, info, info_len, &len, &num, &ResumeHandle);
		if (!ret && GetLastError() != ERROR_MORE_DATA)
			break;

		for (ULONG i = 0; i < num; ++i)
		{
			_wcslwr(info[i].lpServiceName);
			m_Services.push_back(info[i].lpServiceName);
		}

		if (ret)
			break;
	}

	free(info);

	CloseServiceHandle(hManager);
}

void CSbieTemplates::CollectProducts()
{
	m_Products.clear();

	ULONG DesiredAccess = KEY_READ;
	for(;;)
	{
		HKEY hkey;
		LONG rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall", 0, DesiredAccess, &hkey);
		if (rc != 0)
			continue;
		
		WCHAR name[128];
		for(ULONG index = 0; rc != ERROR_NO_MORE_ITEMS; index++)
		{
			ULONG name_len = 120;
			rc = RegEnumKeyEx(hkey, index, name, &name_len, NULL, NULL, NULL, NULL);
			if (rc == 0) {
				_wcslwr(name);
				m_Products.push_back(name);
			}
		}

		RegCloseKey(hkey);
		
#ifdef _WIN64
		if (DesiredAccess & KEY_WOW64_32KEY)
			break;
		DesiredAccess |= KEY_WOW64_32KEY;
#else // ! _WIN64
		break;
#endif _WIN64
	}
}

QStringList CSbieTemplates::GetTemplateNames(const QString& forClass)
{
	QStringList list;

	ULONG buf_len = sizeof(WCHAR) * CONF_LINE_LEN;
	WCHAR *buf = (WCHAR*)malloc(buf_len);

	BOOL all_classes = (forClass.compare("*") == 0);

	for(int index = 0;; index++)
	{
		QString section = m_pAPI->SbieIniGet(QString(), QString(), index);
		if (section.isEmpty())
			break;

		if (section.left(9).compare("Template_", Qt::CaseInsensitive) != 0)
			continue;

		QString value = m_pAPI->SbieIniGet(section, "Tmpl.Class", CONF_GET_NO_GLOBAL);
		if (!value.isEmpty() && (all_classes || forClass.compare(value, Qt::CaseInsensitive) == 0))
			list.append(section.mid(9));
	}

	free(buf);

	return list;
}

void CSbieTemplates::CollectTemplates()
{
	m_Templates.clear();

	QStringList Templates;
	Templates.append(GetTemplateNames("EmailReader"));
	Templates.append(GetTemplateNames("Print"));
	Templates.append(GetTemplateNames("Security"));
	Templates.append(GetTemplateNames("Desktop"));
	Templates.append(GetTemplateNames("Download"));
	Templates.append(GetTemplateNames("Misc"));
	Templates.append(GetTemplateNames("WebBrowser"));
	Templates.append(GetTemplateNames("MediaPlayer"));
	Templates.append(GetTemplateNames("TorrentClient"));

	foreach(const QString& Template, Templates)
		m_Templates.insert(Template, 0);

	QStringList Used = m_pAPI->GetGlobalSettings()->GetTextList("Template", true);
	QStringList Rejected = m_pAPI->GetGlobalSettings()->GetTextList("TemplateReject", true);

	for(QMap<QString, int>::iterator I = m_Templates.begin(); I != m_Templates.end(); ++I)
	{
		int Value = eNone;
		if (Used.contains(I.key(), Qt::CaseInsensitive))
			Value |= eEnabled;
		if (CheckTemplate(I.key()))
			Value |= eRequired;
		if (Rejected.contains(I.key() , Qt::CaseInsensitive))
			Value |= eDisabled;
		I.value() = Value;
	}
}

template <typename T>
const T* wildcmpex(const T* Wild, const T* Str)
{
	const T *cp = NULL, *mp = NULL;

	while ((*Str) && (*Wild != '*'))
	{
		if ((*Wild != *Str) && (*Wild != '?'))
			return NULL;
		Wild++;
		Str++;
	}

	while (*Str)
	{
		if (*Wild == '*')
		{
			if (!*++Wild)
				return Str;
			mp = Wild;
			cp = Str + 1;
		}
		else if ((*Wild == *Str) || (*Wild == '?'))
		{
			Wild++;
			Str++;
		}
		else
		{
			Wild = mp;
			Str = cp++;
		}
	}

	while (*Wild == '*')
		Wild++;
	return *Wild ? NULL : Str;
}

bool CSbieTemplates::CheckTemplate(const QString& Name)
{
	QSharedPointer<CSbieIni> pTemplate = QSharedPointer<CSbieIni>(new CSbieIni("Template_" + Name, m_pAPI));

	QString scan = pTemplate->GetText("Tmpl.Scan");
	BOOL scanIpc = (scan.indexOf(L'i') != -1);
	BOOL scanWin = (scan.indexOf(L'w') != -1);
	BOOL scanSvc = (scan.indexOf(L's') != -1);
	if (!(scanIpc || scanWin || scanSvc))
		return false;

	list<wstring> Keys, Files;
	QList<QPair<QString, QString>> settings = pTemplate->GetIniSection(0, true);
	for(QList<QPair<QString, QString>>::iterator I = settings.begin(); I != settings.end(); ++I)
	{
		QString setting = I->first;

		list<wstring> *List = NULL;
		if (scanIpc && setting.compare("OpenIpcPath", Qt::CaseInsensitive) == 0)
			List = &m_Objects;
		else if (scanSvc && setting.compare("Tmpl.ScanIpc", Qt::CaseInsensitive) == 0)
			List = &m_Objects;
		else if (scanWin && setting.compare("OpenWinClass", Qt::CaseInsensitive) == 0)
			List = &m_Classes;
		else if (scanSvc && setting.compare("Tmpl.ScanWinClass", Qt::CaseInsensitive) == 0)
			List = &m_Classes;
		else if (scanSvc && setting.compare("Tmpl.ScanService", Qt::CaseInsensitive) == 0)
			List = &m_Services;
		else if (scanSvc && setting.compare("Tmpl.ScanProduct", Qt::CaseInsensitive) == 0)
			List = &m_Products;
		else if (scanSvc && setting.compare("Tmpl.ScanKey", Qt::CaseInsensitive) == 0)
			List = &Keys;
		else if (scanSvc && setting.compare("Tmpl.ScanFile", Qt::CaseInsensitive) == 0)
			List = &Files;
		else
			continue;

		QString value = I->second;
		if(!value.isEmpty())
		{
			if (List == &Keys) {
				if (CheckRegistryKey(value))
					return true;
				continue;
			}
			else if (List == &Files) {
				if (CheckFile(value))
					return true;
				continue;
			}

			// skip to unspecific entries
			if (List == &m_Classes)
			{
				if(value.left(2).compare("*:") == 0)
					continue;
			}
			if (List == &m_Objects)
			{
				if (value.compare("\\RPC Control\\epmapper") == 0)
					continue;
				if (value.compare("\\RPC Control\\OLE*") == 0)
					continue;
				if (value.compare("\\RPC Control\\LRPC*") == 0)
					continue;
				if (value.compare("*\\BaseNamedObjects*\\NamedBuffer*mAH*Process*API*") == 0)
					continue;
			}
			//

			wstring wild = value.toLower().toStdWString();
			for (list<wstring>::iterator I = List->begin(); I != List->end(); ++I)
			{
				if (wildcmpex(wild.c_str(), I->c_str()) != NULL)
					return true;
			}
		}
	}

	return false;
}

bool CSbieTemplates::CheckRegistryKey(const QString& Value)
{
	wstring keypath = Value.toStdWString();

	OBJECT_ATTRIBUTES objattrs;
	UNICODE_STRING objname;
	RtlInitUnicodeString(&objname, keypath.c_str());
	InitializeObjectAttributes(&objattrs, &objname, OBJ_CASE_INSENSITIVE, NULL, NULL);

	HANDLE handle;
	NTSTATUS status = NtOpenKey(&handle, KEY_QUERY_VALUE, &objattrs);
	if (NT_SUCCESS(status)) 
	{
		NtClose(handle);
		return true;
	}
	return false;
}

bool CSbieTemplates::CheckFile(const QString& Value)
{
	wstring path = ExpandPath(Value).toStdWString();
	if (GetFileAttributes(path.c_str()) != INVALID_FILE_ATTRIBUTES)
		return true;
	return false;
}

void CSbieTemplates::InitExpandPaths(bool WithUser)
{
	wstring keyPath(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\");
	if (WithUser)
		keyPath += L"User ";
	keyPath += L"Shell Folders";

	HKEY hkey;
	LONG rc = RegOpenKey(HKEY_CURRENT_USER, keyPath.c_str(), &hkey);
	for (ULONG index = 0; rc == 0; index++)
	{
		WCHAR name[64];
		WCHAR value[MAX_PATH + 8];
		ULONG name_len;
		ULONG value_len;
		ULONG type;

		name_len = 60;
		value_len = MAX_PATH + 4;
		rc = RegEnumValue(hkey, index, name, &name_len, NULL, &type, (BYTE *)value, &value_len);
		if (rc == 0 && (type == REG_SZ || type == REG_EXPAND_SZ)) 
		{
			WCHAR expand[MAX_PATH + 8];
			ULONG len = ExpandEnvironmentStrings(value, expand, MAX_PATH + 4);
			if (len > 0 && len <= MAX_PATH) 
			{
				QString value = QString::fromWCharArray(expand);
				if (!value.trimmed().isEmpty())
					m_Expands[QString::fromWCharArray(name)] = value;
			}
		}
	}

	RegCloseKey(hkey);
}

QString CSbieTemplates::ExpandPath(QString path)
{
	foreach(const QString& key, m_Expands.keys())
		path.replace("%" + key + "%", m_Expands.value(key));
	return path;
}
