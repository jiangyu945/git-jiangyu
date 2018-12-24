/********************************************************************************
** Form generated from reading UI file 'mymonitor.ui'
**
** Created by: Qt User Interface Compiler version 4.8.7
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MYMONITOR_H
#define UI_MYMONITOR_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Mymonitor
{
public:
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label_ServerIp;
    QLineEdit *lineEdit_ServerIp;
    QLabel *label_Port;
    QLineEdit *lineEdit_ServerPort;
    QPushButton *pushButton_Bind;
    QPushButton *pushButton_clearRecv;
    QTextEdit *textEdit_Server;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_ServerIp_2;
    QLineEdit *lineEdit_clientIP;
    QLabel *label_Port_2;
    QLineEdit *lineEdit_clientPort;
    QPushButton *pushButton_sendToClient;
    QTextEdit *textEdit_Client;

    void setupUi(QWidget *Mymonitor)
    {
        if (Mymonitor->objectName().isEmpty())
            Mymonitor->setObjectName(QString::fromUtf8("Mymonitor"));
        Mymonitor->resize(585, 513);
        widget = new QWidget(Mymonitor);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(10, 12, 501, 483));
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_ServerIp = new QLabel(widget);
        label_ServerIp->setObjectName(QString::fromUtf8("label_ServerIp"));

        horizontalLayout->addWidget(label_ServerIp);

        lineEdit_ServerIp = new QLineEdit(widget);
        lineEdit_ServerIp->setObjectName(QString::fromUtf8("lineEdit_ServerIp"));

        horizontalLayout->addWidget(lineEdit_ServerIp);

        label_Port = new QLabel(widget);
        label_Port->setObjectName(QString::fromUtf8("label_Port"));

        horizontalLayout->addWidget(label_Port);

        lineEdit_ServerPort = new QLineEdit(widget);
        lineEdit_ServerPort->setObjectName(QString::fromUtf8("lineEdit_ServerPort"));

        horizontalLayout->addWidget(lineEdit_ServerPort);

        pushButton_Bind = new QPushButton(widget);
        pushButton_Bind->setObjectName(QString::fromUtf8("pushButton_Bind"));

        horizontalLayout->addWidget(pushButton_Bind);


        verticalLayout->addLayout(horizontalLayout);

        pushButton_clearRecv = new QPushButton(widget);
        pushButton_clearRecv->setObjectName(QString::fromUtf8("pushButton_clearRecv"));

        verticalLayout->addWidget(pushButton_clearRecv);

        textEdit_Server = new QTextEdit(widget);
        textEdit_Server->setObjectName(QString::fromUtf8("textEdit_Server"));

        verticalLayout->addWidget(textEdit_Server);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_ServerIp_2 = new QLabel(widget);
        label_ServerIp_2->setObjectName(QString::fromUtf8("label_ServerIp_2"));

        horizontalLayout_2->addWidget(label_ServerIp_2);

        lineEdit_clientIP = new QLineEdit(widget);
        lineEdit_clientIP->setObjectName(QString::fromUtf8("lineEdit_clientIP"));

        horizontalLayout_2->addWidget(lineEdit_clientIP);

        label_Port_2 = new QLabel(widget);
        label_Port_2->setObjectName(QString::fromUtf8("label_Port_2"));

        horizontalLayout_2->addWidget(label_Port_2);

        lineEdit_clientPort = new QLineEdit(widget);
        lineEdit_clientPort->setObjectName(QString::fromUtf8("lineEdit_clientPort"));

        horizontalLayout_2->addWidget(lineEdit_clientPort);

        pushButton_sendToClient = new QPushButton(widget);
        pushButton_sendToClient->setObjectName(QString::fromUtf8("pushButton_sendToClient"));

        horizontalLayout_2->addWidget(pushButton_sendToClient);


        verticalLayout->addLayout(horizontalLayout_2);

        textEdit_Client = new QTextEdit(widget);
        textEdit_Client->setObjectName(QString::fromUtf8("textEdit_Client"));

        verticalLayout->addWidget(textEdit_Client);


        retranslateUi(Mymonitor);

        QMetaObject::connectSlotsByName(Mymonitor);
    } // setupUi

    void retranslateUi(QWidget *Mymonitor)
    {
        Mymonitor->setWindowTitle(QApplication::translate("Mymonitor", "Mymonitor", 0, QApplication::UnicodeUTF8));
        label_ServerIp->setText(QApplication::translate("Mymonitor", "\346\234\215\345\212\241\345\231\250IP\357\274\232", 0, QApplication::UnicodeUTF8));
        label_Port->setText(QApplication::translate("Mymonitor", "\346\234\215\345\212\241\345\231\250\347\253\257\345\217\243\357\274\232", 0, QApplication::UnicodeUTF8));
        pushButton_Bind->setText(QApplication::translate("Mymonitor", "\347\273\221\345\256\232", 0, QApplication::UnicodeUTF8));
        pushButton_clearRecv->setText(QApplication::translate("Mymonitor", "\346\270\205\347\251\272", 0, QApplication::UnicodeUTF8));
        label_ServerIp_2->setText(QApplication::translate("Mymonitor", "\345\256\242\346\210\267\347\253\257IP\357\274\232", 0, QApplication::UnicodeUTF8));
        label_Port_2->setText(QApplication::translate("Mymonitor", "\345\256\242\346\210\267\347\253\257\347\253\257\345\217\243\357\274\232", 0, QApplication::UnicodeUTF8));
        pushButton_sendToClient->setText(QApplication::translate("Mymonitor", "\345\217\221\351\200\201", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Mymonitor: public Ui_Mymonitor {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MYMONITOR_H
