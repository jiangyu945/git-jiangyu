#include "workerthread.h"

QMutex myMutex;
extern  QByteArray array;
static  QByteArray tmpArray;
QSemaphore Sem(1);   //定义只含一个信号灯的二值信号量

WorkerThread::WorkerThread(QObject *parent) : QObject(parent)
{
    myClient = new QTcpSocket(this);   //资源由父对象this回收
    datasize = 0;
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

    tt.start();
    Sem.acquire();  //获取二值信号量

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

      array.append(tmpArray);  //拷贝数据到图像存储buffer
      tmpArray.clear();        //清除临时缓存buffer

      tmpArray.append((QByteArray)myClient->readAll());     //接收剩余的下一帧图片数据
      qDebug() << "Received finished! array.size() = " << array.size();
      qDebug() << "emit SigRecvFinished()";
      emit SigRecvFinished();

      qDebug("Receive data's  Time elapsed: %d ms",tt.elapsed());  //打印耗时

      datasize = 0;
   }

   Sem.release();  //释放信号量
}

