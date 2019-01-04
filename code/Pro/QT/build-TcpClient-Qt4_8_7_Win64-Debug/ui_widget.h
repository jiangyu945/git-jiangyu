/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 4.8.7
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

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

class Ui_Widget
{
public:
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label_serverIp;
    QLineEdit *lineEdit_ServerIP;
    QLabel *label_serverPort;
    QLineEdit *lineEdit_ServerPort;
    QPushButton *pushButton_connectToServer;
    QTextEdit *textEdit_RecvBuf;
    QPushButton *pushButton_sendToServer;
    QTextEdit *textEdit_SenBbuf;
    QLabel *label_show;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName(QString::fromUtf8("Widget"));
        Widget->resize(834, 369);
        layoutWidget = new QWidget(Widget);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(400, 0, 431, 361));
        verticalLayout = new QVBoxLayout(layoutWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_serverIp = new QLabel(layoutWidget);
        label_serverIp->setObjectName(QString::fromUtf8("label_serverIp"));

        horizontalLayout->addWidget(label_serverIp);

        lineEdit_ServerIP = new QLineEdit(layoutWidget);
        lineEdit_ServerIP->setObjectName(QString::fromUtf8("lineEdit_ServerIP"));

        horizontalLayout->addWidget(lineEdit_ServerIP);

        label_serverPort = new QLabel(layoutWidget);
        label_serverPort->setObjectName(QString::fromUtf8("label_serverPort"));

        horizontalLayout->addWidget(label_serverPort);

        lineEdit_ServerPort = new QLineEdit(layoutWidget);
        lineEdit_ServerPort->setObjectName(QString::fromUtf8("lineEdit_ServerPort"));
        lineEdit_ServerPort->setEnabled(true);

        horizontalLayout->addWidget(lineEdit_ServerPort);

        pushButton_connectToServer = new QPushButton(layoutWidget);
        pushButton_connectToServer->setObjectName(QString::fromUtf8("pushButton_connectToServer"));

        horizontalLayout->addWidget(pushButton_connectToServer);


        verticalLayout->addLayout(horizontalLayout);

        textEdit_RecvBuf = new QTextEdit(layoutWidget);
        textEdit_RecvBuf->setObjectName(QString::fromUtf8("textEdit_RecvBuf"));

        verticalLayout->addWidget(textEdit_RecvBuf);

        pushButton_sendToServer = new QPushButton(layoutWidget);
        pushButton_sendToServer->setObjectName(QString::fromUtf8("pushButton_sendToServer"));

        verticalLayout->addWidget(pushButton_sendToServer);

        textEdit_SenBbuf = new QTextEdit(layoutWidget);
        textEdit_SenBbuf->setObjectName(QString::fromUtf8("textEdit_SenBbuf"));

        verticalLayout->addWidget(textEdit_SenBbuf);

        label_show = new QLabel(Widget);
        label_show->setObjectName(QString::fromUtf8("label_show"));
        label_show->setGeometry(QRect(9, 9, 381, 351));

        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QApplication::translate("Widget", "TcpClient", 0, QApplication::UnicodeUTF8));
        label_serverIp->setText(QApplication::translate("Widget", "\346\234\215\345\212\241\345\231\250IP\357\274\232", 0, QApplication::UnicodeUTF8));
        label_serverPort->setText(QApplication::translate("Widget", "\346\234\215\345\212\241\345\231\250\347\253\257\345\217\243\357\274\232", 0, QApplication::UnicodeUTF8));
        pushButton_connectToServer->setText(QApplication::translate("Widget", "\350\277\236\346\216\245", 0, QApplication::UnicodeUTF8));
        pushButton_sendToServer->setText(QApplication::translate("Widget", "\345\217\221\351\200\201", 0, QApplication::UnicodeUTF8));
        label_show->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
