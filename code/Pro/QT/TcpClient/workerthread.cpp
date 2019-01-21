#include "workerthread.h"

QMutex myMutex;
extern  QByteArray array;
static  QByteArray tmpArray;
extern QSemaphore Sem;

WorkerThread::WorkerThread(QObject *parent) : QObject(parent)
{
    myClient = new QTcpSocket(this);   //资源由父对象this回收
    datasize = 0;
}

WorkerThread::~WorkerThread()
{
}


//连接服务器
void WorkerThread::doProcessConnectToServer(QString serverIP,QString serverPort)
{
    myClient->connectToHost(QHostAddress(serverIP),serverPort.toUInt());

    connect(myClient,SIGNAL(connected()),this,SLOT(doConnected()));

    connect(myClient,SIGNAL(readyRead()),this,SLOT(doProcessRecvData()));

    connect(myClient,SIGNAL(disconnected()),this,SLOT(doDisConnected()));
}

//发送连接成功信号
void WorkerThread::doConnected(){
    emit SigToConnected();
}

//发送连接断开信号
void WorkerThread::doDisConnected(){
    emit SigDisConnected();
}


//数据接收
void WorkerThread::doProcessRecvData(){

    Sem.acquire();  //获取二值信号量
//    qDebug() << "Sem.available(): " << Sem.available();

    qDebug() << "<<<<<<<<<<<<<<<<<<<Now is in doProcessRecvData thread: ";
    //加锁,利用QMutexLocker管理会在该函数结束时自动析构解锁
   QMutexLocker locker(&myMutex);

   //首先获取图片大小
   if(datasize == 0){
       QDataStream stream(myClient);
       stream.setByteOrder(QDataStream::LittleEndian);
       if(myClient->bytesAvailable() < sizeof(quint32))
           return;

       stream>>datasize;
       read_left = datasize;
       qDebug("datasize = %d",datasize);
       qDebug() << "read_left: " << read_left;
   }

   if(myClient->bytesAvailable() < read_left){   //可读长度<剩余待读长度
      read_left -= (quint32)myClient->bytesAvailable();
      tmpArray.append((QByteArray)myClient->readAll());
   }
   else{   //可读长度>=剩余待读长度
      qDebug() <<">= read_left,readable size: " << myClient->bytesAvailable();

      //读取数据
      tmpArray.append((QByteArray)myClient->read(read_left));

      read_left = 0;
      qDebug() << "SockCli->bytesAvailable(): " << myClient->bytesAvailable();

      array.append(tmpArray);
      tmpArray.clear();

      tmpArray.append((QByteArray)myClient->readAll());
      qDebug() << "Received finished! array.size() = " << array.size();
      qDebug() << "emit SigRecvFinished()";
      emit SigRecvFinished();

      datasize = 0;
   }

   Sem.release();  //释放信号量

//    array.append((QByteArray)SockCli->readAll());

//    //发射接收完成信号到主线程(需确保图片接收完整再发送信号，否则无法加载图片)
//    if((uchar)array.at(array.size()-1) == (uchar)0xAA){
//        qDebug() << "Received finished! array.length() = " << array.length();
//        array.remove(array.size()-1,1);
//        qDebug() << "Remove success! array.length() = " << array.length();

//        qDebug() << "emit SigRecvFinished()";
//        emit SigRecvFinished();

//        Sem.release();  //释放信号量
//        qDebug() << "Sem.available(): " << Sem.available();
//    }
}

