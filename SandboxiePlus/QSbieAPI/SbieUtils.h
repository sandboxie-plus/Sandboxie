#pragma once

#include "qsbieapi_global.h"

#include "SbieStatus.h"

class QSBIEAPI_EXPORT CSbieUtils
{
public:

	enum EComponent
	{
		eNone = 0,
		eDriver = 0x01,
		eService = 0x02,
		eAll = 0xFF
	};

	static SB_STATUS		DoAssist();

	static SB_RESULT(void*)	Start(EComponent Component);
	static SB_RESULT(void*)	Stop(EComponent Component);
	static bool				IsRunning(EComponent Component);

	static SB_RESULT(void*)	Install(EComponent Component);
	static SB_RESULT(void*)	Uninstall(EComponent Component);
	static bool				IsInstalled(EComponent Component);

	static QString			GetContextMenuStartCmd();
	static void				AddContextMenu(const QString& StartPath, const QString& RunStr, /*const QString& ExploreStr,*/ const QString& IconPath = QString());
	static void				RemoveContextMenu();

	static bool 			HasContextMenu2();
	static void				AddContextMenu2(const QString& StartPath, const QString& RunStr, const QString& IconPath = QString());
	static void				RemoveContextMenu2();

	static bool				CreateShortcut(class CSbieAPI* pApi, QString LinkPath, const QString &LinkName, const QString &boxname, const QString &arguments, const QString &iconPath = QString(), int iconIndex = 0, const QString &workdir = QString(), bool bRunElevated = false);
	static bool				GetStartMenuShortcut(class CSbieAPI* pApi, QString &BoxName, QString &LinkPath, QString &IconPath, quint32& IconIndex, QString &WorkDir);

	static CSbieProgressPtr RunCommand(const QString& Command, bool noGui = false);

private:
	static SB_RESULT(void*)	ElevateOps(const QStringList& Ops);
	static SB_STATUS		ExecOps(const QStringList& Ops);

	static void				Start(EComponent Component, QStringList& Ops);
	static void				Stop(EComponent Component, QStringList& Ops);

	static void				Install(EComponent Component, QStringList& Ops);
	static void				Uninstall(EComponent Component, QStringList& Ops);

	static void				CreateShellEntry(const std::wstring& classname, const std::wstring& key, const std::wstring& cmdtext, const std::wstring& iconpath, const std::wstring& startcmd);
};


bool QSBIEAPI_EXPORT ShellOpenRegKey(const QString& KeyName);
