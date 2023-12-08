#pragma once

#include <QWizard>
#include "../../QSbieAPI/SbieStatus.h"

QT_BEGIN_NAMESPACE
class QCheckBox;
class QLabel;
class QLineEdit;
class QRadioButton;
QT_END_NAMESPACE

//#define USE_COMBO

class CNewBoxWizard : public QWizard
{
    Q_OBJECT

public:
    enum { Page_Type, Page_Files, Page_Advanced, Page_Summary };

    CNewBoxWizard(bool bAlowTemp, QWidget *parent = nullptr);

    static QString CreateNewBox(bool bAlowTemp, QWidget* pParent = NULL);

    QString GetDefaultLocation();

private slots:
    void showHelp();

protected:
    friend class CBoxTypePage;
    friend class CSummaryPage;

    SB_STATUS TryToCreateBox();

    bool m_bAdvanced;
};


//////////////////////////////////////////////////////////////////////////////////////////
// CBoxTypePage
// 

//#define USE_COMBO

class CBoxTypePage : public QWizardPage
{
    Q_OBJECT

    Q_PROPERTY(int currentType READ currentType WRITE setCurrentType NOTIFY typeChanged USER true)

public:
    CBoxTypePage(bool bAlowTemp, QWidget *parent = nullptr);

    int nextId() const override;
    bool isComplete() const override;
    bool validatePage() override;

    void setCurrentType(int type);
    int currentType();

signals:
    void typeChanged();

private slots:
    void OnBoxTypChanged();
    void OnAdvanced();

private:
#ifdef USE_COMBO
    QComboBox*      m_pBoxType;
    QLabel*         m_pInfoLabel;
#endif
    QLineEdit*      m_pBoxName;
    QButtonGroup*   m_TypeGroup;
    QCheckBox*      m_pAdvanced;

    bool m_bInstant;
};


//////////////////////////////////////////////////////////////////////////////////////////
// CFilesPage
// 

class CFilesPage : public QWizardPage
{
    Q_OBJECT

public:
    CFilesPage(QWidget *parent = nullptr);

    int nextId() const override;
    void initializePage() override;
    bool validatePage() override;

private:
    QComboBox* m_pBoxLocation;
};


//////////////////////////////////////////////////////////////////////////////////////////
// CAdvancedPage
// 

class CAdvancedPage : public QWizardPage
{
    Q_OBJECT

public:
    CAdvancedPage(QWidget *parent = nullptr);

    int nextId() const override;
    void initializePage() override;
    bool validatePage() override;

private:
    QCheckBox* m_pShareAccess;
    QCheckBox* m_pMSIServer;
    QCheckBox* m_pBoxToken;
};


//////////////////////////////////////////////////////////////////////////////////////////
// CSummaryPage
// 

class CSummaryPage : public QWizardPage
{
    Q_OBJECT

public:
    CSummaryPage(QWidget *parent = nullptr);

    int nextId() const override;
    void initializePage() override;
    bool validatePage() override;

private:
    QTextEdit* m_pSummary;
    QCheckBox* m_pSetDefault;
    QCheckBox* m_pSetInstant;
};


