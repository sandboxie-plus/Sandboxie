#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_SettingsWindow.h"
#include <QProxyStyle>
#include "../../MiscHelpers/Common/SettingsWidgets.h"

class CustomTabStyle : public QProxyStyle {
public:
	CustomTabStyle(QStyle* style = 0) : QProxyStyle(style) {}

	QSize sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const;
	void drawControl(ControlElement element, const QStyleOption* option, QPainter* painter, const QWidget* widget) const;
};

void FixTriStateBoxPallete(QWidget* pWidget);

class CSecretCheckBox : public QCheckBox
{
	Q_OBJECT
public:
	CSecretCheckBox(const QString& Text) : QCheckBox(Text)
	{
		setTristate(false);
		m_SecretMode = false;
	}

	bool IsSecretSet() const { return m_SecretMode && this->checkState() == Qt::PartiallyChecked; }

protected:
	void mouseDoubleClickEvent(QMouseEvent* e)
	{
		if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
			setTristate();
			m_SecretMode = true;
		}
	}

	bool m_SecretMode;
};

void AddIconToLabel(QLabel* pLabel, const QPixmap& Pixmap);

//////////////////////////////////////////////////////////////////////////
// CSettingsWindow

class CSettingsWindow : public CConfigDialog
{
	Q_OBJECT

public:
	CSettingsWindow(QWidget *parent = Q_NULLPTR);
	~CSettingsWindow();

	virtual void accept() {}
	virtual void reject();

	static bool ApplyCertificate(const QByteArray &Certificate, QWidget* widget);

	static void LoadCertificate(QString CertPath = QString());

	enum ETabs {
		eOptions = 0,
		eShell,
		eGuiConfig,
		eAdvanced,
		eProgCtrl,
		eConfigLock,
		eSoftCompat,
		eEditIni,
		eSupport
	};

signals:
	void OptionsChanged(bool bRebuildUI = false);
	void Closed();

public slots:
	void ok();
	void apply();

	void showTab(int Tab, bool bExclusive = false);

private slots:
	void OnTab();

	void OnAddMessage();
	void OnDelMessage();
	void OnMessageChanged() { m_MessagesChanged = true; OnOptChanged(); }

	void OnRootChanged();

	void OnBrowsePath();
	void OnAddCommand();
	void OnCommandUp();
	void OnCommandDown();
	void OnDelCommand();
	void OnRunChanged() { m_RunChanged = true; OnOptChanged(); }

	void OnOptChanged();

	void OnChangeGUI() { m_bRebuildUI = true; OnOptChanged(); }
	void OnFeaturesChanged() { m_FeaturesChanged = true; OnGeneralChanged(); }
	void OnGeneralChanged() { m_GeneralChanged = true; OnOptChanged(); }

	void OnBrowse();

	void OnProtectionChange();
	void OnSetPassword();

	void OnWarnChanged() { m_WarnProgsChanged = true; OnOptChanged(); }
	void OnAddWarnProg();
	void OnAddWarnFolder();
	void OnDelWarnProg();

	void OnCompatChanged() { m_CompatChanged = true; OnOptChanged(); }
	void OnTemplateClicked(QTreeWidgetItem* pItem, int Column);
	void OnTemplateDoubleClicked(QTreeWidgetItem* pItem, int Column);
	void OnAddCompat();
	void OnDelCompat();

	void OnFilterTemplates()		{ LoadTemplates(); }
	void OnAddTemplates();
	void OnTemplateWizard();
	void OnDelTemplates();

	void SetIniEdit(bool bEnable);
	void OnEditIni();
	void OnSaveIni();
	void OnIniChanged();
	void OnCancelEdit();


	void CertChanged();
	void UpdateCert();
	void UpdateUpdater();

	void GetUpdates();
	void OnUpdateData(const QVariantMap& Data, const QVariantMap& Params);
	void OnUpdate(const QString& Channel);

	void OnSetTree();

protected:
	void closeEvent(QCloseEvent *e);

	bool eventFilter(QObject *watched, QEvent *e);

	void OnTab(QWidget* pTab);

	void	AddMessageItem(const QString& ID, const QString& Text = QString());

	void	AddRunItem(const QString& Name, const QString& Icon, const QString& Command);

	void	AddWarnEntry(const QString& Name, int type);

	void	LoadSettings();
	void	SaveSettings();

	void	LoadTemplates();

	void	LoadIniSection();
	void	SaveIniSection();

	bool	m_bRebuildUI;
	bool	m_HoldChange;
	int 	m_CompatLoaded;
	QString m_NewPassword;
	bool	m_MessagesChanged;
	bool	m_WarnProgsChanged;
	bool	m_CompatChanged;
	bool	m_RunChanged;
	bool	m_ProtectionChanged;
	bool	m_GeneralChanged;
	bool	m_FeaturesChanged;
	bool	m_CertChanged;
	QVariantMap m_UpdateData;

private:

	void WriteAdvancedCheck(QCheckBox* pCheck, const QString& Name, const QString& OnValue, const QString& OffValue);
	void WriteText(const QString& Name, const QString& Value);
	void WriteTextList(const QString& Setting, const QStringList& List);

	Ui::SettingsWindow ui;
};

void CSettingsWindow__AddContextMenu(bool bAlwaysClassic = false);
void CSettingsWindow__RemoveContextMenu();
void CSettingsWindow__AddBrowserIcon();

void WindowsMoveFile(const QString& from, const QString& to);

extern quint32 g_FeatureFlags;

extern QByteArray g_Certificate;
union SCertInfo {
    quint64	State;
    struct {
        quint32
            valid     : 1,      // certificate is active
            expired   : 1,      // certificate is expired but may be active
            outdated  : 1,      // certificate is expired, not anymore valid for the current build
            business  : 1,      // certificate is suitable for business use
            evaluation: 1,      // evaluation certificate
            grace_period: 1,    // the certificate is expired and or outdated but we keep it valid for 1 extra month to allof wor a seamless renewal
            reservd_1 : 2,
            reservd_2 : 8,
            reservd_3 : 8,
			reservd_4 : 7,
			insider   : 1;
		qint32 
			expirers_in_sec : 30, 
			unused_1        : 1, // skim a couple high bits to use as flags flag, 0x3fffffff -> is 34 years count down is enough
			about_to_expire : 1; 
    };
};
extern SCertInfo g_CertInfo;
