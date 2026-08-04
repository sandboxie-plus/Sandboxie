#pragma once
#include <qwidget.h>
#include "../../MiscHelpers/Common/TreeItemModel.h"
#include "../Windows/OptionsWindow.h"

//
// CNtObjectModel from Task Explorer https://github.com/DavidXanatos/TaskExplorer
//

struct SAccessRule
{
	QString Pattern;
	QRegularExpression Exp;
	bool Star;
};

struct SAccessLists
{
	QMultiMap<COptionsWindow::EAccessMode, SAccessRule> Rules;
};

struct SAccessRules
{
	QMap<COptionsWindow::EAccessType, SAccessLists> Lists;
	bool RuleSpecificity;
};

struct SNtObjectInfo
{
	QString Name;
	QString Type;

	COptionsWindow::EAccessMode Directive;
};

class CNtObjectModel : public CTreeItemModel
{
    Q_OBJECT

public:
    CNtObjectModel(QObject *parent = 0);
	~CNtObjectModel();

	void			SetRules(const SAccessRules& Rules) { m_Rules = Rules; }

	virtual void	fetchMore(const QModelIndex &parent);
    virtual bool	canFetchMore(const QModelIndex &parent) const;
	virtual bool	hasChildren(const QModelIndex &parent = QModelIndex()) const;
	virtual int		columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant		headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	enum EColumns
	{
		eName = 0,
		eType,
		eDirective,
		eCount
	};

public slots:
	void Refresh();

protected:
	struct SNtObjectNode: STreeNode
	{
		SNtObjectNode(CTreeItemModel* pModel, const QVariant& Id) : STreeNode(pModel, Id), State(0) {}
		~SNtObjectNode() {}
		
		int				State;

		QString			ObjectPath;
		QString			ObjectType;
	};

	virtual QList<SNtObjectInfo> EnumDirectoryObjects(const QString& ObjectPath, const QString& ObjectType);
	virtual void			AddEntryToList(const QString& Name, const QString &Type, const QString& ObjectPath, QList<SNtObjectInfo>& list);

	virtual void FillNode(const struct SNtObjectInfo* pNtObject, SNtObjectNode* pChildNode);
	virtual void Refresh(SNtObjectNode* pNode, QMap<QList<QVariant>, QList<STreeNode*> >& New, QHash<QVariant, STreeNode*>& Old);

	virtual SNtObjectNode*	GetNode(const QModelIndex &index) const;

	virtual STreeNode*		MkNode(const QVariant& Id) { return new SNtObjectNode(this, Id); }

	virtual QVariant		GetDefaultIcon() const { return m_DefaultIcon; }

	QMap<QString, QVariant> m_Icons;
	QVariant				m_DefaultIcon;

	SAccessRules			m_Rules;
};