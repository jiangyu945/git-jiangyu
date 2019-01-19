#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include "widget.h"
#include <QMutex>
#include <QMutexLocker>
#include <QDataStream>


class WorkerThread : public QObject
{
    Q_OBJECT
public:
    explicit WorkerThread(QObject *parent = 0);
    ~WorkerThread();

signals:
    void SigRecvFinished();

public slots:
    void doProcessRecvData(QTcpSocket* SockCli);

private:
    quint32 datasize;  //图片大小
    quint32 read_left; //剩余待读数据长度
};

#endif // WORKERTHREAD_H
