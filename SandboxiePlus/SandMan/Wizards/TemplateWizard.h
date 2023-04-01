#pragma once

#include <QWizard>
#include "../../QSbieAPI/SbieStatus.h"
#include "../SbiePlusAPI.h"

QT_BEGIN_NAMESPACE
class QCheckBox;
class QLabel;
class QLineEdit;
class QRadioButton;
QT_END_NAMESPACE

class CTemplateWizard : public QWizard
{
    Q_OBJECT

public:
	enum ETemplateType { TmplCustom = 0, TmplWebBrowser, TmplMax};

    enum { 
        Page_TemplateType,

        Page_BrowserType, 
        Page_BrowserPaths, 
        Page_BrowserOptions, 

        Page_FinishTemplate
    };

    CTemplateWizard(ETemplateType Type, QWidget *parent = nullptr);

	static QString GetTemplateLabel(ETemplateType Type);
    static bool CreateNewTemplate(CSandBox* pBox, ETemplateType Type, QWidget* pParent = NULL);

private slots:
    void showHelp();

protected:
    friend class CTemplateTypePage;
    ETemplateType m_Type;
    bool m_Hold;
};

//////////////////////////////////////////////////////////////////////////////////////////
// CTemplateTypePage
// 

class CTemplateTypePage : public QWizardPage
{
    Q_OBJECT

public:
    CTemplateTypePage(QWidget *parent = nullptr);

    void initializePage() override;
    int nextId() const override;
    bool isComplete() const override;
    bool validatePage() override;

private slots:
    void OnTypChanged();

private:
    QComboBox*  m_pTemplateType;
};

//////////////////////////////////////////////////////////////////////////////////////////
// CBrowserTypePage
// 

class CBrowserTypePage : public QWizardPage
{
    Q_OBJECT

public:
    CBrowserTypePage(QWidget *parent = nullptr);

    void initializePage() override;
    int nextId() const override;
    bool isComplete() const override;
    bool validatePage() override;

private slots:
    void OnNameChanged();
    void OnPathChanged();

protected:
    friend class CBrowserPathsPage;
    friend class CBrowserOptionsPage;
    friend class CFinishTemplatePage;
    friend class CTemplateWizard;

    struct STestFile {
        QString Name;
        int Confidence;
    };

    static int TestFiles(const QString& ImageDir, const QList<STestFile>& Files);

    enum EBrowserType {
        Browser_Gecko,
        Browser_Chromium,
        Browser_Other
    };

    static EBrowserType DetectBrowserType(const QString& ImagePath, int* pConfidence = NULL); // *pConfidence = 0 ... 100

    EBrowserType m_BrowserType;

private:
    QLineEdit* m_pName;
    int m_NameState;
    QLineEdit* m_pBrowserPath;
    QLabel* m_pInfoLabel;
};

//////////////////////////////////////////////////////////////////////////////////////////
// CBrowserPathsPage
// 

class CBrowserPathsPage : public QWizardPage
{
    Q_OBJECT

public:
    CBrowserPathsPage(QWidget* parent = nullptr);
    
    void initializePage() override;
    int nextId() const override;
    bool isComplete() const override;
    bool validatePage() override;


private slots:
    void OnNoProfile();
    void OnProfilesChange();
    void OnProfileChange();

protected:

    struct SFoundFolder {
        QString Path;
        int Confidence;
    };

    static bool IsFirefoxProfile(const QString& Path);
    static QString GetFirefoxProfiles(const QString& Path);

    static bool IsChromiumProfile(const QString& Path);
    static QString GetChromiumProfiles(const QString& Path);

    static int SmartMatch(const QString& L, const QString& R);

    static QList<SFoundFolder> FindFolders(const QString& Root, const QString& Name, QString(*GetProfilePath)(const QString&), int Depth = 2);

    static QList<SFoundFolder> FindFirefoxFolders(const QString& ImagePath);
    static QList<SFoundFolder> FindChromiumFolders(const QString& ImagePath);

    QList<SFoundFolder> m_FoundFolders;
    
    bool IsBrowserProfile(const QString& Path) const;
    bool TestProfilePath(const QString& Path) const;

private:
    QCheckBox* m_pNoProfile;
    QComboBox* m_pProfilePath;
    QCheckBox* m_pProfileFilter;
    QLabel* m_pInfoLabel;
};

//////////////////////////////////////////////////////////////////////////////////////////
// CBrowserOptionsPage
// 

class CBrowserOptionsPage : public QWizardPage
{
    Q_OBJECT

public:
    CBrowserOptionsPage(QWidget *parent = nullptr);

    int nextId() const override;
    void initializePage() override;
    bool validatePage() override;

private:
    QCheckBox* m_pForce;
    QCheckBox* m_pProfile;
    QCheckBox* m_pPhishingDb;
    QCheckBox* m_pSessions;
    QCheckBox* m_pSyncData;
    QCheckBox* m_pPreferences;
    QCheckBox* m_pPasswords;
    QCheckBox* m_pCookies;
    QCheckBox* m_pBookmarks;
    QCheckBox* m_pHistory;
};


//////////////////////////////////////////////////////////////////////////////////////////
// CFinishTemplatePage
// 

class CFinishTemplatePage : public QWizardPage
{
    Q_OBJECT

public:
    CFinishTemplatePage(QWidget *parent = nullptr);

    int nextId() const override;
    void initializePage() override;
    bool validatePage() override;

private:
    QTextEdit* m_pSummary;
};


