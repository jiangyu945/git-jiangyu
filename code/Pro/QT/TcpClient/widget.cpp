#include "widget.h"
#include "ui_widget.h"

#include <QAbstractSocket>
#include <QDebug>

//定义QByteArray数组,用来存储图像数据
QByteArray  array;

extern QMutex myMutex;

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

    myClient = new QTcpSocket(this);   //资源由父对象this回收

}

void Widget::startObjthread(){

    WorkerThread* workerObj = new WorkerThread;  //此处对象workerObj必须new出来，否则主线程信号无法传达到次线程中！原因未知
    workerObj->moveToThread(&worker);            //将对象移动到线程
    connect(this,SIGNAL(SigToThread(QTcpSocket*)),workerObj,SLOT(doProcessRecvData(QTcpSocket*)));


    //线程结束后自动销毁
    connect(&worker,SIGNAL(finished()),workerObj,SLOT(deleteLater()));

    connect(workerObj,SIGNAL(SigRecvFinished()),this,SLOT(doProcessShow()));

    //启动线程
    worker.start();
}
/*=========================================end function===========================================*/


/*=============================================slot===============================================*/

//连接服务器
void Widget::on_pushButton_connectToServer_clicked()
{
    QString serverIP = ui->lineEdit_ServerIP->text();
    QString serverPort = ui->lineEdit_ServerPort->text();

    myClient->connectToHost(QHostAddress(serverIP),serverPort.toUInt());

    connect(myClient,SIGNAL(connected()),this,SLOT(doProcessConnected()));
    connect(myClient,SIGNAL(readyRead()),this,SLOT(doProcessReadyRead()));

    connect(myClient,SIGNAL(disconnected()),this,SLOT(doProcessDisconnected()));
}

//连接服务器成功
void Widget::doProcessConnected()
{
    QString msg = QString("连接服务器成功！");
    ui->textEdit_RecvBuf->append(msg);
    ui->pushButton_connectToServer->setEnabled(false);
}

//数据接收
void Widget::doProcessReadyRead()
{
    qDebug() << "Tcp connect in Thread ID: " << QThread::currentThreadId();

    //发送信号到线程
    qDebug() << "emit SigToThread(myClient)" ;
    emit SigToThread(myClient);
}

void Widget::doProcessShow(){

    qDebug() << "Image show in Thread ID: " << QThread::currentThreadId() ;

    //加锁
    myMutex.lock();

    //加载图像数据到img
    QImage img;
    img.loadFromData(array);
    //清空接收缓冲区
    array.clear();

    //解锁
    myMutex.unlock();

    qDebug() << "Img size : " << img.byteCount() << endl;
    if(!img.isNull()){

        //实现显示窗口自适应主窗体大小变化
        img.scaled(ui->label_show->size(),Qt::KeepAspectRatio);
        ui->label_show->setScaledContents(true);

        //显示图像
        ui->label_show->setPixmap(QPixmap::fromImage(img));

        //立即刷新屏幕
        repaint();   //update(); //下次循环才刷新
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

void Widget::on_pushButton_sendToServer_clicked()
{
    QString msg = ui->textEdit_SenBbuf->toPlainText();
    int ret = myClient->write(msg.toUtf8());
    if(ret <= 0){
        return;
    }
    ui->textEdit_SenBbuf->clear();
}

/*===========================================end slot=============================================*/
