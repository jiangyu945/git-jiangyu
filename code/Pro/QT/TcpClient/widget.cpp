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
    myClient = new QTcpSocket(this);
}
/*=========================================end function===========================================*/

/*=============================================slot===============================================*/

void Widget::on_pushButton_connectToServer_clicked()
{
    QString serverIP = ui->lineEdit_ServerIP->text();
    QString serverPort = ui->lineEdit_ServerPort->text();

    myClient->connectToHost(QHostAddress(serverIP),serverPort.toUInt());  //连接服务器

    connect(myClient,SIGNAL(connected()),this,SLOT(doProcessConnected()));
    connect(myClient,SIGNAL(readyRead()),this,SLOT(doProcessReadyRead()));
    connect(myClient,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(doProcessError(QAbstractSocket::SocketError)));
    connect(myClient,SIGNAL(disconnected()),this,SLOT(doProcessDisconnected()));


}

void Widget::doProcessConnected()
{
    QString msg = QString("连接服务器成功！");
    ui->textEdit_RecvBuf->append(msg);
    ui->pushButton_connectToServer->setEnabled(false);
}

void Widget::doProcessReadyRead()
{
    QString str,msg;
    str = QString("来自服务器 [%1:%2]:")
            .arg(myClient->peerAddress().toString())
            .arg(myClient->peerPort());
    ui->textEdit_RecvBuf->append(str);
    while(!myClient->atEnd()){
         msg = QString(myClient->readAll());
    }
    ui->textEdit_RecvBuf->append(msg);
}

void Widget::doProcessError(QAbstractSocket::SocketError err){
    qDebug() << err;
}

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
