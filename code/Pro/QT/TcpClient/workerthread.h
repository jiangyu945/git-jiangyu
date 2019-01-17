#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include "widget.h"
#include <QMutex>


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
};

#endif // WORKERTHREAD_H
