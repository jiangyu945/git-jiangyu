#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QHostAddress>
#include <QAbstractSocket>

#include <QImage>
#include <QByteArray>
#include <QImageReader>

#include <QDebug>

#include <QPixmap>

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
    void Sig();

private slots:
    void on_pushButton_connectToServer_clicked();
    void doProcessConnected();
    void doProcessReadyRead();
    void doProcessDisconnected();

    void on_pushButton_sendToServer_clicked();

private:
    Ui::Widget *ui;
    QTcpSocket *myClient;
    QByteArray array;

    void Init();
};



#endif // WIDGET_H
