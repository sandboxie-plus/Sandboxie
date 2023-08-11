#pragma once

#include <QWizard>
#include "SbiePlusAPI.h"

QT_BEGIN_NAMESPACE
class QCheckBox;
class QLabel;
class QLineEdit;
class QRadioButton;
class QButtonGroup;
class QListWidget;
QT_END_NAMESPACE

class CWizardEngine;


class CBoxAssistant : public QWizard
{
    Q_OBJECT

public:
    enum { Page_Begin, Page_Group, Page_List, Page_Run, Page_Submit, Page_Complete, Page_Next };

    CBoxAssistant(QWidget *parent = nullptr);
    ~CBoxAssistant();

    void TryFix(quint32 MsgCode, const QStringList& MsgData, const QString& ProcessName, const QString& BoxName);

    virtual void accept();
    virtual void reject();

private slots:
    void OnIssuesUpdated();

	void OnToggleDebugger();

    void OnBoxUsed(const CSandBoxPtr& pBox);

protected:
    friend class CBeginPage;
    friend class CGroupPage;
    friend class CListPage;
    friend class CRunPage;
    friend class CSubmitPage;
    friend class CCompletePage;

    QList<QVariantMap> GetIssues(const QVariantMap& Root) const;

    void PushIssue(const QVariantMap& Issue) { m_IssueStack.append(Issue); }
    void PopIssue() { m_IssueStack.removeLast(); }
    QVariantMap CurrentIssue() const { return m_IssueStack.isEmpty() ? QVariantMap() : m_IssueStack.last(); }

    bool StartEngine();
    void KillEngine();

    CWizardEngine* GetEngine() { return m_pEngine; }

    int m_NextCounter;

    struct SUsedBox
    {
        CSandBoxPtr pBox;
        QStringList OldDumps;
    };

    QList<SUsedBox> m_UsedBoxes;

private:
    QList<QVariantMap> m_IssueStack;

    CWizardEngine* m_pEngine;
    QVariantMap m_Params;
    bool m_bUseDebugger;
    QMainWindow* m_pDebugger;
};

//////////////////////////////////////////////////////////////////////////////////////////
// CBeginPage
// 

class CBeginPage : public QWizardPage
{
    Q_OBJECT

public:
    CBeginPage(QWidget *parent = nullptr);

    void initializePage() override;
    int nextId() const override;
    bool isComplete() const override;
    bool validatePage() override;

private slots:
    void            OnCategory();

private:
    QGridLayout*    m_pLayout;
    QPushButton*    m_pCurrent;
    QList<QWidget*> m_pWidgets;
};

//////////////////////////////////////////////////////////////////////////////////////////
// CGroupPage
// 

class CGroupPage : public QWizardPage
{
    Q_OBJECT

public:
    CGroupPage(QWidget *parent = nullptr);

    void initializePage() override;
    void cleanupPage() override;
    int nextId() const override;
    bool isComplete() const override;
    bool validatePage() override;

private:
    QGridLayout*    m_pLayout;
    QLabel*         m_pTopLabel;
    QButtonGroup*   m_pGroup;
    QList<QWidget*> m_pWidgets;
};

//////////////////////////////////////////////////////////////////////////////////////////
// CListPage
// 

class CListPage : public QWizardPage
{
    Q_OBJECT

public:
    CListPage(QWidget *parent = nullptr);

    void initializePage() override;
    void cleanupPage() override;
    int nextId() const override;
    bool isComplete() const override;
    bool validatePage() override;

private slots:
    void ApplyFilter();

private:
    void LoadIssues();

    QGridLayout* m_pLayout;
    QLineEdit* m_pFilter;
    QListWidget* m_pList;

};

//////////////////////////////////////////////////////////////////////////////////////////
// CRunPage
// 

class CRunPage : public QWizardPage
{
    Q_OBJECT

public:
    CRunPage(QWidget *parent = nullptr);

    void initializePage() override;
    void cleanupPage() override;
    bool isComplete() const override;
    int nextId() const override;
    bool validatePage() override;

private slots:
    void OnStateChanged(int state, const QString& Text = "");
    void CheckUserInput();

private:
    QGridLayout* m_pLayout;
    QLabel* m_pTopLabel;
    QWidget* m_pForm;
    QMultiMap<QString, QWidget*> m_pWidgets;
};

//////////////////////////////////////////////////////////////////////////////////////////
// CSubmitPage
// 

class CSubmitPage : public QWizardPage
{
    Q_OBJECT

public:
    CSubmitPage(QWidget *parent = nullptr);

    void initializePage() override;
    int nextId() const override {return -1;}
    bool validatePage() override;

private:
    QLabel* m_pTopLabel;
    QTextEdit* m_pReport;
    QLineEdit* m_pMail;
    QCheckBox* m_pAttachIni;
    QCheckBox* m_pAttachLog;
    QCheckBox* m_pAttachDmp;

    CSbieProgressPtr m_pUploadProgress;
};

//////////////////////////////////////////////////////////////////////////////////////////
// CCompletePage
// 

class CCompletePage : public QWizardPage
{
    Q_OBJECT

public:
    CCompletePage(QWidget *parent = nullptr);

    void initializePage() override;
    int nextId() const override {return -1;}

private:
    QLabel* m_pLabel;
};

