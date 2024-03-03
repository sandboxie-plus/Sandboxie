/********************************************************************************
** Form generated from reading UI file 'BoxImageWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_BOXIMAGEWINDOW_H
#define UI_BOXIMAGEWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_BoxImageWindow
{
public:
    QGridLayout *gridLayout_2;
    QGridLayout *gridLayout;
    QLabel *lblImageSizeKb;
    QLineEdit *txtNewPassword;
    QLineEdit *txtRepeatPassword;
    QCheckBox *chkProtect;
    QLineEdit *txtImageSize;
    QDialogButtonBox *buttonBox;
    QComboBox *cmbCipher;
    QLineEdit *txtPassword;
    QLabel *lblIcon;
    QCheckBox *chkShow;
    QLabel *lblPassword;
    QLabel *lblNewPassword;
    QLabel *lblRepeatPassword;
    QLabel *lblImageSize;
    QLabel *lblCipher;
    QLabel *lblInfo;

    void setupUi(QWidget *BoxImageWindow)
    {
        if (BoxImageWindow->objectName().isEmpty())
            BoxImageWindow->setObjectName(QString::fromUtf8("BoxImageWindow"));
        BoxImageWindow->resize(518, 268);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(BoxImageWindow->sizePolicy().hasHeightForWidth());
        BoxImageWindow->setSizePolicy(sizePolicy);
        BoxImageWindow->setMinimumSize(QSize(500, 0));
        gridLayout_2 = new QGridLayout(BoxImageWindow);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        lblImageSizeKb = new QLabel(BoxImageWindow);
        lblImageSizeKb->setObjectName(QString::fromUtf8("lblImageSizeKb"));
        QSizePolicy sizePolicy1(QSizePolicy::Maximum, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(lblImageSizeKb->sizePolicy().hasHeightForWidth());
        lblImageSizeKb->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(lblImageSizeKb, 5, 3, 1, 1);

        txtNewPassword = new QLineEdit(BoxImageWindow);
        txtNewPassword->setObjectName(QString::fromUtf8("txtNewPassword"));
        txtNewPassword->setEchoMode(QLineEdit::Password);

        gridLayout->addWidget(txtNewPassword, 2, 2, 1, 3);

        txtRepeatPassword = new QLineEdit(BoxImageWindow);
        txtRepeatPassword->setObjectName(QString::fromUtf8("txtRepeatPassword"));
        txtRepeatPassword->setEchoMode(QLineEdit::Password);

        gridLayout->addWidget(txtRepeatPassword, 3, 2, 1, 3);

        chkProtect = new QCheckBox(BoxImageWindow);
        chkProtect->setObjectName(QString::fromUtf8("chkProtect"));
        chkProtect->setChecked(true);

        gridLayout->addWidget(chkProtect, 7, 2, 1, 3);

        txtImageSize = new QLineEdit(BoxImageWindow);
        txtImageSize->setObjectName(QString::fromUtf8("txtImageSize"));
        txtImageSize->setMaximumSize(QSize(100, 16777215));

        gridLayout->addWidget(txtImageSize, 5, 2, 1, 1);

        buttonBox = new QDialogButtonBox(BoxImageWindow);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 8, 0, 1, 5);

        cmbCipher = new QComboBox(BoxImageWindow);
        cmbCipher->setObjectName(QString::fromUtf8("cmbCipher"));

        gridLayout->addWidget(cmbCipher, 6, 2, 1, 2);

        txtPassword = new QLineEdit(BoxImageWindow);
        txtPassword->setObjectName(QString::fromUtf8("txtPassword"));
        txtPassword->setEchoMode(QLineEdit::Password);

        gridLayout->addWidget(txtPassword, 1, 2, 1, 3);

        lblIcon = new QLabel(BoxImageWindow);
        lblIcon->setObjectName(QString::fromUtf8("lblIcon"));

        gridLayout->addWidget(lblIcon, 0, 0, 1, 1);

        chkShow = new QCheckBox(BoxImageWindow);
        chkShow->setObjectName(QString::fromUtf8("chkShow"));

        gridLayout->addWidget(chkShow, 4, 4, 1, 1);

        lblPassword = new QLabel(BoxImageWindow);
        lblPassword->setObjectName(QString::fromUtf8("lblPassword"));
        lblPassword->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(lblPassword, 1, 0, 1, 2);

        lblNewPassword = new QLabel(BoxImageWindow);
        lblNewPassword->setObjectName(QString::fromUtf8("lblNewPassword"));
        lblNewPassword->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(lblNewPassword, 2, 0, 1, 2);

        lblRepeatPassword = new QLabel(BoxImageWindow);
        lblRepeatPassword->setObjectName(QString::fromUtf8("lblRepeatPassword"));
        lblRepeatPassword->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(lblRepeatPassword, 3, 0, 1, 2);

        lblImageSize = new QLabel(BoxImageWindow);
        lblImageSize->setObjectName(QString::fromUtf8("lblImageSize"));
        lblImageSize->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(lblImageSize, 5, 0, 1, 2);

        lblCipher = new QLabel(BoxImageWindow);
        lblCipher->setObjectName(QString::fromUtf8("lblCipher"));
        lblCipher->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout->addWidget(lblCipher, 6, 0, 1, 2);

        lblInfo = new QLabel(BoxImageWindow);
        lblInfo->setObjectName(QString::fromUtf8("lblInfo"));
        lblInfo->setWordWrap(true);

        gridLayout->addWidget(lblInfo, 0, 1, 1, 4);


        gridLayout_2->addLayout(gridLayout, 0, 0, 1, 1);


        retranslateUi(BoxImageWindow);

        QMetaObject::connectSlotsByName(BoxImageWindow);
    } // setupUi

    void retranslateUi(QWidget *BoxImageWindow)
    {
        BoxImageWindow->setWindowTitle(QCoreApplication::translate("BoxImageWindow", "Form", nullptr));
        lblImageSizeKb->setText(QCoreApplication::translate("BoxImageWindow", "kilobytes", nullptr));
        chkProtect->setText(QCoreApplication::translate("BoxImageWindow", "Protect Box Root from access by unsandboxed processes", nullptr));
        lblIcon->setText(QCoreApplication::translate("BoxImageWindow", "TextLabel", nullptr));
        chkShow->setText(QCoreApplication::translate("BoxImageWindow", "Show Password", nullptr));
        lblPassword->setText(QCoreApplication::translate("BoxImageWindow", "Enter Password", nullptr));
        lblNewPassword->setText(QCoreApplication::translate("BoxImageWindow", "New Password", nullptr));
        lblRepeatPassword->setText(QCoreApplication::translate("BoxImageWindow", "Repeat Password", nullptr));
        lblImageSize->setText(QCoreApplication::translate("BoxImageWindow", "Disk Image Size", nullptr));
        lblCipher->setText(QCoreApplication::translate("BoxImageWindow", "Encryption Cipher", nullptr));
        lblInfo->setText(QCoreApplication::translate("BoxImageWindow", "TextLabel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class BoxImageWindow: public Ui_BoxImageWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BOXIMAGEWINDOW_H
