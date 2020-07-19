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
#pragma once
#include <qobject.h>

#include "../qsbieapi_global.h"

#include "BoxedProcess.h"
#include "SbieIni.h"

class QSBIEAPI_EXPORT CSandBox : public CSbieIni
{
	Q_OBJECT
public:
	CSandBox(const QString& BoxName, class CSbieAPI* pAPI);
	virtual ~CSandBox();

	virtual void					UpdateDetails();

	virtual QString					GetFileRoot() const { return m_FilePath; }
	virtual QString					GetRegRoot() const { return m_RegPath; }
	virtual QString					GetIpcRoot() const { return m_IpcPath; }

	virtual QMap<quint64, CBoxedProcessPtr>	GetProcessList() const { return m_ProcessList; }

	virtual SB_STATUS				RunStart(const QString& Command);
	virtual SB_STATUS				RunCommand(const QString& Command);
	virtual SB_STATUS				TerminateAll();

	virtual SB_STATUS				CleanBox();
	virtual SB_STATUS				RenameBox(const QString& NewName);
	virtual SB_STATUS				RemoveBox();

	class CSbieAPI*					Api() { return m_pAPI; }

signals:
	void							BoxCleaned();

protected:
	friend class CSbieAPI;

	QString							m_FilePath;
	QString							m_RegPath;
	QString							m_IpcPath;

	QMap<quint64, CBoxedProcessPtr>	m_ProcessList;

//private:
//	struct SSandBox* m;
};

typedef QSharedPointer<CSandBox> CSandBoxPtr;
typedef QWeakPointer<CSandBox> CSandBoxRef;