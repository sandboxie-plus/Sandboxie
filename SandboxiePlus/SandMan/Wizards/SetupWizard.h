#pragma once

#include <QWizard>

QT_BEGIN_NAMESPACE
class QCheckBox;
class QLabel;
class QLineEdit;
class QRadioButton;
QT_END_NAMESPACE

class CSetupWizard : public QWizard
{
    Q_OBJECT

public:
    enum { Page_Intro, Page_Certificate, Page_Shell, Page_Finish };

    CSetupWizard(QWidget *parent = nullptr);

    static bool ShowWizard();

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
    QRadioButton *m_pPersonalRadio;
    QRadioButton *m_pBusinessRadio;
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

private:
    QLabel* m_pTopLabel;
    QPlainTextEdit* m_pCertificate;
    QCheckBox* m_pEvaluate;
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
    QCheckBox *m_pUpdate;
};

