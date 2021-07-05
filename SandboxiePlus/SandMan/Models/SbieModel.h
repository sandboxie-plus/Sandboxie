#pragma once
#include <qwidget.h>
#include "../SbiePlusAPI.h"
#include "../../MiscHelpers/Common/TreeItemModel.h"


class CSbieModel : public CTreeItemModel
{
    Q_OBJECT

public:
    CSbieModel(QObject *parent = 0);
	~CSbieModel();

	QList<QVariant>	Sync(const QMap<QString, CSandBoxPtr>& BoxList, const QMap<QString, QStringList>& Groups = QMap<QString, QStringList>(), bool ShowHidden = false);

	CSandBoxPtr		GetSandBox(const QModelIndex &index) const;
	CBoxedProcessPtr GetProcess(const QModelIndex &index) const;
	QVariant		GetID(const QModelIndex &index) const;

	enum ETypes
	{
		eNone = 0,
		eGroup,
		eBox,
		eProcess
	}				GetType(const QModelIndex &index) const;

	int				columnCount(const QModelIndex &parent = QModelIndex()) const;
	QVariant		headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

	enum EColumns
	{
		eName = 0,
		eProcessId,
		eTitle,
		eStatus,
		//eLogCount,
		eTimeStamp,
		ePath,
		eCount
	};

protected:
	bool			Sync(const CSandBoxPtr& pBox, const QList<QVariant>& Path, const QMap<quint32, CBoxedProcessPtr>& ProcessList, QMap<QList<QVariant>, QList<STreeNode*> >& New, QHash<QVariant, STreeNode*>& Old, QList<QVariant>& Added);

	struct SSandBoxNode: STreeNode
	{
		SSandBoxNode(const QVariant& Id) : STreeNode(Id) { inUse = false; boxType = -1; }

		CSandBoxPtr	pBox;
		bool		inUse;
		int			boxType;

		CBoxedProcessPtr pProcess;
	};

	virtual STreeNode*		MkNode(const QVariant& Id) { return new SSandBoxNode(Id); }

	QList<QVariant>			MakeProcPath(const QString& BoxName, const CBoxedProcessPtr& pProcess, const QMap<quint32, CBoxedProcessPtr>& ProcessList);
	QList<QVariant>			MakeProcPath(const CBoxedProcessPtr& pProcess, const QMap<quint32, CBoxedProcessPtr>& ProcessList);
	bool					TestProcPath(const QList<QVariant>& Path, const QString& BoxName, const CBoxedProcessPtr& pProcess, const QMap<quint32, CBoxedProcessPtr>& ProcessList, int Index = 0);

	QString					FindParent(const QVariant& Name, const QMap<QString, QStringList>& Groups);
	QList<QVariant>			MakeBoxPath(const QVariant& Name, const QMap<QString, QStringList>& Groups);
	void					MakeBoxPath(const QVariant& Name, const QMap<QString, QStringList>& Groups, QList<QVariant>& Path);

	//virtual QVariant		GetDefaultIcon() const;

private:

	//QIcon m_BoxEmpty;
	//QIcon m_BoxInUse;
	QIcon m_ExeIcon;
};