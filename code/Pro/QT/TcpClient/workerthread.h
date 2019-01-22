#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include "widget.h"
#include <QObject>

#include <QTcpSocket>
#include <QHostAddress>
#include <QAbstractSocket>

#include <QThread>
#include <QTime>
#include <QDebug>

#include <QMutex>
#include <QMutexLocker>

#include <QDataStream>
#include <QByteArray>

class WorkerThread : public QObject
{
    Q_OBJECT
public:
    explicit WorkerThread(QObject *parent = 0);

signals:   
    void SigToConnected();   //连接成功信号
    void SigDisConnected();  //连接断开信号
    void SigRecvFinished();  //接收完成信号

public slots:    
    void doProcessConnectToServer(QString,QString);  //连接服务器槽

    void doConnected();       //连接成功槽
    void doDisConnected();    //连接断开槽
    void doProcessRecvData(); //接收完成槽

private:
    QTcpSocket *myClient;

    quint32 datasize;  //图片大小
    quint32 read_left; //剩余待读数据长度
    QTime tt;


};

#endif // WORKERTHREAD_H
