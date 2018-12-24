#include "TcpServer.h"
#include "ui_TcpServer.h"


#define MAXNUM  100   //最大监听数

TcpServer::TcpServer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TcpServer)
{
    ui->setupUi(this);
    Init();
}

TcpServer::~TcpServer()
{
    delete ui;
}


/*===========================================function=============================================*/
void TcpServer::Init()
{
    myServer = new QTcpServer(this);

}
/*=========================================end function===========================================*/


/*=============================================slot===============================================*/
void TcpServer::on_pushButton_Bind_clicked()
{
    QString serverIP;
    QString serverPort;
    QString msg;

//    //方法一，自动获取IP
//    QList<QHostAddress> addrs = QNetworkInterface::allAddresses();
//    for(int i=0;i<addrs.length();i++){
//        QHostAddress addr = addrs.at(i);
//        if(addr.toString().contains("192.")){
//            serverIP = addr.toString();
//            ui->lineEdit_ServerIp->setText(serverIP);
//            break;
//        }
//    }
    //方法二，手动设置IP
    serverIP = ui->lineEdit_ServerIp->text();
    serverPort = ui->lineEdit_ServerPort->text();

    bool ret = myServer->listen(QHostAddress(serverIP),serverPort.toUInt());  //绑定IP、端口
    if(!ret){
        msg = "绑定失败！";
    }
    else{
        msg = "绑定成功！";
        ui->pushButton_Bind->setEnabled(false);
    }

    ui->textEdit_Server->append(msg);
    myServer->setMaxPendingConnections(MAXNUM);  //设置最大监听数

    connect(myServer,SIGNAL(newConnection()),this,SLOT(doProcessNewConnection()));
    //connect(myServer,SIGNAL(acceptError(QAbstractSocket::SocketError)),this,SLOT(doProcessAcceptError(QAbstractSocket::SocketError)));

}

//新客户端连接请求处理
void TcpServer::doProcessNewConnection()
{
     client = myServer->nextPendingConnection();   //获取客户端连接的描述符
     arrayClient.append(client);                   //存入客户端buffer
     //客户端连接
     connect(client,SIGNAL(connected()),this,SLOT(doProcessConnected()));
     //客户端断开
     connect(client,SIGNAL(disconnected()),this,SLOT(doProcessDisconnected()));
     //读取内容
     connect(client,SIGNAL(readyRead()),this,SLOT(doProcessreadyRead()));


}

//连接成功
void TcpServer::doProcessConnected()
{
    QTcpSocket *client = (QTcpSocket*)this->sender();
    QString msg = QString("客户端[%1:%2]成功接入！")
            .arg(client->peerAddress().toString())
            .arg(client->peerPort());
    ui->textEdit_Server->append(msg);
}

//连接错误
//void Mymonitor::doProcessAcceptError(QAbstractSocket::SocketError err)
//{
//    ui->textEdit_Server->append(QString("客户端连接错误！错误码："));
//    ui->textEdit_Server->append(QString(err));
//}


//客户端断开
void TcpServer::doProcessDisconnected()
{
    QTcpSocket *client = (QTcpSocket*)this->sender();  //QObject * QObject::sender ()在信号激活的槽中调用，返回发送信号的对象的指针
    QString msg = QString("客户端[%1:%2]断开！")
            .arg(client->peerAddress().toString())
            .arg(client->peerPort());
    ui->textEdit_Server->append(msg);

    //删除对应客户端
    for(int i=0;i<arrayClient.length();i++){
        if(arrayClient.at(i)->peerAddress() == client->peerAddress()){
            if(arrayClient.at(i)->peerPort() == client->peerPort()){
                arrayClient.removeAt(i);
                break;
            }
        }
    }

}


//接收数据
void TcpServer::doProcessreadyRead(){
    QTcpSocket *client = (QTcpSocket*)this->sender();
    QString msg = QString("来自客户端[%1:%2]:")
            .arg(client->peerAddress().toString())
            .arg(client->peerPort());
    ui->textEdit_Server->append(msg);
    while(!client->atEnd()){
        QString recv = QString(client->readAll());
        ui->textEdit_Server->append(recv);
    }
}


void TcpServer::on_pushButton_sendToClient_clicked()
{
    QString ip = ui->lineEdit_clientIP->text();
    QString port = ui->lineEdit_clientPort->text();

    //遍历列表，查找对应客户端
    for(int i=0;i<arrayClient.length();i++){
        if((arrayClient.at(i)->peerAddress()).toString() == ip){
            if(arrayClient.at(i)->peerPort() == port.toUInt()){
                QString msg = ui->textEdit_Client->toPlainText();
                arrayClient.at(i)->write(msg.toUtf8());
                ui->textEdit_Client->clear();
                break;
            }
        }
    }
}


void TcpServer::on_pushButton_clearRecv_clicked()
{
    ui->textEdit_Server->clear();
}
/*===========================================end slot=============================================*/
