#include "widget.h"
#include "ui_widget.h"

MyThread::MyThread(QObject *parent) :
    QThread(parent)
{
}

void MyThread::run()
{
    timer = new QTimer;
    timer->setSingleShot(false);
    timer->start(5000);

    qDebug() << "thread1 id: " << QThread::currentThreadId();


    connect(timer, SIGNAL(timeout()), this, SLOT(slotFunc()),Qt::DirectConnection);
    exec();
    qDebug() << "*********************";
}

void MyThread::slotFunc(){
    qDebug() << "timer timeout " << i++ << ":" << QThread::currentThreadId();
    qDebug() << "*********************";
    if(i > 5){
        quit();
    }
}
