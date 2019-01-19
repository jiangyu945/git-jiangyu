#include "workerthread.h"

QMutex myMutex;
extern  QByteArray array;
static  QByteArray tmpArray;
extern QSemaphore Sem;

WorkerThread::WorkerThread(QObject *parent) : QObject(parent)
{
    datasize = 0;
}

WorkerThread::~WorkerThread()
{
}

//数据接收
void WorkerThread::doProcessRecvData(QTcpSocket* SockCli){

    qDebug() << "<<<<<<<<<<<<<<<<<<<Now is in doProcessRecvData thread: ";
    //加锁,利用QMutexLocker管理会在该函数结束时自动析构解锁
   QMutexLocker locker(&myMutex);

   //首先获取图片大小
   if(datasize == 0){
       qDebug() << "Getting the size of data...";
       QDataStream stream(SockCli);
       stream.setByteOrder(QDataStream::LittleEndian);
       if(SockCli->bytesAvailable() < sizeof(quint32))
           return;

       stream>>datasize;
       read_left = datasize;
       qDebug("datasize = %d",datasize);
       qDebug() << "read_left: " << read_left;
   }

   if(SockCli->bytesAvailable() < read_left){   //可读长度<剩余待读长度
      qDebug() << "< read_left,readable size: " << SockCli->bytesAvailable();
      read_left -= (quint32)SockCli->bytesAvailable();
      qDebug() << " read size: " << SockCli->bytesAvailable();
      qDebug() << "read_left: " << read_left;
      tmpArray.append((QByteArray)SockCli->readAll());
   }
   else{   //可读长度>=剩余待读长度
      qDebug() <<">= read_left,readable size: " << SockCli->bytesAvailable();

      //读取数据
      tmpArray.append((QByteArray)SockCli->read(read_left));

      read_left = 0;
      qDebug() << "read_left: " << read_left;
      qDebug() << "SockCli->bytesAvailable(): " << SockCli->bytesAvailable();


      array.append(tmpArray);
      tmpArray.append((QByteArray)SockCli->readAll());
      qDebug() << "Received finished! array.length() = " << array.length();
      qDebug() << "emit SigRecvFinished()";
      emit SigRecvFinished();

      datasize = 0;
   }

   Sem.release();  //释放信号量
   qDebug() << "Sem.available(): " << Sem.available();

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

