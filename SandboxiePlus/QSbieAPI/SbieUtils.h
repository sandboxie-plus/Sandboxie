#pragma once

#include "qsbieapi_global.h"

#include "SbieError.h"

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

private:
	static SB_STATUS		ElevateOps(const QStringList& Ops);
	static SB_STATUS		ExecOps(const QStringList& Ops);

	static void				Start(EComponent Component, QStringList& Ops);
	static void				Stop(EComponent Component, QStringList& Ops);

	static void				Install(EComponent Component, QStringList& Ops);
	static void				Uninstall(EComponent Component, QStringList& Ops);
};

