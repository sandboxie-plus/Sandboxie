// SPDX-License-Identifier: LicenseRef-Sandboxie-Plus
// See ../LICENSE for the SandMan license.
#pragma once
#include <QDialog>
#include "ProxyManager.h"
class QTreeWidget;
class QLabel;
class QPushButton;
class QShowEvent;
class QHideEvent;
class CProxyWindow : public QDialog
{
	Q_OBJECT
public:
	explicit CProxyWindow(CProxyManager* Manager, QWidget* Parent = nullptr);
private:
	void showEvent(QShowEvent* Event) override;
	void hideEvent(QHideEvent* Event) override;
	void Refresh();
	void Edit(bool Add);
	void Import();
	void Remove();
	void Run(int Action, bool All);
	void Assign(bool Detach);
	void Error(const QString& Message);
	QStringList Selected() const;
	CProxyManager* m_Manager;
	QTreeWidget* m_Profiles;
	QTreeWidget* m_Boxes;
	QLabel* m_Notice;
	QTimer m_Timer;
	QList<QPushButton*> m_ActivationButtons;
};
