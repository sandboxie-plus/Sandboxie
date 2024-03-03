/********************************************************************************
** Form generated from reading UI file 'PopUpWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_POPUPWINDOW_H
#define UI_POPUPWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PopUpWindow
{
public:
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QTableWidget *table;

    void setupUi(QWidget *PopUpWindow)
    {
        if (PopUpWindow->objectName().isEmpty())
            PopUpWindow->setObjectName(QString::fromUtf8("PopUpWindow"));
        PopUpWindow->resize(573, 179);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(PopUpWindow->sizePolicy().hasHeightForWidth());
        PopUpWindow->setSizePolicy(sizePolicy);
        PopUpWindow->setMinimumSize(QSize(0, 0));
        PopUpWindow->setMaximumSize(QSize(16777215, 16777215));
        verticalLayout_2 = new QVBoxLayout(PopUpWindow);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        table = new QTableWidget(PopUpWindow);
        table->setObjectName(QString::fromUtf8("table"));

        verticalLayout->addWidget(table);


        verticalLayout_2->addLayout(verticalLayout);


        retranslateUi(PopUpWindow);

        QMetaObject::connectSlotsByName(PopUpWindow);
    } // setupUi

    void retranslateUi(QWidget *PopUpWindow)
    {
        PopUpWindow->setWindowTitle(QCoreApplication::translate("PopUpWindow", "SandboxiePlus Notifications", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PopUpWindow: public Ui_PopUpWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_POPUPWINDOW_H
