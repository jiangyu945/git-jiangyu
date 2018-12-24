#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QHostAddress>
#include <QAbstractSocket>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void on_pushButton_connectToServer_clicked();
    void doProcessConnected();
    void doProcessReadyRead();
    void doProcessError(QAbstractSocket::SocketError);
    void doProcessDisconnected();

    void on_pushButton_sendToServer_clicked();

private:
    Ui::Widget *ui;
    QTcpSocket *myClient;
    void Init();
};

#endif // WIDGET_H
