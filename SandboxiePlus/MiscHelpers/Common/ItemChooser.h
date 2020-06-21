#pragma once

#include <QtWidgets/QMainWindow>
#include <QListWidgetItem>

#include "../mischelpers_global.h"

class MISCHELPERS_EXPORT CItemChooser : public QDialog
{
	Q_OBJECT

public:
	CItemChooser(const QString& Prompt = QString(), QWidget *parent = Q_NULLPTR);

	void				SetPrompt(const QString& Text);
	void				AddWidgets(QList<QWidget*> Widgets);
	void				AddWidget(QWidget* pWidget);

	void				AddItem(const QString& Label, const QVariant& Data);
	void				ChooseItems(const QVariantList& ChoosenItems);
	void				ChooseItem(const QVariant& Data);

	QVariantList		GetChoosenItems();

private slots:
	void				OnAdd();
	void				OnRemove();
	void				OnUp();
	void				OnDown();


protected:
	void closeEvent(QCloseEvent *e);

	void				AddItem(QListWidgetItem* pItem);
	void				RemoveItem(QListWidgetItem* pChoosenItem);
	bool				MoveItem(QListWidgetItem* pItem, int Pos);

private:

    QVBoxLayout*		m_pMainLayout;
    QLabel*				m_pLabel;

    QHBoxLayout*		m_pCenterLayout;

    QListWidget*		m_pListAll;

    QVBoxLayout*		m_pMidleLayout;
    QPushButton*		m_pBtnAdd;
    QPushButton*		m_pBtnRemove;

    QListWidget*		m_pListChoosen;

    QVBoxLayout*		m_pRightLayout;
    QPushButton*		m_pBtnUp;
    QPushButton*		m_pBtnDown;

    QDialogButtonBox*	m_pButtonBox;

	int					m_InsertIndex;
};
