#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

#include <QImage>
#include <QByteArray>
#include <QImageReader>

#include <QDebug>

#include <QPixmap>
#include <QThread>
#include <QTimer>
#include <QTime>

#include <QBuffer>
#include <QImageReader>
#include <QSemaphore>

#include <QPainter>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

signals:
    void SigToConnect(QString,QString);

private slots:
    void on_pushButton_connectToServer_clicked();
    void doProcessConnected();
    void doProcessDisconnected();

    void doProcessShow();

private:
    Ui::Widget *ui;

    QString serverIP;
    QString serverPort;

    QThread worker;
    QTimer mytimer;
    QTime  t;
    QPainter mypainter;

    void Init();
    void startObjthread();
};



#endif // WIDGET_H
