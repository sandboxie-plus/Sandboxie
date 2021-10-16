#pragma once
#include "TreeViewEx.h"

#include "../mischelpers_global.h"

class MISCHELPERS_EXPORT CListItemModel : public QAbstractItemModelEx
{
    Q_OBJECT

public:
    CListItemModel(QObject *parent = 0);
	virtual ~CListItemModel();
	
	void			SetUseIcons(bool bUseIcons)		{ m_bUseIcons = bUseIcons; }
	static void		SetDarkMode(bool bDark)			{ m_DarkMode = bDark;}

	QModelIndex		FindIndex(const QVariant& ID);

	QVariant		Data(const QModelIndex &index, int role, int section) const;

	// derived functions
    virtual QVariant		data(const QModelIndex &index, int role) const;
    virtual Qt::ItemFlags	flags(const QModelIndex &index) const;
    virtual QModelIndex		index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex		parent(const QModelIndex &index) const;
    virtual int				rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int				columnCount(const QModelIndex &parent = QModelIndex()) const = 0;
	virtual QVariant		headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const = 0;

public slots:
	void			Clear();

signals:
	void			ToolTipCallback(const QVariant& ID, QString& ToolTip) const;

protected:
	struct SListNode
	{
		SListNode(const QVariant& Id)
		{
			ID = Id;

			IsBold = false;
			IsGray = false;
		}
		virtual ~SListNode() 
		{
		}

		QVariant			ID;

		QVariant			Icon;
		bool				IsBold;
		bool				IsGray;
		QColor				Color;
		struct SValue
		{
			QVariant Raw;
			QVariant Formated;
		};
		QVector<SValue>		Values;
	};

	virtual SListNode* MkNode(const QVariant& Id) = 0; // { return new SListNode(Id); }

	void Sync(QList<SListNode*>& New, QHash<QVariant, SListNode*>& Old);

	virtual QVariant GetDefaultIcon() const { return QVariant(); }

	int GetRow(SListNode* pNode) const;

	QList<SListNode*>			m_List;
	QHash<void*, int>			m_RevList;
	QHash<QVariant, SListNode*>	m_Map;
	bool						m_bUseIcons;

	static bool					m_DarkMode;
};

class MISCHELPERS_EXPORT CSimpleListModel : public CListItemModel
{
	Q_OBJECT

public:
	CSimpleListModel(QObject *parent = 0) : CListItemModel(parent) {}
	
	void			Sync(QList<QVariantMap> List);

	void			setHeaderLabels(const QStringList& Columns) { m_Headers = Columns; }

	virtual int				columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant		headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

protected:
	virtual SListNode* MkNode(const QVariant& Id) { return new SListNode(Id); }

	QStringList		m_Headers;
};