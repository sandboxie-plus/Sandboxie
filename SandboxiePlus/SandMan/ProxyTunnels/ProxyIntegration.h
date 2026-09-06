// SPDX-License-Identifier: LicenseRef-Sandboxie-Plus
// See ../LICENSE for the SandMan license.
#pragma once
#include <QObject>
#include <QPointer>
#include "ProxyManager.h"
class CSandMan;
class CProxyWindow;

class CProxyIntegration : public QObject
{
public:
	explicit CProxyIntegration(CSandMan* Gui);
	~CProxyIntegration() override;
	void Show();
private:
	QString ActivationError() const;
	QList<SProxyBox> Boxes() const;
	QString ModificationError(const QString& Id) const;
	bool Assign(const QString& BoxName, const QString& Id, QString& Error);
	CSandMan* m_Gui;
	CProxyManager* m_Manager;
	QPointer<CProxyWindow> m_Window;
};
