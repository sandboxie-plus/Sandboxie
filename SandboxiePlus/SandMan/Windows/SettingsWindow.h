#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_SettingsWindow.h"
#include <QProxyStyle>

class CustomTabStyle : public QProxyStyle {
public:
	CustomTabStyle(QStyle* style = 0) : QProxyStyle(style) {}

	QSize sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const;
	void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
};


class CSettingsWindow : public QDialog
{
	Q_OBJECT

public:
	CSettingsWindow(QWidget *parent = Q_NULLPTR);
	~CSettingsWindow();

	virtual void accept() {}
	virtual void reject();

	static void LoadCertificate();

signals:
	void OptionsChanged();
	void Closed();

public slots:
	void ok();
	void apply();

	void showCompat();

private slots:
	void OnChange();

	void OnTab();

	void OnFeaturesChanged() { m_FeaturesChanged = true; }

	void OnBrowse();

	void OnSetPassword();

	void OnWarnChanged() { m_WarnProgsChanged = true; }
	void OnAddWarnProg();
	void OnAddWarnFolder();
	void OnDelWarnProg();

	void OnTemplateClicked(QTreeWidgetItem* pItem, int Column);
	void OnAddCompat();
	void OnDelCompat();

	void OnSupport(const QString& url);
	void CertChanged();

protected:
	void closeEvent(QCloseEvent *e);

	void	AddWarnEntry(const QString& Name, int type);

	void	LoadSettings();
	void	SaveSettings();

	int 	m_CompatLoaded;
	QString m_NewPassword;
	bool	m_WarnProgsChanged;
	bool	m_CompatChanged;
	bool	m_FeaturesChanged;
	bool	m_CertChanged;
private:
	Ui::SettingsWindow ui;
};

void WindowsMoveFile(const QString& from, const QString& to);

extern QByteArray g_Certificate;
extern quint32 g_FeatureFlags;