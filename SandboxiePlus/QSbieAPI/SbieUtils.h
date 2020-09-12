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

	static SB_STATUS		Start(EComponent Component);
	static SB_STATUS		Stop(EComponent Component);
	static bool				IsRunning(EComponent Component);

	static SB_STATUS		Install(EComponent Component);
	static SB_STATUS		Uninstall(EComponent Component);
	static bool				IsInstalled(EComponent Component);

	static int				IsContextMenu();
	static void				AddContextMenu(const QString& StartPath);
	static void				RemoveContextMenu();

private:
	static SB_STATUS		ElevateOps(const QStringList& Ops);
	static SB_STATUS		ExecOps(const QStringList& Ops);

	static void				Start(EComponent Component, QStringList& Ops);
	static void				Stop(EComponent Component, QStringList& Ops);

	static void				Install(EComponent Component, QStringList& Ops);
	static void				Uninstall(EComponent Component, QStringList& Ops);

	static bool				CheckRegValue(const wchar_t* key);
	static void				CreateShellEntry(const wstring& classname, const wstring& cmdtext, const wstring& iconpath, const wstring& startcmd);
};

