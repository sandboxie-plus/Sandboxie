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
	void showSupport();

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

	void SetIniEdit(bool bEnable);
	void OnEditIni();
	void OnSaveIni();
	void OnCancelEdit();


	void CertChanged();
	void UpdateCert();

protected:
	void closeEvent(QCloseEvent *e);

	void	AddWarnEntry(const QString& Name, int type);

	void	LoadSettings();
	void	SaveSettings();

	void	LoadIniSection();
	void	SaveIniSection();

	int 	m_CompatLoaded;
	QString m_NewPassword;
	bool	m_WarnProgsChanged;
	bool	m_CompatChanged;
	bool	m_FeaturesChanged;
	bool	m_CertChanged;
private:
	Ui::SettingsWindow ui;
};

void CSettingsWindow__AddContextMenu();

void WindowsMoveFile(const QString& from, const QString& to);

extern quint32 g_FeatureFlags;

extern QByteArray g_Certificate;
union SCertInfo {
    quint64	State;
    struct {
        quint32
            valid     : 1, // certificate is active
            expired   : 1, // certificate is expired but may be active
            outdated  : 1, // certificate is expired, not anymore valid for the current build
            business  : 1, // certificate is siutable for business use
            reservd_1 : 4,
            reservd_2 : 8,
            reservd_3 : 8,
			reservd_4 : 8;
		quint32 
			expirers_in_sec : 30, 
			unused_1        : 1, // skim a couple high bits to use as flags flag, 0x3fffffff -> is 34 years count down isenough
			about_to_expire : 1; 
    };
};
extern SCertInfo g_CertInfo;