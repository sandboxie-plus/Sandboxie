#pragma once
#include <qwidget.h>
#include "../../QSbieAPI/SbieAPI.h"
#include "../../MiscHelpers/Common/TreeItemModel.h"

class CTraceModel : public CTreeItemModel
{
	Q_OBJECT

public:
	CTraceModel(QObject* parent = 0);
	~CTraceModel();

	QList<QVariant>	Sync(const QVector<CTraceEntryPtr>& EntryList, int (*Filter)(const CTraceEntryPtr&, void*), void* params);

	CTraceEntryPtr	GetEntry(const QModelIndex& index) const;

	int				columnCount(const QModelIndex& parent = QModelIndex()) const;
	QVariant		headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	void			Clear();

	enum EColumns
	{
		eProcess = 0,
		//eTimeStamp,
		eType,
		eStatus,
		eValue,
		eCount
	};

	struct SProgInfo
	{
		SProgInfo() { Dirty = true; }
		QString Name;
		bool Dirty;
		QSet<quint32> Threads;
	};
	QMap<quint32, SProgInfo>GetPids() { return m_PidMap; }

signals:
	void			NewBranche();

protected:
	struct STraceNode : STreeNode
	{
		STraceNode(const QVariant& Id) : STreeNode(Id) { bHighLight = false; }

		CTraceEntryPtr		pEntry;
		bool				bHighLight;
	};

	QVariant				m_LastID;
	int						m_LastCount;

	virtual STreeNode* MkNode(const QVariant& Id) { return new STraceNode(Id); }
	virtual STreeNode* MkVirtualNode(const QVariant& Id, STreeNode* pParent);

	/*QList<QVariant>			MakePath(const CTraceEntryPtr& pEntry, const QList<CTraceEntryPtr>& EntryList);
	bool					TestPath(const QList<QVariant>& Path, const CTraceEntryPtr& pEntry, const QList<CTraceEntryPtr>& EntryList, int Index = 0);*/

	void					SetProcessName(const QString& Name, quint32 pid, quint32 tid);
	QString					GetProcessName(quint32 pid);
	void					LogThreadId(quint32 pid, quint32 tid);
	QMap<quint32, SProgInfo>m_PidMap;
};
