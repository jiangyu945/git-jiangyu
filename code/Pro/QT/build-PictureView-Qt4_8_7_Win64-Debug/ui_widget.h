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
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QLabel *labelShow;
    QSlider *horizontalSlider;
    QPushButton *pushButtonQuit;
    QWidget *widget;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButtonOpenPic;
    QPushButton *pushButtonOpenMov;
    QPushButton *pushButtonStart;
    QPushButton *pushButtonStop;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName(QString::fromUtf8("Widget"));
        Widget->resize(480, 400);
        Widget->setMinimumSize(QSize(480, 400));
        Widget->setMaximumSize(QSize(65535, 65535));
        Widget->setMouseTracking(false);
        Widget->setFocusPolicy(Qt::NoFocus);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/new/prefix1/Network.ico"), QSize(), QIcon::Normal, QIcon::Off);
        Widget->setWindowIcon(icon);
        Widget->setStyleSheet(QString::fromUtf8(""));
        labelShow = new QLabel(Widget);
        labelShow->setObjectName(QString::fromUtf8("labelShow"));
        labelShow->setGeometry(QRect(9, 9, 462, 321));
        labelShow->setStyleSheet(QString::fromUtf8("image: url(:/new/prefix1/background-image/player.png);\n"
"background-color: rgb(0, 0, 0);"));
        horizontalSlider = new QSlider(Widget);
        horizontalSlider->setObjectName(QString::fromUtf8("horizontalSlider"));
        horizontalSlider->setGeometry(QRect(160, 370, 311, 22));
        horizontalSlider->setStyleSheet(QString::fromUtf8(""));
        horizontalSlider->setOrientation(Qt::Horizontal);
        pushButtonQuit = new QPushButton(Widget);
        pushButtonQuit->setObjectName(QString::fromUtf8("pushButtonQuit"));
        pushButtonQuit->setGeometry(QRect(10, 370, 75, 23));
        widget = new QWidget(Widget);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(11, 333, 466, 26));
        horizontalLayout = new QHBoxLayout(widget);
        horizontalLayout->setSpacing(50);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        pushButtonOpenPic = new QPushButton(widget);
        pushButtonOpenPic->setObjectName(QString::fromUtf8("pushButtonOpenPic"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(pushButtonOpenPic->sizePolicy().hasHeightForWidth());
        pushButtonOpenPic->setSizePolicy(sizePolicy);
        pushButtonOpenPic->setStyleSheet(QString::fromUtf8("color: rgb(5, 200, 21);"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/new/prefix1/Folder.ico"), QSize(), QIcon::Normal, QIcon::Off);
        pushButtonOpenPic->setIcon(icon1);

        horizontalLayout->addWidget(pushButtonOpenPic);

        pushButtonOpenMov = new QPushButton(widget);
        pushButtonOpenMov->setObjectName(QString::fromUtf8("pushButtonOpenMov"));
        sizePolicy.setHeightForWidth(pushButtonOpenMov->sizePolicy().hasHeightForWidth());
        pushButtonOpenMov->setSizePolicy(sizePolicy);
        pushButtonOpenMov->setStyleSheet(QString::fromUtf8("color: rgb(48, 160, 200);"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/new/prefix1/Movies 2.ico"), QSize(), QIcon::Normal, QIcon::Off);
        pushButtonOpenMov->setIcon(icon2);

        horizontalLayout->addWidget(pushButtonOpenMov);

        pushButtonStart = new QPushButton(widget);
        pushButtonStart->setObjectName(QString::fromUtf8("pushButtonStart"));
        sizePolicy.setHeightForWidth(pushButtonStart->sizePolicy().hasHeightForWidth());
        pushButtonStart->setSizePolicy(sizePolicy);
        pushButtonStart->setStyleSheet(QString::fromUtf8("color: rgb(17, 200, 7);"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/new/prefix1/Sharingan.ico"), QSize(), QIcon::Normal, QIcon::Off);
        pushButtonStart->setIcon(icon3);

        horizontalLayout->addWidget(pushButtonStart);

        pushButtonStop = new QPushButton(widget);
        pushButtonStop->setObjectName(QString::fromUtf8("pushButtonStop"));
        sizePolicy.setHeightForWidth(pushButtonStop->sizePolicy().hasHeightForWidth());
        pushButtonStop->setSizePolicy(sizePolicy);
        pushButtonStop->setStyleSheet(QString::fromUtf8("color: rgb(200, 58, 40);"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/new/prefix1/Pictures (2).ico"), QSize(), QIcon::Normal, QIcon::Off);
        pushButtonStop->setIcon(icon4);

        horizontalLayout->addWidget(pushButtonStop);


        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QApplication::translate("Widget", "Sharingan", 0, QApplication::UnicodeUTF8));
        labelShow->setText(QString());
        pushButtonQuit->setText(QApplication::translate("Widget", "\351\200\200\345\207\272", 0, QApplication::UnicodeUTF8));
        pushButtonOpenPic->setText(QApplication::translate("Widget", "\346\211\223\345\274\200\345\233\276\347\211\207", 0, QApplication::UnicodeUTF8));
        pushButtonOpenMov->setText(QApplication::translate("Widget", "\346\211\223\345\274\200\345\212\250\346\200\201\345\233\276", 0, QApplication::UnicodeUTF8));
        pushButtonStart->setText(QApplication::translate("Widget", "\346\222\255\346\224\276", 0, QApplication::UnicodeUTF8));
        pushButtonStop->setText(QApplication::translate("Widget", "\345\201\234\346\255\242", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
