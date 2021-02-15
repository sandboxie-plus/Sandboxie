#pragma once
#include <qwidget.h>
#include "../../QSbieAPI/SbieAPI.h"
#include "../../MiscHelpers/Common/ListItemModel.h"

class CResMonModel : public CListItemModel
{
    Q_OBJECT

public:
    CResMonModel(QObject *parent = 0);
	~CResMonModel();

	//void			Sync(const QList<CTraceEntryPtr>& List, QSet<quint64> PIDs);
	void			Sync(const QList<CTraceEntryPtr>& List);
	
	CTraceEntryPtr	GetEntry(const QModelIndex &index) const;

    int				columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant		headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	enum EColumns
	{
		eProcess = 0,
		eTimeStamp,
		eType,
		eStatus,
		eValue,
		eCount
	};

protected:
	struct STraceNode: SListNode
	{
		STraceNode(const QVariant& Id) : SListNode(Id), iColor(0) {}

		CTraceEntryPtr		pEntry;

		int					iColor;
	};

	virtual SListNode* MkNode(const QVariant& Id) { return new STraceNode(Id); }
};