/********************************************************************************
** Form generated from reading UI file 'TcpServer.ui'
**
** Created by: Qt User Interface Compiler version 4.8.7
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TCPSERVER_H
#define UI_TCPSERVER_H

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

class Ui_TcpServer
{
public:
    QWidget *layoutWidget;
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

    void setupUi(QWidget *TcpServer)
    {
        if (TcpServer->objectName().isEmpty())
            TcpServer->setObjectName(QString::fromUtf8("TcpServer"));
        TcpServer->resize(585, 513);
        layoutWidget = new QWidget(TcpServer);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(10, 12, 501, 483));
        verticalLayout = new QVBoxLayout(layoutWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_ServerIp = new QLabel(layoutWidget);
        label_ServerIp->setObjectName(QString::fromUtf8("label_ServerIp"));

        horizontalLayout->addWidget(label_ServerIp);

        lineEdit_ServerIp = new QLineEdit(layoutWidget);
        lineEdit_ServerIp->setObjectName(QString::fromUtf8("lineEdit_ServerIp"));

        horizontalLayout->addWidget(lineEdit_ServerIp);

        label_Port = new QLabel(layoutWidget);
        label_Port->setObjectName(QString::fromUtf8("label_Port"));

        horizontalLayout->addWidget(label_Port);

        lineEdit_ServerPort = new QLineEdit(layoutWidget);
        lineEdit_ServerPort->setObjectName(QString::fromUtf8("lineEdit_ServerPort"));

        horizontalLayout->addWidget(lineEdit_ServerPort);

        pushButton_Bind = new QPushButton(layoutWidget);
        pushButton_Bind->setObjectName(QString::fromUtf8("pushButton_Bind"));

        horizontalLayout->addWidget(pushButton_Bind);


        verticalLayout->addLayout(horizontalLayout);

        pushButton_clearRecv = new QPushButton(layoutWidget);
        pushButton_clearRecv->setObjectName(QString::fromUtf8("pushButton_clearRecv"));

        verticalLayout->addWidget(pushButton_clearRecv);

        textEdit_Server = new QTextEdit(layoutWidget);
        textEdit_Server->setObjectName(QString::fromUtf8("textEdit_Server"));

        verticalLayout->addWidget(textEdit_Server);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_ServerIp_2 = new QLabel(layoutWidget);
        label_ServerIp_2->setObjectName(QString::fromUtf8("label_ServerIp_2"));

        horizontalLayout_2->addWidget(label_ServerIp_2);

        lineEdit_clientIP = new QLineEdit(layoutWidget);
        lineEdit_clientIP->setObjectName(QString::fromUtf8("lineEdit_clientIP"));

        horizontalLayout_2->addWidget(lineEdit_clientIP);

        label_Port_2 = new QLabel(layoutWidget);
        label_Port_2->setObjectName(QString::fromUtf8("label_Port_2"));

        horizontalLayout_2->addWidget(label_Port_2);

        lineEdit_clientPort = new QLineEdit(layoutWidget);
        lineEdit_clientPort->setObjectName(QString::fromUtf8("lineEdit_clientPort"));

        horizontalLayout_2->addWidget(lineEdit_clientPort);

        pushButton_sendToClient = new QPushButton(layoutWidget);
        pushButton_sendToClient->setObjectName(QString::fromUtf8("pushButton_sendToClient"));

        horizontalLayout_2->addWidget(pushButton_sendToClient);


        verticalLayout->addLayout(horizontalLayout_2);

        textEdit_Client = new QTextEdit(layoutWidget);
        textEdit_Client->setObjectName(QString::fromUtf8("textEdit_Client"));

        verticalLayout->addWidget(textEdit_Client);


        retranslateUi(TcpServer);

        QMetaObject::connectSlotsByName(TcpServer);
    } // setupUi

    void retranslateUi(QWidget *TcpServer)
    {
        TcpServer->setWindowTitle(QApplication::translate("TcpServer", "TcpServer", 0, QApplication::UnicodeUTF8));
        label_ServerIp->setText(QApplication::translate("TcpServer", "\346\234\215\345\212\241\345\231\250IP\357\274\232", 0, QApplication::UnicodeUTF8));
        label_Port->setText(QApplication::translate("TcpServer", "\346\234\215\345\212\241\345\231\250\347\253\257\345\217\243\357\274\232", 0, QApplication::UnicodeUTF8));
        pushButton_Bind->setText(QApplication::translate("TcpServer", "\347\273\221\345\256\232", 0, QApplication::UnicodeUTF8));
        pushButton_clearRecv->setText(QApplication::translate("TcpServer", "\346\270\205\347\251\272", 0, QApplication::UnicodeUTF8));
        label_ServerIp_2->setText(QApplication::translate("TcpServer", "\345\256\242\346\210\267\347\253\257IP\357\274\232", 0, QApplication::UnicodeUTF8));
        label_Port_2->setText(QApplication::translate("TcpServer", "\345\256\242\346\210\267\347\253\257\347\253\257\345\217\243\357\274\232", 0, QApplication::UnicodeUTF8));
        pushButton_sendToClient->setText(QApplication::translate("TcpServer", "\345\217\221\351\200\201", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class TcpServer: public Ui_TcpServer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TCPSERVER_H
