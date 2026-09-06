// SPDX-License-Identifier: LicenseRef-Sandboxie-Plus
// See ../LICENSE for the SandMan license.
#pragma once
#include "ProxyProfile.h"
#include <QThread>
#include <atomic>

class CProxyTunnel : public QThread
{
	Q_OBJECT
public:
	enum EState { Stopped, Starting, Running, Checking, Stopping, Failed };
	CProxyTunnel(const SProxyProfile& Profile, const QString& Directory, QObject* Parent = nullptr);
	~CProxyTunnel() override;
	void RequestStop() { m_Stop.store(true); requestInterruption(); }
	void RequestCheck() { m_Check.store(true); }
	static QString StateText(int State);

signals:
	void StateChanged(const QString& Id, int State, const QString& Message);
	void EgressChecked(const QString& Id, const QString& Address);

protected:
	void run() override;

private:
	QString RunWindows();
	SProxyProfile m_Profile;
	QString m_Directory;
	std::atomic<bool> m_Stop{false};
	std::atomic<bool> m_Check{false};
};
