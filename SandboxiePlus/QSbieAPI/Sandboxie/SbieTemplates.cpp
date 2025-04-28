/*
 *
 * Copyright (c) 2020-2025, David Xanatos
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
#include "../SbieUtils.h"

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

void CSbieTemplates::RunCheck()
{
	CollectObjects();
	CollectClasses();
	CollectServices();
	CollectProducts();

	CollectTemplates();

	QStringList Used = m_pAPI->GetGlobalSettings()->GetTextList("Template", false);
	QStringList Rejected = m_pAPI->GetGlobalSettings()->GetTextList("TemplateReject", false);

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
}

void CSbieTemplates::SetCheckResult(const QStringList& Result)
{
	CollectTemplates();

	QStringList Used = m_pAPI->GetGlobalSettings()->GetTextList("Template", false);
	QStringList Rejected = m_pAPI->GetGlobalSettings()->GetTextList("TemplateReject", false);

	for(QMap<QString, int>::iterator I = m_Templates.begin(); I != m_Templates.end(); ++I)
	{
		int Value = eNone;
		if (Used.contains(I.key(), Qt::CaseInsensitive))
			Value |= eEnabled;
		if (Result.contains(I.key()))
			Value |= eRequired;
		if (Rejected.contains(I.key() , Qt::CaseInsensitive))
			Value |= eDisabled;
		I.value() = Value;
	}
}

bool CSbieTemplates::GetCheckState()
{
	for (QMap<QString, int>::iterator I = m_Templates.begin(); I != m_Templates.end(); ++I)
	{
		if ((I.value() & eRequired) != 0 && (I.value() & eConfigured) == 0)
			return true;
	}
	return false;
}

void CSbieTemplates::Reset()
{
	m_Objects.clear();
	m_Classes.clear();
	m_Services.clear();
	m_Products.clear();
}

QStringList CSbieTemplates::GetObjects() 
{ 
	if (m_Objects.isEmpty())
		CollectObjects();
	return m_Objects; 
}

QStringList CSbieTemplates::GetClasses() 
{ 
	if (m_Classes.isEmpty())
		CollectClasses();
	return m_Classes; 
}

QStringList CSbieTemplates::GetServices() 
{ 
	if (m_Services.isEmpty())
		CollectServices();
	return m_Services; 
}

QStringList CSbieTemplates::GetProducts() 
{ 
	if (m_Products.isEmpty())
		CollectProducts();
	return m_Products; 
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
		std::wstring wobjdir = objdir.toStdWString();
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

				info_len *= 2;
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
				m_Objects.append(objpath.toLower());
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
		GetClassNameW(hwnd, clsnm, 250);
		clsnm[250] = L'\0';

		if (clsnm[0] && wcsncmp(clsnm, L"Sandbox:", 8) != 0)
		{
			_wcslwr(clsnm);
			((CSbieTemplates*)lparam)->m_Classes.append(QString::fromWCharArray(clsnm));
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
	ENUM_SERVICE_STATUSW* info = (ENUM_SERVICE_STATUSW *)malloc(info_len);

	ULONG ResumeHandle = 0;
	for(;;)
	{
		ULONG len;
		ULONG num;
		BOOL ret = EnumServicesStatusW(hManager, SERVICE_TYPE_ALL, SERVICE_STATE_ALL, info, info_len, &len, &num, &ResumeHandle);
		if (!ret && GetLastError() != ERROR_MORE_DATA)
			break;

		for (ULONG i = 0; i < num; ++i)
		{
			_wcslwr(info[i].lpServiceName);
			m_Services.append(QString::fromWCharArray(info[i].lpServiceName));
		}

		if (ret)
			break;
	}

	free(info);

	CloseServiceHandle(hManager);
}

void CSbieTemplates::CollectProducts()
{
	BOOL is64BitOperatingSystem;
#ifdef _WIN64
	is64BitOperatingSystem = TRUE;
#else // ! _WIN64
	is64BitOperatingSystem = CSbieAPI::IsWow64();
#endif _WIN64

	m_Products.clear();

	QList<HKEY> Roots = QList<HKEY>() << HKEY_LOCAL_MACHINE << HKEY_CURRENT_USER;
	for (auto Root : Roots) 
	{
		ULONG DesiredAccess = KEY_READ;
		for (;;)
		{
			HKEY hkey;
			LONG rc = RegOpenKeyExW(Root, L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall", 0, DesiredAccess, &hkey);
			if (rc != 0)
				break;

			WCHAR name[128];
			for (ULONG index = 0; rc != ERROR_NO_MORE_ITEMS; index++)
			{
				ULONG name_len = 120;
				rc = RegEnumKeyExW(hkey, index, name, &name_len, NULL, NULL, NULL, NULL);
				if (rc == 0) {
					_wcslwr(name);
					m_Products.append(QString::fromWCharArray(name));
				}
			}

			RegCloseKey(hkey);

#ifdef _WIN64
			if (DesiredAccess & KEY_WOW64_32KEY)
				break;
			DesiredAccess |= KEY_WOW64_32KEY;
#else // ! _WIN64
			if (!is64BitOperatingSystem || (DesiredAccess & KEY_WOW64_64KEY))
				break;
			DesiredAccess |= KEY_WOW64_64KEY;
#endif _WIN64
		}
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

bool CSbieTemplates::CheckTemplate(const QString& Name)
{
	QSharedPointer<CSbieIni> pTemplate = QSharedPointer<CSbieIni>(new CSbieIni("Template_" + Name, m_pAPI));

	QString scan = pTemplate->GetText("Tmpl.Scan", QString(), false, false, true);
	BOOL scanIpc = (scan.indexOf(L'i') != -1);
	BOOL scanWindow = (scan.indexOf(L'w') != -1);
	BOOL scanSoftware = (scan.indexOf(L's') != -1);
	if (!(scanIpc || scanWindow || scanSoftware))
		return false;

	QList<QPair<QString, QString>> settings = pTemplate->GetIniSection(0, true);
	for(QList<QPair<QString, QString>>::iterator I = settings.begin(); I != settings.end(); ++I)
	{
		QString setting = I->first;
		QString value = I->second;

		if (scanIpc && ((setting.compare("OpenIpcPath", Qt::CaseInsensitive) == 0) || setting.compare("Tmpl.ScanIpc", Qt::CaseInsensitive) == 0))
		{
			if (value.compare("\\RPC Control\\epmapper") == 0)
				continue;
			if (value.compare("\\RPC Control\\OLE*") == 0)
				continue;
			if (value.compare("\\RPC Control\\LRPC*") == 0)
				continue;
			if (value.compare("*\\BaseNamedObjects*\\NamedBuffer*mAH*Process*API*") == 0)
				continue;

			if (CheckObjects(value))
				return true;
		}
		else if (scanWindow && ((setting.compare("OpenWinClass", Qt::CaseInsensitive) == 0 || setting.compare("Tmpl.ScanWinClass", Qt::CaseInsensitive) == 0)))
		{
			// skip to unspecific entries
			if(value.left(2).compare("*:") == 0)
				continue;

			if (CheckClasses(value))
				return true;
		}
		else if (scanSoftware && setting.compare("Tmpl.ScanService", Qt::CaseInsensitive) == 0)
		{
			if (CheckServices(value))
				return true;
		}
		else if (scanSoftware && setting.compare("Tmpl.ScanProduct", Qt::CaseInsensitive) == 0)
		{
			if (CheckProducts(value))
				return true;
		}
		else if (scanSoftware && setting.compare("Tmpl.ScanKey", Qt::CaseInsensitive) == 0)
		{
			if (CheckRegistryKey(value))
				return true;
		}
		else if (scanSoftware && setting.compare("Tmpl.ScanFile", Qt::CaseInsensitive) == 0)
		{
			if (CheckFile(ExpandPath(value)))
				return true;
		}
	}

	return false;
}

bool CSbieTemplates::CheckRegistryKey(const QString& Value)
{
	QString KeyPath = Value;

	if (KeyPath.startsWith("HKEY_LOCAL_MACHINE", Qt::CaseInsensitive))			KeyPath.replace(0, 18, "\\REGISTRY\\MACHINE");
	else if (KeyPath.startsWith("HKEY_CLASSES_ROOT", Qt::CaseInsensitive))		KeyPath.replace(0, 17, "\\REGISTRY\\MACHINE\\SOFTWARE\\Classes");
	else if (KeyPath.startsWith("HKEY_CURRENT_USER", Qt::CaseInsensitive))		KeyPath.replace(0, 17, "\\REGISTRY\\USER\\" + m_pAPI->GetCurrentUserSid());
	else if (KeyPath.startsWith("HKEY_USERS", Qt::CaseInsensitive))				KeyPath.replace(0, 10, "\\REGISTRY\\USER");
	//else if (KeyPath.startsWith("HKEY_CURRENT_CONFIG", Qt::CaseInsensitive))	KeyPath.replace(0, 19, "\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Hardware Profiles\\Current");
	else if(!KeyPath.startsWith("\\REGISTRY\\", Qt::CaseInsensitive))
		return false;

	qDebug() << Value << KeyPath;

	std::wstring keypath = KeyPath.toStdWString();

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
	std::wstring path = Value.toStdWString();
	if (GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES)
		return true;
	return false;
}

bool CSbieTemplates::CheckClasses(const QString& value)
{
	QString Value = value.toLower();
	for (auto I = m_Classes.begin(); I != m_Classes.end(); ++I)
	{
		if (CSbieUtils::WildCompare(Value, *I))
			return true;
	}
	return false;
}

bool CSbieTemplates::CheckServices(const QString& value)
{
	QString Value = value.toLower();
	for (auto I = m_Services.begin(); I != m_Services.end(); ++I)
	{
		if (CSbieUtils::WildCompare(Value, *I))
			return true;
	}
	return false;
}

bool CSbieTemplates::CheckProducts(const QString& value)
{
	QString Value = value.toLower();
	for (auto I = m_Products.begin(); I != m_Products.end(); ++I)
	{
		if (CSbieUtils::WildCompare(Value, *I))
			return true;
	}
	return false;
}

bool CSbieTemplates::CheckObjects(const QString& value)
{
	QString Value = value.toLower();
	for (auto I = m_Objects.begin(); I != m_Objects.end(); ++I)
	{
		if (CSbieUtils::WildCompare(Value, *I))
			return true;
	}
	return false;
}

void CSbieTemplates::InitExpandPaths(bool WithUser)
{
	std::wstring keyPath(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\");
	if (WithUser)
		keyPath += L"User ";
	keyPath += L"Shell Folders";

	HKEY hkey;
	LONG rc = RegOpenKeyW(HKEY_CURRENT_USER, keyPath.c_str(), &hkey);
	for (ULONG index = 0; rc == 0; index++)
	{
		WCHAR name[64];
		WCHAR value[MAX_PATH + 8];
		ULONG name_len;
		ULONG value_len;
		ULONG type;

		name_len = 60;
		value_len = MAX_PATH + 4;
		rc = RegEnumValueW(hkey, index, name, &name_len, NULL, &type, (BYTE *)value, &value_len);
		if (rc == 0 && (type == REG_SZ || type == REG_EXPAND_SZ)) 
		{
			WCHAR expand[MAX_PATH + 8];
			ULONG len = ExpandEnvironmentStringsW(value, expand, MAX_PATH + 4);
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