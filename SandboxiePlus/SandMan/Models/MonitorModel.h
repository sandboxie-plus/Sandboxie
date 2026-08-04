#pragma once
#include <qwidget.h>
#include "../../QSbieAPI/SbieAPI.h"
#include "../../MiscHelpers/Common/TreeItemModel.h"


class CMonitorEntry : public QSharedData
{
public:
	CMonitorEntry(const QString& Name, quint8 Type) { 
		m_Name = Name;
		m_Type = Type;
		m_Counter = 0;
		m_bOpen = false;
		m_bClosed = false;
	}

	QString		GetName() const { return m_Name; }
	QString		GetStautsStr() const {
		QString Status;
		if (m_bOpen)
			Status.append("O ");
		if (m_bClosed)
			Status.append("X ");
		return Status;
	}
	QString		GetTypeStr() const { return CTraceEntry::GetTypeStr(m_Type); }
	int			GetCount() const { return m_Counter; }

	void		Merge(const CTraceEntryPtr& pEntry) {
#ifdef USE_MERGE_TRACE
		m_Counter += pEntry->GetCount(); 
#else
		m_Counter++;
#endif
		if (!m_bOpen && pEntry->IsOpen())
			m_bOpen = true;
		if (!m_bClosed && pEntry->IsClosed())
			m_bClosed = true;
	}

protected:
	QString m_Name;
	int m_Counter;
	quint8 m_Type;
	bool m_bOpen;
	bool m_bClosed;
};

typedef QSharedDataPointer<CMonitorEntry> CMonitorEntryPtr;

class CMonitorModel : public CTreeItemModel
{
	Q_OBJECT

public:
	CMonitorModel(QObject* parent = 0);
	~CMonitorModel();

	void			SetObjTree(bool bTree)			{ m_bObjTree = bTree; }
	bool			IsObjTree() const				{ return m_bObjTree; }

	QList<QModelIndex>	Sync(const QMap<QString, CMonitorEntryPtr>& EntryMap, void* params);

	int				columnCount(const QModelIndex& parent = QModelIndex()) const;
	QVariant		headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	void			Clear();

	enum EColumns
	{
		eType = 0,
		eStatus,
		eCounter,
		eValue,
		eCount
	};

protected:
	struct STraceNode : STreeNode
	{
		STraceNode(CTreeItemModel* pModel, const QVariant& Id) : STreeNode(pModel, Id) { }

		CMonitorEntryPtr	pEntry;
	};

	virtual STreeNode*	MkNode(const QVariant& Id) { return new STraceNode(this, Id); }
	virtual STreeNode*	MkVirtualNode(const QVariant& Id, STreeNode* pParent);

	bool			m_bObjTree;
};
