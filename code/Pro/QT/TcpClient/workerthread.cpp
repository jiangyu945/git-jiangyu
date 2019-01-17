#include "workerthread.h"

QMutex myMutex;

WorkerThread::WorkerThread(QObject *parent) : QObject(parent)
{
}

WorkerThread::~WorkerThread()
{
}

void WorkerThread::doProcessRecvData(QTcpSocket* SockCli){

    qDebug() << "Receive data in Thread ID: " << QThread::currentThreadId();

    //加锁
    myMutex.lock();

    //清空接收缓冲区
    array.clear();

    //接收图像数据
    while(!SockCli->atEnd()){
            array.append(QByteArray(SockCli->readAll()));
        }


    //发射接收完成信号到主线程
        qDebug() << "emit SigRecvFinished()" << endl;
        emit SigRecvFinished();

    //解锁
    myMutex.unlock();
}

