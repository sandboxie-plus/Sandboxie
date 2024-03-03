/********************************************************************************
** Form generated from reading UI file 'SelectBoxWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SELECTBOXWINDOW_H
#define UI_SELECTBOXWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SelectBoxWindow
{
public:
    QVBoxLayout *verticalLayout_2;
    QGridLayout *gridLayout;
    QFrame *line;
    QRadioButton *radBoxed;
    QDialogButtonBox *buttonBox;
    QTreeWidget *treeBoxes;
    QRadioButton *radUnBoxed;
    QCheckBox *chkAdmin;
    QLabel *label;
    QRadioButton *radBoxedNew;

    void setupUi(QWidget *SelectBoxWindow)
    {
        if (SelectBoxWindow->objectName().isEmpty())
            SelectBoxWindow->setObjectName(QString::fromUtf8("SelectBoxWindow"));
        SelectBoxWindow->resize(263, 430);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(SelectBoxWindow->sizePolicy().hasHeightForWidth());
        SelectBoxWindow->setSizePolicy(sizePolicy);
        SelectBoxWindow->setMinimumSize(QSize(0, 0));
        SelectBoxWindow->setMaximumSize(QSize(16777215, 16777215));
        verticalLayout_2 = new QVBoxLayout(SelectBoxWindow);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        line = new QFrame(SelectBoxWindow);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line, 5, 0, 1, 1);

        radBoxed = new QRadioButton(SelectBoxWindow);
        radBoxed->setObjectName(QString::fromUtf8("radBoxed"));
        radBoxed->setChecked(true);

        gridLayout->addWidget(radBoxed, 1, 0, 1, 1);

        buttonBox = new QDialogButtonBox(SelectBoxWindow);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 7, 0, 1, 1);

        treeBoxes = new QTreeWidget(SelectBoxWindow);
        treeBoxes->setObjectName(QString::fromUtf8("treeBoxes"));

        gridLayout->addWidget(treeBoxes, 2, 0, 1, 1);

        radUnBoxed = new QRadioButton(SelectBoxWindow);
        radUnBoxed->setObjectName(QString::fromUtf8("radUnBoxed"));

        gridLayout->addWidget(radUnBoxed, 4, 0, 1, 1);

        chkAdmin = new QCheckBox(SelectBoxWindow);
        chkAdmin->setObjectName(QString::fromUtf8("chkAdmin"));

        gridLayout->addWidget(chkAdmin, 6, 0, 1, 1);

        label = new QLabel(SelectBoxWindow);
        label->setObjectName(QString::fromUtf8("label"));
        label->setWordWrap(true);

        gridLayout->addWidget(label, 0, 0, 1, 1);

        radBoxedNew = new QRadioButton(SelectBoxWindow);
        radBoxedNew->setObjectName(QString::fromUtf8("radBoxedNew"));
        radBoxedNew->setChecked(false);

        gridLayout->addWidget(radBoxedNew, 3, 0, 1, 1);


        verticalLayout_2->addLayout(gridLayout);


        retranslateUi(SelectBoxWindow);

        QMetaObject::connectSlotsByName(SelectBoxWindow);
    } // setupUi

    void retranslateUi(QWidget *SelectBoxWindow)
    {
        SelectBoxWindow->setWindowTitle(QCoreApplication::translate("SelectBoxWindow", "SandboxiePlus select box", nullptr));
        radBoxed->setText(QCoreApplication::translate("SelectBoxWindow", "Run Sandboxed", nullptr));
        QTreeWidgetItem *___qtreewidgetitem = treeBoxes->headerItem();
        ___qtreewidgetitem->setText(0, QCoreApplication::translate("SelectBoxWindow", "Sandbox", nullptr));
        radUnBoxed->setText(QCoreApplication::translate("SelectBoxWindow", "Run Outside the Sandbox", nullptr));
        chkAdmin->setText(QCoreApplication::translate("SelectBoxWindow", "Run As UAC Administrator", nullptr));
        label->setText(QCoreApplication::translate("SelectBoxWindow", "Select the sandbox in which to start the program, installer or document.", nullptr));
        radBoxedNew->setText(QCoreApplication::translate("SelectBoxWindow", "Run in a new Sandbox", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SelectBoxWindow: public Ui_SelectBoxWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SELECTBOXWINDOW_H
