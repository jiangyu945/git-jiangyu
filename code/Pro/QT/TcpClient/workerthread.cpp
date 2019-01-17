#include "workerthread.h"

QMutex myMutex;
extern  QByteArray array;

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

    //接收图像数据
    while(!SockCli->atEnd()){
            array.append(QByteArray(SockCli->readAll()));
        }

    qDebug() << "array.length = " << array.length();
    //发射接收完成信号到主线程(需确保图片接收完整再发送信号，否则无法加载图片)
    if(array.length()%154189 == 0){
        qDebug() << "emit SigRecvFinished()" << endl;
        emit SigRecvFinished();
    }
    //解锁
    myMutex.unlock();
}

