#ifndef MYMONITOR_H
#define MYMONITOR_H

#include <QWidget>
#include <QTcpServer>
#include <QNetworkInterface>   //该类提供主机IP地址和网络接口的列表
#include <QTcpSocket>


namespace Ui {
class TcpServer;
}

class TcpServer : public QWidget
{
    Q_OBJECT

public:
    explicit TcpServer(QWidget *parent = 0);
    ~TcpServer();

private slots:
    void on_pushButton_Bind_clicked();
    void doProcessNewConnection();
    void doProcessConnected();
    //void doProcessAcceptError(QAbstractSocket::SocketError);
    void doProcessDisconnected();
    void doProcessreadyRead();

    void on_pushButton_sendToClient_clicked();

    void on_pushButton_clearRecv_clicked();

private:
    Ui::TcpServer *ui;
    QTcpServer *myServer;
    QTcpSocket *client;
    QList<QTcpSocket *> arrayClient;  //列表，存储客户端socket

    void Init();
};

#endif // MYMONITOR_H
