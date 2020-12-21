#pragma once
#include <qwidget.h>
#include "../ApiLog.h"
#include "../../MiscHelpers/Common/ListItemModel.h"

class CApiMonModel : public CListItemModel
{
    Q_OBJECT

public:
    CApiMonModel(QObject *parent = 0);
	~CApiMonModel();

	void			Sync(const QList<CApiLogEntryPtr>& List, QSet<quint64> PIDs);
	
	CApiLogEntryPtr	GetEntry(const QModelIndex &index) const;

    int				columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant		headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	enum EColumns
	{
		eProcess = 0,
		eTimeStamp,
		eMessage,
		eCount
	};

protected:
	struct SApiLogNode: SListNode
	{
		SApiLogNode(const QVariant& Id) : SListNode(Id), iColor(0) {}

		CApiLogEntryPtr		pEntry;

		int					iColor;
	};

	virtual SListNode* MkNode(const QVariant& Id) { return new SApiLogNode(Id); }
};