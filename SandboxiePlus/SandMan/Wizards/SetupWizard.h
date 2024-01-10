#pragma once

#include <QWizard>

QT_BEGIN_NAMESPACE
class QCheckBox;
class QLabel;
class QLineEdit;
class QRadioButton;
QT_END_NAMESPACE

#define SETUP_LVL_1 1
#define SETUP_LVL_2 2
#define SETUP_LVL_CURRENT SETUP_LVL_2

class CSetupWizard : public QWizard
{
    Q_OBJECT

public:
    enum { Page_Intro, Page_Certificate, Page_UI, Page_Shell, Page_Update, Page_Finish };

    CSetupWizard(int iOldLevel = 0, QWidget *parent = nullptr);

    static bool ShowWizard(int iOldLevel = 0);

    static void ShellUninstall();

private slots:
    void showHelp();
};

//////////////////////////////////////////////////////////////////////////////////////////
// CIntroPage
// 

class CIntroPage : public QWizardPage
{
    Q_OBJECT

public:
    CIntroPage(QWidget *parent = nullptr);

    int nextId() const override;
    bool isComplete() const override;

private:
    QLabel* m_pLabel;
    QRadioButton *m_pPersonal;
    QRadioButton *m_pBusiness;
};

//////////////////////////////////////////////////////////////////////////////////////////
// CCertificatePage
// 

class CCertificatePage : public QWizardPage
{
    Q_OBJECT

public:
    CCertificatePage(QWidget *parent = nullptr);

    void initializePage() override;
    int nextId() const override;
    bool isComplete() const override;
    bool validatePage() override;

private slots:
    void OnCertData(const QByteArray& Certificate, const QVariantMap& Params);

private:
    QLabel* m_pTopLabel;
    QPlainTextEdit* m_pCertificate;
    QLineEdit* m_pSerial;
    QCheckBox* m_pEvaluate;
};

//////////////////////////////////////////////////////////////////////////////////////////
// CUIPage
// 

class CUIPage : public QWizardPage
{
    Q_OBJECT

public:
    CUIPage(QWidget *parent = nullptr);

    void initializePage() override;
    int nextId() const override;

private slots:
    void UpdatePreview();

private:
    QRadioButton *m_pSimple;
    QRadioButton *m_pAdvanced;
    QRadioButton *m_pClassic;
    QLabel* m_pPreview;
    QRadioButton* m_pBrightMode;
    QRadioButton* m_pDarkMode;
};

//////////////////////////////////////////////////////////////////////////////////////////
// CShellPage
// 

class CShellPage : public QWizardPage
{
    Q_OBJECT

public:
    CShellPage(QWidget *parent = nullptr);

    int nextId() const override;

private:
    QCheckBox *m_pAutoStart;
    QCheckBox *m_pContecxtMenu;
    QCheckBox *m_pBrowserIcon;
};

//////////////////////////////////////////////////////////////////////////////////////////
// CWFPPage
// 

/*class CWFPPage : public QWizardPage
{
    Q_OBJECT

public:
    CWFPPage(QWidget *parent = nullptr);

    int nextId() const override;

private:
    QCheckBox *m_pUseWFP;
};*/

//////////////////////////////////////////////////////////////////////////////////////////
// CSBUpdate
// 

class CSBUpdate : public QWizardPage
{
    Q_OBJECT

public:
    CSBUpdate(QWidget *parent = nullptr);
    
    void initializePage() override;

    int nextId() const override;

private slots:
    void UpdateOptions();

private:
    QCheckBox* m_pUpdate;
    QCheckBox* m_pVersion;
    QLabel* m_pChanelInfo;
    QRadioButton* m_pStable;
    QRadioButton* m_pPreview;
    QRadioButton* m_pInsider;
    QCheckBox* m_pHotfixes;
    //QCheckBox* m_pTemplates;
    QCheckBox* m_pIssues;
    QCheckBox* m_pAddons;
    QLabel* m_pUpdateInfo;
    QLabel* m_pBottomLabel;
};

//////////////////////////////////////////////////////////////////////////////////////////
// CFinishPage
// 

class CFinishPage : public QWizardPage
{
    Q_OBJECT

public:
    CFinishPage(QWidget *parent = nullptr);

    void initializePage() override;
    int nextId() const override;
    //void setVisible(bool visible) override;

private:
    QLabel *m_pLabel;
    //QCheckBox *m_pUpdate;
};

