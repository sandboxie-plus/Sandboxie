#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_SettingsWindow.h"
#include "../../MiscHelpers/Common/SettingsWidgets.h"

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

	static Qt::CheckState IsContextMenu();
	static void AddContextMenu(bool bAlwaysClassic = false);
	static void RemoveContextMenu();
	static bool AddBrowserIcon();

	static bool ApplyCertificate(const QByteArray &Certificate, QWidget* widget);
	static void LoadCertificate(QString CertPath = QString());
	static bool	TryRefreshCert(QWidget* parent, QObject* receiver, const char* member);
	static bool	CertRefreshRequired();

	static QString GetCertType();
	static QColor GetCertColor();
	static QString GetCertLevel();

	static void StartEval(QWidget* parent, QObject* receiver, const char* member);

signals:
	void OptionsChanged(bool bRebuildUI = false);
	void Closed();

public slots:
	void ok();
	void apply();

	void showTab(const QString& Name, bool bExclusive = false, bool bExec = false);

private slots:
	void OnTab();
	void OnCompat();

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

	void OnSkipUAC() { m_SkipUACChanged = true; OnOptChanged(); }

	void OnChangeGUI() { m_bRebuildUI = true; OnOptChanged(); }
	void OnFeaturesChanged() { m_FeaturesChanged = true; OnGeneralChanged(); }
	void OnGeneralChanged() { m_GeneralChanged = true; OnOptChanged(); }

	void OnLoadAddon();
	void OnInstallAddon();
	void OnRemoveAddon();

	void OnBrowse();

	void OnRamDiskChange();

	void OnProtectionChange();
	void OnSetPassword();

	void OnWarnChanged() { m_WarnProgsChanged = true; OnOptChanged(); }
	void OnAddWarnProg();
	void OnAddWarnFolder();
	void OnDelWarnProg();

	void OnMoTWChange();
	void OnVolumeChanged();
	void UpdateDrives();

	void OnCompatChanged() { m_CompatChanged = true; OnOptChanged(); }
	void OnTemplateClicked(QTreeWidgetItem* pItem, int Column);
	void OnTemplateDoubleClicked(QTreeWidgetItem* pItem, int Column);
	void OnAddCompat();
	void OnDelCompat();

	void OnFilterTemplates()		{ LoadTemplates(); }
	void OnAddTemplates();
	void OnTemplateWizard();
	void OnOpenTemplate();
	void OnDelTemplates();

	void SetIniEdit(bool bEnable);
	void OnEditIni();
	void OnSaveIni();
	void OnIniChanged();
	void OnCancelEdit();


	void CertChanged();
	void KeyChanged();
	void UpdateCert();
	void OnGetCert();
	void OnCertData(const QByteArray& Certificate, const QVariantMap& Params);
	void ApplyCert();
	void UpdateUpdater();
	void OnStartEval();

	void GetUpdates();
	void OnUpdateData(const QVariantMap& Data, const QVariantMap& Params);
	void OnUpdate(const QString& Channel);

	void OnSetTree();

	void OnSelectIniEditFont();
	void OnResetIniEditFont();

protected:
	void closeEvent(QCloseEvent *e);

	bool eventFilter(QObject *watched, QEvent *e);

	void OnTab(QWidget* pTab);

	void	AddMessageItem(const QString& ID, const QString& Text = QString());

	void	AddWarnEntry(const QString& Name, int type);

	void	LoadSettings();
	void	SaveSettings();

	void	LoadTemplates();

	void	LoadIniSection();
	void	SaveIniSection();
	void    ApplyIniEditFont();

	bool	m_bRebuildUI;
	bool	m_HoldChange;
	int 	m_CompatLoaded;
	QString m_NewPassword;
	bool	m_MessagesChanged;
	bool	m_WarnProgsChanged;
	bool    m_VolumeChanged;
	bool	m_CompatChanged;
	bool	m_RunChanged;
	bool	m_SkipUACChanged;
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

	class CCodeEdit* m_pCodeEdit;
};

QVariantMap GetRunEntry(const QString& sEntry);
void AddRunItem(QTreeWidget* treeRun, const QVariantMap& Entry);
QString MakeRunEntry(QTreeWidgetItem* pItem);
QString MakeRunEntry(const QVariantMap& Entry);

void WindowsMoveFile(const QString& from, const QString& to);

extern quint32 g_FeatureFlags;

extern QByteArray g_Certificate;

#include "..\..\Sandboxie\core\drv\verify.h"

extern SCertInfo g_CertInfo;

#define EVAL_MAX 3		// for UI only actual limits enforced on server
#define EVAL_DAYS 10	