#pragma once

#include <QWizard>
#include <QProxyStyle>
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

class MyButtonStyle : public QProxyStyle
{
public:
    MyButtonStyle(QStyle* pStyle) : QProxyStyle(pStyle) {}

    virtual QSize sizeFromContents(ContentsType type, const QStyleOption* option, const QSize& size, const QWidget* widget) const
    {
        QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
	    if (type == CE_PushButton && widget->property("leftButton").toBool())
        {
            if (const QStyleOptionButton* button = qstyleoption_cast<const QStyleOptionButton*>(option))
            {
                if (!button->icon.isNull()) {
                    s.setWidth(s.width() + 20 + button->iconSize.width());
                }
            }
	    }
	    return s;
    }

    virtual void drawControl(ControlElement element, const QStyleOption* opt, QPainter* p, const QWidget* widget) const 
    {
        if (element == CE_PushButtonLabel && widget->property("leftButton").toBool())
        {
            if (const QStyleOptionButton* button = qstyleoption_cast<const QStyleOptionButton*>(opt))
            {
                QRect textRect = button->rect;
                uint tf = Qt::AlignVCenter | Qt::TextShowMnemonic;
                if (!proxy()->styleHint(SH_UnderlineShortcut, button, widget))
                    tf |= Qt::TextHideMnemonic;

                if (!button->icon.isNull()) {
                    QRect iconRect;
                    QIcon::Mode mode = button->state & State_Enabled ? QIcon::Normal : QIcon::Disabled;
                    if (mode == QIcon::Normal && button->state & State_HasFocus)
                        mode = QIcon::Active;
                    QIcon::State state = QIcon::Off;
                    if (button->state & State_On)
                        state = QIcon::On;

                    QPixmap pixmap = button->icon.pixmap(widget ? widget->window()->windowHandle() : 0, button->iconSize, mode, state);

                    int pixmapWidth = pixmap.width() / pixmap.devicePixelRatio();
                    int pixmapHeight = pixmap.height() / pixmap.devicePixelRatio();
                    int labelWidth = pixmapWidth;
                    int labelHeight = pixmapHeight;
                    int iconSpacing = 4;//### 4 is currently hardcoded in QPushButton::sizeHint()
                    int textWidth = button->fontMetrics.boundingRect(opt->rect, tf, button->text).width();
                    if (!button->text.isEmpty())
                        labelWidth += (textWidth + iconSpacing);

                    textRect.setLeft(textRect.left() + 20 + button->iconSize.width());

                    /*************************************************************/
                    // Make the icon rectangle always be 10px in from the left edge
                    /*************************************************************/
                    iconRect = QRect(10,
                        textRect.y() + (textRect.height() - labelHeight) / 2,
                        pixmapWidth, pixmapHeight);

                    iconRect = visualRect(button->direction, textRect, iconRect);

                    /***********************************/
                    // Always horizontal align the text
                    /***********************************/
                    tf |= Qt::AlignLeft;


                    if (button->state & (State_On | State_Sunken))
                        iconRect.translate(proxy()->pixelMetric(PM_ButtonShiftHorizontal, opt, widget),
                            proxy()->pixelMetric(PM_ButtonShiftVertical, opt, widget));
                    p->drawPixmap(iconRect, pixmap);
                }
                else {
                    tf |= Qt::AlignHCenter;
                }
                if (button->state & (State_On | State_Sunken))
                    textRect.translate(proxy()->pixelMetric(PM_ButtonShiftHorizontal, opt, widget),
                        proxy()->pixelMetric(PM_ButtonShiftVertical, opt, widget));

                if (button->features & QStyleOptionButton::HasMenu) {
                    int indicatorSize = proxy()->pixelMetric(PM_MenuButtonIndicator, button, widget);
                    if (button->direction == Qt::LeftToRight)
                        textRect = textRect.adjusted(0, 0, -indicatorSize, 0);
                    else
                        textRect = textRect.adjusted(indicatorSize, 0, 0, 0);
                }
                proxy()->drawItemText(p, textRect, tf, button->palette, (button->state & State_Enabled),
                    button->text, QPalette::ButtonText);
            }
            return;
        }
        

        // For all other controls, draw the default
        QProxyStyle::drawControl(element, opt, p, widget);
    }
};


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

