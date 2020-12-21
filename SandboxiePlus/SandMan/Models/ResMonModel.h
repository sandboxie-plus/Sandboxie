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

	void			Sync(const QList<CResLogEntryPtr>& List, QSet<quint64> PIDs);
	
	CResLogEntryPtr	GetEntry(const QModelIndex &index) const;

    int				columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant		headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	enum EColumns
	{
		eProcess = 0,
		eTimeStamp,
		eType,
		eValue,
		eStatus,
		eCount
	};

protected:
	struct SResLogNode: SListNode
	{
		SResLogNode(const QVariant& Id) : SListNode(Id), iColor(0) {}

		CResLogEntryPtr		pEntry;

		int					iColor;
	};

	virtual SListNode* MkNode(const QVariant& Id) { return new SResLogNode(Id); }
};