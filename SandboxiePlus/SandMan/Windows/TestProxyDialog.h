#pragma once

#include <QtWidgets/QDialog>
#include "OptionsWindow.h"
#include "ui_TestProxyDialog.h"
#include <QPropertyAnimation>

class CTestProxyDialog : public QDialog
{
	Q_OBJECT

public:
	CTestProxyDialog(const QString& IP, const QString& Port, COptionsWindow::EAuthMode AuthMode, const QString& Username = QString(), const QString& Password = QString(), QWidget* parent = Q_NULLPTR);
	~CTestProxyDialog() { ; }

protected:
	void showEvent(QShowEvent* event) override;
	void TestProxy();

signals:
	void emitTestMessage(const QString& message);
	void emitUpdateProgress(int value);

private slots:
	void OnRetry();
	void OnTestFinished();
	void OnTestCustomize() { ui.stackedWidget->setCurrentIndex(1); ui.buttonBoxSettings->button(QDialogButtonBox::Ok)->setFocus(); }
	void OnTestSettingsCancel() { RestoreDefaults();  ui.stackedWidget->setCurrentIndex(0); }
	void OnTestSettingsSave();
	void OnTestSettingsRestoreDefaults() { RestoreDefaults(); }
	void RestoreDefaults();

private:
	Ui::TestProxyDialog ui;

	QString m_ProxyIP;
	QString m_ProxyPort;
	QString m_ProxyUsername;
	QString m_ProxyPass;
	COptionsWindow::EAuthMode m_AuthMode;

	int m_TestTimeout;
	int m_TestPort;
	int m_TestPingCount;
	QString m_TestHost;

	QFutureWatcher<bool>* m_Watcher;
	QAtomicInt m_TestShouldCancel;

	void RunTest1(bool& failed, int& progress, int segment);
	void RunTest2(bool& failed, int& progress, int segment, bool loadPage);
	void RunTest2LoadPage(const QNetworkProxy& proxy, bool& failed);
	void RunTest3(bool& failed, int& progress, int segment);
	void Test2EnableParams(bool enable);
};
