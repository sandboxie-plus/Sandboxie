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

struct QSBIEAPI_EXPORT SBoxSnapshot
{
	QString ID;
	QString Parent;

	QString NameStr;
	QString InfoStr;
	QDateTime SnapDate;
};

class QSBIEAPI_EXPORT CSandBox : public CSbieIni
{
	Q_OBJECT
public:
	CSandBox(const QString& BoxName, class CSbieAPI* pAPI);
	virtual ~CSandBox();

	virtual void					UpdateDetails();

	virtual void					SetBoxPaths(const QString& FilePath, const QString& RegPath, const QString& IpcPath);
	virtual QString					GetFileRoot() const { return m_FilePath; }
	virtual QString					GetRegRoot() const { return m_RegPath; }
	virtual QString					GetIpcRoot() const { return m_IpcPath; }
	virtual QString					GetMountRoot() const { return m_Mount; }

	virtual QMap<quint32, CBoxedProcessPtr>	GetProcessList() const { return m_ProcessList; }

	virtual int						GetActiveProcessCount() const { return m_ActiveProcessCount; }

	virtual SB_STATUS				RunStart(const QString& Command, bool Elevated = false);
	virtual SB_STATUS				RunSandboxed(const QString& Command);
	virtual SB_STATUS				TerminateAll();

	virtual void					OpenBox() {}
	virtual void					CloseBox() {}

	virtual bool					IsEnabled() const  { return m_IsEnabled; }

	virtual bool					IsEmpty() const;
	virtual bool					IsInitialized() const;
	virtual bool					HasSnapshots() const;
	virtual SB_PROGRESS				CleanBox();
	virtual SB_STATUS				RenameBox(const QString& NewName);
	virtual SB_STATUS				RemoveBox();

	virtual QString					Expand(const QString& Value);

	virtual QMap<QString, SBoxSnapshot>	GetSnapshots(QString* pCurrent = NULL, QString* pDefault = NULL) const;
	virtual void					SetDefaultSnapshot(QString Default);
	virtual QString					GetDefaultSnapshot(QString* pCurrent = NULL) const;
	virtual SB_PROGRESS				TakeSnapshot(const QString& Name);
	virtual SB_PROGRESS				RemoveSnapshot(const QString& ID);
	virtual SB_PROGRESS				SelectSnapshot(const QString& ID);
	virtual SB_STATUS				SetSnapshotInfo(const QString& ID, const QString& Name, const QString& Description = QString());

	// Mount Manager
	virtual SB_STATUS				ImBoxCreate(quint64 uSizeKb, const QString& Password = QString());
	virtual SB_STATUS				ImBoxMount(const QString& Password = QString(), bool bProtect = false, bool bAutoUnmount = false);
	virtual SB_STATUS				ImBoxUnmount();

	class CSbieAPI*					Api() { return m_pAPI; }

protected:
	friend class CSbieAPI;

	SB_PROGRESS						CleanBoxFolders(const QStringList& BoxFolders);
	static void						CleanBoxAsync(const CSbieProgressPtr& pProgress, const QStringList& BoxFolders);

	static void						DeleteSnapshotAsync(const CSbieProgressPtr& pProgress, const QString& BoxPath, const QString& ID);
	static void						MergeSnapshotAsync(const CSbieProgressPtr& pProgress, const QString& BoxPath, const QString& TargetID, const QString& SourceID, const QPair<const QString, class CSbieAPI*>& params);

	QString							m_FilePath;
	QString							m_RegPath;
	QString							m_IpcPath;
	QString							m_Mount;
	
	bool							m_IsEnabled;
	
	QMap<quint32, CBoxedProcessPtr>	m_ProcessList;
	int								m_ActiveProcessCount;
	bool							m_ActiveProcessDirty;

//private:
//	struct SSandBox* m;
};

typedef QSharedPointer<CSandBox> CSandBoxPtr;
typedef QWeakPointer<CSandBox> CSandBoxRef;
