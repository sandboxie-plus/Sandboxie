/********************************************************************************
** Form generated from reading UI file 'SnapshotsWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SNAPSHOTSWINDOW_H
#define UI_SNAPSHOTSWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SnapshotsWindow
{
public:
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QGroupBox *groupBox;
    QGridLayout *gridLayout_3;
    QLabel *label;
    QLineEdit *txtName;
    QCheckBox *chkDefault;
    QPlainTextEdit *txtInfo;
    QLabel *label_2;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout_2;
    QPushButton *btnTake;
    QSpacerItem *verticalSpacer;
    QPushButton *btnRemove;
    QToolButton *btnSelect;
    QTreeView *treeSnapshots;

    void setupUi(QWidget *SnapshotsWindow)
    {
        if (SnapshotsWindow->objectName().isEmpty())
            SnapshotsWindow->setObjectName(QString::fromUtf8("SnapshotsWindow"));
        SnapshotsWindow->resize(573, 451);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(SnapshotsWindow->sizePolicy().hasHeightForWidth());
        SnapshotsWindow->setSizePolicy(sizePolicy);
        SnapshotsWindow->setMinimumSize(QSize(0, 0));
        SnapshotsWindow->setMaximumSize(QSize(16777215, 16777215));
        verticalLayout_2 = new QVBoxLayout(SnapshotsWindow);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        groupBox = new QGroupBox(SnapshotsWindow);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Ignored);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy1);
        groupBox->setMinimumSize(QSize(0, 0));
        groupBox->setMaximumSize(QSize(16777215, 16777215));
        gridLayout_3 = new QGridLayout(groupBox);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout_3->addWidget(label, 0, 0, 1, 1);

        txtName = new QLineEdit(groupBox);
        txtName->setObjectName(QString::fromUtf8("txtName"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Minimum);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(txtName->sizePolicy().hasHeightForWidth());
        txtName->setSizePolicy(sizePolicy2);
        txtName->setMinimumSize(QSize(0, 23));

        gridLayout_3->addWidget(txtName, 0, 1, 1, 1);

        chkDefault = new QCheckBox(groupBox);
        chkDefault->setObjectName(QString::fromUtf8("chkDefault"));
        QSizePolicy sizePolicy3(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(chkDefault->sizePolicy().hasHeightForWidth());
        chkDefault->setSizePolicy(sizePolicy3);
        chkDefault->setMinimumSize(QSize(0, 23));

        gridLayout_3->addWidget(chkDefault, 0, 2, 1, 1);

        txtInfo = new QPlainTextEdit(groupBox);
        txtInfo->setObjectName(QString::fromUtf8("txtInfo"));

        gridLayout_3->addWidget(txtInfo, 1, 1, 2, 2);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setMinimumSize(QSize(0, 23));
        label_2->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);

        gridLayout_3->addWidget(label_2, 1, 0, 1, 1);


        gridLayout->addWidget(groupBox, 1, 0, 1, 1);

        groupBox_2 = new QGroupBox(SnapshotsWindow);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        groupBox_2->setMaximumSize(QSize(16777215, 125));
        gridLayout_2 = new QGridLayout(groupBox_2);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        btnTake = new QPushButton(groupBox_2);
        btnTake->setObjectName(QString::fromUtf8("btnTake"));
        sizePolicy3.setHeightForWidth(btnTake->sizePolicy().hasHeightForWidth());
        btnTake->setSizePolicy(sizePolicy3);
        btnTake->setMinimumSize(QSize(0, 23));

        gridLayout_2->addWidget(btnTake, 0, 0, 1, 1);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_2->addItem(verticalSpacer, 1, 0, 1, 1);

        btnRemove = new QPushButton(groupBox_2);
        btnRemove->setObjectName(QString::fromUtf8("btnRemove"));
        sizePolicy3.setHeightForWidth(btnRemove->sizePolicy().hasHeightForWidth());
        btnRemove->setSizePolicy(sizePolicy3);
        btnRemove->setMinimumSize(QSize(0, 23));

        gridLayout_2->addWidget(btnRemove, 3, 0, 1, 1);

        btnSelect = new QToolButton(groupBox_2);
        btnSelect->setObjectName(QString::fromUtf8("btnSelect"));
        sizePolicy3.setHeightForWidth(btnSelect->sizePolicy().hasHeightForWidth());
        btnSelect->setSizePolicy(sizePolicy3);
        btnSelect->setMinimumSize(QSize(0, 23));

        gridLayout_2->addWidget(btnSelect, 2, 0, 1, 1);


        gridLayout->addWidget(groupBox_2, 1, 1, 1, 1);

        treeSnapshots = new QTreeView(SnapshotsWindow);
        treeSnapshots->setObjectName(QString::fromUtf8("treeSnapshots"));

        gridLayout->addWidget(treeSnapshots, 0, 0, 1, 2);


        verticalLayout->addLayout(gridLayout);


        verticalLayout_2->addLayout(verticalLayout);

        QWidget::setTabOrder(btnTake, treeSnapshots);
        QWidget::setTabOrder(treeSnapshots, btnRemove);
        QWidget::setTabOrder(btnRemove, txtName);
        QWidget::setTabOrder(txtName, txtInfo);

        retranslateUi(SnapshotsWindow);

        QMetaObject::connectSlotsByName(SnapshotsWindow);
    } // setupUi

    void retranslateUi(QWidget *SnapshotsWindow)
    {
        SnapshotsWindow->setWindowTitle(QCoreApplication::translate("SnapshotsWindow", "SandboxiePlus - Snapshots", nullptr));
        groupBox->setTitle(QCoreApplication::translate("SnapshotsWindow", "Selected Snapshot Details", nullptr));
        label->setText(QCoreApplication::translate("SnapshotsWindow", "Name:", nullptr));
#if QT_CONFIG(tooltip)
        chkDefault->setToolTip(QCoreApplication::translate("SnapshotsWindow", "When deleting a snapshot content, it will be returned to this snapshot instead of none.", nullptr));
#endif // QT_CONFIG(tooltip)
        chkDefault->setText(QCoreApplication::translate("SnapshotsWindow", "Default snapshot", nullptr));
        label_2->setText(QCoreApplication::translate("SnapshotsWindow", "Description:", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("SnapshotsWindow", "Snapshot Actions", nullptr));
        btnTake->setText(QCoreApplication::translate("SnapshotsWindow", "Take Snapshot", nullptr));
        btnRemove->setText(QCoreApplication::translate("SnapshotsWindow", "Remove Snapshot", nullptr));
        btnSelect->setText(QCoreApplication::translate("SnapshotsWindow", "Go to Snapshot", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SnapshotsWindow: public Ui_SnapshotsWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SNAPSHOTSWINDOW_H
