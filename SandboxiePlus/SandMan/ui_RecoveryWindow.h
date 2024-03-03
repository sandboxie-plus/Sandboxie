/********************************************************************************
** Form generated from reading UI file 'RecoveryWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_RECOVERYWINDOW_H
#define UI_RECOVERYWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_RecoveryWindow
{
public:
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QPushButton *btnAddFolder;
    QToolButton *btnDeleteAll;
    QLabel *label;
    QPushButton *btnRefresh;
    QWidget *finder;
    QComboBox *cmbRecover;
    QToolButton *btnRecover;
    QSpacerItem *horizontalSpacer_2;
    QToolButton *btnDelete;
    QTreeView *treeFiles;
    QToolButton *btnClose;
    QCheckBox *chkShowAll;
    QLabel *lblInfo;

    void setupUi(QWidget *RecoveryWindow)
    {
        if (RecoveryWindow->objectName().isEmpty())
            RecoveryWindow->setObjectName(QString::fromUtf8("RecoveryWindow"));
        RecoveryWindow->resize(512, 360);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(RecoveryWindow->sizePolicy().hasHeightForWidth());
        RecoveryWindow->setSizePolicy(sizePolicy);
        RecoveryWindow->setMinimumSize(QSize(0, 0));
        RecoveryWindow->setMaximumSize(QSize(16777215, 16777215));
        verticalLayout_2 = new QVBoxLayout(RecoveryWindow);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        btnAddFolder = new QPushButton(RecoveryWindow);
        btnAddFolder->setObjectName(QString::fromUtf8("btnAddFolder"));

        gridLayout->addWidget(btnAddFolder, 6, 1, 1, 1);

        btnDeleteAll = new QToolButton(RecoveryWindow);
        btnDeleteAll->setObjectName(QString::fromUtf8("btnDeleteAll"));
        QSizePolicy sizePolicy1(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(btnDeleteAll->sizePolicy().hasHeightForWidth());
        btnDeleteAll->setSizePolicy(sizePolicy1);
        btnDeleteAll->setMinimumSize(QSize(75, 0));

        gridLayout->addWidget(btnDeleteAll, 6, 4, 1, 1);

        label = new QLabel(RecoveryWindow);
        label->setObjectName(QString::fromUtf8("label"));
        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(label, 5, 0, 1, 1);

        btnRefresh = new QPushButton(RecoveryWindow);
        btnRefresh->setObjectName(QString::fromUtf8("btnRefresh"));

        gridLayout->addWidget(btnRefresh, 6, 0, 1, 1);

        finder = new QWidget(RecoveryWindow);
        finder->setObjectName(QString::fromUtf8("finder"));
        finder->setMaximumSize(QSize(16777215, 1));

        gridLayout->addWidget(finder, 3, 0, 1, 1);

        cmbRecover = new QComboBox(RecoveryWindow);
        cmbRecover->setObjectName(QString::fromUtf8("cmbRecover"));

        gridLayout->addWidget(cmbRecover, 5, 1, 1, 4);

        btnRecover = new QToolButton(RecoveryWindow);
        btnRecover->setObjectName(QString::fromUtf8("btnRecover"));
        sizePolicy1.setHeightForWidth(btnRecover->sizePolicy().hasHeightForWidth());
        btnRecover->setSizePolicy(sizePolicy1);
        btnRecover->setMinimumSize(QSize(75, 23));

        gridLayout->addWidget(btnRecover, 5, 5, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer_2, 6, 3, 1, 1);

        btnDelete = new QToolButton(RecoveryWindow);
        btnDelete->setObjectName(QString::fromUtf8("btnDelete"));
        sizePolicy1.setHeightForWidth(btnDelete->sizePolicy().hasHeightForWidth());
        btnDelete->setSizePolicy(sizePolicy1);
        btnDelete->setMinimumSize(QSize(75, 23));

        gridLayout->addWidget(btnDelete, 4, 5, 1, 1);

        treeFiles = new QTreeView(RecoveryWindow);
        treeFiles->setObjectName(QString::fromUtf8("treeFiles"));

        gridLayout->addWidget(treeFiles, 0, 0, 3, 6);

        btnClose = new QToolButton(RecoveryWindow);
        btnClose->setObjectName(QString::fromUtf8("btnClose"));
        sizePolicy1.setHeightForWidth(btnClose->sizePolicy().hasHeightForWidth());
        btnClose->setSizePolicy(sizePolicy1);
        btnClose->setMaximumSize(QSize(16777215, 16777215));

        gridLayout->addWidget(btnClose, 6, 5, 1, 1);

        chkShowAll = new QCheckBox(RecoveryWindow);
        chkShowAll->setObjectName(QString::fromUtf8("chkShowAll"));

        gridLayout->addWidget(chkShowAll, 6, 2, 1, 1);

        lblInfo = new QLabel(RecoveryWindow);
        lblInfo->setObjectName(QString::fromUtf8("lblInfo"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(lblInfo->sizePolicy().hasHeightForWidth());
        lblInfo->setSizePolicy(sizePolicy2);

        gridLayout->addWidget(lblInfo, 4, 0, 1, 5);


        verticalLayout->addLayout(gridLayout);


        verticalLayout_2->addLayout(verticalLayout);


        retranslateUi(RecoveryWindow);

        QMetaObject::connectSlotsByName(RecoveryWindow);
    } // setupUi

    void retranslateUi(QWidget *RecoveryWindow)
    {
        RecoveryWindow->setWindowTitle(QCoreApplication::translate("RecoveryWindow", "SandboxiePlus - Recovery", nullptr));
        btnAddFolder->setText(QCoreApplication::translate("RecoveryWindow", "Add Folder", nullptr));
        btnDeleteAll->setText(QCoreApplication::translate("RecoveryWindow", "Delete Content", nullptr));
        label->setText(QCoreApplication::translate("RecoveryWindow", "Recover target:", nullptr));
        btnRefresh->setText(QCoreApplication::translate("RecoveryWindow", "Refresh", nullptr));
        btnRecover->setText(QCoreApplication::translate("RecoveryWindow", "Recover", nullptr));
        btnDelete->setText(QCoreApplication::translate("RecoveryWindow", "Delete", nullptr));
        btnClose->setText(QCoreApplication::translate("RecoveryWindow", "Close", nullptr));
        chkShowAll->setText(QCoreApplication::translate("RecoveryWindow", "Show All Files", nullptr));
        lblInfo->setText(QCoreApplication::translate("RecoveryWindow", "TextLabel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class RecoveryWindow: public Ui_RecoveryWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_RECOVERYWINDOW_H
