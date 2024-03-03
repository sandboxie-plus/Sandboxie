/********************************************************************************
** Form generated from reading UI file 'CompressDialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COMPRESSDIALOG_H
#define UI_COMPRESSDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_CompressDialog
{
public:
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QComboBox *cmbCompression;
    QSpacerItem *verticalSpacer;
    QLabel *label;
    QCheckBox *chkEncrypt;
    QCheckBox *chkSolid;
    QLabel *label_2;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *CompressDialog)
    {
        if (CompressDialog->objectName().isEmpty())
            CompressDialog->setObjectName(QString::fromUtf8("CompressDialog"));
        CompressDialog->resize(424, 207);
        verticalLayout = new QVBoxLayout(CompressDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        cmbCompression = new QComboBox(CompressDialog);
        cmbCompression->setObjectName(QString::fromUtf8("cmbCompression"));

        gridLayout->addWidget(cmbCompression, 1, 1, 1, 1);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer, 3, 0, 1, 1);

        label = new QLabel(CompressDialog);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 1, 0, 1, 1);

        chkEncrypt = new QCheckBox(CompressDialog);
        chkEncrypt->setObjectName(QString::fromUtf8("chkEncrypt"));

        gridLayout->addWidget(chkEncrypt, 4, 1, 1, 1);

        chkSolid = new QCheckBox(CompressDialog);
        chkSolid->setObjectName(QString::fromUtf8("chkSolid"));

        gridLayout->addWidget(chkSolid, 2, 1, 1, 1);

        label_2 = new QLabel(CompressDialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setWordWrap(true);

        gridLayout->addWidget(label_2, 0, 0, 1, 2);


        verticalLayout->addLayout(gridLayout);

        buttonBox = new QDialogButtonBox(CompressDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(CompressDialog);

        QMetaObject::connectSlotsByName(CompressDialog);
    } // setupUi

    void retranslateUi(QDialog *CompressDialog)
    {
        CompressDialog->setWindowTitle(QCoreApplication::translate("CompressDialog", "Compress Files", nullptr));
        label->setText(QCoreApplication::translate("CompressDialog", "Compression", nullptr));
#if QT_CONFIG(tooltip)
        chkEncrypt->setToolTip(QCoreApplication::translate("CompressDialog", "When selected you will be prompted for a password after clicking OK", nullptr));
#endif // QT_CONFIG(tooltip)
        chkEncrypt->setText(QCoreApplication::translate("CompressDialog", "Encrypt archive content", nullptr));
#if QT_CONFIG(tooltip)
        chkSolid->setToolTip(QCoreApplication::translate("CompressDialog", "Solid archiving improves compression ratios by treating multiple files as a single continuous data block. Ideal for a large number of small files, it makes the archive more compact but may increase the time required for extracting individual files.", nullptr));
#endif // QT_CONFIG(tooltip)
        chkSolid->setText(QCoreApplication::translate("CompressDialog", "Create Solide Archive", nullptr));
        label_2->setText(QCoreApplication::translate("CompressDialog", "Export Sandbox to a 7z archive, Choose Your Compression Rate and Customize Additional Compression Settings.", nullptr));
    } // retranslateUi

};

namespace Ui {
    class CompressDialog: public Ui_CompressDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COMPRESSDIALOG_H
