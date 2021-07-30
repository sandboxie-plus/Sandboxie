#pragma once
#include "TreeViewEx.h"

#include "../mischelpers_global.h"

class MISCHELPERS_EXPORT CTreeItemModel : public QAbstractItemModelEx
{
    Q_OBJECT

public:
    CTreeItemModel(QObject *parent = 0);
	virtual ~CTreeItemModel();

	void			SetTree(bool bTree)				{ m_bTree = bTree; }
	bool			IsTree() const					{ return m_bTree; }
	void			SetUseIcons(bool bUseIcons)		{ m_bUseIcons = bUseIcons; }
	static void		SetDarkMode(bool bDark)			{ m_DarkMode = bDark;}

	//void			CountItems();
	QModelIndex		FindIndex(const QVariant& ID);
	void			RemoveIndex(const QModelIndex &index);
	int				Count() const					{ return m_Map.count(); }

	QVariant		GetItemID(const QModelIndex& index) const;

	QVariant		Data(const QModelIndex &index, int role, int section) const;

	// derived functions
    virtual QVariant		data(const QModelIndex &index, int role) const;
	virtual bool			setData(const QModelIndex &index, const QVariant &value, int role);
    virtual Qt::ItemFlags	flags(const QModelIndex &index) const;
    virtual QModelIndex		index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex		parent(const QModelIndex &index) const;
    virtual int				rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int				columnCount(const QModelIndex &parent = QModelIndex()) const = 0;
	virtual QVariant		headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const = 0;

public slots:
	virtual void			Clear();

signals:
	void			CheckChanged(const QVariant& ID, bool State);
	void			ToolTipCallback(const QVariant& ID, QString& ToolTip) const;
	void			Updated();

protected:
	struct STreeNode
	{
		STreeNode(const QVariant& Id){
			ID = Id;
			Parent = NULL;
			Row = 0;
			//AllChildren = 0;

			Virtual = false;

			IsBold = false;
			IsGray = false;
		}
		virtual ~STreeNode(){
			foreach(STreeNode* pNode, Children)
				delete pNode;
		}

		QVariant			ID;

		STreeNode*			Parent;
		int					Row;
		QList<QVariant>		Path;
		QList<STreeNode*>	Children;
		//int				AllChildren;
		QMap<QVariant, int>	Aux;
		bool				Virtual;

		QVariant			Icon;
		bool				IsBold;
		bool				IsGray;
		QColor				Color;
		struct SValue
		{
			QVariant Raw;
			QVariant SortKey;
			QVariant Formated;
		};
		QVector<SValue>		Values;
	};

	virtual QVariant	NodeData(STreeNode* pNode, int role, int section) const;

	virtual STreeNode*	MkNode(const QVariant& Id) = 0; // { return new STreeNode(Id); }
	virtual STreeNode*	MkVirtualNode(const QVariant& Id, STreeNode* pParent);

	void			Sync(QMap<QList<QVariant>, QList<STreeNode*> >& New, QHash<QVariant, STreeNode*>& Old, QList<QVariant>* pAdded = NULL);
	void			Purge(STreeNode* pParent, const QModelIndex &parent, QHash<QVariant, STreeNode*>& Old);
	void			Fill(STreeNode* pParent, const QModelIndex &parent, const QList<QVariant>& Paths, int PathsIndex, const QList<STreeNode*>& New, const QList<QVariant>& Path, QList<QVariant>* pAdded);
	QModelIndex		Find(STreeNode* pParent, STreeNode* pNode);
	//int				CountItems(STreeNode* pRoot);

	virtual QVariant GetDefaultIcon() const { return QVariant(); }

	STreeNode*							m_Root;
	QHash<QVariant, STreeNode*>			m_Map;
	bool								m_bTree;
	bool								m_bUseIcons;

	static bool							m_DarkMode;
};

class MISCHELPERS_EXPORT CSimpleTreeModel : public CTreeItemModel
{
	Q_OBJECT

public:
	CSimpleTreeModel(QObject *parent = 0);
	
	void					Sync(const QMap<QVariant, QVariantMap>& List);

	void					AddColumn(const QString& Name, const QString& Key) { m_ColumnKeys.append(qMakePair(Name, Key)); }

	virtual int				columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant		headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

protected:
	virtual STreeNode*		MkNode(const QVariant& Id) { return new STreeNode(Id); }

	QList<QVariant>			MakePath(const QVariantMap& Cur, const QMap<QVariant, QVariantMap>& List);
	bool					TestPath(const QList<QVariant>& Path, const QVariantMap& Cur, const QMap<QVariant, QVariantMap>& List, int Index = 0);

	QList<QPair<QString, QString>> m_ColumnKeys;
};