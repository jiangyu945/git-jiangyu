#include "widget.h"
#include "workerthread.h"
#include "ui_widget.h"

//定义QByteArray数组,用来存储图像数据
QByteArray  array;


extern QMutex myMutex;
WorkerThread* workerObj = new WorkerThread;  //此处对象workerObj必须new出来，否则主线程信号无法传达到次线程中！原因未知

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    //将参数注册为元数据类型
    qRegisterMetaType<QTcpSocket*>("QTcpSocket*");

    Init();
    startObjthread();
}

Widget::~Widget()
{
    //等待次线程的结束
    worker.quit();
    worker.wait();
    delete ui;
}

/*===========================================function=============================================*/
void Widget::Init()
{
    //设置Label标签初始显示图片
    QPixmap initImg(":/img/player.png");
    initImg.scaled(ui->label_show->size(), Qt::KeepAspectRatio); //图片大小自适应label标签
    ui->label_show->setScaledContents(true);
    ui->label_show->setPixmap(initImg);  //在label上显示图片

    //默认服务器IP、端口
    ui->lineEdit_ServerIP->setText(QString("192.168.31.107"));
    ui->lineEdit_ServerPort->setText(QString("8887"));

}

void Widget::startObjthread(){
    workerObj->moveToThread(&worker);            //将对象移动到线程
    connect(this,SIGNAL(SigToConnect(QString,QString)),workerObj,SLOT(doProcessConnectToServer(QString,QString)));  //建立连接槽
    connect(workerObj,SIGNAL(SigToConnected()),this,SLOT(doProcessConnected()));      //连接成功槽
    connect(workerObj,SIGNAL(SigDisConnected()),this,SLOT(doProcessDisconnected()));  //连接断开槽

    //线程结束后自动销毁
    connect(&worker,SIGNAL(finished()),workerObj,SLOT(deleteLater()));    //资源回收槽

    connect(workerObj,SIGNAL(SigRecvFinished()),this,SLOT(doProcessShow()));  //图像显示槽
//    connect(&mytimer,SIGNAL(timeout()),this,SLOT(repaint()));                 //定时刷新屏幕，若为update()则下次循环才刷新


    //启动线程
    worker.start();
}
/*=========================================end function===========================================*/


/*=============================================slot===============================================*/

//连接服务器
void Widget::on_pushButton_connectToServer_clicked()
{
    serverIP = ui->lineEdit_ServerIP->text();
    serverPort = ui->lineEdit_ServerPort->text();

    emit SigToConnect(serverIP,serverPort);
}

//连接服务器成功
void Widget::doProcessConnected()
{
    QString msg = QString("连接服务器成功！");
    ui->textEdit_RecvBuf->append(msg);
    ui->pushButton_connectToServer->setEnabled(false);
}

//图像显示
void Widget::doProcessShow(){

    t.start();
    qDebug() << "Showing..." << endl;
    //加锁
    QMutexLocker locker(&myMutex);

    //加载图像数据到img
    QImage img;

    //加载方法一
    img.loadFromData(array);
//    //加载方法二
//    QBuffer buffer(&array);
//    buffer.open( QIODevice::ReadOnly );
//    QImageReader reader(&buffer);
//    img = reader.read();

    //清空接收缓冲区
    array.clear();

    if(!img.isNull()){

        //实现显示窗口自适应主窗体大小变化
        img.scaled(ui->label_show->size(),Qt::KeepAspectRatio);
        ui->label_show->setScaledContents(true);

        //显示图像
        ui->label_show->setPixmap(QPixmap::fromImage(img));

        //立即刷新屏幕
        repaint();

        //开启定时器，超时刷新屏幕
//        mytimer.start(33);
        qDebug("Time elapsed: %d ms",t.elapsed());  //打印耗时


    }
    else {
        qDebug()<<"img is NULL" << endl;
    }   


    //接收文本数据
  //    QString str,msg;
  //    str = QString("来自服务器 [%1:%2]:")
  //            .arg(myClient->peerAddress().toString())
  //            .arg(myClient->peerPort());
  //    ui->textEdit_RecvBuf->append(str);
  //    while(!myClient->atEnd()){
  //         msg = QString(myClient->readAll());
  //    }
  //    ui->textEdit_RecvBuf->append(msg);
}

//服务器退出
void Widget::doProcessDisconnected(){
    QString msg = QString("服务器退出！");
    ui->textEdit_RecvBuf->append(msg);
    ui->pushButton_connectToServer->setEnabled(true);
}


/*===========================================end slot=============================================*/
