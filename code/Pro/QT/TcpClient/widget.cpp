#include "widget.h"
#include "ui_widget.h"

#include <QAbstractSocket>
#include <QDebug>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    Init();
}

Widget::~Widget()
{
    delete ui;
}

/*===========================================function=============================================*/
void Widget::Init()
{
    QPixmap initImg(":/img/player.png");
    initImg.scaled(ui->label_show->size(), Qt::KeepAspectRatio); //图片大小自适应label标签
    ui->label_show->setScaledContents(true);
    ui->label_show->setPixmap(initImg);

    myClient = new QTcpSocket(this);
    mytimer  = new QTimer(this);
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

//数据接收及处理
void Widget::doProcessReadyRead()
{

    //清空接收缓冲区
    array.clear();
    qDebug()<<"start to recv data...";

    //接收图像数据
    while(!myClient->atEnd()){
            array.append(QByteArray(myClient->readAll()));
        }
    qDebug() << "Received img data finished,willing to show...";

    //加载图像数据到img
    QImage img;
    img.loadFromData(array);
    if(!img.isNull()){
        qDebug()<<"right"<<endl;

        //实现显示窗口自适应主窗体大小变化
        img.scaled(ui->label_show->size(),Qt::KeepAspectRatio);
        ui->label_show->setScaledContents(true);

        //显示图像
        ui->label_show->setPixmap(QPixmap::fromImage(img));

        //刷新屏幕
        mytimer->start(1);
//        update();  //下次循环刷新
        repaint();   //立即刷新
    }
    else {
        qDebug()<<"error"<<endl;
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
